
#include "network_server.hpp"

using json = nlohmann::json;

namespace
{
    constexpr int MAX_ELO_DIFFERENCE = 100;
    constexpr int MATCHMAKING_TIMEOUT_SEC = 60;
}

NetworkServer::NetworkServer()
{
    m_db = std::make_unique<DatabaseConnection>("kungfu_chess.db");
    m_userRepo = std::make_unique<UserRepository>(*m_db);
    m_userRepo->initializeTable();

    m_roomManager = std::make_unique<RoomManager>(
        [this](websocketpp::connection_hdl hdl, const std::string &msg)
        { this->sendToClient(hdl, msg); },
        *m_userRepo);

    m_matchmaker = std::make_unique<Matchmaker>(
        MAX_ELO_DIFFERENCE,
        MATCHMAKING_TIMEOUT_SEC,
        [this](auto hdl1, auto hdl2)
        { this->onMatchFound(hdl1, hdl2); },
        [this](auto hdl)
        { this->onMatchTimeout(hdl); });

    m_authHandler = std::make_unique<AuthHandler>(*m_userRepo);
    m_lobbyHandler = std::make_unique<LobbyHandler>(*m_roomManager, *m_matchmaker);

    m_server.clear_access_channels(websocketpp::log::alevel::all);

    m_server.init_asio();
    m_server.set_open_handler(bind(&NetworkServer::on_open, this, std::placeholders::_1));
    m_server.set_close_handler(bind(&NetworkServer::on_close, this, std::placeholders::_1));
    m_server.set_message_handler(bind(&NetworkServer::on_message, this, std::placeholders::_1, std::placeholders::_2));
}

void NetworkServer::sendToClient(websocketpp::connection_hdl hdl, const std::string &message)
{
    try
    {
        m_server.send(hdl, message, websocketpp::frame::opcode::text);
    }
    catch (const websocketpp::exception &e)
    {
        std::cout << "Send failed: " << e.what() << std::endl;
    }
}

void NetworkServer::run(uint16_t port)
{
    m_server.listen(port);
    m_server.start_accept();
    std::cout << "Server listening on port " << port << "..." << std::endl;

    start_timer();

    m_server.run();
}

void NetworkServer::on_open(websocketpp::connection_hdl hdl)
{
    connectedClients[hdl] = ClientSession();
    std::cout << "[NetworkServer] New client connected. Awaiting login." << std::endl;
}

void NetworkServer::on_close(websocketpp::connection_hdl hdl)
{
    auto it = connectedClients.find(hdl);
    if (it != connectedClients.end())
    {
        std::string roomId = it->second.roomId;
        if (!roomId.empty())
        {
            auto room = m_roomManager->getRoom(roomId);
            if (room)
            {
                room->handlePlayerDisconnect(hdl);
            }
        }
    }

    connectedClients.erase(hdl);
}

void NetworkServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg)
{
    std::string payload = msg->get_payload();
    auto &client = connectedClients[hdl];

    try
    {
        switch (client.state)
        {

        case ClientState::CONNECTED:
        {
            auto jOpt = parseJsonSafe(payload);
            if (!jOpt.has_value())
                return;

            if (jOpt.value().value("action", "") == "LOGIN")
            {
                std::string errorMsg;
                if (m_authHandler->handleLogin(jOpt.value(), client, errorMsg))
                {
                    sendToClient(hdl, R"({"type": "LOGIN_SUCCESS"})");
                }
                else
                {
                    json errorJson = {{"type", "LOGIN_ERROR"}, {"message", errorMsg}};
                    sendToClient(hdl, errorJson.dump());
                }
            }
            break;
        }

        case ClientState::LOBBY:
        {
            auto jOpt = parseJsonSafe(payload);
            if (!jOpt.has_value())
                return;

            std::string response = m_lobbyHandler->handleMessage(jOpt.value(), client, hdl);
            if (!response.empty())
            {
                sendToClient(hdl, response);
            }
            break;
        }

        case ClientState::IN_QUEUE:
        {
            break;
        }

        case ClientState::IN_ROOM:
        {
            if (payload.empty())
                break;

            // check if json
            if (payload[0] == '{')
            {
                auto jOpt = parseJsonSafe(payload);
                if (jOpt.has_value() && jOpt.value().value("action", "") == "LEAVE_ROOM")
                {
                    auto room = m_roomManager->getRoom(client.roomId);
                    if (room)
                    {
                        room->removePlayer(hdl);
                    }
                    client.roomId = "";
                    client.state = ClientState::LOBBY;
                }
                break;
            }

            auto room = m_roomManager->getRoom(client.roomId);
            if (room)
            {
                PlayerRole role = room->getSession()->getRole(hdl);
                GameActionHandler actionHandler(room->getSession()->getEngine());

                if (!actionHandler.processCommand(payload, role))
                {
                    std::cout << "[NetworkServer] Invalid game command: " << payload << std::endl;
                }
            }
            break;
        }
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "[NetworkServer] Unhandled exception: " << e.what() << std::endl;
        sendToClient(hdl, R"({"type": "ERROR", "message": "Internal server error"})");
    }
}

void NetworkServer::start_timer()
{
    m_server.set_timer(1000, [this](websocketpp::lib::error_code const &ec)
                       {
        if (!ec) {
            m_roomManager->checkAllRoomsForTimeouts();
            if (m_matchmaker) {
                m_matchmaker->processQueue(); 
            }
            this->start_timer();
        } });
}

std::optional<json> NetworkServer::parseJsonSafe(const std::string &payload)
{
    try
    {
        return json::parse(payload);
    }
    catch (const std::exception &e)
    {
        // LOG_MSG("JSON Parse Error: " + std::string(e.what()));
        return std::nullopt;
    }
}

void NetworkServer::onMatchFound(websocketpp::connection_hdl hdl1, websocketpp::connection_hdl hdl2)
{
    auto &client1 = connectedClients[hdl1];
    auto &client2 = connectedClients[hdl2];
    std::string roomName = "Ranked_" + client1.username + "_vs_" + client2.username;

    if (m_roomManager->createRoom(roomName))
    {
        auto room = m_roomManager->getRoom(roomName);

        client1.roomId = roomName;
        client1.state = ClientState::IN_ROOM;
        room->addPlayer(hdl1, client1.username);
        sendToClient(hdl1, R"({"type": "ROOM_SUCCESS", "message": "Match found!"})");

        client2.roomId = roomName;
        client2.state = ClientState::IN_ROOM;
        room->addPlayer(hdl2, client2.username);
        sendToClient(hdl2, R"({"type": "ROOM_SUCCESS", "message": "Match found!"})");
    }
}

void NetworkServer::onMatchTimeout(websocketpp::connection_hdl hdl)
{
    connectedClients[hdl].state = ClientState::LOBBY;
    sendToClient(hdl, R"({"type": "MATCH_ERROR", "message": "Could not find a match within 1 minute."})");
}
