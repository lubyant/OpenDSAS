#include "grid.hpp"

#include <gtest/gtest.h>

#include <cstddef>

#include "gtest/gtest.h"

using namespace dsas;
#define TOL 1e-4

TEST(GridTest, test_create_grid) {
  Grid::grids_bound_left_bottom_x = 0;
  Grid::grids_bound_left_bottom_y = 0;
  Grid::grids_bound_right_top_x = 10;
  Grid::grids_bound_right_top_y = 10;
  Grid::grid_size = 3;
  auto grids = create_grids();
  size_t nx = 4;
  size_t ny = 4;
  ASSERT_EQ(grids.size(), nx);
  ASSERT_EQ(grids[0].size(), ny);
  for (size_t i = 0; i < nx; i++) {
    for (size_t j = 0; j < ny; j++) {
      auto &grid = grids[i][j];
      ASSERT_EQ(grid.i, i);
      ASSERT_EQ(grid.j, j);
      ASSERT_NEAR(grid.min_x, i*Grid::grid_size, TOL);
      ASSERT_NEAR(grid.min_y, j*Grid::grid_size, TOL);
    }
  }
}

TEST(GridTest, test_compute_grid_bound){
  std::vector<Point> shore_vertices{{0,0},{1,1},{2,2},{3,3}};
  boost::gregorian::date t_date(2000,1,1);
  Shoreline shoreline{shore_vertices, 0, t_date};
  std::vector<Shoreline> shorelines{shoreline};

  compute_grid_bound(shorelines);
  ASSERT_NEAR(Grid::grid_size, 1.4142, TOL);
  ASSERT_NEAR(Grid::grids_bound_left_bottom_x, -0.7071, TOL);
  ASSERT_NEAR(Grid::grids_bound_left_bottom_y, -0.7071, TOL);
  ASSERT_NEAR(Grid::grids_bound_right_top_y, 3.7071, TOL);
  ASSERT_NEAR(Grid::grids_bound_right_top_y, 3.7071, TOL);
}

TEST(GridTest, test_build_transect_index) {
  std::vector<Point> points{{0, 0}, {1, 0}, {2, 0}, {3, 0}};
  options.transect_length = 1;
  options.transect_spacing = 0.5;
  options.transect_offset = 0;
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
  options.transect_offset = 0;
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

TEST(GridTest, test_build_shoreline_index){
  Grid::grids_bound_left_bottom_x = 0;
  Grid::grids_bound_left_bottom_y = 0;
  Grid::grids_bound_right_top_x = 10;
  Grid::grids_bound_right_top_y = 10;
  Grid::grid_size = 3;
  auto grids = create_grids();

  std::vector<Point> shore_vertices{{0,0},{1,1},{2,2},{3,3}};

  boost::gregorian::date t_date(2000,1,1);
  Shoreline shoreline{shore_vertices, 0, t_date};
  std::vector<Shoreline> shorelines{shoreline};
  
  build_shoreline_index(shorelines, grids);

  for(size_t i = 0; i < grids.size(); i++){
    for(size_t j = 0; j < grids[0].size(); j++){
        if(i == 0 && j == 0){
            ASSERT_EQ(grids[i][j].shoreline_segs.size(), 3);
            ASSERT_EQ(grids[i][j].shoreline_segs[0].start.x, 0);
            ASSERT_EQ(grids[i][j].shoreline_segs[0].start.y, 0);
            ASSERT_EQ(grids[i][j].shoreline_segs[0].end.x, 1);
            ASSERT_EQ(grids[i][j].shoreline_segs[0].end.y, 1);

            ASSERT_EQ(grids[i][j].shoreline_segs[1].start.x, 1);
            ASSERT_EQ(grids[i][j].shoreline_segs[1].start.y, 1);
            ASSERT_EQ(grids[i][j].shoreline_segs[1].end.x, 2);
            ASSERT_EQ(grids[i][j].shoreline_segs[1].end.y, 2);

            ASSERT_EQ(grids[i][j].shoreline_segs[2].start.x, 2);
            ASSERT_EQ(grids[i][j].shoreline_segs[2].start.y, 2);
            ASSERT_EQ(grids[i][j].shoreline_segs[2].end.x, 3);
            ASSERT_EQ(grids[i][j].shoreline_segs[2].end.y, 3);
        }else{
            ASSERT_EQ(grids[i][j].shoreline_segs.size(), 0);
        }
    }
  }
}