#include "cli.hpp"

#include <gtest/gtest.h>

#include "options.hpp"

using namespace dsas;

TEST(CLITest, test_parser_root) {
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
}

TEST(CLITest, test_parser_cast) {
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

TEST(CLITest, test_parser_cal) {
  char *args[] = {(char *)"dsas",
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