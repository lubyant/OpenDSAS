//
// Created by lby on 10/13/23.
//

#ifndef SRC_UTILITY_HPP_
#define SRC_UTILITY_HPP_

#include <gdal_priv.h>
#include <ogrsf_frmts.h>

#include <algorithm>
#include <cassert>
#include <concepts>
#include <filesystem>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "exception.hpp"
#include "geometry.hpp"

namespace dsas {

inline double crossProduct(double ax, double ay, double bx, double by) {
  return ax * by - ay * bx;
}

inline double calCrossOfTwoVectors(const dsas::Point &p1, const dsas::Point &p2,
                                   const dsas::Point &p3,
                                   const dsas::Point &p4) {
  double x1 = p2.x - p1.x;
  double x2 = p4.x - p3.x;
  double y1 = p2.y - p1.y;
  double y2 = p4.y - p3.y;
  return crossProduct(x1, y1, x2, y2);
}

inline bool testRectangularOfIntersection(const dsas::Point &p1,
                                          const dsas::Point &p2,
                                          const dsas::Point &p3,
                                          const dsas::Point &p4) {
  const double l_x_min = std::min(p1.x, p2.x);
  const double l_x_max = std::max(p1.x, p2.x);
  const double r_x_min = std::min(p3.x, p4.x);
  const double r_x_max = std::max(p3.x, p4.x);
  const double l_y_min = std::min(p1.y, p2.y);
  const double l_y_max = std::max(p1.y, p2.y);
  const double r_y_min = std::min(p3.y, p4.y);
  const double r_y_max = std::max(p3.y, p4.y);
  return (l_x_max >= r_x_min) && (r_x_max >= l_x_min) && (r_y_max >= l_y_min) &&
         (l_y_max >= r_y_min);
}

inline bool isTwoSegmentIntersected(const dsas::Point &p1,
                                    const dsas::Point &p2,
                                    const dsas::Point &p3,
                                    const dsas::Point &p4) {
  if (testRectangularOfIntersection(p1, p2, p3, p4)) {
    if ((calCrossOfTwoVectors(p3, p1, p3, p4) *
             calCrossOfTwoVectors(p3, p2, p3, p4) <=
         0) &&
        (calCrossOfTwoVectors(p2, p3, p2, p1) *
             calCrossOfTwoVectors(p2, p4, p2, p1) <=
         0)) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

inline bool computeIntersectPoint(const Point &p1, const Point &p2,
                                  const Point &p3, const Point &p4,
                                  Point &output) noexcept {
  const double x1 = p1.x, y1 = p1.y;
  const double x2 = p2.x, y2 = p2.y;
  const double x3 = p3.x, y3 = p3.y;
  const double x4 = p4.x, y4 = p4.y;
  const double a0 = y1 - y2, b0 = x2 - x1, c0 = x1 * y2 - x2 * y1;
  const double a1 = y3 - y4, b1 = x4 - x3, c1 = x3 * y4 - x4 * y3;
  const double d = a0 * b1 - a1 * b0;
  if (std::abs(d) < 1e-4) {
    std::cout << "two line are the same\n";
    return false;
  } else {
    const double x = (b0 * c1 - b1 * c0) / d;
    const double y = (c0 * a1 - c1 * a0) / d;
    output.x = x;
    output.y = y;
    return true;
  }
}

template <typename... Args>
void set_ogr_feature(const std::vector<std::string> &names,
                     const std::tuple<Args...> &values, OGRFeature &ogr_feature)
requires(sizeof...(Args) > 0)
{
  assert(names.size() == sizeof...(Args));

  std::size_t i = 0;

  std::apply(
      [&](auto &&...vs) {
        // Fold over the parameter pack with side effects
        ((ogr_feature.SetField(names[i++].c_str(),
                               std::forward<decltype(vs)>(vs))),
         ...);
      },
      values);
}

template <typename T>
requires std::derived_from<T, MultiLine<Point>> &&
         std::derived_from<T, GDALShpSavable<typename T::value_tuple>>
void save_lines(const std::vector<T *> &lines, const char *pszProj,
                const std::filesystem::path &output_path) {
  if (lines.empty()) {
    OPENDSAS_THROW("No line to save!");
  }

  // GCOVR_EXCL_START
  OGRSpatialReference oSRS;
  if (oSRS.importFromWkt(&pszProj) != OGRERR_NONE) {
    OPENDSAS_THROW("Projection setting failed");
  }

  GDALDriver *driver =
      GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
  if (!driver) {
    OPENDSAS_THROW("Unable to load ESRI Shapefile driver");
  }

  GDALDataset *dataset = driver->Create(output_path.string().c_str(), 0, 0, 0,
                                        GDT_Unknown, nullptr);
  if (!dataset) {
    OPENDSAS_THROW("Failed to create output shapefile: " +
                   output_path.string());
  }

  try {
    OGRLayer *layer =
        dataset->CreateLayer("line", &oSRS, wkbLineString, nullptr);
    if (!layer) {
      OPENDSAS_THROW("Failed to create layer in shapefile");
    }

    auto names = lines[0]->get_names();
    auto types = lines[0]->get_types();
    for (size_t i = 0; i < names.size(); ++i) {
      OGRFieldDefn field(names[i].c_str(), types[i]);
      if (layer->CreateField(&field) != OGRERR_NONE) {
        OPENDSAS_THROW("Failed to create field: " + names[i]);
      }
    }

    for (const auto *shape : lines) {
      if (shape->size() < 1) continue;

      OGRFeature *feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
      if (!feature) {
        OPENDSAS_THROW("Failed to create new feature");
      }

      OGRLineString line;
      for (const auto &vertice : *shape) {
        line.addPoint(vertice.get_x(), vertice.get_y());
      }

      set_ogr_feature(shape->get_names(), shape->get_values(), *feature);

      if (feature->SetGeometry(&line) != OGRERR_NONE) {
        OGRFeature::DestroyFeature(feature);
        OPENDSAS_THROW("Failed to set geometry");
      }

      if (layer->CreateFeature(feature) != OGRERR_NONE) {
        OGRFeature::DestroyFeature(feature);
        OPENDSAS_THROW("Failed to write feature");
      }

      OGRFeature::DestroyFeature(feature);
    }

    GDALClose(dataset);
  } catch (...) {
    GDALClose(dataset);
    throw;
  }
  // GCOVR_EXCL_STOP
}

template <typename T>
requires std::derived_from<T, PointAttribute<double>> &&
         std::derived_from<T, GDALShpSavable<typename T::value_tuple>>
void save_points(const std::vector<T *> &shapes, const char *pszProj,
                 const std::filesystem::path &output_path) {
  if (shapes.empty()) {
    OPENDSAS_THROW("No point to save!");
  }

  // GCOVR_EXCL_START
  OGRSpatialReference oSRS;
  if (oSRS.importFromWkt(&pszProj) != OGRERR_NONE) {
    OPENDSAS_THROW("Projection setting failed");
  }

  GDALDriver *driver =
      GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
  if (!driver) {
    OPENDSAS_THROW("Unable to load ESRI Shapefile driver");
  }

  GDALDataset *dataset = driver->Create(output_path.string().c_str(), 0, 0, 0,
                                        GDT_Unknown, nullptr);

  if (!dataset) {
    OPENDSAS_THROW("Failed to create output shapefile: " +
                   output_path.string());
  }

  try {
    OGRLayer *layer = dataset->CreateLayer("points", &oSRS, wkbPoint, nullptr);
    if (!layer) {
      OPENDSAS_THROW("Failed to create layer in shapefile");
    }

    // Create fields
    auto names = shapes[0]->get_names();
    auto types = shapes[0]->get_types();
    for (size_t i = 0; i < names.size(); ++i) {
      OGRFieldDefn field(names[i].c_str(), types[i]);
      if (layer->CreateField(&field) != OGRERR_NONE) {
        OPENDSAS_THROW("Failed to create field: " + names[i]);
      }
    }

    // Write features
    for (auto *shape : shapes) {
      OGRFeature *feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
      if (!feature) {
        OPENDSAS_THROW("Failed to create new feature");
      }

      try {
        OGRPoint point(shape->get_x(), shape->get_y());
        feature->SetGeometry(&point);

        set_ogr_feature(shape->get_names(), shape->get_values(), *feature);

        if (layer->CreateFeature(feature) != OGRERR_NONE) {
          OPENDSAS_THROW("Failed to write feature");
        }

        OGRFeature::DestroyFeature(feature);
      } catch (...) {
        OGRFeature::DestroyFeature(feature);
        throw;
      }
    }

    // Success path: close dataset
    GDALClose(dataset);
  } catch (...) {
    GDALClose(dataset);
    throw;
  }
  // GCOVR_EXCL_STOP
}

double least_square(const std::vector<long long> &x,
                    const std::vector<double> &y);

std::string get_tiff_proj(const std::string &path);
std::string get_shp_proj(const char *path);
}  // namespace dsas
#endif
