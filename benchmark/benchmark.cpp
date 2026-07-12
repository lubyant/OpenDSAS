#include <benchmark/benchmark.h>

#include <cmath>
#include <memory>
#include <vector>

#include "baseline.hpp"
#include "dsas.hpp"
#include "geometry.hpp"
#include "grid.hpp"
#include "shoreline.hpp"
#include "transect.hpp"
#include "utility.hpp"

namespace {

// Wavy shorelines stacked along y so a single vertical transect can cross
// all of them - mimics a set of shoreline positions surveyed over time.
std::vector<std::unique_ptr<dsas::Shoreline>> make_shorelines(
    int num_shorelines, int points_per_line, double x_max) {
  std::vector<std::unique_ptr<dsas::Shoreline>> shorelines;
  shorelines.reserve(num_shorelines);
  const double y_spacing = 10.0;
  const double y_center = (num_shorelines - 1) * y_spacing / 2.0;
  for (int s = 0; s < num_shorelines; ++s) {
    std::vector<dsas::Point> pts;
    pts.reserve(points_per_line);
    const double y_base = s * y_spacing - y_center;
    for (int i = 0; i < points_per_line; ++i) {
      const double x = x_max * static_cast<double>(i) /
                       static_cast<double>(points_per_line - 1);
      const double y = y_base + std::sin(x * 0.01) * 3.0;
      pts.emplace_back(x, y);
    }
    dsas::Date date{2000 + s, 1, 1};
    shorelines.push_back(std::make_unique<dsas::Shoreline>(pts, s, date));
  }
  return shorelines;
}

// Evenly spaced vertical transects, long enough to cross every shoreline
// produced by make_shorelines() with the same num_shorelines.
std::vector<std::unique_ptr<dsas::TransectLine>> make_transects(
    int num_transects, int num_shorelines, double x_max) {
  std::vector<std::unique_ptr<dsas::TransectLine>> transects;
  transects.reserve(num_transects);
  const double half_length = num_shorelines * 10.0 + 20.0;
  for (int i = 0; i < num_transects; ++i) {
    const double x =
        x_max * static_cast<double>(i) / static_cast<double>(num_transects);
    dsas::Point start{x, -half_length};
    dsas::Point end{x, half_length};
    transects.push_back(
        std::make_unique<dsas::TransectLine>(start, end, i, /*baseline_id=*/0));
  }
  return transects;
}

}  // namespace

// ---------------------------------------------------------------------------
// Geometry primitives
// ---------------------------------------------------------------------------

static void BM_PointDistanceToPoint(benchmark::State &state) {
  dsas::Point a{0.0, 0.0};
  dsas::Point b{3.0, 4.0};
  for (auto _ : state) {
    benchmark::DoNotOptimize(a.distance_to_point(b));
  }
}
BENCHMARK(BM_PointDistanceToPoint);

static void BM_LineSegmentIsIntersect(benchmark::State &state) {
  dsas::LineSegment seg{{0.0, -5.0}, {0.0, 5.0}};
  dsas::Point p1{-5.0, 0.0}, p2{5.0, 0.0};
  for (auto _ : state) {
    benchmark::DoNotOptimize(seg.is_intersect(p1, p2));
  }
}
BENCHMARK(BM_LineSegmentIsIntersect);

static void BM_LineSegmentFindIntersection(benchmark::State &state) {
  dsas::LineSegment seg{{0.0, -5.0}, {0.0, 5.0}};
  dsas::Point p1{-5.0, 0.0}, p2{5.0, 0.0};
  for (auto _ : state) {
    benchmark::DoNotOptimize(seg.find_intersection(p1, p2));
  }
}
BENCHMARK(BM_LineSegmentFindIntersection);

// ---------------------------------------------------------------------------
// Regression used to compute shoreline change rate
// ---------------------------------------------------------------------------

static void BM_LeastSquare(benchmark::State &state) {
  const auto n = static_cast<size_t>(state.range(0));
  std::vector<long long> x(n);
  std::vector<double> y(n);
  for (size_t i = 0; i < n; ++i) {
    x[i] = static_cast<long long>(i);
    y[i] = static_cast<double>(i) * 1.5 + 2.0;
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(dsas::least_square(x, y));
  }
  state.SetComplexityN(static_cast<int64_t>(n));
}
BENCHMARK(BM_LeastSquare)->RangeMultiplier(4)->Range(8, 8 << 12)->Complexity();

// ---------------------------------------------------------------------------
// Spatial grid index construction
// ---------------------------------------------------------------------------

static void BM_ComputeGridBound(benchmark::State &state) {
  const auto n = static_cast<int>(state.range(0));
  auto shorelines = make_shorelines(n, 200, 2000.0);
  for (auto _ : state) {
    dsas::compute_grid_bound(shorelines);
  }
  state.SetComplexityN(n);
}
BENCHMARK(BM_ComputeGridBound)->RangeMultiplier(4)->Range(1, 256)->Complexity();

static void BM_BuildShorelineIndex(benchmark::State &state) {
  const auto n = static_cast<int>(state.range(0));
  auto shorelines = make_shorelines(n, 200, 2000.0);
  dsas::compute_grid_bound(shorelines);
  for (auto _ : state) {
    auto grids = dsas::build_shoreline_index(shorelines);
    benchmark::DoNotOptimize(grids);
  }
  state.SetComplexityN(n);
}
BENCHMARK(BM_BuildShorelineIndex)->RangeMultiplier(4)->Range(1, 256)->Complexity();

static void BM_BuildTransectIndex(benchmark::State &state) {
  const auto n = static_cast<int>(state.range(0));
  auto shorelines = make_shorelines(4, 200, 2000.0);
  dsas::compute_grid_bound(shorelines);
  for (auto _ : state) {
    state.PauseTiming();
    auto transects = make_transects(n, 4, 2000.0);
    state.ResumeTiming();
    dsas::build_transect_index(transects);
    benchmark::DoNotOptimize(transects);
  }
  state.SetComplexityN(n);
}
BENCHMARK(BM_BuildTransectIndex)->RangeMultiplier(4)->Range(8, 8 << 10)->Complexity();

// ---------------------------------------------------------------------------
// End-to-end intersection pipeline: grid-accelerated vs. brute force.
// The grid index exists to avoid the O(transects * shorelines) scan, so the
// two are benchmarked side by side to make that payoff visible.
// ---------------------------------------------------------------------------

static void BM_GridIntersectionPipeline(benchmark::State &state) {
  const int num_shorelines = static_cast<int>(state.range(0));
  const int num_transects = static_cast<int>(state.range(1));
  constexpr double x_max = 2000.0;

  for (auto _ : state) {
    state.PauseTiming();
    auto shorelines = make_shorelines(num_shorelines, 200, x_max);
    auto transects = make_transects(num_transects, num_shorelines, x_max);
    state.ResumeTiming();

    auto grids = dsas::build_spatial_grids(shorelines, transects);
    auto intersects = dsas::generate_intersects(transects, grids);
    benchmark::DoNotOptimize(intersects);
  }
}
BENCHMARK(BM_GridIntersectionPipeline)
    ->Args({5, 50})
    ->Args({20, 200})
    ->Args({50, 500});

static void BM_BruteForceIntersectionPipeline(benchmark::State &state) {
  const int num_shorelines = static_cast<int>(state.range(0));
  const int num_transects = static_cast<int>(state.range(1));
  constexpr double x_max = 2000.0;

  for (auto _ : state) {
    state.PauseTiming();
    auto shorelines = make_shorelines(num_shorelines, 200, x_max);
    auto transects = make_transects(num_transects, num_shorelines, x_max);
    state.ResumeTiming();

    auto intersects = dsas::generate_intersects(transects, shorelines);
    benchmark::DoNotOptimize(intersects);
  }
}
BENCHMARK(BM_BruteForceIntersectionPipeline)->Args({5, 50})->Args({20, 200});
