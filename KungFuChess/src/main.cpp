#include <iostream>
#include <chrono>
#include <optional>
#include <opencv2/opencv.hpp>

#include "engine/game_engine.hpp"
#include "input/controller.hpp"
#include "input/board_mapper.hpp"
#include "view/renderer.hpp"
#include "view/move_history_manager.hpp"
#include "view/score_manager.hpp"

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
        int rows = 8;
        int cols = 8;
        int marginLeft = 200;
        int marginTop = 120;
        int cellSize = 70;

        int windowWidth = 1000;
        int windowHeight = 700;

        GameEngine engine(rows, cols);

        engine.setupStandardBoard();

        ScoreManager scoreManager;
        MoveHistoryManager historyManager;

        engine.addObserver(&scoreManager);
        engine.addObserver(&historyManager);

        Renderer renderer(1024, 720, 8, 8, 260, 100, 260, 100, "assets/", &scoreManager, &historyManager);

        BoardMapper mapper(renderer.getBoardStartX(),
                           renderer.getBoardStartY(),
                           renderer.getCellSize());

        Controller controller(engine, mapper);

        std::string windowName = "Kung Fu Chess";
        cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

        cv::setMouseCallback(windowName, onMouse, &controller);

        using clock = std::chrono::high_resolution_clock;
        auto startTime = clock::now();
        long lastAbsoluteTime = 0;

        std::cout << "Starting Kung Fu Chess Real-Time Loop..." << std::endl;

        while (true)
        {
            auto now = clock::now();

            long absoluteTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

            long dt = absoluteTime - lastAbsoluteTime;
            lastAbsoluteTime = absoluteTime;

            engine.wait(dt);

            std::optional<Position> currentSelection = controller.getSelectedCell();

            GameSnapshot snapshot = engine.getSnapshot(currentSelection);
            
            renderer.renderFrame(snapshot, absoluteTime, dt);

            int key = cv::waitKey(1);
            
            if (key == 27)
                break;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error in main: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}