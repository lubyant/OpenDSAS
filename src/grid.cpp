#include "grid.hpp"

namespace dsas {

MapBound map_bound;

double Grid::grids_bound_left_bottom_x = 0.0;
double Grid::grids_bound_left_bottom_y = 0.0;
double Grid::grids_bound_right_top_x = 0;
double Grid::grids_bound_right_top_y = 0;
double Grid::grid_size = 0.0;

Grids create_grids(const MapBound &map_bound) {
  Grids grids;
  double grid_size{Grid::grid_size};
  const double x_range{map_bound.max_x - map_bound.min_x};
  const double y_range{map_bound.max_y - map_bound.max_x};

  const size_t nx{static_cast<size_t>(x_range / grid_size) + 1};
  const size_t ny{static_cast<size_t>(x_range / grid_size) + 1};

  const double left_bottom_x{map_bound.min_x - grid_size / 2};
  const double left_bottom_y{map_bound.min_y - grid_size / 2};
  for (size_t i = 0; i < nx; i++) {
    std::vector<Grid> tmp;
    for (size_t j = 0; j < ny; j++) {
      double min_x{left_bottom_x + i * grid_size};
      double min_y{left_bottom_y + j * grid_size};
      Grid grid{min_x, min_y, i, j};
      tmp.push_back(std::move(grid));
    }
    grids.push_back(std::move(tmp));
  }
  Grid::grids_bound_left_bottom_x = left_bottom_x;
  Grid::grids_bound_left_bottom_y = left_bottom_y;
  return grids;
}

}  // namespace dsas
