/**
 *  @date 2019
 *
 *  @file  DOLWorld.h
 *
 *  An Empirical world class used to manage evolution experiments. Specifically,
 *  this world is designed to explore the evolution of division of labor
 *  (unicell => multicell => differentiated multicell).
 */

#ifndef _DOL_WORLD_H
#define _DOL_WORLD_H

#include <functional>
#include <iostream>

// Empirical includes
#include "base/Ptr.h"
#include "base/vector.h"
#include "config/ArgManager.h"
#include "Evolve/World.h"
#include "hardware/EventDrivenGP.h"
#include "hardware/signalgp_utils.h"
#include "tools/math.h"

// Local includes
#include "DOLWorldConfig.h"
#include "DigitalOrganism.h"
#include "Deme.h"
#include "Mutator.h"

class DOLWorld : public emp::World<DigitalOrganism> {
public:
  // public aliases
  using org_t = DigitalOrganism;
  using sgp_hardware_t = emp::EventDrivenGP_AW<DOLWorldConstants::TAG_WIDTH>;
  using sgp_program_t = typename sgp_hardware_t::Program;
  using sgp_memory_t = typename sgp_hardware_t::memory_t;
  using tag_t = typename sgp_hardware_t::affinity_t;
  using inst_lib_t = typename sgp_hardware_t::inst_lib_t;
  using event_lib_t = typename sgp_hardware_t::event_lib_t;

  using deme_seed_fun_t = std::function<void(Deme&, org_t&)>;

protected:

  // MAIN Configuration Settings
  int SEED;
  size_t UPDATES;
  size_t CPU_CYCLES_PER_UPDATE;
  size_t INIT_POP_SIZE;
  size_t MAX_POP_SIZE;
  std::string INIT_POP_MODE;
  // DEME Configuration Settings
  size_t DEME_WIDTH;
  size_t DEME_HEIGHT;
  // CELLULAR HARDWARE Configuration Settings
  size_t SGP_MAX_THREAD_CNT;
  size_t SGP_MAX_CALL_DEPTH;
  double SGP_MIN_TAG_MATCH_THRESHOLD;
  // PROGRAM Configuration Settings
  size_t MIN_FUNCTION_CNT;
  size_t MAX_FUNCTION_CNT;
  size_t MIN_FUNCTION_LEN;
  size_t MAX_FUNCTION_LEN;
  int MIN_ARGUMENT_VAL;
  int MAX_ARGUMENT_VAL;
  // MUTATION Configuration Settings
  double PROGRAM_ARG_SUB__PER_ARG;
  double PROGRAM_INST_SUB__PER_INST;
  double PROGRAM_INST_INS__PER_INST;
  double PROGRAM_INST_DEL__PER_INST;
  double PROGRAM_SLIP__PER_FUN;
  double PROGRAM_FUNC_DUP__PER_FUN;
  double PROGRAM_FUNC_DEL__PER_FUN;
  double PROGRAM_TAG_BIT_FLIP__PER_BIT;
  double BIRTH_TAG_BIT_FLIP__PER_BIT;

  // Non-configuration member variables
  bool setup = false;

  emp::Ptr<inst_lib_t> inst_lib;
  emp::Ptr<event_lib_t> event_lib;

  Mutator mutator;

  emp::vector<Deme> demes;
  emp::vector<size_t> birth_chamber; ///< IDs of organisms ready to reproduce!

  deme_seed_fun_t fun_seed_deme;

  // Internal functions
  void InitConfigs(DOLWorldConfig & config);
  void InitPop(DOLWorldConfig & config);
  void InitPop_Random(DOLWorldConfig & config);
  void InitPop_Load(DOLWorldConfig & config);

  void SetupDemeHardware();
  void SetupInstructionSet();

public:

  DOLWorld() {}
  DOLWorld(emp::Random & r) : emp::World<org_t>(r) {}

  ~DOLWorld() {
    if (setup) {
      inst_lib.Delete();
      event_lib.Delete();
      on_death_sig.Clear(); // Weird design pattern issue => on death triggers stuff in the derived class, but derived class is deleted by the time base class destructor is run!
    }
  }

  size_t GetCPUCyclesPerUpdate() const { return CPU_CYCLES_PER_UPDATE; }
  size_t GetDemeWidth() const { return DEME_WIDTH; };
  size_t GetDemeHeight() const { return DEME_HEIGHT; };
  size_t GetDemeCapacity() const { return DEME_WIDTH * DEME_HEIGHT; };

  /// Get deme @ position ID
  Deme & GetDeme(size_t id) { return demes[id]; }
  const Deme & GetDeme(size_t id) const { return demes[id]; }

  /// Just give 'em access to all da demes!
  emp::vector<Deme> & GetDemes() { return demes; }

  void Reset(DOLWorldConfig & config);
  void Setup(DOLWorldConfig & config);

  void RunStep();
  void Run();

};

// =============================================================================
//                          DOLWorld member definitions
// =============================================================================

/// Localize configuration settings.
void DOLWorld::InitConfigs(DOLWorldConfig & config) {
  // MAIN Configuration Settings
  UPDATES = config.UPDATES();
  CPU_CYCLES_PER_UPDATE = config.CPU_CYCLES_PER_UPDATE();
  INIT_POP_SIZE = config.INIT_POP_SIZE();
  MAX_POP_SIZE = config.MAX_POP_SIZE();
  INIT_POP_MODE = config.INIT_POP_MODE();
  // DEME Configuration Settings
  DEME_WIDTH = config.DEME_WIDTH();
  DEME_HEIGHT = config.DEME_HEIGHT();
  // CELLULAR HARDWARE Configuration Settings
  SGP_MAX_THREAD_CNT = config.SGP_MAX_THREAD_CNT();
  SGP_MAX_CALL_DEPTH = config.SGP_MAX_CALL_DEPTH();
  SGP_MIN_TAG_MATCH_THRESHOLD = config.SGP_MIN_TAG_MATCH_THRESHOLD();
  // PROGRAM Configuration Settings
  MIN_FUNCTION_CNT = config.MIN_FUNCTION_CNT();
  MAX_FUNCTION_CNT = config.MAX_FUNCTION_CNT();
  MIN_FUNCTION_LEN = config.MIN_FUNCTION_LEN();
  MAX_FUNCTION_LEN = config.MAX_FUNCTION_LEN();
  MIN_ARGUMENT_VAL = config.MIN_ARGUMENT_VAL();
  MAX_ARGUMENT_VAL = config.MAX_ARGUMENT_VAL();
  // MUTATION Configuration Settings
  PROGRAM_ARG_SUB__PER_ARG = config.PROGRAM_ARG_SUB__PER_ARG();
  PROGRAM_INST_SUB__PER_INST = config.PROGRAM_INST_SUB__PER_INST();
  PROGRAM_INST_INS__PER_INST = config.PROGRAM_INST_INS__PER_INST();
  PROGRAM_INST_DEL__PER_INST = config.PROGRAM_INST_DEL__PER_INST();
  PROGRAM_SLIP__PER_FUN = config.PROGRAM_SLIP__PER_FUN();
  PROGRAM_FUNC_DUP__PER_FUN = config.PROGRAM_FUNC_DUP__PER_FUN();
  PROGRAM_FUNC_DEL__PER_FUN = config.PROGRAM_FUNC_DEL__PER_FUN();
  PROGRAM_TAG_BIT_FLIP__PER_BIT = config.PROGRAM_TAG_BIT_FLIP__PER_BIT();
  BIRTH_TAG_BIT_FLIP__PER_BIT = config.BIRTH_TAG_BIT_FLIP__PER_BIT();

  // Verify some requirements
  emp_assert(MIN_FUNCTION_CNT > 0);
  emp_assert(MIN_FUNCTION_LEN > 0);
}

/// Initialize the population
void DOLWorld::InitPop(DOLWorldConfig & config) {
  // How should we initialize the population?
  if (INIT_POP_MODE == "random") {
    InitPop_Random(config);
  } else {
    emp_assert(false, "Invalid INIT_POP_MODE (", INIT_POP_MODE, "). Exiting.");
    exit(-1);
  }
}

/// Initialize the population with random digital organisms
void DOLWorld::InitPop_Random(DOLWorldConfig & config) {
  // Make space!
  pop.resize(MAX_POP_SIZE);
  // Now, there's space for MAX_POP_SIZE orgs, but no orgs have been injected
  // - Note, there's no population structure here (at the deme level), so
  //   we're just going to fill things up from beginning to end.
  emp_assert(INIT_POP_SIZE <= MAX_POP_SIZE, "INIT_POP_SIZE (", INIT_POP_SIZE, ") cannot exceed MAX_POP_SIZE (", MAX_POP_SIZE, ")!");
  for (size_t i = 0; i < INIT_POP_SIZE; ++i) {
    InjectAt(GenRandDigitalOrganismGenome(*random_ptr, *inst_lib, config), i);
  }
  // WARNING: All initial organisms in the population will have independent ancestry.
  //          - We could do a little extra work to tie their ancestry together (e.g.,
  //            have a dummy common ancestor).
}

/// Setup the Deme Hardware (only called by DOLWorld::Setup)
void DOLWorld::SetupDemeHardware() {
  std::cout << "DOLWorld - Setup - DemeHardware" << std::endl;
  demes.clear();

  // Add one deme hardware unit for every possible member of the population
  for (size_t i = 0; i < MAX_POP_SIZE; ++i) {
    /*Deme(size_t _width, size_t _height, emp::Ptr<emp::Random> _rnd,
           emp::Ptr<inst_lib_t> _inst_lib, emp::Ptr<event_lib_t> _event_lib)*/
    demes.emplace_back(DEME_WIDTH, DEME_HEIGHT, random_ptr, inst_lib, event_lib);
    demes.back().SetDemeID(i); // Associate deme with particular position in pop vector
    demes.back().SetCellHardwareMaxThreads(SGP_MAX_THREAD_CNT);
    demes.back().SetCellHardwareMaxCallDepth(SGP_MAX_CALL_DEPTH);
    demes.back().SetCellHardwareMinTagMatchThreshold(SGP_MIN_TAG_MATCH_THRESHOLD);
    demes.back().SetCellHardwareStochasticTieBreaks(false); // make tag-based referencing deterministic
    // TODO - any non-constructor deme configuration
  }
}

/// Setup the signalgp instruction set - todo (finish)!
void DOLWorld::SetupInstructionSet() {
  // Default instructions
  inst_lib->AddInst("Inc", sgp_hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  inst_lib->AddInst("Dec", sgp_hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
  inst_lib->AddInst("Not", sgp_hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
  inst_lib->AddInst("Add", sgp_hardware_t::Inst_Add, 3, "Local memory: Arg3 = Arg1 + Arg2");
  inst_lib->AddInst("Sub", sgp_hardware_t::Inst_Sub, 3, "Local memory: Arg3 = Arg1 - Arg2");
  inst_lib->AddInst("Mult", sgp_hardware_t::Inst_Mult, 3, "Local memory: Arg3 = Arg1 * Arg2");
  inst_lib->AddInst("Div", sgp_hardware_t::Inst_Div, 3, "Local memory: Arg3 = Arg1 / Arg2");
  inst_lib->AddInst("Mod", sgp_hardware_t::Inst_Mod, 3, "Local memory: Arg3 = Arg1 % Arg2");
  inst_lib->AddInst("TestEqu", sgp_hardware_t::Inst_TestEqu, 3, "Local memory: Arg3 = (Arg1 == Arg2)");
  inst_lib->AddInst("TestNEqu", sgp_hardware_t::Inst_TestNEqu, 3, "Local memory: Arg3 = (Arg1 != Arg2)");
  inst_lib->AddInst("TestLess", sgp_hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
  inst_lib->AddInst("If", sgp_hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib->AddInst("While", sgp_hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib->AddInst("Countdown", sgp_hardware_t::Inst_Countdown, 1, "Local memory: Countdown Arg1 to zero.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib->AddInst("Close", sgp_hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
  inst_lib->AddInst("Break", sgp_hardware_t::Inst_Break, 0, "Break out of current block.");
  inst_lib->AddInst("Call", sgp_hardware_t::Inst_Call, 0, "Call function that best matches call affinity.", emp::ScopeType::BASIC, 0, {"affinity"});
  inst_lib->AddInst("Return", sgp_hardware_t::Inst_Return, 0, "Return from current function if possible.");
  inst_lib->AddInst("SetMem", sgp_hardware_t::Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
  inst_lib->AddInst("CopyMem", sgp_hardware_t::Inst_CopyMem, 2, "Local memory: Arg1 = Arg2");
  inst_lib->AddInst("SwapMem", sgp_hardware_t::Inst_SwapMem, 2, "Local memory: Swap values of Arg1 and Arg2.");
  inst_lib->AddInst("Input", sgp_hardware_t::Inst_Input, 2, "Input memory Arg1 => Local memory Arg2.");
  inst_lib->AddInst("Output", sgp_hardware_t::Inst_Output, 2, "Local memory Arg1 => Output memory Arg2.");
  inst_lib->AddInst("Commit", sgp_hardware_t::Inst_Commit, 2, "Local memory Arg1 => Shared memory Arg2.");
  inst_lib->AddInst("Pull", sgp_hardware_t::Inst_Pull, 2, "Shared memory Arg1 => Shared memory Arg2.");
  inst_lib->AddInst("Nop", sgp_hardware_t::Inst_Nop, 0, "No operation.");
  // inst_lib->AddInst("Fork", Inst_Fork, 0, "Fork a new thread. Local memory contents of callee are loaded into forked thread's input memory.");
  inst_lib->AddInst("Terminate", sgp_hardware_t::Inst_Terminate, 0, "Kill current thread.");
  // Task-specific instructions
  // ...
}

/// Setup the experiment.
void DOLWorld::Setup(DOLWorldConfig & config) {
  std::cout << "DOLWorld - Setup" << std::endl;

  if (setup) {
    emp_assert(false, "Cannot Setup DOLWorld more than once!");
    exit(-1);
  }

  InitConfigs(config);
  mutator.Setup(config); // Configure the mutator

  inst_lib = emp::NewPtr<inst_lib_t>();
  event_lib = emp::NewPtr<event_lib_t>();

  // todo - setup setup signalgp instruction/event libraries
  SetupInstructionSet();

  // Setup deme hardware
  SetupDemeHardware();

  SetPopStruct_Mixed(false);  // Mixed population (at deme/organism-level), asynchronous generations
  SetAutoMutate(); // Mutations happen automatically when organisms are born (offspring_ready_sig)

  // Note, this is the function I would modify/parameterize if we wanted
  // to have single birth => multiple cells activated on placement
  // todo - move this functionality into deme?
  fun_seed_deme = [this](Deme & deme, org_t & org) {
    // (1) select a random cell in the deme
    const size_t cell_id = GetRandom().GetUInt(deme.GetCellCapacity());
    Deme::CellularHardware & cell_hw = deme.GetCell(cell_id);
    // (2) activate focal cell
    cell_hw.ActivateCell(org.GetGenome().program, org.GetGenome().birth_tag, sgp_memory_t(), true);
  };

  // What to do when an organism dies?
  // - deactivate the deme hardware
  OnOrgDeath([this](size_t pos) {
    // Clean up deme hardware @ position
    demes[pos].DeactivateDeme();
  });

  // What happens when a new organism is placed?
  OnPlacement([this](size_t pos) {
    // Load organism into deme hardware
    Deme & focal_deme = demes[pos];
    org_t & placed_org = GetOrg(pos);
    placed_org.SetOrgID(pos);
    fun_seed_deme(focal_deme, placed_org);
    focal_deme.ActivateDeme();
  });

  // Setup mutate function
  SetMutFun([this](org_t & org, emp::Random & r) {
    return mutator.Mutate(org, r);
  });

  // todo - setup environment
  // todo - setup systematics

  // todo - finish function

  InitPop(config);

  // todo - configure world appropriately (e.g., asynchronous, mixed)
  setup = true;
  emp_assert(pop.size() == demes.size(), "SETUP ERROR! Population vector size (", pop.size(), ")", "does not match deme vector size (", demes.size(), ").");
}

void DOLWorld::RunStep() {
  std::cout << "Update: " << update << "; NumOrgs: " << GetNumOrgs() << std::endl;
  // Reminder, 1 update = CPU_CYCLES_PER_UPDATE distributed to every CPU thread across all demes
  // (1) Evaluate all organisms (demes)
  std::cout << "EXECUTION" << std::endl;
  for (size_t oid = 0; oid < pop.size(); ++oid) {
    if (!IsOccupied(oid)) continue;
    // Distribute CPU cycles to DEME
    Deme & deme = demes[oid];
    deme.Advance(CPU_CYCLES_PER_UPDATE);
    // Did organism trigger reproduction?
    org_t & org = GetOrg(oid);
    // Age organism
    org.GetPhenotype().age++;
    if (org.GetPhenotype().trigger_repro) {
      birth_chamber.emplace_back(oid);
    }
  }
  // (2) Do reproduction
  emp::Shuffle(*random_ptr, birth_chamber); // Randomize birth chamber priority
  std::cout << "REPRODUCTION?" << std::endl;
  for (size_t oid : birth_chamber) {
    emp_assert(IsOccupied(oid), "Reproducing organism no longer exists?");
    // We have to check if this organism is _still_ reproducing?
    // - In the unfortunate case where this potential parent was overwritten by
    //   a new baby organism, we don't want to replicate the new organism.
    org_t & org = GetOrg(oid);
    if (org.GetPhenotype().trigger_repro) {
      // Birth!
      std::cout << "Org " << oid << " giving birth!" << std::endl;
      DoBirth(org.GetGenome(), oid);
      org.GetPhenotype().trigger_repro = false;
      // todo - OnBirth! and OnOffspringReady
    }
  }
  // Empty the birth chamber
  birth_chamber.clear();
  // For each organism in the population, run its deme forward!
  Update(); // Update!
}

void DOLWorld::Run() {
  for (size_t u = 0 ; u <= UPDATES; ++u) {
    RunStep();
  }
  // Todo - end of run snapshotting/analyses!
}

#endif