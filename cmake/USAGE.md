# Using ZZACOMMON with find_package()

This document explains how to use the ZZACOMMON library in your CMake projects using `find_package()`.

## Installation

After building ZZACOMMON, install it using:

```bash
cmake --build build --target install
```

Or set a custom installation prefix:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build --target install
```

## Using ZZACOMMON in Your Project

### Basic Usage

Add the following to your project's CMakeLists.txt:

```cmake
# Find ZZACOMMON package
find_package(zzacommon REQUIRED)

# Link against ZZACOMMON libraries
target_link_libraries(your_target PRIVATE zzacommon::zzaf zzacommon::zzam)
```

### Component-Based Usage

ZZACOMMON supports the following components:

- **zzaf**: FLTK UI components (requires FLTK, nlohmann/json)
- **zzab**: Boost components (requires Boost filesystem, asio)
- **zzax**: XML, URL handling, and FLTK (requires pugixml, libcurl, zlib, FLTK)
- **zzam**: Common utilities (requires nlohmann/json)

#### Example: Using Specific Components

```cmake
cmake_minimum_required(VERSION 3.21)
project(MyApp)

# Find only the components you need
find_package(zzacommon REQUIRED COMPONENTS zzaf zzam)

add_executable(myapp main.cpp)

# Link against the components
target_link_libraries(myapp PRIVATE
    zzacommon::zzaf
    zzacommon::zzam
)
```

#### Example: Optional Components

```cmake
# Find ZZACOMMON with optional components
find_package(zzacommon QUIET COMPONENTS zzaf zzax zzam)

if(zzacommon_FOUND)
    message(STATUS "Found ZZACOMMON components: ${ZZACOMMON_COMPONENTS}")

    add_executable(myapp main.cpp)

    # Link against found components
    target_link_libraries(myapp PRIVATE ${ZZACOMMON_LIBRARIES})
else()
    message(WARNING "ZZACOMMON not found, building without it")
endif()
```

### Variables Set by find_package(zzacommon)

After successfully finding ZZACOMMON, the following variables are available:

| Variable | Description |
|----------|-------------|
| `zzacommon_FOUND` | TRUE if zzacommon is found |
| `zzacommon_VERSION` | Version of zzacommon |
| `ZZACOMMON_INCLUDE_DIR` | Include directory for zzacommon headers |
| `ZZACOMMON_LIBRARIES` | List of imported targets for requested components |
| `ZZACOMMON_COMPONENTS` | List of components that were found |
| `FLTK_INCLUDE_DIR` | FLTK include directory (if zzaf or zzax used) |

### Imported Targets

| Target | Description | Dependencies |
|--------|-------------|--------------|
| `zzacommon::zzaf` | FLTK UI components | FLTK, nlohmann/json |
| `zzacommon::zzab` | Boost components | Boost (filesystem, asio) |
| `zzacommon::zzax` | XML, URL, and FLTK | pugixml, libcurl, zlib, FLTK |
| `zzacommon::zzam` | Common utilities | nlohmann/json |
| `zzacommon::zc_app` | Application metadata | None |

## Setting Up Dependencies

### FLTK (for zzaf and zzax)

On MSVC, you may need to set the FLTK_DIR variable:

```bash
cmake -B build -DFLTK_DIR="C:/path/to/fltk/build"
```

Or in your CMakeLists.txt before find_package:

```cmake
set(FLTK_DIR "C:/path/to/fltk/build")
find_package(zzacommon REQUIRED COMPONENTS zzax)
```

### nlohmann/json (for zzaf and zzam)

On MSVC, the config assumes json is located at `$ENV{USERPROFILE}/source/repos/json/include`.
If it's elsewhere, add it to your include path:

```cmake
target_include_directories(your_target PRIVATE /path/to/json/include)
```

### Using vcpkg

If you're using vcpkg for dependency management:

```bash
# Install dependencies
vcpkg install boost curl zlib

# Configure with vcpkg toolchain
cmake -B build -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Complete Example

```cmake
cmake_minimum_required(VERSION 3.21)
project(MyZZAApp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set FLTK directory if needed (MSVC)
if(MSVC)
    set(FLTK_DIR "C:/fltk/build")
endif()

# Find ZZACOMMON with required components
find_package(zzacommon 1.0.9 REQUIRED COMPONENTS zzaf zzax zzam)

# Create executable
add_executable(myapp
    src/main.cpp
    src/ui.cpp
    src/network.cpp
)

# Link against ZZACOMMON components
target_link_libraries(myapp PRIVATE
    zzacommon::zzaf      # For UI widgets
    zzacommon::zzax      # For XML and network operations
    zzacommon::zzam      # For utilities
    zzacommon::zc_app    # For app metadata
)

# Include directories are automatically added by the imported targets
message(STATUS "Using ZZACOMMON ${zzacommon_VERSION}")
message(STATUS "ZZACOMMON components: ${ZZACOMMON_COMPONENTS}")
```

## Troubleshooting

### zzacommon not found

If CMake cannot find zzacommon, set the installation prefix in CMAKE_PREFIX_PATH:

```bash
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/zzacommon/install
```

### Component not found

If a required component is missing, check that its dependencies are installed:

- **zzaf**: Requires FLTK
- **zzab**: Requires Boost (1.85.0+ on MSVC, 1.83.0+ on other platforms)
- **zzax**: Requires FLTK, pugixml, libcurl, and zlib
- **zzam**: No external dependencies

Check the CMake output for dependency warnings.

### FLTK_DIR not set (MSVC)

Set FLTK_DIR to point to your FLTK build directory:

```bash
cmake -B build -DFLTK_DIR="C:/path/to/fltk/build"
```

## Build from Source as Subproject

Alternatively, you can include ZZACOMMON as a subdirectory:

```cmake
# Add ZZACOMMON as subdirectory
add_subdirectory(external/zzacommon)

# Specify which components to build
set(ZZACOMMON_COMPONENTS "zzaf;zzam" CACHE STRING "" FORCE)

# Link against components
target_link_libraries(myapp PRIVATE zzaf zzam ${ZC_APP_LIBRARY})
```
