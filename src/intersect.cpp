#include "intersect.hpp"

#include <ogrsf_frmts.h>

#include <cassert>

#include "utility.hpp"

namespace dsas {

void save_intersects(
    const std::vector<std::unique_ptr<IntersectPoint>> &intersects,
    const std::string &prj) {
  std::vector<IntersectPoint *> tmp_save;
  tmp_save.reserve(intersects.size());
  std::transform(intersects.begin(), intersects.end(),
                 std::back_inserter(tmp_save),
                 [](const auto &up) { return up.get(); });
  save_points(tmp_save, prj.c_str(), dsas::options.intersect_path);
}
}  // namespace dsas