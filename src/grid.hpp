#ifndef SRC_GRID_HPP_
#define SRC_GRID_HPP_

#include "shoreline.hpp"
#include "transect.hpp"

#include <limits>
#include <utility>
#include <vector>

namespace dsas {
struct Grid {
  static double grids_bound_left_bottom_x; // left_bottom of all grids
  static double grids_bound_left_bottom_y;
  static double grids_bound_right_top_x; // right_top of all grids
  static double grids_bound_right_top_y;
  static double grid_size;

  double min_x, min_y;
  size_t i, j;  // index in Grids
  std::vector<ShoreSeg> shoreline_segs;
  Grid(double min_x, double min_y, size_t i, size_t j)
      : min_x{min_x}, min_y{min_y}, i{i}, j{j} {}
};

void compute_grid_bound(const std::vector<std::unique_ptr<Shoreline>> &shorelines,
bool padding=true);

using Grids = std::vector<std::vector<Grid>>;
Grids create_grids();

void build_shoreline_index(const std::vector<std::unique_ptr<Shoreline>> &shorelines,
                           Grids &grids);

void build_transect_index(std::vector<std::unique_ptr<TransectLine>> &transects);

}  // namespace dsas
#endif