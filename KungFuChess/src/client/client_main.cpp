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

using json = nlohmann::json;

void onMouse(int event, int x, int y, int flags, void *userdata)
{
    Controller *controller = static_cast<Controller *>(userdata);
    if (event == cv::EVENT_LBUTTONDOWN)
    {
        controller->click(x, y);
    }
    else if (event == cv::EVENT_LBUTTONDBLCLK)
    {
        controller->jump(x, y);
    }
}

int main()
{
    try
    {
        std::string username;
        std::string password;

        std::cout << "======================================\n";
        std::cout << "      Welcome to Kung Fu Chess      \n";
        std::cout << "======================================\n";
        std::cout << "Please Login (Shell Mode):\n";
        std::cout << "Username: ";
        std::cin >> username;
        std::cout << "Password: ";
        std::cin >> password;
        std::cout << "Connecting to server as " << username << "...\n";

        int rows = 8;
        int cols = 8;

        GameSnapshot localSnapshot;
        std::mutex snapshotMutex;

        EventBus clientBus;

        using clock = std::chrono::high_resolution_clock;
        auto startTime = clock::now();
        std::atomic<long> timeOffset{0};

        NetworkClient network;

        network.setOnMessageCallback([&localSnapshot, &snapshotMutex, &startTime, &timeOffset, &clientBus](const std::string &msg)
                                     {                                     
            try {
                auto j = json::parse(msg);
                GameSnapshot newSnap;
                
                newSnap.serverTime = j.value("serverTime", 0LL);
                newSnap.boardWidth = j.value("boardWidth", 8);
                newSnap.boardHeight = j.value("boardHeight", 8);
                newSnap.isGameOver = j.value("isGameOver", false);
                
                if (newSnap.isGameOver && j.contains("winner")) {
                    newSnap.winner = (j["winner"] == "White") ? PieceColor::White : PieceColor::Black;
                }

                if (j.contains("stationaryPieces")) {
                    for (const auto& pJson : j["stationaryPieces"]) {
                        Piece p(pJson["id"], 
                                pJson["color"] == "White" ? PieceColor::White : PieceColor::Black,
                                static_cast<PieceKind>(pJson["kind"]),
                                Position{pJson["row"], pJson["col"]});
                        
                        p.state = static_cast<PieceState>(pJson.value("state", 0)); 
                        p.readyTime = pJson.value("readyTime", 0LL); 
                        
                        newSnap.stationaryPieces.push_back(p);
                    }
                }

                if (j.contains("activeMotions")) {
                    for (const auto& mJson : j["activeMotions"]) {
                        PieceColor color = (mJson["pieceColor"] == "White") ? PieceColor::White : PieceColor::Black;
                        PieceKind kind = static_cast<PieceKind>(mJson["pieceKind"]);
                        
                        Piece p(mJson["pieceId"], color, kind, Position{0,0});
                        
                        MotionType mType = static_cast<MotionType>(mJson.value("type", 0));
                        p.state = (mType == MotionType::Jump) ? PieceState::Jump : PieceState::Move;
                        
                        Motion m {
                            p,
                            Position{mJson["sourceRow"], mJson["sourceCol"]},
                            Position{mJson["destRow"], mJson["destCol"]},
                            mJson.value("startTime", 0LL),
                            mJson.value("arrivalTime", 0LL),   
                            mType,
                            false
                        };
                        
                        newSnap.activeMotions.push_back(m);
                    }
                }

                if (j.contains("events")) {
                    for (const auto& eJson : j["events"]) {
                        std::string type = eJson.value("type", "");
                        
                        if (type == "PieceCapturedEvent") {
                            Piece p(eJson["pieceId"], 
                                    eJson["pieceColor"] == "White" ? PieceColor::White : PieceColor::Black,
                                    static_cast<PieceKind>(eJson["pieceKind"]), 
                                    Position{0, 0});
                                    
                            PieceCapturedEvent event(p);
                            clientBus.publish(event);
                        } 
                        else if (type == "MoveCompletedEvent") {
                            Position dest{eJson["destRow"], eJson["destCol"]};
                            Piece p(eJson["pieceId"], 
                                    eJson["pieceColor"] == "White" ? PieceColor::White : PieceColor::Black,
                                    static_cast<PieceKind>(eJson["pieceKind"]), 
                                    dest);
                                    
                            Position src{eJson["sourceRow"], eJson["sourceCol"]};
                            bool capture = eJson.value("destinationCapture", false);
                            long timeMs = eJson.value("timeMs", 0LL);
                            
                            MoveCompletedEvent event(p, src, dest, capture, timeMs);
                            clientBus.publish(event);
                        }
                    }
                }

                static bool offsetInitialized = false;
                long currentLocalTime = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - startTime).count();
                long currentOffset = newSnap.serverTime - currentLocalTime;
                
                if (!offsetInitialized) {
                    timeOffset = currentOffset;
                    offsetInitialized = true;
                } else {
                    timeOffset = (timeOffset.load() * 0.95) + (currentOffset * 0.05);
                }
                
                {
                    std::lock_guard<std::mutex> lock(snapshotMutex);
                    localSnapshot = newSnap;
                }
                
            } catch (const std::exception& e) {
                std::cerr << "JSON Parse Error: " << e.what() << std::endl;
            } });

        if (!network.connect("ws://localhost:9002"))
        {
            std::cerr << "Failed to connect to server!" << std::endl;
            return -1;
        }

        json loginMsg;
        loginMsg["action"] = "LOGIN";
        loginMsg["username"] = username;
        loginMsg["password"] = password;
        network.send(loginMsg.dump());

        ScoreManager scoreManager(&clientBus);
        MoveHistoryManager historyManager(&clientBus);

        Renderer renderer(1024, 720, rows, cols, 260, 100, 260, 100, "../assets/", &scoreManager, &historyManager);
        BoardMapper mapper(renderer.getBoardStartX(), renderer.getBoardStartY(), renderer.getCellSize());

        Controller controller(network, localSnapshot, mapper);

        std::string windowName = "Kung Fu Chess";
        cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
        cv::setMouseCallback(windowName, onMouse, &controller);

        long lastAbsoluteTime = 0;

        std::cout << "Starting Kung Fu Chess Client Loop..." << std::endl;

        while (true)
        {
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

            int key = cv::waitKey(1);
            if (key == 27)
                break;
        }

        network.disconnect();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error in main: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}