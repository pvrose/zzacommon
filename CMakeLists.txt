cmake_minimum_required(VERSION 3.16)

## Library zzacommon. 
project(zzacommon)
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
set(ZZACOMMON_VERSION "1.0.0")
configure_file(
  ${CMAKE_SOURCE_DIR}/include/zzacommon.h.in
  ${CMAKE_SOURCE_DIR}/include/zzacommon.h
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
  ${CMAKE_SOURCE_DIR}/include/banner.h
  ${CMAKE_SOURCE_DIR}/include/button_input.h
  ${CMAKE_SOURCE_DIR}/include/calendar.h
  ${CMAKE_SOURCE_DIR}/include/calendar_input.h
  ${CMAKE_SOURCE_DIR}/include/calendar_table.h
  ${CMAKE_SOURCE_DIR}/include/callback.h
  ${CMAKE_SOURCE_DIR}/include/drawing.h
  ${CMAKE_SOURCE_DIR}/include/file_holder.h
  ${CMAKE_SOURCE_DIR}/include/filename_input.h
  ${CMAKE_SOURCE_DIR}/include/input_hierch.h
  ${CMAKE_SOURCE_DIR}/include/password_input.h
  ${CMAKE_SOURCE_DIR}/include/settings.h
  ${CMAKE_SOURCE_DIR}/include/status.h
  ${CMAKE_SOURCE_DIR}/include/symbols.h
  ${CMAKE_SOURCE_DIR}/include/tabs_nonav.h
  ${CMAKE_SOURCE_DIR}/include/ticker.h
  ${CMAKE_SOURCE_DIR}/include/utils.h
  ${CMAKE_SOURCE_DIR}/include/zzacommon.h
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
  ./include
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
    FILES ${HPPFILES}
)

# generate API documentation
message(STATUS "Adding API documentation")
add_subdirectory(api)
add_dependencies(${TARGET} api_html)


install(TARGETS ${TARGET})

