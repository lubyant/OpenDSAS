cmake_minimum_required(VERSION 3.14)

project(dsas VERSION 1.2)
add_compile_definitions(PROJECT_NAME_STR="${PROJECT_NAME}" APP_VERSION="${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)


# ========================
# Dependencies
# ========================

# argparse (header-only)
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

# GDAL
find_package(GDAL CONFIG REQUIRED)

# boost
find_package(Boost REQUIRED)

# GoogleTest
add_subdirectory(third_party/googletest)
enable_testing()

# OpenMp
find_package(OpenMP REQUIRED)

# ========================
# Source Files
# ========================

# Gather all source files
file(GLOB_RECURSE ALL_SRC_CPP "src/*.cpp")
file(GLOB_RECURSE ALL_SRC_HPP "src/*.hpp")

# Remove main.cpp from list
list(REMOVE_ITEM ALL_SRC_CPP "${PROJECT_SOURCE_DIR}/src/main.cpp")

# Define static or shared library
add_library(dsas_lib ${ALL_SRC_CPP} ${ALL_SRC_HPP})

target_include_directories(dsas_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(dsas_lib PUBLIC
    argparse
    GDAL::GDAL
    Boost::headers
    OpenMP::OpenMP_CXX
)

# ========================
# Main Executable
# ========================

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    dsas_lib
)

install(TARGETS ${PROJECT_NAME})

# ========================
# Tests
# ========================

file(GLOB TEST_SOURCES "tests/*.test.cpp")

add_executable(dsas_test ${TEST_SOURCES})

target_include_directories(dsas_test PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(dsas_test PRIVATE
    dsas_lib
    gtest
    gtest_main
)

add_test(NAME dsas_test COMMAND dsas_test)
