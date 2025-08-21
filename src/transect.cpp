#include "transect.hpp"

#include <ogrsf_frmts.h>

#include <cstddef>
#include <memory>

#include "grid.hpp"
#include "utility.hpp"

namespace dsas {

LineSegment TransectLine::create_transect(
    Point &transect_base, std::pair<double, double> baseline_normal_vector,
    double transect_length, TransectOrientation orient) {
  Point leftEdge, rightEdge;
  switch (orient) {
    case TransectOrientation::Mix:
      leftEdge = transect_base.create_point(baseline_normal_vector,
                                            transect_length / 2);
      rightEdge = transect_base.create_point(baseline_normal_vector,
                                             -transect_length / 2);
      if (leftEdge == rightEdge) {
        std::cerr << baseline_normal_vector.first
                  << baseline_normal_vector.second << std::endl;
        std::cerr << leftEdge << ", " << rightEdge << std::endl;
        std::cerr << __FILE__ << __LINE__ << std::endl;
        exit(1);
      }
      break;
    case TransectOrientation::Left:
      leftEdge =
          transect_base.create_point(baseline_normal_vector, transect_length);
      rightEdge = transect_base;
      break;
    case TransectOrientation::Right:
      leftEdge = transect_base;
      rightEdge =
          transect_base.create_point(baseline_normal_vector, -transect_length);
      break;
    default:
      throw std::runtime_error("Not a valid orientation!");
  }
  return {leftEdge, rightEdge};
}

std::optional<IntersectPoint> TransectLine::intersection(
    const Shoreline &shoreline) const {
  std::vector<IntersectPoint> intersections;

  // find out all the available intersection
  for (size_t i = 0; i < shoreline.size() - 1; i++) {
    if (is_intersect(shoreline[i], shoreline[i + 1])) {
      auto ret = find_intersection(shoreline[i], shoreline[i + 1]);
      auto point = Point((shoreline[i].x + shoreline[i + 1].x) / 2,
                         (shoreline[i].y + shoreline[i + 1].y) / 2);
      if (ret) {
        point = ret.value();
      }
      auto distance = distance2ref(point);
      IntersectPoint intersect_point{
          point,        transect_id_,    shoreline.shoreline_id_,
          baseline_id_, shoreline.date_, distance};
      intersections.push_back(intersect_point);
    }
  }

  // if no intersection
  if (intersections.empty()) {
    return std::nullopt;
  }

  // if only one intersection
  if (intersections.size() == 1) {
    return intersections[0];
  }

  // if more than two intersections, pick one base on the intersection mode
  std::sort(intersections.begin(), intersections.end(),
            [](const IntersectPoint &a, const IntersectPoint &b) {
              return a.distance_to_ref_ < b.distance_to_ref_;
            });

  if (mode_ == IntersectionMode::Farthest) {
    // farthest mode return farthest distance
    return intersections[intersections.size() - 1];
  } else {
    // close mode return smallest dis
    return intersections[0];
  }
}

std::vector<std::unique_ptr<IntersectPoint>> TransectLine::intersection(
    const Grids &grids) const {
  if (grid_index.empty()) return {};

  std::vector<std::unique_ptr<IntersectPoint>> intersections;

  // find out all the available intersection
  for (auto [grid_i, grid_j] : grid_index) {
    const size_t grid_index = grid_i * Grid::grid_nx + grid_j;
    if(grids.find(grid_index) == grids.end()){
      continue;
    }
    const auto &grid = grids.at(grid_index);
    for (const auto &shore_seg : grid->shoreline_segs) {
      if (is_intersect(shore_seg.start, shore_seg.end)) {
        auto ret = find_intersection(shore_seg.start, shore_seg.end);
        auto point = ret.value();
        auto distance = distance2ref(point);
        auto intersect_point = std::make_unique<IntersectPoint>(
            point, transect_id_, shore_seg.shoreline->shoreline_id_,
            baseline_id_, shore_seg.shoreline->date_, distance);
        intersections.push_back(std::move(intersect_point));
      }
    }
  }

  // if more than two intersections, pick one base on the intersection mode
  std::sort(intersections.begin(), intersections.end(),
            [&](const auto &a, const auto &b) {
              if (a->shoreline_id_ == b->shoreline_id_) {
                return mode_ == IntersectionMode::Farthest
                           ? a->distance_to_ref_ > b->distance_to_ref_
                           : a->distance_to_ref_ < b->distance_to_ref_;
              }
              return a->shoreline_id_ < b->shoreline_id_;
            });

  intersections.erase(
      std::unique(
          intersections.begin(), intersections.end(),
          [](auto &a, auto &b) {
            return a->shoreline_id_ ==
                   b->shoreline_id_;  // consider "same" if first is equal
          }),
      intersections.end());
  return intersections;
}

void save_points(const std::vector<TransectLine> &shapes, const char *pszProj,
                 const std::filesystem::path &output_path) {
  // Initialize GDAL
  OGRSpatialReference oSRS;
  if (oSRS.importFromWkt(&pszProj) != OGRERR_NONE) {
    throw std::runtime_error("Projection setting fail!");
  }

  // Get the shapefile driver
  GDALDriver *driver =
      GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
  if (driver == nullptr) {
    throw std::runtime_error("Unable to get ESRI Shapefile driver");
  }

  // Create a new shapefile
  GDALDataset *dataset = driver->Create(output_path.string().c_str(), 0, 0, 0,
                                        GDT_Unknown, nullptr);

  // Step 4: Create a layer for the shapefile
  OGRLayer *layer =
      dataset->CreateLayer("pointLayer", &oSRS, wkbPoint, nullptr);
  if (layer == nullptr) {
    throw std::runtime_error("Layer is not created!");
  }

  // define attributes
  for (size_t i = 0; i < shapes[0].get_names().size(); i++) {
    OGRFieldDefn field(shapes[0].get_names()[i].c_str(),
                       shapes[0].get_types()[i]);
    if (layer->CreateField(&field) != OGRERR_NONE) {
      std::cerr << __FILE__ << ", " << __LINE__
                << ": Failed to create Name field" << std::endl;
      exit(1);
    }
  }

  // Step 6: Create a line geometry and add points to it
  for (const auto &shape : shapes) {
    // Step 5: Create a new feature
    OGRFeature *feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
    // Step 7: Add the geometry to the feature
    OGRPoint point;
    point.setX(shape.transect_base_point_.x);
    point.setY(shape.transect_base_point_.y);
    feature->SetGeometry(&point);

    set_ogr_feature(shape.get_names(), shape.get_values(), *feature);

    if (layer->CreateFeature(feature) != OGRERR_NONE) {
      std::cerr << "Failed to create feature in shapefile!" << std::endl;
      exit(1);
    }

    OGRFeature::DestroyFeature(feature);
  }

  // Clean up
  GDALClose(dataset);
}

std::vector<std::unique_ptr<TransectLine>> create_transects_from_baseline(
    Baseline &baseline) {
  std::vector<std::unique_ptr<TransectLine>> transect_lines;
  // smoothing the transects
  auto smooth_factor = options.smooth_factor;
  if (options.smooth_factor < 0) {
    throw std::runtime_error("smooth factor should no less than 0");
  }
  auto transect_length = options.transect_length;
  auto mode = options.intersection_mode;
  auto orient = options.transect_orient;
  int transect_id{0};
  for (size_t i = 0; i < baseline.normal_vectors_.size(); i++) {
    size_t start = i;
    size_t num = i + smooth_factor < baseline.normal_vectors_.size()
                     ? smooth_factor
                     : baseline.normal_vectors_.size() - start;

    double x = 0, y = 0;
    for (size_t j = start; j < start + num; j++) {
      x += baseline.normal_vectors_.at(j).first;
      y += baseline.normal_vectors_.at(j).second;
    }
    auto smoothed_normal_vector = std::make_pair(x, y);
    if (smoothed_normal_vector.first == 0 &&
        smoothed_normal_vector.second == 0) {
      smoothed_normal_vector.first = baseline.normal_vectors_.at(i).first;
      smoothed_normal_vector.second = baseline.normal_vectors_.at(i).second;
    }
    transect_lines.emplace_back(std::make_unique<TransectLine>(
        baseline.transects_base_points_.at(i), transect_length,
        smoothed_normal_vector, transect_id++, baseline.baseline_id_, mode,
        orient));
    baseline.baseline_vertices_.push_back(
        transect_lines.at(transect_lines.size() - 1)->transect_ref_point_);
  }
  return transect_lines;
}

void save_transect(const std::vector<std::unique_ptr<TransectLine>> &transects,
                   const std::string &prj) {
  std::vector<TransectLine *> tmp_save;
  tmp_save.reserve(transects.size());
  std::transform(transects.begin(), transects.end(),
                 std::back_inserter(tmp_save),
                 [](const auto &up) { return up.get(); });
  dsas::save_lines(tmp_save, prj.c_str(), dsas::options.intersect_path);
}

}  // namespace dsas