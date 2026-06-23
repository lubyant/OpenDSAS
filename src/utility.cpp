//
// Created by lby on 10/13/23.
//
#include "utility.hpp"

#include <nlohmann/json.hpp>

#include <cassert>
#include <fstream>
#include <limits>
#include <unordered_set>

#define MAX_DOUBLE (999999.9)
#define MIN_DOUBLE (-999999.9)
namespace dsas {

double least_square(const std::vector<long long> &x,
                    const std::vector<double> &y) {
  if (x.size() != y.size()) {
    return -999.99;
  }
  if (x.empty() || y.empty()) {
    return -999.99;
  }
  double mean_x, mean_y, sum_x = 0, sum_y = 0;
  for (size_t i = 0; i < x.size(); i++) {
    sum_x += x[i];
    sum_y += y[i];
  }
  mean_x = sum_x / static_cast<double>(x.size());
  mean_y = sum_y / static_cast<double>(y.size());

  double var = 0, co_var = 0;
  for (size_t i = 0; i < x.size(); i++) {
    var += (x[i] - mean_x) * (x[i] - mean_x);
    co_var += (x[i] - mean_x) * (y[i] - mean_y);
  }
  if (var == 0) {
    return -999.99;
  }
  return co_var / var;
}

std::string get_shp_proj(const char *path) {
  std::filesystem::path fp(path);
  auto ext = fp.extension().string();

  if (ext == ".shp") {
    // Read the .prj sidecar — it contains the WKT projection string
    auto prj_path = fp;
    prj_path.replace_extension(".prj");
    std::ifstream f(prj_path);
    if (!f) {
      OPENDSAS_THROW("Shapefile has no projection information (missing .prj): " +
                     prj_path.string());
    }
    return std::string(std::istreambuf_iterator<char>(f),
                       std::istreambuf_iterator<char>());
  }

  // GeoJSON / JSON: extract the CRS name from the FeatureCollection
  std::ifstream f(fp);
  if (!f) {
    OPENDSAS_THROW("Cannot open file: " + fp.string());
  }
  auto j = nlohmann::json::parse(f);
  if (!j.contains("crs")) {
    OPENDSAS_THROW("GeoJSON file has no CRS information: " + fp.string());
  }
  return j["crs"]["properties"]["name"].get<std::string>();
}

}  // namespace dsas
