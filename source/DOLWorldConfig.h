
#ifndef _DOL_WORLD_CONFIG_H
#define _DOL_WORLD_CONFIG_H

#include "config/config.h"

namespace DOLWorldConstants {
  constexpr size_t TAG_WIDTH = 16;
}

// DOLWorld Configuration
EMP_BUILD_CONFIG( DOLWorldConfig,
  GROUP(MAIN, "Global Settings"),
  VALUE(SEED, int, -1, "Random number generator seed"),
  VALUE(UPDATES, size_t, 1000, "Number of updates to run the experiment."),
  VALUE(CPU_CYCLES_PER_UPDATE, size_t, 30, "Number of CPU cycles to distribute to each cell every update."),
  VALUE(INIT_POP_SIZE, size_t, 1, "How many organisms should we seed the world with?"),
  VALUE(MAX_POP_SIZE, size_t, 1000, "What is the maximum size of the population?"),
  VALUE(INIT_POP_MODE, std::string, "random", "How should the population be initialized? Options:\n\t'random'"),

  GROUP(RESOURCES, "Resource Settings"),
  VALUE(RESOURCE_CONSUMPTION_MODE, std::string, "fixed", "How are resources consumed? Options:\n\t(1) 'fixed'\n\t(2) 'proportional'"),

  VALUE(NUM_PERIODIC_RESOURCES, size_t, 4, "How many simple tasks should there be?"),
  VALUE(PERIODIC_RESOURCES__LEVEL, double, 100.0, "How much of a periodic resource is made available on pulse?"),

  VALUE(PERIODIC_RESOURCES__CONSUME_FIXED, double, 25.0, "How much of a periodic resource is collected when metabolized?"),
  VALUE(PERIODIC_RESOURCES__CONSUME_PROPORTIONAL, double, 1.0, "How much of a periodic resource is collected when metabolized?"),
  VALUE(PERIODIC_RESOURCES__FAILURE_COST, double, 0.0, "What is the cost of attempting to consume an unavailable static resource?"),
  VALUE(PERIODIC_RESOURCES__MIN_UPDATES_UNAVAILABLE, size_t, 1, "How many updates should a resource be unavailable before it is eligible to pulse?"),

  VALUE(NUM_STATIC_RESOURCES, size_t, 1, "How many tasks are always rewarded?"),
  VALUE(STATIC_RESOURCES__LEVEL, double, 25.0, "How much of a static resource is made available every update?"),

  VALUE(STATIC_RESOURCES__CONSUME_FIXED, double, 25.0, "How much of a static resource is collected when metabolized?"),
  VALUE(STATIC_RESOURCES__CONSUME_PROPORTIONAL, double, 1.0, "How much of a static resource is collected when metabolized?"),
  VALUE(STATIC_RESOURCES__FAILURE_COST, double, 0.0, "What is the cost of attempting to consume an unavailable static resource?"),

  GROUP(DEME, "Deme Settings"),
  VALUE(DEME_WIDTH, size_t, 5, "What is the maximum cell-width of a deme?"),
  VALUE(DEME_HEIGHT, size_t, 5, "What is the maximum cell-height of a deme?"),

  GROUP(CELLULAR_HARDWARE, "Within-deme cellular hardware unit settings (SignalGP CPUs + extras)"),
  VALUE(SGP_MAX_THREAD_CNT, size_t, 4, "What is the maximum number of concurrently running threads allowed on a SignalGP CPU?"),
  VALUE(SGP_MAX_CALL_DEPTH, size_t, 128 , "What is the maximum call stack depth allowed on a SignalGP CPU?"),
  VALUE(SGP_MIN_TAG_MATCH_THRESHOLD, double, 0.0, "What is the tag matching threshold required for successful tag-based references on a SignalGP CPU?"),

  GROUP(PROGRAM, "SignalGP Program Settings"),
  VALUE(MIN_FUNCTION_CNT, size_t, 1, "Minimum allowed number of functions in a SignalGP program."),
  VALUE(MAX_FUNCTION_CNT, size_t, 64, "Maximum allowed number of functions in a SignalGP program."),
  VALUE(MIN_FUNCTION_LEN, size_t, 1, "Minimum number of instructions allowed in a SignalGP program function."),
  VALUE(MAX_FUNCTION_LEN, size_t, 256, "Maximum number of instructions allowed in a SignalGP program function."),
  VALUE(MIN_ARGUMENT_VAL, int, 0, "Minimum value for SignalGP instruction arguments."),
  VALUE(MAX_ARGUMENT_VAL, int, 15, "Maximum value for SignalGP instruction arguments."),

  GROUP(MUTATION, "DigitalOrganism Mutation Settings"),
  VALUE(PROGRAM_ARG_SUB__PER_ARG, double, 0.0025, "Program argument substitution rate (per-argument)"),
  VALUE(PROGRAM_INST_SUB__PER_INST, double, 0.005, "Program instruction substitution rate (per-instruction)"),
  VALUE(PROGRAM_INST_INS__PER_INST, double, 0.005, "Program instruction insertion rate (per-instruction)"),
  VALUE(PROGRAM_INST_DEL__PER_INST, double, 0.005, "Program instruction deletion rate (per-instruction)"),
  VALUE(PROGRAM_SLIP__PER_FUN, double, 0.05, "Program slip-mutation rate (per-function)"),
  VALUE(PROGRAM_FUNC_DUP__PER_FUN, double, 0.05, "Program whole-function duplication rate (per-function)"),
  VALUE(PROGRAM_FUNC_DEL__PER_FUN, double, 0.05, "Program whole-function deletion rate (per-function)"),
  VALUE(PROGRAM_TAG_BIT_FLIP__PER_BIT, double, 0.0025, "Program tags bit flip rate (per-bit)"),
  VALUE(BIRTH_TAG_BIT_FLIP__PER_BIT, double, 0.0025, "Birth tag bit flip rate (per-bit)"),

)

#endif