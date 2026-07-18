  # Create test executables (EXCLUDE_FROM_ALL means they won't build by default)
  if(NOT ZZAF_INDEX EQUAL -1)


    # Create a 'tests' target that builds all test executables
    add_custom_target(tests)

    foreach(TEST
      zoom_scroll_bar
      graph_bar_horizontal
    )

    # Loop through TESTS

    add_executable(test_${TEST} EXCLUDE_FROM_ALL 
      ${ZZACOMMON_SOURCE_DIR}/tests/test_${TEST}.cpp
    )
    
    # Link with zzaf component (which includes zc_zoom_scroll_bar)
    target_link_libraries(test_${TEST} PRIVATE zzaf)
    
    # Include directories
    target_include_directories(test_${TEST} PRIVATE 
      ${ZZACOMMON_INCLUDE_DIR}
      ${FLTK_INCLUDE_DIR}
    )
    
    # Copy runtime DLLs if needed (Windows)
    if(MSVC)
      zzacommon_copy_runtime_dlls(test_${TEST})
    endif()
    
    message(STATUS "Created test target: test_${TEST} (build with --target tests)")
    
    if (TARGET test_${TEST})
      add_dependencies(tests test_${TEST})
    endif()

    endforeach()

  endif()
  
  message(STATUS "Created target: tests (build all tests with: cmake --build . --target tests)")
