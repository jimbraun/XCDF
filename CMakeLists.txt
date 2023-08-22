cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

include(cmake/GitVersion.cmake)
version_from_git(xcdf LOG)

project(xcdf
  DESCRIPTION "The eXplicitly Compacted Data Format"
  VERSION ${GIT_VERSION}
  HOMEPAGE_URL https://github.com/jimbraun/XCDF
  LANGUAGES CXX C)

# provide version information
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY SameMajorVersion
)

ADD_DEFINITIONS("-Wall -O2")

# ------------------------------------------------------------------------------
# Prevent in-place builds
# ------------------------------------------------------------------------------
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  MESSAGE(FATAL_ERROR "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                XCDF Build Warning:
     Please do not try building inside the source tree; it creates a mess!

 Instructions:
   1) rm -rf CMakeFiles CMakeCache.txt
   2) cd ../build
   3) cmake -DCMAKE_INSTALL_PREFIX=/path/to/install ../src
   4) make && make install
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")

# -----------------------------------------------------------------------------
# Set up CMake policies (if any)
# -----------------------------------------------------------------------------
IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0009 NEW) # Do not follow symlinks w/ FILE_GLOB_RECURSE

  IF(("${CMAKE_VERSION}" VERSION_GREATER 3.0) OR("${CMAKE_VERSION}" VERSION_EQUAL 3.0))
    CMAKE_POLICY(SET CMP0026 OLD) # Read LOCATION properties from build targets
    CMAKE_POLICY(SET CMP0042 OLD) # Disable @rpath in target install name
  ENDIF()
ENDIF(COMMAND cmake_policy)

# ------------------------------------------------------------------------------
# Set up build paths
# ------------------------------------------------------------------------------
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(Utility)
INCLUDE(RPathHandling)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include/utility)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include/alias)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src/pybindings)

# ------------------------------------------------------------------------------
# Set up core library and utility programs
# ------------------------------------------------------------------------------

# add version info file
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/version.cpp")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in ${version_file})

XCDF_ADD_LIBRARY(TARGET xcdf
  HEADERS include/xcdf/*.h
  include/xcdf/*.h.in
  include/xcdf/utility/*.h
  include/xcdf/alias/*.h
  SOURCES src/*.cc ${version_file})

# -------------------------------------
# Build Python bindings using pybind11
# -------------------------------------
OPTION(BUILD_PYTHON "Build python bindings" ON)

if(BUILD_PYTHON)
  message(STATUS "Will build python bindings for XCDF")

  if(SKBUILD)
    message(STATUS "The project is built using scikit-build")

    # Scikit-Build does not add your site-packages to the search path
    # automatically, so we need to add it _or_ the pybind11 specific directory
    # here.
    execute_process(
      COMMAND "${PYTHON_EXECUTABLE}" -c
      "import pybind11; print(pybind11.get_cmake_dir())"
      OUTPUT_VARIABLE _tmp_dir
      OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT)
    list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")
  endif()

  find_package(pybind11 REQUIRED)

  # to avoid a naming conflict with the cpp library but still
  # have the python module called "xcdf" we have the cmake target pyxcdf
  # and set its outputname to xcdf
  pybind11_add_module(pyxcdf src/pybindings/xcdf/xcdf.cpp)

  set_target_properties(pyxcdf PROPERTIES OUTPUT_NAME xcdf)
  target_link_libraries(pyxcdf PRIVATE xcdf)

  if(SKBUILD)
    install(TARGETS pyxcdf xcdf DESTINATION .)
  else()
    install(TARGETS pyxcdf DESTINATION lib)
  endif()
endif()

XCDF_ADD_EXECUTABLE(TARGET seek-test SOURCES tests/SeekTest.cc)
XCDF_ADD_EXECUTABLE(TARGET concat-seek-test SOURCES tests/ConcatSeekTest.cc)
XCDF_ADD_EXECUTABLE(TARGET random-test SOURCES tests/RandomTest.cc)
XCDF_ADD_EXECUTABLE(TARGET speed-test SOURCES tests/SpeedTest.cc)
XCDF_ADD_EXECUTABLE(TARGET simple-test SOURCES tests/SimpleTest.cc)
XCDF_ADD_EXECUTABLE(TARGET buffer-fill-test SOURCES tests/BufferFillTest.cc)
XCDF_ADD_EXECUTABLE(TARGET append-test SOURCES tests/AppendTest.cc)
XCDF_ADD_EXECUTABLE(TARGET utility SOURCES utilities/XCDFUtility.cc EXE_NAME xcdf)

# Add an uninstallation script
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake
")

# ------------------------------------------------------------------------------
# Package control
# ------------------------------------------------------------------------------
SET(CPACK_PACKAGE_NAME "xcdf")
SET(CPACK_PACKAGE_VENDOR "J. Braun <jbraun@wipac.wisc.edu>")
SET(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_MAJOR_VERSION}")
PAD_STRING(CPACK_PACKAGE_VERSION_MINOR 2 0 "${PROJECT_MINOR_VERSION}")
PAD_STRING(CPACK_PACKAGE_VERSION_PATCH 2 0 "${PROJECT_PATCH_VERSION}")
SET(CPACK_SOURCE_GENERATOR "TBZ2")
SET(CPACK_SOURCE_IGNORE_FILES "[.]svn;[.]setting;[.].*project;[.]git.*")

SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
INCLUDE(CPack)

# ---------------------
# Testing suite
# ---------------------
include(CTest)

add_test(NAME seek-test COMMAND xcdf-seek-test)
add_test(NAME concat-seek-test COMMAND xcdf-concat-seek-test)
add_test(NAME random-test COMMAND xcdf-random-test)
add_test(NAME speed-test COMMAND xcdf-speed-test)
add_test(NAME simple-test COMMAND xcdf-simple-test)
add_test(NAME buffer-fill-test COMMAND xcdf-buffer-fill-test)
add_test(NAME append-test COMMAND xcdf-append-test)
