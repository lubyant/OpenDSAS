#ifndef SRC_SHORELINE_HPP_
#define SRC_SHORELINE_HPP_
#include "geometry.hpp"

namespace dsas {
extern double mean_shore_segment;  // mean length of shoreline segment
struct Shoreline : public MultiLine<Point> {
  std::vector<Point> shoreline_vertices_;  // shoreline vertices
  int shoreline_id_{};                     // shoreline id
  boost::gregorian::date date_;

  Shoreline(std::vector<Point> shoreline_vertices, int shoreline_id,
            boost::gregorian::date date)
      : shoreline_vertices_{std::move(shoreline_vertices)},
        shoreline_id_{shoreline_id},
        date_{std::move(date)} {};

  Shoreline() = default;

  [[nodiscard]] size_t size() const override {
    return shoreline_vertices_.size();
  }
  [[nodiscard]] const Point &operator[](size_t i) const override {
    return shoreline_vertices_[i];
  }
};

std::vector<Shoreline> load_shorelines_shp(
    const std::filesystem::path &shoreline_shp_path,
    const char *date_field_name);

}  // namespace dsas
#endif