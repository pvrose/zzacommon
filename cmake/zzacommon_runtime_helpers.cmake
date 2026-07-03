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

# zzacommon Runtime DLL Helper Functions
# This module provides:
# 1. DLL detection functions (zzacommon_find_*_dlls) to locate vcpkg DLLs during zzacommon build
# 2. DLL copy/install functions for user apps to deploy DLLs

# ====== DLL DETECTION FUNCTIONS (used during zzacommon build) ======

# Function to detect zlib DLL paths from vcpkg
# Sets: ZZACOMMON_ZLIB_DLL_RELEASE, ZZACOMMON_ZLIB_DLL_DEBUG
function(zzacommon_find_zlib_dlls VCPKG_ROOT_DIR)
    if(NOT MSVC)
        return()
    endif()

    if(NOT VCPKG_ROOT_DIR OR NOT EXISTS "${VCPKG_ROOT_DIR}")
        return()
    endif()

    set(VCPKG_BIN_DIR "${VCPKG_ROOT_DIR}/installed/x64-windows/bin")
    set(VCPKG_DEBUG_BIN_DIR "${VCPKG_ROOT_DIR}/installed/x64-windows/debug/bin")

    # Find release DLL
    if(EXISTS "${VCPKG_BIN_DIR}")
        file(GLOB ZLIB_RELEASE_DLLS "${VCPKG_BIN_DIR}/zlib*.dll")
        if(ZLIB_RELEASE_DLLS)
            list(GET ZLIB_RELEASE_DLLS 0 _ZLIB_DLL_RELEASE)
            set(ZZACOMMON_ZLIB_DLL_RELEASE "${_ZLIB_DLL_RELEASE}" PARENT_SCOPE)
            set(ZZACOMMON_ZLIB_DLL_RELEASE "${_ZLIB_DLL_RELEASE}" CACHE INTERNAL "zzacommon zlib release dll" FORCE)
            message(STATUS "zzacommon: Found ZLIB release DLL: ${_ZLIB_DLL_RELEASE}")
        endif()
    endif()

    # Find debug DLL
    if(EXISTS "${VCPKG_DEBUG_BIN_DIR}")
        file(GLOB ZLIB_DEBUG_DLLS "${VCPKG_DEBUG_BIN_DIR}/zlib*.dll")
        if(ZLIB_DEBUG_DLLS)
            list(GET ZLIB_DEBUG_DLLS 0 _ZLIB_DLL_DEBUG)
            set(ZZACOMMON_ZLIB_DLL_DEBUG "${_ZLIB_DLL_DEBUG}" PARENT_SCOPE)
            set(ZZACOMMON_ZLIB_DLL_DEBUG "${_ZLIB_DLL_DEBUG}" CACHE INTERNAL "zzacommon zlib debug dll" FORCE)
            message(STATUS "zzacommon: Found ZLIB debug DLL: ${_ZLIB_DLL_DEBUG}")
        endif()
    endif()
endfunction()

# Function to detect Boost DLL paths from vcpkg
# Sets: ZZACOMMON_BOOST_DLLS_RELEASE, ZZACOMMON_BOOST_DLLS_DEBUG
function(zzacommon_find_boost_dlls VCPKG_ROOT_DIR)
    if(NOT MSVC)
        return()
    endif()

    if(NOT VCPKG_ROOT_DIR OR NOT EXISTS "${VCPKG_ROOT_DIR}")
        return()
    endif()

    set(VCPKG_BIN_DIR "${VCPKG_ROOT_DIR}/installed/x64-windows/bin")
    set(VCPKG_DEBUG_BIN_DIR "${VCPKG_ROOT_DIR}/installed/x64-windows/debug/bin")

    set(_BOOST_DLLS_RELEASE "")
    set(_BOOST_DLLS_DEBUG "")

    # Find release Boost DLLs
    if(EXISTS "${VCPKG_BIN_DIR}")
        file(GLOB BOOST_FILESYSTEM_RELEASE_DLLS "${VCPKG_BIN_DIR}/boost_filesystem*.dll")
        file(GLOB BOOST_SYSTEM_RELEASE_DLLS "${VCPKG_BIN_DIR}/boost_system*.dll")
        file(GLOB BOOST_ASIO_RELEASE_DLLS "${VCPKG_BIN_DIR}/boost_asio*.dll")
        list(APPEND _BOOST_DLLS_RELEASE ${BOOST_FILESYSTEM_RELEASE_DLLS} ${BOOST_SYSTEM_RELEASE_DLLS} ${BOOST_ASIO_RELEASE_DLLS})
        if(_BOOST_DLLS_RELEASE)
            list(REMOVE_DUPLICATES _BOOST_DLLS_RELEASE)
            set(ZZACOMMON_BOOST_DLLS_RELEASE "${_BOOST_DLLS_RELEASE}" PARENT_SCOPE)
            set(ZZACOMMON_BOOST_DLLS_RELEASE "${_BOOST_DLLS_RELEASE}" CACHE INTERNAL "zzacommon boost release dlls" FORCE)
            message(STATUS "zzacommon: Found Boost release DLLs: ${_BOOST_DLLS_RELEASE}")
        endif()
    endif()

    # Find debug Boost DLLs
    if(EXISTS "${VCPKG_DEBUG_BIN_DIR}")
        file(GLOB BOOST_FILESYSTEM_DEBUG_DLLS "${VCPKG_DEBUG_BIN_DIR}/boost_filesystem*.dll")
        file(GLOB BOOST_SYSTEM_DEBUG_DLLS "${VCPKG_DEBUG_BIN_DIR}/boost_system*.dll")
        file(GLOB BOOST_ASIO_DEBUG_DLLS "${VCPKG_DEBUG_BIN_DIR}/boost_asio*.dll")
        list(APPEND _BOOST_DLLS_DEBUG ${BOOST_FILESYSTEM_DEBUG_DLLS} ${BOOST_SYSTEM_DEBUG_DLLS} ${BOOST_ASIO_DEBUG_DLLS})
        if(_BOOST_DLLS_DEBUG)
            list(REMOVE_DUPLICATES _BOOST_DLLS_DEBUG)
            set(ZZACOMMON_BOOST_DLLS_DEBUG "${_BOOST_DLLS_DEBUG}" PARENT_SCOPE)
            set(ZZACOMMON_BOOST_DLLS_DEBUG "${_BOOST_DLLS_DEBUG}" CACHE INTERNAL "zzacommon boost debug dlls" FORCE) 
            message(STATUS "zzacommon: Found Boost debug DLLs: ${_BOOST_DLLS_DEBUG}")       
        endif()
    endif()
endfunction()

# Function to detect CURL DLL paths from vcpkg
# Sets: ZZACOMMON_CURL_DLL_RELEASE, ZZACOMMON_CURL_DLL_DEBUG
function(zzacommon_find_curl_dlls VCPKG_ROOT_DIR)
    if(NOT MSVC)
        return()
    endif()

    if(NOT VCPKG_ROOT_DIR OR NOT EXISTS "${VCPKG_ROOT_DIR}")
        return()
    endif()

    set(VCPKG_BIN_DIR "${VCPKG_ROOT_DIR}/installed/x64-windows/bin")
    set(VCPKG_DEBUG_BIN_DIR "${VCPKG_ROOT_DIR}/installed/x64-windows/debug/bin")

    # Find release DLL
    if(EXISTS "${VCPKG_BIN_DIR}")
        file(GLOB CURL_RELEASE_DLLS "${VCPKG_BIN_DIR}/libcurl*.dll")
        if(CURL_RELEASE_DLLS)
            list(GET CURL_RELEASE_DLLS 0 _CURL_DLL_RELEASE)
            set(ZZACOMMON_CURL_DLL_RELEASE "${_CURL_DLL_RELEASE}" PARENT_SCOPE)
            set(ZZACOMMON_CURL_DLL_RELEASE "${_CURL_DLL_RELEASE}" CACHE INTERNAL "zzacommon curl release dll" FORCE)
            message(STATUS "zzacommon: Found CURL release DLL: ${_CURL_DLL_RELEASE}")
        endif()
    endif()

    # Find debug DLL
    if(EXISTS "${VCPKG_DEBUG_BIN_DIR}")
        file(GLOB CURL_DEBUG_DLLS "${VCPKG_DEBUG_BIN_DIR}/libcurl*.dll")
        if(CURL_DEBUG_DLLS)
            list(GET CURL_DEBUG_DLLS 0 _CURL_DLL_DEBUG)
            set(ZZACOMMON_CURL_DLL_DEBUG "${_CURL_DLL_DEBUG}" PARENT_SCOPE)
            set(ZZACOMMON_CURL_DLL_DEBUG "${_CURL_DLL_DEBUG}" CACHE INTERNAL "zzacommon curl debug dll" FORCE)
            message(STATUS "zzacommon: Found CURL debug DLL: ${_CURL_DLL_DEBUG}")
        endif()
    endif()
endfunction()

function(_zzacommon_restore_runtime_dll_vars_from_cache)
    foreach(_v
        ZZACOMMON_ZLIB_DLL_RELEASE
        ZZACOMMON_ZLIB_DLL_DEBUG
        ZZACOMMON_CURL_DLL_RELEASE
        ZZACOMMON_CURL_DLL_DEBUG
        ZZACOMMON_BOOST_DLLS_RELEASE
        ZZACOMMON_BOOST_DLLS_DEBUG
    )
        if((NOT DEFINED ${_v}) OR ("${${_v}}" STREQUAL ""))
            get_property(_cached CACHE ${_v} PROPERTY VALUE)
            if(_cached)
                set(${_v} "${_cached}" PARENT_SCOPE)
            endif()
        endif()
    endforeach()
endfunction()

function(_zzacommon_collect_explicit_runtime_dlls OUT_VAR)
    set(_dlls "")

    # Debug config
    if(ZZACOMMON_ZLIB_DLL_DEBUG AND EXISTS "${ZZACOMMON_ZLIB_DLL_DEBUG}")
        list(APPEND _dlls "$<$<CONFIG:Debug>:${ZZACOMMON_ZLIB_DLL_DEBUG}>")
    endif()
    if(ZZACOMMON_CURL_DLL_DEBUG AND EXISTS "${ZZACOMMON_CURL_DLL_DEBUG}")
        list(APPEND _dlls "$<$<CONFIG:Debug>:${ZZACOMMON_CURL_DLL_DEBUG}>")
    endif()
    foreach(DLL ${ZZACOMMON_BOOST_DLLS_DEBUG})
        if(EXISTS "${DLL}")
            list(APPEND _dlls "$<$<CONFIG:Debug>:${DLL}>")
        endif()
    endforeach()

    # Non-Debug configs (Release/RelWithDebInfo/MinSizeRel)
    if(ZZACOMMON_ZLIB_DLL_RELEASE AND EXISTS "${ZZACOMMON_ZLIB_DLL_RELEASE}")
        list(APPEND _dlls "$<$<NOT:$<CONFIG:Debug>>:${ZZACOMMON_ZLIB_DLL_RELEASE}>")
    endif()
    if(ZZACOMMON_CURL_DLL_RELEASE AND EXISTS "${ZZACOMMON_CURL_DLL_RELEASE}")
        list(APPEND _dlls "$<$<NOT:$<CONFIG:Debug>>:${ZZACOMMON_CURL_DLL_RELEASE}>")
    endif()
    foreach(DLL ${ZZACOMMON_BOOST_DLLS_RELEASE})
        if(EXISTS "${DLL}")
            list(APPEND _dlls "$<$<NOT:$<CONFIG:Debug>>:${DLL}>")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES _dlls)
    set(${OUT_VAR} "${_dlls}" PARENT_SCOPE)
endfunction()

# ====== DLL COPY/INSTALL FUNCTIONS (used by user apps) ======

# Helper function to copy runtime DLLs to target output directory
# Usage: zzacommon_copy_runtime_dlls(target_name)
# Copies all vcpkg-provided runtime DLLs plus target's own DLLs
function(zzacommon_copy_runtime_dlls TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "Target ${TARGET_NAME} does not exist")
    endif()

    if(NOT MSVC)
        message(STATUS "zzacommon: Runtime DLL copy is only needed on Windows/MSVC")
        return()
    endif()

    _zzacommon_restore_runtime_dll_vars_from_cache()

    _zzacommon_collect_explicit_runtime_dlls(_EXPLICIT_DLLS)


    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${TARGET_NAME}>
            ${_EXPLICIT_DLLS}
            $<TARGET_FILE_DIR:${TARGET_NAME}>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime DLLs for ${TARGET_NAME}"
    )

    message(STATUS "zzacommon: Runtime DLL copy configured for ${TARGET_NAME}")
    message(STATUS "zzacommon: ZLIB debug DLL var='${ZZACOMMON_ZLIB_DLL_DEBUG}'")
endfunction()

# Helper function to install runtime DLLs alongside the application
# Usage: zzacommon_install_runtime_dlls(TARGET target_name [DESTINATION dir] [COMPONENT comp_name])
# Installs all vcpkg-provided runtime DLLs plus target's own DLLs
function(zzacommon_install_runtime_dlls)
    set(options "")
    set(oneValueArgs TARGET DESTINATION COMPONENT)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_TARGET)
        message(FATAL_ERROR "zzacommon_install_runtime_dlls: TARGET parameter is required")
        return()
    endif()

    if(NOT TARGET ${ARG_TARGET})
        message(FATAL_ERROR "zzacommon_install_runtime_dlls: Target ${ARG_TARGET} does not exist")
        return()
    endif()

    if(NOT MSVC)
        message(STATUS "zzacommon: Runtime DLL install is only needed on Windows/MSVC")
        return()
    endif()

    if(NOT ARG_DESTINATION)
        set(ARG_DESTINATION "bin")
    endif()

    if(NOT ARG_COMPONENT)
        set(ARG_COMPONENT "runtime")
    endif()

    _zzacommon_restore_runtime_dll_vars_from_cache()
    _zzacommon_collect_explicit_runtime_dlls(DLLS_TO_INSTALL)


    if(DLLS_TO_INSTALL)
        list(REMOVE_DUPLICATES DLLS_TO_INSTALL)
        install(FILES ${DLLS_TO_INSTALL}
            DESTINATION ${ARG_DESTINATION}
            COMPONENT ${ARG_COMPONENT}
        )
        list(LENGTH DLLS_TO_INSTALL DLL_COUNT)
        message(STATUS "zzacommon: Will install ${DLL_COUNT} runtime DLL(s) to ${ARG_DESTINATION}")
    else()
        message(WARNING "zzacommon: No runtime DLLs available for installation")
    endif()
endfunction()

# Legacy function for API documentation copy (unchanged)
# Usage: zzacommon_copy_api_docs(destination_directory)
function(zzacommon_copy_api_docs DESTINATION_DIR)
    if(NOT ZZACOMMON_API_HTML_DIR)
        message(WARNING "zzacommon: API documentation directory not available. Build zzacommon with ZZACOMMON_DOX=ON to generate documentation.")
        return()
    endif()

    if(NOT EXISTS "${ZZACOMMON_API_HTML_DIR}")
        message(WARNING "zzacommon: API documentation directory does not exist: ${ZZACOMMON_API_HTML_DIR}")
        return()
    endif()

    file(MAKE_DIRECTORY "${DESTINATION_DIR}")

    file(COPY "${ZZACOMMON_API_HTML_DIR}/"
        DESTINATION "${DESTINATION_DIR}"
        PATTERN "*"
    )

    message(STATUS "zzacommon: Copied API documentation from ${ZZACOMMON_API_HTML_DIR} to ${DESTINATION_DIR}")
endfunction()