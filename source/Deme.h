#ifndef _DEME_H
#define _DEME_H

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

/// A 'deme' of CellularHardware.
class Deme {
public:
  // public aliases
  using org_t = DigitalOrganism;
  using sgp_hardware_t = emp::EventDrivenGP_AW<DOLWorldConstants::TAG_WIDTH>;
  using sgp_program_t = typename sgp_hardware_t::Program;
  using sgp_memory_t = typename sgp_hardware_t::memory_t;
  using tag_t = typename sgp_hardware_t::affinity_t;
  using inst_lib_t = typename sgp_hardware_t::inst_lib_t;
  using event_lib_t = typename sgp_hardware_t::event_lib_t;

  enum Facing { N=0, NE=1, E=2, SE=3, S=4, SW=5, W=6, NW=7 };                   ///< All possible directions
  static constexpr Facing Dir[] {Facing::N, Facing::NE, Facing::E, Facing::SE,  ///< Array of possible directions
                                Facing::S, Facing::SW, Facing::W, Facing::NW};
  static constexpr size_t NUM_DIRECTIONS = 8;                                   ///< Number of neighbors each board space has.

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

    void AdvanceStep() {
      sgp_hw.SingleProcess();
    }
  };

private:
  size_t deme_id = 0;                  ///< ID associated with deme. Corresponds to position in world population.
  bool deme_active = false;            ///< Is this deme actively running?
  size_t width;                        ///< Width of grid
  size_t height;                       ///< Height of grid
  emp::Ptr<emp::Random> random_ptr;
  emp::vector<size_t> neighbor_lookup; ///< Lookup table for neighbors
  emp::vector<CellularHardware> cells; ///< Toroidal grid of CellularHardware units
  emp::vector<size_t> cell_schedule;   ///< Order to execute cells

  /// Build neighbor lookup (according to current width and height)
  void BuildNeighborLookup();

  /// Calculate the neighbor ID of the cell (specified by id) in the specified direction.
  size_t CalcNeighbor(size_t id, Facing dir) const;

public:
  Deme(size_t _width, size_t _height, emp::Ptr<emp::Random> _rnd,
      emp::Ptr<inst_lib_t> _inst_lib, emp::Ptr<event_lib_t> _event_lib)
    : width(_width), height(_height), random_ptr(_rnd)
  {
    for (size_t i = 0; i < width*height; ++i) {
      cells.emplace_back(_rnd, _inst_lib, _event_lib);
      cells.back().cell_id = i; // Cell id corresponds to position in cells vector
      cell_schedule.emplace_back(i);
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

  void Advance(size_t steps) {
    for (size_t i = 0; i < steps; ++i) {
      SingleAdvance();
    }
  }

  void SingleAdvance() {
    // Advance cells in random order
    emp::Shuffle(*random_ptr, cell_schedule);
    for (size_t id : cell_schedule) {
      if (!cells[id].active) {
        continue;
      }
      CellularHardware & cell = cells[id];
      cell.AdvanceStep(); // Advance cell by one step
      // todo - if no threads and no sensors => mark as deactivated!
    }
  }

  /// Return a string representation of the given facing direction (useful for debugging)
  std::string FacingStr(Facing dir) const;

  /// Pretty print the neighbor map (useful for debugging)
  void PrintNeighborMap(std::ostream & os = std::cout) const;
};

void Deme::BuildNeighborLookup()  {
  const size_t num_cells = width * height;
  neighbor_lookup.resize(num_cells * NUM_DIRECTIONS);
  for (size_t i = 0; i < num_cells; ++i) {
    for (size_t d = 0; d < NUM_DIRECTIONS; ++d) {
      neighbor_lookup[i*NUM_DIRECTIONS + d] = CalcNeighbor(i, Dir[d]);
    }
  }
}

size_t Deme::CalcNeighbor(size_t id, Facing dir) const {
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
void Deme::SetCellHardwareMaxThreads(size_t val) {
  for (CellularHardware & cell : cells) {
    cell.sgp_hw.SetMaxCores(val);
  }
}

/// Set SignalGP hardware (on cellular hardware) maximum call depth
void Deme::SetCellHardwareMaxCallDepth(size_t val) {
  for (CellularHardware & cell : cells) {
    cell.sgp_hw.SetMaxCallDepth(val);
  }
}

/// Set SignalGP hardware (on cellular hardware) minimum tag matching threshold
void Deme::SetCellHardwareMinTagMatchThreshold(double val) {
  for (CellularHardware & cell : cells) {
    cell.sgp_hw.SetMinBindThresh(val);
  }
}

/// Set SignalGP hardware (on cellular hardware) tie break procedure
void Deme::SetCellHardwareStochasticTieBreaks(bool val) {
  for (CellularHardware & cell : cells) {
    cell.sgp_hw.SetStochasticFunCall(val);
  }
}

/// Given a Facing direction, return a representative string (useful for debugging)
std::string Deme::FacingStr(Facing dir) const {
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
void Deme::PrintNeighborMap(std::ostream & os /*= std::cout*/) const {
  const size_t num_cells = width * height;
  for (size_t i = 0; i < num_cells; ++i) {
    os << i << " (" << GetCellX(i) << ", " << GetCellY(i) << "): " << std::endl;
    for (size_t d = 0; d < NUM_DIRECTIONS; ++d) {
      const size_t neighbor_id = neighbor_lookup[i*NUM_DIRECTIONS + d];
      os << "  " << FacingStr(Dir[d]) << "(" << d << "): " << neighbor_id << "(" << GetCellX(neighbor_id) << ", " << GetCellY(neighbor_id) << ")" << std::endl;
    }
  }
}

#endif