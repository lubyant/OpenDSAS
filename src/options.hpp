#ifndef SRC_OPTIONS_HPP_
#define SRC_OPTIONS_HPP_
#include <string>

namespace dsas {

struct Options {
  std::string baseline_path;
  std::string baseline_id_field {"Id"};
  std::string shoreline_path;
  std::string date_field {"DATE"};
  std::string date_format {"%Y/%m/%d"};
  std::string intersect_path{"intersects.shp"};
  std::string transect_path{"transects.shp"};

  enum class IntersectionMode { Closest, Farthest };
  enum class TransectOrientation { Left, Right, Mix };
  int smooth_factor{1};
  double transect_length{500};
  double transect_spacing{30};
  IntersectionMode intersection_mode{IntersectionMode::Closest};
  TransectOrientation transect_orient{TransectOrientation::Mix};

  bool build_index = false;
};

extern Options options;
}  // namespace dsas

#endif