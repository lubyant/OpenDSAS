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
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  // ---------- iterator definition ----------
  struct const_iterator {
    using value_type = T;
    using reference = const T &;
    using pointer = const T *;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    const MultiLine *owner = nullptr;
    size_type index = 0;

    reference operator*() const { return (*owner)[index]; }

    const_iterator &operator++() {
      ++index;
      return *this;
    }

    const_iterator operator++(int) {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const const_iterator &other) const {
      return owner == other.owner && index == other.index;
    }

    bool operator!=(const const_iterator &other) const {
      return !(*this == other);
    }
  };

  // ---------- required virtual interface ----------
  [[nodiscard]] virtual size_type size() const = 0;
  [[nodiscard]] virtual const T &operator[](size_type i) const = 0;
  virtual ~MultiLine() = default;

  // ---------- range support ----------
  [[nodiscard]] const_iterator begin() const { return const_iterator{this, 0}; }

  [[nodiscard]] const_iterator end() const {
    return const_iterator{this, size()};
  }

  [[nodiscard]] const_iterator cbegin() const { return begin(); }
  [[nodiscard]] const_iterator cend() const { return end(); }
};

template <typename Tuple>
struct GDALShpSavable;

template <typename... Args>
struct GDALShpSavable<std::tuple<Args...>> {
  using value_tuple = std::tuple<Args...>;

  [[nodiscard]] virtual std::vector<std::string> get_names() const = 0;
  [[nodiscard]] virtual std::vector<OGRFieldType> get_types() const = 0;
  [[nodiscard]] virtual value_tuple get_values() const = 0;

  virtual ~GDALShpSavable() = default;
};

template <typename T>
requires std::is_arithmetic_v<T>
struct PointAttribute {
  [[nodiscard]] virtual T get_x() const = 0;
  [[nodiscard]] virtual T get_y() const = 0;

  virtual ~PointAttribute() = default;
};

struct Point : public PointAttribute<double> {
  double x, y;

  Point() : x(0), y(0){};
  Point(double x, double y) : x(x), y(y) {}

  friend std::ostream &operator<<(std::ostream &os, const Point &point);

  friend bool operator==(const Point &point1, const Point &point2);
  friend bool operator!=(const Point &point1, const Point &point2);
  [[nodiscard]] double distance_to_point(const Point &point) const;

  void move_point(std::pair<double, double> orient, double dest);

  Point create_point(std::pair<double, double> orient, double dest);

  [[nodiscard]] double get_x() const override { return x; }
  [[nodiscard]] double get_y() const override { return y; }
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