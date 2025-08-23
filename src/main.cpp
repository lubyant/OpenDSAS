#include <argparse/argparse.hpp>

#include "dsas.hpp"
#include "grid.hpp"
#include "intersect.hpp"
#include "options.hpp"
#include "utility.hpp"

static void parse_args(int argc, char *argv[]) {
  argparse::ArgumentParser program(PROJECT_NAME_STR, APP_VERSION);

  program.add_argument("--baseline")
      .required()
      .help("Path to the baseline file");

  program.add_argument("--shoreline")
      .required()
      .help("Path to the shoreline file");

  // Optional parameters with defaults
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

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  // positional args
  dsas::options.baseline_path = program.get<std::string>("--baseline");
  dsas::options.shoreline_path = program.get<std::string>("--shoreline");
  dsas::options.intersect_path = program.get<std::string>("--output-intersect");
  dsas::options.transect_path = program.get<std::string>("--output-transect");

  // some optional args
  dsas::options.smooth_factor = program.get<int>("--smooth-factor");
  dsas::options.edge_distance = program.get<int>("--edge-distance");
  dsas::options.transect_length = program.get<double>("--transect-length");
  dsas::options.transect_spacing = program.get<double>("--transect-spacing");
  dsas::options.transect_offset = program.get<double>("--transect-offset");

  std::string s = program.get<std::string>("--intersection-mode");
  if (s == "closest")
    dsas::options.intersection_mode = dsas::Options::IntersectionMode::Closest;
  if (s == "farthest")
    dsas::options.intersection_mode = dsas::Options::IntersectionMode::Farthest;

  s = program.get<std::string>("--transect-orientation");
  if (s == "left")
    dsas::options.transect_orient = dsas::Options::TransectOrientation::Left;
  if (s == "right")
    dsas::options.transect_orient = dsas::Options::TransectOrientation::Right;
  if (s == "mix")
    dsas::options.transect_orient = dsas::Options::TransectOrientation::Mix;

  dsas::options.build_index = program.get<bool>("--build_index");
}

static void cal_erosion_rate(std::vector<dsas::TransectLine> &transects) {
  for (auto &transect : transects) {
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

static void run() {
  print_messages();
  auto baselines = dsas::load_baselines_shp(dsas::options.baseline_path, "Id");
  auto shorelines =
      dsas::load_shorelines_shp(dsas::options.shoreline_path, "Date_");
  auto transects = dsas::generate_transects(baselines);

  std::vector<std::unique_ptr<dsas::IntersectPoint>> intersects;
  if (dsas::options.build_index) {
    auto grids = dsas::build_spatial_grids(shorelines, transects);
    intersects = dsas::generate_intersects(transects, grids);
  } else {
    intersects = dsas::generate_intersects(transects, shorelines);
  }

  // compute regression rate
  for (auto &transect : transects) {
    if (!(transect->intersects.empty())) {
      auto reg_rate = dsas::linearRegressRate(transect->intersects);
      transect->change_rate = reg_rate;
    }
  }

  // read prj
  auto prj = dsas::get_shp_proj(dsas::options.shoreline_path.c_str());

  // save the output
  dsas::save_transect(transects, prj);
  dsas::save_intersects(intersects, prj);
}

int main(int argc, char *argv[]) {
  parse_args(argc, argv);
  run();
  return 0;
}
