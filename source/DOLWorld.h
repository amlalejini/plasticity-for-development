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

#include "Evolve/World.h"
#include "config/ArgManager.h"

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

/// A 'deme' of cells.
struct Deme {

};

/// Struct that houses demes.
struct DemeVat {

};

class DOLWorld : public emp::World<DigitalOrganism<TAG_WIDTH>> {
public:
  using org_t = DigitalOrganism<TAG_WIDTH>;

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