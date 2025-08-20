#include "dsas.hpp"

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
  std::vector<std::unique_ptr<IntersectPoint>> intersects;
  for (size_t i = 0; i < transects.size(); i++) {
    for (size_t j = 0; j < shorelines.size(); j++) {
      auto ret = transects[i]->intersection(*shorelines[j]);
      if (ret.has_value()) {
        intersects.push_back(std::make_unique<IntersectPoint>(ret.value()));
        transects[i]->intersects.push_back(
            intersects[intersects.size() - 1].get());
      }
    }
  }
  return intersects;
}

std::vector<std::unique_ptr<IntersectPoint>> generate_intersects(
    std::vector<std::unique_ptr<TransectLine>> &transects, const Grids &grids) {
  std::vector<std::unique_ptr<IntersectPoint>> intersects;
  for (auto &transect : transects) {
    auto tmp_intersects = transect->intersection(grids);
    if (!tmp_intersects.empty()) {
      for (auto &intersect : tmp_intersects) {
        transect->intersects.push_back(intersect.get());
        intersects.push_back(std::move(intersect));
      }
    }
  }
  return intersects;
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
      continue;
    }
    x.push_back(intersections[i]->date_.julian_day());
    y.push_back(intersections[i]->distance_to_ref_);
  }
  double change_rate_in_day = least_square(x, y);
  double change_rate_yr = change_rate_in_day * 365.25;
  return change_rate_yr;
}

}  // namespace dsas