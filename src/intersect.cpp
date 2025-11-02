#include "intersect.hpp"

#include <ogrsf_frmts.h>

#include <cassert>

#include "utility.hpp"

namespace dsas {

// TODO: this function belong to utility.h as a template
static void save_points(const std::vector<IntersectPoint *> &shapes,
                        const char *pszProj,
                        const std::filesystem::path &output_path) {
  assert(shapes.size() > 0);
  // Initialize GDAL
  OGRSpatialReference oSRS;
  if (oSRS.importFromWkt(&pszProj) != OGRERR_NONE) {
    throw std::runtime_error("Projection setting fail!");
  }

  // Get the shapefile driver
  GDALDriver *driver =
      GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
  if (driver == nullptr) {
    throw std::runtime_error("Unable to get ESRI Shapefile driver");
  }

  // Create a new shapefile
  GDALDataset *dataset = driver->Create(output_path.string().c_str(), 0, 0, 0,
                                        GDT_Unknown, nullptr);

  // Step 4: Create a layer for the shapefile
  OGRLayer *layer =
      dataset->CreateLayer("pointLayer", &oSRS, wkbPoint, nullptr);
  if (layer == nullptr) {
    throw std::runtime_error("Layer is not created!");
  }

  // define attributes
  for (size_t i = 0; i < shapes[0]->get_names().size(); i++) {
    OGRFieldDefn field(shapes[0]->get_names()[i].c_str(),
                       shapes[0]->get_types()[i]);
    if (layer->CreateField(&field) != OGRERR_NONE) {
      std::cerr << __FILE__ << ", " << __LINE__
                << ": Failed to create Name field" << std::endl;
      exit(1);
    }
  }

  // Step 6: Create a line geometry and add points to it
  for (const auto *shape : shapes) {
    // Step 5: Create a new feature
    OGRFeature *feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
    // Step 7: Add the geometry to the feature
    OGRPoint point;
    point.setX(shape->x);
    point.setY(shape->y);
    feature->SetGeometry(&point);

    set_ogr_feature(shape->get_names(), shape->get_values(), *feature);

    if (layer->CreateFeature(feature) != OGRERR_NONE) {
      std::cerr << "Failed to create feature in shapefile!" << std::endl;
      exit(1);
    }

    OGRFeature::DestroyFeature(feature);
  }

  // Clean up
  GDALClose(dataset);
}

void save_intersects(
    const std::vector<std::unique_ptr<IntersectPoint>> &intersects,
    const std::string &prj) {
  std::vector<IntersectPoint *> tmp_save;
  tmp_save.reserve(intersects.size());
  std::transform(intersects.begin(), intersects.end(),
                 std::back_inserter(tmp_save),
                 [](const auto &up) { return up.get(); });
  dsas::save_points(tmp_save, prj.c_str(), dsas::options.intersect_path);
}
}  // namespace dsas