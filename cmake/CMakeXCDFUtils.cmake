# ------------------------------------------------------------------------------
# Macro XCDF_ADD_LIBRARY
# ------------------------------------------------------------------------------
FUNCTION (XCDF_ADD_LIBRARY)
  SET (oneValueArgs TARGET SOURCES)
  SET (multiValueArgs HEADERS)
  CMAKE_PARSE_ARGUMENTS(XCDF_ADD_LIBRARY "${options}" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}")

  SET (_lib "${XCDF_ADD_LIBRARY_TARGET}")
  NO_DOTFILE_GLOB (${_lib}_SOURCES ${XCDF_ADD_LIBRARY_SOURCES})
  NO_DOTFILE_GLOB (${_lib}_HEADERS ${XCDF_ADD_LIBRARY_HEADERS})

  # Build and install library
  ADD_LIBRARY (${XCDF_ADD_LIBRARY_TARGET} SHARED ${${_lib}_SOURCES})
  TARGET_LINK_LIBRARIES (${XCDF_ADD_LIBRARY_TARGET} z m)
  INSTALL (TARGETS ${XCDF_ADD_LIBRARY_TARGET} LIBRARY DESTINATION lib)

  # Install headers
  SET (_include_regex ".*include/(.*)")
  FOREACH (HEADER_FULLPATH ${${_lib}_HEADERS})
    STRING (REGEX MATCH "${_include_regex}" RELPATH ${HEADER_FULLPATH})
    IF (RELPATH)
      STRING (REGEX REPLACE "${_include_regex}" "\\1" HEADER_RELPATH ${HEADER_FULLPATH})
      GET_FILENAME_COMPONENT (RELPATH ${HEADER_RELPATH} PATH)

      # Build any headers that need build-time configuration
      STRING (REGEX MATCH ".*/(.*)\\.in" _build_file ${HEADER_FULLPATH})
      IF (_build_file)
        STRING (REGEX REPLACE ".*/(.*)\\.in" "\\1" _out_file ${_build_file})
        MESSAGE (STATUS "${_out_file}")
        SET (_out_file "${PROJECT_BINARY_DIR}/include/${RELPATH}/${_out_file}")
        CONFIGURE_FILE (${_build_file} ${_out_file} @ONLY)
        INCLUDE_DIRECTORIES (${PROJECT_BINARY_DIR}/include)
        INSTALL (FILES ${_out_file} DESTINATION "include/${RELPATH}")
      # Otherwise just install the headers normally
      ELSE ()
        INSTALL (FILES ${HEADER_FULLPATH} DESTINATION "include/${RELPATH}")
      ENDIF ()
    ENDIF (RELPATH)
  ENDFOREACH (HEADER_FULLPATH ${${_lib}_HEADERS})
ENDFUNCTION( )

# ------------------------------------------------------------------------------
# Macro XCDF_ADD_EXECUTABLE
# ------------------------------------------------------------------------------
FUNCTION (XCDF_ADD_EXECUTABLE)
  SET (oneValueArgs TARGET SOURCES EXE_NAME)

  CMAKE_PARSE_ARGUMENTS(XCDF_ADD_EXECUTABLE "${options}" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}")

  SET (_exe "${XCDF_ADD_EXECUTABLE_TARGET}")
  SET (_exename "xcdf-${XCDF_ADD_EXECUTABLE_TARGET}")

  NO_DOTFILE_GLOB (${_exe}_SOURCES ${XCDF_ADD_EXECUTABLE_SOURCES})

  ADD_EXECUTABLE (${_exename} ${${_exe}_SOURCES})
  TARGET_LINK_LIBRARIES (${_exename} xcdf z m)
  IF (XCDF_ADD_EXECUTABLE_EXE_NAME)
    SET_TARGET_PROPERTIES(${_exename} PROPERTIES OUTPUT_NAME "${XCDF_ADD_EXECUTABLE_EXE_NAME}")
  ENDIF (XCDF_ADD_EXECUTABLE_EXE_NAME)
  INSTALL (TARGETS ${_exename} DESTINATION bin)
ENDFUNCTION( )
