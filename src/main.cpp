#include <argparse/argparse.hpp>
#include <filesystem>
#include <iostream>

#include "dsas.hpp"
#include "grid.hpp"
#include "intersect.hpp"
#include "options.hpp"
#include "utility.hpp"

// --------------------------- enum mappers -----------------------------

static dsas::Options::IntersectionMode parse_intersection_mode(const std::string& s) {
  if (s == "closest")  return dsas::Options::IntersectionMode::Closest;
  if (s == "farthest") return dsas::Options::IntersectionMode::Farthest;
  throw std::runtime_error("Invalid --intersection-mode: " + s);
}

static dsas::Options::TransectOrientation parse_transect_orient(const std::string& s) {
  if (s == "left") return dsas::Options::TransectOrientation::Left;
  if (s == "right") return dsas::Options::TransectOrientation::Right;
  if (s == "mix") return dsas::Options::TransectOrientation::Mix;
  throw std::runtime_error("Invalid --transect-orientation: " + s);
}

// --------------------------- runners ----------------------------------

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

  auto baselines  = dsas::load_baselines_shp(dsas::options.baseline_path, "Id");
  auto shorelines = dsas::load_shorelines_shp(dsas::options.shoreline_path, "Date");
  auto transects  = dsas::generate_transects(baselines);

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

  auto baselines  = dsas::load_baselines_shp(dsas::options.baseline_path, "Id");
  auto shorelines = dsas::load_shorelines_shp(dsas::options.shoreline_path, "Date");
  auto transects  = dsas::generate_transects(baselines);

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

// ------------------------------ main ----------------------------------

int main(int argc, char* argv[]) {
  argparse::ArgumentParser program(PROJECT_NAME_STR, APP_VERSION);
  program.add_description("OpenDSAS: Digital Shoreline Analysis System");

  // Root parser (no subcommand) — original all-in-one arguments
  program.add_argument("--baseline")
      .required()
      .help("Path to the baseline file");

  program.add_argument("--shoreline")
      .required()
      .help("Path to the shoreline file");

  program.add_argument("--output-intersect")
      .default_value(dsas::options.intersect_path)
      .help("Path to save the intersection output");

  program.add_argument("--output-transect")
      .default_value(dsas::options.transect_path)
      .help("Path to save the generated transects");

  program.add_argument("--smooth-factor")
      .scan<'i', int>()
      .default_value(dsas::options.smooth_factor)
      .help("Smoothing factor for filtering");

  program.add_argument("--edge-distance")
      .scan<'i', int>()
      .default_value(dsas::options.edge_distance)
      .help("Minimum distance from edge");

  program.add_argument("--transect-length")
      .scan<'g', double>()
      .default_value(dsas::options.transect_length)
      .help("Length of transects");

  program.add_argument("--transect-spacing")
      .scan<'g', double>()
      .default_value(dsas::options.transect_spacing)
      .help("Spacing between transects");

  program.add_argument("--transect-offset")
      .scan<'g', double>()
      .default_value(dsas::options.transect_offset)
      .help("Offset distance for transects");

  program.add_argument("--intersection-mode")
      .default_value(std::string("closest"))
      .help("Intersection mode: closest or farthest");

  program.add_argument("--transect-orientation")
      .default_value(std::string("mix"))
      .help("Transect orientation: left, right, or mix");

  program.add_argument("-bi", "--build_index")
      .default_value(false)
      .implicit_value(true)
      .help("Build spatial index to speed up search");

  // Subcommand: cast
  argparse::ArgumentParser cast_cmd("cast");
  cast_cmd.add_description("Generate transects from a baseline.");
  cast_cmd.add_argument("--baseline")
          .required()
          .help("Path to the baseline file");
  cast_cmd.add_argument("--output-transect")
          .default_value(dsas::options.transect_path)
          .help("Path to save the generated transects");
  cast_cmd.add_argument("--transect-length")
          .scan<'g', double>()
          .default_value(dsas::options.transect_length)
          .help("Length of transects");
  cast_cmd.add_argument("--transect-spacing")
          .scan<'g', double>()
          .default_value(dsas::options.transect_spacing)
          .help("Spacing between transects");
  cast_cmd.add_argument("--transect-offset")
          .scan<'g', double>()
          .default_value(dsas::options.transect_offset)
          .help("Offset distance for transects");

  // Subcommand: cal
  argparse::ArgumentParser cal_cmd("cal");
  cal_cmd.add_description("Compute intersections and erosion rates.");
  cal_cmd.add_argument("--baseline")
         .required()
         .help("Path to the baseline file");
  cal_cmd.add_argument("--shoreline")
         .required()
         .help("Path to the shoreline file");
  cal_cmd.add_argument("--intersection-mode")
         .default_value(std::string("closest"))
         .help("Intersection mode: closest or farthest");
  cal_cmd.add_argument("--transect-orientation")
         .default_value(std::string("mix"))
         .help("Transect orientation: left, right, or mix");
  cal_cmd.add_argument("-bi", "--build_index")
         .default_value(false)
         .implicit_value(true)
         .help("Build spatial index to speed up search");
  cal_cmd.add_argument("--output-intersect")
         .default_value(dsas::options.intersect_path)
         .help("Path to save the intersection output");
  cal_cmd.add_argument("--output-transect")
         .default_value(dsas::options.transect_path)
         .help("Path to save the generated transects");
  cal_cmd.add_argument("--transect-length")
         .scan<'g', double>()
         .default_value(dsas::options.transect_length)
         .help("Length of transects");
  cal_cmd.add_argument("--transect-spacing")
         .scan<'g', double>()
         .default_value(dsas::options.transect_spacing)
         .help("Spacing between transects");
  cal_cmd.add_argument("--transect-offset")
         .scan<'g', double>()
         .default_value(dsas::options.transect_offset)
         .help("Offset distance for transects");
  cal_cmd.add_argument("--smooth-factor")
         .scan<'i', int>()
         .default_value(dsas::options.smooth_factor)
         .help("Smoothing factor for filtering");
  cal_cmd.add_argument("--edge-distance")
         .scan<'i', int>()
         .default_value(dsas::options.edge_distance)
         .help("Minimum distance from edge");

  program.add_subparser(cast_cmd);
  program.add_subparser(cal_cmd);

  try {
    program.parse_args(argc, argv);

    // Case 1: cast subcommand
    if (program.is_subcommand_used("cast")) {
      dsas::options.baseline_path    = cast_cmd.get<std::string>("--baseline");
      dsas::options.transect_path    = cast_cmd.get<std::string>("--output-transect");
      dsas::options.transect_length  = cast_cmd.get<double>("--transect-length");
      dsas::options.transect_spacing = cast_cmd.get<double>("--transect-spacing");
      dsas::options.transect_offset  = cast_cmd.get<double>("--transect-offset");
      run_cast();
      return 0;
    }

    // Case 2: cal subcommand
    if (program.is_subcommand_used("cal")) {
      dsas::options.baseline_path     = cal_cmd.get<std::string>("--baseline");
      dsas::options.shoreline_path    = cal_cmd.get<std::string>("--shoreline");
      dsas::options.intersect_path    = cal_cmd.get<std::string>("--output-intersect");
      dsas::options.transect_path     = cal_cmd.get<std::string>("--output-transect");
      dsas::options.transect_length   = cal_cmd.get<double>("--transect-length");
      dsas::options.transect_spacing  = cal_cmd.get<double>("--transect-spacing");
      dsas::options.transect_offset   = cal_cmd.get<double>("--transect-offset");
      dsas::options.smooth_factor     = cal_cmd.get<int>("--smooth-factor");
      dsas::options.edge_distance     = cal_cmd.get<int>("--edge-distance");
      dsas::options.intersection_mode =
          parse_intersection_mode(cal_cmd.get<std::string>("--intersection-mode"));
      dsas::options.transect_orient =
          parse_transect_orient(cal_cmd.get<std::string>("--transect-orientation"));
      dsas::options.build_index = cal_cmd.get<bool>("--build_index");
      run_cal();
      return 0;
    }

    // Case 3: no subcommand -> root parser flow
    dsas::options.baseline_path     = program.get<std::string>("--baseline");
    dsas::options.shoreline_path    = program.get<std::string>("--shoreline");
    dsas::options.intersect_path    = program.get<std::string>("--output-intersect");
    dsas::options.transect_path     = program.get<std::string>("--output-transect");
    dsas::options.smooth_factor     = program.get<int>("--smooth-factor");
    dsas::options.edge_distance     = program.get<int>("--edge-distance");
    dsas::options.transect_length   = program.get<double>("--transect-length");
    dsas::options.transect_spacing  = program.get<double>("--transect-spacing");
    dsas::options.transect_offset   = program.get<double>("--transect-offset");
    dsas::options.intersection_mode =
        parse_intersection_mode(program.get<std::string>("--intersection-mode"));
    dsas::options.transect_orient =
        parse_transect_orient(program.get<std::string>("--transect-orientation"));
    dsas::options.build_index = program.get<bool>("--build_index");

    run_root();
    return 0;

  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << "\n";
    // Show the most relevant help block
    if (program.is_subcommand_used("cast")) {
      std::cerr << cast_cmd;
    } else if (program.is_subcommand_used("cal")) {
      std::cerr << cal_cmd;
    } else {
      std::cerr << program;
    }
    return 1;
  }
}
