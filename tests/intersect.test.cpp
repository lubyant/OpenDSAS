#include "intersect.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <memory>

#include "utility.hpp"

constexpr double TOL = 1e-4;
using namespace dsas;

TEST(TestIntersect, test_save_intersects) {
  const std::filesystem::path baseline_shp_path{
      std::string(TEST_DATA_DIR) + "/sample_baseline_offshore.geojson"};
  auto prj = get_shp_proj(baseline_shp_path.string().c_str());

  dsas::Date date1{2000, 1, 1};
  dsas::Date date2{2010, 1, 1};
  dsas::Date date3{2020, 1, 1};

  Point fake_point{0, 0};
  int fake_tid{0};
  int fake_bid{0};
  int fake_sid{0};
  double dist2ref1 = 0;
  double dist2ref2 = 1;
  double dist2ref3 = 2;

  auto tmp_file = std::filesystem::temp_directory_path() / "intersects.shp";
  options.intersect_path = tmp_file.string();

  auto make_pts = [&]() {
    std::vector<std::unique_ptr<IntersectPoint>> v;
    v.push_back(std::make_unique<IntersectPoint>(
        fake_point, fake_tid, fake_sid, fake_bid, date1, dist2ref1));
    v.push_back(std::make_unique<IntersectPoint>(
        fake_point, fake_tid, fake_sid, fake_bid, date2, dist2ref2));
    v.push_back(std::make_unique<IntersectPoint>(
        fake_point, fake_tid, fake_sid, fake_bid, date3, dist2ref3));
    return v;
  };

  // Shapefile output
  options.intersect_path = tmp_file.string();
  { save_intersects(make_pts(), prj); }

  // GeoJSON output — verify the file is valid JSON with expected structure
  auto geojson_file = std::filesystem::temp_directory_path() / "intersects.geojson";
  options.intersect_path = geojson_file.string();
  { save_intersects(make_pts(), prj); }
  {
    std::ifstream f(geojson_file);
    ASSERT_TRUE(f.good());
    auto j = nlohmann::json::parse(f);
    ASSERT_EQ(j["type"].get<std::string>(), "FeatureCollection");
    ASSERT_EQ(j["features"].size(), 3u);
    ASSERT_EQ(j["features"][0]["geometry"]["type"].get<std::string>(), "Point");
  }

  {
    std::vector<std::unique_ptr<IntersectPoint>> intersections;
    ASSERT_THROW(save_intersects(intersections, prj), std::runtime_error);
  }
  {
    prj = "INVALID_PROJ_STRING";
    ASSERT_THROW(save_intersects(make_pts(), prj), std::runtime_error);
  }
}