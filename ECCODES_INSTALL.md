# ecCodes Installation Guide

ecCodes is ECMWF's library for encoding and decoding meteorological data in GRIB and BUFR formats.

## Method 1: System Package Manager (Easiest)

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y libeccodes-dev libeccodes-tools

# Verify installation
eccodes_info
which grib_ls
pkg-config --modversion eccodes
```

### Fedora/RHEL/CentOS
```bash
sudo yum install eccodes-devel eccodes
# or
sudo dnf install eccodes-devel eccodes
```

### Arch Linux
```bash
sudo pacman -S eccodes
```

## Method 2: Build from Source

This gives you the latest version and full control.

### Prerequisites
```bash
sudo apt-get install -y \
    build-essential \
    cmake \
    gfortran \
    libnetcdf-dev \
    libpng-dev \
    libaec-dev \
    libjpeg-dev
```

### Download and Build
```bash
# Download latest version (check https://confluence.ecmwf.int/display/ECC/Releases)
cd /tmp
wget https://confluence.ecmwf.int/download/attachments/45757960/eccodes-2.33.0-Source.tar.gz
tar -xzf eccodes-2.33.0-Source.tar.gz
cd eccodes-2.33.0-Source

# Create build directory
mkdir build && cd build

# Configure
# Option A: Install to /usr/local (requires sudo)
cmake .. \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DENABLE_PNG=ON \
    -DENABLE_NETCDF=ON \
    -DENABLE_FORTRAN=OFF \
    -DENABLE_PYTHON=OFF

# Option B: Install to user directory (no sudo needed)
cmake .. \
    -DCMAKE_INSTALL_PREFIX=$HOME/local/eccodes \
    -DENABLE_PNG=ON \
    -DENABLE_NETCDF=ON \
    -DENABLE_FORTRAN=OFF \
    -DENABLE_PYTHON=OFF

# Build (use all CPU cores)
make -j$(nproc)

# Test (optional but recommended)
ctest

# Install
sudo make install  # If installing to /usr/local
# OR
make install  # If installing to $HOME/local/eccodes
```

### Post-Installation Setup (for user install)

If you installed to `$HOME/local/eccodes`, add these to your `~/.bashrc`:

```bash
# ecCodes environment
export ECCODES_DIR=$HOME/local/eccodes
export PATH=$ECCODES_DIR/bin:$PATH
export LD_LIBRARY_PATH=$ECCODES_DIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$ECCODES_DIR/lib/pkgconfig:$PKG_CONFIG_PATH
export CMAKE_PREFIX_PATH=$ECCODES_DIR:$CMAKE_PREFIX_PATH
```

Then reload:
```bash
source ~/.bashrc
```

## Method 3: Use Pre-built Project Directory

If you can't install ecCodes system-wide, you can place the library in the project:

```bash
cd grib-viewer
mkdir -p external/eccodes/{include,lib}

# Copy headers
cp -r /path/to/eccodes/include/* external/eccodes/include/

# Copy libraries
cp /path/to/eccodes/lib/libeccodes.so* external/eccodes/lib/

# Or if you built it:
cp -r $HOME/local/eccodes/include/* external/eccodes/include/
cp $HOME/local/eccodes/lib/libeccodes.so* external/eccodes/lib/
```

The CMakeLists.txt will automatically detect and use this location.

## Verification

After installation, verify everything works:

```bash
# Check version
eccodes_info

# List available GRIB keys
grib_dump -D

# Test with sample file
grib_ls /usr/share/eccodes/samples/regular_ll_sfc_grib2.tmpl

# Check library can be found by CMake
pkg-config --cflags eccodes
pkg-config --libs eccodes
```

## Sample GRIB Files for Testing

### From ecCodes installation
```bash
# List sample files
ls -lh /usr/share/eccodes/samples/

# Common samples:
# - regular_ll_sfc_grib2.tmpl  (regular latitude/longitude)
# - regular_gg_sfc_grib2.tmpl  (regular gaussian grid)
```

### Download test data
```bash
mkdir -p ~/grib-data
cd ~/grib-data

# ERA5 sample
wget "https://get.ecmwf.int/repository/test-data/grib/regular_latlon_surface.grib2"

# Or create a simple test file
grib_set -s \
    centre=98,gridType=regular_ll,Ni=360,Nj=181 \
    /usr/share/eccodes/samples/regular_ll_sfc_grib2.tmpl \
    test.grib2
```

### Download real meteorological data

**NOAA GFS (Global Forecast System):**
```bash
# Latest GFS forecast
# Format: https://nomads.ncep.noaa.gov/pub/data/nccf/com/gfs/prod/gfs.YYYYMMDD/HH/atmos/
# Example:
wget "https://nomads.ncep.noaa.gov/pub/data/nccf/com/gfs/prod/gfs.20240101/00/atmos/gfs.t00z.pgrb2.0p25.f000"
```

**ECMWF Test Data:**
```bash
wget "https://get.ecmwf.int/repository/test-data/grib/sample.grib2"
```

## Common Issues

### Library not found at runtime
```bash
# Add library path
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
# Or
sudo ldconfig
```

### CMake can't find ecCodes
```bash
# Set CMAKE_PREFIX_PATH
export CMAKE_PREFIX_PATH=/usr/local:$CMAKE_PREFIX_PATH
# Or specify in cmake command:
cmake .. -DCMAKE_PREFIX_PATH=/usr/local
```

### Missing dependencies during build
```bash
# Install all optional dependencies
sudo apt-get install -y \
    libnetcdf-dev \
    libpng-dev \
    libaec-dev \
    libjpeg-dev \
    libopenjp2-7-dev
```

## Building ecCodes with Different Options

### Minimal build (fastest)
```bash
cmake .. \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DENABLE_PNG=OFF \
    -DENABLE_NETCDF=OFF \
    -DENABLE_FORTRAN=OFF \
    -DENABLE_PYTHON=OFF \
    -DENABLE_EXAMPLES=OFF \
    -DENABLE_TESTS=OFF
```

### Full-featured build
```bash
cmake .. \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DENABLE_PNG=ON \
    -DENABLE_NETCDF=ON \
    -DENABLE_JPG=ON \
    -DENABLE_AEC=ON \
    -DENABLE_FORTRAN=ON \
    -DENABLE_PYTHON=ON
```

## Resources

- **Official Website:** https://confluence.ecmwf.int/display/ECC
- **Documentation:** https://confluence.ecmwf.int/display/ECC/ecCodes+Home
- **GitHub:** https://github.com/ecmwf/eccodes
- **Releases:** https://confluence.ecmwf.int/display/ECC/Releases
- **User Guide:** https://confluence.ecmwf.int/display/ECC/ecCodes+User+Guide

## Summary

**Quick Start (Recommended):**
```bash
# Install via package manager
sudo apt-get install libeccodes-dev libeccodes-tools

# Verify
eccodes_info
pkg-config --modversion eccodes
```

This should work for most Linux distributions and is the easiest method.
