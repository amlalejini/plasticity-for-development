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

class DigitalOrganism {
public:
  struct Genome;
  struct Phenotype;
  using sgp_hardware_t = typename emp::EventDrivenGP_AW<DOLWorldConstants::TAG_WIDTH>;
  using program_t = typename sgp_hardware_t::Program;
  using tag_t = typename sgp_hardware_t::affinity_t;   // Actual type: BitSet<TAG_WIDTH>

  struct Genome {
    program_t program;           /// Organism program (a SignalGP program)
    tag_t birth_tag = tag_t();   /// Default tag used to trigger module on birth.

    Genome(const program_t & _program, const tag_t & _birth_tag)
      : program(_program), birth_tag(_birth_tag) {}

    Genome(const program_t & _program)
      : program(_program) {}
  };

  struct Phenotype {
    size_t age=0;                     ///< How many updates has this organism been alive?
    bool trigger_repro=false;         ///< Trigger reproduction?
    double resource_pool=0.0;               ///< How many resources are currently in this organism's resource pool?
    double total_resources_collected=0.0;   ///< How many resources has this organism collected in total across all cells?
    double total_resources_donated=0.0;     ///< How many resources have individual cells donated to the deme-level organism?
    size_t offspring_cnt=0;
    emp::vector<double> consumption_amount_by_type;
    emp::vector<size_t> consumption_successes_by_type;
    emp::vector<size_t> consumption_failures_by_type;
    emp::vector<size_t> resource_alerts_received_by_type;

    /// Reset this organism's phenotype
    void Reset(size_t num_resources) {
      age=0;
      trigger_repro=false;
      resource_pool=0.0;
      total_resources_collected=0.0;
      total_resources_donated=0.0;
      offspring_cnt=0;
      consumption_amount_by_type.clear();
      consumption_amount_by_type.resize(num_resources, 0.0);
      consumption_successes_by_type.clear();
      consumption_successes_by_type.resize(num_resources, 0);
      consumption_failures_by_type.clear();
      consumption_failures_by_type.resize(num_resources, 0);
      resource_alerts_received_by_type.clear();
      resource_alerts_received_by_type.resize(num_resources, 0);
    }
  };

protected:
  size_t org_id = 0;
  Genome genome;
  Phenotype phenotype;

public:
  DigitalOrganism(const Genome & _genome) : genome(_genome) {}

  /// Get organism id
  size_t GetOrgID() const { return org_id; }

  /// Get reference to digital organism genome.
  Genome & GetGenome() { return genome; }

  /// Get const reference to digital organism genome.
  const Genome & GetGenome() const { return genome; }

  /// Get reference to digital organism's phenotype.
  Phenotype & GetPhenotype() { return phenotype; }

  /// Get const reference to digital organism's phenotype.
  const Phenotype & GetPhenotype() const { return phenotype; }

  /// Set organism id
  void SetOrgID(size_t val) { org_id = val; }

  void PrettyPrintPhenotype(std::ostream & os = std::cout);

};

void DigitalOrganism::PrettyPrintPhenotype(std::ostream & os /*= std::cout*/) {
  // todo
}

// Generate and return a digital organism with a random genome.
// - Makes use of signalgp_utils.h's generate random program & random tag functions.
typename DigitalOrganism::Genome GenRandDigitalOrganismGenome(
    emp::Random & rnd,
    const emp::InstLib<emp::EventDrivenGP_AW<DOLWorldConstants::TAG_WIDTH>> & inst_lib,
    const DOLWorldConfig & config)
{
  return {emp::GenRandSignalGPProgram<DOLWorldConstants::TAG_WIDTH>(
                rnd, inst_lib,
                config.MIN_FUNCTION_CNT(), config.MAX_FUNCTION_CNT(),
                config.MIN_FUNCTION_LEN(), config.MAX_FUNCTION_LEN(),
                config.MIN_ARGUMENT_VAL(), config.MAX_ARGUMENT_VAL()),
          emp::GenRandSignalGPTag<DOLWorldConstants::TAG_WIDTH>(rnd)};
}

#endif