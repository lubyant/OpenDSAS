#include "dsas.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <boost/date_time/gregorian/greg_month.hpp>
#include <memory>
#include <stdexcept>

#include "intersect.hpp"
constexpr double TOL = 1e-4;
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
    options.smooth_factor = 1;
    options.intersection_mode = dsas::Options::IntersectionMode::Closest;

    BaselineSeg::cumulative_segment_distance = 0;
    BaselineSeg::cumulative_transects_distance = 0;
    std::vector<Point> points{{0, 0}, {1, 0}, {2, 0}, {3, 0}};
    Baseline baseline{points, 0};
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

TEST_F(DsasTest, test_generate_transects) {
  auto transects = generate_transects(baselines);
  ASSERT_EQ(transects.size(), 4);
}

TEST_F(DsasTest, test_generate_intersects_with_grids) {
  auto transects = generate_transects(baselines);
  ASSERT_EQ(transects.size(), 4);

  auto grids = build_spatial_grids(shorelines, transects);

  auto intersects = generate_intersects(transects, grids);

  ASSERT_EQ(intersects.size(), 4);
}

TEST_F(DsasTest, test_generate_intersects_without_grids) {
  auto transects = generate_transects(baselines);
  ASSERT_EQ(transects.size(), 4);

  auto intersects = generate_intersects(transects, shorelines);

  ASSERT_EQ(intersects.size(), 4);
}

TEST_F(DsasTest, test_linearRegressionRate) {
  // it won't used for rate
  Point fake_point{0, 0};
  int fake_tid{0};
  int fake_bid{0};
  int fake_sid{0};

  boost::gregorian::date date1{
      2000, boost::gregorian::greg_month::month_enum::Jan, 1};
  boost::gregorian::date date2{
      2010, boost::gregorian::greg_month::month_enum::Jan, 1};
  boost::gregorian::date date3{
      2020, boost::gregorian::greg_month::month_enum::Jan, 1};
  double dist2ref1 = 0;
  double dist2ref2 = 1;
  double dist2ref3 = 2;

  {
    std::vector<IntersectPoint *> intersections;
    IntersectPoint p1{fake_point, fake_tid, fake_sid,
                      fake_bid,   date1,    dist2ref1};
    IntersectPoint p2{fake_point, fake_tid, fake_sid,
                      fake_bid,   date2,    dist2ref2};
    IntersectPoint p3{fake_point, fake_tid, fake_sid,
                      fake_bid,   date3,    dist2ref3};

    intersections.push_back(&p1);
    intersections.push_back(&p2);
    intersections.push_back(&p3);

    double ans = linearRegressRate(intersections);

    ASSERT_NEAR(ans, 0.1, TOL);
  }

  {
    std::vector<IntersectPoint *> intersections;
    IntersectPoint p1{fake_point, fake_tid, fake_sid,
                      fake_bid,   date1,    dist2ref1};
    IntersectPoint p2{fake_point, fake_tid, fake_sid,
                      fake_bid,   date2,    dist2ref2};

    intersections.push_back(&p1);
    intersections.push_back(&p2);

    double ans = linearRegressRate(intersections);

    ASSERT_NEAR(ans, 0.1, TOL);
  }

  {
    std::vector<IntersectPoint *> intersections;
    IntersectPoint p1{fake_point, fake_tid, fake_sid,
                      fake_bid,   date1,    dist2ref1};

    intersections.push_back(&p1);

    double ans = linearRegressRate(intersections);

    ASSERT_NEAR(ans, 0, TOL);
  }

  {
    std::vector<IntersectPoint *> intersections;
    EXPECT_THROW(linearRegressRate(intersections), std::runtime_error);
  }

  {
    std::vector<IntersectPoint *> intersections;
    IntersectPoint p1{fake_point, fake_tid, fake_sid,
                      fake_bid,   date1,    dist2ref1};
    IntersectPoint p2{fake_point, fake_tid, fake_sid,
                      fake_bid,   date2,    dist2ref2};
    IntersectPoint p3{fake_point, fake_tid, fake_sid,
                      fake_bid,   date2,    dist2ref3};

    intersections.push_back(&p1);
    intersections.push_back(&p2);
    intersections.push_back(&p3);

    EXPECT_THROW(linearRegressRate(intersections), std::runtime_error);
  }
}