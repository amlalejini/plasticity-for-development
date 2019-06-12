#ifndef _DIGITAL_ORGANISM_MUTATOR_H
#define _DIGITAL_ORGANISM_MUTATOR_H

#include "hardware/EventDrivenGP.h"
#include "hardware/signalgp_utils.h"

#include "DOLWorldConfig.h"

class Mutator {
public:
  using tag_t = typename DigitalOrganism::tag_t;

protected:
  emp::SignalGPMutator<DOLWorldConstants::TAG_WIDTH> sgp_program_mutator;

  double BIRTH_TAG_BIT_FLIP__PER_BIT=0.0;

public:

  void Setup(const DOLWorldConfig & config) {
    // Setup sgp mutator
    // - Program constraints
    sgp_program_mutator.SetProgMinFuncCnt(config.MIN_FUNCTION_CNT());
    sgp_program_mutator.SetProgMaxFuncCnt(config.MAX_FUNCTION_CNT());
    sgp_program_mutator.SetProgMinFuncLen(config.MIN_FUNCTION_LEN());
    sgp_program_mutator.SetProgMaxFuncLen(config.MAX_FUNCTION_LEN());
    sgp_program_mutator.SetProgMaxTotalLen(config.MAX_FUNCTION_LEN() * config.MAX_FUNCTION_CNT());
    sgp_program_mutator.SetProgMinArgVal(config.MIN_ARGUMENT_VAL());
    sgp_program_mutator.SetProgMaxArgVal(config.MAX_ARGUMENT_VAL());
    // - Program mutation rates
    sgp_program_mutator.ARG_SUB__PER_ARG(config.PROGRAM_ARG_SUB__PER_ARG());
    sgp_program_mutator.INST_SUB__PER_INST(config.PROGRAM_INST_SUB__PER_INST());
    sgp_program_mutator.INST_INS__PER_INST(config.PROGRAM_INST_INS__PER_INST());
    sgp_program_mutator.INST_DEL__PER_INST(config.PROGRAM_INST_DEL__PER_INST());
    sgp_program_mutator.SLIP__PER_FUNC(config.PROGRAM_SLIP__PER_FUN());
    sgp_program_mutator.FUNC_DUP__PER_FUNC(config.PROGRAM_FUNC_DUP__PER_FUN());
    sgp_program_mutator.FUNC_DEL__PER_FUNC(config.PROGRAM_FUNC_DEL__PER_FUN());
    sgp_program_mutator.TAG_BIT_FLIP__PER_BIT(config.PROGRAM_TAG_BIT_FLIP__PER_BIT());

    BIRTH_TAG_BIT_FLIP__PER_BIT = config.BIRTH_TAG_BIT_FLIP__PER_BIT();
  }

  size_t Mutate(DigitalOrganism & org, emp::Random & rnd) {
    return Mutate(org.GetGenome(), rnd);
  }

  size_t Mutate(DigitalOrganism::Genome & genome, emp::Random & rnd) {
    size_t num_mutations = 0;
    num_mutations += sgp_program_mutator.ApplyMutations(genome.program, rnd);
    tag_t & tag = genome.birth_tag;
    for (size_t k = 0; k < tag.GetSize(); ++k) {
      if (rnd.P(BIRTH_TAG_BIT_FLIP__PER_BIT)) {
        tag.Toggle(k);
        ++num_mutations;
      }
    }
    return num_mutations;
  }

};

#endif