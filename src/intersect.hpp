#ifndef SRC_INTERSECT_HPP_
#define SRC_INTERSECT_HPP_

#include "geometry.hpp"

namespace dsas {
#define IntersectPoint_t int, int, int, const char *, double, double, double
struct IntersectPoint : public Point, GDALShpSaver<IntersectPoint_t> {
  int transect_id_;
  int shoreline_id_;
  int baseline_id_;
  boost::gregorian::date date_;
  std::string date_string_;
  double distance_to_ref_{-1};
  IntersectPoint(Point point, int transect_id, int shoreline_id,
                 int baseline_id, boost::gregorian::date date,
                 double distance_to_ref)
      : Point(point),
        transect_id_(transect_id),
        shoreline_id_(shoreline_id),
        baseline_id_(baseline_id),
        date_(date),
        distance_to_ref_(distance_to_ref) {
    std::ostringstream oss;
    oss << date_.year() << "/" << std::setw(2) << std::setfill('0')
        << date_.month().as_number() << "/" << std::setw(2) << std::setfill('0')
        << date_.day();
    date_string_ = oss.str();
  }

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
            date_string_.c_str(),
            distance_to_ref_,
            x,
            y};
  }
};

void save_intersects(const std::vector<std::unique_ptr<IntersectPoint>> &,
                     const std::string &);
}  // namespace dsas

#endif
