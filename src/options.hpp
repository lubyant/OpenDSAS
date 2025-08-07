#ifndef SRC_OPTIONS_HPP_
#define SRC_OPTIONS_HPP_
#include <filesystem>

namespace dsas {

struct Options {
  std::string baseline_path;
  std::string shoreline_path;
  std::string intersect_path {"intersects.shp"};
  std::string transect_path {"transects.shp"};

  enum class IntersectionMode { Closest, Farthest };
  enum class TransectOrientation { Left, Right, Mix };
  int smooth_factor{1};
  int edge_distance{100};
  double transect_length{500};
  double transect_spacing{30};
  double transect_offset{0};
  IntersectionMode intersection_mode{IntersectionMode::Closest};
  TransectOrientation transect_orient{TransectOrientation::Mix};
};

extern Options options;
}  // namespace dsas

#endif