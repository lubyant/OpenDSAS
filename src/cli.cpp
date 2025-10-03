#include "cli.hpp"

#include "options.hpp"

namespace dsas {

void parse_args(int argc, char *argv[]) {
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
}  // namespace dsas