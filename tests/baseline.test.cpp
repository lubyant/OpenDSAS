#include "baseline.hpp"

#include <gtest/gtest.h>

#include "options.hpp"

using namespace dsas;
constexpr double TOL = 1e-4;

TEST(BaselineSegTest, test_transects) {
  {
    Point left{0, 0};
    Point right{10, 0};
    BaselineSeg baselineSeg{1, left, right};

    auto transects_base_points = baselineSeg.transects_base_points_;

    ASSERT_EQ(transects_base_points.size(), 10);
  }
  {
    Point left{0, 0};
    Point right{10, 10};
    BaselineSeg baselineSeq{1, left, right};

    auto transects_base_points = baselineSeq.transects_base_points_;

    const int num = 14;
    double x[]{0.7071067811865475, 1.414213562373095,  2.1213203435596424,
               2.82842712474619,   3.5355339059327373, 4.242640687119285,
               4.949747468305832,  5.65685424949238,   6.363961030678928,
               7.071067811865475,  7.778174593052022,  8.48528137423857,
               9.192388155425117,  9.899494936611664};
    double y[]{0.7071067811865475, 1.414213562373095,  2.1213203435596424,
               2.82842712474619,   3.5355339059327373, 4.242640687119285,
               4.949747468305832,  5.65685424949238,   6.363961030678928,
               7.071067811865475,  7.778174593052022,  8.48528137423857,
               9.192388155425117,  9.899494936611664};

    ASSERT_EQ(transects_base_points.size(), num);

    for (int i = 0; i < num; i++) {
      ASSERT_NEAR(x[i], transects_base_points[i].x, TOL);
      ASSERT_NEAR(y[i], transects_base_points[i].y, TOL);
    }
  }
}

class BaselineTest : public ::testing::Test {
 protected:
  std::unique_ptr<Baseline> baseline;

  void SetUp() override {
    options = Options();
    BaselineSeg::cumulative_segment_distance = 0;
    BaselineSeg::cumulative_transects_distance = 0;
  }

  void TearDown() override {
    options = Options();
    BaselineSeg::cumulative_segment_distance = 0;
    BaselineSeg::cumulative_transects_distance = 0;
  }
};

TEST_F(BaselineTest, test_baseline_transect_points1) {
  const std::vector<Point> points{{0, 0}, {0, 1}, {1, 1}, {1, 0}};

  options.transect_length = 10;
  options.transect_spacing = 0.5;
  options.smooth_factor = 1;
  options.intersection_mode = dsas::Options::IntersectionMode::Closest;
  baseline = std::make_unique<Baseline>(points, 0);

  auto transects_points = baseline->transects_base_points_;

  ASSERT_EQ(transects_points.size(), 7);
  double x[]{0, 0, 0, 0.5, 1, 1, 1};
  double y[]{0, 0.5, 1, 1, 1, 0.5, 0};

  for (size_t i = 0; i < transects_points.size(); i++) {
    ASSERT_NEAR(x[i], transects_points[i].x, TOL);
    ASSERT_NEAR(y[i], transects_points[i].y, TOL);
  }
}

TEST_F(BaselineTest, test_baseline_transect_points2) {
  options.transect_length = 10;
  options.transect_spacing = 1.5;
  options.smooth_factor = 1;
  options.intersection_mode = dsas::Options::IntersectionMode::Closest;
  std::vector<Point> points{{0, 0}, {0, 1}, {1, 1}, {1, 0}};
  baseline = std::make_unique<Baseline>(points, 0);
  auto transects_points = baseline->transects_base_points_;

  ASSERT_EQ(transects_points.size(), 3);
  double x[]{0, 0.5, 1};
  double y[]{0, 1, 0};
  ASSERT_NEAR(x[0], transects_points[0].x, TOL);
  ASSERT_NEAR(x[1], transects_points[1].x, TOL);
  ASSERT_NEAR(y[0], transects_points[0].y, TOL);
  ASSERT_NEAR(y[1], transects_points[1].y, TOL);
}

TEST(BaselineLoadTest, test_load_baselines_shp) {
  {
    const std::filesystem::path baseline_shp_path{
        std::string(TEST_DATA_DIR) + "/sample_baseline_offshore.geojson"};
    auto ret = load_baselines_shp(baseline_shp_path);
    ASSERT_TRUE(!ret.empty());
  }
  {
    const std::filesystem::path baseline_shp_path{
        std::string(TEST_DATA_DIR) + "/sample_baseline_offshore.geojson"};
    auto baseline_id_field = "MissingField";
    ASSERT_THROW(load_baselines_shp(baseline_shp_path, baseline_id_field), std::runtime_error);
  }
}