# RPATH handling. This makes setting LD_LIBRARY_PATH unecessary
# as it will compile the path to the libraries into the libraries / executables
if(APPLE)
    message(STATUS "ON APPLE")
    cmake_policy(SET CMP0068 NEW)
    list(APPEND CMAKE_INSTALL_RPATH "@executable_path/../lib")
    list(APPEND CMAKE_INSTALL_RPATH "@loader_path")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_NAME_DIR "@rpath")
else()
    list(APPEND CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
    list(APPEND CMAKE_INSTALL_RPATH "\$ORIGIN")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
endif()

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
