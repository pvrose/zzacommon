cmake_minimum_required(VERSION 3.21)

## Library zzacommon. 
project(ZZACOMMON VERSION 1.0.7)
set(ZZACOMMON_INCLUDE_DIR ${ZZACOMMON_SOURCE_DIR}/include)
set (TARGET zzacommon)

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


# Set timestamp and version into files (requires to be rebuilt to take effect)
string(TIMESTAMP ZZACOMMON_TIMESTAMP UTC)
# Set app details
if (NOT APP_TIMESTAMP)
  set(APP_TIMESTAMP ${ZZACOMMON_TIMESTAMP})
endif()
if (NOT APP_VERSION)
  set(APP_VERSION ${ZZACOMMON_VERSION})
endif()
if (NOT APP_NAME)
  set(APP_NAME "ZZATEST")
endif()
if (NOT APP_VENDOR)
  set(APP_VENDOR "GM3ZZA")
endif()
if (NOT APP_SOURCE_DIR) 
  set(APP_SOURCE_DIR ${ZZACOMMON_SOURCE_DIR})
endif()
configure_file(
  ${ZZACOMMON_SOURCE_DIR}/src/zc_app.cpp.in
  ${ZZACOMMON_SOURCE_DIR}/src/zc_app.cpp
  @ONLY
)

string(TOLOWER ${APP_NAME} APP_NAME_LOWER)
set(ZC_APP_LIBRARY zc_${APP_NAME_LOWER})

message(STATUS "ZZACOMMON for parent ${APP_NAME}")

# Source files
set(ZZACOMMON_CPPFILES 
#  ${ZZACOMMON_SOURCE_DIR}/src/zc_app.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_banner.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_calendar.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_calendar_input.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_calendar_table.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_file_holder.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_filename_input.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_graph.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_input_hierch.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_password_input.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_rpc_data_item.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_rpc_handler.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_serial.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_settings.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_socket_server.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_status.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_symbols.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_tabs_nonav.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_ticker.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_url_handler.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_utils.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_fft.cpp
)

# Header files - used as dependencies for API documentation
set(ZZACOMMON_HPPFILES
  ${ZZACOMMON_INCLUDE_DIR}/zc_app.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_banner.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_button_input.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_calendar.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_calendar_input.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_calendar_table.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_callback.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_drawing.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_fft.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_file_holder.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_filename_input.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_graph.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_input_hierch.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_password_input.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_rpc_data_item.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_rpc_handler.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_serial.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_settings.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_socket_server.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_status.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_symbols.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_tabs_nonav.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_ticker.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_url_handler.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_utils.h
  ${ZZACOMMON_INCLUDE_DIR}/zzacommon.h
)

# Find fltk
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

# libboost
if (MSVC)
  find_package(Boost 1.85.0 CONFIG REQUIRED COMPONENTS filesystem)
else()
  find_package(Boost 1.83.0 CONFIG REQUIRED COMPONENTS ALL)
endif()
if (Boost_FOUND)
  message(STATUS "Boost found: ${Boost_VERSION}")
  message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
  message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
endif()

# zlib (CURL dependency - needed for runtime DLL detection)
if(MSVC)
  # Try CONFIG mode first (vcpkg modern packages)
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

# TODO Make External Project?
add_subdirectory(pugixml)

set(CMAKE_MODULE_PATH ../)

## Target is libzzacommon
add_library(${TARGET} STATIC)
add_library(${ZC_APP_LIBRARY} STATIC)

## Set include files.
target_include_directories(${TARGET} PRIVATE 
  ${ZZACOMMON_INCLUDE_DIR}
  ${FLTK_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${pugixml_INCLUDE_DIR}
  ${CURL_INCLUDE_DIRS}
)
if (MSVC)
  target_include_directories(${TARGET} PRIVATE
    ${USER_HOME_DIR}\\source\\repos\\json\\include
  )
endif()
target_include_directories(${ZC_APP_LIBRARY} PRIVATE
  ${ZZACOMMON_INCLUDE_DIR}
)
target_link_libraries(${TARGET}
  ${Boost_LIBRARIES}
  ${FLTK_LIBRARIES}
  CURL::libcurl
)

# On MSVC, explicitly link ZLIB so it appears in TARGET_RUNTIME_DLLS
if(MSVC AND ZLIB_FOUND)
  if(TARGET ZLIB::ZLIB)
    target_link_libraries(${TARGET} ZLIB::ZLIB)
    message(STATUS "Linked ZLIB::ZLIB to ${TARGET}")
  elseif(ZLIB_LIBRARIES)
    # Fallback: use old-style variables if target doesn't exist
    target_link_libraries(${TARGET} ${ZLIB_LIBRARIES})
    message(STATUS "Linked ZLIB using variables: ${ZLIB_LIBRARIES}")
  else()
    message(WARNING "ZLIB found but no target or libraries available to link")
  endif()
endif()

## Export include directories
if (NOT PROJECT_IS_TOP_LEVEL)
  set(ZZACOMMON_INCLUDE_DIR ${ZZACOMMON_INCLUDE_DIR} PARENT_SCOPE)
  set(ZZACOMMON_HPPFILES ${ZZACOMMON_HPPFILES} PARENT_SCOPE)
  set(ZZACOMMON_LIBRARIES zzacommon PARENT_SCOPE)
  set(ZZACOMMON_VERSION ${ZZACOMMON_VERSION} PARENT_SCOPE)
  set(ZC_APP_LIBRARY ${ZC_APP_LIBRARY} PARENT_SCOPE)
  set(FLTK_INCLUDE_DIR ${FLTK_INCLUDE_DIR} PARENT_SCOPE)
  set(VCPKG_TARGET_TRIPLET ${VCPKG_TARGET_TRIPLET} PARENT_SCOPE)
  set(_VCPKG_INSTALLED_DIR ${_VCPKG_INSTALLED_DIR} PARENT_SCOPE)
endif()

# Set sources.
target_sources(${TARGET} 
  PRIVATE 
    ${ZZACOMMON_CPPFILES}
)
target_sources(${ZC_APP_LIBRARY}
  PRIVATE
    ${ZZACOMMON_SOURCE_DIR}/src/zc_app.cpp
)

# generate API documentation
if (ZZACOMMON_DOX)
  message(STATUS "ZZACOMMON: Adding API documentation")
  add_subdirectory(api)
  add_dependencies(${TARGET} zzacommon_api_html)
endif()

# Installation rules
if (BUILD_SHARED_LIBS)
  install(TARGETS ${TARGET})
endif()

#[=[
# Install runtime artifacts for distribution
if (MSVC)
  install(IMPORTED_RUNTIME_ARTIFACTS CURL::libcurl DESTINATION bin)
  if (Boost_FOUND)
    install(IMPORTED_RUNTIME_ARTIFACTS Boost::filesystem DESTINATION bin)
  endif()
endif()
#]=]
