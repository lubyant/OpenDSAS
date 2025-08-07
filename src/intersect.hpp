#ifndef SRC_INTERSECT_HPP_
#define SRC_INTERSECT_HPP_

#include "geometry.hpp"
#include "shoreline.hpp"

namespace dsas {
#define IntersectPoint_t int, int, int, const char *, double, double, double
struct IntersectPoint : public Point, GDALShpSaver<IntersectPoint_t> {
  int transect_id_;
  int shoreline_id_;
  int baseline_id_;
  boost::gregorian::date date_;
  double distance_to_ref_{-1};
  IntersectPoint(Point point, int transect_id, int shoreline_id,
                 int baseline_id, boost::gregorian::date date,
                 double distance_to_ref)
      : Point(point),
        transect_id_(transect_id),
        shoreline_id_(shoreline_id),
        baseline_id_(baseline_id),
        date_(date),
        distance_to_ref_(distance_to_ref) {}

  [[nodiscard]] std::vector<std::string> get_names() const override {
    return {"BaselineId", "TransectId", "ShoreID", "Date",
            "ref_dist",   "X",          "Y"};
  }

  [[nodiscard]] std::vector<OGRFieldType> get_types() const override {
    return {OGRFieldType::OFTInteger, OGRFieldType::OFTInteger,
            OGRFieldType::OFTInteger, OGRFieldType::OFTString,
            OGRFieldType::OFTReal,    OGRFieldType::OFTReal,
            OGRFieldType::OFTReal};
  }

  [[nodiscard]] std::tuple<IntersectPoint_t> get_values() const override {
    return {baseline_id_,
            transect_id_,
            shoreline_id_,
            boost::gregorian::to_simple_string(date_).c_str(),
            distance_to_ref_,
            x,
            y};
  }
};

void save_points(const std::vector<dsas::IntersectPoint> &shapes,
                 const char *pszProj, const std::filesystem::path &output_path);

}  // namespace dsas

#endif
