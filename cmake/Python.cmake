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

  FIND_PACKAGE (PythonInterp REQUIRED QUIET)

  IF (IS_DIRECTORY $ENV{PYTHONHOME})
    SET (PYTHONROOT $ENV{PYTHONHOME})

    SET (Python_VERSION ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})

    FIND_PATH (PYTHON_INCLUDE_DIR
      NAMES Python.h
      PATHS $ENV{PYTHONHOME}/include
      PATH_SUFFIXES python${Python_VERSION}mu
                    python${Python_VERSION}m
                    python${Python_VERSION}u
                    python${Python_VERSION}
      NO_DEFAULT_PATH)

    FIND_LIBRARY (PYTHON_LIBRARIES
      NAMES python${Python_VERSION}mu
            python${Python_VERSION}m
            python${Python_VERSION}u
            python${Python_VERSION}
      PATHS $ENV{PYTHONHOME}/lib
      NO_SYSTEM_ENVIRONMENT_PATH
      NO_DEFAULT_PATH)
  ELSE ()
    EXECUTE_PROCESS (COMMAND python-config --prefix
      RESULT_VARIABLE _result
      OUTPUT_VARIABLE PYTHONROOT
      ERROR_QUIET)
    FIND_PACKAGE (PythonLibs ${PYTHON_VERSION_STRING} EXACT REQUIRED QUIET)
  ENDIF ()

  IF (PYTHON_EXECUTABLE)
    SET (PYTHON_FOUND TRUE)
  ELSE ()
    SET (PYTHON_FOUND FALSE)
  ENDIF ()

  COLORMSG (HICYAN "Python version: ${PYTHON_VERSION}")

  SET (PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_PATH})
  IF (NOT EXISTS "${PYTHON_INCLUDE_DIR}/Python.h")
    MESSAGE (STATUS "Error: ${PYTHON_INCLUDE_DIR}/Python.h does not exist.\n")
    SET (PYTHON_FOUND FALSE)
    SET (PYTHON_CONFIG_ERROR TRUE)
  ENDIF (NOT EXISTS "${PYTHON_INCLUDE_DIR}/Python.h")

#  EXECUTE_PROCESS (COMMAND python-config --cppflags
#    RESULT_VARIABLE _result
#    OUTPUT_VARIABLE PYTHON_CPPFLAGS
#    ERROR_QUIET)
#  EXECUTE_PROCESS (COMMAND python-config --ldflags
#    RESULT_VARIABLE _result
#    OUTPUT_VARIABLE PYTHON_LDFLAGS
#    ERROR_QUIET)
  SET (PYTHON_CPPFLAGS "-I${PYTHON_INCLUDE_DIR}")
  SET (PYTHON_LDFLAGS "${PYTHON_LIBRARIES}")

ENDIF (NOT PYTHON_FOUND)

IF (PYTHON_FOUND)
  MESSAGE (STATUS "  * binary:   ${PYTHON_EXECUTABLE}")
  MESSAGE (STATUS "  * includes: ${PYTHON_INCLUDE_DIR}")
  MESSAGE (STATUS "  * libs:     ${PYTHON_LIBRARIES}")
ENDIF (PYTHON_FOUND)
