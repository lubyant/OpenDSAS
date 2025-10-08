#include "cli.hpp"

#include "options.hpp"

namespace dsas {

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

static void init_root_cmd(argparse::ArgumentParser& root_cmd) {
  root_cmd.add_argument("--baseline")
      .help("Path to the baseline file")
      .default_value(std::string{});

  root_cmd.add_argument("--shoreline")
      .help("Path to the shoreline file")
      .default_value(std::string{});

  // Optional parameters with defaults
  root_cmd.add_argument("--output-intersect")
      .default_value(dsas::options.intersect_path)
      .help("Path to save the intersection output");

  root_cmd.add_argument("--output-transect")
      .default_value(dsas::options.transect_path)
      .help("Path to save the generated transects");

  root_cmd.add_argument("--smooth-factor")
      .scan<'i', int>()
      .default_value(dsas::options.smooth_factor)
      .help("Smoothing factor for filtering");

  root_cmd.add_argument("--transect-length")
      .scan<'g', double>()
      .default_value(dsas::options.transect_length)
      .help("Length of transects");

  root_cmd.add_argument("--transect-spacing")
      .scan<'g', double>()
      .default_value(dsas::options.transect_spacing)
      .help("Spacing between transects");

  root_cmd.add_argument("--intersection-mode")
      .default_value(std::string("closest"))
      .help("Intersection mode: closest or farthest");

  root_cmd.add_argument("--transect-orientation")
      .default_value(std::string("mix"))
      .help("Transect orientation: left, right, or mix");

  root_cmd.add_argument("-bi", "--build_index")
      .default_value(false)
      .implicit_value(true)
      .help("Build spatial index to speed up search");
}

static void init_cast_cmd(argparse::ArgumentParser& cast_cmd) {
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
  cast_cmd.add_argument("--smooth-factor")
      .scan<'i', int>()
      .default_value(dsas::options.smooth_factor)
      .help("Smoothing factor for filtering");
  cast_cmd.add_argument("--intersection-mode")
      .default_value(std::string("closest"))
      .help("Intersection mode: closest or farthest");
  cast_cmd.add_argument("--transect-orientation")
      .default_value(std::string("mix"))
      .help("Transect orientation: left, right, or mix");
}

static void init_cal_cmd(argparse::ArgumentParser& cal_cmd) {
  cal_cmd.add_description(
      "Generate intersects and calculate erosion rate from shoreline and "
      "transect.");
  cal_cmd.add_argument("--transect")
      .required()
      .help("Path to the transect file");
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
}

CliStatus parse_args(int argc, char* argv[]) {
  argparse::ArgumentParser root_cmd(PROJECT_NAME_STR, APP_VERSION);
  init_root_cmd(root_cmd);

  argparse::ArgumentParser cast_cmd("cast");
  init_cast_cmd(cast_cmd);

  argparse::ArgumentParser cal_cmd("cal");
  init_cal_cmd(cal_cmd);

  root_cmd.add_subparser(cast_cmd);
  root_cmd.add_subparser(cal_cmd);

  try {
    root_cmd.parse_args(argc, argv);
    if (root_cmd.is_subcommand_used("cast")) {
      dsas::options.baseline_path = cast_cmd.get<std::string>("--baseline");
      dsas::options.transect_path =
          cast_cmd.get<std::string>("--output-transect");
      dsas::options.transect_length = cast_cmd.get<double>("--transect-length");
      dsas::options.transect_spacing =
          cast_cmd.get<double>("--transect-spacing");
      dsas::options.smooth_factor = cast_cmd.get<int>("--smooth-factor");
      dsas::options.intersection_mode = parse_intersection_mode(
          cast_cmd.get<std::string>("--intersection-mode"));
      dsas::options.transect_orient = parse_transect_orient(
          cast_cmd.get<std::string>("--transect-orientation"));
      return CliStatus::Cast;
    }

    if (root_cmd.is_subcommand_used("cal")) {
      dsas::options.shoreline_path = cal_cmd.get<std::string>("--shoreline");
      dsas::options.transect_path = cal_cmd.get<std::string>("--transect");
      dsas::options.intersection_mode = parse_intersection_mode(
          cal_cmd.get<std::string>("--intersection-mode"));
      dsas::options.transect_orient = parse_transect_orient(
          cal_cmd.get<std::string>("--transect-orientation"));
      dsas::options.build_index = cal_cmd.get<bool>("--build_index");
      return CliStatus::Cal;
    }

    dsas::options.baseline_path = root_cmd.get<std::string>("--baseline");
    dsas::options.shoreline_path = root_cmd.get<std::string>("--shoreline");
    dsas::options.intersect_path =
        root_cmd.get<std::string>("--output-intersect");
    dsas::options.transect_path =
        root_cmd.get<std::string>("--output-transect");
    dsas::options.smooth_factor = root_cmd.get<int>("--smooth-factor");
    dsas::options.transect_length = root_cmd.get<double>("--transect-length");
    dsas::options.transect_spacing = root_cmd.get<double>("--transect-spacing");
    dsas::options.intersection_mode = parse_intersection_mode(
        root_cmd.get<std::string>("--intersection-mode"));
    dsas::options.transect_orient = parse_transect_orient(
        root_cmd.get<std::string>("--transect-orientation"));
    dsas::options.build_index = root_cmd.get<bool>("--build_index");
    return CliStatus::Root;
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::exit(1);
  }
}
}  // namespace dsas