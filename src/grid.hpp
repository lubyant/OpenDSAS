#ifndef SRC_GRID_HPP_
#define SRC_GRID_HPP_
#include <limits>
#include <utility>
#include <vector>

namespace dsas {
struct Grid {
  static double grids_bound_left_bottom_x;
  static double grids_bound_left_bottom_y;

  double min_x, min_y;
  size_t i, j;  // index in Grids
  std::vector<std::pair<size_t, size_t>> shoreline_segs_indices;
  Grid(double min_x, double min_y, size_t i, size_t j)
      : min_x{min_x}, min_y{min_y}, i{i}, j{j} {}
};

struct MapBound {
  double min_x = std::numeric_limits<double>::max();
  double min_y = std::numeric_limits<double>::max();
  double max_x = std::numeric_limits<double>::lowest();
  double max_y = std::numeric_limits<double>::lowest();
  double grid_size = 0;
};
extern MapBound map_bound;
extern double grid_size;

using Grids = std::vector<std::vector<Grid>>;
Grids create_grids(const MapBound &map_bound);
}  // namespace dsas
#endif