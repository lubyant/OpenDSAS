[![codecov](https://codecov.io/gh/USERNAME/REPO/branch/main/graph/badge.svg)](https://codecov.io/gh/lubyant/DSAS)

# OpenDSAS (Digital Shoreline Analysis System)
Open Digital Shoreline Analysis System is a reimplementation of USGS
[DSAS](https://www.usgs.gov/centers/whcmsc/science/digital-shoreline-analysis-system-dsas)
in high-speed code developed in linux. This project targets to provide a
alternative approach of USGS DSAS that allow user to run calculation in linux
platform.

### When should you use our DSAS instead of USGS DSAS?
If your shoreline calculation is time and memory consuming and also you are good at cmd, you could try our program. Our DSAS was developed with high-performance code with parallisms which have 800x speed up than USGS DSAS with 5% memory usage.

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
## How to use DSAS
1. prepare your shoreline shapefile

Your shoreline shapefile should have following attribute tables

| Date | 
|------|
| date of shoreline1|
| date of shoreline1|

Date field should be a formated string as "YYYY/MM/DD" (e.g. 2000/01/01)

2. prepare your baseline shapefile

Your baseline shapefile should have following attribute tables

| Id | 
|------|
| Id of baseline 1|
| Id of baseline 1|

3. command-line options

| Option                   | Description                                                                 | Default            |
|---------------------------|-----------------------------------------------------------------------------|--------------------|
| `-h, --help`              | Show help message and exit                                                  | —                  |
| `-v, --version`           | Print version information and exit                                          | —                  |
| `--baseline`              | Path to the baseline file (**required**)                                    | —                  |
| `--shoreline`             | Path to the shoreline file (**required**)                                   | —                  |
| `--output-intersect` | Path to save the intersection output. If given without a value, uses default. | `intersects.shp`   |
| `--output-transect`  | Path to save the generated transects. If given without a value, uses default. | `transects.shp`    |
| `--smooth-factor`     | Smoothing factor for filtering. If no value is provided, uses default.      | `1`                |
| `--edge-distance`     | Minimum distance from edge. If no value is provided, uses default.          | `100`              |
| `--transect-length`   | Length of transects. If no value is provided, uses default.                 | `500`              |
| `--transect-spacing`  | Spacing between transects. If no value is provided, uses default.           | `30`               |
| `--transect-offset`   | Offset distance for transects. If no value is provided, uses default.       | `0`                |
| `--intersection-mode` | Intersection mode: `closest` or `farthest`. If no value is provided, uses default. | `closest`          |
| `--transect-orientation` | Transect orientation: `left`, `right`, or `mix`. If no value is provided, uses default. | `mix`              |
| `-bi, --build_index`      | Build spatial index to speed up search                                      | —                  |


## TODO (PR welcome)
1. add gui
2. Cross-plaform on MSVC
