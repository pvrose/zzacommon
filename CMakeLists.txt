cmake_minimum_required(VERSION 3.16)

project(zzacommon)

set (TARGET zzacommon)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VENDOR "GM3ZZA")
set(PROGRAMID "ZZALOG")

set(CPPFILES 
  src/utils.cpp
)

set(HPPFILES
  include/callback.h
  include/drawing.h
  include/utils.h
)

# set document directory
if(MSVC)
  set(CMAKE_INSTALL_RPATH C:/ProgramData/${VENDOR}/${PROGRAMID})
else()
  set(CMAKE_INSTALL_RPATH /etc/${VENDOR}/${PROGRAMID})
endif()

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
add_library(${TARGET})
target_include_directories(${TARGET} PRIVATE 
  ./include
  ${FLTK_INCLUDE_DIR}
)
# Add QBS_logo.h to ensure it gets generated
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

