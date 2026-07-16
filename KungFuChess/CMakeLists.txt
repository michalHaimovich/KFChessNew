cmake_minimum_required(VERSION 3.15)
project(KungFuChess CXX)


# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



# --- הגדרת OpenCV מתוך התיקייה המקומית בפרויקט ---
# אנחנו אומרים ל-CMake איפה נמצאים קבצי ה-Header (.hpp)
include_directories(src/OpenCV/include)

# אנחנו אומרים ל-CMake איפה נמצאים קבצי ה-Binary (.dll או .lib)
# בהנחה שהקבצים נמצאים ב-src/OpenCV/bin
link_directories(src/OpenCV/bin)

# בהמשך, כשנקשר את הספריות, נצטרך לציין את שמות הקבצים הספציפיים
# (למשל: opencv_world460.lib - תלוי מה יש לך בתיקיית ה-bin)

# Include directories
include_directories(src)
include_directories(tests) # doctest.h is located here
include_directories(${OpenCV_INCLUDE_DIRS}) # חושף את הנתיבים של OpenCV לכל הפרויקט

# ==========================================
# 1. Core game library (KungFuChess Core)
# ==========================================
add_library(kungfu_chess_core
    # --- Model & Rules ---
    src/model/position.cpp
    src/model/board.cpp
    src/rules/piece_rules.cpp
    src/rules/rule_engine.cpp
    
    # --- Engine & Real-Time ---
    src/realtime/real_time_arbiter.cpp
    src/engine/game_engine.cpp
    src/input/controller.cpp
    
    # --- IO & Testing ---
    src/io/board_parser.cpp
    src/io/board_printer.cpp
    src/texttests/script_parser.cpp
    src/texttests/script_runner.cpp

    # --- View (Graphics Layer) ---
    src/view/asset_locator.cpp
    src/view/board_layout.cpp
    src/view/img.cpp
    src/view/animation_data.cpp
    src/view/animation_player.cpp
    src/view/asset_manager.cpp
    src/view/piece_view.cpp
    src/view/renderer.cpp
)

# מקשר את הליבה שלנו לספריות של OpenCV כדי שנוכל להשתמש בפונקציות הגרפיות
target_link_libraries(kungfu_chess_core PUBLIC ${OpenCV_LIBS})

# ==========================================
# 2. Game executable
# ==========================================
add_executable(kungfu_chess_app src/main.cpp)
target_link_libraries(kungfu_chess_app PRIVATE kungfu_chess_core)

# ==========================================
# 3. Test executable
# ==========================================
add_executable(kungfu_chess_tests
    tests/test_main.cpp
    tests/unit/test_board.cpp
    tests/unit/test_position.cpp
    tests/unit/test_piece_rules.cpp
    tests/unit/test_rule_engine.cpp
    tests/unit/test_real_time_arbiter.cpp
    tests/unit/test_game_engine.cpp
    tests/unit/test_board_mapper.cpp
    tests/unit/test_controller.cpp
    tests/unit/test_board_parser.cpp
    tests/unit/test_board_printer.cpp
    tests/integration/test_text_scripts.cpp
)

# Link the core library
target_link_libraries(kungfu_chess_tests PRIVATE kungfu_chess_core)

target_link_libraries(kungfu_chess_core PUBLIC 
    opencv_world451d  # שנו את זה לשם הקובץ שיש לכם בתיקיית ה-bin (בלי סיומת .lib)
)
# ==========================================
# Optional: coverage settings (for VSCode coverage reporting)
# ==========================================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(kungfu_chess_core PRIVATE --coverage)
    target_link_options(kungfu_chess_core PRIVATE --coverage)
    target_compile_options(kungfu_chess_tests PRIVATE --coverage)
    target_link_options(kungfu_chess_tests PRIVATE --coverage)
endif()