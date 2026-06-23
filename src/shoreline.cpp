#include "shoreline.hpp"

#include <nlohmann/json.hpp>
#include <shapefil.h>

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>

#ifndef _WIN32
#include <cstring>  // memset — ensure tm is clean before strptime
#else
#include <iomanip>
#include <sstream>
#endif

#include "exception.hpp"

namespace dsas {

double mean_shore_segment = 0;

Date generate_date_from_str(const char *date_str) {
  auto format = dsas::options.date_format;
  std::tm tm{};
#ifndef _WIN32
  // strptime is POSIX: reliable %b/%B/%y support with correct century pivot
  const char *end = strptime(date_str, format.c_str(), &tm);
  if (end == nullptr || *end != '\0') {
    OPENDSAS_THROW("Failed to parse date: " + std::string(date_str) +
                   " using format: " + format);
  }
#else
  std::istringstream ss(date_str);
  ss.imbue(std::locale::classic());
  ss >> std::get_time(&tm, format.c_str());
  if (ss.fail()) {
    OPENDSAS_THROW("Failed to parse date: " + std::string(date_str) +
                   " using format: " + format);
  }
#endif
  return Date{tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday};
}

// ---- GeoJSON reader ----

static std::vector<std::unique_ptr<Shoreline>> load_shorelines_geojson(
    const std::filesystem::path &path, const char *date_field_name) {
  std::ifstream f(path);
  if (!f) OPENDSAS_THROW("Cannot open: " + path.string());
  auto j = nlohmann::json::parse(f);

  std::vector<std::unique_ptr<Shoreline>> shorelines;
  int shoreline_id = 0;

  // Case-insensitive property lookup helper
  auto get_prop = [](const nlohmann::json &props,
                     const std::string &name) -> std::string {
    std::string lower_name = name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
                   ::tolower);
    for (auto &[k, v] : props.items()) {
      std::string lk = k;
      std::transform(lk.begin(), lk.end(), lk.begin(), ::tolower);
      if (lk == lower_name) return v.get<std::string>();
    }
    OPENDSAS_THROW("Date field '" + name + "' not found in shoreline feature");
    return {};
  };

  for (auto &feature : j["features"]) {
    auto date_str = get_prop(feature["properties"], date_field_name);
    auto date = generate_date_from_str(date_str.c_str());

    auto &geom = feature["geometry"];
    std::string gtype = geom["type"].get<std::string>();

    auto read_line = [&](const nlohmann::json &coords) {
      auto sl = std::make_unique<Shoreline>();
      sl->shoreline_vertices_.reserve(coords.size());
      for (auto &c : coords) {
        sl->shoreline_vertices_.emplace_back(c[0].get<double>(),
                                             c[1].get<double>());
      }
      sl->shoreline_id_ = shoreline_id;
      sl->date_ = date;
      return sl;
    };

    if (gtype == "LineString") {
      shorelines.push_back(read_line(geom["coordinates"]));
      ++shoreline_id;
    } else if (gtype == "MultiLineString") {
      for (auto &line : geom["coordinates"]) {
        shorelines.push_back(read_line(line));
      }
      ++shoreline_id;
    } else {
      std::cout << "Unsupported geometry type: " << gtype << "\n";
      ++shoreline_id;
    }
  }
  return shorelines;
}

// ---- Shapefile reader ----

// GCOVR_EXCL_START
static std::vector<std::unique_ptr<Shoreline>> load_shorelines_shapelib(
    const std::filesystem::path &path, const char *date_field_name) {
  const std::string base_path =
      (path.parent_path() / path.stem()).string();

  SHPHandle hSHP = SHPOpen(base_path.c_str(), "rb");
  if (!hSHP) OPENDSAS_THROW("Cannot open shapefile: " + path.string());
  DBFHandle hDBF = DBFOpen(base_path.c_str(), "rb");
  if (!hDBF) {
    SHPClose(hSHP);
    OPENDSAS_THROW("Cannot open DBF: " + path.string());
  }

  int date_idx = DBFGetFieldIndex(hDBF, date_field_name);
  if (date_idx < 0) {
    SHPClose(hSHP);
    DBFClose(hDBF);
    OPENDSAS_THROW("Date field '" + std::string(date_field_name) +
                   "' not found in shoreline shapefile");
  }

  int n_entities = 0;
  SHPGetInfo(hSHP, &n_entities, nullptr, nullptr, nullptr);

  std::vector<std::unique_ptr<Shoreline>> shorelines;
  for (int i = 0; i < n_entities; ++i) {
    const char *date_str = DBFReadStringAttribute(hDBF, i, date_idx);
    auto date = generate_date_from_str(date_str);

    SHPObject *obj = SHPReadObject(hSHP, i);
    if (!obj) continue;

    if (obj->nSHPType == SHPT_ARC || obj->nSHPType == SHPT_ARCZ) {
      for (int p = 0; p < obj->nParts; ++p) {
        int start = obj->panPartStart[p];
        int end = (p + 1 < obj->nParts) ? obj->panPartStart[p + 1]
                                         : obj->nVertices;
        auto sl = std::make_unique<Shoreline>();
        sl->shoreline_vertices_.reserve(static_cast<size_t>(end - start));
        for (int k = start; k < end; ++k) {
          sl->shoreline_vertices_.emplace_back(obj->padfX[k], obj->padfY[k]);
        }
        sl->shoreline_id_ = i;
        sl->date_ = date;
        shorelines.push_back(std::move(sl));
      }
    }
    SHPDestroyObject(obj);
  }

  SHPClose(hSHP);
  DBFClose(hDBF);
  return shorelines;
}
// GCOVR_EXCL_STOP

// ---- Public dispatch ----

std::vector<std::unique_ptr<Shoreline>> load_shorelines_shp(
    const std::filesystem::path &shoreline_shp_path,
    const char *date_field_name) {
  auto ext = shoreline_shp_path.extension().string();
  if (ext == ".geojson" || ext == ".json") {
    return load_shorelines_geojson(shoreline_shp_path, date_field_name);
  }
  return load_shorelines_shapelib(shoreline_shp_path, date_field_name);  // GCOVR_EXCL_LINE
}

}  // namespace dsas
