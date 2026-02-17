set(VERBOSE=1)
cmake_minimum_required(VERSION 3.20)
project(GribViewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Compiler flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Warning flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -march=native
        -fopenmp
    )
    
    # Debug flags
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    
    # Release flags
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4)
endif()

# Position Independent Code (for shared libraries)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find vcpkg packages
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# eccodes library (GRIB library)
# We'll link to system-installed eccodes or build from source
find_library(ECCODES_LIB NAMES eccodes PATHS /usr/lib /usr/local/lib)
find_path(ECCODES_INCLUDE NAMES eccodes.h PATHS /usr/include /usr/local/include)

if(NOT ECCODES_LIB OR NOT ECCODES_INCLUDE)
    message(STATUS "eccodes not found in system, will use manual installation")
    set(ECCODES_INCLUDE "${CMAKE_SOURCE_DIR}/external/eccodes/include")
    set(ECCODES_LIB "${CMAKE_SOURCE_DIR}/external/eccodes/lib/libeccodes.so")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/grib_reader.cpp
    src/renderer.cpp
    src/mpl_gradients.cpp
	src/settings.cpp
)

set(HEADERS
    src/grib_reader.h
    src/renderer.h
    src/mpl_gradients.h
    src/gradient.h
    src/settings.h
	src/messagesWindow.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

set_source_files_properties(
    src/your_colormap_file.cpp
    PROPERTIES
    COMPILE_FLAGS "-O0 -g0"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${ECCODES_INCLUDE}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui::imgui
    glfw
    OpenGL::GL
    ${ECCODES_LIB}
    m
    pthread
    gomp
)

# Copy shaders or resources if needed
file(COPY ${CMAKE_SOURCE_DIR}/README.md DESTINATION ${CMAKE_BINARY_DIR})
