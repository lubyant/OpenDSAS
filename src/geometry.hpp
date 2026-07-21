//
// Created by lby on 10/20/23.
//

#ifndef SRC_GEOMETRY_HPP_
#define SRC_GEOMETRY_HPP_

#define EPS_OFFSET 1e-6
#define PI 3.1415926

#include <cmath>
#include <compare>
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

struct Date {
  int year_{};
  int month_{};  // 1-12
  int day_{};

  [[nodiscard]] int year() const { return year_; }
  [[nodiscard]] int month() const { return month_; }
  [[nodiscard]] int day() const { return day_; }

  // Standard Julian Day Number for Gregorian calendar (Meeus algorithm).
  // Used as a linear x-axis in regression; only differences matter.
  [[nodiscard]] long long julian_day() const {
    int a = (14 - month_) / 12;
    int y = year_ + 4800 - a;
    int m = month_ + 12 * a - 3;
    return day_ + (153 * m + 2) / 5 + 365LL * y + y / 4 - y / 100 + y / 400 -
           32045;
  }

  auto operator<=>(const Date &) const = default;
};

inline long long days_between(const Date &a, const Date &b) {
  return a.julian_day() - b.julian_day();
}

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

enum class FieldType { Integer, Real, String };

template <typename Tuple>
struct ShpSavable;

template <typename... Args>
struct ShpSavable<std::tuple<Args...>> {
  using value_tuple = std::tuple<Args...>;

  [[nodiscard]] virtual std::vector<std::string> get_names() const = 0;
  [[nodiscard]] virtual std::vector<FieldType> get_types() const = 0;
  [[nodiscard]] virtual value_tuple get_values() const = 0;

  virtual ~ShpSavable() = default;
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