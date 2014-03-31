################################################################################
# Module to find Python                                                        #
#                                                                              #
# This module will call the default CMake python modules and define:           #
#                                                                              #
#   PYTHON_FOUND                                                               #
#   PYTHON_LIBRARIES                                                           #
#   PYTHON_INCLUDE_DIR                                                         #
#   PYTHON_CPPFLAGS                                                            #
#   PYTHON_LDFLAGS                                                             #
#                                                                              #
# plus the other PYTHON_* variables defined by CMake (see CMake documentation) #
################################################################################

IF (NOT PYTHON_FOUND)
  SET (PYTHONROOT $ENV{PYTHONHOME})

  IF (IS_DIRECTORY ${PYTHONROOT})

    FIND_PATH (PYTHON_EXECUTABLE_DIR python
      PATHS ${PYTHONROOT}/bin
      NO_DEFAULT_PATH)

    IF (IS_DIRECTORY ${PYTHON_EXECUTABLE_DIR})
      SET (PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE_DIR}/python)
      
      EXECUTE_PROCESS (COMMAND ${PYTHON_EXECUTABLE} -V
        ERROR_VARIABLE PYTHON_VERSION_STRING
        ERROR_STRIP_TRAILING_WHITESPACE)
      SEPARATE_ARGUMENTS (PYTHON_VERSION_STRING)

      SET (PYTHON_VERSION "")
      SET (PYTHON_MAJOR_VERSION "")
      FOREACH (item ${PYTHON_VERSION_STRING})
        SET (PYTHON_VERSION ${item})
      ENDFOREACH (item ${PYTHON_VERSION_STRING})

      STRING(LENGTH ${PYTHON_VERSION} POS)
      MATH(EXPR POS "${POS} - 2")
      STRING(SUBSTRING ${PYTHON_VERSION} 0 ${POS} PYTHON_MAJOR_VERSION)

      SET (PYTHON_INCLUDE_PATH ${PYTHONROOT}/include/python${PYTHON_MAJOR_VERSION})

      FIND_PATH (PYTHON_INCLUDE_DIR Python.h
        PATHS ${PYTHON_INCLUDE_PATH} 
        NO_DEFAULT_PATH)

      FIND_LIBRARY (PYTHON_LIBRARIES NAMES python2.7
        PATHS ${PYTHONROOT}/lib
        NO_DEFAULT_PATH)

      SET (PYTHON_CPPFLAGS "-I${PYTHON_INCLUDE_DIR}")
      SET (PYTHON_LDFLAGS "${PYTHON_LIBRARIES}")
    ENDIF (IS_DIRECTORY ${PYTHON_EXECUTABLE_DIR})

    IF (IS_DIRECTORY ${PYTHON_INCLUDE_DIR} AND EXISTS ${PYTHON_LIBRARIES})
      COLORMSG (HICYAN "Python version: ${PYTHON_VERSION}")
      SET (PYTHON_FOUND TRUE)
      SET (PYTHONLIBS_FOUND TRUE)
    ELSE (IS_DIRECTORY ${PYTHON_INCLUDE_DIR} AND EXISTS ${PYTHON_LIBRARIES})
      MESSAGE (STATUS "Python not found in PYTHONHOME = $ENV{PYTHONHOME}")
      MESSAGE (STATUS "Searching in standard locations...")
      SET (PYTHON_FOUND FALSE)
    ENDIF (IS_DIRECTORY ${PYTHON_INCLUDE_DIR} AND EXISTS ${PYTHON_LIBRARIES})
    
  ENDIF (IS_DIRECTORY ${PYTHONROOT})
  
ENDIF (NOT PYTHON_FOUND)

IF (NOT PYTHON_FOUND)

  FIND_PACKAGE (PythonInterp QUIET)
  FIND_PACKAGE (PythonLibs QUIET)

  IF (PYTHON_EXECUTABLE)
    SET (PYTHON_FOUND TRUE)
  ELSE ()
    SET (PYTHON_FOUND FALSE)
  ENDIF ()

  EXECUTE_PROCESS (COMMAND ${PYTHON_EXECUTABLE} -V
    ERROR_VARIABLE PYTHON_VERSION_STRING
    ERROR_STRIP_TRAILING_WHITESPACE)
  SEPARATE_ARGUMENTS (PYTHON_VERSION_STRING)
  SET (PYTHON_VERSION "")
  FOREACH (item ${PYTHON_VERSION_STRING})
    SET (PYTHON_VERSION ${item})
  ENDFOREACH (item ${PYTHON_VERSION_STRING})

  COLORMSG (HICYAN "Python version: ${PYTHON_VERSION}")

  SET (PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_PATH})
  IF (NOT EXISTS "${PYTHON_INCLUDE_DIR}/Python.h")
    MESSAGE (STATUS "Error: ${PYTHON_INCLUDE_DIR}/Python.h does not exist.\n")
    SET (PYTHON_FOUND FALSE)
    SET (PYTHON_CONFIG_ERROR TRUE)
  ENDIF (NOT EXISTS "${PYTHON_INCLUDE_DIR}/Python.h")

  SET (PYTHON_CPPFLAGS "-I${PYTHON_INCLUDE_DIR}")
  SET (PYTHON_LDFLAGS "${PYTHON_LIBRARIES}")

ENDIF (NOT PYTHON_FOUND)

IF (PYTHON_FOUND)
  MESSAGE (STATUS "  * binary:   ${PYTHON_EXECUTABLE}")
  MESSAGE (STATUS "  * includes: ${PYTHON_INCLUDE_DIR}")
  MESSAGE (STATUS "  * libs:     ${PYTHON_LIBRARIES}")
ENDIF (PYTHON_FOUND)
