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

TEST_CASE( "DOLWorld Setup", "[world][setup]" ) {
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

TEST_CASE ( "DOLWorld Deme - Topology", "[world][deme]" ) {
    using facing_t = DOLWorld::Deme::Facing;

    DOLWorld::Deme deme1x1(1, 1);
    REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::N) == 0);
    REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::NE) == 0);
    REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::E) == 0);
    REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::SE) == 0);
    REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::S) == 0);
    REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::SW) == 0);
    REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::W) == 0);
    REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::NW) == 0);

    DOLWorld::Deme deme2x2(2, 2);
    // Check cell 0's neighbors
    REQUIRE(deme2x2.GetNeighboringCellID(0, facing_t::N) == 2);
    REQUIRE(deme2x2.GetNeighboringCellID(0, facing_t::NE) == 3);
    REQUIRE(deme2x2.GetNeighboringCellID(0, facing_t::E) == 1);
    REQUIRE(deme2x2.GetNeighboringCellID(0, facing_t::SE) == 3);
    REQUIRE(deme2x2.GetNeighboringCellID(0, facing_t::S) == 2);
    REQUIRE(deme2x2.GetNeighboringCellID(0, facing_t::SW) == 3);
    REQUIRE(deme2x2.GetNeighboringCellID(0, facing_t::W) == 1);
    REQUIRE(deme2x2.GetNeighboringCellID(0, facing_t::NW) == 3);
    // Check cell 1's neighbors
    REQUIRE(deme2x2.GetNeighboringCellID(3, facing_t::N) == 1);
    REQUIRE(deme2x2.GetNeighboringCellID(3, facing_t::NE) == 0);
    REQUIRE(deme2x2.GetNeighboringCellID(3, facing_t::E) == 2);
    REQUIRE(deme2x2.GetNeighboringCellID(3, facing_t::SE) == 0);
    REQUIRE(deme2x2.GetNeighboringCellID(3, facing_t::S) == 1);
    REQUIRE(deme2x2.GetNeighboringCellID(3, facing_t::SW) == 0);
    REQUIRE(deme2x2.GetNeighboringCellID(3, facing_t::W) == 2);
    REQUIRE(deme2x2.GetNeighboringCellID(3, facing_t::NW) == 0);

    DOLWorld::Deme deme4x4(4, 4);
    // Pretty print the neighbor map
    deme4x4.PrintNeighborMap();
    REQUIRE(deme4x4.GetNeighboringCellID(5, facing_t::N) == 9);
    REQUIRE(deme4x4.GetNeighboringCellID(5, facing_t::NE) == 10);
    REQUIRE(deme4x4.GetNeighboringCellID(5, facing_t::E) == 6);
    REQUIRE(deme4x4.GetNeighboringCellID(5, facing_t::SE) == 2);
    REQUIRE(deme4x4.GetNeighboringCellID(5, facing_t::S) == 1);
    REQUIRE(deme4x4.GetNeighboringCellID(5, facing_t::SW) == 0);
    REQUIRE(deme4x4.GetNeighboringCellID(5, facing_t::W) == 4);
    REQUIRE(deme4x4.GetNeighboringCellID(5, facing_t::NW) == 8);

    // REQUIRE(deme4x4.GetCellCapacity() == 4*4);
    REQUIRE(deme4x4.GetCellX(5) == 1);
    REQUIRE(deme4x4.GetCellY(5) == 1);
    REQUIRE(deme4x4.GetCellID(1,1) == 5);
}
