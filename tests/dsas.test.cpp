#include "dsas.hpp"
#include <gtest/gtest.h>
#include <algorithm>
using namespace dsas;

class DsasTest : public ::testing::Test {
 protected:
  std::vector<Baseline> baselines;
  std::vector<std::unique_ptr<TransectLine>> transects;
  std::vector<std::unique_ptr<Shoreline>> shorelines;

  void SetUp() override {
    options = Options();
    options.transect_length = 10;
    options.transect_spacing = 1;
    options.transect_offset = 0;
    options.smooth_factor = 1;
    options.intersection_mode = dsas::Options::IntersectionMode::Closest;

    BaselineSeg::cumulative_segment_distance = 0;
    BaselineSeg::cumulative_transects_distance = 0;
    std::vector<Point> points{{0, 0}, {1, 0}, {2, 0}, {3, 0}};
    Baseline baseline {points, 0};
    baselines.push_back(std::move(baseline));

    std::vector<Point> shore_vertices{{0, 1}, {1, 1}, {2, 1}, {3, 1}};
    boost::gregorian::date t_date(2000, 1, 1);
    auto shoreline = std::make_unique<Shoreline>(shore_vertices, 0, t_date);
    shorelines.push_back(std::move(shoreline));
  }

  void TearDown() override {
    options = Options();
    BaselineSeg::cumulative_segment_distance = 0;
    BaselineSeg::cumulative_transects_distance = 0;
  }
};


TEST_F(DsasTest, test_generate_transects){
  auto transects = generate_transects(baselines);
  ASSERT_EQ(transects.size(), 4);
}

TEST_F(DsasTest, test_generate_intersects_with_grids){
  auto transects = generate_transects(baselines);
  ASSERT_EQ(transects.size(), 4);

  auto grids = build_spatial_grids(shorelines, transects);
  
  auto intersects = generate_intersects(transects, grids);

  ASSERT_EQ(intersects.size(), 4);
}