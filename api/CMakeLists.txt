find_package(Doxygen)

if(DOXYGEN_FOUND)

  message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

  # Set project files
  set(DOXY_API_INDEX_FILE index.html)
  set(DOXY_API_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXY_API_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  #file(GLOB DOXY_API_FILES 
  #  ${CMAKE_CURRENT_SOURCE_DIR}/../contests/include/*.h
  #  ${CMAKE_CURRENT_SOURCE_DIR}/../zzalog/include/*.h
  #  ${CMAKE_CURRENT_SOURCE_DIR}/../common/include/*.h
  #  ${CMAKE_CURRENT_SOURCE_DIR}/../widgets/include/*.h
  #)

  set(DOXY_API_FILES 
    ${HPPFILES}
  )

  # Copy Doxyfile
  configure_file(${DOXY_API_IN} ${DOXY_API_OUT} @ONLY)
 
  # Command for generating dox from config file
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
    DEPENDS ${DOXY_API_FILES} ${DOXY_API_OUT}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_API_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation"
    VERBATIM
  )
  add_custom_target(api_html ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  )

endif()
