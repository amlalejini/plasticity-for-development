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
#include "Resource.h"
#include "Utilities.h"

class DOLWorld : public emp::World<DigitalOrganism> {
public:
  // public aliases
  using org_t = DigitalOrganism;
  using sgp_hardware_t = emp::EventDrivenGP_AW<DOLWorldConstants::TAG_WIDTH>;
  using sgp_program_t = typename sgp_hardware_t::Program;
  using sgp_memory_t = typename sgp_hardware_t::memory_t;
  using sgp_inst_t = typename sgp_hardware_t::inst_t;
  using sgp_event_t = typename sgp_hardware_t::event_t;
  using tag_t = typename sgp_hardware_t::affinity_t;
  using inst_lib_t = typename sgp_hardware_t::inst_lib_t;
  using event_lib_t = typename sgp_hardware_t::event_lib_t;
  using base_world_t = typename emp::World<DigitalOrganism>;

  using sgp_trait_ids_t = typename Deme::CellularHardware::SGPTraitIDs;

  using deme_seed_fun_t = std::function<void(Deme&, org_t&)>;
  using consume_resource_fun_t = std::function<void(size_t,size_t,size_t)>;
  using decay_resource_fun_t = std::function<void(size_t,size_t)>;

  using inst_attempt_cell_division_fun_t = std::function<void(size_t,size_t,const sgp_inst_t&)>;

  using sgp_event_handler_fun_t = std::function<void(sgp_hardware_t&, const sgp_event_t&)>;
  using sgp_event_dispatcher_fun_t = std::function<void(sgp_hardware_t&, const sgp_event_t&)>;

  /// Each deme has a local environment
  struct Environment {
    size_t env_id;                    ///< Corresponds to associated deme_id (& org id)
    emp::vector<Resource> resources;

    void Reset() {
      for (Resource & res : resources) {
        res.Reset();
      }
    }
  };

protected:

  // MAIN Configuration Settings
  int SEED;
  size_t UPDATES;
  size_t CPU_CYCLES_PER_UPDATE;
  size_t INIT_POP_SIZE;
  size_t MAX_POP_SIZE;
  std::string INIT_POP_MODE;
  std::string LOAD_ANCESTOR_INDIV_FPATH;
  // RESOURCES Configuration Settings
  std::string RESOURCE_CONSUMPTION_MODE;
  std::string RESOURCE_DECAY_MODE;
  std::string RESOURCE_TAGGING_MODE;
  size_t NUM_PERIODIC_RESOURCES;
  double PERIODIC_RESOURCES__LEVEL;
  double PERIODIC_RESOURCES__CONSUME_FIXED;
  double PERIODIC_RESOURCES__CONSUME_PROPORTIONAL;
  double PERIODIC_RESOURCES__FAILURE_COST;
  size_t PERIODIC_RESOURCES__MIN_UPDATES_UNAVAILABLE;
  size_t PERIODIC_RESOURCES__DECAY_DELAY;
  double PERIODIC_RESOURCES__DECAY_FIXED;
  double PERIODIC_RESOURCES__DECAY_PROPORTIONAL;
  size_t NUM_STATIC_RESOURCES;
  double STATIC_RESOURCES__LEVEL;
  double STATIC_RESOURCES__CONSUME_FIXED;
  double STATIC_RESOURCES__CONSUME_PROPORTIONAL;
  double STATIC_RESOURCES__FAILURE_COST;
  double PERIODIC_RESOURCES__PULSE_PROB;
  // DEME Configuration Settings
  size_t DEME_WIDTH;
  size_t DEME_HEIGHT;
  // CELLULAR HARDWARE Configuration Settings
  size_t SGP_MAX_THREAD_CNT;
  size_t SGP_MAX_CALL_DEPTH;
  double SGP_MIN_TAG_MATCH_THRESHOLD;
  bool CELL_SENSOR_LOCK_IN;
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
  // REPRODUCTION Configuration Settings
  double DEME_REPRODUCTION_COST;
  double TISSUE_ACCRETION_COST;

  // Non-configuration member variables
  bool setup = false;

  emp::Ptr<inst_lib_t> inst_lib;
  emp::Ptr<event_lib_t> event_lib;

  Mutator mutator;

  emp::vector<Environment> environments;  ///< Each organism (and deme) is has a local environment
  emp::vector<tag_t> resource_tags;
  emp::vector<ResourceType> resource_types;
  size_t TOTAL_RESOURCES;

  emp::vector<Deme> demes;
  emp::vector<size_t> birth_chamber; ///< IDs of organisms ready to reproduce!

  deme_seed_fun_t fun_seed_deme;

  consume_resource_fun_t fun_consume_resource;
  consume_resource_fun_t fun_consume_fail;
  decay_resource_fun_t fun_decay_resource;

  inst_attempt_cell_division_fun_t fun_instruction_attempted_cell_division; ///< What an instruction calls when it attempts to trigger cell division

  sgp_event_handler_fun_t fun_handle_msg;
  sgp_event_dispatcher_fun_t fun_dispatch_broadcast_msg;
  sgp_event_dispatcher_fun_t fun_dispatch_send_msg;

  // Internal functions
  void InitConfigs(DOLWorldConfig & config);
  void InitPop(DOLWorldConfig & config);
  void InitPop_Random(DOLWorldConfig & config);
  void InitPop_LoadIndividual(DOLWorldConfig & config);

  void SetupDemeHardware();
  void SetupInstructionSet();
  void SetupEventSet();
  void SetupEnvironment();

  /// Attempt to metabolize resource
  void AttemptToMetabolize(size_t org_id, size_t cell_id, size_t resource_id);

  /// Helper function to set cell sensor
  void SetCellSensor(size_t org_id, size_t cell_id, size_t resource_id, bool value);
  bool IsCellSensing(size_t org_id, size_t cell_id, size_t resource_id);

  void DonateCellResourcesToOrganism(size_t org_id, size_t cell_id) {
    org_t & org = GetOrg(org_id);
    Deme & deme = GetDeme(org_id);
    Deme::CellularHardware & cell = deme.GetCell(cell_id);
    double local_resources = cell.local_resources;
    emp_assert(local_resources >= 0, "SOMETHING WRONG! cell's local_resources < 0 during resource donation!", local_resources);
    org.GetPhenotype().resource_pool += local_resources;
    org.GetPhenotype().total_resources_donated += local_resources;
    cell.local_resources -= local_resources;
  }

  void PulsePeriodicResource(size_t env_id, size_t res_id) {
    Environment & local_env = environments[env_id];
    Resource & res = local_env.resources[res_id];
    // Replinish the pulsing resource!
    res.SetAmount(PERIODIC_RESOURCES__LEVEL);

    // We only need to alert the organism/deme if this position of the population
    // is occupied
    if (!IsOccupied(env_id)) return;

    // Localize the organism and the deme
    org_t & org = GetOrg(env_id);
    Deme & deme = GetDeme(env_id);

    // For any cells that are sensing, alert them!
    for (size_t cell_id = 0; cell_id < deme.GetCellCapacity(); ++cell_id) {
      // If cell isn't active, we don't need to worry about it
      if (!deme.IsCellActive(cell_id)) continue;
      // Cell is active, is it sensing for this resource?
      if (!deme.IsCellSensingResource(cell_id, res_id)) continue;
      // Cell is active & sensing for this resource! Alert!
      // std::cout << "Spawning core on cell!" << std::endl;
      deme.GetCell(cell_id).sgp_hw.SpawnCore(resource_tags[res_id], SGP_MIN_TAG_MATCH_THRESHOLD);
      // - cell_hw.sgp_hw.TriggerEvent(resource_alert, resource_tag)
      // Track that this organism received a signal for this resource!
      org.GetPhenotype().resource_alerts_received_by_type[res_id]++;
    }
  }

  /// Advance the all environment states
  void AdvanceEnvironment() {
    for (size_t env_id = 0; env_id < environments.size(); ++env_id) {
      // We only need to advance the environment for active organisms!
      if (!IsOccupied(env_id)) continue;
      Environment & local_env = environments[env_id];
      // Update resources
      for (size_t res_id = 0; res_id < local_env.resources.size(); ++res_id) {
        Resource & res = local_env.resources[res_id];
        if (res.GetType() == ResourceType::STATIC) { // Handle STATIC resource
          // Replinish resource levels
          res.SetAmount(STATIC_RESOURCES__LEVEL);
        } else if (res.GetType() == ResourceType::PERIODIC) { // Handle PERIODIC resource
          // If the resource is available, decay that resource.
          // else, if resource is unavailble, decide whether or not to pulse the resource
          if (res.IsAvailable()) {
            // If resource has been available for at least as long as the decay delay, decay the resource!
            if (res.GetTimeAvailable() >= PERIODIC_RESOURCES__DECAY_DELAY) {
              fun_decay_resource(env_id, res_id);
            }
          } else {
            // Pulse resource (maybe)!
            if (res.GetTimeUnavailable() >= PERIODIC_RESOURCES__MIN_UPDATES_UNAVAILABLE && random_ptr->P(PERIODIC_RESOURCES__PULSE_PROB)) {
              // res.SetAmount(PERIODIC_RESOURCES__LEVEL);
              PulsePeriodicResource(env_id, res_id);
            }
          }
        }
        // advance time on this resource!
        res.AdvanceAvailabilityTracking();
      }
    }
  }

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

  /// Get local environment @ position ID
  Environment & GetEnvironment(size_t id) { return environments[id]; }
  const Environment & GetEnvironment(size_t id) const { return environments[id]; }

  /// Just give 'em access to all da demes!
  emp::vector<Deme> & GetDemes() { return demes; }

  void Reset(DOLWorldConfig & config);
  void Setup(DOLWorldConfig & config);

  void RunStep();
  void Run();

  void PrintResourceTags(std::ostream & os = std::cout);

};

// =============================================================================
//                          DOLWorld member definitions
// =============================================================================

void DOLWorld::AttemptToMetabolize(size_t org_id, size_t cell_id, size_t resource_id) {
  emp_assert(org_id < GetSize());
  emp_assert(resource_id < TOTAL_RESOURCES);
  emp_assert(cell_id < DEME_WIDTH * DEME_HEIGHT);
  Deme & deme = GetDeme(org_id);
  Deme::CellularHardware & cell_hw = deme.GetCell(cell_id);
  Environment & local_env = GetEnvironment(org_id);
  Resource & res_state = local_env.resources[resource_id];
  // Only allow one attempt per update?
  if (cell_hw.metabolized_on_advance[resource_id]) return;
  // Attempt to consume!
  if (res_state.IsAvailable()) {
    // Attempt to consume resource
    fun_consume_resource(org_id, cell_id, resource_id);
  } else {
    // Apply cost of attempting to metabolize unavailable resource
    fun_consume_fail(org_id, cell_id, resource_id);
  }
  // Mark that we've attempted to consume!
  cell_hw.metabolized_on_advance[resource_id] = true;
}

void DOLWorld::SetCellSensor(size_t org_id, size_t cell_id, size_t resource_id, bool value) {
  Deme & deme = GetDeme(org_id);
  Deme::CellularHardware & cell_hw = deme.GetCell(cell_id);
  cell_hw.SetResourceSensor(resource_id, value);
}

bool DOLWorld::IsCellSensing(size_t org_id, size_t cell_id, size_t resource_id) {
  Deme & deme = GetDeme(org_id);
  Deme::CellularHardware & cell_hw = deme.GetCell(cell_id);
  return cell_hw.IsSensingResource(resource_id);
}

/// Localize configuration settings.
void DOLWorld::InitConfigs(DOLWorldConfig & config) {
  // MAIN Configuration Settings
  UPDATES = config.UPDATES();
  CPU_CYCLES_PER_UPDATE = config.CPU_CYCLES_PER_UPDATE();
  INIT_POP_SIZE = config.INIT_POP_SIZE();
  MAX_POP_SIZE = config.MAX_POP_SIZE();
  INIT_POP_MODE = config.INIT_POP_MODE();
  LOAD_ANCESTOR_INDIV_FPATH = config.LOAD_ANCESTOR_INDIV_FPATH();
  // RESOURCES Configuration Settings
  NUM_PERIODIC_RESOURCES = config.NUM_PERIODIC_RESOURCES();
  NUM_STATIC_RESOURCES = config.NUM_STATIC_RESOURCES();
  RESOURCE_CONSUMPTION_MODE = config.RESOURCE_CONSUMPTION_MODE();
  RESOURCE_DECAY_MODE = config.RESOURCE_DECAY_MODE();
  RESOURCE_TAGGING_MODE = config.RESOURCE_TAGGING_MODE();
  PERIODIC_RESOURCES__LEVEL = config.PERIODIC_RESOURCES__LEVEL();
  PERIODIC_RESOURCES__CONSUME_FIXED = config.PERIODIC_RESOURCES__CONSUME_FIXED();
  PERIODIC_RESOURCES__CONSUME_PROPORTIONAL = config.PERIODIC_RESOURCES__CONSUME_PROPORTIONAL();
  PERIODIC_RESOURCES__FAILURE_COST = config.PERIODIC_RESOURCES__FAILURE_COST();
  PERIODIC_RESOURCES__MIN_UPDATES_UNAVAILABLE = config.PERIODIC_RESOURCES__MIN_UPDATES_UNAVAILABLE();
  PERIODIC_RESOURCES__DECAY_DELAY = config.PERIODIC_RESOURCES__DECAY_DELAY();
  PERIODIC_RESOURCES__DECAY_FIXED = config.PERIODIC_RESOURCES__DECAY_FIXED();
  PERIODIC_RESOURCES__DECAY_PROPORTIONAL = config.PERIODIC_RESOURCES__DECAY_PROPORTIONAL();
  PERIODIC_RESOURCES__PULSE_PROB = config.PERIODIC_RESOURCES__PULSE_PROB();
  STATIC_RESOURCES__LEVEL = config.STATIC_RESOURCES__LEVEL();
  STATIC_RESOURCES__CONSUME_FIXED = config.STATIC_RESOURCES__CONSUME_FIXED();
  STATIC_RESOURCES__CONSUME_PROPORTIONAL = config.STATIC_RESOURCES__CONSUME_PROPORTIONAL();
  STATIC_RESOURCES__FAILURE_COST = config.STATIC_RESOURCES__FAILURE_COST();
  // DEME Configuration Settings
  DEME_WIDTH = config.DEME_WIDTH();
  DEME_HEIGHT = config.DEME_HEIGHT();
  // CELLULAR HARDWARE Configuration Settings
  SGP_MAX_THREAD_CNT = config.SGP_MAX_THREAD_CNT();
  SGP_MAX_CALL_DEPTH = config.SGP_MAX_CALL_DEPTH();
  SGP_MIN_TAG_MATCH_THRESHOLD = config.SGP_MIN_TAG_MATCH_THRESHOLD();
  CELL_SENSOR_LOCK_IN = config.CELL_SENSOR_LOCK_IN();
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
  // REPRODUCTION Configuration Settings
  DEME_REPRODUCTION_COST = config.DEME_REPRODUCTION_COST();
  TISSUE_ACCRETION_COST = config.TISSUE_ACCRETION_COST();
  // Various constants that depend on configuration parameters
  TOTAL_RESOURCES = NUM_PERIODIC_RESOURCES + NUM_STATIC_RESOURCES;
  // Verify some requirements
  emp_assert(MIN_FUNCTION_CNT > 0);
  emp_assert(MIN_FUNCTION_LEN > 0);
}

/// Initialize the population
void DOLWorld::InitPop(DOLWorldConfig & config) {
  // Make space!
  pop.resize(MAX_POP_SIZE);
  // How should we initialize the population?
  if (INIT_POP_MODE == "random") {
    InitPop_Random(config);
  } else if (INIT_POP_MODE == "load-single") {
    InitPop_LoadIndividual(config);
  } else {
    emp_assert(false, "Invalid INIT_POP_MODE (", INIT_POP_MODE, "). Exiting.");
    exit(-1);
  }
}

/// Initialize the population with random digital organisms
void DOLWorld::InitPop_Random(DOLWorldConfig & config) {
  // Now, there's space for MAX_POP_SIZE orgs, but no orgs have been injected
  // - Note, there's no population structure here (at the deme level), so
  //   we're just going to fill things up from beginning to end.
  emp_assert(INIT_POP_SIZE <= MAX_POP_SIZE, "INIT_POP_SIZE (", INIT_POP_SIZE, ") cannot exceed MAX_POP_SIZE (", MAX_POP_SIZE, ")!");
  for (size_t i = 0; i < INIT_POP_SIZE; ++i) {
    InjectAt(GenRandDigitalOrganismGenome(*random_ptr, *inst_lib, config), i);
    // std::cout << "Inst lib size: " << GetOrg(i).GetGenome().program.GetInstLib()->GetSize() << std::endl;
    // std::cout << "  expected size: " << inst_lib->GetSize() << std::endl;
  }
  // WARNING: All initial organisms in the population will have independent ancestry.
  //          - We could do a little extra work to tie their ancestry together (e.g.,
  //            have a dummy common ancestor).
}

/// Initialize the population with individual loaded from single-ancestor file
void DOLWorld::InitPop_LoadIndividual(DOLWorldConfig & config) {
  std::cout << "Initializing population from single-ancestor file!" << std::endl;

  // Configure the ancestor program.
  sgp_program_t ancestor_prog(inst_lib);
  tag_t birth_tag;
  std::ifstream ancestor_fstream(LOAD_ANCESTOR_INDIV_FPATH);
  if (!ancestor_fstream.is_open()) {
    std::cout << "Failed to open ancestor program file(" << LOAD_ANCESTOR_INDIV_FPATH << "). Exiting..." << std::endl;
    exit(-1);
  }

  // Get the birth(day) tag!
  std::string cur_line;
  while (!ancestor_fstream.eof()) {
    std::getline(ancestor_fstream, cur_line);
    emp::remove_whitespace(cur_line);
    if (cur_line == emp::empty_string()) continue;
    // Are we looking at the BIRTH tag?
    std::string birth_prefix = emp::to_lower(emp::string_get_range(cur_line, 0, 5));
    if (birth_prefix == "birth") {
      size_t tag_begin = cur_line.find_first_of('[');
      size_t tag_end = cur_line.find_first_of(']');
      if ((tag_begin != std::string::npos) && (tag_end != std::string::npos) && (tag_begin < tag_end)) {
        // Found tag.
        std::string tag_str = emp::string_get_range(cur_line, tag_begin+1, tag_end-(tag_begin+1));
        for (size_t i = 0; i < tag_str.size(); ++i) {
          if (i >= birth_tag.GetSize()) break;
          if (tag_str[i] == '1') birth_tag.Set(birth_tag.GetSize() - i - 1, true);
        }
      }
      break;
    } else {
      std::cout << "FAILED TO FIND BIRTH TAG! EXITING" << std::endl;
      exit(-1);
    }
  }
  // Here's the birth tag:
  std::cout << " --- Ancestor birth tag: ---" << std::endl;
  birth_tag.Print();
  std::cout << std::endl;

  // Load the ancestor program
  ancestor_prog.Load(ancestor_fstream);
  std::cout << " --- Ancestor program: ---" << std::endl;
  ancestor_prog.PrintProgramFull();
  std::cout << " -------------------------" << std::endl;

  DigitalOrganism::Genome ancestor_genome(ancestor_prog, birth_tag);
  emp_assert(ValidateDigitalOrganismGenome(config, ancestor_genome), "Loaded ancestor does not comply with configured requirements.");
  // todo - tie ancestry together!

  emp_assert(INIT_POP_SIZE <= MAX_POP_SIZE, "INIT_POP_SIZE (", INIT_POP_SIZE, ") cannot exceed MAX_POP_SIZE (", MAX_POP_SIZE, ")!");
  for (size_t i = 0; i < INIT_POP_SIZE; ++i) {
    InjectAt(ancestor_genome, i);
  }
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
    demes.back().SetupCellMetabolism(TOTAL_RESOURCES);
    // TODO - any non-constructor deme configuration
  }
}

/// Setup the signalgp event set
void DOLWorld::SetupEventSet() {

  // REMINDER:
  // - Event Handlers:
  //   - hw = hardware that is _handling_ (received) the event
  //   - event = event that is being handled on hw
  // - Event Dispatchers:
  //   - hw = hardware that is dispatching (emitting) the event)
  //   - event = event that is being dispatched
  // Generally, dispatchers will queue the given events on appropriate receiver
  // hardware (not necessarily the given hardware), and handlers will process the
  // given event on the given hardware.

  // todo - do we want handlers/dispatchers that do local=>local mem vs output=>input
  //        mem?
  fun_handle_msg = [this](sgp_hardware_t & hw, const sgp_event_t & event) {
    hw.SpawnCore(event.affinity, hw.GetMinBindThresh(), event.msg);
  };

  fun_dispatch_broadcast_msg = [this](sgp_hardware_t & hw, const sgp_event_t & event) {
    const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
    const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
    Deme & deme = this->GetDeme(world_id);
    emp_assert(deme.IsActive());
    // Dispatch event to all neighboring cells
    for (size_t d = 0; d < Deme::NUM_DIRECTIONS; ++d) {
      // Who is thy neighbor in direction[d]?
      const size_t neighbor_cell_id = deme.GetNeighboringCellID(cell_id, Deme::Dir[d]);
      // if neighboring cell is not active, do not message
      // if neighboring cell == this cell (small deme=>wrap around), do not message
      if ( (!deme.IsCellActive(neighbor_cell_id)) || cell_id == neighbor_cell_id) continue;
      // pass that message!
      Deme::CellularHardware & neighbor_cell = deme.GetCell(neighbor_cell_id);
      neighbor_cell.sgp_hw.QueueEvent(event);
    }
  };

  fun_dispatch_send_msg = [this](sgp_hardware_t & hw, const sgp_event_t & event) {
    const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
    const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
    Deme & deme = this->GetDeme(world_id);
    emp_assert(deme.IsActive());
    const size_t neighbor_cell_id = deme.GetNeighboringCellID(cell_id, deme.GetCellFacing(cell_id));
    // Is neighbor active?
    if (deme.IsCellActive(neighbor_cell_id) && cell_id != neighbor_cell_id) {
      Deme::CellularHardware & neighbor_cell = deme.GetCell(neighbor_cell_id);
      neighbor_cell.sgp_hw.QueueEvent(event);
    }
  };

  // Messaging events
  event_lib->AddEvent("SendMessageFacing", fun_handle_msg, "SendMessage event (cell (facing) ==={MESSAGE}===> cell)");
  event_lib->AddEvent("BroadcastMessage", fun_handle_msg, "Broadcast message event");

  // Register messaging dispatchers
  event_lib->RegisterDispatchFun("SendMessageFacing", fun_dispatch_send_msg);
  event_lib->RegisterDispatchFun("BroadcastMessage", fun_dispatch_broadcast_msg);
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

  // Messaging instructions
  inst_lib->AddInst("SendMsgFacing", [this](sgp_hardware_t & hw, const sgp_inst_t & inst) {
    sgp_hardware_t::State & state = hw.GetCurState();
    hw.TriggerEvent("SendMessageFacing", inst.affinity, state.output_mem);
  }, 0, "Send messaging to neighbor in direction that cell is facing");
  inst_lib->AddInst("BroadcastMsg", [this](sgp_hardware_t & hw, const sgp_inst_t & inst) {
    sgp_hardware_t::State & state = hw.GetCurState();
    hw.TriggerEvent("BroadcastMessage", inst.affinity, state.output_mem);
  }, 0, "Broadcast message to all neighbors");

  // Is faced cell empty?
  inst_lib->AddInst("IsFacingActive", [this](sgp_hardware_t & hw, const sgp_inst_t & inst) {
    // Localize world id and cell id
    const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
    const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
    sgp_hardware_t::State & state = hw.GetCurState();
    Deme & deme = this->GetDeme(world_id);
    state.SetLocal(inst.args[0], deme.IsCellActive(deme.GetNeighboringCellID(cell_id, deme.GetCellFacing(cell_id))));
  }, 1, "Is the neighboring cell faced by this cell empty (inactive)?");

  // Get/set facing
  inst_lib->AddInst("GetFacing", [this](sgp_hardware_t & hw, const sgp_inst_t & inst) {
    // Localize world id and cell id
    const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
    const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
    const size_t facing = (size_t)this->GetDeme(world_id).GetCellFacing(cell_id);
    sgp_hardware_t::State & state = hw.GetCurState();
    state.SetLocal(inst.args[0], facing);
  }, 1, "Get cell facing");
  inst_lib->AddInst("SetFacing", [this](sgp_hardware_t & hw, const sgp_inst_t & inst) {
    // Localize world id and cell id
    const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
    const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
    sgp_hardware_t::State & state = hw.GetCurState();
    const Deme::Facing facing = Deme::Dir[emp::Mod((int)state.GetLocal(inst.args[0]), (int)Deme::NUM_DIRECTIONS)];
    this->GetDeme(world_id).SetCellFacing(cell_id, facing);
  }, 1, "Set cell facing to local_mem[arg[0]] % NUM_DIRECTIONS");

  // Add simple rotation instructions
  inst_lib->AddInst("RotateCW", [this](sgp_hardware_t & hw, const sgp_inst_t & inst) {
    // Localize world id and cell id
    const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
    const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
    this->GetDeme(world_id).RotateCellCW(cell_id, 1);
  }, 0, "Rotate cell one step clockwise.");
  inst_lib->AddInst("RotateCCW", [this](sgp_hardware_t & hw, const sgp_inst_t & inst) {
    // Localize world id and cell id
    const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
    const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
    this->GetDeme(world_id).RotateCellCCW(cell_id, 1);
  }, 0, "Rotate cell one step counter clockwise.");
  inst_lib->AddInst("Rotate", [this](sgp_hardware_t & hw, const sgp_inst_t & inst) {
    // Localize world id and cell id
    const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
    const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
    sgp_hardware_t::State & state = hw.GetCurState();
    this->GetDeme(world_id).RotateCellCCW(cell_id, (int)state.GetLocal(inst.args[0]));
  }, 1, "Rotate cell local_mem[arg[0]]. If rotation is negative, rotate ccw. If rotation is 0, no rotation. If rotation is positive, rotate cw.");

  // Reproduction
  inst_lib->AddInst("CellDivide", [this](sgp_hardware_t & hw, const sgp_inst_t & inst) {
    // Localize world id and cell id
    const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
    const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
    fun_instruction_attempted_cell_division(world_id, cell_id, inst);
  }, 0, "Trigger cell division");

  // Once a soma-lineage has set their repro tag, that repro tag is locked in
  inst_lib->AddInst("SetDivisionTag", [this](sgp_hardware_t & hw, const sgp_inst_t & inst) {
    // Localize world id and cell id
    const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
    const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
    Deme & deme = GetDeme(world_id);
    Deme::CellularHardware & cell = deme.GetCell(cell_id);
    if (!cell.repro_tag_locked) { // If cell's repro tag isn't locked, lock it in w/instruction's tag
      deme.GetCell(cell_id).LockReproTag(inst.affinity);
    }
  });

  // Add resource donation instructions to instruction set
  inst_lib->AddInst("DonateResources", [this](sgp_hardware_t & hw, const sgp_inst_t & inst) {
    // Localize world id and cell id
    const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
    const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
    // Get organism, deme, and cell
    this->DonateCellResourcesToOrganism(world_id, cell_id);
  }, 0, "Donate cell's local resources to deme-level organism.");

  // Add resource-specific instructions to instruction set
  for (size_t resource_id = 0; resource_id < TOTAL_RESOURCES; ++resource_id) {
    // - Add metabolize instructions for each resource
    inst_lib->AddInst("Express-" + emp::to_string(resource_id),
      [this, resource_id](sgp_hardware_t & hw, const sgp_inst_t & inst) {
        // Need to get world ID and cell ID
        const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
        const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
        // Attempt to consume resource
        this->AttemptToMetabolize(world_id, cell_id, resource_id);
      }, 0, "Attempt to metabolize resource " + emp::to_string(resource_id));

    // - Add sensor activation instruction for each resource
    if (resource_types[resource_id] == ResourceType::PERIODIC) {

      inst_lib->AddInst("ActivateSensor-" + emp::to_string(resource_id),
        [this, resource_id](sgp_hardware_t & hw, const sgp_inst_t & inst) {
          // Need to get world ID and cell ID
          const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
          const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
          this->SetCellSensor(world_id, cell_id, resource_id, true);
        }, 0, "Activate sensor for resource " + emp::to_string(resource_id));

      // Are cells allowed to deactivate previously activated sensors?
      if (!CELL_SENSOR_LOCK_IN) {
        // - Add sensor deactivation instruction for each resource
        inst_lib->AddInst("DeactivateSensor-" + emp::to_string(resource_id),
          [this, resource_id](sgp_hardware_t & hw, const sgp_inst_t & inst) {
            // Need to get world ID and cell ID
            const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
            const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
            this->SetCellSensor(world_id, cell_id, resource_id, false);
          }, 0, "Deactivate sensor for resource " + emp::to_string(resource_id));

        // - Add sensor toggle instruction for each resource
        inst_lib->AddInst("ToggleSensor-" + emp::to_string(resource_id),
          [this, resource_id](sgp_hardware_t & hw, const sgp_inst_t & inst) {
            // Need to get world ID and cell ID
            const size_t world_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__DEME_ID);
            const size_t cell_id = (size_t)hw.GetTrait(sgp_trait_ids_t::TRAIT_ID__CELL_ID);
            const bool sensor_state = this->IsCellSensing(world_id, cell_id, resource_id);
            this->SetCellSensor(world_id, cell_id, resource_id, !sensor_state);
          }, 0, "Toggle sensor for resource " + emp::to_string(resource_id));
      }
    }
  }
}

/// Setup the environment (might add instructions to the instruction set!)
void DOLWorld::SetupEnvironment() {
  // Each position in the population has its own local environment
  environments.clear();
  environments.resize(MAX_POP_SIZE);
  for (size_t env_id = 0; env_id < environments.size(); ++env_id) {
    // std::cout << "Configuring env " << env_id << std::endl;
    Environment & env = environments[env_id]; // Configure environment @ env_id
    env.env_id = env_id;
    env.resources.resize(TOTAL_RESOURCES);
    size_t taskID = 0; // Each resource is associated with a 'task'
    // How many static resources?
    while (taskID < NUM_STATIC_RESOURCES) {
      // std::cout << "Configuring resource " << taskID << " (STATIC)" << std::endl;
      env.resources[taskID].SetID(taskID);
      env.resources[taskID].SetType(ResourceType::STATIC);
      env.resources[taskID].Reset();
      ++taskID;
    }
    // How many periodic resources?
    while (taskID < NUM_STATIC_RESOURCES + NUM_PERIODIC_RESOURCES) {
      // std::cout << "Configuring resource " << taskID << " (PERIODIC)" << std::endl;
      env.resources[taskID].SetID(taskID);
      env.resources[taskID].SetType(ResourceType::PERIODIC);
      env.resources[taskID].Reset();
      ++taskID;
    }
    emp_assert(env.resources.size() == TOTAL_RESOURCES);
  }
  std::cout << "Configured " << environments.size() << " environments, each with " << environments.back().resources.size() << " resources." << std::endl;

  // Configure resource tags!
  if (RESOURCE_TAGGING_MODE == "random") {
    /*emp::Random & rnd, size_t count, bool guarantee_unique*/
    resource_tags = GenRandTags<DOLWorldConstants::TAG_WIDTH>(*random_ptr, TOTAL_RESOURCES, true);
  } else if (RESOURCE_TAGGING_MODE == "hadamard") {
    emp_assert(DOLWorldConstants::TAG_WIDTH >= TOTAL_RESOURCES, "TAG_WIDTH (", DOLWorldConstants::TAG_WIDTH, ") must be >= TOTAL_RESOURCES (", TOTAL_RESOURCES, ") when RESOURCE_TAGGING_MODE=hadamard");
    resource_tags = GenHadamardMatrix<DOLWorldConstants::TAG_WIDTH>();
    resource_tags.resize(TOTAL_RESOURCES);
  } else {
    std::cout << "Unrecognized RESOURCE_TAGGING_MODE (" << RESOURCE_TAGGING_MODE << "). Exiting." << std::endl;
    exit(-1);
  }
  emp_assert(resource_tags.size() == TOTAL_RESOURCES);

  // Configure resource types
  resource_types.resize(TOTAL_RESOURCES);
  size_t taskID = 0; // Each resource is associated with a 'task'
  // How many static resources?
  while (taskID < NUM_STATIC_RESOURCES) {
    resource_types[taskID] = ResourceType::STATIC;
    ++taskID;
  }
  // How many periodic resources?
  while (taskID < NUM_STATIC_RESOURCES + NUM_PERIODIC_RESOURCES) {
    resource_types[taskID] = ResourceType::PERIODIC;
    ++taskID;
  }

  // Print resource tags!
  std::cout << "Resource tags: ";
  PrintResourceTags();
  std::cout << std::endl;

  // todo - output a resource tag file
}

void DOLWorld::Reset(DOLWorldConfig & config) {
  // --- Clear signals ---
  // OnOrgDeath
  on_death_sig.Clear();
  // OnPlacement
  on_placement_sig.Clear();
  // OnOffspringReady
  offspring_ready_sig.Clear();
  // --- Clear the world! ---
  emp::World<DigitalOrganism>::Reset(); // clear world, update = 0
  // --- Clean up dynamic memory ---
  inst_lib.Delete();
  event_lib.Delete();
  setup = false;
  // --- Setup the world again! ---
  Setup(config);
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

  // Setup the environment
  SetupEnvironment();
  // Setup event set
  SetupEventSet();
  // Setup instruction set
  SetupInstructionSet();
  // Setup deme hardware
  SetupDemeHardware();

  // Tell the emp::World how to be
  SetPopStruct_Mixed(false);  // Mixed population (at deme/organism-level), asynchronous generations
  SetAutoMutate(); // Mutations happen automatically when organisms are born (offspring_ready_sig)

  // Note, this is the function I would modify/parameterize if we wanted
  // to have single birth => multiple cells activated on placement
  // todo - move this functionality into deme?
  fun_seed_deme = [this](Deme & deme, org_t & org) {
    // (1) select a random cell in the deme
    // const size_t cell_id = GetRandom().GetUInt(deme.GetCellCapacity());
    const size_t cell_id = (size_t)deme.GetCellCapacity()/2;
    Deme::CellularHardware & cell_hw = deme.GetCell(cell_id);
    // (2) activate focal cell
    // - Program = genome's program, entry point tag = genome's birth tag, initial input memory = empty,
    //   entry point function main? = no, lock in entry point tag? = no
    cell_hw.ActivateCell(org.GetGenome().program, org.GetGenome().birth_tag, sgp_memory_t(), false, false);
    cell_hw.cell_facing = Deme::Facing::N;
  };

  // What happens when an organism consumes a resource?
  if (RESOURCE_CONSUMPTION_MODE == "fixed") {
    fun_consume_resource = [this](size_t org_id, size_t cell_id, size_t resource_id) {
      org_t & org = GetOrg(org_id);
      Deme & deme = GetDeme(org_id);
      Deme::CellularHardware & cell = deme.GetCell(cell_id);
      Environment & local_env = environments[org_id];
      Resource & res_state = local_env.resources[resource_id];
      double collected = res_state.ConsumeFixed((res_state.GetType() == ResourceType::STATIC) ? STATIC_RESOURCES__CONSUME_FIXED : PERIODIC_RESOURCES__CONSUME_FIXED);
      // Collect those sweet delicious resources!
      // org.GetPhenotype().resource_pool += collected;
      cell.local_resources += collected;
      org.GetPhenotype().total_resources_collected += collected;
      // Track consumption info
      emp_assert(resource_id < org.GetPhenotype().consumption_amount_by_type.size());
      emp_assert(resource_id < org.GetPhenotype().consumption_successes_by_type.size());
      org.GetPhenotype().consumption_amount_by_type[resource_id] += collected;
      org.GetPhenotype().consumption_successes_by_type[resource_id] += 1;
    };
  } else if (RESOURCE_CONSUMPTION_MODE == "proportional") {
    fun_consume_resource = [this](size_t org_id, size_t cell_id, size_t resource_id) {
      org_t & org = GetOrg(org_id);
      Deme & deme = GetDeme(org_id);
      Deme::CellularHardware & cell = deme.GetCell(cell_id);
      Environment & local_env = environments[org_id];
      Resource & res_state = local_env.resources[resource_id];
      double collected = res_state.ConsumeProportion((res_state.GetType() == ResourceType::STATIC) ? STATIC_RESOURCES__CONSUME_PROPORTIONAL : PERIODIC_RESOURCES__CONSUME_PROPORTIONAL);
      // Collect those sweet delicious resources!
      // org.GetPhenotype().resource_pool += collected;
      cell.local_resources += collected;
      org.GetPhenotype().total_resources_collected += collected;
      // Track consumption info
      emp_assert(resource_id < org.GetPhenotype().consumption_amount_by_type.size());
      emp_assert(resource_id < org.GetPhenotype().consumption_successes_by_type.size());
      org.GetPhenotype().consumption_amount_by_type[resource_id] += collected;
      org.GetPhenotype().consumption_successes_by_type[resource_id] += 1;
    };
  } else {
    std::cout << "Unrecognized RESOURCE_CONSUMPTION_MODE (" << RESOURCE_CONSUMPTION_MODE << ")! Exiting." << std::endl;
    exit(-1);
  }

  // What happens when a resource consumption fails?
  // (i.e., a misqueue? => attempted consumption when resource is unavailable)
  fun_consume_fail = [this](size_t org_id, size_t cell_id, size_t resource_id) {
    org_t & org = GetOrg(org_id);
    Environment & local_env = environments[org_id];
    Resource & res_state = local_env.resources[resource_id];
    switch (res_state.GetType()) {
      case ResourceType::STATIC: {
        org.GetPhenotype().resource_pool -= STATIC_RESOURCES__FAILURE_COST;
        break;
      }
      case ResourceType::PERIODIC: {
        org.GetPhenotype().resource_pool -= PERIODIC_RESOURCES__FAILURE_COST;
        break;
      }
      default: {
        emp_assert(false, "Trying to penalize organism for consuming unrecognized resource type!");
      }
    }
    // Don't let organism get into resource debt!
    if (org.GetPhenotype().resource_pool < 0) org.GetPhenotype().resource_pool = 0.0;
    // Track consumption misqueue
    org.GetPhenotype().consumption_failures_by_type[resource_id]++;
  };

  if (RESOURCE_DECAY_MODE == "fixed") {
    fun_decay_resource = [this](size_t env_id, size_t resource_id) {
      Environment & local_env = environments[env_id];
      Resource & res_state = local_env.resources[resource_id];
      res_state.DecayFixed(PERIODIC_RESOURCES__DECAY_FIXED);
    };
  } else if (RESOURCE_DECAY_MODE == "proportional") {
    fun_decay_resource = [this](size_t env_id, size_t resource_id) {
      Environment & local_env = environments[env_id];
      Resource & res_state = local_env.resources[resource_id];
      res_state.DecayProportion(PERIODIC_RESOURCES__DECAY_PROPORTIONAL);
    };
  } else {
    std::cout << "Unrecognized RESOURCE_DECAY_MODE (" << RESOURCE_DECAY_MODE << ")! Exiting." << std::endl;
    exit(-1);
  }

  // Setup instruction-triggered cellular division (within-deme reproduction)
  fun_instruction_attempted_cell_division = [this](size_t world_id,
                                                   size_t cell_id,
                                                   const sgp_inst_t & inst) {
    // Get deme
    Deme & deme = this->GetDeme(world_id);
    emp_assert(deme.IsCellActive(cell_id));
    emp_assert(IsOccupied(world_id));
    Deme::CellularHardware & cell = deme.GetCell(cell_id);
    // Does cell have the requisite resources to reproduce?
    if (cell.local_resources < TISSUE_ACCRETION_COST) {
      return; // If not, return
    }
    // Where is cell reproducing into?
    const size_t offspring_cell_id = deme.GetNeighboringCellID(cell_id, deme.GetCellFacing(cell_id));
    // Not allowed to reproduce over yourself?
    if (offspring_cell_id == cell_id) {
      // std::cout << "Organism ("<<cell_id<<") trying to birth over itself("<<offspring_cell_id<<")?" << std::endl;
      // std::cout << "Facing? " << deme.FacingStr(deme.GetCellFacing(cell_id)) << std::endl;
      // std::cout << "Neighbor? " << deme.GetNeighboringCellID(cell_id, deme.GetCellFacing(cell_id)) << std::endl;
      // std::cout << "Neighbor map: "<< std::endl;
      // deme.PrintNeighborMap();
      // std::cout << "================" << std::endl;
      // for (size_t id = 0; id < deme.GetCellCapacity(); ++id) {
        // size_t world_id = deme.GetCell(id).GetDemeID();
        // size_t cell_id = deme.GetCell(id).GetCellID();
        // std::cout << "  Deme-"<< deme.GetDemeID()<<" Cell " << id << ": world_id ="<< world_id <<"; cell_id ="<< cell_id <<";" << std::endl;
      // }
      // exit(0);
      return;
    }
    // If that location is already active, reset it (killing existing cell)
    if (deme.IsCellActive(offspring_cell_id)) {
      deme.GetCell(offspring_cell_id).Reset();
    }

    // Do the reproduction (active offspring cell)
    deme.GetCell(offspring_cell_id).ActivateCell(cell.sgp_hw.GetProgram(),      // What program should we initialize cell with?
                                                 cell.repro_tag,                // What tag should we use to trigger init function with?
                                                 sgp_memory_t(),                // What should input memory of init function call be?
                                                 false,                         // Should init function be a 'main'?
                                                 cell.repro_tag_locked);        // Should offspring's repro tag be locked?
    // mark cell as new born
    deme.GetCell(offspring_cell_id).new_born = true;
    // rotate cell to face parent
    const size_t child_dir = (size_t)emp::Mod((int)(cell.cell_facing + 4), (int)Deme::NUM_DIRECTIONS);
    deme.GetCell(offspring_cell_id).cell_facing = Deme::Dir[child_dir];
    // std::cout << "My direction: ";
    // pay costs of reproduction
    // WARNING - if cells allowed to reproduce over themselves, this will be a
    // bad reference (currently disallowed)
    cell.local_resources -= TISSUE_ACCRETION_COST;
  };

  // What to do when an organism dies?
  // - deactivate the deme hardware
  OnOrgDeath([this](size_t pos) {
    // Clean up deme hardware @ position
    emp_assert(pos < demes.size());
    demes[pos].DeactivateDeme();
  });

  // What happens when a new organism is placed?
  OnPlacement([this](size_t pos) {
    // Load organism into deme hardware
    emp_assert(pos < demes.size());
    emp_assert(pos < environments.size());
    Deme & focal_deme = demes[pos];
    org_t & placed_org = GetOrg(pos);
    placed_org.SetOrgID(pos);
    fun_seed_deme(focal_deme, placed_org);
    focal_deme.ActivateDeme();
    // Reset the local environment
    Environment & local_env = environments[pos];
    local_env.Reset();
  });

  // Todo
  OnOffspringReady([this](org_t & org, size_t parent_pos) {
    org.GetPhenotype().Reset(TOTAL_RESOURCES);
  });

  // Setup mutate function
  SetMutFun([this](org_t & org, emp::Random & r) {
    return mutator.Mutate(org, r);
  });

  // todo - setup systematics

  InitPop(config);

  // Reset phenotypes of initial population
  for (size_t i = 0; i < GetSize(); ++i) {
    if (!IsOccupied(i)) continue;
    GetOrg(i).GetPhenotype().Reset(TOTAL_RESOURCES);
  }

  setup = true;
  emp_assert(pop.size() == demes.size(), "SETUP ERROR! Population vector size (", pop.size(), ")", "does not match deme vector size (", demes.size(), ").");
  emp_assert(pop.size() == environments.size(), "SETUP ERROR! Population vector size (", pop.size(), ")", "does not match environments vector size (", environments.size(), ").");
}

void DOLWorld::RunStep() {
  std::cout << "Update: " << update << "; NumOrgs: " << GetNumOrgs() << std::endl;
  // Reminder, 1 update = CPU_CYCLES_PER_UPDATE distributed to every CPU thread across all demes
  // () Update the environment
  // std::cout << "ADVANCE ENVIRONMENT" << std::endl;
  AdvanceEnvironment();
  // () Evaluate all organisms (demes)
  // std::cout << "EXECUTION" << std::endl;
  for (size_t oid = 0; oid < pop.size(); ++oid) {
    if (!IsOccupied(oid)) continue;
    // Distribute CPU cycles to DEME
    Deme & deme = demes[oid];
    emp_assert(deme.IsActive());
    deme.Advance(CPU_CYCLES_PER_UPDATE);
    org_t & org = GetOrg(oid);
    // This organism lived through yet another trying update...
    org.GetPhenotype().age++;
    // Trigger reproduction if organism has sufficient resources in their resource pool post-update
    if (org.GetPhenotype().resource_pool >= DEME_REPRODUCTION_COST) {
      // Organism has sufficient resources
      org.GetPhenotype().resource_pool -= DEME_REPRODUCTION_COST;
      org.GetPhenotype().trigger_repro = true;
    }
    // if (org.GetPhenotype().age >= 20) {
    //   org.GetPhenotype().trigger_repro = true;
    //   org.GetPhenotype().age = 0;
    // }
    // Did organism trigger reproduction?
    if (org.GetPhenotype().trigger_repro) {
      birth_chamber.emplace_back(oid);
    }
  }
  // () Do organism-level (deme-level) reproduction
  emp::Shuffle(*random_ptr, birth_chamber); // Randomize birth chamber priority
  // std::cout << "REPRODUCTION?" << std::endl;
  for (size_t oid : birth_chamber) {
    emp_assert(IsOccupied(oid), "Reproducing organism no longer exists?");
    // We have to check if this organism is _still_ reproducing?
    // - In the unfortunate case where this potential parent was overwritten by
    //   a new baby organism, we don't want to replicate the new organism.
    org_t & org = GetOrg(oid);
    if (org.GetPhenotype().trigger_repro) {
      // Birth!
      org.GetPhenotype().trigger_repro = false;
      org.GetPhenotype().offspring_cnt++;
      DoBirth(org.GetGenome(), oid);
      // WARNING (to future me): org could be an invalid reference now!!!!
      // todo - OnBirth! and OnOffspringReady
    }
  }
  // Empty the birth chamber
  birth_chamber.clear();
  // birth_chamber.resize(0);
  // For each organism in the population, run its deme forward!
  Update(); // Update!
}

void DOLWorld::Run() {
  for (size_t u = 0 ; u <= UPDATES; ++u) {
    RunStep();
  }
  // Todo - end of run snapshotting/analyses!
  std::cout << "Done running!" << std::endl;
}

void DOLWorld::PrintResourceTags(std::ostream & os /*= std::cout*/) {
  os << "[";
  for (size_t res_id = 0; res_id < resource_tags.size(); ++res_id) {
    if (res_id) os << ",";
    resource_tags[res_id].Print(os);
  }
  os << "]";
}

#endif