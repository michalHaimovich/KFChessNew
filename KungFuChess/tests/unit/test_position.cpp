#include "doctest.h"
#include "model/position.hpp"
#include <sstream>

TEST_CASE("Position - Value Object Semantics") {
    
    SUBCASE("Two positions with the same row and column are equal") {
        Position p1{3, 4};
        Position p2{3, 4};
        
        // Assert equality operator
        CHECK(p1 == p2);
    }

    SUBCASE("Two positions with different row or column are not equal") {
        Position p1{3, 4};
        Position p2{4, 3}; // Flipped coordinates
        Position p3{3, 5}; // Different column
        
        // Assert inequality operator
        CHECK(p1 != p2);
        CHECK(p1 != p3);
    }

    SUBCASE("Position objects produce readable assertion failures") {
        Position p1{5, 7};
        std::ostringstream oss;
        
        // Feed the position object into the output stream
        oss << p1;
        
        // Verify the readable string format
        CHECK(oss.str() == "Position(row: 5, col: 7)");
    }
}