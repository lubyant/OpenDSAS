cmake_minimum_required(VERSION 3.14)

project(dsas)
add_compile_definitions(PROJECT_NAME_STR="${PROJECT_NAME}")

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
find_package(Boost REQUIRED COMPONENTS date_time)

# GoogleTest
add_subdirectory(third_party/googletest)
enable_testing()

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
    Boost::date_time
)

# ========================
# Main Executable
# ========================

add_executable(dsas src/main.cpp)

target_link_libraries(dsas PRIVATE
    dsas_lib
)

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
