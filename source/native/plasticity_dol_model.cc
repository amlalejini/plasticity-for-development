//  This file is part of example
//  Copyright (C) Alex Lalejini, 2019.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "base/vector.h"
#include "config/command_line.h"

#include "../DOLWorld.h"
#include "../DOLWorldConfig.h"

#include <unordered_map>
#include "tools/map_utils.h"

// This is the main function for the NATIVE version of example.

int main(int argc, char* argv[])
{
  std::string config_fname = "DOLWorldConfig.cfg";
  DOLWorldConfig config;
  auto args = emp::cl::ArgManager(argc, argv);
  config.Read(config_fname);
  if (args.ProcessConfigOptions(config, std::cout, "DOLWorldConfig.cfg", "DOLWorld-macros.h") == false) exit(0);
  if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.
  // Write to screen how the experiment is configured
  std::cout << "==============================" << std::endl;
  std::cout << "|    How am I configured?    |" << std::endl;
  std::cout << "==============================" << std::endl;
  config.Write(std::cout);
  std::cout << "==============================\n" << std::endl;

  emp::Random rnd(config.SEED());
  DOLWorld world(rnd);

  world.Setup(config);
  world.Run();
}
