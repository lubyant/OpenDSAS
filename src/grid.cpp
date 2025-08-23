#include "grid.hpp"

#include <cstddef>
#include <memory>
#include <queue>

namespace dsas {

double Grid::grids_bound_left_bottom_x = 0.0;
double Grid::grids_bound_left_bottom_y = 0.0;
double Grid::grids_bound_right_top_x = 0;
double Grid::grids_bound_right_top_y = 0;
double Grid::grid_size = 0.0;
size_t Grid::grid_nx = 0;
size_t Grid::grid_ny = 0;

void compute_grid_bound(
    const std::vector<std::unique_ptr<Shoreline>> &shorelines, bool padding) {
  // two pq to find the median length of shorelines
  std::priority_queue<double> max_pq;
  std::priority_queue<double, std::vector<double>, std::greater<double>> min_pq;
  double min_x = std::numeric_limits<double>::max();
  double min_y = std::numeric_limits<double>::max();
  double max_x = std::numeric_limits<double>::lowest();
  double max_y = std::numeric_limits<double>::lowest();

  for (const auto &shoreline : shorelines) {
    for (size_t i = 0; i < shoreline->size(); i++) {
      min_x = std::min(min_x, (*shoreline)[i].x);
      max_x = std::max(max_x, (*shoreline)[i].x);
      min_y = std::min(min_y, (*shoreline)[i].y);
      max_y = std::max(max_y, (*shoreline)[i].y);
      if (i < shoreline->size() - 1) {
        double seg_len = (*shoreline)[i].distance_to_point((*shoreline)[i + 1]);
        max_pq.push(seg_len);
        min_pq.push(max_pq.top());
        max_pq.pop();

        if (max_pq.size() < min_pq.size()) {
          max_pq.push(min_pq.top());
          min_pq.pop();
        }
      }
    }
  }
  double median_seg_len = max_pq.size() > min_pq.size()
                              ? max_pq.top()
                              : (min_pq.top() + max_pq.top()) / 2;

  Grid::grid_size = median_seg_len;

  double padding_space{0};
  if (padding) {
    padding_space = Grid::grid_size / 2;
  }
  Grid::grids_bound_left_bottom_x = min_x - padding_space;
  Grid::grids_bound_left_bottom_y = min_y - padding_space;
  Grid::grids_bound_right_top_x = max_x + padding_space;
  Grid::grids_bound_right_top_y = max_y + padding_space;
  Grid::grid_nx = static_cast<size_t>(
      (Grid::grids_bound_right_top_x - Grid::grids_bound_left_bottom_x) /
      Grid::grid_size);
  Grid::grid_ny = static_cast<size_t>(
      (Grid::grids_bound_right_top_y - Grid::grids_bound_left_bottom_y) /
      Grid::grid_size);
}

Grids build_shoreline_index(
    const std::vector<std::unique_ptr<Shoreline>> &shorelines) {
  // basic sanity
  assert(Grid::grid_size > 0.0);
  assert(Grid::grids_bound_right_top_x > Grid::grids_bound_left_bottom_x);
  assert(Grid::grids_bound_right_top_y > Grid::grids_bound_left_bottom_y);

  const double left_bottom_x = Grid::grids_bound_left_bottom_x;
  const double left_bottom_y = Grid::grids_bound_left_bottom_y;
  const double right_top_x = Grid::grids_bound_right_top_x;
  const double right_top_y = Grid::grids_bound_right_top_y;
  const double grid_size = Grid::grid_size;

  // derive nx, ny from grids or bounds
  int nx = static_cast<int>((right_top_x - left_bottom_x) / grid_size);
  int ny = static_cast<int>((right_top_y - left_bottom_y) / grid_size);
  assert(nx > 0 && ny > 0);

  auto compute_index_x = [&](const double x) {
    int x_index =
        static_cast<int>(std::ceil(x - left_bottom_x) / grid_size) - 1;
    if (x_index < 0) {
      x_index = 0;
    }
    if (x_index >= nx) {
      x_index = nx - 1;
    }
    return x_index;
  };

  auto compute_index_y = [&](const double y) {
    int y_index =
        static_cast<int>(std::ceil(y - left_bottom_y) / grid_size) - 1;
    if (y_index < 0) {
      y_index = 0;
    }
    if (y_index >= ny) {
      y_index = ny - 1;
    }
    return y_index;
  };

  Grids grids;
  for (size_t si = 0; si < shorelines.size(); ++si) {
    const auto &pts = shorelines[si]->shoreline_vertices_;
    if (pts.size() < 2) continue;

    for (size_t j = 0; j + 1 < pts.size(); ++j) {
      const auto &a = pts[j];
      const auto &b = pts[j + 1];

      // segment bbox
      const double xmin = std::min(a.x, b.x);
      const double xmax = std::max(a.x, b.x);
      const double ymin = std::min(a.y, b.y);
      const double ymax = std::max(a.y, b.y);

      // map to inclusive cell range (half-open convention)
      int ix0 = compute_index_x(xmin);
      int ix1 = compute_index_x(xmax);
      int iy0 = compute_index_y(ymin);
      int iy1 = compute_index_y(ymax);

      // if clamped min > max, the bbox doesn't overlap the grid
      if (ix0 > ix1 || iy0 > iy1) continue;

      // insert this segment ID (shoreline si, local segment j) into all
      // overlapped cells
      for (int ix = ix0; ix <= ix1; ++ix) {
        for (int iy = iy0; iy <= iy1; ++iy) {
          size_t grids_id = ix * nx + iy;
          if (grids.find(grids_id) == grids.end()) {
            grids[grids_id] = std::make_unique<Grid>(
                xmin + ix * grids_id, ymin + iy * grid_size, ix, iy);
          }
          grids[grids_id]->shoreline_segs.emplace_back(a, b,
                                                       shorelines[si].get());
        }
      }
    }
  }
  return grids;
}

void build_transect_index(
    std::vector<std::unique_ptr<TransectLine>> &transects) {
  // Basic sanity
  assert(Grid::grid_size > 0.0);
  assert(Grid::grids_bound_right_top_x > Grid::grids_bound_left_bottom_x);
  assert(Grid::grids_bound_right_top_y > Grid::grids_bound_left_bottom_y);

  const double left_bottom_x = Grid::grids_bound_left_bottom_x;
  const double left_bottom_y = Grid::grids_bound_left_bottom_y;
  const double right_top_x = Grid::grids_bound_right_top_x;
  const double right_top_y = Grid::grids_bound_right_top_y;
  const double grid_size = Grid::grid_size;

  // compute grid dims once (avoid recomputing in every loop)
  int nx = static_cast<int>((right_top_x - left_bottom_x) / grid_size);
  int ny = static_cast<int>((right_top_y - left_bottom_y) / grid_size);
  assert(nx > 0 && ny > 0);

  auto compute_index_x = [&](const double x) {
    int x_index =
        static_cast<int>(std::ceil(x - left_bottom_x) / grid_size) - 1;
    if (x_index < 0) {
      x_index = 0;
    }
    if (x_index >= nx) {
      x_index = nx - 1;
    }
    return x_index;
  };

  auto compute_index_y = [&](const double y) {
    int y_index =
        static_cast<int>(std::ceil(y - left_bottom_y) / grid_size) - 1;
    if (y_index < 0) {
      y_index = 0;
    }
    if (y_index >= ny) {
      y_index = ny - 1;
    }
    return y_index;
  };
  for (auto &transect : transects) {
    const double min_x =
        std::min(transect->leftEdge_.x, transect->rightEdge_.x);
    const double max_x =
        std::max(transect->leftEdge_.x, transect->rightEdge_.x);
    const double min_y =
        std::min(transect->leftEdge_.y, transect->rightEdge_.y);
    const double max_y =
        std::max(transect->leftEdge_.y, transect->rightEdge_.y);

    // Early skip if bbox is entirely outside the grid bounds (cheap)
    if (max_x < left_bottom_x || min_x > right_top_x || max_y < left_bottom_y ||
        min_y > right_top_y) {
      continue;
    }

    // Convert to cell indices (inclusive range)
    int ix0 = compute_index_x(min_x);
    int ix1 = compute_index_x(max_x);
    int iy0 = compute_index_y(min_y);
    int iy1 = compute_index_y(max_y);

    // Handle possible inversion (segment entirely outside after clamping)
    if (ix0 > ix1 || iy0 > iy1) {
      continue;
    }

    // Reserve to avoid repeated reallocs if you store many cells
    const int cells_x = ix1 - ix0 + 1;
    const int cells_y = iy1 - iy0 + 1;
    transect->grid_index.reserve(transect->grid_index.size() +
                                 cells_x * cells_y);

    for (int ix = ix0; ix <= ix1; ++ix) {
      for (int iy = iy0; iy <= iy1; ++iy) {
        transect->grid_index.emplace_back(static_cast<size_t>(ix),
                                          static_cast<size_t>(iy));
      }
    }
  }
}
}  // namespace dsas
