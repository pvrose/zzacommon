# Using zzacommon with Runtime DLLs

When using zzacommon components that depend on external libraries (zlib, Boost), you need to ensure that the required DLLs are available:
1. In your application's **build directory** during development
2. In your application's **installation directory** when deploying

## Components and Their DLL Dependencies

- **zzax**: Requires `zlib1.dll` (or `zlib1d.dll` for debug)
- **zzab**: Requires Boost DLLs (`boost_filesystem*.dll`, `boost_system*.dll`)
- **zzafb**: Requires both FLTK and Boost DLLs

## Build-Time: Copying DLLs to Build Directory

zzacommon provides helper functions to automatically copy runtime DLLs to your application's build output directory during development:

### Quick Start - Copy All DLLs (Recommended)

```cmake
find_package(zzacommon REQUIRED COMPONENTS zzax zzab)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE zzacommon::zzax zzacommon::zzab)

# Automatically copy ALL required DLLs to build directory (zlib + Boost)
zzacommon_copy_runtime_dlls(myapp)
```

### Copy Specific DLLs

If you only want to copy specific DLLs during build:

```cmake
find_package(zzacommon REQUIRED COMPONENTS zzax zzab)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE zzacommon::zzax zzacommon::zzab)

# Copy only zlib DLL
zzacommon_copy_zlib_dll(myapp)

# Copy only Boost DLLs
zzacommon_copy_boost_dlls(myapp)
```

## Install-Time: Installing DLLs with Your Application

When you install your application (via `cmake --install` or CPack), you also need to install the runtime DLLs:

### Quick Start - Install All DLLs (Recommended)

```cmake
find_package(zzacommon REQUIRED COMPONENTS zzax zzab)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE zzacommon::zzax zzacommon::zzab)

# Copy DLLs to build directory
zzacommon_copy_runtime_dlls(myapp)

# Install your application
install(TARGETS myapp
    RUNTIME DESTINATION bin
    COMPONENT application
)

# Install all required runtime DLLs
zzacommon_install_runtime_dlls(DESTINATION bin COMPONENT application)
```

### Custom Install Configuration

You can customize the installation destination and component:

```cmake
# Install to a specific directory
zzacommon_install_runtime_dlls(DESTINATION "myapp/bin")

# Install with a specific component name for CPack
zzacommon_install_runtime_dlls(DESTINATION bin COMPONENT runtime_dependencies)

# Default: DESTINATION is "bin", COMPONENT is "runtime"
zzacommon_install_runtime_dlls()
```

### Usage with FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    zzacommon
    GIT_REPOSITORY https://github.com/pvrose/zzacommon
    GIT_TAG master
)

# Request the zzax component
set(ZZACOMMON_COMPONENTS "zzax" CACHE STRING "")
FetchContent_MakeAvailable(zzacommon)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE zzacommon::zzax)

# Copy zlib DLL
if(ZZACOMMON_ZLIB_DLL_RELEASE OR ZZACOMMON_ZLIB_DLL_DEBUG)
    # For FetchContent, the DLL paths are exported to parent scope
    if(ZZACOMMON_ZLIB_DLL_RELEASE AND EXISTS "${ZZACOMMON_ZLIB_DLL_RELEASE}")
        add_custom_command(TARGET myapp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ZZACOMMON_ZLIB_DLL_RELEASE}"
                "$<TARGET_FILE_DIR:myapp>"
        )
    endif()
    if(ZZACOMMON_ZLIB_DLL_DEBUG AND EXISTS "${ZZACOMMON_ZLIB_DLL_DEBUG}")
        add_custom_command(TARGET myapp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ZZACOMMON_ZLIB_DLL_DEBUG}"
                "$<TARGET_FILE_DIR:myapp>"
        )
    endif()
endif()
```

## Available Variables

When using components with external dependencies, the following variables are exported:

### zlib (zzax component)
- `ZZACOMMON_ZLIB_DLL_RELEASE` - Path to the release version of zlib1.dll
- `ZZACOMMON_ZLIB_DLL_DEBUG` - Path to the debug version of zlib1d.dll (if available)

### Boost (zzab, zzafb components)
- `ZZACOMMON_BOOST_DLLS_RELEASE` - List of paths to release Boost DLLs
- `ZZACOMMON_BOOST_DLLS_DEBUG` - List of paths to debug Boost DLLs (if available)

## Manual DLL Location

If you prefer to manage DLLs manually, you can find them in your vcpkg installation:

### zlib DLLs
- **Release**: `${VCPKG_ROOT}/installed/x64-windows/bin/zlib1.dll`
- **Debug**: `${VCPKG_ROOT}/installed/x64-windows/debug/bin/zlib1d.dll`

### Boost DLLs
- **Release**: `${VCPKG_ROOT}/installed/x64-windows/bin/boost_*.dll`
- **Debug**: `${VCPKG_ROOT}/installed/x64-windows/debug/bin/boost_*.dll`

## Troubleshooting

If DLLs are not being copied:

1. Verify that the required components are properly configured and built
2. Check that `VCPKG_ROOT` is set correctly
3. Ensure dependencies are installed via vcpkg:
   - `vcpkg install zlib:x64-windows` (for zzax)
   - `vcpkg install boost-filesystem:x64-windows boost-system:x64-windows` (for zzab/zzafb)
4. Check the CMake configuration output for messages about DLL detection

The configuration process will output messages like:
```
-- zzacommon: Found ZLIB release DLL: C:/path/to/vcpkg/installed/x64-windows/bin/zlib1.dll
-- zzacommon: Found Boost release DLLs: ...boost_filesystem-vc143-mt-x64-1_85.dll;...boost_system-vc143-mt-x64-1_85.dll
-- zzacommon: Will copy zlib1.dll (Release) to myapp output directory
-- zzacommon: Will copy Boost release DLLs to myapp output directory
```

## FetchContent Support

When using FetchContent, the DLL paths are exported to parent scope, allowing you to use the same helper functions or access the variables directly for custom copy commands.

## Function Reference

### Build-Time Functions

#### zzacommon_copy_runtime_dlls(target_name)

Copies all required runtime DLLs (zlib + Boost) to the target's output directory during build.

**Parameters:**
- `target_name` - The name of your executable or library target

**Example:**
```cmake
add_executable(myapp main.cpp)
zzacommon_copy_runtime_dlls(myapp)
```

**When to use:** During development to ensure DLLs are available in the build directory for running/debugging.

---

#### zzacommon_copy_zlib_dll(target_name)

Copies only zlib DLL(s) to the target's output directory.

**Parameters:**
- `target_name` - The name of your executable or library target

**Example:**
```cmake
zzacommon_copy_zlib_dll(myapp)
```

---

#### zzacommon_copy_boost_dlls(target_name)

Copies only Boost DLL(s) to the target's output directory.

**Parameters:**
- `target_name` - The name of your executable or library target

**Example:**
```cmake
zzacommon_copy_boost_dlls(myapp)
```

### Install-Time Functions

#### zzacommon_install_runtime_dlls([DESTINATION dir] [COMPONENT name])

Installs all required runtime DLLs (zlib + Boost) to the specified installation directory.

**Parameters:**
- `DESTINATION` (optional) - Installation directory. Default: `bin`
- `COMPONENT` (optional) - CMake install component name. Default: `runtime`

**Examples:**
```cmake
# Default installation to bin directory
zzacommon_install_runtime_dlls()

# Custom destination
zzacommon_install_runtime_dlls(DESTINATION "myapp/bin")

# Custom component for CPack
zzacommon_install_runtime_dlls(DESTINATION bin COMPONENT dependencies)

# Both custom
zzacommon_install_runtime_dlls(DESTINATION lib COMPONENT runtime_libs)
```

**When to use:** In your CMakeLists.txt after defining `install(TARGETS ...)` commands to ensure DLLs are included in the installation package.

**Note:** This function uses CMake's `install(FILES ...)` command, so the DLLs will be installed when you run:
- `cmake --install .`
- `make install`
- CPack to create installers

---

## Complete Example

Here's a complete example showing both build-time and install-time DLL handling:

```cmake
cmake_minimum_required(VERSION 3.21)
project(MyApplication)

# Find zzacommon
find_package(zzacommon REQUIRED COMPONENTS zzax zzab)

# Create executable
add_executable(myapp
    main.cpp
    app.cpp
)

# Link zzacommon components
target_link_libraries(myapp PRIVATE
    zzacommon::zzax
    zzacommon::zzab
)

# Copy DLLs to build directory for development
zzacommon_copy_runtime_dlls(myapp)

# Install the application
install(TARGETS myapp
    RUNTIME DESTINATION bin
    COMPONENT application
)

# Install runtime DLLs alongside the application
zzacommon_install_runtime_dlls(
    DESTINATION bin
    COMPONENT application
)

# Optional: Create installation package
include(CPack)
set(CPACK_GENERATOR "ZIP;NSIS")
```

After building, you can:
- **Run locally:** `./build/Debug/myapp.exe` (DLLs are in the same directory)
- **Install:** `cmake --install build --prefix install_dir`
- **Create package:** `cpack --config build/CPackConfig.cmake`
