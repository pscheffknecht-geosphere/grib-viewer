#!/bin/bash

# Dependency Checker for GRIB Viewer
echo "=== GRIB Viewer Dependency Checker ==="
echo

MISSING_DEPS=0

# Check build tools
echo "Checking build tools..."
for tool in cmake g++ make git; do
    if command -v $tool &> /dev/null; then
        VERSION=$($tool --version | head -n1)
        echo "  ✓ $tool: $VERSION"
    else
        echo "  ✗ $tool: NOT FOUND"
        MISSING_DEPS=1
    fi
done
echo

# Check vcpkg
echo "Checking vcpkg..."
if [ -n "$VCPKG_ROOT" ] && [ -d "$VCPKG_ROOT" ]; then
    echo "  ✓ VCPKG_ROOT: $VCPKG_ROOT"
    if [ -f "$VCPKG_ROOT/vcpkg" ]; then
        echo "  ✓ vcpkg executable found"
    else
        echo "  ✗ vcpkg executable not found"
        MISSING_DEPS=1
    fi
elif [ -d "$HOME/vcpkg" ]; then
    echo "  ! VCPKG_ROOT not set, but found at $HOME/vcpkg"
    echo "    Run: export VCPKG_ROOT=$HOME/vcpkg"
    export VCPKG_ROOT=$HOME/vcpkg
else
    echo "  ✗ vcpkg not found"
    echo "    Install: git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg"
    echo "             cd ~/vcpkg && ./bootstrap-vcpkg.sh"
    MISSING_DEPS=1
fi
echo

# Check vcpkg packages
echo "Checking vcpkg packages..."
if [ -n "$VCPKG_ROOT" ] && [ -f "$VCPKG_ROOT/vcpkg" ]; then
    for pkg in imgui glfw3 opengl; do
        if $VCPKG_ROOT/vcpkg list | grep -q "^$pkg"; then
            echo "  ✓ $pkg installed"
        else
            echo "  ✗ $pkg not installed"
            echo "    Install: $VCPKG_ROOT/vcpkg install $pkg"
            MISSING_DEPS=1
        fi
    done
fi
echo

# Check ecCodes
echo "Checking ecCodes..."
ECCODES_FOUND=0

# Check via pkg-config
if pkg-config --exists eccodes 2>/dev/null; then
    VERSION=$(pkg-config --modversion eccodes)
    CFLAGS=$(pkg-config --cflags eccodes)
    LIBS=$(pkg-config --libs eccodes)
    echo "  ✓ ecCodes found via pkg-config"
    echo "    Version: $VERSION"
    echo "    CFLAGS: $CFLAGS"
    echo "    LIBS: $LIBS"
    ECCODES_FOUND=1
fi

# Check system libraries
for path in /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu; do
    if [ -f "$path/libeccodes.so" ]; then
        echo "  ✓ libeccodes.so found at $path"
        ECCODES_FOUND=1
    fi
done

# Check headers
for path in /usr/include /usr/local/include; do
    if [ -f "$path/eccodes.h" ]; then
        echo "  ✓ eccodes.h found at $path"
        ECCODES_FOUND=1
    fi
done

# Check tools
if command -v eccodes_info &> /dev/null; then
    echo "  ✓ eccodes_info tool found"
    ECCODES_FOUND=1
fi

if [ $ECCODES_FOUND -eq 0 ]; then
    echo "  ✗ ecCodes not found"
    echo "    Install: sudo apt-get install libeccodes-dev libeccodes-tools"
    echo "    Or see ECCODES_INSTALL.md for build instructions"
    MISSING_DEPS=1
fi
echo

# Check OpenGL
echo "Checking OpenGL..."
if pkg-config --exists gl 2>/dev/null; then
    echo "  ✓ OpenGL found"
else
    if [ -f "/usr/lib/x86_64-linux-gnu/libGL.so" ] || [ -f "/usr/lib/libGL.so" ]; then
        echo "  ✓ OpenGL library found"
    else
        echo "  ✗ OpenGL not found"
        echo "    Install: sudo apt-get install libgl1-mesa-dev"
        MISSING_DEPS=1
    fi
fi
echo

# Check X11 (for GLFW)
echo "Checking X11 development files..."
if pkg-config --exists x11 2>/dev/null; then
    echo "  ✓ X11 found"
else
    echo "  ✗ X11 development files not found"
    echo "    Install: sudo apt-get install xorg-dev"
    MISSING_DEPS=1
fi
echo

# Summary
echo "=== Summary ==="
if [ $MISSING_DEPS -eq 0 ]; then
    echo "✓ All dependencies found! Ready to build."
    echo
    echo "Next steps:"
    echo "  ./build.sh"
    echo "  # or manually:"
    echo "  mkdir build && cd build"
    echo "  cmake .. -DCMAKE_TOOLCHAIN_FILE=\$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
    echo "  cmake --build ."
    exit 0
else
    echo "✗ Some dependencies are missing. Please install them first."
    echo
    echo "Quick install (Ubuntu/Debian):"
    echo "  sudo apt-get install build-essential cmake git libgl1-mesa-dev xorg-dev"
    echo "  sudo apt-get install libeccodes-dev libeccodes-tools"
    echo "  git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg"
    echo "  cd ~/vcpkg && ./bootstrap-vcpkg.sh"
    echo "  export VCPKG_ROOT=~/vcpkg"
    echo "  ~/vcpkg/vcpkg install imgui[glfw-binding,opengl3-binding] glfw3 opengl"
    exit 1
fi
