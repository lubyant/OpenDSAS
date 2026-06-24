[![codecov](https://codecov.io/gh/lubyant/OpenDSAS/branch/main/graph/badge.svg)](https://app.codecov.io/gh/lubyant/OpenDSAS)

# OpenDSAS (Digital Shoreline Analysis System)

**OpenDSAS** is a high-performance, cross-platform reimplementation of the USGS  
[Digital Shoreline Analysis System (DSAS)](https://www.usgs.gov/centers/whcmsc/science/digital-shoreline-analysis-system-dsas).  
It is designed for **fast, memory-efficient shoreline change analysis** in command-line workflows.

Compared to the official USGS DSAS:
- 🚀 **Up to 100× faster** execution  
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

### Linux

Download the pre-built static binary from [Releases](https://github.com/lubyant/OpenDSAS/releases):

```bash
# Replace vX.Y with the latest version, e.g. v1.4
curl -LO https://github.com/lubyant/OpenDSAS/releases/download/vX.Y/opendsas-vX.Y-linux-x86_64
chmod +x opendsas-vX.Y-linux-x86_64
sudo mv opendsas-vX.Y-linux-x86_64 /usr/local/bin/dsas
```

The binary is fully statically linked — it runs on any Linux distribution (Ubuntu, Debian, RHEL, Alpine, etc.) with no dependencies.

An `arm64` build (`opendsas-vX.Y-linux-arm64`) is also available for ARM servers and Raspberry Pi.

### macOS

Install via [Homebrew](https://brew.sh) using the OpenDSAS tap:

```bash
brew tap lubyant/opendsas
brew install lubyant/opendsas/opendsas
```

Requires Apple Silicon (M1/M2/M3). An Intel build is not provided.

### Windows

Download `opendsas-vX.Y-windows-x86_64.exe` from [Releases](https://github.com/lubyant/OpenDSAS/releases), rename it to `dsas.exe`, and add it to your `PATH`.

---

### Build from Source

**Prerequisites**
- CMake ≥ 3.14
- C++20-compatible compiler (GCC 11+, Clang 14+, MSVC 2022+)
- macOS only: `brew install libomp`

All other dependencies ([nlohmann/json](https://github.com/nlohmann/json), [shapelib](https://github.com/OSGeo/shapelib), [argparse](https://github.com/p-ranav/argparse)) are fetched automatically by CMake.

```bash
git clone https://github.com/lubyant/OpenDSAS.git
cd OpenDSAS

cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# optional: install to system PATH
sudo cmake --install build
```

---

## ⚙️ Usage

### Supported Input Formats

Both **Shapefile** (`.shp`) and **GeoJSON** (`.geojson` / `.json`) are accepted for all input files.  
All input and output files in a single command must use the **same format** — mixing formats (e.g. a `.geojson` input with a `.shp` output) is an error.

### Quick Example
```bash
# Shapefile inputs
dsas --baseline baseline.shp --shoreline shoreline.shp --transect-length 1000 --transect-spacing 10

# GeoJSON inputs
dsas --baseline baseline.geojson --shoreline shoreline.geojson \
     --output-intersect intersects.geojson --output-transect transects.geojson \
     --transect-length 1000 --transect-spacing 10
```

### Command-Line Options

#### Root Command
| Option                          | Description                                                             | Default          |
| ------------------------------- | ----------------------------------------------------------------------- | ---------------- |
| `-h, --help`                    | Show help message                                                       | —                |
| `-v, --version`                 | Print version info                                                      | —                |
| `--baseline [FILE]`             | Path to baseline file (`.shp` or `.geojson`)                            | —                |
| `--bid-field [STR]`             | Field name for baseline ID in baseline data                             | `id`             |
| `--shoreline [FILE]`            | Path to shoreline file (`.shp` or `.geojson`)                           | —                |
| `--date-field [STR]`            | Field name for date in shoreline data                                   | `Date`           |
| `--date-format [STR]`           | Date format in shoreline data                                           | `%Y/%m/%d`       |
| `--output-intersect [FILE]`     | Output intersections file (must match input format)                     | `intersects.shp` |
| `--output-transect [FILE]`      | Output transects file (must match input format)                         | `transects.shp`  |
| `--smooth-factor [N]`           | Smoothing factor                                                        | `1`              |
| `--transect-length [N]`         | Transect length                                                         | `500`            |
| `--transect-spacing [N]`        | Spacing between transects                                               | `30`             |
| `--intersection-mode [MODE]`    | Intersection rule: `closest` or `farthest`                              | `closest`        |
| `--transect-orientation [MODE]` | Transect orientation: `left`, `right`, or `mix` (half left, half right) | `mix`            |
| `-bi, --build_index`            | Build spatial index (faster queries, slower initial build)              | `false`          |

---

#### Cast Command
Generate transects from a baseline.

<details>
<summary>Click to expand <code>cast</code> options</summary>

| Option                          | Description                                     | Default         |
| ------------------------------- | ----------------------------------------------- | --------------- |
| `-h, --help`                    | Show help message                               | —               |
| `-v, --version`                 | Print version info                              | —               |
| `--baseline [FILE]`             | Path to baseline file (`.shp` or `.geojson`) (**required**) | —               |
| `--bid-field [STR]`             | Field name for baseline ID in baseline data                 | `id`            |
| `--output-transect [FILE]`      | Output transects file (must match input format)             | `transects.shp` |
| `--smooth-factor [N]`           | Smoothing factor                                | `1`             |
| `--transect-length [N]`         | Transect length                                 | `500`           |
| `--transect-spacing [N]`        | Spacing between transects                       | `30`            |
| `--intersection-mode [MODE]`    | Intersection rule: `closest` or `farthest`      | `closest`       |
| `--transect-orientation [MODE]` | Transect orientation: `left`, `right`, or `mix` | `mix`           |

</details>

---

#### Cal Command
Generate intersections and calculate erosion rates using predefined transects.

<details>
<summary>Click to expand <code>cal</code> options</summary>

| Option                          | Description                                                | Default    |
| ------------------------------- | ---------------------------------------------------------- | ---------- |
| `-h, --help`                    | Show help message                                          | —          |
| `-v, --version`                 | Print version info                                         | —          |
| `--transect [FILE]`             | Path to transect file (`.shp` or `.geojson`) (**required**)      | —                |
| `--shoreline [FILE]`            | Path to shoreline file (`.shp` or `.geojson`) (**required**)     | —                |
| `--date-field [STR]`            | Field name for date in shoreline data                             | `Date`           |
| `--date-format [STR]`           | Date format in shoreline data                                     | `%Y/%m/%d`       |
| `--output-intersect [FILE]`     | Output intersections file (must match input format)               | `intersects.shp` |
| `--intersection-mode [MODE]`    | Intersection rule: `closest` or `farthest`                        | `closest`        |
| `--transect-orientation [MODE]` | Transect orientation: `left`, `right`, or `mix`                   | `mix`            |
| `-bi, --build_index`            | Build spatial index (faster queries, slower initial build)        | `false`          |

</details>

---

## 🗂 Preparing Input Data

Input files may be **Shapefile** (`.shp`) or **GeoJSON** (`.geojson` / `.json`).  
All files in one command invocation must use the same format.

### Shoreline File
Must contain a `Date` field (or the name given by `--date-field`):

| Date       |
| ---------- |
| YYYY/MM/DD |

- Example: `2000/01/01`  
- A shoreline ID field is auto-generated (custom IDs not yet supported)

### Baseline File
Must contain an `Id` field (or the name given by `--bid-field`):

| Id          |
| ----------- |
| baseline id |

---

## 📤 Output

OpenDSAS generates two output files (Shapefile by default; format matches your inputs):

### 1. `intersects.shp`
| BaselineId  | TransectId  | ShoreID      | Date              | ref_dist             | X   | Y   |
| ----------- | ----------- | ------------ | ----------------- | -------------------- | --- | --- |
| baseline id | transect id | shoreline id | intersection date | distance to baseline | x   | y   |

### 2. `transects.shp`
| TransectId  | BaselineId  | ChangeRate            |
| ----------- | ----------- | --------------------- |
| transect id | baseline id | shoreline change rate |

---

## 🛠 Roadmap
- [ ] GUI frontend  

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
