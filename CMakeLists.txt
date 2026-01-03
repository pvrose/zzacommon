cmake_minimum_required(VERSION 3.16)

## Library zzacommon. 
project(ZZACOMMON VERSION 1.0.1)
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
configure_file(
  ${ZZACOMMON_INCLUDE_DIR}/zzacommon.h.in
  ${ZZACOMMON_INCLUDE_DIR}/zzacommon.h
  @ONLY
)

# Source files
set(ZZACOMMON_CPPFILES 
  src/banner.cpp
  src/calendar.cpp
  src/calendar_input.cpp
  src/calendar_table.cpp
  src/file_holder.cpp
  src/filename_input.cpp
  src/input_hierch.cpp
  src/password_input.cpp
  src/settings.cpp
  src/status.cpp
  src/symbols.cpp
  src/tabs_nonav.cpp
  src/ticker.cpp
  src/utils.cpp
)

# Header files - used as dependencies for API documentation
set(ZZACOMMON_HPPFILES
  ${ZZACOMMON_INCLUDE_DIR}/banner.h
  ${ZZACOMMON_INCLUDE_DIR}/button_input.h
  ${ZZACOMMON_INCLUDE_DIR}/calendar.h
  ${ZZACOMMON_INCLUDE_DIR}/calendar_input.h
  ${ZZACOMMON_INCLUDE_DIR}/calendar_table.h
  ${ZZACOMMON_INCLUDE_DIR}/callback.h
  ${ZZACOMMON_INCLUDE_DIR}/drawing.h
  ${ZZACOMMON_INCLUDE_DIR}/file_holder.h
  ${ZZACOMMON_INCLUDE_DIR}/filename_input.h
  ${ZZACOMMON_INCLUDE_DIR}/input_hierch.h
  ${ZZACOMMON_INCLUDE_DIR}/password_input.h
  ${ZZACOMMON_INCLUDE_DIR}/settings.h
  ${ZZACOMMON_INCLUDE_DIR}/status.h
  ${ZZACOMMON_INCLUDE_DIR}/symbols.h
  ${ZZACOMMON_INCLUDE_DIR}/tabs_nonav.h
  ${ZZACOMMON_INCLUDE_DIR}/ticker.h
  ${ZZACOMMON_INCLUDE_DIR}/utils.h
  ${ZZACOMMON_INCLUDE_DIR}/zzacommon.h
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
add_library(${TARGET} STATIC)

## Set include files.
target_include_directories(${TARGET} PRIVATE 
  ${ZZACOMMON_INCLUDE_DIR}
  ${FLTK_INCLUDE_DIR}
)
## Add nlohmann/json for MSVC
if (MSVC)
  target_include_directories(${TARGET} PRIVATE 
    "${USER_HOME_DIR}\\source\\repos\\json\\include"
  )
endif()

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


install(TARGETS ${TARGET})

