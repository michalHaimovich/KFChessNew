cmake_minimum_required(VERSION 3.15)
project(KungFuChess CXX)
add_definitions(-DASIO_STANDALONE -D_WEBSOCKETPP_CPP11_STL_ -DASIO_ENABLE_OLD_SERVICES)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================
# 0. Third-Party Libraries (WebSockets & ASIO & JSON)
# ==========================================
include_directories(${CMAKE_SOURCE_DIR}/third_party/asio/include)

include_directories(${CMAKE_SOURCE_DIR}/third_party)

# OpenCV include and library directories (Moved to Client)
include_directories(src/client/OpenCV/include)
link_directories(src/client/OpenCV/bin)

include_directories(src/shared)
include_directories(src/client)
include_directories(src/server)
include_directories(tests)

# ==========================================
# 1. Shared Library (Model & IO)
# ==========================================
add_library(kungfu_shared
    src/shared/model/position.cpp
    src/shared/model/board.cpp
    src/shared/io/board_parser.cpp
    src/shared/io/board_printer.cpp
)

# ==========================================
# 2. Server Core & Executable (המוח)
# ==========================================
add_library(kungfu_server_core
    src/server/rules/piece_rules.cpp
    src/server/rules/rule_engine.cpp
    src/server/realtime/real_time_arbiter.cpp
    src/server/engine/game_engine.cpp
   # src/server/texttests/script_parser.cpp
   # src/server/texttests/script_runner.cpp
    src/server/network/network_server.cpp 
    src/server/game/game_session.cpp
    src/server/game/server_controller.cpp
)
target_link_libraries(kungfu_server_core PRIVATE kungfu_shared)

# רק ה-main נשאר פה, השאר עברו ל-core
add_executable(kungfu_server_app src/server/server_main.cpp)
target_link_libraries(kungfu_server_app PRIVATE kungfu_server_core)

if(WIN32)
    target_link_libraries(kungfu_server_app PRIVATE ws2_32)
endif()

# ==========================================
# 3. Client Core & Executable (התצוגה)
# ==========================================
add_library(kungfu_client_core
    src/client/input/controller.cpp
    src/client/view/asset_locator.cpp
    src/client/view/board_layout.cpp
    src/client/view/img.cpp
    src/client/view/animation_data.cpp
    src/client/view/animation_player.cpp
    src/client/view/asset_manager.cpp
    src/client/view/piece_view.cpp
    src/client/view/renderer.cpp
    src/client/view/move_history_manager.cpp
    src/client/view/score_manager.cpp
    src/client/network/network_client.cpp
)

target_link_libraries(kungfu_client_core PUBLIC opencv_world451d)
target_link_libraries(kungfu_client_core PRIVATE kungfu_shared)

# רק ה-main נשאר פה, השאר עברו ל-core
add_executable(kungfu_client_app src/client/client_main.cpp)
target_link_libraries(kungfu_client_app PRIVATE kungfu_client_core)

if(WIN32)
    target_link_libraries(kungfu_client_app PRIVATE ws2_32)
endif()

# ==========================================
# 4. Test executable
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

target_link_libraries(kungfu_chess_tests PRIVATE kungfu_client_core kungfu_server_core kungfu_shared)

# ==========================================
# Optional: coverage settings
# ==========================================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(kungfu_client_core PRIVATE --coverage)
    target_link_options(kungfu_client_core PRIVATE --coverage)
    target_compile_options(kungfu_server_core PRIVATE --coverage)
    target_link_options(kungfu_server_core PRIVATE --coverage)
    target_compile_options(kungfu_chess_tests PRIVATE --coverage)
    target_link_options(kungfu_chess_tests PRIVATE --coverage)
endif()