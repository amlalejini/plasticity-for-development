#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "Deme.h"
#include "DOLWorld.h"
#include "DOLWorldConfig.h"
#include "DigitalOrganism.h"
#include "Mutator.h"
#include "Utilities.h"
#include "Resource.h"

// Tests
// - [ ] Test that phenotypes are property reset on birth/placement!
// - [ ] INIT_POP_MODE == "load-single"
// - [ ] Resources as hadamard
// - [ ] World reset

TEST_CASE( "DOLWorld Setup - Configuration Initialization", "[world][setup]" ) {
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
  REQUIRE(world.GetSize() == 200);
  world.RunStep();

  config.LOAD_ANCESTOR_INDIV_FPATH("tests/test-configs/single-static-task.gp");
  config.INIT_POP_MODE("load-single");
  config.MAX_POP_SIZE(500);
  config.DEME_WIDTH(16);
  config.DEME_HEIGHT(16);

  world.Reset(config);
  REQUIRE(world.GetSize() == 500);
  REQUIRE(world.GetDemeWidth() == 16);
  REQUIRE(world.GetDemeHeight() == 16);
  world.RunStep();
}

TEST_CASE ( "Deme - Topology", "[deme]" ) {
  using facing_t = Deme::Facing;

  Deme deme1x1(1, 1, nullptr, nullptr, nullptr);
  deme1x1.PrintNeighborMap();
  REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::N) == 0);
  REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::NE) == 0);
  REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::E) == 0);
  REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::SE) == 0);
  REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::S) == 0);
  REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::SW) == 0);
  REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::W) == 0);
  REQUIRE(deme1x1.GetNeighboringCellID(0, facing_t::NW) == 0);

  Deme deme2x2(2, 2, nullptr, nullptr, nullptr);
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

  Deme deme4x4(4, 4, nullptr, nullptr, nullptr);
  // Pretty print the neighbor map
  // deme4x4.PrintNeighborMap();
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

TEST_CASE ("Deme - Rotation", "[deme]") {
  using facing_t = Deme::Facing;

  Deme deme3x3(3, 3, nullptr, nullptr, nullptr);
  // Check facing
  const size_t mid_id = 4;
  // deme3x3.PrintNeighborMap();
  REQUIRE(deme3x3.GetNeighboringCellID(mid_id, facing_t::N) == 7);
  REQUIRE(deme3x3.GetNeighboringCellID(mid_id, facing_t::NE) == 8);
  REQUIRE(deme3x3.GetNeighboringCellID(mid_id, facing_t::E) == 5);
  REQUIRE(deme3x3.GetNeighboringCellID(mid_id, facing_t::SE) == 2);
  REQUIRE(deme3x3.GetNeighboringCellID(mid_id, facing_t::S) == 1);
  REQUIRE(deme3x3.GetNeighboringCellID(mid_id, facing_t::SW) == 0);
  REQUIRE(deme3x3.GetNeighboringCellID(mid_id, facing_t::W) == 3);
  REQUIRE(deme3x3.GetNeighboringCellID(mid_id, facing_t::NW) == 6);
  // Okay, check facing of mid id
  deme3x3.SetCellFacing(mid_id, facing_t::N);
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::N);
  REQUIRE(deme3x3.GetCellFacing(mid_id) == deme3x3.GetCell(mid_id).cell_facing);
  // Do some rotations!
  deme3x3.RotateCellCW(mid_id); // NE
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::NE);
  deme3x3.RotateCellCW(mid_id); // E
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::E);
  deme3x3.RotateCellCW(mid_id); // SE
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::SE);
  deme3x3.RotateCellCW(mid_id); // S
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::S);
  deme3x3.RotateCellCW(mid_id); // SW
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::SW);
  deme3x3.RotateCellCW(mid_id); // W
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::W);
  deme3x3.RotateCellCW(mid_id); // NW
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::NW);
  deme3x3.RotateCellCW(mid_id); // N
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::N);
  // Test multi-step rotations
  deme3x3.SetCellFacing(mid_id, facing_t::N);
  deme3x3.RotateCellCW(mid_id, 3); // SE
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::SE);
  deme3x3.RotateCellCW(mid_id, 4); // NW
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::NW);
  deme3x3.RotateCellCW(mid_id, 2); // NE
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::NE);
  deme3x3.RotateCellCCW(mid_id);   // N
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::N);
  // Test multi-step, multi-cycle rotations
  deme3x3.SetCellFacing(mid_id, facing_t::N);
  deme3x3.RotateCellCW(mid_id, 16); // N
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::N);
  deme3x3.RotateCellCW(mid_id, 32); // N
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::N);
  deme3x3.RotateCellCCW(mid_id, 16); // N
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::N);
  deme3x3.RotateCellCCW(mid_id, 32); // N
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::N);
  deme3x3.RotateCellCW(mid_id, 17); // N
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::NE);
  deme3x3.RotateCellCCW(mid_id, 7); // N
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::E);

  deme3x3.SetCellFacing(mid_id, facing_t::N);
  deme3x3.RotateCellCW(mid_id, -2); // N
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::W);
  deme3x3.RotateCellCW(mid_id, -17); // N
  REQUIRE(deme3x3.GetCellFacing(mid_id) == facing_t::SW);

}

TEST_CASE ("Deme - Reset", "[deme]") {
  Deme deme3x3(3, 3, nullptr, nullptr, nullptr);
  // Set a bunch of cell hardware stuff, reset. Test if each cell was reset.
  for (size_t i = 0; i < 3*3; ++i) {
    Deme::CellularHardware & cell = deme3x3.GetCell(i);
    REQUIRE(cell.cell_id == i);
    cell.active = true;
    cell.repro_tag.SetAll();
    cell.repro_tag_locked = true;
    cell.new_born = true;
    cell.Reset();
    REQUIRE(!cell.active);
    REQUIRE(cell.repro_tag.None());
    REQUIRE(!cell.repro_tag_locked);
    REQUIRE(!cell.new_born);
  }

  for (size_t i = 0; i < 3*3; ++i) {
    Deme::CellularHardware & cell = deme3x3.GetCell(i);
    REQUIRE(cell.cell_id == i);
    cell.active = true;
    cell.repro_tag.SetAll();
    cell.repro_tag_locked = true;
    cell.new_born = true;
  }
  deme3x3.DeactivateDeme();
  for (size_t i = 0; i < 3*3; ++i) {
    Deme::CellularHardware & cell = deme3x3.GetCell(i);
    REQUIRE(cell.cell_id == i);
    REQUIRE(!cell.active);
    REQUIRE(cell.repro_tag.None());
    REQUIRE(!cell.repro_tag_locked);
    REQUIRE(!cell.new_born);
  }
}

TEST_CASE ("Deme - CellularHardware", "[deme][cell_hardware]") {
  Deme deme3x3(3, 3, nullptr, nullptr, nullptr);
  // Test set resource sensor function
  deme3x3.SetupCellMetabolism(9);
  for (size_t i = 0; i < 3*3; ++i) {
    Deme::CellularHardware & cell = deme3x3.GetCell(i);
    cell.SetResourceSensor(i, true);
  }
  for (size_t i = 0; i < 3*3; ++i) {
    Deme::CellularHardware & cell = deme3x3.GetCell(i);
    for (size_t j = 0; j < 3*3; ++j) {
      if (j == i) {
        REQUIRE(cell.resource_sensors[j]);
      } else {
        REQUIRE(!cell.resource_sensors[j]);
      }
    }
  }
}

TEST_CASE ( "DOLWorld Setup - Random Population Initialization", "[world][setup][population]" ) {
  // Create a configuration object
  DOLWorldConfig config;
  config.SEED(1);
  config.INIT_POP_SIZE(10);
  config.MAX_POP_SIZE(200);
  config.INIT_POP_MODE("random");
  config.MIN_FUNCTION_CNT(1);
  config.MAX_FUNCTION_CNT(64);
  config.MIN_FUNCTION_LEN(1);
  config.MAX_FUNCTION_LEN(256);
  config.MIN_ARGUMENT_VAL(0);
  config.MAX_ARGUMENT_VAL(15);

  // Create a new DOLWorld
  emp::Random rnd(config.SEED());
  DOLWorld world(rnd);
  world.Setup(config);

  // Check that population size meets expectations!
  REQUIRE(world.GetSize() == 200);
  REQUIRE(world.GetFullPop().size() == 200);
  REQUIRE(world.GetNumOrgs() == 10);

  // Verify genomes in the population.
  for (size_t i = 0; i < world.GetSize(); ++i) {
    if (!world.IsOccupied(i)) continue;
    REQUIRE(ValidateDigitalOrganismGenome(config, world.GetGenomeAt(i)));
  }

  config.INIT_POP_SIZE(200);
  DOLWorld world2(rnd);
  world2.Setup(config);

  // Check that population size meets expectations!
  REQUIRE(world2.GetSize() == 200);
  REQUIRE(world2.GetFullPop().size() == 200);
  REQUIRE(world2.GetNumOrgs() == 200);

  // Verify genomes in the population.
  for (size_t i = 0; i < world2.GetSize(); ++i) {
    if (!world2.IsOccupied(i)) continue;
    REQUIRE(ValidateDigitalOrganismGenome(config, world2.GetGenomeAt(i)));
  }
}

TEST_CASE ( "DOLWorld Setup - Deme Hardware Setup", "[world][setup][deme]" ) {
  using deme_t = Deme;
  using cell_hw_t = Deme::CellularHardware;
  // Create a configuration object
  DOLWorldConfig config;
  config.SEED(1);
  config.INIT_POP_SIZE(10);
  config.MAX_POP_SIZE(200);
  config.INIT_POP_MODE("random");
  config.SGP_MAX_THREAD_CNT(2);
  config.SGP_MAX_CALL_DEPTH(512);
  config.SGP_MIN_TAG_MATCH_THRESHOLD(0.5);

  // Create a new DOLWorld
  emp::Random rnd(config.SEED());
  DOLWorld world(rnd);
  world.Setup(config);

  // Check deme configuration
  REQUIRE(world.GetDemes().size() == config.MAX_POP_SIZE());
  size_t active_cell_cnt = 0;
  for (size_t i = 0; i < config.MAX_POP_SIZE(); ++i) {
    deme_t & deme = world.GetDeme(i);
    REQUIRE(deme.GetDemeID() == i); // Deme IDs should match up with population IDs
    // Check cellular hardware configuration
    for (size_t k = 0; k < config.DEME_HEIGHT() * config.DEME_WIDTH(); ++k) {
      cell_hw_t & cell = deme.GetCell(k);
      REQUIRE(cell.sgp_hw.GetMaxCores() == config.SGP_MAX_THREAD_CNT());
      REQUIRE(cell.sgp_hw.GetMaxCallDepth() == config.SGP_MAX_CALL_DEPTH());
      REQUIRE(cell.sgp_hw.GetMinBindThresh() == config.SGP_MIN_TAG_MATCH_THRESHOLD());
      REQUIRE(cell.sgp_hw.IsStochasticFunCall() == false);
      REQUIRE(cell.cell_id == k);
      if (cell.active) {
        ++active_cell_cnt;
        REQUIRE(cell.sgp_hw.GetProgram().GetSize() > 0);
        REQUIRE(cell.sgp_hw.GetProgram().GetInstCnt() > 0);
      }
    }
  }
  REQUIRE(active_cell_cnt == config.INIT_POP_SIZE());
}

TEST_CASE ( "DOLWorld Run - Default Settings", "[world][run]" ) {
  // Create a configuration object
  DOLWorldConfig config;
  config.SEED(1);
  // Create a new DOLWorld
  emp::Random rnd(config.SEED());
  DOLWorld world(rnd);
  world.Setup(config);
  // Run world under default configuration options
  world.Run();
  REQUIRE(world.GetUpdate() == config.UPDATES()+1);
}

TEST_CASE ( "Resource", "[resource]") {
  Resource resource;

  resource.SetID(0);
  resource.SetType(ResourceType::PERIODIC);
  REQUIRE(resource.GetID() == 0);
  REQUIRE(resource.GetType() == ResourceType::PERIODIC);

  // set to 0
  resource.SetAmount(0);
  REQUIRE(resource.GetAmount() == 0.0);
  REQUIRE(resource.IsAvailable() == false);
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);

  resource.IncAmount(100);
  REQUIRE(resource.GetAmount() == 100.0);
  REQUIRE(resource.IsAvailable());
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);

  resource.SetAmount(101);
  REQUIRE(resource.GetAmount() == 101.0);
  REQUIRE(resource.IsAvailable());
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);
  resource.AdvanceAvailabilityTracking();
  REQUIRE(resource.GetTimeAvailable() == 1);
  REQUIRE(resource.GetTimeUnavailable() == 0);

  resource.SetAmount(0.5*Resource::MIN_RESOURCE_AMOUNT);
  REQUIRE(resource.GetAmount() == 0.0);
  REQUIRE(resource.IsAvailable() == false);
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);
  resource.AdvanceAvailabilityTracking();
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 1);

  resource.IncAmount(100);
  REQUIRE(resource.GetAmount() == 100.0);
  REQUIRE(resource.IsAvailable());
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);

  // Consume
  double consumed;
  // - Consume unavailable
  resource.SetAmount(0);
  consumed = resource.ConsumeFixed(10);
  REQUIRE(consumed == 0.0);
  REQUIRE(resource.GetAmount() == 0.0);
  REQUIRE(resource.IsAvailable() == false);
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);

  // - Consume available => enough available
  resource.SetAmount(100);
  consumed = resource.ConsumeFixed(10);
  REQUIRE(consumed == 10.0);
  REQUIRE(resource.GetAmount() == 90.0);
  REQUIRE(resource.IsAvailable() == true);
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);

  // - Consume available => not enough available
  consumed = resource.ConsumeFixed(100.0);
  REQUIRE(consumed == 90.0);
  REQUIRE(resource.GetAmount() == 0.0);
  REQUIRE(resource.IsAvailable() == false);
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);

  resource.SetAmount(100);
  consumed = resource.ConsumeProportion(0.5);
  REQUIRE(consumed == 50.0);
  REQUIRE(resource.GetAmount() == 50.0);
  REQUIRE(resource.IsAvailable() == true);
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);

  // Decay
  // - decay unavailable
  resource.SetAmount(0.0);
  resource.DecayFixed(100);
  REQUIRE(resource.GetAmount() == 0.0);
  REQUIRE(resource.IsAvailable() == false);
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);

  resource.DecayProportion(0.5);
  REQUIRE(resource.GetAmount() == 0.0);
  REQUIRE(resource.IsAvailable() == false);
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);

  // - decay available => not enough available
  resource.SetAmount(100);
  resource.DecayFixed(150);
  REQUIRE(resource.GetAmount() == 0.0);
  REQUIRE(resource.IsAvailable() == false);
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);

  // - decay available => enough availble
  resource.SetAmount(100);
  resource.DecayProportion(0.5);
  REQUIRE(resource.GetAmount() == 50.0);
  REQUIRE(resource.IsAvailable() == true);
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);

  resource.DecayFixed(20);
  REQUIRE(resource.GetAmount() == 30);
  REQUIRE(resource.IsAvailable() == true);
  REQUIRE(resource.GetTimeAvailable() == 0);
  REQUIRE(resource.GetTimeUnavailable() == 0);
}

TEST_CASE ( "Mutator", "[mutator]") {
  using genome_t = typename DigitalOrganism::Genome;
  using sgp_hardware_t = typename DOLWorld::sgp_hardware_t;
  using inst_lib_t = typename DOLWorld::inst_lib_t;

  emp::Random rnd(10);
  inst_lib_t inst_lib;
  DOLWorldConfig config;
  Mutator mutator;

  inst_lib.AddInst("Nop-A", sgp_hardware_t::Inst_Nop, 0, "No operation.");
  inst_lib.AddInst("Nop-B", sgp_hardware_t::Inst_Nop, 0, "No operation.");
  inst_lib.AddInst("Nop-C", sgp_hardware_t::Inst_Nop, 0, "No operation.");

  // Set program constraints
  config.MIN_FUNCTION_CNT(1);
  config.MAX_FUNCTION_CNT(64);
  config.MIN_FUNCTION_LEN(1);
  config.MAX_FUNCTION_LEN(128);
  config.MIN_ARGUMENT_VAL(0);
  config.MAX_ARGUMENT_VAL(32);
  // Crank up the mutation rates
  config.PROGRAM_ARG_SUB__PER_ARG(0.1);
  config.PROGRAM_INST_SUB__PER_INST(0.1);
  config.PROGRAM_INST_INS__PER_INST(0.1);
  config.PROGRAM_INST_DEL__PER_INST(0.1);
  config.PROGRAM_SLIP__PER_FUN(0.1);
  config.PROGRAM_FUNC_DUP__PER_FUN(0.1);
  config.PROGRAM_FUNC_DEL__PER_FUN(0.1);
  config.PROGRAM_TAG_BIT_FLIP__PER_BIT(0.1);
  config.BIRTH_TAG_BIT_FLIP__PER_BIT(0.1);

  mutator.Setup(config); // Configure the mutator

  // Test lots of mutated random genomes
  for (size_t i = 0; i < 1000; ++i) {
    // (1) Generate a random genome
    genome_t genome = GenRandDigitalOrganismGenome(rnd, inst_lib, config);
    REQUIRE(ValidateDigitalOrganismGenome(config, genome));
    // (2) Mutate genome a bunch, validating each time
    for (size_t m = 0; m < 10; ++m) {
      mutator.Mutate(genome, rnd);
      REQUIRE(ValidateDigitalOrganismGenome(config, genome));
    }
  }
}

TEST_CASE ("Utilities - GenRandTag", "[utilities]") {
  constexpr size_t TWIDTH = 4;
  using tag_t = emp::BitSet<TWIDTH>;

  emp::Random rnd(10);

  // Generate a few random tags - make sure function doesn't fail
  for (size_t i = 0; i < 10; ++i) {
    tag_t tag = GenRandTag<TWIDTH>(rnd);
  }

  // Generate all 16 possible tags
  emp::vector<tag_t> tags;
  for (size_t i = 0; i < 16; ++i) {
    tags.emplace_back(GenRandTag<TWIDTH>(rnd, tags));
  }
}

TEST_CASE ("Utilities - GenRandTags", "[utilities]") {
  constexpr size_t TWIDTH = 4;
  using tag_t = emp::BitSet<TWIDTH>;

  emp::Random rnd(10);

  // Generate a few random tags - make sure function doesn't fail
  emp::vector<tag_t> tags;
  for (size_t i = 0; i < 10; ++i) {
    tags = GenRandTags<TWIDTH>(rnd, 4, true);
    tags = GenRandTags<TWIDTH>(rnd, 4, false);
  }

  for (size_t i = 0; i < 100; ++i) {
    emp::vector<tag_t> tags_1 = GenRandTags<TWIDTH>(rnd, 8, true);
    emp::vector<tag_t> tags_2 = GenRandTags<TWIDTH>(rnd, 4, true, tags_1);
    // make sure nothing in tags_1 == tags_2
    for (size_t t1 = 0; t1 < tags_1.size(); ++t1) {
      for (size_t t2 = 0; t2 < tags_2.size(); ++t2) {
        REQUIRE(tags_1[t1] != tags_2[t2]);
      }
    }
  }
}

TEST_CASE ("Utilities - GenHadamardMatrix") {
  constexpr size_t TWIDTH = 4;
  using tag_t = emp::BitSet<TWIDTH>;
  emp::Random rnd(10);
  // 4 bit tags
  emp::vector<tag_t> tags = GenHadamardMatrix<TWIDTH>();
  // std::cout << "Tags: " << std::endl;
  for (size_t i = 0; i < tags.size(); ++i) {
    for (size_t j = i+1; j < tags.size(); ++j) {
      // std::cout << "tag["<<i<<"] vs tag["<<j<<"] = "<< emp::SimpleMatchCoeff(tags[i], tags[j]) << std::endl;
      REQUIRE(emp::SimpleMatchCoeff(tags[i], tags[j]) == 0.5);
      REQUIRE(HammingDist(tags[i], tags[j]) == 2);
    }
  }
  // 16 bit tags
  using tag16_t = emp::BitSet<16>;
  emp::vector<tag16_t> tags16 = GenHadamardMatrix<16>();
  // std::cout << "Tags: " << std::endl;
  for (size_t i = 0; i < tags16.size(); ++i) {
    for (size_t j = i+1; j < tags16.size(); ++j) {
      // std::cout << "tag["<<i<<"] vs tag["<<j<<"] = "<< emp::SimpleMatchCoeff(tags[i], tags[j]) << std::endl;
      REQUIRE(emp::SimpleMatchCoeff(tags16[i], tags16[j]) == 0.5);
      REQUIRE(HammingDist(tags16[i], tags16[j]) == 8);
    }
  }
}

TEST_CASE ("Utilities - to_titlecase") {
  std::string s1 = "HELLO WORLD";
  std::string s2 = "HeLlO wOrLd";
  std::string s3 = "hello world";
  std::string s4 = "Hello World";

  REQUIRE(to_titlecase(s1) == "Hello World");
  REQUIRE(to_titlecase(s2) == "Hello World");
  REQUIRE(to_titlecase(s3) == "Hello World");
  REQUIRE(to_titlecase(s4) == "Hello World");
}

TEST_CASE ("Utilities - join") {
  std::string s1 = ",";
  emp::vector<std::string> vec = {"hello","world","again"};
  REQUIRE(join(vec, s1) == "hello,world,again");
}