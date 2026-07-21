//
// Created by lby on 10/13/23.
//

#ifndef SRC_UTILITY_HPP_
#define SRC_UTILITY_HPP_

#include <shapefil.h>

#include <algorithm>
#include <cassert>
#include <concepts>
#include <filesystem>
#include <fstream>
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

// Returns true if s looks like WKT or an EPSG code — used to validate
// the projection string before writing .prj files.
inline bool looks_like_proj(const std::string &s) {
  return s.rfind("PROJCS[", 0) == 0 || s.rfind("GEOGCS[", 0) == 0 ||
         s.rfind("EPSG:", 0) == 0;
}

// Overloaded helpers to write a single DBF attribute by C++ type.
inline void write_dbf_field(DBFHandle h, int rec, int fld, int v) {
  DBFWriteIntegerAttribute(h, rec, fld, v);
}
inline void write_dbf_field(DBFHandle h, int rec, int fld, double v) {
  DBFWriteDoubleAttribute(h, rec, fld, v);
}
inline void write_dbf_field(DBFHandle h, int rec, int fld, const char *v) {
  DBFWriteStringAttribute(h, rec, fld, v);
}

template <typename... Args>
void write_dbf_record(DBFHandle h, int rec, const std::tuple<Args...> &values) {
  int fld = 0;
  std::apply(
      [&](auto &&...vs) {
        ((write_dbf_field(h, rec, fld++, std::forward<decltype(vs)>(vs))), ...);
      },
      values);
}

// Map our FieldType enum to shapelib's DBFFieldType + width/decimal defaults.
inline void dbf_add_field(DBFHandle h, const std::string &name, FieldType ft) {
  switch (ft) {
    case FieldType::Integer:
      DBFAddField(h, name.c_str(), FTInteger, 10, 0);
      break;
    case FieldType::Real:
      DBFAddField(h, name.c_str(), FTDouble, 20, 8);
      break;
    case FieldType::String:
      DBFAddField(h, name.c_str(), FTString, 64, 0);
      break;
  }
}

// Write a .prj sidecar (plain-text projection string alongside the .shp).
inline void write_prj(const std::filesystem::path &shp_path,
                      const std::string &prj) {
  auto prj_path = shp_path;
  prj_path.replace_extension(".prj");
  std::ofstream f(prj_path);
  if (f) f << prj;
}

// GCOVR_EXCL_START
template <typename T>
requires std::derived_from<T, MultiLine<Point>> &&
         std::derived_from<T, ShpSavable<typename T::value_tuple>>
void save_lines(const std::vector<T *> &lines, const std::string &prj,
                const std::filesystem::path &output_path) {
  if (lines.empty()) {
    OPENDSAS_THROW("No line to save!");
  }
  if (!looks_like_proj(prj)) {
    OPENDSAS_THROW("Projection setting failed");
  }

  auto base = output_path;
  base.replace_extension("");
  const std::string base_str = base.string();

  SHPHandle hSHP = SHPCreate(base_str.c_str(), SHPT_ARC);
  if (!hSHP) {
    OPENDSAS_THROW("Failed to create shapefile: " + output_path.string());
  }
  DBFHandle hDBF = DBFCreate(base_str.c_str());
  if (!hDBF) {
    SHPClose(hSHP);
    OPENDSAS_THROW("Failed to create DBF: " + output_path.string());
  }

  auto names = lines[0]->get_names();
  auto types = lines[0]->get_types();
  for (size_t i = 0; i < names.size(); ++i) {
    dbf_add_field(hDBF, names[i], types[i]);
  }

  int rec = 0;
  for (const auto *shape : lines) {
    if (shape->size() < 1) continue;
    std::vector<double> xs, ys;
    xs.reserve(shape->size());
    ys.reserve(shape->size());
    for (const auto &pt : *shape) {
      xs.push_back(pt.get_x());
      ys.push_back(pt.get_y());
    }
    SHPObject *obj = SHPCreateSimpleObject(
        SHPT_ARC, static_cast<int>(xs.size()), xs.data(), ys.data(), nullptr);
    SHPWriteObject(hSHP, -1, obj);
    SHPDestroyObject(obj);
    write_dbf_record(hDBF, rec++, shape->get_values());
  }

  SHPClose(hSHP);
  DBFClose(hDBF);
  write_prj(output_path, prj);
}
// GCOVR_EXCL_STOP

template <typename T>
requires std::derived_from<T, PointAttribute<double>> &&
         std::derived_from<T, ShpSavable<typename T::value_tuple>>
void save_points(const std::vector<T *> &shapes, const std::string &prj,
                 const std::filesystem::path &output_path) {
  if (shapes.empty()) {
    OPENDSAS_THROW("No point to save!");
  }
  if (!looks_like_proj(prj)) {
    OPENDSAS_THROW("Projection setting failed");
  }

  // GCOVR_EXCL_START
  auto base = output_path;
  base.replace_extension("");
  const std::string base_str = base.string();

  SHPHandle hSHP = SHPCreate(base_str.c_str(), SHPT_POINT);
  if (!hSHP) {
    OPENDSAS_THROW("Failed to create shapefile: " + output_path.string());
  }
  DBFHandle hDBF = DBFCreate(base_str.c_str());
  if (!hDBF) {
    SHPClose(hSHP);
    OPENDSAS_THROW("Failed to create DBF: " + output_path.string());
  }

  auto names = shapes[0]->get_names();
  auto types = shapes[0]->get_types();
  for (size_t i = 0; i < names.size(); ++i) {
    dbf_add_field(hDBF, names[i], types[i]);
  }

  int rec = 0;
  for (auto *shape : shapes) {
    double x = shape->get_x(), y = shape->get_y();
    SHPObject *obj = SHPCreateSimpleObject(SHPT_POINT, 1, &x, &y, nullptr);
    SHPWriteObject(hSHP, -1, obj);
    SHPDestroyObject(obj);
    write_dbf_record(hDBF, rec++, shape->get_values());
  }

  SHPClose(hSHP);
  DBFClose(hDBF);
  write_prj(output_path, prj);
  // GCOVR_EXCL_STOP
}

double least_square(const std::vector<long long> &x,
                    const std::vector<double> &y);

// Extracts the projection string from a vector file.
// For .shp: reads the adjacent .prj sidecar (returns WKT).
// For .geojson/.json: extracts the "crs"."properties"."name" value (e.g.
// "EPSG:32617").
std::string get_shp_proj(const char *path);
}  // namespace dsas
#endif
