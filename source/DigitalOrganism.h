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

template<size_t TAG_WIDTH>
class DigitalOrganism {
public:
  struct Genome;
  struct Phenotype;

  using program_t = typename emp::EventDrivenGP_AW<TAG_WIDTH>::Program;
  using tag_t = typename emp::EventDrivenGP_AW<TAG_WIDTH>::affinity_t;   // Actual type: BitSet<TAG_WIDTH>

  struct Genome {
    tag_t birth_tag = tag_t();   /// Default tag used to trigger module on birth.
    program_t program;           /// Organism program (a SignalGP program)

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

  /// Get reference to digital organism genome.
  Genome & GetGenome() { return genome; }

  /// Get const reference to digital organism genome.
  const Genome & GetGenome() const { return genome; }

  /// Get reference to digital organism's phenotype.
  Phenotype & GetPhenotype() { return phenotype; }

  /// Get const reference to digital organism's phenotype.
  const Phenotype & GetPhenotype() const { return phenotype; }

};

#endif