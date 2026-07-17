  # Create test executables (EXCLUDE_FROM_ALL means they won't build by default)
  if(NOT ZZAF_INDEX EQUAL -1)
    add_executable(test_zoom_scroll_bar EXCLUDE_FROM_ALL 
      ${ZZACOMMON_SOURCE_DIR}/tests/test_zoom_scroll_bar.cpp
    )
    
    # Link with zzaf component (which includes zc_zoom_scroll_bar)
    target_link_libraries(test_zoom_scroll_bar PRIVATE zzaf)
    
    # Include directories
    target_include_directories(test_zoom_scroll_bar PRIVATE 
      ${ZZACOMMON_INCLUDE_DIR}
      ${FLTK_INCLUDE_DIR}
    )
    
    # Copy runtime DLLs if needed (Windows)
    if(MSVC)
      zzacommon_copy_runtime_dlls(test_zoom_scroll_bar)
    endif()
    
    message(STATUS "Created test target: test_zoom_scroll_bar (build with --target tests)")
  endif()
  
  # Create a 'tests' target that builds all test executables
  add_custom_target(tests)
  if(TARGET test_zoom_scroll_bar)
    add_dependencies(tests test_zoom_scroll_bar)
  endif()
  
  message(STATUS "Created target: tests (build all tests with: cmake --build . --target tests)")
