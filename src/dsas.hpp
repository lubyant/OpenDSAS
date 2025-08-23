#ifndef SRC_DSAS_HPP_
#define SRC_DSAS_HPP_

#include <memory>

#include "baseline.hpp"
#include "grid.hpp"
#include "intersect.hpp"
#include "shoreline.hpp"
#include "transect.hpp"

namespace dsas {

std::vector<std::unique_ptr<TransectLine>> generate_transects(
    std::vector<Baseline> &);

std::vector<std::unique_ptr<IntersectPoint>> generate_intersects(
    std::vector<std::unique_ptr<TransectLine>> &,
    const std::vector<std::unique_ptr<Shoreline>> &);

std::vector<std::unique_ptr<IntersectPoint>> generate_intersects(
    std::vector<std::unique_ptr<TransectLine>> &, const Grids &);

Grids build_spatial_grids(const std::vector<std::unique_ptr<Shoreline>> &,
                          std::vector<std::unique_ptr<TransectLine>> &);

double linearRegressRate(std::vector<IntersectPoint *> &intersections);
}  // namespace dsas

#endif