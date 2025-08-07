//
// Created by lby on 10/13/23.
//

#include "utility.hpp"

#include "gtest/gtest.h"
#define TOL 1e-4
using namespace dsas;
TEST(UtilityTest, TestCrossProduct) {
  std::vector<double> vec1{0.0, 1.0};
  std::vector<double> vec2{1.0, 0.0};

  auto ans = crossProduct(vec1, vec2);

  EXPECT_EQ(ans, -1);
}

TEST(UtilityTest, TestIntersect) {
  {
    Point p1{0, 0};
    Point p2{1, 1};
    Point p3{0, 1};
    Point p4{1, 0};

    ASSERT_TRUE(isTwoSegmentIntersected(p1, p2, p3, p4));
  }
  {
    Point p1{0, 0};
    Point p2{2, 0};
    Point p3{1, -1};
    Point p4{1, 1};

    ASSERT_TRUE(isTwoSegmentIntersected(p1, p2, p3, p4));
  }
}
