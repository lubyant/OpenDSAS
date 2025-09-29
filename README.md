[![codecov](https://codecov.io/gh/lubyant/OpenDSAS/branch/main/graph/badge.svg)](https://app.codecov.io/gh/lubyant/OpenDSAS)

# OpenDSAS (Digital Shoreline Analysis System)

**OpenDSAS** is a high-performance, Linux-native reimplementation of the USGS  
[Digital Shoreline Analysis System (DSAS)](https://www.usgs.gov/centers/whcmsc/science/digital-shoreline-analysis-system-dsas).  
It is designed to provide a fast, memory-efficient alternative for shoreline change analysis.

Compared to the official USGS DSAS:
- 🚀 **Up to 800× faster** performance
- 💾 **~5% of the memory usage**
- 🖥️ Fully optimized for **command-line workflows on Linux**

---

## Why OpenDSAS?

Use OpenDSAS if:
- Your shoreline analysis is **large-scale, time-consuming, or memory-intensive**.
- You prefer working with the **command line** and Linux-based HPC environments.
- You need a **parallelized** solution for large datasets.

---

## Build Instructions

### Prerequisites
- [CMake](https://cmake.org/) ≥ 3.14  
- C++20 compatible compiler  
- [GDAL](https://gdal.org/) ≥ 3.8.5 (⚠️ recommended: build from source, not `apt install`)  
- [Boost](https://www.boost.org/)

### Build from Source
```bash
git clone --recurse-submodules https://github.com/lubyant/dsas.git
cd dsas
mkdir build

cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
sudo cmake --install build --prefix /usr/local/
```

---

## Usage

### Basic Example
```bash
dsas --baseline baseline.shp --shoreline shoreline.shp
```

### Command-Line Options
| Option                   | Description                                                                 | Default            |
|---------------------------|-----------------------------------------------------------------------------|--------------------|
| `-h, --help`              | Show help message and exit                                                  | —                  |
| `-v, --version`           | Print version information and exit                                          | —                  |
| `--baseline`              | Path to the baseline file (**required**)                                    | —                  |
| `--shoreline`             | Path to the shoreline file (**required**)                                   | —                  |
| `--output-intersect [FILE]` | Path to save the intersection output. If omitted, uses default.            | `intersects.shp`   |
| `--output-transect [FILE]`  | Path to save generated transects. If omitted, uses default.                 | `transects.shp`    |
| `--smooth-factor [N]`     | Smoothing factor for filtering (omit for default)                           | `1`                |
| `--edge-distance [N]`     | Minimum distance from edge (omit for default)                               | `100`              |
| `--transect-length [N]`   | Length of transects (omit for default)                                      | `500`              |
| `--transect-spacing [N]`  | Spacing between transects (omit for default)                                | `30`               |
| `--transect-offset [N]`   | Offset distance for transects (omit for default)                            | `0`                |
| `--intersection-mode [MODE]` | Intersection mode: `closest` or `farthest` (omit for default)            | `closest`          |
| `--transect-orientation [MODE]` | Transect orientation: `left`, `right`, or `mix` (omit for default)    | `mix`              |
| `-bi, --build_index`      | Build spatial index to speed up search                                      | —                  |

---

## Preparing Input Data

### Shoreline Shapefile
Your shoreline shapefile must contain the following attribute table:

| Date       |
|------------|
| YYYY/MM/DD |
| YYYY/MM/DD |

- Dates must be formatted as `YYYY/MM/DD` (e.g., `2000/01/01`).  
- A shoreline ID field will be generated automatically (custom IDs are not yet supported, PRs welcome).

### Baseline Shapefile
Your baseline shapefile must contain the following attribute table:

| Id |
|----|
|  1 |
|  2 |

---

## Output

OpenDSAS generates two shapefiles:

### 1. Intersects (`intersects.shp`)
| BaselineId | TransectId | ShoreID | Date       | ref_dist | X    | Y    |
|------------|------------|---------|------------|----------|------|------|
| baseline id | transect id | shoreline id | intersection date | distance to reference point (from `--intersection-mode`) | x | y |

### 2. Transects (`transects.shp`)
| TransectId | BaselineId | ChangeRate |
|------------|------------|------------|
| transect id | baseline id | shoreline change rate |

---

## Roadmap / TODO (PRs Welcome)
- [ ] Add GUI  
- [ ] Cross-platform support (MSVC / Windows)  
- [ ] Distribute on linux distro  

---

## License
[MIT License](LICENSE)
