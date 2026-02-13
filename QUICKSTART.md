# Quick Start Guide

## 1. Install Dependencies (5 minutes)

```bash
# System packages
sudo apt-get update
sudo apt-get install -y build-essential cmake git curl zip unzip tar \
    libgl1-mesa-dev xorg-dev libeccodes-dev libeccodes-tools

# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg
./bootstrap-vcpkg.sh
export VCPKG_ROOT=~/vcpkg

# Install ImGui and dependencies via vcpkg
~/vcpkg/vcpkg install imgui[glfw-binding,opengl3-binding] glfw3 opengl
```

## 2. Build the Project (1 minute)

```bash
cd grib-viewer
./build.sh
```

Or manually:

```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build . -j$(nproc)
```

## 3. Get Test Data

```bash
# Use ecCodes sample files
cp /usr/share/eccodes/samples/regular_ll_sfc_grib2.tmpl ~/test.grib2

# Or download real data
wget https://get.ecmwf.int/repository/test-data/grib/sample.grib2
```

## 4. Run

```bash
./build/GribViewer ~/test.grib2
```

## Troubleshooting

Run the dependency checker:
```bash
./check-deps.sh
```

For detailed installation instructions, see:
- `README.md` - Full documentation
- `ECCODES_INSTALL.md` - ecCodes installation guide

## Quick Command Reference

```bash
# Check dependencies
./check-deps.sh

# Build
./build.sh

# Clean and rebuild
rm -rf build && ./build.sh

# Run
./build/GribViewer [grib-file]

# Test ecCodes installation
eccodes_info
grib_ls /usr/share/eccodes/samples/*.tmpl
```
