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

// Empirical includes
#include "base/Ptr.h"
#include "base/vector.h"
#include "config/ArgManager.h"
#include "Evolve/World.h"
#include "tools/math.h"

// Local includes
#include "DigitalOrganism.h"

EMP_BUILD_CONFIG( DOLWorldConfig,
  GROUP(MAIN, "Global Settings"),
  VALUE(SEED, int, -1, "Random number generator seed"),
  VALUE(UPDATES, size_t, 1000, "Number of updates to run the experiment."),
  VALUE(CPU_CYCLES_PER_UPDATE, size_t, 30, "Number of CPU cycles to distribute to each cell every update."),
  VALUE(INIT_POP_SIZE, size_t, 1, "How many organisms should we seed the world with?"),
  VALUE(MAX_POP_SIZE, size_t, 1000, "What is the maximum size of the population?"),

  GROUP(DEME, "Deme Settings"),
  VALUE(DEME_WIDTH, size_t, 5, "What is the maximum cell-width of a deme?"),
  VALUE(DEME_HEIGHT, size_t, 5, "What is the maximum cell-height of a deme?"),

);

constexpr size_t TAG_WIDTH = 16;

/// Struct that houses demes.
// struct DemeVat {

// };

class DOLWorld : public emp::World<DigitalOrganism<TAG_WIDTH>> {
public:
  using org_t = DigitalOrganism<TAG_WIDTH>;
  using sgp_hardware_t = emp::EventDrivenGP_AW<TAG_WIDTH>;
  using inst_lib_t = typename sgp_hardware_t::inst_lib_t;
  using event_lib_t = typename sgp_hardware_t::event_lib_t;

  /// Hardware unit that each cell in a deme 'runs' on
  struct CellularHardware {

    sgp_hardware_t sgp_hw;

    // - sensors: emp::vector<size_t> sensors;
    // - sensor refractory state: emp::vector<size_t> refractory_states;
  };

  /// A 'deme' of CellularHardware.
  class Deme {
  public:
    enum Facing { N=0, NE=1, E=2, SE=3, S=4, SW=5, W=6, NW=7 };                   ///< All possible directions
    static constexpr Facing Dir[] {Facing::N, Facing::NE, Facing::E, Facing::SE,  ///< Array of possible directions
                                  Facing::S, Facing::SW, Facing::W, Facing::NW};
    static constexpr size_t NUM_DIRECTIONS = 8;                                   ///< Number of neighbors each board space has.

    // - Torodial grid of hardware units.
    emp::vector<CellularHardware> cells;

  private:

    size_t width;   ///< Width of grid
    size_t height;  ///< Height of grid
    emp::vector<size_t> neighbor_lookup; ///< Lookup table for neighbors

    /// Build neighbor lookup (according to current width and height)
    void BuildNeighborLookup();

    /// Calculate the neighbor ID of the cell (specified by id) in the specified direction.
    size_t CalcNeighbor(size_t id, Facing dir) const;

  public:
    Deme(size_t _width, size_t _height)
      : width(_width), height(_height)
    {
      // cells.resize(width * height);
      BuildNeighborLookup();
    }

    // todo
    void ConfigureCellularHardware();

    /// Get cell capacity of deme
    size_t GetCellCapacity() const { return cells.size(); }

    /// Given cell ID, return x coordinate
    size_t GetCellX(size_t id) const { return id % width; }

    /// Given cell ID, return y coordinate
    size_t GetCellY(size_t id) const { return id / width; }

    /// Given x,y coordinate, return cell ID
    size_t GetCellID(size_t x, size_t y) const { return (y * width) + x; }

    /// Given a cell ID and facing (of that cell), return the appropriate neighboring cell ID
    size_t GetNeighboringCellID(size_t id, Facing dir) const { return neighbor_lookup[id*NUM_DIRECTIONS + (size_t)dir]; }

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
  // DEME Configuration Settings
  size_t DEME_WIDTH;
  size_t DEME_HEIGHT;

  void InitConfigs(DOLWorldConfig & config);
  void InitPop();

public:

  DOLWorld() {}
  DOLWorld(emp::Random & r) : emp::World<org_t>(r) {}

  ~DOLWorld() {}

  size_t GetCPUCyclesPerUpdate() const { return CPU_CYCLES_PER_UPDATE; }
  size_t GetDemeWidth() const { return DEME_WIDTH; };
  size_t GetDemeHeight() const { return DEME_HEIGHT; };
  size_t GetDemeCapacity() const { return DEME_WIDTH * DEME_HEIGHT; };

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

void DOLWorld::InitConfigs(DOLWorldConfig & config) {
  // MAIN Configuration Settings
  UPDATES = config.UPDATES();
  CPU_CYCLES_PER_UPDATE = config.CPU_CYCLES_PER_UPDATE();
  INIT_POP_SIZE = config.INIT_POP_SIZE();
  MAX_POP_SIZE = config.MAX_POP_SIZE();
  // DEME Configuration Settings
  DEME_WIDTH = config.DEME_WIDTH();
  DEME_HEIGHT = config.DEME_HEIGHT();
}

void DOLWorld::Setup(DOLWorldConfig & config) {
  InitConfigs(config);
  // todo - finish function
}

#endif