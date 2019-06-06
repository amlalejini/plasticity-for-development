//  This file is part of example
//  Copyright (C) Alex Lalejini, 2019.
//  Released under MIT license; see LICENSE

#include <iostream>
#include <cmath>

#include "web/web.h"
#include "tools/math.h"

#include "../DOLWorld.h"
#include "../DOLWorldConfig.h"

namespace UI = emp::web;

// todo - allow config params to be modified
// todo - parameterize colors

// reminder: source ~/gen_ws/emsdk/emsdk_env.sh

double GetHTMLElementWidthByID(const std::string & id) {
  return EM_ASM_DOUBLE({
      var id = UTF8ToString($0);
      return $('#' + id).width();
    }, id.c_str());
}

double GetHTMLElementHeightByID(const std::string & id) {
  return EM_ASM_DOUBLE({
      var id = UTF8ToString($0);
      return $('#' + id).height();
    }, id.c_str());
}

class DOLWorldWebInterface : public UI::Animate, public DOLWorld {
public:
  static constexpr double MIN_ENV_CANVAS_RES_WIDTH=10;
  static constexpr double DEME_CELL_SIZE=15;
  static constexpr double DEME_MARGIN_SIZE=5;

  using env_res_color_fun_t = std::function<std::string(size_t)>;
  using canvas_draw_fun_t = std::function<void(void)>;

protected:
  DOLWorldConfig config;

  UI::Document world_view;
  UI::Document stats_view;
  UI::Document controls;

  UI::Button run_toggle_but;
  UI::Button run_step_but;
  UI::Button configure_but;
  UI::Selector world_display_selector;

  UI::Canvas world_display;

  emp::vector<std::string> env_res_color_map;


  size_t num_deme_cols=0;
  size_t num_deme_rows=0;
  double deme_width=0;
  double deme_height=0;

  double max_res_level=0;

  canvas_draw_fun_t canvas_draw_fun;
  canvas_draw_fun_t draw_deme_cell_sensors = [this]() {
    world_display.Clear("white");

    for (size_t deme_id = 0; deme_id < demes.size(); ++deme_id) {
      Deme & deme = GetDeme(deme_id);
      // What's this deme's row/column id?
      const size_t deme_row = deme_id / num_deme_cols;
      const size_t deme_col = deme_id % num_deme_cols;
      const double margin = DEME_MARGIN_SIZE/2.0;
      const double deme_x = (deme_col * deme_width) + margin;
      const double deme_y = (deme_row * deme_height) + margin;
      if (deme.IsActive()) {
        // world_display.Rect(deme_x, deme_y, deme_width - margin, deme_height - margin, "white", "white");
        // Draw Cells
        for (size_t cell_id = 0; cell_id < deme.GetCellCapacity(); ++cell_id) {
          Deme::CellularHardware & cell = deme.GetCell(cell_id);
          const size_t cell_col = deme.GetCellX(cell_id);
          const size_t cell_row = deme.GetCellY(cell_id);
          const double cell_x = deme_x + (cell_col * DEME_CELL_SIZE);
          const double cell_y = deme_y + (cell_row * DEME_CELL_SIZE);
          if (deme.IsCellActive(cell_id)) {
            // Alive cell => tan
            world_display.Rect(cell_x, cell_y, DEME_CELL_SIZE, DEME_CELL_SIZE, "tan", "black");

            // Display PERIODIC sensors
            const size_t num_sensors = NUM_PERIODIC_RESOURCES;
            if (num_sensors > 0) {
              const double bar_height = DEME_CELL_SIZE / (double)num_sensors;
              const double bar_width = DEME_CELL_SIZE;
              for (size_t sensor_id = NUM_STATIC_RESOURCES; sensor_id < TOTAL_RESOURCES; ++sensor_id) {
                const size_t offset = sensor_id - NUM_STATIC_RESOURCES;
                const double sensor_x = cell_x;
                const double sensor_y = cell_y + (bar_height*offset);
                // std::cout << cell.resource_sensors[sensor_id] << std::endl;
                if (cell.resource_sensors[sensor_id]) {
                  world_display.Rect(sensor_x, sensor_y, bar_width, bar_height, env_res_color_fun(sensor_id), "black");
                } else {
                  world_display.Rect(sensor_x, sensor_y, bar_width, bar_height, "tan", "black");
                }
              }
            }

          } else {
            world_display.Rect(cell_x, cell_y, DEME_CELL_SIZE, DEME_CELL_SIZE, "grey", "black");
          }
        }
      } else {
        world_display.Rect(deme_x, deme_y, deme_width-DEME_MARGIN_SIZE, deme_height-DEME_MARGIN_SIZE, "black");
      }
    }
  };

  canvas_draw_fun_t draw_env_res_levels = [this]() {
    world_display.Clear("white");
    for (size_t env_id = 0; env_id < environments.size(); ++env_id) {
      // todo - inactive environments should not get drawn (just be greyed out)
      Environment & env = environments[env_id];
      // What row/col is this environment on?
      const size_t env_row = env_id / num_deme_cols;
      const size_t env_col = env_id % num_deme_cols;
      const double margin = DEME_MARGIN_SIZE/2.0;
      const double env_x = (env_col * deme_width) + margin;
      const double env_y = (env_row * deme_height) + margin;
      const double env_width = deme_width-DEME_MARGIN_SIZE;
      const double env_height = deme_width-DEME_MARGIN_SIZE;
      world_display.Rect(env_x, env_y, env_width, env_height, "grey", "grey");

      // draw each resource level
      for (size_t res_id = 0; res_id < env.resources.size(); ++res_id) {
        Resource res = env.resources[res_id];
        double res_width = env_width / TOTAL_RESOURCES;
        double res_height = (res.GetAmount() / max_res_level) * env_height;
        double res_x = env_x + res_width*res_id;
        double res_y = env_y + (env_height - res_height);
        world_display.Rect(res_x, res_y, res_width, res_height, env_res_color_fun(res_id), env_res_color_fun(res_id));
      }
    }
  };

  env_res_color_fun_t env_res_color_fun = [this](size_t env_id) {
    emp_assert(env_id < env_res_color_map.size());
    return env_res_color_map[env_id];
  };

  void ConfigCanvasSize() {
    const size_t num_demes = demes.size();
    double parent_w = GetHTMLElementWidthByID("world-view");

    deme_width = (double)(DEME_WIDTH * DEME_CELL_SIZE) + DEME_MARGIN_SIZE;
    deme_height = (double)(DEME_HEIGHT * DEME_CELL_SIZE) + DEME_MARGIN_SIZE;

    num_deme_cols = (size_t)emp::Max(std::floor(parent_w/(deme_width)), 1.0);
    emp_assert(num_deme_cols > 0);

    num_deme_rows = std::ceil( ((double)num_demes) / ((double)num_deme_cols) );
    emp_assert(num_deme_cols * num_deme_rows >= num_demes);

    world_display.SetSize(deme_width*num_deme_cols, deme_height*num_deme_rows);
  }

public:
  DOLWorldWebInterface()
    : world_view("world-view"),
      stats_view("stats-view"),
      controls("controls"),
      world_display_selector("world-display-selector"),
      world_display(10,10,"world-display-canvas")
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
    config.MAX_POP_SIZE(100);
    config.INIT_POP_SIZE(10);
    Setup(config);

    max_res_level = emp::Max(config.PERIODIC_RESOURCES__LEVEL(), config.STATIC_RESOURCES__LEVEL());

    // Setup color maps
    env_res_color_map = emp::GetHueMap(TOTAL_RESOURCES, 0, 280, 50, 50);

    // SETUP INTERFACE CONTROL BUTTONS
    // Create run/pause button => Not using Animation's 'GetToggleButton' because
    // we want control over the button's callback.
    run_toggle_but = UI::Button([this] () {
      ToggleActive();
      run_toggle_but.SetLabel(active ? "Stop" : "Start");
      active ? run_step_but.SetDisabled(true) : run_step_but.SetDisabled(false);
    }, "Start", "run-toggle-button");
    run_toggle_but.SetAttr("class", "btn btn-primary m-1");
    // Run-step (run world for a single step) button
    run_step_but = GetStepButton("run-step-button");
    run_step_but.SetAttr("class", "btn btn-primary m-1");
    // todo - configure button
    configure_but = UI::Button([this]() {
      std::cout << "Configure!" << std::endl; // todo!
    }, "Configure", "run-config-button");
    configure_but.SetAttr("class", "btn btn-primary m-1");

    world_display_selector.SetOption("Demes - Cell Sensors",
                                     [this]() {
                                       canvas_draw_fun = draw_deme_cell_sensors;
                                       RedrawWorldCanvas();
                                     }, 0);
    world_display_selector.SetOption("Environments - Resource Levels",
                                     [this]() {
                                       canvas_draw_fun = draw_env_res_levels;
                                       RedrawWorldCanvas();
                                     }, 1);
    world_display_selector.SetAttr("class", "custom-select");

    canvas_draw_fun = draw_deme_cell_sensors;

    // Add buttons to dashboard
    controls << run_toggle_but;
    controls << run_step_but;     // todo - disable step when running
    controls << configure_but;
    controls << world_display_selector;

    // Stats area
    stats_view << "Update: " << UI::Live( [this]() { return GetUpdate(); } );
    stats_view << "<br/>Number of organisms: " << UI::Live( [this]() { return GetNumOrgs(); } );

    // World view area

    // Configure canvas
    ConfigCanvasSize();
    world_view << world_display;

    // What happens if page resizes?
    emp::OnResize([this]() {
      std::cout << "Resize?" << std::endl;
      ConfigCanvasSize();
      RedrawWorldCanvas();
    });
    RedrawWorldCanvas();
    stats_view.Redraw();
  }

  void DoFrame() {
    RunStep();
    RedrawWorldCanvas();
    stats_view.Redraw();
  }


  void RedrawWorldCanvas() {
    world_display.Freeze();
    canvas_draw_fun();
    world_display.Activate();
  }

};

DOLWorldWebInterface interface;

int main() {

}
