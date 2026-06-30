//
// Created by lby on 10/21/23.
//
#include "geometry.hpp"

#include <gtest/gtest.h>

#include <sstream>

#include "options.hpp"

#define TOL 1e-4
using namespace dsas;

TEST(PointTest, test_move) {
  {
    Point point{0, 0};
    point.move_point({1, 1}, 1);
    ASSERT_NEAR(point.x, sqrt(2) / 2, TOL);
    ASSERT_NEAR(point.y, sqrt(2) / 2, TOL);
  }
  {
    Point point{0, 0};
    point.move_point({0, 1}, 1);
    ASSERT_NEAR(point.x, 1, TOL);
    ASSERT_NEAR(point.y, 0, TOL);
  }
}

TEST(LineTest, test_slope) {
  Point left{0, 0};
  Point right{1, 1};
  LineSegment lineSegment{left, right};
  ASSERT_NEAR(lineSegment.orient_, (double)45 / 180 * PI, TOL);
}

TEST(LineTest, test_move) {
  Point left{0, 0};
  Point right{1, 1};
  LineSegment lineSegment{left, right};
  lineSegment.move_line(1);
  ASSERT_NEAR(lineSegment.leftEdge_.x, -(double)sqrt(2) / 2, TOL);
  ASSERT_NEAR(lineSegment.leftEdge_.y, (double)sqrt(2) / 2, TOL);
  ASSERT_NEAR(lineSegment.rightEdge_.x, (double)(1 - sqrt(2) / 2), TOL);
  ASSERT_NEAR(lineSegment.rightEdge_.y, (double)(1 + sqrt(2) / 2), TOL);
}

TEST(PointTest, test_stream_output) {
  Point p{3.0, 4.0};
  std::ostringstream oss;
  oss << p;
  ASSERT_EQ(oss.str(), "x: 3, y: 4");
}

TEST(PointTest, test_inequality) {
  ASSERT_TRUE(Point(1.0, 2.0) != Point(3.0, 4.0));
  ASSERT_FALSE(Point(1.0, 2.0) != Point(1.0, 2.0));
}

TEST(PointTest, test_create_point_zero_orient) {
  Point p{1.0, 2.0};
  auto q = p.create_point({0.0, 0.0}, 5.0);
  ASSERT_NEAR(q.x, 1.0, TOL);
  ASSERT_NEAR(q.y, 2.0, TOL);
}

TEST(LineTest, test_find_intersection_nullopt) {
  // Parallel horizontal lines — computeIntersectPoint returns false → nullopt
  Point p1{0.0, 0.0}, p2{1.0, 0.0};
  LineSegment line{p1, p2};
  Point p3{0.0, 1.0}, p4{1.0, 1.0};
  auto result = line.find_intersection(p3, p4);
  ASSERT_FALSE(result.has_value());
}
