# Using zzacommon API Documentation

When zzacommon is built with API documentation enabled (`ZZACOMMON_DOX=ON`), the generated HTML documentation can be easily integrated into your application's documentation.

## Prerequisites

- zzacommon must be built with `ZZACOMMON_DOX=ON`
- Doxygen must be installed and available during zzacommon's build

## Available Variable

When zzacommon is loaded (via `find_package` or FetchContent), the following variable is set:

- **`ZZACOMMON_API_HTML_DIR`** - Path to the zzacommon API documentation HTML directory

If documentation is not available, this variable will be empty.

## Usage Examples

### Quick Copy - FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    zzacommon
    GIT_REPOSITORY https://github.com/pvrose/zzacommon
    GIT_TAG master
)

# Enable documentation generation
set(ZZACOMMON_DOX ON CACHE BOOL "")
set(ZZACOMMON_COMPONENTS "zzax;zzam" CACHE STRING "")
FetchContent_MakeAvailable(zzacommon)

# Copy API docs to your documentation directory
if(ZZACOMMON_API_HTML_DIR)
    zzacommon_copy_api_docs("${CMAKE_CURRENT_SOURCE_DIR}/docs/api/zzacommon")
endif()
```

### Quick Copy - find_package

```cmake
find_package(zzacommon REQUIRED COMPONENTS zzax zzam)

# Copy API docs to your documentation directory
if(ZZACOMMON_API_HTML_DIR)
    zzacommon_copy_api_docs("${CMAKE_CURRENT_SOURCE_DIR}/docs/api/zzacommon")
endif()
```

### Manual Copy (Advanced)

If you prefer manual control:

```cmake
find_package(zzacommon REQUIRED)

if(ZZACOMMON_API_HTML_DIR AND EXISTS "${ZZACOMMON_API_HTML_DIR}")
    # Create your docs directory
    set(MY_DOCS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/documentation/zzacommon-api")
    file(MAKE_DIRECTORY "${MY_DOCS_DIR}")

    # Copy the documentation
    file(COPY "${ZZACOMMON_API_HTML_DIR}/"
        DESTINATION "${MY_DOCS_DIR}"
        PATTERN "*"
    )

    message(STATUS "Copied zzacommon API docs to ${MY_DOCS_DIR}")
else()
    message(STATUS "zzacommon API documentation not available")
endif()
```

### Copy at Build Time

If you want to copy documentation as part of your build process:

```cmake
find_package(zzacommon REQUIRED)

add_custom_target(copy_zzacommon_docs ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ZZACOMMON_API_HTML_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/docs/zzacommon-api"
    COMMENT "Copying zzacommon API documentation"
)
```

## Helper Function

### zzacommon_copy_api_docs(destination_directory)

Copies the zzacommon API documentation to the specified directory.

**Parameters:**
- `destination_directory` - Path where the documentation should be copied

**Example:**
```cmake
zzacommon_copy_api_docs("${CMAKE_CURRENT_SOURCE_DIR}/docs/zzacommon")
```

This will copy all HTML files from `ZZACOMMON_API_HTML_DIR` to the specified destination.

## Integration with Your Documentation

After copying, you can:

1. **Link from your main documentation:**
   ```html
   <a href="api/zzacommon/index.html">zzacommon API Reference</a>
   ```

2. **Include in a Doxygen project:**
   Add to your Doxyfile:
   ```
   EXAMPLE_PATH = docs/zzacommon
   ```

3. **Reference in Markdown:**
   ```markdown
   See the [zzacommon API documentation](api/zzacommon/index.html)
   ```

## Installation

When zzacommon is installed (via `make install` or similar), the API documentation is installed to:
```
${CMAKE_INSTALL_PREFIX}/share/doc/zzacommon/html/
```

The `ZZACOMMON_API_HTML_DIR` variable will automatically point to this location when using `find_package` with an installed version.

## Troubleshooting

**Documentation not available:**
- Ensure zzacommon was built with `ZZACOMMON_DOX=ON`
- Verify Doxygen is installed: `doxygen --version`
- Check the build output for "Generating API documentation" messages

**Variable is empty:**
```cmake
if(NOT ZZACOMMON_API_HTML_DIR)
    message(WARNING "zzacommon API documentation not available")
endif()
```

**Directory doesn't exist:**
The helper function will warn you if the directory doesn't exist, but you can also check manually:
```cmake
if(NOT EXISTS "${ZZACOMMON_API_HTML_DIR}")
    message(WARNING "Documentation directory not found: ${ZZACOMMON_API_HTML_DIR}")
endif()
```
