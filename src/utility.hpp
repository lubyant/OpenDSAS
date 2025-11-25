//
// Created by lby on 10/13/23.
//

#ifndef SRC_UTILITY_HPP_
#define SRC_UTILITY_HPP_

#include <gdal_priv.h>
#include <ogrsf_frmts.h>

#include <cassert>
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <future>
#include <iostream>
#include <numeric>
#include <queue>
#include <tuple>
#include <vector>

#include "geometry.hpp"
#include "options.hpp"

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
  double l_x_min = std::min(p1.x, p2.x);
  double l_x_max = std::max(p1.x, p2.x);
  double r_x_min = std::min(p3.x, p4.x);
  double r_x_max = std::max(p3.x, p4.x);
  double l_y_min = std::min(p1.y, p2.y);
  double l_y_max = std::max(p1.y, p2.y);
  double r_y_min = std::min(p3.y, p4.y);
  double r_y_max = std::max(p3.y, p4.y);
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
  assert(lines.size() > 0);
  GDALAllRegister();
  // Step 1: Initialize GDAL
  OGRSpatialReference oSRS;
  if (oSRS.importFromWkt(&pszProj) != OGRERR_NONE) {
    throw std::runtime_error("Projection setting fail!");
  }

  // Step 2: Get the shapefile driver
  GDALDriver *driver =
      GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");

  // Step 3: Create a new shapefile
  GDALDataset *dataset = driver->Create(output_path.string().c_str(), 0, 0, 0,
                                        GDT_Unknown, nullptr);
  if (!dataset) {
    throw std::runtime_error("Failed to create dataset\n");
  }

  // Step 4: Create a layer for the shapefile
  OGRLayer *layer = dataset->CreateLayer("line", &oSRS, wkbLineString, nullptr);
  if (layer == nullptr) {
    throw std::runtime_error("Failed to create layer");
  }

  // define attributes
  for (size_t i = 0; i < lines[0]->get_names().size(); i++) {
    OGRFieldDefn field(lines[0]->get_names()[i].c_str(),
                       lines[0]->get_types()[i]);
    if (layer->CreateField(&field) != OGRERR_NONE) {
      std::cerr << __FILE__ << ", " << __LINE__
                << ": Failed to create Name field" << std::endl;
      exit(1);
    }
  }

  for (const auto *shape : lines) {
    if (shape->size() < 1) {
      continue;
    }
    // Step 5: Create a new feature
    OGRFeature *feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
    if (!feature) {
      throw std::runtime_error("Failed to create feature");
    }
    OGRLineString line;

    // Step 6: Create a line geometry and add points to it
    for (const auto &vertice : *shape) {
      line.addPoint(vertice.get_x(), vertice.get_y());
    }

    set_ogr_feature(shape->get_names(), shape->get_values(), *feature);

    // Step 7: Add the geometry to the feature
    auto err = feature->SetGeometry(&line);
    if (err != OGRERR_NONE) {
      throw std::runtime_error("Failed to set geometry");
    }

    // Step 8: Add the feature to the layer
    err = layer->CreateFeature(feature);
    if (err != OGRERR_NONE) {
      throw std::runtime_error("Failed to set geometry");
    }
    OGRFeature::DestroyFeature(feature);
  }

  // Clean up
  GDALClose(dataset);
}

template <typename T>
requires std::derived_from<T, PointAttribute<double>> &&
         std::derived_from<T, GDALShpSavable<typename T::value_tuple>>
void save_points(const std::vector<T *> &shapes, const char *pszProj,
                 const std::filesystem::path &output_path) {
  assert(shapes.size() > 0);
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
  for (size_t i = 0; i < shapes[0]->get_names().size(); i++) {
    OGRFieldDefn field(shapes[0]->get_names()[i].c_str(),
                       shapes[0]->get_types()[i]);
    if (layer->CreateField(&field) != OGRERR_NONE) {
      std::cerr << __FILE__ << ", " << __LINE__
                << ": Failed to create Name field" << std::endl;
      exit(1);
    }
  }

  // Step 6: Create a line geometry and add points to it
  for (const auto *shape : shapes) {
    // Step 5: Create a new feature
    OGRFeature *feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
    // Step 7: Add the geometry to the feature
    OGRPoint point;
    point.setX(shape->get_x());
    point.setY(shape->get_y());
    feature->SetGeometry(&point);

    set_ogr_feature(shape->get_names(), shape->get_values(), *feature);

    if (layer->CreateFeature(feature) != OGRERR_NONE) {
      std::cerr << "Failed to create feature in shapefile!" << std::endl;
      exit(1);
    }

    OGRFeature::DestroyFeature(feature);
  }

  // Clean up
  GDALClose(dataset);
}

double least_square(const std::vector<long long> &x,
                    const std::vector<double> &y);

std::string get_tiff_proj(const std::string &path);
std::string get_shp_proj(const char *path);
}  // namespace dsas
#endif
