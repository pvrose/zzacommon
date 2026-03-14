cmake_minimum_required(VERSION 3.16)

project(pugixml)

set (TARGET pugixml)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CPPFILES 
  pugixml.cpp
)

set(HPPFILES
  pugiconfig.hpp
  pugixml.hpp
)

# set compiler options
if (MSVC)
  add_compile_options(/FS)
else()
  add_compile_options(-g)
endif()

set(CMAKE_BUILD_RPATH ${CMAKE_CURRENT_BINARY_DIR})
add_library(${TARGET})
target_include_directories(${TARGET} PRIVATE 
  ./)

target_sources(${TARGET} 
  PRIVATE 
    ${CPPFILES}
  PUBLIC 
    FILE_SET common_headers
    TYPE HEADERS
    FILES ${HPPFILES}
)

set(pugixml_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

if (BUILD_SHARED_OBJECTS)
install(TARGETS ${TARGET})
endif()
