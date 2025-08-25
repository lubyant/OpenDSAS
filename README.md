# DSAS (Digital Shoreline Analysis System)
This is a reimplementation of Digital Shoreline Analysis System in linux/CPP.

## Build Instructions

### Prerequisites

- CMake ≥ 3.14
- C++20 compatible compiler
- GDAL ≥ 3.8.5 (you better build from source, do not use apt install)
- Boost

### Building the Project

```bash
git clone --recurse-submodules https://github.com/lubyant/dsas.git
cd dsas
mkdir build

cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
cmake --install build --prefix /usr/local/
```

### Running DSAS

Basic usage
```bash
dsas --baseline your_baseline.shp --shoreline your_shoreline.shp
```

## TODO (PR welcome)
1. add parallel computation
2. add gui
3. Cross-plaform on MSVC
4. geojson input support

[![codecov](https://codecov.io/gh/USERNAME/REPO/branch/main/graph/badge.svg)](https://codecov.io/gh/lubyant/DSAS)
