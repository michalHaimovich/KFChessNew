#include <iostream>
#include <chrono>
#include <optional>
#include <opencv2/opencv.hpp>
#include <websocketpp/client.hpp>

#include "network/network_client.hpp"
#include "model/game_snapshot.hpp" 

#include "input/controller.hpp"
#include "input/board_mapper.hpp"
#include "view/renderer.hpp"
#include "view/move_history_manager.hpp"
#include "view/score_manager.hpp"

void onMouse(int event, int x, int y, int flags, void *userdata) {
    Controller *controller = static_cast<Controller *>(userdata);
    if (event == cv::EVENT_LBUTTONDOWN) {
        controller->click(x, y);
    } else if (event == cv::EVENT_LBUTTONDBLCLK) {
        controller->jump(x, y);
    }
}

int main() {
    try {
        // --- הגדרות תצוגה ---
        int rows = 8;
        int cols = 8;
        int windowWidth = 1000;
        int windowHeight = 700;

        // --- 1. יצירת עותק לוח מקומי ---
        GameSnapshot localSnapshot; 
        
        // --- 2. הפעלת רכיב התקשורת ---
        NetworkClient network;
        
        // הגדרת מה קורה כשהשרת שולח עדכון
        network.setOnMessageCallback([&localSnapshot](const std::string& msg) {
            // TODO: כאן נכתוב בעתיד פונקציה שמפרשת את המחרוזת מהשרת
            // ומעדכנת את ה-localSnapshot בהתאם!
            std::cout << "Server says: " << msg << std::endl;
        });

        // חיבור לשרת (חייב להיות דלוק ברקע)
        if (!network.connect("ws://localhost:9002")) {
            std::cerr << "Failed to connect to server!" << std::endl;
            return -1;
        }

        // --- 3. הגדרות תצוגה וקלט ---
        ScoreManager scoreManager;
        MoveHistoryManager historyManager;

        Renderer renderer(1024, 720, rows, cols, 260, 100, 260, 100, "assets/", &scoreManager, &historyManager);
        BoardMapper mapper(renderer.getBoardStartX(), renderer.getBoardStartY(), renderer.getCellSize());
        
        // ה-Controller מקבל עכשיו את הרשת ואת הלוח המקומי
        Controller controller(network, localSnapshot, mapper);

        std::string windowName = "Kung Fu Chess";
        cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
        cv::setMouseCallback(windowName, onMouse, &controller);

        using clock = std::chrono::high_resolution_clock;
        auto startTime = clock::now();
        long lastAbsoluteTime = 0;

        std::cout << "Starting Kung Fu Chess Client Loop..." << std::endl;

        while (true) {
            auto now = clock::now();
            long absoluteTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
            long dt = absoluteTime - lastAbsoluteTime;
            lastAbsoluteTime = absoluteTime;

            // המנוע המקומי והשהיות הזמן הוסרו - השרת שולט בזמן!

            // מעבירים את מצב הבחירה לציור
            std::optional<Position> currentSelection = controller.getSelectedCell();
            
            // TODO: במקור המנוע החזיר סנאפשוט מעודכן עם הבחירה.
            // עכשיו נצטרך לעדכן את הבחירה באופן ויזואלי בסנאפשוט הנוכחי לפני הרינדור.
            GameSnapshot renderSnapshot = localSnapshot;
            // renderSnapshot.setSelected(currentSelection); // תלוי איך מימשת את זה במודל

            renderer.renderFrame(renderSnapshot, absoluteTime, dt);

            int key = cv::waitKey(1);
            if (key == 27) break; // יציאה עם ESC
        }
        
        network.disconnect();
    }
    catch (const std::exception &e) {
        std::cerr << "Fatal Error in main: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}