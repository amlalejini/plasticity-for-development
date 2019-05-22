#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "TestStruct.h"
#include "DOLWorld.h"

TEST_CASE( "First test!", "[first]" ) {
    REQUIRE(true);
    TestStruct s;
    s.a = 5;
    s.b = 10;
    REQUIRE(s.Add() == 15);
}

TEST_CASE( "DOLWorld Setup", "[world][setup]") {
    // Create a configuration object
    DOLWorldConfig config;
    config.SEED(1);
    config.UPDATES(50);
    config.CPU_CYCLES_PER_UPDATE(3);
    config.INIT_POP_SIZE(10);
    config.MAX_POP_SIZE(200);
    config.DEME_WIDTH(2);
    config.DEME_HEIGHT(2);

    emp::Random rnd(config.SEED());
    DOLWorld world(rnd);
    world.Setup(config);

    REQUIRE(world.GetDemeWidth() == 2);
    REQUIRE(world.GetDemeHeight() == 2);
    REQUIRE(world.GetDemeCapacity() == 4);
    REQUIRE(world.GetCPUCyclesPerUpdate() == 3);

}
