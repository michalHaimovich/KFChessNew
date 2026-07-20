#include <iostream>
#include <nlohmann/json.hpp>

#include "game/server_controller.hpp"
#include "game/game_session.hpp"
#include "network_server.hpp"

using json = nlohmann::json;

NetworkServer::NetworkServer()
{
    m_roomManager = std::make_unique<RoomManager>(
        [this](websocketpp::connection_hdl hdl, const std::string &msg)
        { this->sendToClient(hdl, msg); });

    m_db = std::make_unique<DatabaseConnection>("kungfu_chess.db");
    m_userRepo = std::make_unique<UserRepository>(*m_db);
    m_userRepo->initializeTable();

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
                PlayerRole role = room->getSession()->getRole(hdl);
                
                room->removePlayer(hdl);
                
                if (role == PlayerRole::White || role == PlayerRole::Black) {
                    room->startGracePeriod(hdl, role);
                }
            }
        }
    }

    connectedClients.erase(hdl);
}

void NetworkServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg)
{
    std::string payload = msg->get_payload();
    auto &client = connectedClients[hdl];

    if (client.state == ClientState::IN_ROOM)
    {
        auto room = m_roomManager->getRoom(client.roomId);
        if (room)
        {
            PlayerRole role = room->getSession()->getRole(hdl);
            ServerController controller(room->getSession()->getEngine());

            bool success = controller.processCommand(payload, role);
            if (!success)
            {
                std::cout << "[NetworkServer] Invalid game command or unauthorized role: " << payload << std::endl;
            }
        }
        return; 
    }

    if (client.state == ClientState::CONNECTED)
    {
        try
        {
            auto j = json::parse(payload);
            if (j.value("action", "") == "LOGIN")
            {
                std::string user = j.value("username", "Guest");
                std::string pass = j.value("password", "");

                auto existingUser = m_userRepo->getByUsername(user);
                bool loginSuccess = false;

                if (existingUser.has_value())
                {
                    if (existingUser->password == pass)
                    {
                        loginSuccess = true;
                        client.rating = existingUser->rating;
                        std::cout << "[NetworkServer] User authenticated: " << user << std::endl;
                    }
                    else
                    {
                        std::cout << "[NetworkServer] Invalid password for: " << user << std::endl;
                        sendToClient(hdl, R"({"type": "LOGIN_ERROR", "message": "Invalid password"})");
                    }
                }
                else
                {
                    User newUser(user, pass);
                    if (m_userRepo->create(newUser))
                    {
                        loginSuccess = true;
                        client.rating = newUser.rating;
                        std::cout << "[NetworkServer] New user registered and authenticated: " << user << std::endl;
                    }
                    else
                    {
                        std::cout << "[NetworkServer] Failed to create user: " << user << std::endl;
                        sendToClient(hdl, R"({"type": "LOGIN_ERROR", "message": "Database error"})");
                    }
                }

                if (loginSuccess)
                {
                    client.state = ClientState::LOBBY;
                    client.username = user;
                    sendToClient(hdl, R"({"type": "LOGIN_SUCCESS"})");
                }
            }
            else
            {
                std::cout << "[NetworkServer] Unauthenticated client sent non-login message." << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cout << "[NetworkServer] Invalid JSON during login: " << e.what() << std::endl;
        }
        return;
    }

    if (client.state == ClientState::LOBBY)
    {
        try
        {
            auto j = json::parse(payload);
            std::string action = j.value("action", "");

            if (action == "CREATE_ROOM")
            {
                std::string roomName = j.value("roomName", "");
                if (m_roomManager->createRoom(roomName))
                {
                    client.roomId = roomName;
                    client.state = ClientState::IN_ROOM;

                    auto room = m_roomManager->getRoom(roomName);
                    room->addPlayer(hdl, client.username);

                    sendToClient(hdl, R"({"type": "ROOM_SUCCESS", "message": "Room created"})");
                    std::cout << "[NetworkServer] Room created: " << roomName << std::endl;
                }
                else
                {
                    sendToClient(hdl, R"({"type": "ROOM_ERROR", "message": "Room name already exists"})");
                }
            }
            else if (action == "JOIN_ROOM")
            {
                std::string roomName = j.value("roomName", "");
                auto room = m_roomManager->getRoom(roomName);
                if (room)
                {
                    client.roomId = roomName;
                    client.state = ClientState::IN_ROOM;

                    room->addPlayer(hdl, client.username);

                    sendToClient(hdl, R"({"type": "ROOM_SUCCESS", "message": "Joined room"})");
                    std::cout << "[NetworkServer] Client joined room: " << roomName << std::endl;
                }
                else
                {
                    sendToClient(hdl, R"({"type": "ROOM_ERROR", "message": "Room not found"})");
                }
            }
            else if (action == "FIND_MATCH")
            {
                client.state = ClientState::IN_QUEUE;
                client.queueStartTime = std::chrono::steady_clock::now();
                std::cout << "[NetworkServer] Client entered matchmaking queue: " << client.username << " (Rating: " << client.rating << ")" << std::endl;
            }
            else
            {
                std::cout << "[NetworkServer] Unknown lobby action: " << action << std::endl;
            }
        }
        catch (const std::exception &e)
        {
            std::cout << "[NetworkServer] Invalid JSON in Lobby: " << e.what() << std::endl;
        }
    }
}

void NetworkServer::start_timer()
{
    m_server.set_timer(1000, [this](websocketpp::lib::error_code const & ec) {
        if (ec) {
            return; 
        }

        m_roomManager->checkAllRoomsForTimeouts();

        auto now = std::chrono::steady_clock::now();
        std::vector<websocketpp::connection_hdl> queueClients;

        for (auto& pair : connectedClients) {
            if (pair.second.state == ClientState::IN_QUEUE) {
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - pair.second.queueStartTime).count();
                
                if (duration >= 60) {
                    pair.second.state = ClientState::LOBBY;
                    sendToClient(pair.first, R"({"type": "MATCH_ERROR", "message": "Could not find a match within 1 minute."})");
                    std::cout << "[NetworkServer] Matchmaking timeout for: " << pair.second.username << std::endl;
                } else {
                    queueClients.push_back(pair.first);
                }
            }
        }

        for (size_t i = 0; i < queueClients.size(); ++i) {
            auto hdl1 = queueClients[i];
            if (connectedClients[hdl1].state != ClientState::IN_QUEUE) continue;

            for (size_t j = i + 1; j < queueClients.size(); ++j) {
                auto hdl2 = queueClients[j];
                if (connectedClients[hdl2].state != ClientState::IN_QUEUE) continue;

                int rating1 = connectedClients[hdl1].rating;
                int rating2 = connectedClients[hdl2].rating;

                if (std::abs(rating1 - rating2) <= 100) {
                    std::string newRoomName = "Ranked_" + connectedClients[hdl1].username + "_vs_" + connectedClients[hdl2].username;
                    
                    if (m_roomManager->createRoom(newRoomName)) {
                        auto room = m_roomManager->getRoom(newRoomName);
                        
                        connectedClients[hdl1].roomId = newRoomName;
                        connectedClients[hdl1].state = ClientState::IN_ROOM;
                        room->addPlayer(hdl1, connectedClients[hdl1].username);
                        sendToClient(hdl1, R"({"type": "ROOM_SUCCESS", "message": "Match found!"})");
                        
                        connectedClients[hdl2].roomId = newRoomName;
                        connectedClients[hdl2].state = ClientState::IN_ROOM;
                        room->addPlayer(hdl2, connectedClients[hdl2].username);
                        sendToClient(hdl2, R"({"type": "ROOM_SUCCESS", "message": "Match found!"})");

                        std::cout << "[NetworkServer] Match created: " << newRoomName << " (" << connectedClients[hdl1].username << " vs " << connectedClients[hdl2].username << ")" << std::endl;
                    }
                    break; 
                }
            }
        }

        this->start_timer(); 
    });
}