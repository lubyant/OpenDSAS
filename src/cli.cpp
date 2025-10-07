#include "cli.hpp"

#include "options.hpp"

namespace dsas {

// --------------------------- enum mappers -----------------------------

static dsas::Options::IntersectionMode parse_intersection_mode(
    const std::string& s) {
  if (s == "closest") return dsas::Options::IntersectionMode::Closest;
  if (s == "farthest") return dsas::Options::IntersectionMode::Farthest;
  throw std::runtime_error("Invalid --intersection-mode: " + s);
}

static dsas::Options::TransectOrientation parse_transect_orient(
    const std::string& s) {
  if (s == "left") return dsas::Options::TransectOrientation::Left;
  if (s == "right") return dsas::Options::TransectOrientation::Right;
  if (s == "mix") return dsas::Options::TransectOrientation::Mix;
  throw std::runtime_error("Invalid --transect-orientation: " + s);
}

argparse::ArgumentParser init_root() {
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

  program.add_argument("--transect-length")
      .scan<'g', double>()
      .default_value(dsas::options.transect_length)
      .help("Length of transects");

  program.add_argument("--transect-spacing")
      .scan<'g', double>()
      .default_value(dsas::options.transect_spacing)
      .help("Spacing between transects");

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
  return program;
}

argparse::ArgumentParser init_cast() {
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
  return cast_cmd;
}

argparse__ArgumentParser init_cal() {
  // Subcommand: cal
  argparse::ArgumentParser cal_cmd("cal");
  cal_cmd.add_description(
      "Compute intersections and erosion rates by given transects and "
      "shoreline.");
  cal_cmd.add_argument("--transect")
      .required()
      .help("Path to the transeect file");
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
  return cal_cmd;
}

void parse_args(int argc, char* argv[]) {
  auto program = init_root();
  auto cast_cmd = init_cast();
  auto cal_cmd = init_cal();

  program.add_subparser(cast_cmd);
  program.add_subparser(cal_cmd);

  try {
    program.parse_args(argc, argv);

    if (program.is_subcommand_used("cast")) {
      dsas::options.baseline_path = cast_cmd.get<std::string>("--baseline");
      dsas::options.transect_path =
          cast_cmd.get<std::string>("--output-transect");
      dsas::options.transect_length = cast_cmd.get<double>("--transect-length");
      dsas::options.transect_spacing =
          cast_cmd.get<double>("--transect-spacing");
      return CliState::cast;
    }
    if (program.is_subcommand_used("cal")) {
      dsas::options.transect_length = cal_cmd.get<std::string>("--transect");
      dsas::options.shoreline_path = cal_cmd.get<std::string>("--shoreline");
      dsas::options.intersect_path =
          cal_cmd.get<std::string>("--output-intersect");
      dsas::options.intersection_mode = parse_intersection_mode(
          cal_cmd.get<std::string>("--intersection-mode"));
      dsas::options.transect_orient = parse_transect_orient(
          cal_cmd.get<std::string>("--transect-orientation"));
      dsas::options.build_index = cal_cmd.get<bool>("--build_index");
      return CliState::cast;
    }
    dsas::options.baseline_path = program.get<std::string>("--baseline");
    dsas::options.shoreline_path = program.get<std::string>("--shoreline");
    dsas::options.intersect_path =
        program.get<std::string>("--output-intersect");
    dsas::options.transect_path = program.get<std::string>("--output-transect");
    dsas::options.smooth_factor = program.get<int>("--smooth-factor");
    dsas::options.transect_length = program.get<double>("--transect-length");
    dsas::options.transect_spacing = program.get<double>("--transect-spacing");
    dsas::options.intersection_mode = parse_intersection_mode(
        program.get<std::string>("--intersection-mode"));
    dsas::options.transect_orient = parse_transect_orient(
        program.get<std::string>("--transect-orientation"));
    dsas::options.build_index = program.get<bool>("--build_index");
    return CliState::root;
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }
}
}  // namespace dsas