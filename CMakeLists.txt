cmake_minimum_required(VERSION 3.16)

project(zzacommon)

set (TARGET zzacommon)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CPPFILES 
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

set(HPPFILES
  include/banner.h
  include/button_input.h
  include/calendar.h
  include/calendar_input.h
  include/calendar_table.h
  include/callback.h
  include/drawing.h
  include/file_holder.h
  include/filename_input.h
  include/input_hierch.h
  include/password_input.h
  include/settings.h
  include/status.h
  include/symbols.h
  include/tabs_nonav.h
  include/ticker.h
  include/utils.h
)

# Find fltk
set(FLTK_DIR "C:\\Users\\pvros\\source\\repos\\fltk\\build")
set(CMAKE_MODULE_PATH ../ ./)
find_package(FLTK CONFIG REQUIRED)
if (FLTK_FOUND)
  message(STATUS "FLTK found: ${FLTK_VERSION}")
  message(STATUS "FLTK include dir: ${FLTK_INCLUDE_DIR}")
#     set(FLTK_LIBRARIES fltk::fltk fltk::fltk_images)
else()
  message(FATAL_ERROR "FLTK not found")
endif()

set(CMAKE_MODULE_PATH ../)

set(CMAKE_BUILD_RPATH ${CMAKE_CURRENT_BINARY_DIR})
add_library(${TARGET} STATIC)
target_include_directories(${TARGET} PRIVATE 
  ./include
  ${FLTK_INCLUDE_DIR}
)
target_sources(${TARGET} 
  PRIVATE 
    ${CPPFILES}
  PUBLIC 
    FILE_SET common_headers
    TYPE HEADERS
    BASE_DIRS ./include
    FILES ${HPPFILES}
)

install(TARGETS ${TARGET})

