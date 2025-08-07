#ifndef SRC_BASELINE_CPP_
#define SRC_BASELINE_CPP_

#include "geometry.hpp"

namespace dsas {
struct BaselineSeg : public LineSegment {
  using TransectBasePoint = Point;
  double spacing_, offset_;
  static double cumulative_transects_distance,
      cumulative_segment_distance;  // cumulative distance
  std::vector<TransectBasePoint> transects_base_points_;

  // constructor
  BaselineSeg(double spacing, double offset, const Point &leftEdge,
              const Point &rightEdge);

  // element accessing
  const TransectBasePoint &operator[](const size_t n) const {
    return transects_base_points_[n];
  }
};

struct Baseline : public MultiLine<Point>, GDALShpSaver<int> {
  using BaselinesVertex = Point;

  int baseline_id_;
  std::vector<Point> transects_base_points_;
  std::vector<BaselinesVertex>
      origin_vertices_;  // the shoreline vertices for generate baseline
  std::vector<BaselinesVertex>
      baseline_vertices_;  // final baseline vertices for compute the rate
  std::vector<std::pair<double, double>> normal_vectors_;

  Baseline(const std::vector<BaselinesVertex> &points, int baseline_id);

  [[nodiscard]] size_t size() const override {
    return baseline_vertices_.size();
  };

  [[nodiscard]] const Point &operator[](size_t i) const override {
    return baseline_vertices_.at(i);
  }

  [[nodiscard]] std::vector<std::string> get_names() const override {
    return {"BaselineId"};
  }

  [[nodiscard]] std::vector<OGRFieldType> get_types() const override {
    return {OGRFieldType::OFTInteger};
  }

  [[nodiscard]] std::tuple<int> get_values() const override {
    return {baseline_id_};
  }
};

std::vector<Baseline> load_baselines_shp(
    const std::filesystem::path &baseline_shp_path,
    const std::string &baseline_id_field = "");
}  // namespace dsas

#endif