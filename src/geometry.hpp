//
// Created by lby on 10/20/23.
//

#ifndef SRC_GEOMETRY_HPP_
#define SRC_GEOMETRY_HPP_

#define EPS_OFFSET 1e-6
#define PI 3.1415926

#include <gdal_priv.h>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <cmath>
#include <filesystem>
#include <memory>
#include <optional>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

#include "options.hpp"

// classes
namespace dsas {

template <typename T>
struct MultiLine {
  [[nodiscard]] virtual size_t size() const = 0;

  [[nodiscard]] virtual const T &operator[](size_t i) const = 0;

  virtual ~MultiLine() = default;
};

template <typename... Arg>
struct GDALShpSaver {
  [[nodiscard]] virtual std::vector<std::string> get_names() const = 0;
  [[nodiscard]] virtual std::vector<OGRFieldType> get_types() const = 0;
  [[nodiscard]] virtual std::tuple<Arg...> get_values() const = 0;
  virtual ~GDALShpSaver() = default;
};

struct Point {
  double x, y;

  Point() : x(0), y(0) {};
  Point(double x, double y) : x(x), y(y) {}

  friend std::ostream &operator<<(std::ostream &os, const Point &point);

  friend bool operator==(const Point &point1, const Point &point2);
  friend bool operator!=(const Point &point1, const Point &point2);
  [[nodiscard]] double distance_to_point(const Point &point) const;

  void move_point(std::pair<double, double> orient, double dest);

  Point create_point(std::pair<double, double> orient, double dest);
};

struct LineSegment {
  Point leftEdge_, rightEdge_;
  double slope_, orient_;
  static int num_lines;
  std::pair<double, double> slope_vector_, normal_vector_;  // {y, x}

  // constructor
  LineSegment(Point leftEdge, Point rightEdge);

  // destructor
  virtual ~LineSegment() { num_lines--; }

  // print
  friend std::ostream &operator<<(std::ostream &os,
                                  const LineSegment &lineSedsasent) {
    os << "left edge: " << lineSedsasent.leftEdge_
       << ", right edge: " << lineSedsasent.rightEdge_;
    return os;
  }

  // move the current line by distant in normal direction
  void move_line(double dest);

  [[nodiscard]] bool is_intersect(const Point &point1,
                                  const Point &point2) const;

  [[nodiscard]] std::optional<Point> find_intersection(
      const Point &point1, const Point &point2) const;
};

}  // namespace dsas

#endif  // SHORELINECALCULATOR_GEOMETRY_HPP