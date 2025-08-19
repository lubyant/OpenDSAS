#include "grid.hpp"

#include <gtest/gtest.h>

#include <cstddef>

#include "gtest/gtest.h"

using namespace dsas;
#define TOL 1e-4

TEST(GridTest, test_create_grid) {
  map_bound.min_x = 0;
  map_bound.min_y = 0;
  map_bound.max_x = 10;
  map_bound.max_y = 10;
  Grid::grid_size = 1;
  auto grids = create_grids(map_bound);
  size_t nx = 11;
  size_t ny = 11;
  ASSERT_EQ(grids.size(), nx);
  ASSERT_EQ(grids[0].size(), ny);
  ASSERT_NEAR(Grid::grids_bound_left_bottom_x, -0.5, TOL);
  ASSERT_NEAR(Grid::grids_bound_left_bottom_y, -0.5, TOL);
  for (size_t i = 0; i < nx; i++) {
    for (size_t j = 0; j < ny; j++) {
      auto &grid = grids[i][j];
      ASSERT_EQ(grid.i, i);
      ASSERT_EQ(grid.j, j);
      ASSERT_NEAR(grid.min_x, -0.5 + i, TOL);
      ASSERT_NEAR(grid.min_y, -0.5 + j, TOL);
    }
  }
}
