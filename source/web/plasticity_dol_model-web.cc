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
  UI::Document controls;

  UI::Button run_toggle;
  UI::Button run_step;

public:
  DOLWorldWebInterface()
    : world_view("world-view"),
      stats_view("stats-view"),
      controls("controls")
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

    // Create run buttons => Not using Animation's 'GetToggleButton' because we want control over the button's callback.
    run_toggle = UI::Button([this] () {
      ToggleActive();
      run_toggle.SetLabel(active ? "Stop" : "Start");
      active ? run_step.SetDisabled(true) : run_step.SetDisabled(false);
    }, "Start", "run-toggle-button");
    run_toggle.SetAttr("class", "btn btn-primary mx-1");

    run_step = GetStepButton("run-step-button");
    run_step.SetAttr("class", "btn btn-primary mx-1");

    // Add buttons to dashboard
    controls << run_toggle;
    controls << run_step;     // todo - disable step when running

  }

  void DoFrame() {
    std::cout << "DoFrame" << std::endl;
  }

};

DOLWorldWebInterface interface;

int main() {

}
