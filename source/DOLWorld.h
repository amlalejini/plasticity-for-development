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
);

constexpr size_t TAG_WIDTH = 16;

struct Deme {

};

struct DemeBioFilm {

};

class DOLWorld : public emp::World<DigitalOrganism<TAG_WIDTH>> {

};

#endif