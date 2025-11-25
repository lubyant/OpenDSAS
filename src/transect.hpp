#ifndef SRC_TRANSECT_HPP_
#define SRC_TRANSECT_HPP_
#include <optional>
#include <stdexcept>

#include "baseline.hpp"
#include "exception.hpp"
#include "geometry.hpp"
#include "intersect.hpp"
#include "shoreline.hpp"

namespace dsas {
struct Grid;  // forward declaration

using TransectFields = std::tuple<int, int, double>;

struct TransectLine : public LineSegment,
                      MultiLine<Point>,
                      PointAttribute<double>,
                      GDALShpSavable<TransectFields> {
  using IntersectionMode = dsas::Options::IntersectionMode;
  using TransectOrientation = dsas::Options::TransectOrientation;
  using Grids = std::unordered_map<size_t, std::unique_ptr<Grid>>;
  Point transect_base_point_;  // point to generate the shapefile
  Point transect_ref_point_;   // point to calculate the erosion
  int transect_id_;
  int baseline_id_;
  double change_rate{};  // change rate for all the intersections
  IntersectionMode mode_;
  TransectOrientation orient_;
  std::vector<IntersectPoint *>
      intersects;  // pointers to intersects in this transects
  std::vector<std::pair<int, int>> grid_index;

  TransectLine(Point &transect_base, double transect_length,
               std::pair<double, double> baseline_normal_vector,
               int transect_id, int baseline_id,
               IntersectionMode mode = IntersectionMode::Closest,
               TransectOrientation orient = TransectOrientation::Mix)
      : LineSegment(create_transect(transect_base, baseline_normal_vector,
                                    transect_length, orient)),

        transect_base_point_(transect_base),
        transect_ref_point_(orient == TransectOrientation::Right ? leftEdge_
                                                                 : rightEdge_),
        transect_id_(transect_id),
        baseline_id_(baseline_id),
        mode_(mode),
        orient_(orient) {
    if (std::isnan(transect_ref_point_.x) ||
        std::isnan(transect_ref_point_.y)) {
      OPENDSAS_THROW("Error: transect reference point is NaN");
    }
  }

  TransectLine(Point start, Point end, int transect_id, int baseline_id,
               IntersectionMode mode = IntersectionMode::Closest,
               TransectOrientation orient = TransectOrientation::Mix)
      : LineSegment(start, end),
        transect_id_(transect_id),
        baseline_id_(baseline_id),
        mode_(mode),
        orient_(orient) {
    switch (orient_) {
      case TransectOrientation::Left:
        transect_ref_point_ = start;
        break;
      case TransectOrientation::Right:
        transect_ref_point_ = end;
        break;
      case TransectOrientation::Mix:
        transect_ref_point_ =
            Point((start.x + end.x) / 2, (start.y + end.y) / 2);
        break;
      default:
        OPENDSAS_THROW("Not a valid orientation!");
    }
    transect_base_point_ = Point((start.x + end.x) / 2, (start.y + end.y) / 2);
  }

  static LineSegment create_transect(
      Point &transect_base, std::pair<double, double> baseline_normal_vector,
      double transect_length, TransectOrientation orient);

  [[nodiscard]] std::optional<IntersectPoint> intersection(
      const Shoreline &shoreline) const;

  [[nodiscard]] std::vector<std::unique_ptr<IntersectPoint>> intersection(
      const Grids &grids) const;

  double distance2ref(Point &point) const {
    return transect_ref_point_.distance_to_point(point);
  }

  [[nodiscard]] size_t size() const override { return 3; }

  [[nodiscard]] const Point &operator[](size_t i) const override {
    switch (i) {
      case 0:
        return leftEdge_;
      case 1:
        return transect_ref_point_;
      case 2:
        return rightEdge_;
      default:
        OPENDSAS_THROW("Index out of range in TransectLine");
    }
  }

  [[nodiscard]] std::vector<std::string> get_names() const override {
    return {"TransectId", "BaselineId", "ChangeRate"};
  }
  [[nodiscard]] std::vector<OGRFieldType> get_types() const override {
    return {OGRFieldType::OFTInteger, OGRFieldType::OFTInteger,
            OGRFieldType::OFTReal};
  }
  [[nodiscard]] value_tuple get_values() const override {
    return {transect_id_, baseline_id_, change_rate};
  }

  [[nodiscard]] double get_x() const override { return transect_base_point_.x; }
  [[nodiscard]] double get_y() const override { return transect_base_point_.y; }
};

std::vector<std::unique_ptr<TransectLine>> create_transects_from_baseline(
    Baseline &);

std::vector<std::unique_ptr<TransectLine>> load_transects_from_shp(
    const std::filesystem::path &transect_shp_path);

void save_transect(const std::vector<std::unique_ptr<TransectLine>> &transects,
                   const std::string &prj, bool save_as_point = false);

}  // namespace dsas

#endif