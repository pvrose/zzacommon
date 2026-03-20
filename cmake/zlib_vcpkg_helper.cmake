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

function (find_zlib)
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
endfunction()

function (copy_zlib_dlls)
  # Manual fallback for ZLIB DLL (debug and release versions)
  # vcpkg stores debug DLLs in debug/bin and release DLLs in bin
  set(VCPKG_BIN_DIR "")
  set(VCPKG_DEBUG_BIN_DIR "")

  if(_VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
    set(VCPKG_BIN_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
    set(VCPKG_DEBUG_BIN_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin")
  elseif(ZLIB_ROOT)
    # ZLIB_ROOT is set in CMakePresets.json
    set(VCPKG_BIN_DIR "${ZLIB_ROOT}/bin")
    set(VCPKG_DEBUG_BIN_DIR "${ZLIB_ROOT}/debug/bin")
  elseif(VCPKG_ROOT)
    # Deduce from VCPKG_ROOT
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(VCPKG_BIN_DIR "${VCPKG_ROOT}/installed/x64-windows/bin")
      set(VCPKG_DEBUG_BIN_DIR "${VCPKG_ROOT}/installed/x64-windows/debug/bin")
    else()
      set(VCPKG_BIN_DIR "${VCPKG_ROOT}/installed/x86-windows/bin")
      set(VCPKG_DEBUG_BIN_DIR "${VCPKG_ROOT}/installed/x86-windows/debug/bin")
    endif()
  endif()

  # Copy ZLIB DLLs from both debug and release directories
  set(ALL_ZLIB_DLLS "")

  # Check release bin directory
  if(VCPKG_BIN_DIR AND EXISTS "${VCPKG_BIN_DIR}")
    message(STATUS "Searching for ZLIB DLLs in: ${VCPKG_BIN_DIR}")
    file(GLOB RELEASE_ZLIB_DLLS "${VCPKG_BIN_DIR}/zlib*.dll")
    if(RELEASE_ZLIB_DLLS)
      list(APPEND ALL_ZLIB_DLLS ${RELEASE_ZLIB_DLLS})
      message(STATUS "Found release ZLIB DLLs: ${RELEASE_ZLIB_DLLS}")
    endif()
  endif()

  # Check debug bin directory
  if(VCPKG_DEBUG_BIN_DIR AND EXISTS "${VCPKG_DEBUG_BIN_DIR}")
    message(STATUS "Searching for debug ZLIB DLLs in: ${VCPKG_DEBUG_BIN_DIR}")
    file(GLOB DEBUG_ZLIB_DLLS "${VCPKG_DEBUG_BIN_DIR}/zlib*.dll")
    if(DEBUG_ZLIB_DLLS)
      list(APPEND ALL_ZLIB_DLLS ${DEBUG_ZLIB_DLLS})
      message(STATUS "Found debug ZLIB DLLs: ${DEBUG_ZLIB_DLLS}")
    endif()
  endif()

  # Copy all found ZLIB DLLs
  if(ALL_ZLIB_DLLS)
    foreach(ZLIB_DLL ${ALL_ZLIB_DLLS})
      add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${ZLIB_DLL}"
          "${RUNTIME_OUTPUT_DIR}"
        COMMENT "Copying ZLIB DLL: ${ZLIB_DLL}"
      )
    endforeach()
    message(STATUS "Will copy ZLIB DLLs: ${ALL_ZLIB_DLLS}")
  else()
    message(WARNING "No ZLIB DLLs found in ${VCPKG_BIN_DIR} or ${VCPKG_DEBUG_BIN_DIR}")
  endif()

endfunction()
