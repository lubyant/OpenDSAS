//
// Created by lby on 10/13/23.
//
#include "utility.hpp"

#include <cassert>
#include <limits>
#include <unordered_set>

#define MAX_DOUBLE (999999.9)
#define MIN_DOUBLE (-999999.9)
namespace dsas {

double least_square(const std::vector<long long> &x,
                    const std::vector<double> &y) {
  if (x.size() != y.size()) {
    throw std::runtime_error("x, y need to have the same size!");
  }
  if (x.empty() || y.empty()) {
    return -999.99;
  }
  double mean_x, mean_y, sum_x = 0, sum_y = 0;
  for (size_t i = 0; i < x.size(); i++) {
    sum_x += x[i];
    sum_y += y[i];
  }
  mean_x = sum_x / static_cast<double>(x.size());
  mean_y = sum_y / static_cast<double>(y.size());

  double var = 0, co_var = 0;
  for (size_t i = 0; i < x.size(); i++) {
    var += (x[i] - mean_x) * (x[i] - mean_x);
    co_var += (x[i] - mean_x) * (y[i] - mean_y);
  }
  if (var == 0) {
    return -999.99;
  }
  return co_var / var;
}

std::string get_tiff_proj(const std::string &path) {
  GDALAllRegister();
  auto *poTIFFDataset =
      static_cast<GDALDataset *>(GDALOpen(path.c_str(), GA_ReadOnly));
  if (poTIFFDataset == nullptr) {
    throw std::runtime_error("Not available tiff!");
  }
  std::string psz_prj_ = std::string(poTIFFDataset->GetProjectionRef());
  GDALClose(poTIFFDataset);
  return psz_prj_;
}

std::string get_shp_proj(const char *path) {
  GDALAllRegister();

  GDALDataset *poDS;
  poDS = static_cast<GDALDataset *>(
      GDALOpenEx(path, GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
  if (poDS == nullptr) {
    throw std::runtime_error("shapefile not open!");
  }

  OGRLayer *poLayer = poDS->GetLayer(0);
  OGRSpatialReference *poSRS = poLayer->GetSpatialRef();
  std::string psz_prj_;
  if (poSRS != nullptr) {
    char *pszProjection;
    poSRS->exportToWkt(&pszProjection);
    psz_prj_ = std::string(pszProjection);
    CPLFree(pszProjection);
  } else {
    throw std::runtime_error("No spatial reference information available\n");
  }
  return psz_prj_;
}
}  // namespace dsas