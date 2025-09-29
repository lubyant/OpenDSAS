cmake_minimum_required(VERSION 3.14)

# -------------------------
# Project & global settings
# -------------------------
project(opendsas VERSION 1.2 LANGUAGES CXX)
add_compile_definitions(PROJECT_NAME_STR="${PROJECT_NAME}" APP_VERSION="${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Library/external prefs
set(BUILD_SHARED_LIBS OFF)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# Nice install dir vars like bin/, lib/
include(GNUInstallDirs)

# -------------------------
# Dependencies
# -------------------------

# argparse (header-only)
include(FetchContent)
FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

# GDAL (module mode works on Debian/Ubuntu)
find_package(GDAL CONFIG REQUIRED)

# Boost headers (no specific components)
find_package(Boost REQUIRED)

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
target_include_directories(dsas_lib PUBLIC
  ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(dsas_lib PUBLIC
  argparse          # header-only
  GDAL::GDAL
  Boost::headers
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
  # If you vendor googletest at third_party/googletest
  add_subdirectory(third_party/googletest)

  file(GLOB TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/*.test.cpp")
  if(TEST_SOURCES)
    add_executable(dsas_test ${TEST_SOURCES})
    target_include_directories(dsas_test PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(dsas_test PRIVATE
      dsas_lib
      gtest
      gtest_main
    )
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
