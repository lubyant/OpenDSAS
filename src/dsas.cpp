#include "dsas.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include "grid.hpp"
#include "intersect.hpp"
#include "options.hpp"
#include "utility.hpp"
namespace dsas {

std::vector<std::unique_ptr<TransectLine>> generate_transects(
    std::vector<Baseline> &baselines) {
  std::vector<std::unique_ptr<TransectLine>> transects;
  for (auto &baseline : baselines) {
    auto transect_lines = create_transects_from_baseline(baseline);
    transects.insert(transects.end(),
                     std::make_move_iterator(transect_lines.begin()),
                     std::make_move_iterator(transect_lines.end()));
  }
  return transects;
}

std::vector<std::unique_ptr<IntersectPoint>> generate_intersects(
    std::vector<std::unique_ptr<TransectLine>> &transects,
    const std::vector<std::unique_ptr<Shoreline>> &shorelines) {
  using Pending = std::pair<size_t, std::unique_ptr<IntersectPoint>>;
  std::vector<std::unique_ptr<IntersectPoint>> intersects;
  std::vector<Pending> all_pending;
#pragma omp parallel
  {
    std::vector<Pending> local;

#pragma omp for collapse(2) schedule(dynamic)
    for (std::int64_t i = 0; i < transects.size(); i++) {
      for (std::int64_t j = 0; j < shorelines.size(); j++) {
        auto ret = transects[i]->intersection(*shorelines[j]);
        if (ret.has_value()) {
          auto up = std::make_unique<IntersectPoint>(*ret);
          local.emplace_back(static_cast<size_t>(i), std::move(up));
        }
      }
    }
#pragma omp critical
    {
      // move-append local -> all_pending
      all_pending.insert(all_pending.end(),
                         std::make_move_iterator(local.begin()),
                         std::make_move_iterator(local.end()));
    }
  }

  intersects.reserve(all_pending.size());  // optional; avoids a few reallocs
  for (auto &p : all_pending) {
    intersects.push_back(std::move(p.second));  // move unique_ptr in
    TransectLine *t = transects[p.first].get();
    t->intersects.push_back(
        intersects.back().get());  // raw pointer to heap object (stable)
  }
  return intersects;
}

std::vector<std::unique_ptr<IntersectPoint>> generate_intersects(
    std::vector<std::unique_ptr<TransectLine>> &transects, const Grids &grids) {
  std::vector<std::unique_ptr<IntersectPoint>> intersects;
#pragma omp parallel
  {
    std::vector<std::unique_ptr<IntersectPoint>> local;

#pragma omp for schedule(static)
    for (std::int64_t i = 0; i < transects.size(); i++) {
      auto tmp_intersects = transects[i]->intersection(grids);
      if (!tmp_intersects.empty()) {
        for (auto &intersect : tmp_intersects) {
          transects[i]->intersects.push_back(intersect.get());
          local.push_back(std::move(intersect));
        }
      }
    }
#pragma omp critical
    {
      // move-append local -> all_pending
      intersects.insert(intersects.end(),
                        std::make_move_iterator(local.begin()),
                        std::make_move_iterator(local.end()));
    }
  }

  return intersects;
}

Grids build_spatial_grids(
    const std::vector<std::unique_ptr<Shoreline>> &shorelines,
    std::vector<std::unique_ptr<TransectLine>> &transects) {
  compute_grid_bound(shorelines);
  auto grids = build_shoreline_index(shorelines);
  build_transect_index(transects);
  return grids;
}
double linearRegressRate(std::vector<IntersectPoint *> &intersections) {
  // if no intersection
  if (intersections.empty()) {
    throw std::runtime_error("It should not be empty\n");
  }

  // if only one intersection
  if (intersections.size() == 1) {
    return 0;
  }

  //  sort the vector
  std::sort(intersections.begin(), intersections.end(),
            [](const IntersectPoint *a, const IntersectPoint *b) {
              return a->date_ < b->date_;
            });

  // if two intersections
  if (intersections.size() == 2) {
    double d_distance =
        intersections[1]->distance_to_ref_ - intersections[0]->distance_to_ref_;
    double d_year =
        (intersections[1]->date_ - intersections[0]->date_).days() / 365;
    return d_distance / d_year;
  }

  // change rate
  std::vector<double> y;
  std::vector<long long> x;
  y.push_back(intersections[0]->distance_to_ref_);
  x.push_back(intersections[0]->date_.julian_day());
  for (size_t i = 1; i < intersections.size(); ++i) {
    if (intersections[i]->date_ == intersections[i - 1]->date_) {
      // Prevent division by zero
      throw std::runtime_error("Duplicate dates were found!\n");
    }
    x.push_back(intersections[i]->date_.julian_day());
    y.push_back(intersections[i]->distance_to_ref_);
  }
  double change_rate_in_day = least_square(x, y);
  double change_rate_yr = change_rate_in_day * 365.25;
  return change_rate_yr;
}

}  // namespace dsas