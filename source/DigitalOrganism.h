/**
 *  @date 2019
 *
 *  @file  DigitalOrganism.h
 *
 *  The DigitalOrganism class represents a digital organism. A digital organism
 *  has a phenotype and a genotype.
 */

#ifndef _DIGITAL_ORGANISM_H
#define _DIGITAL_ORGANISM_H

#include "hardware/EventDrivenGP.h"
#include "hardware/signalgp_utils.h"

#include "DOLWorldConfig.h"

template<size_t TAG_WIDTH>
class DigitalOrganism {
public:
  struct Genome;
  struct Phenotype;

  using program_t = typename emp::EventDrivenGP_AW<TAG_WIDTH>::Program;
  using tag_t = typename emp::EventDrivenGP_AW<TAG_WIDTH>::affinity_t;   // Actual type: BitSet<TAG_WIDTH>

  struct Genome {
    program_t program;           /// Organism program (a SignalGP program)
    tag_t birth_tag = tag_t();   /// Default tag used to trigger module on birth.

    Genome(const program_t & _program, const tag_t & _birth_tag)
      : program(_program), birth_tag(_birth_tag) {}

    Genome(const program_t & _program)
      : program(_program) {}
  };

  struct Phenotype {

  };

protected:
  Genome genome;
  Phenotype phenotype;

public:
  DigitalOrganism(const Genome & _genome) : genome(_genome) {}

  /// Get reference to digital organism genome.
  Genome & GetGenome() { return genome; }

  /// Get const reference to digital organism genome.
  const Genome & GetGenome() const { return genome; }

  /// Get reference to digital organism's phenotype.
  Phenotype & GetPhenotype() { return phenotype; }

  /// Get const reference to digital organism's phenotype.
  const Phenotype & GetPhenotype() const { return phenotype; }

};

// Generate and return a digital organism with a random genome.
// - Makes use of signalgp_utils.h's generate random program & random tag functions.
template<size_t W>
typename DigitalOrganism<W>::Genome GenRandDigitalOrganismGenome(
    emp::Random & rnd,
    const emp::InstLib<emp::EventDrivenGP_AW<W>> & inst_lib,
    const DOLWorldConfig & config)
{
  return {emp::GenRandSignalGPProgram<W>(rnd, inst_lib, config.MIN_FUNCTION_CNT(),
                                                        config.MAX_FUNCTION_CNT(),
                                                        config.MIN_FUNCTION_LEN(),
                                                        config.MAX_FUNCTION_LEN(),
                                                        config.MIN_ARGUMENT_VAL(),
                                                        config.MAX_ARGUMENT_VAL()),
          emp::GenRandSignalGPTag<W>(rnd)};
}

#endif