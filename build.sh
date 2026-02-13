#!/bin/bash

# GRIB Viewer Build Script
set -e

echo "=== GRIB Viewer Build Script ==="
echo

# Check for vcpkg
if [ -z "$VCPKG_ROOT" ]; then
    echo "VCPKG_ROOT not set. Checking common locations..."
    if [ -d "$HOME/vcpkg" ]; then
        export VCPKG_ROOT=$HOME/vcpkg
        echo "Found vcpkg at $VCPKG_ROOT"
    else
        echo "ERROR: vcpkg not found. Please install vcpkg first:"
        echo "  git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg"
        echo "  cd ~/vcpkg && ./bootstrap-vcpkg.sh"
        echo "  export VCPKG_ROOT=~/vcpkg"
        exit 1
    fi
fi

# Install vcpkg dependencies
echo "Installing vcpkg dependencies..."
$VCPKG_ROOT/vcpkg install imgui[glfw-binding,opengl3-binding] glfw3 opengl

# Check for ecCodes
echo
echo "Checking for ecCodes library..."
if pkg-config --exists eccodes 2>/dev/null; then
    echo "✓ ecCodes found via pkg-config"
    ECCODES_VERSION=$(pkg-config --modversion eccodes)
    echo "  Version: $ECCODES_VERSION"
elif [ -f "/usr/lib/libeccodes.so" ] || [ -f "/usr/local/lib/libeccodes.so" ]; then
    echo "✓ ecCodes library found in system"
else
    echo "✗ ecCodes not found in system"
    echo
    echo "Please install ecCodes:"
    echo "  sudo apt-get install libeccodes-dev libeccodes-tools"
    echo
    echo "Or build from source (see README.md for instructions)"
    echo
    read -p "Continue anyway? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Create build directory
echo
echo "Creating build directory..."
mkdir -p build
cd build

# Configure
echo
echo "Configuring with CMake..."
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release

# Build
echo
echo "Building..."
cmake --build . -j$(nproc)

# Success
echo
echo "=== Build Complete ==="
echo
echo "Executable: $(pwd)/GribViewer"
echo
echo "Run with:"
echo "  ./GribViewer [path-to-grib-file]"
echo
