#include <iostream>
#include <chrono>
#include <optional>
#include <mutex>
#include <atomic>
#include <opencv2/opencv.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>

#include "network/network_client.hpp"
#include "model/game_snapshot.hpp"
#include "model/event_bus.hpp"
#include "input/controller.hpp"
#include "input/board_mapper.hpp"
#include "view/renderer.hpp"
#include "view/move_history_manager.hpp"
#include "view/score_manager.hpp"
#include "windows/login_dialog.hpp"
#include "windows/home_dialog.hpp"
#include "windows/room_dialog.hpp"

using json = nlohmann::json;

int ESC = 27;

int main()
{
    LoginResult loginInput = LoginDialog::ShowDialog();

    if (!loginInput.success)
    {
        std::cout << "User closed the login window. Exiting..." << std::endl;
        return 0; 
    }

    int rows = 8, cols = 8;
    GameSnapshot localSnapshot;
    std::mutex snapshotMutex;
    EventBus clientBus;

    using clock = std::chrono::high_resolution_clock;
    auto startTime = clock::now();
    std::atomic<long> timeOffset{0};

    NetworkClient network;

    std::atomic<int> loginStatus{0};
    std::string loginErrorMessage = "";
    std::mutex loginMutex;

    std::atomic<int> roomStatus{0};
    std::string roomErrorMessage = "";
    std::mutex roomMutex;

    network.setOnMessageCallback([&](const std::string &msg)
    {
        try
        {
            auto j = json::parse(msg);

            if (j.contains("type"))
            {
                std::string type = j["type"];
                if (type == "LOGIN_SUCCESS") { loginStatus = 1; return; }
                else if (type == "LOGIN_ERROR")
                {
                    std::lock_guard<std::mutex> lock(loginMutex);
                    loginErrorMessage = j.value("message", "Unknown error");
                    loginStatus = -1;
                    return;
                }
                else if (type == "ROOM_SUCCESS") { roomStatus = 1; return; }
                else if (type == "ROOM_ERROR" || type == "MATCH_ERROR")
                {
                    std::lock_guard<std::mutex> lock(roomMutex);
                    roomErrorMessage = j.value("message", "Unknown room/match error");
                    roomStatus = -1;
                    return;
                }
            }

            GameSnapshot newSnap;
            newSnap.serverTime = j.value("serverTime", 0LL);
            newSnap.boardWidth = j.value("boardWidth", 8);
            newSnap.boardHeight = j.value("boardHeight", 8);
            newSnap.isGameOver = j.value("isGameOver", false);
            
            if (j.contains("whitePlayerName")) newSnap.whitePlayerName = j["whitePlayerName"];
            if (j.contains("blackPlayerName")) newSnap.blackPlayerName = j["blackPlayerName"];

            if (newSnap.isGameOver && j.contains("winner"))
                newSnap.winner = (j["winner"] == "White") ? PieceColor::White : PieceColor::Black;

            if (j.contains("stationaryPieces"))
            {
                for (const auto &pJson : j["stationaryPieces"])
                {
                    Piece p(pJson["id"],
                            pJson["color"] == "White" ? PieceColor::White : PieceColor::Black,
                            static_cast<PieceKind>(pJson["kind"]),
                            Position{pJson["row"], pJson["col"]});
                    p.state = static_cast<PieceState>(pJson.value("state", 0));
                    p.readyTime = pJson.value("readyTime", 0LL);
                    newSnap.stationaryPieces.push_back(p);
                }
            }

            if (j.contains("activeMotions"))
            {
                for (const auto &mJson : j["activeMotions"])
                {
                    PieceColor color = (mJson["pieceColor"] == "White") ? PieceColor::White : PieceColor::Black;
                    PieceKind kind = static_cast<PieceKind>(mJson["pieceKind"]);
                    Piece p(mJson["pieceId"], color, kind, Position{0, 0});
                    MotionType mType = static_cast<MotionType>(mJson.value("type", 0));
                    p.state = (mType == MotionType::Jump) ? PieceState::Jump : PieceState::Move;

                    Motion m{p, Position{mJson["sourceRow"], mJson["sourceCol"]},
                             Position{mJson["destRow"], mJson["destCol"]},
                             mJson.value("startTime", 0LL), mJson.value("arrivalTime", 0LL),
                             mType, false};
                    newSnap.activeMotions.push_back(m);
                }
            }

            if (j.contains("events"))
            {
                for (const auto &eJson : j["events"])
                {
                    std::string type = eJson.value("type", "");
                    if (type == "PieceCapturedEvent")
                    {
                        Piece p(eJson["pieceId"], eJson["pieceColor"] == "White" ? PieceColor::White : PieceColor::Black,
                                static_cast<PieceKind>(eJson["pieceKind"]), Position{0, 0});
                        clientBus.publish(PieceCapturedEvent(p));
                    }
                    else if (type == "MoveCompletedEvent")
                    {
                        Position dest{eJson["destRow"], eJson["destCol"]};
                        Piece p(eJson["pieceId"], eJson["pieceColor"] == "White" ? PieceColor::White : PieceColor::Black,
                                static_cast<PieceKind>(eJson["pieceKind"]), dest);
                        Position src{eJson["sourceRow"], eJson["sourceCol"]};
                        bool capture = eJson.value("destinationCapture", false);
                        long timeMs = eJson.value("timeMs", 0LL);
                        clientBus.publish(MoveCompletedEvent(p, src, dest, capture, timeMs));
                    }
                }
            }

            static bool offsetInitialized = false;
            long currentLocalTime = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - startTime).count();
            long currentOffset = newSnap.serverTime - currentLocalTime;

            if (!offsetInitialized) { timeOffset = currentOffset; offsetInitialized = true; }
            else { timeOffset = (timeOffset.load() * 0.95) + (currentOffset * 0.05); }

            {
                std::lock_guard<std::mutex> lock(snapshotMutex);
                localSnapshot = newSnap;
            }

        }
        catch (const std::exception &e) { std::cerr << "JSON Parse Error: " << e.what() << std::endl; }
    });

    while (true) 
    {
        try
        {
            loginStatus = 0;
            roomStatus = 0;

            if (!network.isConnected())
            {
                if (!network.connect("ws://localhost:9002"))
                {
                    MessageBoxA(NULL, "Failed to connect to server!", "Connection Error", MB_ICONERROR | MB_OK);
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    continue; 
                }

                while (!network.isConnected()) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }

                json loginMsg;
                loginMsg["action"] = "LOGIN";
                loginMsg["username"] = loginInput.username;
                loginMsg["password"] = loginInput.password;
                network.send(loginMsg.dump());

                while (loginStatus.load() == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }

                if (loginStatus.load() == -1)
                {
                    MessageBoxA(NULL, "Authentication failed. Server might have reset.", "Login Error", MB_ICONERROR | MB_OK);
                    return 0;
                }
            }

            HomeResult homeInput = HomeDialog::ShowDialog(loginInput.username);

            if (homeInput.action == HomeAction::NONE)
            {
                network.disconnect();
                return 0; 
            }

            if (homeInput.action == HomeAction::ENTER_ROOM)
            {
                RoomResult roomInput = RoomDialog::ShowDialog();
                if (!roomInput.success) continue; 

                json roomMsg;
                roomMsg["action"] = roomInput.action;
                roomMsg["roomName"] = roomInput.roomName;
                network.send(roomMsg.dump());
                std::cout << "Waiting for room confirmation..." << std::endl;
            }
            else if (homeInput.action == HomeAction::PLAY_RANDOM)
            {
                json matchMsg;
                matchMsg["action"] = "FIND_MATCH";
                network.send(matchMsg.dump());
                std::cout << "Searching for a random match..." << std::endl;
            }

            while (roomStatus.load() == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }

            if (roomStatus.load() == -1)
            {
                std::string errMsg;
                { std::lock_guard<std::mutex> lock(roomMutex); errMsg = roomErrorMessage; }
                MessageBoxA(NULL, errMsg.c_str(), "Room/Match Error", MB_ICONERROR | MB_OK);
                continue; 
            }

            std::cout << "Match started! Loading game board..." << std::endl;

            ScoreManager scoreManager(&clientBus);
            MoveHistoryManager historyManager(&clientBus);

            Renderer renderer(1024, 720, rows, cols, 260, 100, 260, 100, "../assets/", &scoreManager, &historyManager);
            BoardMapper mapper(renderer.getBoardStartX(), renderer.getBoardStartY(), renderer.getCellSize());
            Controller controller(network, localSnapshot, mapper); 

            std::string windowName = "Kung Fu Chess";
            cv::namedWindow(windowName, cv::WINDOW_NORMAL);
            cv::resizeWindow(windowName, 1024, 720);

            cv::setMouseCallback(windowName, [](int event, int x, int y, int flags, void *userdata) {
                Controller *ctrl = static_cast<Controller *>(userdata);
                if (event == cv::EVENT_LBUTTONDOWN) ctrl->click(x, y);
                else if (event == cv::EVENT_LBUTTONDBLCLK) ctrl->jump(x, y);
            }, &controller);

            long lastAbsoluteTime = 0;
            bool returnToLobby = false;

            while (!returnToLobby)
            {
                cv::Rect windowRect = cv::getWindowImageRect(windowName);
                if (windowRect.width > 0 && windowRect.height > 0)
                {
                    if (windowRect.width != renderer.getWindowWidth() || windowRect.height != renderer.getWindowHeight())
                    {
                        renderer.updateWindowSize(windowRect.width, windowRect.height);
                        mapper.updateLayout(renderer.getBoardStartX(), renderer.getBoardStartY(), renderer.getCellSize());
                    }
                }

                auto now = clock::now();
                long absoluteTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
                long dt = absoluteTime - lastAbsoluteTime;
                lastAbsoluteTime = absoluteTime;
                long serverSyncedTime = absoluteTime + timeOffset.load();

                GameSnapshot renderSnapshot;
                {
                    std::lock_guard<std::mutex> lock(snapshotMutex);
                    renderSnapshot = localSnapshot;
                }

                std::optional<Position> currentSelection = controller.getSelectedCell();
                if (currentSelection.has_value())
                {
                    renderSnapshot.selectedPiece = renderSnapshot.getPieceAt(currentSelection.value());
                }

                renderer.renderFrame(renderSnapshot, serverSyncedTime, dt);

                if (controller.shouldReturnToLobby())
                {
                    std::cout << "Returning to Lobby..." << std::endl;
                    returnToLobby = true; 
                    break; 
                }

                int key = cv::waitKey(16);
                if (key == ESC)
                {
                    cv::destroyAllWindows();
                    return 0;
                }
            }

            cv::destroyWindow(windowName);
            network.disconnect();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        catch (const std::exception &e)
        {
            std::cerr << "Game Error: " << e.what() << " - Recovering to Lobby..." << std::endl;
            cv::destroyAllWindows();
            network.disconnect();
        }
    } 
    return 0;
}