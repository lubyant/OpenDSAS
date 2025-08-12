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

bool testRectangularOfIntersection(const dsas::Point &p1, const dsas::Point &p2,
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

bool isTwoSegmentIntersected(const dsas::Point &p1, const dsas::Point &p2,
                             const dsas::Point &p3, const dsas::Point &p4) {
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

Point computeIntersectPoint(const Point &p1, const Point &p2, const Point &p3,
                            const Point &p4) {
  double x1 = p1.x, y1 = p1.y;
  double x2 = p2.x, y2 = p2.y;
  double x3 = p3.x, y3 = p3.y;
  double x4 = p4.x, y4 = p4.y;
  double a0 = y1 - y2, b0 = x2 - x1, c0 = x1 * y2 - x2 * y1;
  double a1 = y3 - y4, b1 = x4 - x3, c1 = x3 * y4 - x4 * y3;
  double d = a0 * b1 - a1 * b0;
  if (std::abs(d) < 1e-4) {
    throw std::runtime_error("two line are the same");
  } else {
    double x = (b0 * c1 - b1 * c0) / d;
    double y = (c0 * a1 - c1 * a0) / d;
    return {x, y};
  }
}

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