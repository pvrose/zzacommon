cmake_minimum_required(VERSION 3.16)

## Library zzacommon. 
project(ZZACOMMON VERSION 1.0.1.1)
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
  ${ZZACOMMON_SOURCE_DIR}/src/banner.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/calendar.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/calendar_input.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/calendar_table.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/file_holder.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/filename_input.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/input_hierch.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/password_input.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/settings.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/status.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/symbols.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/tabs_nonav.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/ticker.cpp
  ${ZZACOMMON_SOURCE_DIR}/src/utils.cpp
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
add_library(${TARGET})

## Set include files.
if (MSVC) 
  ## Add nlohmann/json for MSVC
  set(ZZACOMMON_INCLUDE_DIR ${ZZACOMMON_INCLUDE_DIR} ${USER_HOME_DIR}\\source\\repos\\json\\include)
endif()
message(STATUS "ZZACOMMON: Include dir: ${ZZACOMMON_INCLUDE_DIR}")
target_include_directories(${TARGET} PRIVATE 
  ${ZZACOMMON_INCLUDE_DIR}
  ${FLTK_INCLUDE_DIR}
)
## Export include directories
set(ZZACOMMON_INCLUDE_DIR ${ZZACOMMON_INCLUDE_DIR} PARENT_SCOPE)

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
