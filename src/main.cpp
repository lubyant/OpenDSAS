#include <iostream>

#include "cli.hpp"
#include "dsas.hpp"
#include "grid.hpp"
#include "intersect.hpp"
#include "options.hpp"
#include "utility.hpp"

// --------------------------- runners ----------------------------------

static void cal_erosion_rate(std::vector<dsas::TransectLine>& transects) {
  for (auto& transect : transects) {
    auto intersects = transect.intersects;
    auto reg_rate = dsas::linearRegressRate(intersects);
    transect.change_rate = reg_rate;
  }
}

static void print_messages() {
  std::cout << "Welcome to digital shoreline analysis system\n";
  std::cout << "Your shoreline path: "
            << std::filesystem::absolute(dsas::options.shoreline_path) << "\n"
            << "Your baseline path: "
            << std::filesystem::absolute(dsas::options.baseline_path) << "\n"
            << "Output intersects path: "
            << std::filesystem::absolute(dsas::options.intersect_path) << "\n"
            << "Output transects path (with erosion rate): "
            << std::filesystem::absolute(dsas::options.transect_path) << "\n";
  std::cout << "Start to run\n";
}

static void run_root() {
  // Original all-in-one flow using baseline + shoreline
  print_messages();

  auto baselines = dsas::load_baselines_shp(dsas::options.baseline_path, "Id");
  auto shorelines =
      dsas::load_shorelines_shp(dsas::options.shoreline_path, "Date");
  auto transects = dsas::generate_transects(baselines);

  std::vector<std::unique_ptr<dsas::IntersectPoint>> intersects;
  if (dsas::options.build_index) {
    auto grids = dsas::build_spatial_grids(shorelines, transects);
    intersects = dsas::generate_intersects(transects, grids);
  } else {
    intersects = dsas::generate_intersects(transects, shorelines);
  }

  for (auto& t : transects) {
    if (!t->intersects.empty()) {
      t->change_rate = dsas::linearRegressRate(t->intersects);
    }
  }

  auto prj = dsas::get_shp_proj(dsas::options.shoreline_path.c_str());
  dsas::save_transect(transects, prj);
  dsas::save_intersects(intersects, prj);
}

static void run_cast() {
  auto baselines = dsas::load_baselines_shp(dsas::options.baseline_path, "Id");
  auto transects = dsas::generate_transects(baselines);
  auto prj = dsas::get_shp_proj(dsas::options.baseline_path.c_str());
  dsas::save_transect(transects, prj);
}

static void run_cal() {
  print_messages();

  auto baselines = dsas::load_baselines_shp(dsas::options.baseline_path, "Id");
  auto shorelines =
      dsas::load_shorelines_shp(dsas::options.shoreline_path, "Date");
  auto transects = dsas::generate_transects(baselines);

  std::cout << "Start to run\n";
}

// ------------------------------ main ----------------------------------

int main(int argc, char* argv[]) {
  auto cli_state = parse_args(argc, argv);
  switch (cli_state) {
    case dsas::CliState::root:
      run_root();
      break;
    case dsas::CliState::cast:
      run_cast();
      break;
    case dsas::CliState::cal:
      run_cal();
      break;
    default:
      std::cerr << "wrong cli command\n";
      exit(1);
  }
  return 0;
}
