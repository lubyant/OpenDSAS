#include "transect.hpp"

#include <gtest/gtest.h>

#include "grid.hpp"
#include "options.hpp"
using namespace dsas;
constexpr double TOL = 1e-4;

class TransectTest : public ::testing::Test {
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

TEST_F(TransectTest, test_baseline_transect_lines) {
  std::vector<Point> points{{0, 0}, {0, 1}, {1, 1}, {1, 0}};
  options.transect_length = 1;
  options.transect_spacing = 0.5;
  options.transect_offset = 0;
  options.smooth_factor = 0;
  options.intersection_mode = dsas::Options::IntersectionMode::Closest;
  baseline = std::make_unique<Baseline>(points, 0);

  auto transects_lines = create_transects_from_baseline(*baseline);

  ASSERT_EQ(transects_lines.size(), 7);
  double x[]{0.5, 0.5, 0.5, 0.5, 1, 0.5, 0.5};
  double y[]{0, 0.5, 1, 0.5, 0.5, 0.5, 0};

  for (size_t i = 0; i < transects_lines.size(); i++) {
    ASSERT_NEAR(x[i], transects_lines[i]->transect_ref_point_.x, TOL);
    ASSERT_NEAR(y[i], transects_lines[i]->transect_ref_point_.y, TOL);
  }
}

TEST_F(TransectTest, test_baseline_transect_lines_right) {
  std::vector<Point> points{{0, 0}, {0, 1}, {1, 1}, {1, 0}};
  options.transect_length = 1;
  options.transect_spacing = 0.5;
  options.transect_offset = 0;
  options.smooth_factor = 0;
  options.intersection_mode = dsas::Options::IntersectionMode::Closest;
  options.transect_orient = dsas::Options::TransectOrientation::Right;
  baseline = std::make_unique<Baseline>(points, 0);
  auto transects_lines = create_transects_from_baseline(*baseline);

  // assert if the number of transects is correct
  ASSERT_EQ(transects_lines.size(), 7);

  // assert if the transect edge is correct
  double x_right[]{1, 1, 1, 0.5, 1, 0, 0};
  double y_right[]{0, 0.5, 1, 0, 0, 0.5, 0};
  double x_left[]{0, 0, 0, 0.5, 1, 1, 1};
  double y_left[]{0, 0.5, 1, 1, 1, 0.5, 0};
  for (size_t i = 0; i < transects_lines.size(); i++) {
    ASSERT_NEAR(x_right[i], transects_lines[i]->rightEdge_.x, TOL);
    ASSERT_NEAR(y_right[i], transects_lines[i]->rightEdge_.y, TOL);
    ASSERT_NEAR(x_left[i], transects_lines[i]->leftEdge_.x, TOL);
    ASSERT_NEAR(y_left[i], transects_lines[i]->leftEdge_.y, TOL);
  }
}

TEST_F(TransectTest, test_baseline_transect_lines_mix) {
  std::vector<Point> points{{0, 0}, {0, 1}, {1, 1}, {1, 0}};
  options.transect_length = 1;
  options.transect_spacing = 0.5;
  options.transect_offset = 0;
  options.smooth_factor = 0;
  options.intersection_mode = dsas::Options::IntersectionMode::Closest;
  options.transect_orient = dsas::Options::TransectOrientation::Mix;

  baseline = std::make_unique<Baseline>(points, 0);
  auto transects_lines = create_transects_from_baseline(*baseline);

  // assert if the number of transects is correct
  ASSERT_EQ(transects_lines.size(), 7);

  // assert if the transect edge is correct
  double x_right[]{0.5, 0.5, 0.5, 0.5, 1, 0.5, 0.5};
  double y_right[]{0, 0.5, 1, 0.5, 0.5, 0.5, 0};
  double x_left[]{-0.5, -0.5, -0.5, 0.5, 1, 1.5, 1.5};
  double y_left[]{0, 0.5, 1, 1.5, 1.5, 0.5, 0};
  for (size_t i = 0; i < transects_lines.size(); i++) {
    ASSERT_NEAR(x_right[i], transects_lines[i]->rightEdge_.x, TOL);
    ASSERT_NEAR(y_right[i], transects_lines[i]->rightEdge_.y, TOL);
    ASSERT_NEAR(x_left[i], transects_lines[i]->leftEdge_.x, TOL);
    ASSERT_NEAR(y_left[i], transects_lines[i]->leftEdge_.y, TOL);
  }
}

TEST_F(TransectTest, test_baseline_transect_lines_left) {
  std::vector<Point> points{{0, 0}, {0, 1}, {1, 1}, {1, 0}};
  options.transect_length = 1;
  options.transect_spacing = 0.5;
  options.transect_offset = 0;
  options.smooth_factor = 0;
  options.intersection_mode = dsas::Options::IntersectionMode::Closest;
  options.transect_orient = dsas::Options::TransectOrientation::Left;
  baseline = std::make_unique<Baseline>(points, 0);
  auto transects_lines = create_transects_from_baseline(*baseline);

  ASSERT_EQ(transects_lines.size(), 7);

  double x_left[]{-1, -1, -1, 0.5, 1, 2, 2};
  double y_left[]{0, 0.5, 1, 2, 2, 0.5, 0};
  double x_right[]{0, 0, 0, 0.5, 1, 1, 1};
  double y_right[]{0, 0.5, 1, 1, 1, 0.5, 0};

  for (size_t i = 0; i < transects_lines.size(); i++) {
    ASSERT_NEAR(x_right[i], transects_lines[i]->rightEdge_.x, TOL);
    ASSERT_NEAR(y_right[i], transects_lines[i]->rightEdge_.y, TOL);
    ASSERT_NEAR(x_left[i], transects_lines[i]->leftEdge_.x, TOL);
    ASSERT_NEAR(y_left[i], transects_lines[i]->leftEdge_.y, TOL);
  }
}

TEST_F(TransectTest, test_baseline_transect_length1) {
  std::vector<Point> points{{0, 0}, {0, 1}, {1, 1}, {1, 0}};
  options.transect_length = 1;
  options.transect_spacing = 0.5;
  options.transect_offset = 1;
  options.smooth_factor = 0;
  options.intersection_mode = dsas::Options::IntersectionMode::Closest;
  baseline = std::make_unique<Baseline>(points, 0);
  auto transects_lines = create_transects_from_baseline(*baseline);
  for (const auto &transect : transects_lines) {
    ASSERT_NEAR(transect->leftEdge_.distance_to_point(transect->rightEdge_), 1,
                TOL);
  }
}

TEST_F(TransectTest, test_baseline_transect_length2) {
  std::vector<Point> points{{0, 0}, {1, 1}, {2, 0}, {3, 1}};
  options.transect_length = 1;
  options.transect_spacing = 0.5;
  options.transect_offset = 1;
  options.smooth_factor = 0;
  options.intersection_mode = dsas::Options::IntersectionMode::Closest;
  baseline = std::make_unique<Baseline>(points, 0);
  auto transects_lines = create_transects_from_baseline(*baseline);
  for (const auto &transect : transects_lines) {
    ASSERT_NEAR(transect->leftEdge_.distance_to_point(transect->rightEdge_), 1,
                TOL);
  }
}

TEST_F(TransectTest, test_baseline_smooth) {
  // TODO: need more test cases
  {
    int smooth_factor{1};
    std::vector<Point> points{{0, 0}, {1, 1}, {2, 0}, {3, 1}};
    options.transect_length = 1;
    options.transect_spacing = 0.5;
    options.transect_offset = 1;
    options.smooth_factor = 1;
    options.intersection_mode = dsas::Options::IntersectionMode::Closest;
    baseline = std::make_unique<Baseline>(points, 0);
    auto transects_lines = create_transects_from_baseline(*baseline);
    ASSERT_EQ(baseline->origin_vertices_.size(), 4);
    double slope{transects_lines[0]->normal_vector_.first /
                 transects_lines[0]->normal_vector_.second};
    ASSERT_NEAR(slope, 1, TOL);
  }
  {
    int smooth_factor{2};
    std::vector<Point> points{{0, 0}, {1, 1}, {2, 0}, {3, 1},
                              {3, 0}, {4, 1}, {4, 0}};
    options.transect_length = 1;
    options.transect_spacing = 0.5;
    options.transect_offset = 1;
    options.smooth_factor = 2;
    options.intersection_mode = dsas::Options::IntersectionMode::Closest;
    baseline = std::make_unique<Baseline>(points, 0);
    auto transects_lines = create_transects_from_baseline(*baseline);
    ASSERT_EQ(baseline->origin_vertices_.size(), 7);
  }
  {
    int smooth_factor{8};
    std::vector<Point> points{{0, 0}, {1, 1}, {2, 0}, {3, 1},
                              {3, 0}, {4, 1}, {4, 0}};
    options.transect_length = 1;
    options.transect_spacing = 0.5;
    options.transect_offset = 1;
    options.smooth_factor = 8;
    options.intersection_mode = dsas::Options::IntersectionMode::Closest;
    baseline = std::make_unique<Baseline>(points, 0);
    auto transects_lines = create_transects_from_baseline(*baseline);
    ASSERT_EQ(baseline->origin_vertices_.size(), 7);
  }
}
