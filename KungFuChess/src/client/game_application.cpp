#include "game_application.hpp"

namespace {
    int ESC = 27;
    int BEST_RENDER_MS = 16;
}

GameApplication::GameApplication()
{
    startTime = std::chrono::high_resolution_clock::now();
}

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

bool GameApplication::joinRoom(const std::string &username)
{
    HomeResult homeInput = HomeDialog::ShowDialog(username);

    if (homeInput.action == HomeAction::NONE)
    {
        return false;
    }

    if (homeInput.action == HomeAction::ENTER_ROOM)
    {
        RoomResult roomInput = RoomDialog::ShowDialog();
        if (!roomInput.success)
        {
            return false;
        }

        nlohmann::json roomMsg;
        roomMsg["action"] = roomInput.action;
        roomMsg["roomName"] = roomInput.roomName;
        network.send(roomMsg.dump());

        std::cout << "Waiting for room confirmation..." << std::endl;
    }
    else if (homeInput.action == HomeAction::PLAY_RANDOM)
    {
        nlohmann::json matchMsg;
        matchMsg["action"] = "FIND_MATCH";
        network.send(matchMsg.dump());

        std::cout << "Searching for a random match... (up to 60 seconds)" << std::endl;
    }

    while (roomStatus.load() == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (roomStatus.load() == -1)
    {
        std::string errMsg;
        {
            std::lock_guard<std::mutex> lock(roomMutex);
            errMsg = roomErrorMessage;
        }
        MessageBoxA(NULL, errMsg.c_str(), "Room/Match Error", MB_ICONERROR | MB_OK);
        return false;
    }

    std::cout << "Match found/joined! Loading game board..." << std::endl;
    return true;
}

void GameApplication::runGameLoop(const std::string& username) {
    std::cout << "[DEBUG] 1. Entered runGameLoop" << std::endl;
    
    try {
        ScoreManager scoreManager(&clientBus);
        MoveHistoryManager historyManager(&clientBus);
        
        std::cout << "[DEBUG] 2. Initializing Renderer" << std::endl;
        Renderer renderer(Theme::WindowWidth, Theme::WindowHeight, Theme::BoardCells, Theme::BoardCells, 
                          Theme::BoardStartX, Theme::BoardStartY, Theme::UIOffsetX, Theme::UIOffsetY, 
                          Theme::AssetsPath, &scoreManager, &historyManager);
        
        std::cout << "[DEBUG] 3. Setting up Controller" << std::endl;
        BoardMapper mapper(renderer.getBoardStartX(), renderer.getBoardStartY(), renderer.getCellSize());
        Controller controller(network, localSnapshot, mapper);

        std::cout << "[DEBUG] 4. Creating OpenCV Window" << std::endl;
        cv::namedWindow(Theme::WindowName, cv::WINDOW_NORMAL);
        cv::resizeWindow(Theme::WindowName, Theme::WindowWidth, Theme::WindowHeight);
        cv::setMouseCallback(Theme::WindowName, onMouse, &controller);
        
        cv::waitKey(Theme::UiRefreshDelayMs); 

        std::cout << "[DEBUG] 5. Starting inner game loop" << std::endl;
        
        auto nowInit = std::chrono::high_resolution_clock::now();
        long lastAbsoluteTime = std::chrono::duration_cast<std::chrono::milliseconds>(nowInit - startTime).count();
        bool returnToLobby = false;

        while (!returnToLobby) {
            try {
                cv::Rect windowRect = cv::getWindowImageRect(Theme::WindowName);
                if (windowRect.width > 0 && windowRect.height > 0) {
                    if (windowRect.width != renderer.getWindowWidth() || windowRect.height != renderer.getWindowHeight()) {
                        renderer.updateWindowSize(windowRect.width, windowRect.height);
                        mapper.updateLayout(renderer.getBoardStartX(), renderer.getBoardStartY(), renderer.getCellSize());
                    }
                }
            } catch (...) {
            }

            auto now = std::chrono::high_resolution_clock::now();
            long absoluteTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
            long dt = absoluteTime - lastAbsoluteTime;
            lastAbsoluteTime = absoluteTime;
            long serverSyncedTime = absoluteTime + timeOffset.load();

            GameSnapshot renderSnapshot;
            {
                std::lock_guard<std::mutex> lock(snapshotMutex);
                renderSnapshot = localSnapshot;
            }

            if (auto currentSelection = controller.getSelectedCell()) {
                renderSnapshot.selectedPiece = renderSnapshot.getPieceAt(*currentSelection);
            }

            renderer.renderFrame(renderSnapshot, serverSyncedTime, dt);

            if (controller.shouldReturnToLobby()) {
                returnToLobby = true; 
            }

            if (cv::waitKey(Theme::FpsDelayMs) == Theme::KeyEsc) {
                returnToLobby = true;
            }
        }

        cv::destroyWindow(Theme::WindowName);
        
    } catch (const std::exception& e) {
        std::cerr << "\n[CRASH IN GAME LOOP]: " << e.what() << "\n" << std::endl;
    }
}

void GameApplication::setupNetworkCallbacks()
{
    network.setOnMessageCallback([this](const std::string &msg)
                                 {
        try {
            handleServerMessage(msg);
        } catch (const std::exception &e) {
            std::cerr << "Network parsing error: " << e.what() << std::endl;
        } });
}

void GameApplication::handleServerMessage(const std::string& msg) {
    auto j = nlohmann::json::parse(msg);

    if (j.contains("type")) {
        std::string type = j["type"];
        
        if (type == "LOGIN_SUCCESS") {
            loginStatus = 1;
            return;
        }
        else if (type == "LOGIN_ERROR") {
            std::lock_guard<std::mutex> lock(loginMutex);
            loginErrorMessage = j.value("message", "Unknown error");
            loginStatus = -1;
            return;
        }
        else if (type == "ROOM_SUCCESS") {
            roomStatus = 1;
            return;
        }
        else if (type == "ROOM_ERROR" || type == "MATCH_ERROR") {
            std::lock_guard<std::mutex> lock(roomMutex);
            roomErrorMessage = j.value("message", "Unknown room/match error");
            roomStatus = -1;
            return;
        }
    }

    parseGameState(j); 
}

void GameApplication::parseGameState(const nlohmann::json& j) {
    GameSnapshot newSnap;
    newSnap.serverTime = j.value("serverTime", 0LL);
    newSnap.boardWidth = j.value("boardWidth", 8);
    newSnap.boardHeight = j.value("boardHeight", 8);
    newSnap.isGameOver = j.value("isGameOver", false);
    
    if (j.contains("whitePlayerName"))
        newSnap.whitePlayerName = j["whitePlayerName"];
    if (j.contains("blackPlayerName"))
        newSnap.blackPlayerName = j["blackPlayerName"];

    if (newSnap.isGameOver && j.contains("winner")) {
        newSnap.winner = (j["winner"] == "White") ? PieceColor::White : PieceColor::Black;
    }

    if (j.contains("stationaryPieces")) {
        for (const auto &pJson : j["stationaryPieces"]) {
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
        for (const auto &mJson : j["activeMotions"]) {
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

    if (j.contains("events")) {
        for (const auto &eJson : j["events"]) {
            std::string type = eJson.value("type", "");
            if (type == "PieceCapturedEvent") {
                Piece p(eJson["pieceId"], eJson["pieceColor"] == "White" ? PieceColor::White : PieceColor::Black,
                        static_cast<PieceKind>(eJson["pieceKind"]), Position{0, 0});
                PieceCapturedEvent event(p);
                clientBus.publish(event);
            }
            else if (type == "MoveCompletedEvent") {
                Position dest{eJson["destRow"], eJson["destCol"]};
                Piece p(eJson["pieceId"], eJson["pieceColor"] == "White" ? PieceColor::White : PieceColor::Black,
                        static_cast<PieceKind>(eJson["pieceKind"]), dest);
                Position src{eJson["sourceRow"], eJson["sourceCol"]};
                bool capture = eJson.value("destinationCapture", false);
                long timeMs = eJson.value("timeMs", 0LL);
                MoveCompletedEvent event(p, src, dest, capture, timeMs);
                clientBus.publish(event);
            }
        }
    }

    static bool offsetInitialized = false;
    long currentLocalTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
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
}

bool GameApplication::performLogin(std::string &outUsername)
{
    LoginResult loginInput = LoginDialog::ShowDialog();
    if (!loginInput.success)
        return false;

    outUsername = loginInput.username;

    setupNetworkCallbacks();

    if (!network.connect("ws://localhost:9002"))
    {
        MessageBoxA(NULL, "Failed to connect to server!", "Connection Error", MB_ICONERROR | MB_OK);
        return false;
    }

    while (!network.isConnected())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    nlohmann::json loginMsg;
    loginMsg["action"] = "LOGIN";
    loginMsg["username"] = loginInput.username;
    loginMsg["password"] = loginInput.password;
    network.send(loginMsg.dump());

    while (loginStatus.load() == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (loginStatus.load() == -1)
    {
        std::lock_guard<std::mutex> lock(loginMutex);
        MessageBoxA(NULL, loginErrorMessage.c_str(), "Login Failed", MB_ICONERROR | MB_OK);
        return false;
    }

    return true;
}

void GameApplication::run() {
    while (true) {
        loginStatus = 0;
        roomStatus = 0;
        timeOffset = 0;

        std::string username;
        
        if (!performLogin(username)) {
            break; 
        }

        if (!joinRoom(username)) {
            network.disconnect();
            continue; 
        }

        runGameLoop(username);
        
        network.disconnect();
    }
}