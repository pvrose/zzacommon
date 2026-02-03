cmake_minimum_required(VERSION 3.16)

## Library zzacommon. 
project(ZZACOMMON VERSION 1.0.2)
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
  set(APP_NAME "UNNAMED")
endif()
if (NOT APP_VENDOR)
  set(APP_VENDOR "GM3ZZA")
endif()
configure_file(
  ${ZZACOMMON_INCLUDE_DIR}/zzacommon.h.in
  ${ZZACOMMON_INCLUDE_DIR}/zzacommon.h
  @ONLY
)

message(STATUS "ZZACOMMON for parent ${APP_NAME}")

# Source files
set(ZZACOMMON_CPPFILES 
  ${ZZACOMMON_SOURCE_DIR}/src/zc_banner.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_calendar.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_calendar_input.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_calendar_table.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_file_holder.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_filename_input.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_graph.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_input_hierch.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_password_input.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_settings.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_status.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_symbols.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_tabs_nonav.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_ticker.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_utils.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/zc_fft.cpp
)

# Header files - used as dependencies for API documentation
set(ZZACOMMON_HPPFILES
  ${ZZACOMMON_INCLUDE_DIR}/zc_banner.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_button_input.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_calendar.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_calendar_input.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_calendar_table.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_callback.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_drawing.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_file_holder.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_filename_input.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_graph.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_input_hierch.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_password_input.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_settings.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_status.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_symbols.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_tabs_nonav.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_ticker.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_utils.h
  ${ZZACOMMON_INCLUDE_DIR}/zzacommon.h
  ${ZZACOMMON_INCLUDE_DIR}/zc_fft.h
)

# set compiler options
if (MSVC)
#  add_compile_options(/MD)
else()
  add_compile_options(-g)
endif()

# Find fltk
# MSVC work-round as FLTK built in user.
if (NOT FLTK_DIR AND MSVC)
  set(FLTK_DIR "${USER_HOME_DIR}\\source\\repos\\fltk\\build")
endif()
set(CMAKE_MODULE_PATH ../ ./)
find_package(FLTK CONFIG)
if (FLTK_FOUND)
  message(STATUS "FLTK found: ${FLTK_VERSION}")
  message(STATUS "FLTK include dir: ${FLTK_INCLUDE_DIR}")
  message(STATUS "FLTK libraries: ${FLTK_LIBRARIES}")
else()
  message(FATAL_ERROR "FLTK not found")
endif()

set(CMAKE_MODULE_PATH ../)

## Target is libzzacommon
add_library(${TARGET})

## Set include files.
target_include_directories(${TARGET} PRIVATE 
  ${ZZACOMMON_INCLUDE_DIR}
  ${FLTK_INCLUDE_DIR}
)
if (MSVC)
  target_include_directories(${TARGET} PRIVATE
    ${USER_HOME_DIR}\\source\\repos\\json\\include
  )
endif()

## Export include directories
set(ZZACOMMON_INCLUDE_DIR ${ZZACOMMON_INCLUDE_DIR} PARENT_SCOPE)
set(ZZACOMMON_HPPFILES ${ZZACOMMON_HPPFILES} PARENT_SCOPE)

# Set sources.
target_sources(${TARGET} 
  PRIVATE 
    ${ZZACOMMON_CPPFILES}
  PUBLIC 
    FILE_SET common_headers
    TYPE HEADERS
    BASE_DIRS ./include
    FILES ${ZZACOMMON_HPPFILES}
)

# generate API documentation
message(STATUS "ZZACOMMON: Adding API documentation")
add_subdirectory(api)
add_dependencies(${TARGET} zzacommon_api_html)


if (MSVC)
else()
  if (BUILD_SHARED_LIBS)
    install(TARGETS ${TARGET})
  endif()
endif()
