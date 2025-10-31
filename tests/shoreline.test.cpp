#include "shoreline.hpp"

#include <gtest/gtest.h>
using namespace dsas;
#define TOL 1e-4

TEST(TestShoreline, test_load_shorelines_shp) {
  const std::filesystem::path shoreline_shp_path{std::string(TEST_DATA_DIR) +
                                                 "/sample_shorelines.geojson"};
  load_shorelines_shp(shoreline_shp_path, "date");
}