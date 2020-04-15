find_package (Doxygen)

if (DOXYGEN_FOUND)
  # Find all the public headers
  set(IOTECH_C_UTILS_PUBLIC_HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../include/iot)
  file(GLOB_RECURSE IOTECH_C_UTILS_PUBLIC_HEADERS ${IOTECH_C_UTILS_PUBLIC_HEADER_DIR}/*.h)

  set(DOXYGEN_INPUT_DIR ${IOTECH_C_UTILS_PUBLIC_HEADER_DIR})
  set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
  set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  # Replace variables inside @@ with the current values
  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

  file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR}) #Doxygen won't create this for us
  add_custom_target(doc_doxygen ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
  install (DIRECTORY ${DOXYGEN_OUTPUT_DIR}/html DESTINATION docs)
endif (DOXYGEN_FOUND)
