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

  # Set project files
  set(DOXY_ZZACOMMON_API_INDEX_FILE index.html)
  set(DOXY_ZZACOMMON_API_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXY_ZZACOMMON_API_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  set(DOXY_ZZACOMMON_API_FILES 
    ${ZZACOMMON_HPPFILES}
  )

  # Copy Doxyfile
  configure_file(${DOXY_ZZACOMMON_API_IN} ${DOXY_ZZACOMMON_API_OUT} @ONLY)
 
  # Command for generating dox from config file
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
    DEPENDS ${DOXY_ZZACOMMON_API_FILES} ${DOXY_ZZACOMMON_API_OUT}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_ZZACOMMON_API_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "ZZACOMMON: Generating API documentation"
    VERBATIM
  )
  add_custom_target(zzacommon_api_html ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  )

endif()
