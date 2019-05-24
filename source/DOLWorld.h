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
#include "tools/math.h"

// Local includes
#include "DigitalOrganism.h"
#include "DOLWorldConfig.h"

class DOLWorld : public emp::World<DigitalOrganism<16>> {
public:
  // static constants
  static constexpr size_t TAG_WIDTH = 16;
  // Forward declarations
  struct CellularHardware;
  class Deme;
  // public aliases
  using org_t = DigitalOrganism<TAG_WIDTH>;
  using sgp_hardware_t = emp::EventDrivenGP_AW<TAG_WIDTH>;
  using sgp_program_t = typename sgp_hardware_t::Program;
  using sgp_memory_t = typename sgp_hardware_t::memory_t;
  using tag_t = typename sgp_hardware_t::affinity_t;
  using inst_lib_t = typename sgp_hardware_t::inst_lib_t;
  using event_lib_t = typename sgp_hardware_t::event_lib_t;

  using deme_seed_fun_t = std::function<void(Deme&, org_t&)>;

  /// Hardware unit that each cell in a deme 'runs' on
  /// Note, CellularHardware can't extend SGP hardware because SGP programs
  /// contain instruction libraries templated off of SGP hardware =/= inst_lib<CellularHardware>
  struct CellularHardware {

    size_t cell_id = 0;
    sgp_hardware_t sgp_hw;
    bool active = false;

    CellularHardware(emp::Ptr<emp::Random> _rnd, emp::Ptr<inst_lib_t> _inst_lib,
                     emp::Ptr<event_lib_t> _event_lib)
      : sgp_hw(_inst_lib, _event_lib, _rnd) { sgp_hw.ResetHardware(); }

    // - sensors: emp::vector<size_t> sensors;
    // - sensor refractory state: emp::vector<size_t> refractory_states;

    /// On reset:
    /// - reset signalgp hardware & program
    /// - todo - clear out traits (non-permanent ones)
    void Reset() { sgp_hw.ResetProgram(); active = false; }

    void ActivateCell(const sgp_program_t & program,
                      const tag_t & init_tag,
                      const sgp_memory_t & init_mem,
                      bool init_main) {
      sgp_hw.SetProgram(program);
      sgp_hw.SpawnCore(init_tag, sgp_hw.GetMinBindThresh(), init_mem, init_main);
      active = true;
    }
  };

  /// A 'deme' of CellularHardware.
  class Deme {
  public:
    enum Facing { N=0, NE=1, E=2, SE=3, S=4, SW=5, W=6, NW=7 };                   ///< All possible directions
    static constexpr Facing Dir[] {Facing::N, Facing::NE, Facing::E, Facing::SE,  ///< Array of possible directions
                                  Facing::S, Facing::SW, Facing::W, Facing::NW};
    static constexpr size_t NUM_DIRECTIONS = 8;                                   ///< Number of neighbors each board space has.

  private:
    size_t deme_id = 0;                  ///< ID associated with deme. Corresponds to position in world population.
    bool deme_active = false;            ///< Is this deme actively running?
    size_t width;                        ///< Width of grid
    size_t height;                       ///< Height of grid
    emp::vector<size_t> neighbor_lookup; ///< Lookup table for neighbors
    emp::vector<CellularHardware> cells; ///< Toroidal grid of CellularHardware units

    /// Build neighbor lookup (according to current width and height)
    void BuildNeighborLookup();

    /// Calculate the neighbor ID of the cell (specified by id) in the specified direction.
    size_t CalcNeighbor(size_t id, Facing dir) const;

  public:
    Deme(size_t _width, size_t _height, emp::Ptr<emp::Random> _rnd,
        emp::Ptr<inst_lib_t> _inst_lib, emp::Ptr<event_lib_t> _event_lib)
      : width(_width), height(_height)
    {
      for (size_t i = 0; i < width*height; ++i) {
        cells.emplace_back(_rnd, _inst_lib, _event_lib);
        cells.back().cell_id = i; // Cell id corresponds to position in cells vector
      }
      BuildNeighborLookup();
    }

    /// Get this deme's id
    size_t GetDemeID() const { return deme_id; }

    /// Is deme active?
    bool IsActive() const { return deme_active; }

    /// Get cell capacity of deme
    size_t GetCellCapacity() const { return cells.size(); }

    /// Given cell ID, return x coordinate
    size_t GetCellX(size_t id) const { return id % width; }

    /// Given cell ID, return y coordinate
    size_t GetCellY(size_t id) const { return id / width; }

    /// Given x,y coordinate, return cell ID
    size_t GetCellID(size_t x, size_t y) const { return (y * width) + x; }

    /// Get cell at position ID (outsource bounds checking to emp::vector)
    CellularHardware & GetCell(size_t id) { return cells[id]; }

    /// Get const cell at position ID (outsource bounds checking to emp::vector)
    const CellularHardware & GetCell(size_t id) const { return cells[id]; }

    /// Given a cell ID and facing (of that cell), return the appropriate neighboring cell ID
    size_t GetNeighboringCellID(size_t id, Facing dir) const { return neighbor_lookup[id*NUM_DIRECTIONS + (size_t)dir]; }

    /// Set this deme's ID
    void SetDemeID(size_t id) { deme_id = id; }

    /// Set SignalGP hardware (on cellular hardware) maximum thread count
    void SetCellHardwareMaxThreads(size_t val);

    /// Set SignalGP hardware (on cellular hardware) maximum call depth
    void SetCellHardwareMaxCallDepth(size_t val);

    /// Set SignalGP hardware (on cellular hardware) minimum tag matching threshold
    void SetCellHardwareMinTagMatchThreshold(double val);

    /// Set SignalGP hardware (on cellular hardware) tie break procedure
    void SetCellHardwareStochasticTieBreaks(bool val);

    /// Activate deme - todo - maybe more required to activate deme (e.g., loading a digital organism)
    /// - used to flag that deme is currently active (running)
    void ActivateDeme() {
      deme_active = true;
    }

    /// Deactivate deme - todo - maybe more needs to happen on deativate?
    void DeactivateDeme() {
      for (CellularHardware & cell : cells) {
        cell.Reset(); // Reset cell
      }
      deme_active = false;
    }

    /// Return a string representation of the given facing direction (useful for debugging)
    std::string FacingStr(Facing dir) const;

    /// Pretty print the neighbor map (useful for debugging)
    void PrintNeighborMap(std::ostream & os = std::cout) const;
  };

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

  // Non-configuration member variables
  bool setup = false;

  emp::Ptr<inst_lib_t> inst_lib;
  emp::Ptr<event_lib_t> event_lib;

  emp::vector<Deme> demes;

  deme_seed_fun_t fun_seed_deme;

  // Internal functions
  void InitConfigs(DOLWorldConfig & config);
  void InitPop(DOLWorldConfig & config);
  void InitPop_Random(DOLWorldConfig & config);
  void InitPop_Load(DOLWorldConfig & config);

  void SetupDemeHardware();

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
//                          DOLWorld::DEME member definitions
// =============================================================================

void DOLWorld::Deme::BuildNeighborLookup()  {
  const size_t num_cells = width * height;
  neighbor_lookup.resize(num_cells * NUM_DIRECTIONS);
  for (size_t i = 0; i < num_cells; ++i) {
    for (size_t d = 0; d < NUM_DIRECTIONS; ++d) {
      neighbor_lookup[i*NUM_DIRECTIONS + d] = CalcNeighbor(i, Dir[d]);
    }
  }
}

size_t DOLWorld::Deme::CalcNeighbor(size_t id, Facing dir) const {
  int facing_x = (int)GetCellY(id);
  int facing_y = (int)GetCellX(id);
  switch (dir) {
    case Facing::N:
      facing_y = emp::Mod(facing_y + 1, (int)height);
      break;
    case Facing::NE:
      facing_x = emp::Mod(facing_x + 1, (int)width);
      facing_y = emp::Mod(facing_y + 1, (int)height);
      break;
    case Facing::E:
      facing_x = emp::Mod(facing_x + 1, (int)width);
      break;
    case Facing::SE:
      facing_x = emp::Mod(facing_x + 1, (int)width);
      facing_y = emp::Mod(facing_y - 1, (int)height);
      break;
    case Facing::S:
      facing_y = emp::Mod(facing_y - 1, (int)height);
      break;
    case Facing::SW:
      facing_x = emp::Mod(facing_x - 1, (int)width);
      facing_y = emp::Mod(facing_y - 1, (int)height);
      break;
    case Facing::W:
      facing_x = emp::Mod(facing_x - 1, (int)width);
      break;
    case Facing::NW:
      facing_x = emp::Mod(facing_x - 1, (int)width);
      facing_y = emp::Mod(facing_y + 1, (int)height);
      break;
    default:
      emp_assert(false, "Invalid direction!");
      break;
  }
  return GetCellID(facing_x, facing_y);
}

/// Set SignalGP hardware (on cellular hardware) maximum thread count
void DOLWorld::Deme::SetCellHardwareMaxThreads(size_t val) {
  for (CellularHardware & cell : cells) {
    cell.sgp_hw.SetMaxCores(val);
  }
}

/// Set SignalGP hardware (on cellular hardware) maximum call depth
void DOLWorld::Deme::SetCellHardwareMaxCallDepth(size_t val) {
  for (CellularHardware & cell : cells) {
    cell.sgp_hw.SetMaxCallDepth(val);
  }
}

/// Set SignalGP hardware (on cellular hardware) minimum tag matching threshold
void DOLWorld::Deme::SetCellHardwareMinTagMatchThreshold(double val) {
  for (CellularHardware & cell : cells) {
    cell.sgp_hw.SetMinBindThresh(val);
  }
}

/// Set SignalGP hardware (on cellular hardware) tie break procedure
void DOLWorld::Deme::SetCellHardwareStochasticTieBreaks(bool val) {
  for (CellularHardware & cell : cells) {
    cell.sgp_hw.SetStochasticFunCall(val);
  }
}

/// Given a Facing direction, return a representative string (useful for debugging)
std::string DOLWorld::Deme::FacingStr(Facing dir) const {
  switch (dir) {
    case Facing::N: return "N";
    case Facing::NE: return "NE";
    case Facing::E: return "E";
    case Facing::SE: return "SE";
    case Facing::S: return "S";
    case Facing::SW: return "SW";
    case Facing::W: return "W";
    case Facing::NW: return "NW";
    default:
      emp_assert(false, "Invalid direction!");
      return "";
  }
}

/// Print the deme's neighbor map! (useful for debugging)
void DOLWorld::Deme::PrintNeighborMap(std::ostream & os /*= std::cout*/) const {
  const size_t num_cells = width * height;
  for (size_t i = 0; i < num_cells; ++i) {
    os << i << " (" << GetCellX(i) << ", " << GetCellY(i) << "): " << std::endl;
    for (size_t d = 0; d < NUM_DIRECTIONS; ++d) {
      const size_t neighbor_id = neighbor_lookup[i*NUM_DIRECTIONS + d];
      os << "  " << FacingStr(Dir[d]) << "(" << d << "): " << neighbor_id << "(" << GetCellX(neighbor_id) << ", " << GetCellY(neighbor_id) << ")" << std::endl;
    }
  }
}

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
    InjectAt(GenRandDigitalOrganismGenome<TAG_WIDTH>(*random_ptr, *inst_lib, config), i);
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

/// Setup the experiment.
void DOLWorld::Setup(DOLWorldConfig & config) {
  std::cout << "DOLWorld - Setup" << std::endl;

  if (setup) {
    emp_assert(false, "Cannot Setup DOLWorld more than once!");
    exit(-1);
  }

  InitConfigs(config);

  // todo - setup setup signalgp instruction/event libraries

  // Setup deme hardware
  SetupDemeHardware();

  // Note, this is the function I would modify/parameterize if we wanted
  // to have single birth => multiple cells activated on placement
  fun_seed_deme = [this](Deme & deme, org_t & org) {
    // (1) select a random cell in the deme
    const size_t cell_id = GetRandom().GetUInt(deme.GetCellCapacity());
    CellularHardware & cell_hw = deme.GetCell(cell_id);
    // (2) activate focal cell
    cell_hw.ActivateCell(org.GetGenome().program, org.GetGenome().birth_tag, sgp_memory_t(), true);
  };

  SetPopStruct_Mixed(false);  // Mixed population (at deme/organism-level), asynchronous generations

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
  // todo - when do mutations happen? automatically?

  // todo - setup environment
  // todo - setup systematics

  // todo - finish function
  inst_lib = emp::NewPtr<inst_lib_t>();
  inst_lib->AddInst("Inc", sgp_hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  event_lib = emp::NewPtr<event_lib_t>();
  InitPop(config);

  // todo - configure world appropriately (e.g., asynchronous, mixed)
  setup = true;
  emp_assert(pop.size() == demes.size(), "SETUP ERROR! Population vector size (", pop.size(), ")", "does not match deme vector size (", demes.size(), ").");
}

#endif