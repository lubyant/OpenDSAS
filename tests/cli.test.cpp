#include "cli.hpp"

#include <gtest/gtest.h>

#include "options.hpp"

using namespace dsas;

struct CLITest : public ::testing::Test {
  void SetUp() override { options = Options{}; }
};

TEST_F(CLITest, test_parser_root) {
  {
    char *args[] = {(char *)"dsas",
                    (char *)"--baseline",
                    (char *)"base.shp",
                    (char *)"--shoreline",
                    (char *)"shore.shp",
                    (char *)"--intersection-mode",
                    (char *)"closest",
                    (char *)"--transect-orientation",
                    (char *)"mix",
                    (char *)"-bi",
                    (char *)"--transect-length",
                    (char *)"200",
                    (char *)"--transect-spacing",
                    (char *)"50"};

    parse_args(sizeof(args) / sizeof(args[0]), args);
    EXPECT_EQ(options.baseline_path, "base.shp");
    EXPECT_EQ(options.shoreline_path, "shore.shp");
    EXPECT_EQ(options.intersection_mode, Options::IntersectionMode::Closest);
    EXPECT_EQ(options.transect_orient, Options::TransectOrientation::Mix);
    EXPECT_EQ(options.build_index, true);
    EXPECT_EQ(options.transect_length, 200);
    EXPECT_EQ(options.transect_spacing, 50);
    EXPECT_EQ(options.smooth_factor, 1);
  }
  {
    char *args[] = {(char *)"dsas",      (char *)"--baseline",
                    (char *)"base.shp",  (char *)"--shoreline",
                    (char *)"shore.shp", (char *)"--smooth-factor",
                    (char *)"0"};

    EXPECT_EXIT(parse_args(sizeof(args) / sizeof(args[0]), args),
                ::testing::ExitedWithCode(1),
                "Error: smooth factor is less than 1");
  }
}

TEST_F(CLITest, test_parser_cast) {
  {
    char *args[] = {(char *)"dsas",
                    (char *)"cast",
                    (char *)"--baseline",
                    (char *)"base.shp",
                    (char *)"--output-transect",
                    (char *)"trans.shp",
                    (char *)"--transect-orientation",
                    (char *)"mix",
                    (char *)"--transect-length",
                    (char *)"200",
                    (char *)"--transect-spacing",
                    (char *)"50"};

    parse_args(sizeof(args) / sizeof(args[0]), args);
    EXPECT_EQ(options.baseline_path, "base.shp");
    EXPECT_EQ(options.transect_path, "trans.shp");
    EXPECT_EQ(options.intersection_mode, Options::IntersectionMode::Closest);
    EXPECT_EQ(options.transect_orient, Options::TransectOrientation::Mix);
    EXPECT_EQ(options.build_index, false);
    EXPECT_EQ(options.transect_length, 200);
    EXPECT_EQ(options.transect_spacing, 50);
  }
  {
    char *args[] = {(char *)"dsas",
                    (char *)"cast",
                    (char *)"--baseline",
                    (char *)"base.shp",
                    (char *)"--output-transect",
                    (char *)"--smooth-factor",
                    (char *)"0",
                    (char *)"--transect-spacing",
                    (char *)"50"};

    EXPECT_EXIT(parse_args(sizeof(args) / sizeof(args[0]), args),
                ::testing::ExitedWithCode(1),
                "Error: smooth factor is less than 1");
  }
}
TEST_F(CLITest, test_parser_cal) {
  char *args[] = {
      (char *)"dsas",
      (char *)"cal",
      (char *)"--transect",
      (char *)"trans.shp",
      (char *)"--shoreline",
      (char *)"shores.shp",
      (char *)"--intersection-mode",
      (char *)"closest",
      (char *)"--transect-orientation",
      (char *)"mix",
      (char *)"-bi",
  };

  parse_args(sizeof(args) / sizeof(args[0]), args);
  EXPECT_EQ(options.shoreline_path, "shores.shp");
  EXPECT_EQ(options.transect_path, "trans.shp");
  EXPECT_EQ(options.intersection_mode, Options::IntersectionMode::Closest);
  EXPECT_EQ(options.transect_orient, Options::TransectOrientation::Mix);
  EXPECT_EQ(options.build_index, true);
}

TEST_F(CLITest, test_format_consistency) {
  // cast: GeoJSON input, Shapefile output → error
  {
    char *args[] = {(char *)"dsas",         (char *)"cast",
                    (char *)"--baseline",   (char *)"base.geojson",
                    (char *)"--output-transect", (char *)"trans.shp"};
    EXPECT_EXIT(parse_args(sizeof(args) / sizeof(args[0]), args),
                ::testing::ExitedWithCode(1), "Format mismatch");
  }
  // root: Shapefile inputs, GeoJSON transect output → error
  {
    char *args[] = {(char *)"dsas",
                    (char *)"--baseline",        (char *)"base.shp",
                    (char *)"--shoreline",        (char *)"shore.shp",
                    (char *)"--output-transect",  (char *)"trans.geojson"};
    EXPECT_EXIT(parse_args(sizeof(args) / sizeof(args[0]), args),
                ::testing::ExitedWithCode(1), "Format mismatch");
  }
  // cal: GeoJSON inputs, Shapefile intersect output → error
  {
    char *args[] = {(char *)"dsas",
                    (char *)"cal",
                    (char *)"--transect",         (char *)"trans.geojson",
                    (char *)"--shoreline",         (char *)"shore.geojson",
                    (char *)"--output-intersect",  (char *)"out.shp"};
    EXPECT_EXIT(parse_args(sizeof(args) / sizeof(args[0]), args),
                ::testing::ExitedWithCode(1), "Format mismatch");
  }
  // Consistent formats pass without error
  {
    char *args[] = {(char *)"dsas",         (char *)"cast",
                    (char *)"--baseline",   (char *)"base.geojson",
                    (char *)"--output-transect", (char *)"trans.geojson"};
    auto status = parse_args(sizeof(args) / sizeof(args[0]), args);
    EXPECT_EQ(status, CliStatus::Cast);
  }
  // .json extension is treated as GeoJSON (same as .geojson)
  {
    char *args[] = {(char *)"dsas",         (char *)"cast",
                    (char *)"--baseline",   (char *)"base.json",
                    (char *)"--output-transect", (char *)"trans.json"};
    auto status = parse_args(sizeof(args) / sizeof(args[0]), args);
    EXPECT_EQ(status, CliStatus::Cast);
  }
  // Unrecognised extensions are skipped; only known-format paths are compared
  {
    char *args[] = {(char *)"dsas",         (char *)"cast",
                    (char *)"--baseline",   (char *)"base.csv",
                    (char *)"--output-transect", (char *)"trans.shp"};
    auto status = parse_args(sizeof(args) / sizeof(args[0]), args);
    EXPECT_EQ(status, CliStatus::Cast);
  }
  // Empty (unspecified) paths are skipped
  {
    char *args[] = {(char *)"dsas",
                    (char *)"--shoreline",       (char *)"shore.shp",
                    (char *)"--output-transect", (char *)"trans.shp"};
    // --baseline is not provided, so baseline_path is "" — skipped in check
    auto status = parse_args(sizeof(args) / sizeof(args[0]), args);
    EXPECT_EQ(status, CliStatus::Root);
  }
}

TEST_F(CLITest, test_intersection_mode_farthest) {
  char *args[] = {(char *)"dsas",        (char *)"cal",
                  (char *)"--transect",  (char *)"trans.shp",
                  (char *)"--shoreline", (char *)"shores.shp",
                  (char *)"--intersection-mode", (char *)"farthest"};
  parse_args(sizeof(args) / sizeof(args[0]), args);
  EXPECT_EQ(options.intersection_mode, Options::IntersectionMode::Farthest);
}

TEST_F(CLITest, test_invalid_intersection_mode) {
  char *args[] = {(char *)"dsas",        (char *)"cal",
                  (char *)"--transect",  (char *)"trans.shp",
                  (char *)"--shoreline", (char *)"shores.shp",
                  (char *)"--intersection-mode", (char *)"bogus"};
  EXPECT_EXIT(parse_args(sizeof(args) / sizeof(args[0]), args),
              ::testing::ExitedWithCode(1), "Invalid --intersection-mode");
}

TEST_F(CLITest, test_invalid_transect_orientation) {
  char *args[] = {(char *)"dsas",
                  (char *)"cast",
                  (char *)"--baseline",       (char *)"base.shp",
                  (char *)"--output-transect", (char *)"trans.shp",
                  (char *)"--transect-orientation", (char *)"bogus"};
  EXPECT_EXIT(parse_args(sizeof(args) / sizeof(args[0]), args),
              ::testing::ExitedWithCode(1), "Invalid --transect-orientation");
}