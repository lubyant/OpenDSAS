# opendsas

Pip-installable wrapper around the [OpenDSAS](https://github.com/lubyant/OpenDSAS) `dsas` CLI —
a high-performance, cross-platform reimplementation of the USGS Digital Shoreline Analysis System.

This package does not compile anything: it bundles the same statically-linked `dsas` binary
published in [GitHub Releases](https://github.com/lubyant/OpenDSAS/releases), and installs a
`dsas` console script that execs it directly.

## Install

```bash
pip install opendsas
```

## Usage

```bash
dsas --baseline baseline.shp --shoreline shoreline.shp \
     --transect-length 1000 --transect-spacing 10
```

See the [main README](https://github.com/lubyant/OpenDSAS#readme) for full CLI documentation.

## Supported platforms

- Linux x86_64 / aarch64
- macOS arm64 (Apple Silicon)
- Windows x86_64
