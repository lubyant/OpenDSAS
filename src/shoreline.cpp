#include "shoreline.hpp"

#include <ogrsf_frmts.h>

#include "baseline.hpp"

namespace dsas {

double mean_shore_segment = 0;

boost::gregorian::date generate_date_from_str(const char *date_str) {
  std::string date_string = std::string(date_str);
  std::stringstream ss(date_string);
  std::string sub_str;
  std::vector<std::string> y_m_d;
  while (std::getline(ss, sub_str, '/')) {
    y_m_d.push_back(sub_str);
  }
  if (y_m_d.size() != 3) {
    std::cerr << "Shoreline date field is not formated in YYYY/MM/dd\n";
    exit(1);
  }

  boost::gregorian::date g_date(std::stoi(y_m_d[0]), std::stoi(y_m_d[1]),
                                std::stoi(y_m_d[2]));
  return g_date;
}

std::vector<Shoreline> load_shorelines_shp(
    const std::filesystem::path &shoreline_shp_path,
    const char *date_field_name) {
  std::vector<Shoreline> shorelines;
  int shoreline_id{0};

  // Initialize GDAL
  GDALAllRegister();

  // Open the Shapefile
  GDALDataset *poDS;
  poDS = static_cast<GDALDataset *>(GDALOpenEx(
      shoreline_shp_path.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
  if (poDS == nullptr) {
    std::cerr << "Open failed.\n";
    exit(1);
  }

  // Get the Layer Containing the Line Features
  OGRLayer *poLayer;
  poLayer = poDS->GetLayer(0);

  // Iterate Through the Features in the Layer and Access Points
  OGRFeature *poFeature;
  poLayer->ResetReading();
  double prev_x, prev_y;
  double cur_x, cur_y;
  double total_length = 0;
  size_t count = 0;
  while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
    OGRGeometry *poGeometry;
    poGeometry = poFeature->GetGeometryRef();
    const char *date_field = poFeature->GetFieldAsString(date_field_name);
    auto date = generate_date_from_str(date_field);
    if (poGeometry != nullptr) {
      auto gtype = wkbFlatten(poGeometry->getGeometryType());
      Shoreline shoreline;
      if (gtype == wkbLineString) {
        OGRLineString *poLine = dynamic_cast<OGRLineString *>(poGeometry);
        for (int i = 0; i < poLine->getNumPoints(); i++) {
          OGRPoint point;
          poLine->getPoint(i, &point);
          shoreline.shoreline_vertices_.emplace_back(point.getX(),
                                                     point.getY());
          if (i == 0) {
            prev_x = point.getX();
            prev_y = point.getY();
          } else {
            cur_x = point.getX();
            cur_y = point.getY();

            total_length += std::sqrt((cur_x - prev_x) * (cur_x - prev_x) +
                                      (cur_y - prev_y) * (cur_y - prev_y));
            count++;
            prev_x = cur_x;
            prev_y = cur_y;
          }
          map_bound.min_x = std::min(map_bound.min_x, point.getX());
          map_bound.max_x = std::max(map_bound.max_x, point.getX());
          map_bound.min_y = std::min(map_bound.min_y, point.getY());
          map_bound.max_y = std::max(map_bound.max_y, point.getY());
        }
        shoreline.shoreline_id_ = shoreline_id++;
        shoreline.date_ = date;
        shorelines.push_back(std::move(shoreline));
      } else if (gtype == wkbMultiLineString) {
        OGRMultiLineString *poMulti =
            dynamic_cast<OGRMultiLineString *>(poGeometry);
        for (int j = 0; j < poMulti->getNumGeometries(); j++) {
          Shoreline shoreline;
          OGRGeometry *subGeom = poMulti->getGeometryRef(j);
          OGRLineString *poLine = dynamic_cast<OGRLineString *>(subGeom);
          for (int i = 0; i < poLine->getNumPoints(); i++) {
            OGRPoint point;
            poLine->getPoint(i, &point);
            shoreline.shoreline_vertices_.emplace_back(point.getX(),
                                                       point.getY());
          }
          shoreline.shoreline_id_ = shoreline_id;
          shoreline.date_ = date;
          shorelines.push_back(std::move(shoreline));
        }
        shoreline_id++;
      } else {
        std::cout << "Unsupported geometry type.\n";
        OGRFeature::DestroyFeature(poFeature);
        continue;
      }
    }
    OGRFeature::DestroyFeature(poFeature);
  }

  // Cleanup
  GDALClose(poDS);

  mean_shore_segment = total_length / count;
  grid_size = mean_shore_segment;
  return shorelines;
}
void build_shoreline_index(const std::vector<Shoreline> &shorelines,
                           Grids &grids) {
  for (size_t i = 0; i < shorelines.size(); i++) {
    for (size_t j = 0; j < shorelines[i].shoreline_vertices_.size() - 1; j++) {
      const double min_x{std::min(shorelines[i].shoreline_vertices_[j].x,
                                  shorelines[i].shoreline_vertices_[j + 1].x)};
      const double max_x{std::max(shorelines[i].shoreline_vertices_[j].x,
                                  shorelines[i].shoreline_vertices_[j + 1].x)};
      const double min_y{std::min(shorelines[i].shoreline_vertices_[j].y,
                                  shorelines[i].shoreline_vertices_[j + 1].y)};
      const double max_y{std::max(shorelines[i].shoreline_vertices_[j].y,
                                  shorelines[i].shoreline_vertices_[j + 1].y)};
      size_t nx_min{(min_x - Grid::grids_bound_left_bottom_x) / grid_size};
      size_t nx_max{(max_x - Grid::grids_bound_left_bottom_x) / grid_size};
      size_t ny_min{(min_y - Grid::grids_bound_left_bottom_y) / grid_size};
      size_t ny_max{(max_y - Grid::grids_bound_left_bottom_y) / grid_size};
      for (size_t nx = nx_min; nx <= nx_max; nx++) {
        for (size_t ny = ny_min; ny <= ny_max; ny++) {
          grids[nx][ny].shoreline_segs_indices.push_back({i, j});
        }
      }
    }
  }
}
}  // namespace dsas