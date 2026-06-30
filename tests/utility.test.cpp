//
// Created by lby on 10/13/23.
//

#include "utility.hpp"

#include "gtest/gtest.h"

#include <fstream>

#define TOL 1e-4
using namespace dsas;
TEST(UtilityTest, TestCrossProduct) {
  auto ans = crossProduct(0.0, 1.0, 1.0, 0.0);

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

TEST(UtilityTest, TestLeastSquareMismatch) {
  ASSERT_NEAR(least_square({1, 2}, {3.0}), -999.99, TOL);
}

TEST(UtilityTest, TestLeastSquareEmpty) {
  ASSERT_NEAR(least_square({}, {}), -999.99, TOL);
}

TEST(UtilityTest, TestLeastSquareZeroVariance) {
  // All x values identical → variance = 0 → undefined slope
  ASSERT_NEAR(least_square({5, 5, 5}, {1.0, 2.0, 3.0}), -999.99, TOL);
}

TEST(UtilityTest, TestIntersectNonIntersecting) {
  // Bounding boxes overlap but segments don't actually cross → false (line 68)
  Point p1{0.0, 0.0}, p2{1.0, 1.0};
  Point p3{0.0, 1.0}, p4{1.0, 2.0};
  ASSERT_FALSE(isTwoSegmentIntersected(p1, p2, p3, p4));
}

TEST(UtilityTest, TestComputeIntersectParallel) {
  // Parallel horizontal lines → d ≈ 0 → returns false (lines 86-87)
  Point p1{0.0, 0.0}, p2{1.0, 0.0}, p3{0.0, 1.0}, p4{1.0, 1.0};
  Point output;
  ASSERT_FALSE(computeIntersectPoint(p1, p2, p3, p4, output));
}

TEST(UtilityTest, TestGetShpProj) {
  // Non-existent file — should throw
  ASSERT_THROW(get_shp_proj("/nonexistent/path.geojson"), std::runtime_error);

  // RFC 7946 GeoJSON without "crs" — implicit WGS84, returns EPSG:4326
  {
    auto tmp = std::filesystem::temp_directory_path() / "no_crs.geojson";
    std::ofstream f(tmp);
    f << R"({"type":"FeatureCollection","features":[]})";
    f.close();
    ASSERT_EQ(get_shp_proj(tmp.string().c_str()), "EPSG:4326");
  }

  // Old-format "crs" with type != "name" — should throw
  {
    auto tmp = std::filesystem::temp_directory_path() / "link_crs.geojson";
    std::ofstream f(tmp);
    f << R"({"type":"FeatureCollection","crs":{"type":"link","properties":{"href":"http://example.com","type":"proj4"}},"features":[]})";
    f.close();
    ASSERT_THROW(get_shp_proj(tmp.string().c_str()), std::runtime_error);
  }
}
