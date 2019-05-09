#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "TestStruct.h"

TEST_CASE( "First test!", "[first]" ) {
    REQUIRE(true);
    TestStruct s;
    s.a = 5;
    s.b = 10;
    REQUIRE(s.Add() == 15);
}
