message(STATUS "Building up the project documentation with Doxygen")

find_package(Doxygen)

if(${Doxygen_FOUND})
  configure_file(DoxyFile.in DoxyFile @ONLY)

  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} DoxyFile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)

else(${Doxygen_FOUND})
  message(AUTHOR_WARNING "Couldn't find Doxygen - Documentation cancelled")
endif(${Doxygen_FOUND})
