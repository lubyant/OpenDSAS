//
// Created by lby on 10/13/23.
//

#ifndef SRC_UTILITY_HPP_
#define SRC_UTILITY_HPP_

#include <gdal_priv.h>
#include <ogrsf_frmts.h>

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

double crossProduct(std::vector<double> &vec1, std::vector<double> &vec2);

double calCrossOfTwoVectors(const dsas::Point &p1, const dsas::Point &p2,
                            const dsas::Point &p3, const dsas::Point &p4);

bool testRectangularOfIntersection(const dsas::Point &p1, const dsas::Point &p2,
                                   const dsas::Point &p3,
                                   const dsas::Point &p4);

bool isTwoSegmentIntersected(const dsas::Point &p1, const dsas::Point &p2,
                             const dsas::Point &p3, const dsas::Point &p4);

Point computeIntersectPoint(const Point &p1, const Point &p2, const Point &p3,
                            const Point &p4);

template <size_t I = 0, typename... Args>
typename std::enable_if<I == sizeof...(Args), void>::type set_ogr_feature(
    const std::vector<std::string> &, const std::tuple<Args...> &,
    OGRFeature &) {}
template <size_t I = 0, typename... Args>
typename std::enable_if<I != sizeof...(Args), void>::type set_ogr_feature(
    const std::vector<std::string> &names, const std::tuple<Args...> &values,
    OGRFeature &ogr_feature) {
  ogr_feature.SetField(names[I].c_str(), std::get<I>(values));
  set_ogr_feature<I + 1, Args...>(names, values, ogr_feature);
}

template <typename T>
void save_lines(const std::vector<T> &lines, const char *pszProj,
                const std::filesystem::path &output_path) {
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
  for (size_t i = 0; i < lines[0].get_names().size(); i++) {
    OGRFieldDefn field(lines[0].get_names()[i].c_str(),
                       lines[0].get_types()[i]);
    if (layer->CreateField(&field) != OGRERR_NONE) {
      std::cerr << __FILE__ << ", " << __LINE__
                << ": Failed to create Name field" << std::endl;
      exit(1);
    }
  }

  for (const auto &shape : lines) {
    if (shape.size() < 1) {
      continue;
    }
    // Step 5: Create a new feature
    OGRFeature *feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
    if (!feature) {
      throw std::runtime_error("Failed to create feature");
    }
    OGRLineString line;

    // Step 6: Create a line geometry and add points to it
    for (size_t i = 0; i < shape.size(); i++) {
      if (std::isnan(shape[i].x) || std::isnan(shape[i].y)) {
        std::cerr << shape[i].x << std::endl;
        exit(1);
      }
      line.addPoint(shape[i].x, shape[i].y);
    }

    set_ogr_feature(shape.get_names(), shape.get_values(), *feature);

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

double least_square(const std::vector<long long> &x,
                    const std::vector<double> &y);

std::string get_tiff_proj(const std::string &path);
std::string get_shp_proj(const char *path);
}  // namespace dsas
#endif
