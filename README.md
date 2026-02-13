# A simple GRIB Viewer

A simple C++ application using ImGui to visualize GRIB meteorological data files.

## Features

- Open and read GRIB files using ecCodes library
- Display meteorological fields with color mapping
- Browse through multiple messages in a GRIB file
- Simple and clean ImGui interface

## Prerequisites

### System packages (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git curl zip unzip tar pkg-config
sudo apt-get install -y libgl1-mesa-dev libglu1-mesa-dev xorg-dev
```

### vcpkg Setup
```bash
# Clone vcpkg if not already installed
cd ~
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# Add to PATH (add this to your ~/.bashrc for permanent use)
export VCPKG_ROOT=~/vcpkg
export PATH=$VCPKG_ROOT:$PATH
```

### Install ImGui and dependencies via vcpkg
```bash
~/vcpkg/vcpkg install imgui[glfw-binding,opengl3-binding] glfw3 opengl
```

### Install ecCodes (GRIB library)

ecCodes is ECMWF's library for reading and writing GRIB files.

#### Option 1: Install from system packages (Recommended)
```bash
# Ubuntu/Debian
sudo apt-get install -y libeccodes-dev libeccodes-tools

# Verify installation
eccodes_info
```

#### Option 2: Build from source
```bash
# Download ecCodes
cd /tmp
wget https://confluence.ecmwf.int/download/attachments/45757960/eccodes-2.33.0-Source.tar.gz
tar -xzf eccodes-2.33.0-Source.tar.gz
cd eccodes-2.33.0-Source

# Create build directory
mkdir build && cd build

# Configure and build
cmake .. \
    -DCMAKE_INSTALL_PREFIX=$HOME/local/eccodes \
    -DENABLE_PNG=ON \
    -DENABLE_FORTRAN=OFF

make -j$(nproc)
make install

# If installed to custom location, update CMakeLists.txt or set:
export LD_LIBRARY_PATH=$HOME/local/eccodes/lib:$LD_LIBRARY_PATH
export CMAKE_PREFIX_PATH=$HOME/local/eccodes:$CMAKE_PREFIX_PATH
```

#### Option 3: Manual installation in project (if system install fails)
```bash
cd grib-viewer
mkdir -p external/eccodes
# Copy eccodes library and headers to external/eccodes/lib and external/eccodes/include
# The CMakeLists.txt will automatically detect this location
```

## Building the Project

```bash
cd grib-viewer

# Create build directory
mkdir build && cd build

# Configure with CMake (pointing to vcpkg toolchain)
cmake .. -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build . -j$(nproc)
```

## Running

```bash
# From the build directory
./GribViewer [path-to-grib-file]

# Example:
./GribViewer ~/data/sample.grib2
```

You can also load files through the GUI by entering the path in the text field and clicking "Load".

## Getting Sample GRIB Files

You can download sample GRIB files from:

1. **ECMWF Sample Files:**
   ```bash
   # Sample GRIB files are often included with ecCodes
   ls /usr/share/eccodes/samples/
   
   # Or download from ECMWF test data
   wget https://get.ecmwf.int/repository/test-data/grib/sample.grib2
   ```

2. **NOAA/NCEP:**
   - https://www.ncei.noaa.gov/products/weather-climate-models/global-forecast
   - https://nomads.ncep.noaa.gov/

3. **Generate test data with ecCodes:**
   ```bash
   # If eccodes-tools is installed
   grib_set -s centre=98 /usr/share/eccodes/samples/regular_ll_sfc_grib2.tmpl test.grib2
   ```

## Project Structure

```
grib-viewer/
├── CMakeLists.txt          # CMake build configuration
├── README.md               # This file
├── src/
│   ├── main.cpp           # Application entry point and GUI
│   ├── grib_reader.h      # GRIB file reading interface
│   ├── grib_reader.cpp    # GRIB file reading implementation
│   ├── renderer.h         # Field visualization interface
│   └── renderer.cpp       # Field visualization implementation
└── external/              # External dependencies (if not using system)
    └── eccodes/
        ├── include/
        └── lib/
```

## Troubleshooting

### CMake can't find packages
Make sure you've set the vcpkg toolchain file:
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### ecCodes library not found
- Check if installed: `pkg-config --modversion eccodes`
- If installed to custom location, set: `export CMAKE_PREFIX_PATH=/path/to/eccodes:$CMAKE_PREFIX_PATH`
- Or copy to `external/eccodes/` in project directory

### Runtime library errors
If you get "cannot open shared object file" errors:
```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
# Or wherever eccodes is installed
```

### Display/OpenGL issues
Make sure X11 development packages are installed:
```bash
sudo apt-get install -y libgl1-mesa-dev libglu1-mesa-dev xorg-dev
```

## Dependencies Summary

| Library | Purpose | Installation |
|---------|---------|--------------|
| ImGui | GUI framework | vcpkg |
| GLFW | Window/input handling | vcpkg |
| OpenGL | Graphics rendering | System package |
| ecCodes | GRIB file reading | System package or build from source |

## License

This is a demonstration project. Check the licenses of the individual dependencies:
- ImGui: MIT License
- ecCodes: Apache License 2.0
- GLFW: zlib/libpng License

## Further Development

Possible enhancements:
- Add zoom and pan functionality
- Export visualizations as images
- Support for different color maps
- Display geographical coordinates
- Overlay multiple fields
- Animation of time series
- Advanced interpolation options
