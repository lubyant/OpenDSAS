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

  mean_shore_segment = total_length / static_cast<double>(count);
  Grid::grid_size = mean_shore_segment;
  return shorelines;
}
void build_shoreline_index(const std::vector<Shoreline> &shorelines,
                           Grids &grids) {
  // basic sanity
  assert(Grid::grid_size > 0.0);
  assert(Grid::grids_bound_right_top_x > Grid::grids_bound_left_bottom_x);
  assert(Grid::grids_bound_right_top_y > Grid::grids_bound_left_bottom_y);

  const double left_bottom_x = Grid::grids_bound_left_bottom_x;
  const double left_bottom_y = Grid::grids_bound_left_bottom_y;
  const double right_top_x = Grid::grids_bound_right_top_x;
  const double right_top_y = Grid::grids_bound_right_top_y;
  const double grid_size = Grid::grid_size;

  // derive nx, ny from grids or bounds
  int nx = static_cast<int>((right_top_x - left_bottom_x) / grid_size);
  int ny = static_cast<int>((right_top_y - left_bottom_y) / grid_size);
  assert(nx > 0 && ny > 0);

  auto compute_index_x = [&](const double x) {
    int x_index =
        static_cast<int>(std::ceil(x - left_bottom_x) / grid_size) - 1;
    if (x_index < 0) {
      x_index = 0;
    }
    if (x_index >= nx) {
      x_index = nx - 1;
    }
    return x_index;
  };

  auto compute_index_y = [&](const double y) {
    int y_index =
        static_cast<int>(std::ceil(y - left_bottom_y) / grid_size) - 1;
    if (y_index < 0) {
      y_index = 0;
    }
    if (y_index >= ny) {
      y_index = ny - 1;
    }
    return y_index;
  };

  for (size_t si = 0; si < shorelines.size(); ++si) {
    const auto &pts = shorelines[si].shoreline_vertices_;
    if (pts.size() < 2) continue;

    for (size_t j = 0; j + 1 < pts.size(); ++j) {
      const auto &a = pts[j];
      const auto &b = pts[j + 1];

      // segment bbox
      const double xmin = std::min(a.x, b.x);
      const double xmax = std::max(a.x, b.x);
      const double ymin = std::min(a.y, b.y);
      const double ymax = std::max(a.y, b.y);

      // map to inclusive cell range (half-open convention)
      int ix0 = compute_index_x(xmin);
      int ix1 = compute_index_x(xmax);
      int iy0 = compute_index_y(ymin);
      int iy1 = compute_index_y(ymax);

      // if clamped min > max, the bbox doesn't overlap the grid
      if (ix0 > ix1 || iy0 > iy1) continue;

      // insert this segment ID (shoreline si, local segment j) into all
      // overlapped cells
      for (int ix = ix0; ix <= ix1; ++ix) {
        for (int iy = iy0; iy <= iy1; ++iy) {
          grids[static_cast<size_t>(ix)][static_cast<size_t>(iy)]
              .shoreline_segs_indices.emplace_back(si, j);
        }
      }
    }
  }
}
}  // namespace dsas