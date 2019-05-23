
#ifndef _DOL_WORLD_CONFIG_H
#define _DOL_WORLD_CONFIG_H

#include "config/config.h"

// DOLWorld Configuration
EMP_BUILD_CONFIG( DOLWorldConfig,
  GROUP(MAIN, "Global Settings"),
  VALUE(SEED, int, -1, "Random number generator seed"),
  VALUE(UPDATES, size_t, 1000, "Number of updates to run the experiment."),
  VALUE(CPU_CYCLES_PER_UPDATE, size_t, 30, "Number of CPU cycles to distribute to each cell every update."),
  VALUE(INIT_POP_SIZE, size_t, 1, "How many organisms should we seed the world with?"),
  VALUE(MAX_POP_SIZE, size_t, 1000, "What is the maximum size of the population?"),
  VALUE(INIT_POP_MODE, std::string, "random", "How should the population be initialized? Options:\n\t'random'"),

  GROUP(DEME, "Deme Settings"),
  VALUE(DEME_WIDTH, size_t, 5, "What is the maximum cell-width of a deme?"),
  VALUE(DEME_HEIGHT, size_t, 5, "What is the maximum cell-height of a deme?"),

  GROUP(PROGRAM, "SignalGP Program Settings"),
  VALUE(MIN_FUNCTION_CNT, size_t, 1, "Minimum allowed number of functions in a SignalGP program."),
  VALUE(MAX_FUNCTION_CNT, size_t, 64, "Maximum allowed number of functions in a SignalGP program."),
  VALUE(MIN_FUNCTION_LEN, size_t, 1, "Minimum number of instructions allowed in a SignalGP program function."),
  VALUE(MAX_FUNCTION_LEN, size_t, 256, "Maximum number of instructions allowed in a SignalGP program function."),
  VALUE(MIN_ARGUMENT_VAL, int, 0, "Minimum value for SignalGP instruction arguments."),
  VALUE(MAX_ARGUMENT_VAL, int, 15, "Maximum value for SignalGP instruction arguments."),

)

#endif