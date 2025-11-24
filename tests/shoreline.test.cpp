#include "shoreline.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
using namespace dsas;
#define TOL 1e-4

TEST(TestShoreline, test_load_shorelines_shp) {
  const std::filesystem::path shoreline_shp_path{std::string(TEST_DATA_DIR) +
                                                 "/sample_shorelines.geojson"};
  load_shorelines_shp(shoreline_shp_path, "date");
}

TEST(TestShoreline, test_generate_date_from_str) {
  {
    const char* date_str = "20200115";
    options.date_format = "%Y%m%d";
    auto date = generate_date_from_str(date_str);
    ASSERT_EQ(date.year(), 2020);
    ASSERT_EQ(date.month(), 1);
    ASSERT_EQ(date.day(), 15);
  }
  {
    const char* date_str = "15-01-2020";
    options.date_format = "%d-%m-%Y";
    auto date = generate_date_from_str(date_str);
    ASSERT_EQ(date.year(), 2020);
    ASSERT_EQ(date.month(), 1);
    ASSERT_EQ(date.day(), 15);
  }
  {
    const char* date_str = "01/15/2020";
    options.date_format = "%m/%d/%Y";
    auto date = generate_date_from_str(date_str);
    ASSERT_EQ(date.year(), 2020);
    ASSERT_EQ(date.month(), 1);
    ASSERT_EQ(date.day(), 15);
  }
  {
    const char* date_str = "2020-01-15";
    options.date_format = "%Y-%m-%d";
    auto date = generate_date_from_str(date_str);
    ASSERT_EQ(date.year(), 2020);
    ASSERT_EQ(date.month(), 1);
    ASSERT_EQ(date.day(), 15);
  }
  {
    const char* date_str = "15.Jan.2020";
    options.date_format = "%d.%b.%Y";
    auto date = generate_date_from_str(date_str);
    ASSERT_EQ(date.year(), 2020);
    ASSERT_EQ(date.month(), 1);
    ASSERT_EQ(date.day(), 15);
  }
  {
    const char* date_str = "15.January.2020";
    options.date_format = "%d.%B.%Y";
    auto date = generate_date_from_str(date_str);
    ASSERT_EQ(date.year(), 2020);
    ASSERT_EQ(date.month(), 1);
    ASSERT_EQ(date.day(), 15);
  }
  {
    const char* date_str = "2020/01/15";
    options.date_format = "%Y/%m/%d";
    auto date = generate_date_from_str(date_str);
    ASSERT_EQ(date.year(), 2020);
    ASSERT_EQ(date.month(), 1);
    ASSERT_EQ(date.day(), 15);
  }
  {
    const char* date_str = "15-01-20";
    options.date_format = "%d-%m-%y";
    auto date = generate_date_from_str(date_str);
    ASSERT_EQ(date.year(), 2020);
    ASSERT_EQ(date.month(), 1);
    ASSERT_EQ(date.day(), 15);
  }
  {
    const char* date_str = "01/15/20";
    options.date_format = "%m/%d/%y";
    auto date = generate_date_from_str(date_str);
    ASSERT_EQ(date.year(), 2020);
    ASSERT_EQ(date.month(), 1);
    ASSERT_EQ(date.day(), 15);
  }
  {
    const char* date_str = "20-01-15";
    options.date_format = "%y-%m-%d";
    auto date = generate_date_from_str(date_str);
    ASSERT_EQ(date.year(), 2020);
    ASSERT_EQ(date.month(), 1);
    ASSERT_EQ(date.day(), 15);
  }
  {
    const char* date_str = "201-5";
    options.date_format = "%y-%m-%d";
    ASSERT_THROW(generate_date_from_str(date_str),
                 std::runtime_error);
  }
}