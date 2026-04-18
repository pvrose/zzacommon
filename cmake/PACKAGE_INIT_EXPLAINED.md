# Understanding @PACKAGE_INIT@ in CMake Config Files

## What is @PACKAGE_INIT@?

`@PACKAGE_INIT@` is a special macro placeholder that gets expanded by CMake's `configure_package_config_file()` function (from the `CMakePackageConfigHelpers` module). It provides essential functionality for creating relocatable package configuration files.

## What Does @PACKAGE_INIT@ Expand To?

When `configure_package_config_file()` processes your `.cmake.in` file, it replaces `@PACKAGE_INIT@` with code that:

### 1. Sets PACKAGE_PREFIX_DIR
Calculates the installation prefix directory based on where the config file is installed:
```cmake
# Sets PACKAGE_PREFIX_DIR to the install prefix
# e.g., if config is at /usr/local/lib/cmake/zzacommon/zzacommonConfig.cmake
# then PACKAGE_PREFIX_DIR = /usr/local
```

### 2. Provides Helper Macros

#### `set_and_check(VAR PATH)`
Sets a variable and verifies the path exists:
```cmake
set_and_check(ZZACOMMON_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
# Equivalent to:
#   set(ZZACOMMON_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
#   if(NOT EXISTS "${ZZACOMMON_INCLUDE_DIR}")
#       message(FATAL_ERROR "File not found: ${ZZACOMMON_INCLUDE_DIR}")
#   endif()
```

#### `check_required_components(PACKAGE_NAME)`
Validates that all required components were found:
```cmake
check_required_components(zzacommon)
# Checks all zzacommon_FIND_REQUIRED_<component> flags
```

### 3. Enables Relocatable Packages
The key benefit is that paths are calculated relative to where the package is actually installed, not hardcoded. This means:

```cmake
# Without @PACKAGE_INIT@ (hardcoded paths - breaks when moved):
set(ZZACOMMON_INCLUDE_DIR "C:/Program Files/zzacommon/include")

# With @PACKAGE_INIT@ (relocatable - works anywhere):
set_and_check(ZZACOMMON_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
```

## Why Use configure_package_config_file()?

### Before (using configure_file):
```cmake
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/MyConfig.cmake"
    "${CMAKE_BINARY_DIR}/MyConfig.cmake"
    @ONLY
)
```
**Problems:**
- Paths are hardcoded to build directory
- No validation of paths
- Not relocatable
- `@PACKAGE_INIT@` is just removed (empty substitution)

### After (using configure_package_config_file):
```cmake
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/MyConfig.cmake.in"
    "${CMAKE_BINARY_DIR}/MyConfig.cmake"
    INSTALL_DESTINATION lib/cmake/mypackage
    PATH_VARS CMAKE_INSTALL_PREFIX
)
```
**Benefits:**
- `@PACKAGE_INIT@` expands to useful helper code
- Automatic path calculation
- Built-in validation with `set_and_check()`
- Package is relocatable

## Example Expansion

### Input File (zzacommonConfig.cmake.in):
```cmake
@PACKAGE_INIT@

set_and_check(ZZACOMMON_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
set_and_check(ZZAF_LIB "${PACKAGE_PREFIX_DIR}/lib/zzaf.lib")
```

### After configure_package_config_file():
```cmake
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

set_and_check(ZZACOMMON_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
set_and_check(ZZAF_LIB "${PACKAGE_PREFIX_DIR}/lib/zzaf.lib")
```

## Best Practices

### DO:
✅ Use `configure_package_config_file()` instead of `configure_file()`
✅ Use `set_and_check()` for all paths that must exist
✅ Use `PACKAGE_PREFIX_DIR` for all installation paths
✅ Use `check_required_components()` at the end of your config
✅ Name template files with `.in` extension

### DON'T:
❌ Don't use absolute hardcoded paths
❌ Don't use `configure_file()` for package config files
❌ Don't manually calculate installation paths
❌ Don't forget to validate required components

## ZZACOMMON Implementation

In our zzacommonConfig.cmake.in, we use:

```cmake
@PACKAGE_INIT@

# Uses PACKAGE_PREFIX_DIR set by @PACKAGE_INIT@
set_and_check(ZZACOMMON_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
set_and_check(ZZACOMMON_CMAKE_DIR "${PACKAGE_PREFIX_DIR}/lib/cmake/zzacommon")

# Each library location is validated
set_and_check(ZZAF_LIB "${PACKAGE_PREFIX_DIR}/lib/zzaf${CMAKE_STATIC_LIBRARY_SUFFIX}")
```

This ensures:
- ✅ Package works no matter where it's installed
- ✅ Paths are validated to exist
- ✅ Clear error messages if files are missing
- ✅ Package can be redistributed or moved

## References

- [CMake configure_package_config_file documentation](https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html#command:configure_package_config_file)
- [CMake Package Configuration Guide](https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html)
