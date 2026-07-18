#include "doctest.h"
#include "input/controller.hpp"
#include "engine/game_engine.hpp"

TEST_CASE("Controller - Click and Selection Logic") {
    GameEngine engine(8, 8);
    engine.setupStandardBoard();
    
    BoardMapper mapper(0, 0, 10);
    Controller controller(engine, mapper);

    SUBCASE("Clicking out of pixel bounds is ignored") {
        CHECK(controller.click(-5, -5) == ControllerResult::Ignored);
        CHECK(controller.getSelectedCell().has_value() == false);
    }

    SUBCASE("First click on empty cell is ignored") {
        CHECK(controller.click(45, 45) == ControllerResult::Ignored);
        CHECK(controller.getSelectedCell().has_value() == false);
    }

    SUBCASE("First click on a friendly piece selects it") {
        CHECK(controller.click(5, 65) == ControllerResult::PieceSelected);
        REQUIRE(controller.getSelectedCell().has_value());
        CHECK(controller.getSelectedCell()->row == 6);
        CHECK(controller.getSelectedCell()->col == 0);
    }

    SUBCASE("Second click switches selection to another friendly piece") {
        controller.click(5, 65); 
        
        CHECK(controller.click(15, 65) == ControllerResult::PieceSelected);
        REQUIRE(controller.getSelectedCell().has_value());
        CHECK(controller.getSelectedCell()->row == 6);
        CHECK(controller.getSelectedCell()->col == 1);
    }

    SUBCASE("Second click outside logical board clears selection") {
        controller.click(5, 65); 
        
        CHECK(controller.click(105, 105) == ControllerResult::SelectionCleared);
        CHECK(controller.getSelectedCell().has_value() == false);
    }

    SUBCASE("Second click on destination requests move and clears selection") {
        controller.click(5, 65); 
        
        CHECK(controller.click(5, 55) == ControllerResult::MoveRequested);
        CHECK(controller.getSelectedCell().has_value() == false); 
    }
}

TEST_CASE("Controller - Jump Logic") {
    GameEngine engine(8, 8);
    engine.setupStandardBoard();
    BoardMapper mapper(0, 0, 10);
    Controller controller(engine, mapper);

    SUBCASE("Jump out of pixel bounds is ignored") {
        CHECK(controller.jump(-5, -5) == ControllerResult::Ignored);
    }

    SUBCASE("Jump on a valid piece requests a jump") {
        CHECK(controller.jump(5, 65) == ControllerResult::JumpRequested);
    }

    SUBCASE("Jump on an empty cell is ignored") {
        CHECK(controller.jump(45, 45) == ControllerResult::Ignored);
    }
}