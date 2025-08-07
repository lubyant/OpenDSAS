//
// Created by lby on 10/21/23.
//
#include "geometry.hpp"

#include <gtest/gtest.h>

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
