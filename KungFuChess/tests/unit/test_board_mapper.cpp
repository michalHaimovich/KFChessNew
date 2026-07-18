#include "doctest.h"
#include "input/board_mapper.hpp"

TEST_CASE("BoardMapper - Pixel to Cell Translation") {
    BoardMapper mapper(200, 100, 50);

    SUBCASE("Valid coordinates map to correct cells") {
        auto pos1 = mapper.pixelToCell(200, 100);
        REQUIRE(pos1.has_value());
        CHECK(pos1->row == 0);
        CHECK(pos1->col == 0);

        auto pos2 = mapper.pixelToCell(275, 125); 
        REQUIRE(pos2.has_value());
        CHECK(pos2->row == 0);
        CHECK(pos2->col == 1);

        auto pos3 = mapper.pixelToCell(350, 250);
        REQUIRE(pos3.has_value());
        CHECK(pos3->row == 3);
        CHECK(pos3->col == 3);
    }

    SUBCASE("Negative mapping relative to start coordinates returns nullopt") {
        CHECK(mapper.pixelToCell(199, 100).has_value() == false);
        CHECK(mapper.pixelToCell(200, 99).has_value() == false);
        CHECK(mapper.pixelToCell(0, 0).has_value() == false);
    }
}