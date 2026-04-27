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
# This module provides functions for detecting and managing runtime DLLs (zlib, Boost, CURL)

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
            message(STATUS "zzacommon: Found ZLIB release DLL: ${_ZLIB_DLL_RELEASE}")
        endif()
    endif()

    # Find debug DLL
    if(EXISTS "${VCPKG_DEBUG_BIN_DIR}")
        file(GLOB ZLIB_DEBUG_DLLS "${VCPKG_DEBUG_BIN_DIR}/zlib*.dll")
        if(ZLIB_DEBUG_DLLS)
            list(GET ZLIB_DEBUG_DLLS 0 _ZLIB_DLL_DEBUG)
            set(ZZACOMMON_ZLIB_DLL_DEBUG "${_ZLIB_DLL_DEBUG}" PARENT_SCOPE)
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
        list(APPEND _BOOST_DLLS_RELEASE ${BOOST_FILESYSTEM_RELEASE_DLLS} ${BOOST_SYSTEM_RELEASE_DLLS})
        if(_BOOST_DLLS_RELEASE)
            list(REMOVE_DUPLICATES _BOOST_DLLS_RELEASE)
            set(ZZACOMMON_BOOST_DLLS_RELEASE "${_BOOST_DLLS_RELEASE}" PARENT_SCOPE)
            message(STATUS "zzacommon: Found Boost release DLLs: ${_BOOST_DLLS_RELEASE}")
        endif()
    endif()

    # Find debug Boost DLLs
    if(EXISTS "${VCPKG_DEBUG_BIN_DIR}")
        file(GLOB BOOST_FILESYSTEM_DEBUG_DLLS "${VCPKG_DEBUG_BIN_DIR}/boost_filesystem*.dll")
        file(GLOB BOOST_SYSTEM_DEBUG_DLLS "${VCPKG_DEBUG_BIN_DIR}/boost_system*.dll")
        list(APPEND _BOOST_DLLS_DEBUG ${BOOST_FILESYSTEM_DEBUG_DLLS} ${BOOST_SYSTEM_DEBUG_DLLS})
        if(_BOOST_DLLS_DEBUG)
            list(REMOVE_DUPLICATES _BOOST_DLLS_DEBUG)
            set(ZZACOMMON_BOOST_DLLS_DEBUG "${_BOOST_DLLS_DEBUG}" PARENT_SCOPE)
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
            message(STATUS "zzacommon: Found CURL release DLL: ${_CURL_DLL_RELEASE}")
        endif()
    endif()

    # Find debug DLL
    if(EXISTS "${VCPKG_DEBUG_BIN_DIR}")
        file(GLOB CURL_DEBUG_DLLS "${VCPKG_DEBUG_BIN_DIR}/libcurl*.dll")
        if(CURL_DEBUG_DLLS)
            list(GET CURL_DEBUG_DLLS 0 _CURL_DLL_DEBUG)
            set(ZZACOMMON_CURL_DLL_DEBUG "${_CURL_DLL_DEBUG}" PARENT_SCOPE)
            message(STATUS "zzacommon: Found CURL debug DLL: ${_CURL_DLL_DEBUG}")
        endif()
    endif()
endfunction()

# Helper function to copy zlib DLL to target output directory
# Usage: zzacommon_copy_zlib_dll(target_name)
function(zzacommon_copy_zlib_dll TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "Target ${TARGET_NAME} does not exist")
        return()
    endif()

    if(ZZACOMMON_ZLIB_DLL_RELEASE OR ZZACOMMON_ZLIB_DLL_DEBUG)
        if(ZZACOMMON_ZLIB_DLL_RELEASE AND EXISTS "${ZZACOMMON_ZLIB_DLL_RELEASE}")
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${ZZACOMMON_ZLIB_DLL_RELEASE}"
                    "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                COMMENT "Copying zlib DLL (Release) to $<TARGET_FILE_DIR:${TARGET_NAME}>"
            )
            message(STATUS "zzacommon: Will copy zlib1.dll (Release) to ${TARGET_NAME} output directory")
        endif()

        if(ZZACOMMON_ZLIB_DLL_DEBUG AND EXISTS "${ZZACOMMON_ZLIB_DLL_DEBUG}")
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${ZZACOMMON_ZLIB_DLL_DEBUG}"
                    "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                COMMENT "Copying zlib DLL (Debug) to $<TARGET_FILE_DIR:${TARGET_NAME}>"
            )
            message(STATUS "zzacommon: Will copy zlib1d.dll (Debug) to ${TARGET_NAME} output directory")
        endif()
    else()
        message(WARNING "zzacommon: No zlib DLL paths available. Make sure zzax component was built with vcpkg.")
    endif()
endfunction()

# Helper function to copy Boost DLLs to target output directory
# Usage: zzacommon_copy_boost_dlls(target_name)
function(zzacommon_copy_boost_dlls TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "Target ${TARGET_NAME} does not exist")
        return()
    endif()

    if(ZZACOMMON_BOOST_DLLS_RELEASE OR ZZACOMMON_BOOST_DLLS_DEBUG)
        # Copy release Boost DLLs
        if(ZZACOMMON_BOOST_DLLS_RELEASE)
            foreach(BOOST_DLL ${ZZACOMMON_BOOST_DLLS_RELEASE})
                if(EXISTS "${BOOST_DLL}")
                    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${BOOST_DLL}"
                            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                        COMMENT "Copying Boost DLL (Release): ${BOOST_DLL}"
                    )
                endif()
            endforeach()
            message(STATUS "zzacommon: Will copy Boost release DLLs to ${TARGET_NAME} output directory")
        endif()

        # Copy debug Boost DLLs
        if(ZZACOMMON_BOOST_DLLS_DEBUG)
            foreach(BOOST_DLL ${ZZACOMMON_BOOST_DLLS_DEBUG})
                if(EXISTS "${BOOST_DLL}")
                    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "${BOOST_DLL}"
                            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                        COMMENT "Copying Boost DLL (Debug): ${BOOST_DLL}"
                    )
                endif()
            endforeach()
            message(STATUS "zzacommon: Will copy Boost debug DLLs to ${TARGET_NAME} output directory")
        endif()
    else()
        message(WARNING "zzacommon: No Boost DLL paths available. Make sure zzab or zzafb component was built with vcpkg.")
    endif()
endfunction()

# Helper function to copy CURL DLL to target output directory
# Usage: zzacommon_copy_curl_dll(target_name)
function(zzacommon_copy_curl_dll TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "Target ${TARGET_NAME} does not exist")
        return()
    endif()

    if(ZZACOMMON_CURL_DLL_RELEASE OR ZZACOMMON_CURL_DLL_DEBUG)
        if(ZZACOMMON_CURL_DLL_RELEASE AND EXISTS "${ZZACOMMON_CURL_DLL_RELEASE}")
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${ZZACOMMON_CURL_DLL_RELEASE}"
                    "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                COMMENT "Copying CURL DLL (Release) to $<TARGET_FILE_DIR:${TARGET_NAME}>"
            )
            message(STATUS "zzacommon: Will copy libcurl.dll (Release) to ${TARGET_NAME} output directory")
        endif()

        if(ZZACOMMON_CURL_DLL_DEBUG AND EXISTS "${ZZACOMMON_CURL_DLL_DEBUG}")
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${ZZACOMMON_CURL_DLL_DEBUG}"
                    "$<TARGET_FILE_DIR:${TARGET_NAME}>"
                COMMENT "Copying CURL DLL (Debug) to $<TARGET_FILE_DIR:${TARGET_NAME}>"
            )
            message(STATUS "zzacommon: Will copy libcurl-d.dll (Debug) to ${TARGET_NAME} output directory")
        endif()
    else()
        message(WARNING "zzacommon: No CURL DLL paths available. Make sure zzax component was built with vcpkg.")
    endif()
endfunction()

# Convenience function to copy all required DLLs (zlib + Boost + CURL)
# Usage: zzacommon_copy_runtime_dlls(target_name)
function(zzacommon_copy_runtime_dlls TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "Target ${TARGET_NAME} does not exist")
        return()
    endif()

    # Copy zlib DLL if zzax component is used
    if(ZZACOMMON_ZLIB_DLL_RELEASE OR ZZACOMMON_ZLIB_DLL_DEBUG)
        zzacommon_copy_zlib_dll(${TARGET_NAME})
    endif()

    # Copy Boost DLLs if zzab or zzafb component is used
    if(ZZACOMMON_BOOST_DLLS_RELEASE OR ZZACOMMON_BOOST_DLLS_DEBUG)
        zzacommon_copy_boost_dlls(${TARGET_NAME})
    endif()

    # Copy CURL DLL if zzax component is used
    if(ZZACOMMON_CURL_DLL_RELEASE OR ZZACOMMON_CURL_DLL_DEBUG)
        zzacommon_copy_curl_dll(${TARGET_NAME})
    endif()
endfunction()

# Function to install runtime DLLs alongside the application
# Usage: zzacommon_install_runtime_dlls(DESTINATION destination_dir [COMPONENT component_name])
function(zzacommon_install_runtime_dlls)
    set(options "")
    set(oneValueArgs DESTINATION COMPONENT)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_DESTINATION)
        set(ARG_DESTINATION "bin")
    endif()

    if(NOT ARG_COMPONENT)
        set(ARG_COMPONENT "runtime")
    endif()

    # Collect all DLLs to install
    set(DLLS_TO_INSTALL "")

    # Add zlib DLLs
    if(ZZACOMMON_ZLIB_DLL_RELEASE AND EXISTS "${ZZACOMMON_ZLIB_DLL_RELEASE}")
        list(APPEND DLLS_TO_INSTALL "${ZZACOMMON_ZLIB_DLL_RELEASE}")
    endif()
    if(ZZACOMMON_ZLIB_DLL_DEBUG AND EXISTS "${ZZACOMMON_ZLIB_DLL_DEBUG}")
        list(APPEND DLLS_TO_INSTALL "${ZZACOMMON_ZLIB_DLL_DEBUG}")
    endif()

    # Add Boost DLLs
    if(ZZACOMMON_BOOST_DLLS_RELEASE)
        foreach(BOOST_DLL ${ZZACOMMON_BOOST_DLLS_RELEASE})
            if(EXISTS "${BOOST_DLL}")
                list(APPEND DLLS_TO_INSTALL "${BOOST_DLL}")
            endif()
        endforeach()
    endif()
    if(ZZACOMMON_BOOST_DLLS_DEBUG)
        foreach(BOOST_DLL ${ZZACOMMON_BOOST_DLLS_DEBUG})
            if(EXISTS "${BOOST_DLL}")
                list(APPEND DLLS_TO_INSTALL "${BOOST_DLL}")
            endif()
        endforeach()
    endif()

    # Add CURL DLLs
    if(ZZACOMMON_CURL_DLL_RELEASE AND EXISTS "${ZZACOMMON_CURL_DLL_RELEASE}")
        list(APPEND DLLS_TO_INSTALL "${ZZACOMMON_CURL_DLL_RELEASE}")
    endif()
    if(ZZACOMMON_CURL_DLL_DEBUG AND EXISTS "${ZZACOMMON_CURL_DLL_DEBUG}")
        list(APPEND DLLS_TO_INSTALL "${ZZACOMMON_CURL_DLL_DEBUG}")
    endif()

    # Install the DLLs
    if(DLLS_TO_INSTALL)
        list(REMOVE_DUPLICATES DLLS_TO_INSTALL)
        install(FILES ${DLLS_TO_INSTALL}
            DESTINATION ${ARG_DESTINATION}
            COMPONENT ${ARG_COMPONENT}
        )
        list(LENGTH DLLS_TO_INSTALL DLL_COUNT)
        message(STATUS "zzacommon: Will install ${DLL_COUNT} runtime DLL(s) to ${ARG_DESTINATION}")
    else()
        message(WARNING "zzacommon: No runtime DLLs available for installation.")
    endif()
endfunction()

# Helper function to copy API documentation to user documentation directory
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

    # Create destination directory if it doesn't exist
    file(MAKE_DIRECTORY "${DESTINATION_DIR}")

    # Copy the entire html directory
    file(COPY "${ZZACOMMON_API_HTML_DIR}/"
        DESTINATION "${DESTINATION_DIR}"
        PATTERN "*"
    )

    message(STATUS "zzacommon: Copied API documentation from ${ZZACOMMON_API_HTML_DIR} to ${DESTINATION_DIR}")
endfunction()
