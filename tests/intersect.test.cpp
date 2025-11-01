#include "intersect.hpp"

#include <gtest/gtest.h>

#include <memory>

#include "utility.hpp"

constexpr double TOL = 1e-4;
using namespace dsas;

TEST(TestIntersect, test_save_intersects) {
  const std::filesystem::path baseline_shp_path{
      std::string(TEST_DATA_DIR) + "/sample_baseline_offshore.geojson"};
  auto prj = get_shp_proj(baseline_shp_path.string().c_str());

  boost::gregorian::date date1{
      2000, boost::gregorian::greg_month::month_enum::Jan, 1};
  boost::gregorian::date date2{
      2010, boost::gregorian::greg_month::month_enum::Jan, 1};
  boost::gregorian::date date3{
      2020, boost::gregorian::greg_month::month_enum::Jan, 1};

  Point fake_point{0, 0};
  int fake_tid{0};
  int fake_bid{0};
  int fake_sid{0};
  double dist2ref1 = 0;
  double dist2ref2 = 1;
  double dist2ref3 = 2;

  {
    std::vector<std::unique_ptr<IntersectPoint>> intersections;
    auto p1 = std::make_unique<dsas::IntersectPoint>(
        fake_point, fake_tid, fake_sid, fake_bid, date1, dist2ref1);
    auto p2 = std::make_unique<dsas::IntersectPoint>(
        fake_point, fake_tid, fake_sid, fake_bid, date2, dist2ref2);
    auto p3 = std::make_unique<dsas::IntersectPoint>(
        fake_point, fake_tid, fake_sid, fake_bid, date3, dist2ref3);

    intersections.push_back(std::move(p1));
    intersections.push_back(std::move(p2));
    intersections.push_back(std::move(p3));
    auto tmp_file = std::filesystem::temp_directory_path() / "intersects.shp";
    options.intersect_path = tmp_file.string();
    save_intersects(intersections, prj);
  }
}