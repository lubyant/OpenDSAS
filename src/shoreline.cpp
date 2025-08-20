#include "shoreline.hpp"

#include <ogrsf_frmts.h>

#include "baseline.hpp"
#include "grid.hpp"
#include <queue>

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

  return shorelines;
}


}  // namespace dsas