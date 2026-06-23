#include "baseline.hpp"
#include "exception.hpp"

#include <nlohmann/json.hpp>
#include <shapefil.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>

namespace dsas {

/**
 *  static members
 */
// cumulative distance from the first object of baselineSeg to last the transect
double BaselineSeg::cumulative_transects_distance = 0;
// cumulative distance  of baselineSeg
double BaselineSeg::cumulative_segment_distance = 0;

BaselineSeg::BaselineSeg(double spacing, const Point &leftEdge,
                         const Point &rightEdge)
    : LineSegment(leftEdge, rightEdge), spacing_(spacing) {
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
  int num = static_cast<int>(floor((p0.distance_to_point(rightEdge) + spacing_) / spacing_));

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

// ---- GeoJSON reader ----

static std::vector<Baseline> load_baselines_geojson(
    const std::filesystem::path &path, const std::string &id_field) {
  std::ifstream f(path);
  if (!f) OPENDSAS_THROW("Cannot open: " + path.string());
  auto j = nlohmann::json::parse(f);

  // Validate requested id_field exists in the first feature (if specified)
  if (!id_field.empty() && !j["features"].empty()) {
    const auto &props = j["features"][0]["properties"];
    bool found = false;
    for (auto &[k, v] : props.items()) {
      if (k == id_field) { found = true; break; }
    }
    if (!found) {
      OPENDSAS_THROW("Field '" + id_field + "' not found in baseline shapefile.");
    }
  }

  std::vector<Baseline> baselines;
  int auto_id = 0;

  for (auto &feature : j["features"]) {
    int baseline_id = auto_id;
    if (!id_field.empty()) {
      baseline_id = feature["properties"][id_field].get<int>();
    }

    auto &geom = feature["geometry"];
    std::string gtype = geom["type"].get<std::string>();

    auto read_line = [&](const nlohmann::json &coords) {
      std::vector<Point> pts;
      pts.reserve(coords.size());
      for (auto &c : coords) {
        pts.emplace_back(c[0].get<double>(), c[1].get<double>());
      }
      return pts;
    };

    if (gtype == "LineString") {
      baselines.emplace_back(read_line(geom["coordinates"]), baseline_id);
    } else if (gtype == "MultiLineString") {
      for (auto &line : geom["coordinates"]) {
        baselines.emplace_back(read_line(line), baseline_id);
      }
    } else {
      std::cout << "Unsupported geometry type: " << gtype << "\n";
      ++auto_id;
      continue;
    }
    ++auto_id;
  }
  return baselines;
}

// ---- Shapefile reader ----

static std::vector<Baseline> load_baselines_shapelib(
    const std::filesystem::path &path, const std::string &id_field) {
  const std::string base = path.stem().string();
  const std::string dir  = path.parent_path().string();
  std::string base_path  = dir.empty() ? base : dir + "/" + base;

  SHPHandle hSHP = SHPOpen(base_path.c_str(), "rb");
  if (!hSHP) OPENDSAS_THROW("Cannot open shapefile: " + path.string());
  DBFHandle hDBF = DBFOpen(base_path.c_str(), "rb");
  if (!hDBF) {
    SHPClose(hSHP);
    OPENDSAS_THROW("Cannot open DBF: " + path.string());
  }

  int id_field_idx = -1;
  if (!id_field.empty()) {
    id_field_idx = DBFGetFieldIndex(hDBF, id_field.c_str());
    if (id_field_idx < 0) {
      SHPClose(hSHP);
      DBFClose(hDBF);
      OPENDSAS_THROW("Field '" + id_field + "' not found in baseline shapefile.");
    }
  }

  int n_entities = 0;
  SHPGetInfo(hSHP, &n_entities, nullptr, nullptr, nullptr);

  std::vector<Baseline> baselines;
  for (int i = 0; i < n_entities; ++i) {
    int baseline_id = i;
    if (id_field_idx >= 0) {
      baseline_id = DBFReadIntegerAttribute(hDBF, i, id_field_idx);
    }
    SHPObject *obj = SHPReadObject(hSHP, i);
    if (!obj) continue;

    if (obj->nSHPType == SHPT_ARC || obj->nSHPType == SHPT_ARCZ) {
      for (int p = 0; p < obj->nParts; ++p) {
        int start = obj->panPartStart[p];
        int end   = (p + 1 < obj->nParts) ? obj->panPartStart[p + 1] : obj->nVertices;
        std::vector<Point> pts;
        pts.reserve(static_cast<size_t>(end - start));
        for (int k = start; k < end; ++k) {
          pts.emplace_back(obj->padfX[k], obj->padfY[k]);
        }
        baselines.emplace_back(pts, baseline_id);
      }
    }
    SHPDestroyObject(obj);
  }

  SHPClose(hSHP);
  DBFClose(hDBF);
  return baselines;
}

// ---- Public dispatch ----

std::vector<Baseline> load_baselines_shp(
    const std::filesystem::path &baseline_shp_path,
    const std::string &baseline_id_field) {
  auto ext = baseline_shp_path.extension().string();
  if (ext == ".geojson" || ext == ".json") {
    return load_baselines_geojson(baseline_shp_path, baseline_id_field);
  }
  return load_baselines_shapelib(baseline_shp_path, baseline_id_field);
}

}  // namespace dsas
