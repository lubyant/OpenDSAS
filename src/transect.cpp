#include "transect.hpp"

#include <nlohmann/json.hpp>
#include <shapefil.h>

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

#include "exception.hpp"
#include "grid.hpp"
#include "intersect.hpp"
#include "options.hpp"
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
      OPENDSAS_THROW("Not a valid orientation!");
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
    if (grids.find(grid_index) == grids.end()) {
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

std::vector<std::unique_ptr<TransectLine>> create_transects_from_baseline(
    Baseline &baseline) {
  std::vector<std::unique_ptr<TransectLine>> transect_lines;
  // smoothing the transects
  auto smooth_factor = options.smooth_factor;
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

// ---- GeoJSON reader ----

static std::vector<std::unique_ptr<TransectLine>> load_transects_geojson(
    const std::filesystem::path &path) {
  std::ifstream f(path);
  if (!f) OPENDSAS_THROW("Cannot open: " + path.string());
  auto j = nlohmann::json::parse(f);

  if (!j.contains("features") || j["features"].empty()) {
    OPENDSAS_THROW("No features found in transect file: " + path.string());
  }

  // Validate required fields in first feature
  const auto &first_props = j["features"][0]["properties"];
  if (!first_props.contains("TransectId")) {
    OPENDSAS_THROW("Need to specify TransectId!");
  }
  if (!first_props.contains("BaselineId")) {
    OPENDSAS_THROW("Need to specify BaselineId!");
  }

  std::vector<std::unique_ptr<TransectLine>> transects;
  for (auto &feature : j["features"]) {
    auto &props = feature["properties"];
    int transect_id = props["TransectId"].get<int>();
    int baseline_id = props["BaselineId"].get<int>();

    auto &coords = feature["geometry"]["coordinates"];
    Point left(coords.front()[0].get<double>(), coords.front()[1].get<double>());
    Point right(coords.back()[0].get<double>(), coords.back()[1].get<double>());

    transects.push_back(std::make_unique<TransectLine>(
        left, right, transect_id, baseline_id,
        options.intersection_mode, options.transect_orient));
  }

  // Derive length and spacing from the loaded transects
  if (transects.size() >= 1) {
    options.transect_length =
        transects[0]->leftEdge_.distance_to_point(transects[0]->rightEdge_);
  }
  if (transects.size() >= 2) {
    options.transect_spacing =
        transects[0]->leftEdge_.distance_to_point(transects[1]->leftEdge_);
  }

  return transects;
}

// ---- Shapefile reader ----

// GCOVR_EXCL_START
static std::vector<std::unique_ptr<TransectLine>> load_transects_shapelib(
    const std::filesystem::path &path) {
  const std::string base_path =
      (path.parent_path() / path.stem()).string();

  SHPHandle hSHP = SHPOpen(base_path.c_str(), "rb");
  if (!hSHP) OPENDSAS_THROW("Cannot open shapefile: " + path.string());
  DBFHandle hDBF = DBFOpen(base_path.c_str(), "rb");
  if (!hDBF) {
    SHPClose(hSHP);
    OPENDSAS_THROW("Cannot open DBF: " + path.string());
  }

  int tid_idx = DBFGetFieldIndex(hDBF, "TransectId");
  if (tid_idx < 0) {
    SHPClose(hSHP); DBFClose(hDBF);
    OPENDSAS_THROW("Need to specify TransectId!");
  }
  int bid_idx = DBFGetFieldIndex(hDBF, "BaselineId");
  if (bid_idx < 0) {
    SHPClose(hSHP); DBFClose(hDBF);
    OPENDSAS_THROW("Need to specify BaselineId!");
  }

  int n_entities = 0;
  SHPGetInfo(hSHP, &n_entities, nullptr, nullptr, nullptr);

  std::vector<std::unique_ptr<TransectLine>> transects;
  for (int i = 0; i < n_entities; ++i) {
    int transect_id = DBFReadIntegerAttribute(hDBF, i, tid_idx);
    int baseline_id = DBFReadIntegerAttribute(hDBF, i, bid_idx);

    SHPObject *obj = SHPReadObject(hSHP, i);
    if (!obj || obj->nVertices < 2) {
      if (obj) SHPDestroyObject(obj);
      continue;
    }
    Point left(obj->padfX[0], obj->padfY[0]);
    Point right(obj->padfX[obj->nVertices - 1], obj->padfY[obj->nVertices - 1]);
    SHPDestroyObject(obj);

    transects.push_back(std::make_unique<TransectLine>(
        left, right, transect_id, baseline_id,
        options.intersection_mode, options.transect_orient));
  }

  SHPClose(hSHP);
  DBFClose(hDBF);

  if (transects.size() >= 1) {
    options.transect_length =
        transects[0]->leftEdge_.distance_to_point(transects[0]->rightEdge_);
  }
  if (transects.size() >= 2) {
    options.transect_spacing =
        transects[0]->leftEdge_.distance_to_point(transects[1]->leftEdge_);
  }

  return transects;
}
// GCOVR_EXCL_STOP

// ---- Public dispatch ----

std::vector<std::unique_ptr<TransectLine>> load_transects_from_shp(
    const std::filesystem::path &transect_shp_path) {
  auto ext = transect_shp_path.extension().string();
  if (ext == ".geojson" || ext == ".json") {
    return load_transects_geojson(transect_shp_path);
  }
  return load_transects_shapelib(transect_shp_path);  // GCOVR_EXCL_LINE
}

void save_transect(const std::vector<std::unique_ptr<TransectLine>> &transects,
                   const std::string &prj, bool save_as_point) {
  std::vector<TransectLine *> tmp_save;
  tmp_save.reserve(transects.size());
  std::transform(transects.begin(), transects.end(),
                 std::back_inserter(tmp_save),
                 [](const auto &up) { return up.get(); });
  if (save_as_point) {
    save_points(tmp_save, prj, dsas::options.transect_path);
  } else {
    save_lines(tmp_save, prj, dsas::options.transect_path);
  }
}

}  // namespace dsas
