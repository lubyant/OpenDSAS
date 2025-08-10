#include "geometry.hpp"

#include "utility.hpp"

namespace dsas {

// for the struct Point
std::ostream &operator<<(std::ostream &os, const Point &point) {
  os << "x: " << point.x << ", y: " << point.y;
  return os;
}
bool operator==(const Point &point1, const Point &point2) {
  return (point1.x == point2.x) && (point1.y == point2.y);
}
bool operator!=(const Point &point1, const Point &point2) {
  return (point1.x != point2.x) || (point1.y != point2.y);
}
[[nodiscard]] double Point::distance_to_point(const Point &point) const {
  return sqrt(pow(x - point.x, 2) + pow(y - point.y, 2));
}
void Point::move_point(std::pair<double, double> orient, double dest) {
  double dist =
      sqrt(orient.first * orient.first + orient.second * orient.second);
  y += (dest * orient.first / dist);
  x += (dest * orient.second / dist);
}
Point Point::create_point(std::pair<double, double> orient, double dest) {
  if (orient.first == 0 && orient.second == 0) {
    return {x, y};
  }
  double dist =
      sqrt(orient.first * orient.first + orient.second * orient.second);
  double y_new = y + dest * orient.first / dist;
  double x_new = x + dest * orient.second / dist;
  return {x_new, y_new};
}

// for the strct LineSegment
int LineSegment::num_lines = 0;

LineSegment::LineSegment(Point leftEdge, Point rightEdge)
    : leftEdge_(leftEdge), rightEdge_(rightEdge) {
  num_lines++;
  double dx = rightEdge.x - leftEdge.x, dy = rightEdge.y - leftEdge.y;
  slope_ = dy / (dx + EPS_OFFSET);
  slope_vector_.first = dy;
  slope_vector_.second = dx;

  normal_vector_.first = slope_vector_.second;
  normal_vector_.second = -slope_vector_.first;
  if (dy >= 0) {
    if (dx >= 0) {
      orient_ = atan(fabs(slope_));
    } else {
      orient_ = atan(fabs(slope_)) + PI / 2;
    }
  } else {
    if (dx >= 0) {
      orient_ = 2 * PI - atan(fabs(slope_));
    } else {
      orient_ = PI + atan(fabs(slope_));
    }
  }
}

void LineSegment::move_line(double dest) {
  if (dest != 0) {
    leftEdge_.move_point(normal_vector_, dest);
    rightEdge_.move_point(normal_vector_, dest);
  }
}

bool LineSegment::is_intersect(const Point &point1, const Point &point2) const {
  return isTwoSegmentIntersected(leftEdge_, rightEdge_, point1, point2);
}

Point LineSegment::find_intersection(const Point &point1,
                                     const Point &point2) const {
  return computeIntersectPoint(leftEdge_, rightEdge_, point1, point2);
}

}  // namespace dsas
