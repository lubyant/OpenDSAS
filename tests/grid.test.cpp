#include "grid.hpp"

#include <gtest/gtest.h>

using namespace dsas;
#define TOL 1e-4

TEST(GridTest, test_compute_grid_bound) {
  std::vector<Point> shore_vertices{{0, 0}, {1, 1}, {2, 2}, {3, 3}};
  dsas::Date t_date{2000, 1, 1};
  auto shoreline = std::make_unique<Shoreline>(shore_vertices, 0, t_date);
  std::vector<std::unique_ptr<Shoreline>> shorelines;
  shorelines.push_back(std::move(shoreline));

  compute_grid_bound(shorelines);
  ASSERT_NEAR(Grid::grid_size, 1.4142, TOL);
  ASSERT_NEAR(Grid::grids_bound_left_bottom_x, -0.7071, TOL);
  ASSERT_NEAR(Grid::grids_bound_left_bottom_y, -0.7071, TOL);
  ASSERT_NEAR(Grid::grids_bound_right_top_y, 3.7071, TOL);
  ASSERT_NEAR(Grid::grids_bound_right_top_y, 3.7071, TOL);
  ASSERT_EQ(Grid::grid_nx, 3);
  ASSERT_EQ(Grid::grid_ny, 3);
}

TEST(GridTest, test_build_transect_index) {
  std::vector<Point> points{{0, 0}, {1, 0}, {2, 0}, {3, 0}};
  options.transect_length = 1;
  options.transect_spacing = 0.5;
  options.smooth_factor = 0;
  options.intersection_mode = dsas::Options::IntersectionMode::Closest;
  auto baseline = std::make_unique<Baseline>(points, 0);
  auto transects_lines = create_transects_from_baseline(*baseline);
  Grid::grid_size = 1;
  Grid::grids_bound_left_bottom_x = 0;
  Grid::grids_bound_left_bottom_y = 0;
  Grid::grids_bound_right_top_x = 3;
  Grid::grids_bound_right_top_y = 3;
  build_transect_index(transects_lines);

  {
    auto &transect = transects_lines[0];
    ASSERT_EQ(transect->grid_index.size(), 1);
    ASSERT_EQ(transect->grid_index[0].first, 0);
    ASSERT_EQ(transect->grid_index[0].second, 0);
  }

  {
    auto &transect = transects_lines[1];
    ASSERT_EQ(transect->grid_index.size(), 1);
    ASSERT_EQ(transect->grid_index[0].first, 0);
    ASSERT_EQ(transect->grid_index[0].second, 0);
  }

  {
    auto &transect = transects_lines[2];
    ASSERT_EQ(transect->grid_index.size(), 1);
    ASSERT_EQ(transect->grid_index[0].first, 0);
    ASSERT_EQ(transect->grid_index[0].second, 0);
  }

  {
    auto &transect = transects_lines[3];
    ASSERT_EQ(transect->grid_index.size(), 1);
    ASSERT_EQ(transect->grid_index[0].first, 1);
    ASSERT_EQ(transect->grid_index[0].second, 0);
  }

  {
    auto &transect = transects_lines[4];
    ASSERT_EQ(transect->grid_index.size(), 1);
    ASSERT_EQ(transect->grid_index[0].first, 1);
    ASSERT_EQ(transect->grid_index[0].second, 0);
  }

  {
    auto &transect = transects_lines[5];
    ASSERT_EQ(transect->grid_index.size(), 1);
    ASSERT_EQ(transect->grid_index[0].first, 2);
    ASSERT_EQ(transect->grid_index[0].second, 0);
  }

  {
    auto &transect = transects_lines[6];
    ASSERT_EQ(transect->grid_index.size(), 1);
    ASSERT_EQ(transect->grid_index[0].first, 2);
    ASSERT_EQ(transect->grid_index[0].second, 0);
  }
}

TEST(GridTest, test_build_transect_out_of_bound) {
  std::vector<Point> points{{0, 0}, {1, 0}, {2, 0}, {3, 0}};
  options.transect_length = 1;
  options.transect_spacing = 0.5;
  options.smooth_factor = 0;
  options.intersection_mode = dsas::Options::IntersectionMode::Closest;
  auto baseline = std::make_unique<Baseline>(points, 0);
  auto transects_lines = create_transects_from_baseline(*baseline);
  Grid::grid_size = 1;
  Grid::grids_bound_left_bottom_x = 5;
  Grid::grids_bound_left_bottom_y = 0;
  Grid::grids_bound_right_top_x = 6;
  Grid::grids_bound_right_top_y = 3;
  build_transect_index(transects_lines);

  for (auto &transect : transects_lines) {
    ASSERT_EQ(transect->grid_index.size(), 0);
  }
}

TEST(GridTest, test_build_shoreline_index) {
  Grid::grids_bound_left_bottom_x = 0;
  Grid::grids_bound_left_bottom_y = 0;
  Grid::grids_bound_right_top_x = 10;
  Grid::grids_bound_right_top_y = 10;
  Grid::grid_size = 3;

  std::vector<Point> shore_vertices{{0, 0}, {1, 1}, {2, 2}, {3, 3}};

  dsas::Date t_date{2000, 1, 1};
  auto shoreline = std::make_unique<Shoreline>(shore_vertices, 0, t_date);
  std::vector<std::unique_ptr<Shoreline>> shorelines;
  shorelines.push_back(std::move(shoreline));

  auto grids = build_shoreline_index(shorelines);

  ASSERT_EQ(grids.size(), 1);
  ASSERT_EQ(grids.count(0), 1);
  ASSERT_EQ(grids.count(1), 0);
}

TEST(GridTest, test_build_shoreline_index_taller_than_wide) {
  // Grid is 2 cells wide (nx=2) and 6 cells tall (ny=6).
  // The buggy formula  ix*nx+iy  maps cells (0,2) and (1,0) both to key 2.
  // The correct formula ix*ny+iy  maps them to keys 2 and 6 — distinct.
  Grid::grids_bound_left_bottom_x = 0;
  Grid::grids_bound_left_bottom_y = 0;
  Grid::grids_bound_right_top_x = 2;
  Grid::grids_bound_right_top_y = 6;
  Grid::grid_size = 1;

  dsas::Date date{2000, 1, 1};

  // Segment A lands in cell (ix=0, iy=2): correct key = 0*ny+2 = 2
  auto shore_a = std::make_unique<Shoreline>(
      std::vector<Point>{{0.0, 2.1}, {0.5, 2.5}}, 0, date);

  // Segment B lands in cell (ix=1, iy=0): correct key = 1*ny+0 = 6
  // Under the buggy formula: A -> 0*2+2=2, B -> 1*2+0=2 (collision)
  auto shore_b = std::make_unique<Shoreline>(
      std::vector<Point>{{1.1, 0.0}, {1.5, 0.5}}, 1, date);

  std::vector<std::unique_ptr<Shoreline>> shorelines;
  shorelines.push_back(std::move(shore_a));
  shorelines.push_back(std::move(shore_b));

  auto grids = build_shoreline_index(shorelines);

  // Two segments in two distinct cells must produce two map entries.
  // With the bug grids.size() == 1 (both collapsed into key 2).
  ASSERT_EQ(grids.size(), 2);

  // Cell (0,2) -> key 2: holds only segment A
  ASSERT_EQ(grids.count(2), 1);
  ASSERT_EQ(grids.at(2)->shoreline_segs.size(), 1);

  // Cell (1,0) -> key 6: holds only segment B
  ASSERT_EQ(grids.count(6), 1);
  ASSERT_EQ(grids.at(6)->shoreline_segs.size(), 1);
}

TEST(GridTest, test_build_shoreline_index_cell_origin) {
  // Regression test: Grid::min_x/min_y must be the cell's world-space
  // origin (left_bottom + index * grid_size), not derived from the
  // segment's own bbox or the flat cell index.
  Grid::grids_bound_left_bottom_x = 0;
  Grid::grids_bound_left_bottom_y = 0;
  Grid::grids_bound_right_top_x = 10;
  Grid::grids_bound_right_top_y = 10;
  Grid::grid_size = 1;

  dsas::Date d{2000, 1, 1};
  // Segment sits inside cell (ix=2, iy=2), offset from that cell's origin,
  // so a formula that leaks the segment's own coordinates (instead of the
  // cell origin) would produce a visibly wrong result.
  auto shore = std::make_unique<Shoreline>(
      std::vector<Point>{{2.5, 2.5}, {2.6, 2.6}}, 0, d);
  std::vector<std::unique_ptr<Shoreline>> shores;
  shores.push_back(std::move(shore));

  auto grids = build_shoreline_index(shores);

  ASSERT_EQ(grids.size(), 1);
  auto &grid = grids.begin()->second;
  ASSERT_EQ(grid->i, 2u);
  ASSERT_EQ(grid->j, 2u);
  ASSERT_NEAR(grid->min_x, 2.0, TOL);
  ASSERT_NEAR(grid->min_y, 2.0, TOL);
}

TEST(GridTest, test_compute_grid_bound_even_segments) {
  // 5 points → 4 segments → even count → median uses two-heap average branch
  std::vector<Point> pts{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}};
  dsas::Date d{2000, 1, 1};
  auto shore = std::make_unique<Shoreline>(pts, 0, d);
  std::vector<std::unique_ptr<Shoreline>> shores;
  shores.push_back(std::move(shore));
  compute_grid_bound(shores);
  ASSERT_NEAR(Grid::grid_size, sqrt(2.0), TOL);
}

TEST(GridTest, test_build_shoreline_index_clamping) {
  // Segment extends beyond grid bounds → ix/iy clamped to grid limits
  Grid::grids_bound_left_bottom_x = 0;
  Grid::grids_bound_left_bottom_y = 0;
  Grid::grids_bound_right_top_x = 3;
  Grid::grids_bound_right_top_y = 3;
  Grid::grid_size = 1;
  dsas::Date d{2000, 1, 1};
  auto shore = std::make_unique<Shoreline>(
      std::vector<Point>{{2.0, 2.0}, {4.0, 4.0}}, 0, d);
  std::vector<std::unique_ptr<Shoreline>> shores;
  shores.push_back(std::move(shore));
  auto grids = build_shoreline_index(shores);
  ASSERT_FALSE(grids.empty());
}

TEST(GridTest, test_build_transect_index_clamping) {
  // Transect extends beyond grid bounds → ix/iy clamped to grid limits
  Grid::grids_bound_left_bottom_x = 0;
  Grid::grids_bound_left_bottom_y = 0;
  Grid::grids_bound_right_top_x = 3;
  Grid::grids_bound_right_top_y = 3;
  Grid::grid_size = 1;
  Point start{2.0, 1.0}, end{4.0, 1.0};
  auto t = std::make_unique<TransectLine>(start, end, 0, 0);
  std::vector<std::unique_ptr<TransectLine>> transects;
  transects.push_back(std::move(t));
  build_transect_index(transects);
  ASSERT_FALSE(transects[0]->grid_index.empty());
}