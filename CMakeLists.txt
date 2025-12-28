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
)

# ser compiler options
if (MSVC)
#  add_compile_options(/MD)
else()
  add_compile_options(-g)
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
add_library(${TARGET} STATIC)
target_include_directories(${TARGET} PRIVATE 
  ./include
  ${FLTK_INCLUDE_DIR}
)
if (MSVC)
  target_include_directories(${TARGET} PRIVATE 
    "C:\\Users\\pvros\\source\\repos\\json\\include"
  )
endif()

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

# generate docus
message(STATUS "Adding API documentation")
add_subdirectory(api)
add_dependencies(${TARGET} api_html)


install(TARGETS ${TARGET})

