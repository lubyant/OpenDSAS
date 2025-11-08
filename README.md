[![codecov](https://codecov.io/gh/lubyant/OpenDSAS/branch/main/graph/badge.svg)](https://app.codecov.io/gh/lubyant/OpenDSAS)

# OpenDSAS (Digital Shoreline Analysis System)

**OpenDSAS** is a high-performance, cross-platform reimplementation of the USGS  
[Digital Shoreline Analysis System (DSAS)](https://www.usgs.gov/centers/whcmsc/science/digital-shoreline-analysis-system-dsas).  
It is designed for **fast, memory-efficient shoreline change analysis** in command-line workflows.

Compared to the official USGS DSAS:
- 🚀 **Up to 800× faster** execution  
- 💾 Uses only **~5% of the memory**  
- 🖥️ Optimized for **Linux + CLI/HPC environments**

---

## 🔍 Why OpenDSAS?

OpenDSAS is ideal if:
- Your shoreline analysis is **large-scale, time-consuming, or memory-intensive**  
- You prefer **command-line workflows** and Linux HPC environments  
- You need **parallelized computation** on large datasets  

---

## 📦 Installation

### Install from Release (Ubuntu 24.04)
Download the `.deb` package from [Releases](https://github.com/lubyant/OpenDSAS/releases) and install:

```bash
sudo apt install ./OpenDSAS-x.y.z.deb
```

### Build from Source

**Prerequisites**
- Ubuntu 24.04  
- [CMake](https://cmake.org/) ≥ 3.14  
- C++20-compatible compiler  
- [GDAL](https://gdal.org/) ≥ 3.8.5 *(recommended: build from source)*  
- [Boost](https://www.boost.org/)

**Steps**
```bash
git clone https://github.com/lubyant/OpenDSAS.git
cd OpenDSAS

cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# if you want to install as a system cmd
sudo cmake --install build
```

---

## ⚙️ Usage

### Quick Example
```bash
# compute shoreline change in 1000 meter transect with 10 meter spacing
dsas --baseline baseline.shp --shoreline shoreline.shp --transect-length 1000 --transect-spacing 10
```

### Command-Line Options

#### Root Command
| Option                          | Description                                                                                                              | Default          |
|---------------------------------|--------------------------------------------------------------------------------------------------------------------------|------------------|
| `-h, --help`                    | Show help message                                                                                                        | —                |
| `-v, --version`                 | Print version info                                                                                                       | —                |
| `--baseline [FILE]`             | Path to baseline shapefile                                                                                                | —                |
| `--shoreline [FILE]`            | Path to shoreline shapefile                                                                                              | —                |
| `--output-intersect [FILE]`     | Save intersections shapefile                                                                                              | `intersects.shp` |
| `--output-transect [FILE]`      | Save transects shapefile                                                                                                  | `transects.shp`  |
| `--smooth-factor [N]`           | Smoothing factor                                                                                                         | `1`              |
| `--transect-length [N]`         | Transect length                                                                                                          | `500`            |
| `--transect-spacing [N]`        | Spacing between transects                                                                                                | `30`             |
| `--intersection-mode [MODE]`    | Intersection rule: `closest` or `farthest`                                                                               | `closest`        |
| `--transect-orientation [MODE]` | Transect orientation: `left`, `right`, or `mix` (half left, half right)                                                  | `mix`            |
| `-bi, --build_index`            | Build spatial index (faster queries, slower initial build)                                                               | `false`          |

---

#### Cast Command
Generate transects from a baseline.

<details>
<summary>Click to expand <code>cast</code> options</summary>

| Option                          | Description                                                                                                            | Default          |
|---------------------------------|------------------------------------------------------------------------------------------------------------------------|------------------|
| `-h, --help`                    | Show help message                                                                                                      | —                |
| `-v, --version`                 | Print version info                                                                                                     | —                |
| `--baseline [FILE]`             | Path to baseline shapefile (**required**)                                                                              | —                |
| `--output-transect [FILE]`      | Save transects shapefile                                                                                                | `transects.shp`  |
| `--smooth-factor [N]`           | Smoothing factor                                                                                                       | `1`              |
| `--transect-length [N]`         | Transect length                                                                                                        | `500`            |
| `--transect-spacing [N]`        | Spacing between transects                                                                                              | `30`             |
| `--intersection-mode [MODE]`    | Intersection rule: `closest` or `farthest`                                                                             | `closest`        |
| `--transect-orientation [MODE]` | Transect orientation: `left`, `right`, or `mix`                                                                        | `mix`            |

</details>

---

#### Cal Command
Generate intersections and calculate erosion rates using predefined transects.

<details>
<summary>Click to expand <code>cal</code> options</summary>

| Option                          | Description                                                                                                            | Default          |
|---------------------------------|------------------------------------------------------------------------------------------------------------------------|------------------|
| `-h, --help`                    | Show help message                                                                                                      | —                |
| `-v, --version`                 | Print version info                                                                                                     | —                |
| `--transect [FILE]`             | Path to transect shapefile (**required**)                                                                              | —                |
| `--shoreline [FILE]`            | Path to shoreline shapefile (**required**)                                                                             | —                |
| `--intersection-mode [MODE]`    | Intersection rule: `closest` or `farthest`                                                                             | `closest`        |
| `--transect-orientation [MODE]` | Transect orientation: `left`, `right`, or `mix`                                                                        | `mix`            |
| `-bi, --build_index`            | Build spatial index (faster queries, slower initial build)                                                             | `false`          |

</details>

---

## 🗂 Preparing Input Data

### Shoreline Shapefile
Must contain a `Date` field:

| Date       |
|------------|
| YYYY/MM/DD |

- Example: `2000/01/01`  
- A shoreline ID field is auto-generated (custom IDs not yet supported)

### Baseline Shapefile
Must contain an `Id` field:

| Id          |
|-------------|
| baseline id |

---

## 📤 Output

OpenDSAS generates two shapefiles:

### 1. `intersects.shp`
| BaselineId | TransectId | ShoreID | Date       | ref_dist | X    | Y    |
|------------|------------|---------|------------|----------|------|------|
| baseline id | transect id | shoreline id | intersection date | distance to baseline | x | y |

### 2. `transects.shp`
| TransectId | BaselineId | ChangeRate |
|------------|------------|------------|
| transect id | baseline id | shoreline change rate |

---

## 🛠 Roadmap
- [ ] GUI frontend  
- [ ] Distribution packages for other Linux distros  

---

## 📚 Citing OpenDSAS

If you use **OpenDSAS** in your research, please cite it as:

```bibtex
@misc{OpenDSAS,
  author       = {Lu, Boyuan; Wang, wei},
  title        = {OpenDSAS: A High-Performance Digital Shoreline Analysis System},
  year         = {2025},
  publisher    = {GitHub},
  journal      = {GitHub repository},
  howpublished = {\url{https://github.com/lubyant/OpenDSAS}},
}
```

---

## 📜 License
[MIT License](LICENSE)
