//  This file is part of example
//  Copyright (C) Alex Lalejini, 2019.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "web/web.h"

#include "../DOLWorld.h"
#include "../DOLWorldConfig.h"

namespace UI = emp::web;

// todo - allow config params to be modified

// reminder: source ~/gen_ws/emsdk/emsdk_env.sh

class DOLWorldWebInterface : public UI::Animate, public DOLWorld {
public:
protected:
  DOLWorldConfig config;

  UI::Document world_view;
  UI::Document stats_view;
  UI::Document dashboard;

public:
  DOLWorldWebInterface()
    : world_view("world-view"), stats_view("stats-view"), dashboard("dashboard")
  {
    std::cout << "DOLWorldInterface Constructor" << std::endl;
    SetupInterface();
  }

  void SetupInterface() {
    std::cout << "SetupInterface" << std::endl;
    // Configure random number seed
    GetRandom().ResetSeed(config.SEED());
    // todo - allow configuration to be changed
    // Setup the world based on default config
    Setup(config);

    // Attach
  }

  void DoFrame() {
    std::cout << "DoFrame" << std::endl;
  }

};

DOLWorldWebInterface interface;

int main() {

}
