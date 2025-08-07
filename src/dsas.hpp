#ifndef SRC_DSAS_HPP_
#define SRC_DSAS_HPP_

#include "baseline.hpp"
#include "geometry.hpp"
#include "intersect.hpp"
#include "shoreline.hpp"
#include "transect.hpp"

namespace dsas {

std::vector<TransectLine> generate_transects(std::vector<Baseline> &);

std::vector<IntersectPoint> generate_intersects(std::vector<TransectLine> &,
                                                const std::vector<Shoreline> &);

double linearRegressRate(std::vector<IntersectPoint> &intersections);
}  // namespace dsas

#endif