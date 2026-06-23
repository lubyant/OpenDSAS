cmake_minimum_required(VERSION 3.14)

# -------------------------
# Project & global settings
# -------------------------
project(opendsas VERSION 1.4 LANGUAGES C CXX)
add_compile_definitions(PROJECT_NAME_STR="${PROJECT_NAME}" APP_VERSION="${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Library/external prefs
set(BUILD_SHARED_LIBS OFF)

# Nice install dir vars like bin/, lib/
include(GNUInstallDirs)

# -------------------------
# Dependencies
# -------------------------

include(FetchContent)

# argparse (header-only)
FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

# nlohmann/json (header-only, for GeoJSON I/O)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW TRUE
)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(nlohmann_json)

# shapelib (embed source files directly to avoid target-naming complexity)
FetchContent_Declare(
  shapelib
  GIT_REPOSITORY https://github.com/OSGeo/shapelib.git
  GIT_TAG v1.6.1
  GIT_SHALLOW TRUE
)
FetchContent_GetProperties(shapelib)
if(NOT shapelib_POPULATED)
  FetchContent_Populate(shapelib)
endif()

# GDAL (still used by loaders — removed in a later PR)
find_package(GDAL CONFIG REQUIRED)

# OpenMP
find_package(OpenMP REQUIRED)

# -------------------------
# Sources & targets
# -------------------------
# Gather all sources (keep main separate so the lib is main-free)
file(GLOB_RECURSE ALL_SRC_CPP "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE ALL_SRC_HPP "${PROJECT_SOURCE_DIR}/src/*.hpp")
list(REMOVE_ITEM ALL_SRC_CPP "${PROJECT_SOURCE_DIR}/src/main.cpp")

# Core library
add_library(dsas_lib ${ALL_SRC_CPP} ${ALL_SRC_HPP})

# Compile shapelib C sources directly into the library
target_sources(dsas_lib PRIVATE
  ${shapelib_SOURCE_DIR}/shpopen.c
  ${shapelib_SOURCE_DIR}/dbfopen.c
  ${shapelib_SOURCE_DIR}/safileio.c
)

target_include_directories(dsas_lib PUBLIC
  ${PROJECT_SOURCE_DIR}/src
  ${shapelib_SOURCE_DIR}
)
target_link_libraries(dsas_lib PUBLIC
  argparse
  nlohmann_json::nlohmann_json
  GDAL::GDAL
  OpenMP::OpenMP_CXX
)

# CLI executable (keeps name "dsas")
add_executable(dsas src/main.cpp)
target_link_libraries(dsas PRIVATE dsas_lib)

# Install CLI to /usr/bin (via ${CMAKE_INSTALL_BINDIR})
install(TARGETS dsas RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# -------------------------
# Tests (optional)
# -------------------------
include(CTest)  # provides BUILD_TESTING option (ON by default in CTest)
if(BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  file(GLOB TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/*.test.cpp")
  if(TEST_SOURCES)
    add_executable(dsas_test ${TEST_SOURCES})
    target_link_libraries(
      dsas_test PRIVATE dsas_lib GTest::gtest_main
    )
    target_compile_definitions(dsas_test PRIVATE
      TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/test_case"
    )
    include(GoogleTest)
    gtest_discover_tests(dsas_test)
    add_test(NAME dsas_test COMMAND dsas_test)
  endif()
endif()

# -------------------------
# Packaging (.deb via CPack)
# -------------------------
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "opendsas")  # package name shown to apt/dpkg
set(CPACK_PACKAGE_VENDOR "OpenDSAS Project")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "Boyuan Lu blu38@wisc.edu")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenDSAS — high-performance Digital Shoreline Analysis System (CLI)")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")

# Let dpkg-shlibdeps compute runtime shared-library deps automatically
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# Optional: ship docs
# install(FILES LICENSE README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})

include(CPack)
