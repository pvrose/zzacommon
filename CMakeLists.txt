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
if(NOT MSVC)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON) 
endif()

## Library zzacommon. 
project(ZZACOMMON VERSION 1.1)
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


# Set timestamp and version into files (build directory to avoid unnecessary rebuilds)
string(TIMESTAMP ZZACOMMON_TIMESTAMP UTC)
configure_file(
  ${ZZACOMMON_SOURCE_DIR}/src/zc_debug.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/zc_debug.cpp
  @ONLY
)

# Source files - keep separate for the separate targets.

# FLTK-based components (without Boost dependency)
set(ZZAF_CPPFILES 
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
  ${ZZACOMMON_SOURCE_DIR}/src/zc_symbols.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_tabs_nonav.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_text_style.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_ticker.cpp
)

# XML and URL components - require pugixml, libcurl, and zlib
set(ZZAX_CPPFILES 
  ${ZZACOMMON_SOURCE_DIR}/src/zc_rpc_data_item.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_rpc_handler.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_url_handler.cpp
)

# Components requiring Boost (filesystem, asio)
set(ZZAB_CPPFILES 
  ${ZZACOMMON_SOURCE_DIR}/src/zc_serial.cpp
)

# Components requiring both FLTK and Boost 
set(ZZAFB_CPPFILES
  ${ZZACOMMON_SOURCE_DIR}/src/zc_banner.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_file_holder.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_status.cpp
)

# Common utility components
set(ZZAM_CPPFILES 
  ${ZZACOMMON_SOURCE_DIR}/src/zc_settings.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_utils.cpp
)

# App metadata component (no external dependencies)
set(ZZAD_CPPFILES 
  ${CMAKE_CURRENT_BINARY_DIR}/zc_debug.cpp
)

# PortAudio component - requires PortAudio
set(ZZAP_CPPFILES
  ${ZZACOMMON_SOURCE_DIR}/src/zc_speaker.cpp
)

# Header files - used as dependencies for API documentation
set(ZZACOMMON_HPPFILES
  ${ZZACOMMON_INCLUDE_DIR}/zc_debug.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_audio_data.h
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
  ${ZZACOMMON_INCLUDE_DIR}/zc_speaker.h
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
# Define which components to build (default: all components for top-level and subproject usage)
if(NOT DEFINED ZZACOMMON_COMPONENTS OR "${ZZACOMMON_COMPONENTS}" STREQUAL "")
  set(ZZACOMMON_COMPONENTS "zzaf;zzab;zzax;zzam;zzafb;zzad;zzap" CACHE STRING "Semicolon-separated list of components to build" FORCE)
endif()

if(PROJECT_IS_TOP_LEVEL)
  message(STATUS "ZZACOMMON build mode: top-level project")
else()
  message(STATUS "ZZACOMMON build mode: subproject (included via FetchContent or add_subdirectory)")
endif()

message(STATUS "Building ZZACOMMON components: ${ZZACOMMON_COMPONENTS}")

# When used as a subproject (e.g. via FetchContent), avoid building all
# component libraries by default. Targets remain available and will be built
# when linked by the parent project.
set(ZZACOMMON_COMPONENT_LIB_EXCLUDE_FROM_ALL)
if(NOT PROJECT_IS_TOP_LEVEL)
  set(ZZACOMMON_COMPONENT_LIB_EXCLUDE_FROM_ALL EXCLUDE_FROM_ALL)
  message(STATUS "zzacommon: Components marked EXCLUDE_FROM_ALL (subproject mode)")
else()
  message(STATUS "zzacommon: All components will be built by default (top-level mode)")
endif()

# Local helper scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# Set CMake prefix path to pick up vcpkg-installed packages
list(APPEND CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows/share")
include(vcpkg_helper)
include(zzacommon_runtime_helpers)

# Check if a component is requested
list(FIND ZZACOMMON_COMPONENTS "zzaf" ZZAF_INDEX)
list(FIND ZZACOMMON_COMPONENTS "zzab" ZZAB_INDEX)
list(FIND ZZACOMMON_COMPONENTS "zzax" ZZAX_INDEX)
list(FIND ZZACOMMON_COMPONENTS "zzam" ZZAM_INDEX)
list(FIND ZZACOMMON_COMPONENTS "zzafb" ZZAFB_INDEX)
list(FIND ZZACOMMON_COMPONENTS "zzad" ZZAD_INDEX)
list(FIND ZZACOMMON_COMPONENTS "zzap" ZZAP_INDEX)

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
  if(MSVC)
  if (DEFINED VCPKG_ROOT)
    list(APPEND CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows")
    message(STATUS "Added vcpkg Boost path: ${VCPKG_ROOT}/installed/x64-windows")
    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
  else()
    message(WARNING "VCPKG_ROOT is not set. If Boost is installed via vcpkg, please set VCPKG_ROOT environment variable or CMake variable.")
  endif()
  endif()

  if (MSVC)
    find_package(Boost 1.85.0 CONFIG COMPONENTS filesystem asio)
  else()
    find_package(Boost 1.83.0 CONFIG COMPONENTS filesystem)
  endif()

  if (Boost_FOUND)
    message(STATUS "Boost found: ${Boost_VERSION}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
    message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")

    # Find and export Boost DLL locations for user applications
    zzacommon_find_boost_dlls("${VCPKG_ROOT}")
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
  message(STATUS "Component zzax requested - adding pugixml target (EXCLUDE_FROM_ALL)")
  add_subdirectory(pugixml EXCLUDE_FROM_ALL)

  # Create namespace alias for pugixml for consistency
  if(TARGET pugixml AND NOT TARGET zzacommon::pugixml)
    add_library(zzacommon::pugixml ALIAS pugixml)
  endif()

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

      # Find and export zlib DLL location for user applications
      zzacommon_find_zlib_dlls("${VCPKG_ROOT}")
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

if(ZZAX_INDEX EQUAL -1)
  message(STATUS "Component zzax not requested - pugixml target will not be created")
endif()

# zzap requires PortAudio
if(NOT ZZAP_INDEX EQUAL -1)
  message(STATUS "Component zzap requested - finding PortAudio")

  if(PORTAUDIO_ROOT)
    if(MSVC)
      # Add PortAudio source as subdirectory (as done in zzatts)
      add_subdirectory(${PORTAUDIO_ROOT}
        ${CMAKE_BINARY_DIR}/portaudio 
        EXCLUDE_FROM_ALL
      )
      set(PORTAUDIO_INCLUDE_DIR ${PORTAUDIO_ROOT}/include)
      set(PORTAUDIO_LIBRARIES portaudio_static)
      message(STATUS "PortAudio: Building from source at ${PORTAUDIO_ROOT}")
      message(STATUS "PortAudio: Using target 'portaudio_static'")
    else()
      find_library(PORTAUDIO_LIBRARIES
        NAMES portaudio
        HINTS "${PORTAUDIO_ROOT}/lib" "${PORTAUDIO_ROOT}/lib64" "/usr/local/lib" "/usr/lib" "/usr/lib64"
      )
      find_path(PORTAUDIO_INCLUDE_DIR
        NAMES portaudio.h
        HINTS "${PORTAUDIO_ROOT}/include" "/usr/local/include" "/usr/include"
      )
    endif()
  else()
    # Fallback to find_package for vcpkg or system installations
    if(MSVC)
      # Try to find PortAudio via vcpkg
      find_package(portaudio CONFIG QUIET)
      if(NOT portaudio_FOUND)
        find_package(PortAudio MODULE QUIET)
      endif()

      if(portaudio_FOUND OR PORTAUDIO_FOUND OR PortAudio_FOUND)
        if(TARGET portaudio)
          set(PORTAUDIO_LIBRARIES portaudio)
          message(STATUS "PortAudio: Using vcpkg target 'portaudio'")
        elseif(TARGET portaudio_static)
          set(PORTAUDIO_LIBRARIES portaudio_static)
          message(STATUS "PortAudio: Using vcpkg target 'portaudio_static'")
        elseif(PORTAUDIO_LIBRARIES)
          message(STATUS "PortAudio libraries: ${PORTAUDIO_LIBRARIES}")
        endif()
      else()
        message(FATAL_ERROR "PortAudio not found. To resolve this:\n"
          "  1. Set PORTAUDIO_ROOT environment variable to PortAudio source directory, OR\n"
          "  2. Install PortAudio via vcpkg: vcpkg install portaudio:x64-windows\n"
          "  3. Set VCPKG_ROOT environment variable to your vcpkg installation directory")
      endif()
    else()
      find_package(PkgConfig QUIET)
      if(PkgConfig_FOUND)
        pkg_check_modules(PORTAUDIO QUIET portaudio-2.0)
      endif()

      if(PORTAUDIO_FOUND)
        if(PORTAUDIO_INCLUDE_DIRS)
          set(PORTAUDIO_INCLUDE_DIR ${PORTAUDIO_INCLUDE_DIRS})
        endif()
        if(PORTAUDIO_LINK_LIBRARIES)
          set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LINK_LIBRARIES})
        elseif(PORTAUDIO_LIBRARIES)
          set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIBRARIES})
        endif()
        message(STATUS "PortAudio: Found via pkg-config (portaudio-2.0)")
      else()
        find_library(PORTAUDIO_LIBRARIES
          NAMES portaudio
          PATHS /usr/local/lib /usr/lib /usr/lib64
        )
        find_path(PORTAUDIO_INCLUDE_DIR
          NAMES portaudio.h
          PATHS /usr/local/include /usr/include
        )
      endif()

      if(NOT PORTAUDIO_LIBRARIES AND NOT TARGET portaudio AND NOT TARGET portaudio_static)
        message(FATAL_ERROR "PortAudio not found on Linux. To resolve this:\n"
          "  1. Install development package (e.g. portaudio19-dev on Debian/Ubuntu), OR\n"
          "  2. Set PORTAUDIO_ROOT to a custom PortAudio installation path")
      endif()
    endif()
  endif()

  if(PORTAUDIO_LIBRARIES OR TARGET portaudio OR TARGET portaudio_static)
    message(STATUS "PortAudio found and ready for zzap component")
  else()
    message(FATAL_ERROR "PortAudio configuration failed")
  endif()
endif()

# Create component library targets

# zzad has no dependencies, so create it first
if(NOT ZZAD_INDEX EQUAL -1)
  add_library(zzad STATIC ${ZZACOMMON_COMPONENT_LIB_EXCLUDE_FROM_ALL})
  target_sources(zzad PRIVATE ${ZZAD_CPPFILES})
  target_include_directories(zzad PUBLIC ${ZZACOMMON_INCLUDE_DIR})
  message(STATUS "Created target: zzad")
endif()

# zzafb depends on zzad, Boost, FLTK, and nlohmann_json (must be before zzaf since they use file_holder)
if(NOT ZZAFB_INDEX EQUAL -1)
  # Find nlohmann_json first (required by zc_banner which uses zc_settings.h)
  if(MSVC)
    # Try to find via vcpkg CONFIG mode first
    find_package(nlohmann_json CONFIG QUIET)

    if(NOT nlohmann_json_FOUND)
      # Fallback to hardcoded path if it exists
      if(EXISTS "${USER_HOME_DIR}\\source\\repos\\json\\include")
        message(STATUS "nlohmann_json: Using local installation at ${USER_HOME_DIR}\\source\\repos\\json\\include")
        set(NLOHMANN_JSON_INCLUDE_DIR "${USER_HOME_DIR}\\source\\repos\\json\\include")
      else()
        message(FATAL_ERROR "nlohmann_json not found. To resolve this:\n"
          "  1. Install nlohmann_json via vcpkg: vcpkg install nlohmann-json:x64-windows\n"
          "  2. Set VCPKG_ROOT environment variable, OR\n"
          "  3. Install nlohmann_json to ${USER_HOME_DIR}\\source\\repos\\json\\include")
      endif()
    else()
      message(STATUS "nlohmann_json found via vcpkg CONFIG mode")
    endif()
  else()
    # Unix-like systems
    find_package(nlohmann_json CONFIG QUIET)
    if(NOT nlohmann_json_FOUND)
      # Try system include paths
      find_path(NLOHMANN_JSON_INCLUDE_DIR 
        NAMES nlohmann/json.hpp
        PATHS /usr/include /usr/local/include
      )
      if(NOT NLOHMANN_JSON_INCLUDE_DIR)
        message(FATAL_ERROR "nlohmann_json not found. Install via package manager or vcpkg.")
      endif()
    endif()
  endif()

  add_library(zzafb STATIC ${ZZACOMMON_COMPONENT_LIB_EXCLUDE_FROM_ALL})
  target_sources(zzafb PRIVATE ${ZZAFB_CPPFILES})
  target_include_directories(zzafb 
    PUBLIC ${ZZACOMMON_INCLUDE_DIR}
    PRIVATE ${FLTK_INCLUDE_DIR}
  )
  # Use modern Boost and FLTK targets
  if(MSVC)
    target_link_libraries(zzafb PUBLIC Boost::filesystem Boost::asio ${FLTK_LIBRARIES})
  else()
    target_link_libraries(zzafb PUBLIC Boost::filesystem Boost::headers ${FLTK_LIBRARIES})
  endif()
  if(NOT ZZAD_INDEX EQUAL -1)
    target_link_libraries(zzafb PUBLIC zzad)
  endif()
  # Link nlohmann_json (required by zc_settings.h included in zc_banner.cpp)
  if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(zzafb PUBLIC nlohmann_json::nlohmann_json)
    message(STATUS "zzafb: Linked with nlohmann_json::nlohmann_json target")
  elseif(NLOHMANN_JSON_INCLUDE_DIR)
    target_include_directories(zzafb PUBLIC ${NLOHMANN_JSON_INCLUDE_DIR})
    message(STATUS "zzafb: Using nlohmann_json include directory: ${NLOHMANN_JSON_INCLUDE_DIR}")
  endif()
  message(STATUS "Created target: zzafb")
endif()

# zzaf depends on zzad (optional) and zzafb (for file_holder)
if(NOT ZZAF_INDEX EQUAL -1)
  add_library(zzaf STATIC ${ZZACOMMON_COMPONENT_LIB_EXCLUDE_FROM_ALL})
  target_sources(zzaf PRIVATE ${ZZAF_CPPFILES})
  target_include_directories(zzaf 
    PUBLIC ${ZZACOMMON_INCLUDE_DIR}
    PRIVATE ${FLTK_INCLUDE_DIR}
  )
  if(MSVC)
    target_include_directories(zzaf PRIVATE ${USER_HOME_DIR}\\source\\repos\\json\\include)
  endif()
  target_link_libraries(zzaf PUBLIC ${FLTK_LIBRARIES})
  if(NOT ZZAD_INDEX EQUAL -1)
    target_link_libraries(zzaf PUBLIC zzad)
  endif()
  # zzaf uses file_holder_ which is defined in zzafb
  if(NOT ZZAFB_INDEX EQUAL -1)
    target_link_libraries(zzaf PUBLIC zzafb)
  endif()
  message(STATUS "Created target: zzaf")
endif()

if(NOT ZZAB_INDEX EQUAL -1)
  add_library(zzab STATIC ${ZZACOMMON_COMPONENT_LIB_EXCLUDE_FROM_ALL})
  target_sources(zzab PRIVATE ${ZZAB_CPPFILES})
  target_include_directories(zzab PUBLIC ${ZZACOMMON_INCLUDE_DIR})
  # Use modern Boost imported targets
  if (MSVC)
    target_link_libraries(zzab PUBLIC Boost::filesystem Boost::asio)
  else()
    target_link_libraries(zzab PUBLIC Boost::filesystem Boost::headers)
  endif()
  target_compile_definitions(zzab PUBLIC _WIN32_WINNT=0x0601)
  message(STATUS "Created target: zzab")
endif()

if(NOT ZZAX_INDEX EQUAL -1)
  add_library(zzax STATIC ${ZZACOMMON_COMPONENT_LIB_EXCLUDE_FROM_ALL})
  target_sources(zzax PRIVATE ${ZZAX_CPPFILES})
  target_include_directories(zzax 
    PUBLIC ${ZZACOMMON_INCLUDE_DIR}
    PRIVATE ${pugixml_INCLUDE_DIR} ${CURL_INCLUDE_DIRS} ${FLTK_INCLUDE_DIR}
  )
  target_link_libraries(zzax PUBLIC pugixml CURL::libcurl ${FLTK_LIBRARIES})
  if(NOT ZZAD_INDEX EQUAL -1)
    target_link_libraries(zzax PUBLIC zzad)
  endif()
  if(MSVC AND TARGET ZLIB::ZLIB)
    target_link_libraries(zzax PUBLIC ZLIB::ZLIB)
  endif()
  message(STATUS "Created target: zzax (combined XML, URL handling, and FLTK)")
endif()

if(NOT ZZAM_INDEX EQUAL -1)
  message(STATUS "Component zzam requested - finding nlohmann_json")

  # Find nlohmann_json (header-only library)
  if(MSVC)
    # Try to find via vcpkg CONFIG mode first
    find_package(nlohmann_json CONFIG QUIET)

    if(NOT nlohmann_json_FOUND)
      # Fallback to hardcoded path if it exists
      if(EXISTS "${USER_HOME_DIR}\\source\\repos\\json\\include")
        message(STATUS "nlohmann_json: Using local installation at ${USER_HOME_DIR}\\source\\repos\\json\\include")
        set(NLOHMANN_JSON_INCLUDE_DIR "${USER_HOME_DIR}\\source\\repos\\json\\include")
      else()
        message(FATAL_ERROR "nlohmann_json not found. To resolve this:\n"
          "  1. Install nlohmann_json via vcpkg: vcpkg install nlohmann-json:x64-windows\n"
          "  2. Set VCPKG_ROOT environment variable, OR\n"
          "  3. Install nlohmann_json to ${USER_HOME_DIR}\\source\\repos\\json\\include")
      endif()
    else()
      message(STATUS "nlohmann_json found via vcpkg CONFIG mode")
    endif()
  else()
    # Unix-like systems
    find_package(nlohmann_json CONFIG QUIET)
    if(NOT nlohmann_json_FOUND)
      # Try system include paths
      find_path(NLOHMANN_JSON_INCLUDE_DIR 
        NAMES nlohmann/json.hpp
        PATHS /usr/include /usr/local/include
      )
      if(NOT NLOHMANN_JSON_INCLUDE_DIR)
        message(FATAL_ERROR "nlohmann_json not found. Install via package manager or vcpkg.")
      endif()
    endif()
  endif()

  add_library(zzam STATIC ${ZZACOMMON_COMPONENT_LIB_EXCLUDE_FROM_ALL})
  target_sources(zzam PRIVATE ${ZZAM_CPPFILES})
  target_include_directories(zzam PUBLIC ${ZZACOMMON_INCLUDE_DIR})

  # Link nlohmann_json
  if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(zzam PUBLIC nlohmann_json::nlohmann_json)
    message(STATUS "zzam: Linked with nlohmann_json::nlohmann_json target")
  elseif(NLOHMANN_JSON_INCLUDE_DIR)
    # Use PUBLIC so that dependents (like zzap) can also use zc_settings.h
      target_include_directories(zzam PUBLIC ${NLOHMANN_JSON_INCLUDE_DIR})
      message(STATUS "zzam: Using nlohmann_json include directory: ${NLOHMANN_JSON_INCLUDE_DIR}")
    endif()

    # zzam uses file_holder_ which is defined in zzafb
    if(NOT ZZAFB_INDEX EQUAL -1)
      target_link_libraries(zzam PUBLIC zzafb)
    endif()

    message(STATUS "Created target: zzam")
endif()

# zzafb already defined above before zzaf/zzam

if(NOT ZZAP_INDEX EQUAL -1)
  add_library(zzap STATIC ${ZZACOMMON_COMPONENT_LIB_EXCLUDE_FROM_ALL})
  target_sources(zzap PRIVATE ${ZZAP_CPPFILES})
  target_include_directories(zzap 
    PUBLIC 
      ${ZZACOMMON_INCLUDE_DIR}
      ${PORTAUDIO_INCLUDE_DIR}
  )
  target_link_libraries(zzap PUBLIC ${PORTAUDIO_LIBRARIES})

  # zzap depends on zzam for zc_settings
  if(NOT ZZAM_INDEX EQUAL -1)
    target_link_libraries(zzap PUBLIC zzam)
  else()
    message(FATAL_ERROR "zzap component requires zzam component (for zc_settings). Please add zzam to ZZACOMMON_COMPONENTS.")
  endif()

  # zzap optionally depends on zzaf for status_ global pointer
  if(NOT ZZAF_INDEX EQUAL -1)
    target_link_libraries(zzap PUBLIC zzaf)
  endif()

  message(STATUS "Created target: zzap (PortAudio audio output)")
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
if(NOT ZZAP_INDEX EQUAL -1)
  add_library(zzacommon::zzap ALIAS zzap)
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
if(NOT ZZAD_INDEX EQUAL -1)
  list(APPEND ZZACOMMON_EXPORT_TARGETS zzad)
endif()
if(NOT ZZAP_INDEX EQUAL -1)
  list(APPEND ZZACOMMON_EXPORT_TARGETS zzap)
endif()

if(ZZACOMMON_EXPORT_TARGETS)
  export(TARGETS ${ZZACOMMON_EXPORT_TARGETS}
    NAMESPACE zzacommon::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/zzacommonTargets.cmake"
  )
endif()

# Create a custom target to build all components when building as top-level project
# This ensures all components are rebuilt when switching between Debug/Release
if(PROJECT_IS_TOP_LEVEL AND ZZACOMMON_EXPORT_TARGETS)
  add_custom_target(zzacommon_all ALL
    DEPENDS ${ZZACOMMON_EXPORT_TARGETS}
    COMMENT "Building all zzacommon components"
    VERBATIM
  )
  message(STATUS "Created target: zzacommon_all (depends on all component libraries)")
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
  if(NOT ZZAP_INDEX EQUAL -1)
    list(APPEND ZZACOMMON_LIBRARIES zzap)
  endif()
  set(ZZACOMMON_LIBRARIES ${ZZACOMMON_LIBRARIES} PARENT_SCOPE)

  # Export zlib DLL paths for user applications (if zzax component is used)
  if(NOT ZZAX_INDEX EQUAL -1)
    set(ZZACOMMON_ZLIB_DLL_RELEASE ${ZZACOMMON_ZLIB_DLL_RELEASE} PARENT_SCOPE)
    set(ZZACOMMON_ZLIB_DLL_DEBUG ${ZZACOMMON_ZLIB_DLL_DEBUG} PARENT_SCOPE)
  endif()

  # Export Boost DLL paths for user applications (if zzab or zzafb component is used)
  if(NOT ZZAB_INDEX EQUAL -1 OR NOT ZZAFB_INDEX EQUAL -1)
    set(ZZACOMMON_BOOST_DLLS_RELEASE ${ZZACOMMON_BOOST_DLLS_RELEASE} PARENT_SCOPE)
    set(ZZACOMMON_BOOST_DLLS_DEBUG ${ZZACOMMON_BOOST_DLLS_DEBUG} PARENT_SCOPE)
  endif()

  # Export API documentation directory
  if(ZZACOMMON_DOX AND ZZACOMMON_API_HTML_DIR)
    set(ZZACOMMON_API_HTML_DIR ${ZZACOMMON_API_HTML_DIR} PARENT_SCOPE)
  endif()

  # Export PortAudio include directory for user applications (if zzap component is used)
  if(NOT ZZAP_INDEX EQUAL -1)
    set(ZZACOMMON_PORTAUDIO_INCLUDE_DIR ${PORTAUDIO_INCLUDE_DIR} PARENT_SCOPE)
  endif()

  set(VCPKG_TARGET_TRIPLET ${VCPKG_TARGET_TRIPLET} PARENT_SCOPE)
  set(_VCPKG_INSTALLED_DIR ${_VCPKG_INSTALLED_DIR} PARENT_SCOPE)
endif()

# Helper function to copy runtime DLLs for applications using zzacommon via
# add_subdirectory/FetchContent (without find_package).
# Usage: zzacommon_copy_runtime_dlls(target_name)
if(NOT COMMAND zzacommon_copy_runtime_dlls)
  function(zzacommon_copy_runtime_dlls TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
      message(FATAL_ERROR "Target ${TARGET_NAME} does not exist")
      return()
    endif()

    if(NOT MSVC)
      message(STATUS "zzacommon: Runtime DLL copy is only needed on Windows/MSVC")
      return()
    endif()

    set(_ZZA_RUNTIME_DLLS "")

    if(ZZACOMMON_ZLIB_DLL_RELEASE AND EXISTS "${ZZACOMMON_ZLIB_DLL_RELEASE}")
      list(APPEND _ZZA_RUNTIME_DLLS "${ZZACOMMON_ZLIB_DLL_RELEASE}")
    endif()
    if(ZZACOMMON_ZLIB_DLL_DEBUG AND EXISTS "${ZZACOMMON_ZLIB_DLL_DEBUG}")
      list(APPEND _ZZA_RUNTIME_DLLS "${ZZACOMMON_ZLIB_DLL_DEBUG}")
    endif()

    if(ZZACOMMON_BOOST_DLLS_RELEASE)
      foreach(_BOOST_DLL ${ZZACOMMON_BOOST_DLLS_RELEASE})
        if(EXISTS "${_BOOST_DLL}")
          list(APPEND _ZZA_RUNTIME_DLLS "${_BOOST_DLL}")
        endif()
      endforeach()
    endif()
    if(ZZACOMMON_BOOST_DLLS_DEBUG)
      foreach(_BOOST_DLL ${ZZACOMMON_BOOST_DLLS_DEBUG})
        if(EXISTS "${_BOOST_DLL}")
          list(APPEND _ZZA_RUNTIME_DLLS "${_BOOST_DLL}")
        endif()
      endforeach()
    endif()

    if(_ZZA_RUNTIME_DLLS)
      list(REMOVE_DUPLICATES _ZZA_RUNTIME_DLLS)
      foreach(_RUNTIME_DLL ${_ZZA_RUNTIME_DLLS})
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_RUNTIME_DLL}"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
          COMMENT "Copying runtime DLL: ${_RUNTIME_DLL}"
        )
      endforeach()
      message(STATUS "zzacommon: Will copy runtime DLLs to ${TARGET_NAME} output directory")
    else()
      message(WARNING "zzacommon: No runtime DLL paths available. Build zzax/zzab/zzafb with vcpkg on Windows.")
    endif()
  endfunction()
endif()

# Function to install runtime DLLs alongside the application install tree.
# Usage: zzacommon_install_runtime_dlls(DESTINATION destination_dir [COMPONENT component_name])
if(NOT COMMAND zzacommon_install_runtime_dlls)
  function(zzacommon_install_runtime_dlls)
    set(options "")
    set(oneValueArgs DESTINATION COMPONENT)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_DESTINATION)
      set(ARG_DESTINATION "bin")
    endif()

    if(NOT ARG_COMPONENT)
      set(ARG_COMPONENT "runtime")
    endif()

    if(NOT MSVC)
      message(STATUS "zzacommon: Runtime DLL install is only needed on Windows/MSVC")
      return()
    endif()

    set(_ZZA_DLLS_TO_INSTALL "")

    if(ZZACOMMON_ZLIB_DLL_RELEASE AND EXISTS "${ZZACOMMON_ZLIB_DLL_RELEASE}")
      list(APPEND _ZZA_DLLS_TO_INSTALL "${ZZACOMMON_ZLIB_DLL_RELEASE}")
    endif()
    if(ZZACOMMON_ZLIB_DLL_DEBUG AND EXISTS "${ZZACOMMON_ZLIB_DLL_DEBUG}")
      list(APPEND _ZZA_DLLS_TO_INSTALL "${ZZACOMMON_ZLIB_DLL_DEBUG}")
    endif()

    if(ZZACOMMON_BOOST_DLLS_RELEASE)
      foreach(_BOOST_DLL ${ZZACOMMON_BOOST_DLLS_RELEASE})
        if(EXISTS "${_BOOST_DLL}")
          list(APPEND _ZZA_DLLS_TO_INSTALL "${_BOOST_DLL}")
        endif()
      endforeach()
    endif()
    if(ZZACOMMON_BOOST_DLLS_DEBUG)
      foreach(_BOOST_DLL ${ZZACOMMON_BOOST_DLLS_DEBUG})
        if(EXISTS "${_BOOST_DLL}")
          list(APPEND _ZZA_DLLS_TO_INSTALL "${_BOOST_DLL}")
        endif()
      endforeach()
    endif()

    if(_ZZA_DLLS_TO_INSTALL)
      list(REMOVE_DUPLICATES _ZZA_DLLS_TO_INSTALL)
      install(FILES ${_ZZA_DLLS_TO_INSTALL}
        DESTINATION ${ARG_DESTINATION}
        COMPONENT ${ARG_COMPONENT}
      )
      list(LENGTH _ZZA_DLLS_TO_INSTALL _ZZA_DLL_COUNT)
      message(STATUS "zzacommon: Will install ${_ZZA_DLL_COUNT} runtime DLL(s) to ${ARG_DESTINATION}")
    else()
      message(WARNING "zzacommon: No runtime DLLs available for installation.")
    endif()
  endfunction()
endif()

# Helper function to copy API documentation to user documentation directory.
# Usage: zzacommon_copy_api_docs(destination_directory)
if(NOT COMMAND zzacommon_copy_api_docs)
  function(zzacommon_copy_api_docs DESTINATION_DIR)
    if(NOT ZZACOMMON_API_HTML_DIR)
      message(WARNING "zzacommon: API documentation directory not available. Build zzacommon with ZZACOMMON_DOX=ON to generate documentation.")
      return()
    endif()

    if(NOT EXISTS "${ZZACOMMON_API_HTML_DIR}")
      message(WARNING "zzacommon: API documentation directory does not exist: ${ZZACOMMON_API_HTML_DIR}")
      return()
    endif()

    file(MAKE_DIRECTORY "${DESTINATION_DIR}")
    file(COPY "${ZZACOMMON_API_HTML_DIR}/"
      DESTINATION "${DESTINATION_DIR}"
      PATTERN "*"
    )

    message(STATUS "zzacommon: Copied API documentation from ${ZZACOMMON_API_HTML_DIR} to ${DESTINATION_DIR}")
  endfunction()
endif()

# generate API documentation
if (ZZACOMMON_DOX)
  message(STATUS "ZZACOMMON: Adding API documentation")
  add_subdirectory(api)

  # Set the API documentation output directory
  set(ZZACOMMON_API_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/api/html")
  if (NOT PROJECT_IS_TOP_LEVEL)
    set(ZZACOMMON_API_HTML_DIR ${ZZACOMMON_API_HTML_DIR} PARENT_SCOPE)
  endif()

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
  if(NOT ZZAP_INDEX EQUAL -1)
    add_dependencies(zzap zzacommon_api_html)
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
  if(NOT ZZAP_INDEX EQUAL -1)
    list(APPEND INSTALL_TARGETS zzap)
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
  # Also install pugixml if zzax is being installed (since it's a bundled dependency)
  if(TARGET pugixml)
    list(APPEND INSTALL_LIB_TARGETS pugixml)
  endif()
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
if(NOT ZZAP_INDEX EQUAL -1)
  list(APPEND INSTALL_LIB_TARGETS zzap)
endif()

if(INSTALL_LIB_TARGETS)
  set(_ZZACOMMON_INSTALL_CORE_TARGETS ${INSTALL_LIB_TARGETS})
  list(REMOVE_ITEM _ZZACOMMON_INSTALL_CORE_TARGETS pugixml)

  if(_ZZACOMMON_INSTALL_CORE_TARGETS)
    install(TARGETS ${_ZZACOMMON_INSTALL_CORE_TARGETS}
      ARCHIVE DESTINATION lib
      LIBRARY DESTINATION lib
      RUNTIME DESTINATION bin
    )
  endif()

  if("pugixml" IN_LIST INSTALL_LIB_TARGETS)
    install(TARGETS pugixml
      ARCHIVE DESTINATION lib
      LIBRARY DESTINATION lib
      RUNTIME DESTINATION bin
      FILE_SET common_headers DESTINATION include/pugixml
    )
  endif()
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
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/zzacommon_runtime_helpers.cmake"
  DESTINATION lib/cmake/zzacommon
)

# Install API documentation (if generated)
if(ZZACOMMON_DOX AND EXISTS "${ZZACOMMON_API_HTML_DIR}")
  install(DIRECTORY "${ZZACOMMON_API_HTML_DIR}"
    DESTINATION share/doc/zzacommon
    COMPONENT documentation
  )
  message(STATUS "zzacommon: Will install API documentation to share/doc/zzacommon/html")
endif()

