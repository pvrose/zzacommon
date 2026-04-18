#[=[
	Copyright 2026, Philip Rose, GM3ZZA
	
    This file is part of ZZACOMMON.

    ZZACOMMON is free software: you can redistribute it and/or modify it under the
	terms of the Lesser GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later version.

    ZZACOMMON is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
	PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with ZZACOMMON. 
	If not, see <https://www.gnu.org/licenses/>. 

#]=]
cmake_minimum_required(VERSION 3.21)

## Library zzacommon. 
project(ZZACOMMON VERSION 1.0.10)
set(ZZACOMMON_INCLUDE_DIR ${ZZACOMMON_SOURCE_DIR}/include)

# Language requirememnts
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## Set home directory per OS.
if(MSVC)
  set(USER_HOME_DIR $ENV{USERPROFILE})
else()
  set(USER_HOME_DIR $ENV{HOME})
endif()

## Detect vcpkg root if not already set
if(NOT DEFINED VCPKG_ROOT AND DEFINED ENV{VCPKG_ROOT})
  set(VCPKG_ROOT $ENV{VCPKG_ROOT})
  message(STATUS "Using VCPKG_ROOT from environment: ${VCPKG_ROOT}")
elseif(NOT DEFINED VCPKG_ROOT)
  # Try common locations
  if(MSVC)
    if(EXISTS "${USER_HOME_DIR}/source/repos/vcpkg")
      set(VCPKG_ROOT "${USER_HOME_DIR}/source/repos/vcpkg")
      message(STATUS "Auto-detected VCPKG_ROOT: ${VCPKG_ROOT}")
    elseif(EXISTS "C:/vcpkg")
      set(VCPKG_ROOT "C:/vcpkg")
      message(STATUS "Auto-detected VCPKG_ROOT: ${VCPKG_ROOT}")
    endif()
  endif()
endif()


# Set timestamp and version into files (requires to be rebuilt to take effect)
string(TIMESTAMP ZZACOMMON_TIMESTAMP UTC)
configure_file(
  ${ZZACOMMON_SOURCE_DIR}/src/zc_debug.cpp.in
  ${ZZACOMMON_SOURCE_DIR}/src/zc_debug.cpp
  @ONLY
)

# Source files - keep separate for the separate targets.

# FLTK-based components
set(ZZAF_CPPFILES 
  ${ZZACOMMON_SOURCE_DIR}/src/zc_banner.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_calendar.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_calendar_input.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_calendar_table.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_file_viewer.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_filename_input.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_fltk.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_graph.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_graph_axis.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_graph_base.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_graph_plot.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_graph_xy.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_graph_x2y.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_input_hierch.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_line_style.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_password_input.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_socket_server.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_status.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_symbols.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_tabs_nonav.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_text_style.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_ticker.cpp
)

# XML components - require pugixml
set(ZZAX_CPPFILES 
  ${ZZACOMMON_SOURCE_DIR}/src/zc_rpc_data_item.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_rpc_handler.cpp
)

# URL handler - requires libcurl and zlib
set(ZZAU_CPPFILES 
  ${ZZACOMMON_SOURCE_DIR}/src/zc_url_handler.cpp
)

# Components requiring Boost (filesystem, asio)
set(ZZAB_CPPFILES 
  ${ZZACOMMON_SOURCE_DIR}/src/zc_serial.cpp
)

# Components requiring both FLTK and Boost 
set(ZZAFB_CPPFILES
  ${ZZACOMMON_SOURCE_DIR}/src/zc_file_holder.cpp
)  

# Common utility components
set(ZZAM_CPPFILES 
  ${ZZACOMMON_SOURCE_DIR}/src/zc_settings.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_utils.cpp
)

# App metadata component (no external dependencies)
set(ZZAD_CPPFILES 
  ${ZZACOMMON_SOURCE_DIR}/src/zc_debug.cpp
)

# Header files - used as dependencies for API documentation
set(ZZACOMMON_HPPFILES
  ${ZZACOMMON_INCLUDE_DIR}/zc_debug.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_banner.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_button_dialog.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_button_input.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_calendar.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_calendar_input.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_calendar_table.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_callback.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_drawing.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_file_holder.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_file_viewer.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_filename_input.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_fltk.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_graph.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_graph_axis.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_graph_base.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_graph_plot.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_graph_xy.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_graph_x2y.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_input_hierch.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_line_style.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_password_input.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_rpc_data_item.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_rpc_handler.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_serial.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_settings.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_socket_server.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_status.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_symbols.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_tabs_nonav.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_text_style.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_ticker.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_url_handler.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_utils.h
  ${ZZACOMMON_INCLUDE_DIR}/zzacommon.h
)

# Component-based build system
# Define which components to build (default: all components)
if(NOT DEFINED ZZACOMMON_COMPONENTS)
  set(ZZACOMMON_COMPONENTS "zzaf;zzab;zzax;zzam;zzafb;zzad" CACHE STRING "Semicolon-separated list of components to build")
endif()

message(STATUS "Building ZZACOMMON components: ${ZZACOMMON_COMPONENTS}")

# Local helper scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# Set CMake prefix path to pick up vcpkg-installed packages
list(APPEND CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows/share")
include(vcpkg_helper)

# Check if a component is requested
list(FIND ZZACOMMON_COMPONENTS "zzaf" ZZAF_INDEX)
list(FIND ZZACOMMON_COMPONENTS "zzab" ZZAB_INDEX)
list(FIND ZZACOMMON_COMPONENTS "zzax" ZZAX_INDEX)
list(FIND ZZACOMMON_COMPONENTS "zzam" ZZAM_INDEX)
list(FIND ZZACOMMON_COMPONENTS "zzafb" ZZAFB_INDEX)
list(FIND ZZACOMMON_COMPONENTS "zzad" ZZAD_INDEX)

# Find dependencies only for requested components

# FLTK is required by zzaf and zzax
if(NOT ZZAF_INDEX EQUAL -1 OR NOT ZZAX_INDEX EQUAL -1 OR NOT ZZAFB_INDEX EQUAL -1)
  message(STATUS "Component zzaf or zzax requested - finding FLTK")
  # MSVC work-round as FLTK built in user.
  if(MSVC)
    if (NOT FLTK_DIR)
      message(FATAL_ERROR "Please set FLTK_DIR") 
    else()
      message(STATUS "Using FLTK_DIR: ${FLTK_DIR}")
    endif()
  endif()
  find_package(FLTK CONFIG)
  if (FLTK_FOUND)
    message(STATUS "FLTK found: ${FLTK_VERSION}")
    message(STATUS "FLTK include dir: ${FLTK_INCLUDE_DIR}")
    message(STATUS "FLTK libraries: ${FLTK_LIBRARIES}")
  else()
    message(FATAL_ERROR "FLTK not found")
  endif()
endif()

# zzab and zzafb require Boost
if(NOT ZZAB_INDEX EQUAL -1 OR NOT ZZAFB_INDEX EQUAL -1)
  message(STATUS "Component zzab or zzafb requested - finding Boost")

  # If using vcpkg, add Boost-specific paths
  if(DEFINED VCPKG_ROOT)
    list(APPEND CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows")
    message(STATUS "Added vcpkg Boost path: ${VCPKG_ROOT}/installed/x64-windows")
    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
  else()
    message(WARNING "VCPKG_ROOT is not set. If Boost is installed via vcpkg, please set VCPKG_ROOT environment variable or CMake variable.")
  endif()

  if (MSVC)
    find_package(Boost 1.85.0 CONFIG COMPONENTS filesystem asio)
  else()
    find_package(Boost 1.83.0 CONFIG COMPONENTS ALL)
  endif()

  if (Boost_FOUND)
    message(STATUS "Boost found: ${Boost_VERSION}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
    message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
  else()
    message(FATAL_ERROR "Boost not found. To resolve this:\n"
      "  1. Install Boost via vcpkg: vcpkg install boost-filesystem:x64-windows boost-asio:x64-windows\n"
      "  2. Set VCPKG_ROOT environment variable to your vcpkg installation directory\n"
      "  3. Or set CMAKE_PREFIX_PATH to include your Boost installation directory\n"
      "  4. Or set Boost_DIR to point to BoostConfig.cmake location")
  endif()
endif()

# zzax requires pugixml, libcurl, zlib, and FLTK
if(NOT ZZAX_INDEX EQUAL -1)
  message(STATUS "Component zzax requested - finding pugixml, CURL, ZLIB (FLTK found above)")

  # pugixml
  add_subdirectory(pugixml)

  # zlib (CURL dependency - needed for runtime DLL detection)
  if(MSVC)
    # Try CONFIG mode first (vcpkg modern packages)
    set(ZLIB_ROOT "${VCPKG_ROOT}/installed/x64-windows")
    find_package(ZLIB CONFIG QUIET)

    # If CONFIG mode fails, try MODULE mode
    if(NOT ZLIB_FOUND)
      find_package(ZLIB MODULE REQUIRED)
      message(STATUS "ZLIB found via MODULE mode: ${ZLIB_VERSION_STRING}")
    else()
      message(STATUS "ZLIB found via CONFIG mode: ${ZLIB_VERSION}")
    endif()

    if(ZLIB_FOUND)
      message(STATUS "ZLIB libraries: ${ZLIB_LIBRARIES}")

      # Check if modern imported target exists
      if(TARGET ZLIB::ZLIB)
        message(STATUS "ZLIB::ZLIB imported target is available")
      else()
        message(STATUS "ZLIB::ZLIB target not available, will use variables")
        # Create an imported target manually for consistency
        if(NOT TARGET ZLIB::ZLIB AND ZLIB_LIBRARIES)
          add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
          set_target_properties(ZLIB::ZLIB PROPERTIES
            IMPORTED_LOCATION "${ZLIB_LIBRARIES}"
          )
          if(ZLIB_INCLUDE_DIRS)
            set_target_properties(ZLIB::ZLIB PROPERTIES
              INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS}"
            )
          endif()
          message(STATUS "Created ZLIB::ZLIB imported target manually")
        endif()
      endif()
    endif()
  endif()

  # libcurl 
  if(MSVC)
    find_package(CURL CONFIG REQUIRED)
  else()
    find_package(CURL REQUIRED)
  endif()
  if (CURL_FOUND)
    message(STATUS "CURL found: ${CURL_VERSION}")
    message(STATUS "CURL include dir: ${CURL_INCLUDE_DIR}")
    message(STATUS "CURL libraries: ${CURL_LIBRARIES}")
  else()
    message(STATUS "CURL not found")
  endif()
endif()

# Create component library targets
if(NOT ZZAF_INDEX EQUAL -1)
  add_library(zzaf STATIC)
  target_sources(zzaf PRIVATE ${ZZAF_CPPFILES})
  target_include_directories(zzaf 
    PUBLIC ${ZZACOMMON_INCLUDE_DIR}
    PRIVATE ${FLTK_INCLUDE_DIR}
  )
  if(MSVC)
    target_include_directories(zzaf PRIVATE ${USER_HOME_DIR}\\source\\repos\\json\\include)
  endif()
  target_link_libraries(zzaf PUBLIC ${FLTK_LIBRARIES})
  message(STATUS "Created target: zzaf")
endif()

if(NOT ZZAB_INDEX EQUAL -1)
  add_library(zzab STATIC)
  target_sources(zzab PRIVATE ${ZZAB_CPPFILES})
  target_include_directories(zzab PUBLIC ${ZZACOMMON_INCLUDE_DIR})
  # Use modern Boost imported targets
  target_link_libraries(zzab PUBLIC Boost::filesystem Boost::asio)
  target_compile_definitions(zzab PUBLIC _WIN32_WINNT=0x0601)
  message(STATUS "Created target: zzab")
endif()

if(NOT ZZAX_INDEX EQUAL -1)
  add_library(zzax STATIC)
  target_sources(zzax PRIVATE ${ZZAX_CPPFILES} ${ZZAU_CPPFILES})
  target_include_directories(zzax 
    PUBLIC ${ZZACOMMON_INCLUDE_DIR}
    PRIVATE ${pugixml_INCLUDE_DIR} ${CURL_INCLUDE_DIRS} ${FLTK_INCLUDE_DIR}
  )
  target_link_libraries(zzax PUBLIC pugixml CURL::libcurl ${FLTK_LIBRARIES})
  if(TARGET ZLIB::ZLIB)
    target_link_libraries(zzax PUBLIC ZLIB::ZLIB)
  endif()
  message(STATUS "Created target: zzax (combined XML, URL handling, and FLTK)")
endif()

if(NOT ZZAM_INDEX EQUAL -1)
  add_library(zzam STATIC)
  target_sources(zzam PRIVATE ${ZZAM_CPPFILES})
  target_include_directories(zzam PUBLIC ${ZZACOMMON_INCLUDE_DIR})
  if(MSVC)
    target_include_directories(zzam PRIVATE ${USER_HOME_DIR}\\source\\repos\\json\\include)
  endif()
  message(STATUS "Created target: zzam")
endif()

if (NOT ZZAFB_INDEX EQUAL -1)
  add_library(zzafb STATIC)
  target_sources(zzafb PRIVATE ${ZZAFB_CPPFILES})
  target_include_directories(zzafb 
    PUBLIC ${ZZACOMMON_INCLUDE_DIR}
    PRIVATE ${FLTK_INCLUDE_DIR}
  )
  # Use modern Boost and FLTK targets
  target_link_libraries(zzafb PUBLIC Boost::filesystem Boost::asio ${FLTK_LIBRARIES})
  message(STATUS "Created target: zzafb")
endif()

if(NOT ZZAD_INDEX EQUAL -1)
  add_library(zzad STATIC)
  target_sources(zzad PRIVATE ${ZZAD_CPPFILES})
  target_include_directories(zzad PUBLIC ${ZZACOMMON_INCLUDE_DIR})
  message(STATUS "Created target: zzad")
endif()

set(CMAKE_MODULE_PATH ../)

# Create namespace aliases for targets (allows using zzacommon::target syntax in build tree)
if(NOT ZZAF_INDEX EQUAL -1)
  add_library(zzacommon::zzaf ALIAS zzaf)
endif()
if(NOT ZZAB_INDEX EQUAL -1)
  add_library(zzacommon::zzab ALIAS zzab)
endif()
if(NOT ZZAX_INDEX EQUAL -1)
  add_library(zzacommon::zzax ALIAS zzax)
endif()
if(NOT ZZAM_INDEX EQUAL -1)
  add_library(zzacommon::zzam ALIAS zzam)
endif()
if(NOT ZZAFB_INDEX EQUAL -1)
  add_library(zzacommon::zzafb ALIAS zzafb)
endif()
if (NOT ZZAD_INDEX EQUAL -1)
  add_library(zzacommon::zzad ALIAS zzad)
endif()

# Export targets for build tree (creates zzacommonTargets.cmake in build directory)
set(ZZACOMMON_EXPORT_TARGETS "")
if(NOT ZZAF_INDEX EQUAL -1)
  list(APPEND ZZACOMMON_EXPORT_TARGETS zzaf)
endif()
if(NOT ZZAB_INDEX EQUAL -1)
  list(APPEND ZZACOMMON_EXPORT_TARGETS zzab)
endif()
if(NOT ZZAX_INDEX EQUAL -1)
  list(APPEND ZZACOMMON_EXPORT_TARGETS zzax)
  # Include pugixml in export since zzax depends on it
  if(TARGET pugixml)
    list(APPEND ZZACOMMON_EXPORT_TARGETS pugixml)
  endif()
endif()
if(NOT ZZAM_INDEX EQUAL -1)
  list(APPEND ZZACOMMON_EXPORT_TARGETS zzam)
endif()
if(NOT ZZAFB_INDEX EQUAL -1)
  list(APPEND ZZACOMMON_EXPORT_TARGETS zzafb)
endif()
list(APPEND ZZACOMMON_EXPORT_TARGETS zzad)

if(ZZACOMMON_EXPORT_TARGETS)
  export(TARGETS ${ZZACOMMON_EXPORT_TARGETS}
    NAMESPACE zzacommon::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/zzacommonTargets.cmake"
  )
endif()

## Export include directories and component targets
if (NOT PROJECT_IS_TOP_LEVEL)
  set(ZZACOMMON_INCLUDE_DIR ${ZZACOMMON_INCLUDE_DIR} PARENT_SCOPE)
  set(ZZACOMMON_HPPFILES ${ZZACOMMON_HPPFILES} PARENT_SCOPE)
  set(ZZACOMMON_VERSION ${ZZACOMMON_VERSION} PARENT_SCOPE)
  set(ZZACOMMON_COMPONENTS ${ZZACOMMON_COMPONENTS} PARENT_SCOPE)

  # Export component library names that were built
  set(ZZACOMMON_LIBRARIES "" PARENT_SCOPE)
  if(NOT ZZAF_INDEX EQUAL -1 OR NOT ZZAX_INDEX EQUAL -1)
    set(FLTK_INCLUDE_DIR ${FLTK_INCLUDE_DIR} PARENT_SCOPE)
  endif()
  if(NOT ZZAF_INDEX EQUAL -1)
    list(APPEND ZZACOMMON_LIBRARIES zzaf)
  endif()
  if(NOT ZZAB_INDEX EQUAL -1)
    list(APPEND ZZACOMMON_LIBRARIES zzab)
  endif()
  if(NOT ZZAX_INDEX EQUAL -1)
    list(APPEND ZZACOMMON_LIBRARIES zzax)
  endif()
  if(NOT ZZAM_INDEX EQUAL -1)
    list(APPEND ZZACOMMON_LIBRARIES zzam)
  endif()
  if (NOT ZZAFB_INDEX EQUAL -1)
    list(APPEND ZZACOMMON_LIBRARIES zzafb)
  endif()
  if(NOT ZZAD_INDEX EQUAL -1)
    list(APPEND ZZACOMMON_LIBRARIES zzad)
  endif()
  set(ZZACOMMON_LIBRARIES ${ZZACOMMON_LIBRARIES} PARENT_SCOPE)

  set(VCPKG_TARGET_TRIPLET ${VCPKG_TARGET_TRIPLET} PARENT_SCOPE)
  set(_VCPKG_INSTALLED_DIR ${_VCPKG_INSTALLED_DIR} PARENT_SCOPE)
endif()

# generate API documentation
if (ZZACOMMON_DOX)
  message(STATUS "ZZACOMMON: Adding API documentation")
  add_subdirectory(api)
  # Add dependency for all built component targets
  if(NOT ZZAF_INDEX EQUAL -1)
    add_dependencies(zzaf zzacommon_api_html)
  endif()
  if(NOT ZZAB_INDEX EQUAL -1)
    add_dependencies(zzab zzacommon_api_html)
  endif()
  if(NOT ZZAX_INDEX EQUAL -1)
    add_dependencies(zzax zzacommon_api_html)
  endif()
  if(NOT ZZAM_INDEX EQUAL -1)
    add_dependencies(zzam zzacommon_api_html)
  endif()
  if(NOT ZZAFB_INDEX EQUAL -1)
    add_dependencies(zzafb zzacommon_api_html)
  endif()
  if(NOT ZZAD_INDEX EQUAL -1)
    add_dependencies(zzad zzacommon_api_html)
  endif()
endif()

# Installation rules for component targets
if (BUILD_SHARED_LIBS)
  set(INSTALL_TARGETS "")
  if(NOT ZZAF_INDEX EQUAL -1)
    list(APPEND INSTALL_TARGETS zzaf)
  endif()
  if(NOT ZZAB_INDEX EQUAL -1)
    list(APPEND INSTALL_TARGETS zzab)
  endif()
  if(NOT ZZAX_INDEX EQUAL -1)
    list(APPEND INSTALL_TARGETS zzax)
  endif()
  if(NOT ZZAM_INDEX EQUAL -1)
    list(APPEND INSTALL_TARGETS zzam)
  endif()
  if(NOT ZZAFB_INDEX EQUAL -1)
    list(APPEND INSTALL_TARGETS zzafb)
  endif()
  if(NOT ZZAD_INDEX EQUAL -1)
    list(APPEND INSTALL_TARGETS zzad)
  endif()

  if(INSTALL_TARGETS)
    install(TARGETS ${INSTALL_TARGETS})
  endif()
endif()

# Install library files and CMake config for find_package support
include(CMakePackageConfigHelpers)

# Configure version file (use write_basic_package_version_file for proper versioning)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/zzacommonConfigVersion.cmake"
  VERSION ${ZZACOMMON_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Configure build tree config file (simple version that uses exported targets)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/zzacommonConfig-build.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/zzacommonConfig.cmake"
  @ONLY
)

# Set the CMake directory variable for install tree config file
set(ZZACOMMON_CMAKE_DIR "${CMAKE_INSTALL_PREFIX}/lib/cmake/zzacommon")

# Configure install tree config file (use configure_package_config_file for proper package support)
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/zzacommonConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/zzacommonConfig-install.cmake"
  INSTALL_DESTINATION lib/cmake/zzacommon
  PATH_VARS ZZACOMMON_CMAKE_DIR
)

# Install component libraries
set(INSTALL_LIB_TARGETS "")
if(NOT ZZAF_INDEX EQUAL -1)
  list(APPEND INSTALL_LIB_TARGETS zzaf)
endif()
if(NOT ZZAB_INDEX EQUAL -1)
  list(APPEND INSTALL_LIB_TARGETS zzab)
endif()
if(NOT ZZAX_INDEX EQUAL -1)
  list(APPEND INSTALL_LIB_TARGETS zzax)
endif()
if(NOT ZZAM_INDEX EQUAL -1)
  list(APPEND INSTALL_LIB_TARGETS zzam)
endif()
if(NOT ZZAFB_INDEX EQUAL -1)
  list(APPEND INSTALL_LIB_TARGETS zzafb)
endif()
if(NOT ZZAD_INDEX EQUAL -1)
  list(APPEND INSTALL_LIB_TARGETS zzad)
endif()

if(INSTALL_LIB_TARGETS)
  install(TARGETS ${INSTALL_LIB_TARGETS} ${ZC_APP_LIBRARY}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )
endif()

# Install headers
install(FILES ${ZZACOMMON_HPPFILES}
  DESTINATION include
)

# Install CMake config files
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/zzacommonConfigVersion.cmake"
  DESTINATION lib/cmake/zzacommon
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/zzacommonConfig-install.cmake"
  DESTINATION lib/cmake/zzacommon
  RENAME zzacommonConfig.cmake
)

# Install helper CMake modules
install(FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vcpkg_helper.cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/zlib_vcpkg_helper.cmake"
  DESTINATION lib/cmake/zzacommon
)

