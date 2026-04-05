#[=[
	Copyright 2026, Philip Rose, GM3ZZA
	
    This file is part of ZZALOG. Amateur Radio Logging Software.

    ZZALOG is free software: you can redistribute it and/or modify it under the
	terms of the Lesser GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later version.

    ZZALOG is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
	PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with ZZALOG. 
	If not, see <https://www.gnu.org/licenses/>. 

#]=]

# CMake helper function for FFTW integration.
# Note this assumes that FFTW has been installed:
# for MSVC using the packages in https://www.fftw.org/install/windows.html.
# For Linux, use the package manager to install the appropriate FFTW packages.
function(find_fftw)
	if (MSVC)
	    if (FFTW_ROOT)
			message(STATUS "Using FFTW_ROOT: ${FFTW_ROOT}")
			set(FFTW3_INCLUDE_DIRS "${FFTW_ROOT}")
			set(FFTW3_LIBRARIES "${FFTW_ROOT}/libfftw3-3.lib")
			set(FFTW3_DLL "${FFTW_ROOT}/libfftw3-3.dll")
			add_custom_target(fftw_dll ALL)
			add_custom_command(TARGET fftw_dll POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy ${FFTW3_DLL} ${CMAKE_BINARY_DIR}
			)
		else()
		  message(FATAL_ERROR "FFTW_ROOT environment variable must be set to the FFTW installation directory for MSVC builds.")
		endif()
	else()
		message(FATAL_ERROR "FFTW3 not found. Please install FFTW3 using your package manager.")
	endif()
	set(FFTW3_INCLUDE_DIRS "${FFTW3_INCLUDE_DIRS}" PARENT_SCOPE)
	set(FFTW3_LIBRARIES "${FFTW3_LIBRARIES}" PARENT_SCOPE)
	set(FFTW3_DLL "${FFTW3_DLL}" PARENT_SCOPE)
endfunction()

# Copy FFTW DLLs to the install directory.
function(install_fftw_dlls)
   install(FILES ${FFTW3_DLL} 
	   DESTINATION bin
	   COMPONENT applications
   )
endfunction()