#include "baseline.hpp"

#include <ogrsf_frmts.h>

namespace dsas {

/**
 *  static members
 */
// cumulative distance from the first object of baselineSeg to last the transect
double BaselineSeg::cumulative_transects_distance = 0;
// cumulative distance  of baselineSeg
double BaselineSeg::cumulative_segment_distance = 0;

BaselineSeg::BaselineSeg(double spacing, double offset, const Point &leftEdge,
                         const Point &rightEdge)
    : LineSegment(leftEdge, rightEdge), spacing_(spacing), offset_(offset) {
  move_line(offset_);
  double length = leftEdge.distance_to_point(rightEdge);

  // if current baselineSeg is too short, don't create any transects
  if (cumulative_transects_distance + spacing_ >
      cumulative_segment_distance + length) {
    cumulative_segment_distance += length;
    return;
  }

  // starting length
  double start =
      cumulative_transects_distance + spacing_ - cumulative_segment_distance;
  double ratio = start / length;

  // left edge
  double x_l{leftEdge.x}, y_l{leftEdge.y};
  // right edge
  double x_r{rightEdge.x}, y_r{rightEdge.y};
  // start point
  double x_start{x_l + ratio * (x_r - x_l)};
  double y_start{y_l + ratio * (y_r - y_l)};
  Point p0{x_start, y_start};

  // number of step
  int num = floor((p0.distance_to_point(rightEdge) + spacing_) / spacing_);

  // step size
  double dist = sqrt(slope_vector_.second * slope_vector_.second +
                     slope_vector_.first * slope_vector_.first);
  double x_step = spacing * slope_vector_.second / dist;
  double y_step = spacing * slope_vector_.first / dist;
  double x_cur, y_cur;
  for (int i = 0; i < num; i++) {
    x_cur = x_start + i * x_step;
    y_cur = y_start + i * y_step;
    transects_base_points_.emplace_back(x_cur, y_cur);
    // update the cumulative length
    cumulative_transects_distance += spacing_;
  }
  cumulative_segment_distance += length;
}

Baseline::Baseline(const std::vector<BaselinesVertex> &points, int baseline_id)
    : baseline_id_(baseline_id) {
  // create the baselineSeq
  for (size_t i = 0; i < points.size() - 1; i++) {
    BaselineSeg baselineSeg{options.transect_spacing, points.at(i),
                            points.at(i + 1)};
    // starting point
    if (i == 0) {
      transects_base_points_.push_back(baselineSeg.leftEdge_);
      normal_vectors_.push_back(baselineSeg.normal_vector_);
      origin_vertices_.push_back(baselineSeg.leftEdge_);
    }
    origin_vertices_.push_back(baselineSeg.rightEdge_);
    for (auto &point : baselineSeg.transects_base_points_) {
      transects_base_points_.push_back(point);
      normal_vectors_.push_back(baselineSeg.normal_vector_);
    }
  }
}

std::vector<Baseline> load_baselines_shp(
    const std::filesystem::path &baseline_shp_path,
    const std::string &baseline_id_field) {
  // Initialize GDAL
  GDALAllRegister();

  // Open the Shapefile
  GDALDataset *poDS = nullptr;
  poDS = static_cast<GDALDataset *>(GDALOpenEx(
      baseline_shp_path.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
  if (poDS == nullptr) {
    std::cerr << "Open failed.\n";
    exit(1);
  }

  // Get the Layer Containing the Line Features
  OGRLayer *poLayer = nullptr;
  poLayer = poDS->GetLayer(0);

  // Iterate Through the Features in the Layer and Access Points
  OGRFeature *poFeature = nullptr;
  poLayer->ResetReading();
  std::vector<Baseline> baselines;
  int baseline_id = 0;
  if (baseline_id_field.empty()) {
    baseline_id = 0;
  } else {
    OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
    int field_index = poFDefn->GetFieldIndex(baseline_id_field.c_str());

    if (field_index < 0) {
      std::cerr << "Field '" << baseline_id_field
                << "' not found in shapefile.\n";
      GDALClose(poDS);
      exit(1);
    }
  }

  while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
    OGRGeometry *poGeometry = nullptr;
    poGeometry = poFeature->GetGeometryRef();
    if (baseline_id_field.empty()) {
      baseline_id++;
    } else {
      baseline_id = poFeature->GetFieldAsInteger(baseline_id_field.c_str());
    }

    if (poGeometry != nullptr) {
      auto gtype = wkbFlatten(poGeometry->getGeometryType());
      if (gtype == wkbLineString) {
        std::vector<Point> baseline_vertices;
        auto *poLine = dynamic_cast<OGRLineString *>(poGeometry);
        for (int i = 0; i < poLine->getNumPoints(); i++) {
          OGRPoint point;
          poLine->getPoint(i, &point);
          baseline_vertices.emplace_back(point.getX(), point.getY());
        }
        baselines.emplace_back(baseline_vertices, baseline_id);
      } else if (gtype == wkbMultiLineString) {
        auto *poMulti = dynamic_cast<OGRMultiLineString *>(poGeometry);
        for (int j = 0; j < poMulti->getNumGeometries(); j++) {
          std::vector<Point> baseline_vertices;
          OGRGeometry *subGeom = poMulti->getGeometryRef(j);
          auto *poLine = dynamic_cast<OGRLineString *>(subGeom);
          for (int i = 0; i < poLine->getNumPoints(); i++) {
            OGRPoint point;
            poLine->getPoint(i, &point);
            baseline_vertices.emplace_back(point.getX(), point.getY());
          }
          baselines.emplace_back(baseline_vertices, baseline_id);
        }
      } else {
        std::cout << "Unsupported geometry type.\n";
        OGRFeature::DestroyFeature(poFeature);
        continue;
      }
    }
    OGRFeature::DestroyFeature(poFeature);
  }

  // Cleanup
  GDALClose(poDS);

  return baselines;
}
}  // namespace dsas