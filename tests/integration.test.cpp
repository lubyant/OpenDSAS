#include <gtest/gtest.h>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <iostream>

namespace fs = std::filesystem;

// --------- helper functions ---------

// Portable extraction of exit code from std::system()
static int get_exit_code(int system_ret) {
#ifdef _WIN32
  return system_ret;
#else
  if (WIFEXITED(system_ret))
    return WEXITSTATUS(system_ret);
  return system_ret;
#endif
}

// Create a unique temporary directory for each test
static fs::path make_temp_dir(const std::string& prefix) {
  fs::path base = fs::temp_directory_path();  // /tmp on Linux, %TEMP% on Windows
  fs::path tmp = base / (prefix + "_" + std::to_string(::getpid()));
  fs::create_directories(tmp);
  return tmp;
}

// --------- integration tests ---------

TEST(IntegrationTest, RunsSuccessfullyOnValidArgs) {
  const fs::path dsas_path = DSAS_EXE_PATH;
  const fs::path test_data_dir = TEST_DATA_DIR;

  const fs::path baseline  = test_data_dir / "sample_baseline_offshore.geojson";
  const fs::path shoreline = test_data_dir / "sample_shorelines.geojson";

  // Create isolated temp directory
  const fs::path tmp_dir = make_temp_dir("dsas_integration");
  const fs::path output  = tmp_dir / "output_transects.shp";

  std::cout << "Temp directory: " << tmp_dir << std::endl;

  // Build absolute command with quoted paths
  std::string cmd = "\"" + dsas_path.string() + "\"" +
                    " --baseline \"" + baseline.string() + "\"" +
                    " --shoreline \"" + shoreline.string() + "\"" +
                    " --output-transect \"" + output.string() + "\"" +
                    " --transect-length 4000 --transect-spacing 50";

  int ret = std::system(cmd.c_str());
  int exit_code = get_exit_code(ret);

  EXPECT_EQ(exit_code, 0)
      << "CLI returned non-zero exit code: " << exit_code
      << " (cmd: " << cmd << ")";
  EXPECT_TRUE(fs::exists(output))
      << "Output shapefile not created at " << fs::absolute(output);

  // Optional cleanup
  fs::remove_all(tmp_dir);
}

TEST(IntegrationTest, FailsOnMissingArgs) {
  const fs::path dsas_path = DSAS_EXE_PATH;
  const fs::path tmp_dir = make_temp_dir("dsas_integration_fail");

  // Missing shoreline argument
  std::string cmd = "\"" + dsas_path.string() + "\"" +
                    " --baseline base.shp";

  int ret = std::system(cmd.c_str());
  int exit_code = get_exit_code(ret);

  EXPECT_NE(exit_code, 0)
      << "CLI should have failed but returned 0";

  fs::remove_all(tmp_dir);
}
