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
find_package(Doxygen)

if(DOXYGEN_FOUND)

  message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

  # Canonical API docs output path that consumers can use.
  set(DOXY_ZZACOMMON_API_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")
  set(ZZACOMMON_API_HTML_DIR "${DOXY_ZZACOMMON_API_HTML_DIR}" CACHE INTERNAL "zzacommon API documentation directory" FORCE)

  # Set project files
  set(DOXY_ZZACOMMON_API_INDEX_FILE "${DOXY_ZZACOMMON_API_HTML_DIR}/index.html")
  set(DOXY_ZZACOMMON_API_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXY_ZZACOMMON_API_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  set(DOXY_ZZACOMMON_API_FILES 
    ${ZZACOMMON_HPPFILES}
  )

  # Copy Doxyfile
  configure_file(${DOXY_ZZACOMMON_API_IN} ${DOXY_ZZACOMMON_API_OUT} @ONLY)
 
  # Command for generating dox from config file
  
  add_custom_command(
    OUTPUT ${DOXY_ZZACOMMON_API_INDEX_FILE}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_ZZACOMMON_API_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${DOXY_ZZACOMMON_API_FILES} ${DOXY_ZZACOMMON_API_OUT}
    COMMENT "ZZACOMMON: Generating API documentation"
    VERBATIM
  )

  add_custom_target(zzacommon_api_html DEPENDS ${DOXY_ZZACOMMON_API_INDEX_FILE})



endif()
