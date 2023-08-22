# MIT licensed code, originally taken from
# https://github.com/Munkei/munkei-cmake/blob/master/MunkeiVersionFromGit.cmake
# slightly adapted by the Cherenkov Telescope Array Observatory
include(CMakeParseArguments)

function(version_from_git PROJECT_NAME)
    # Parse arguments
    set(options LOG NO_DIRTY)
    set(oneValueArgs GIT_EXECUTABLE MATCH)
    set(multiValueArgs)
    cmake_parse_arguments(GIT_VERSION "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    if(DEFINED GIT_VERSION_GIT_EXECUTABLE)
        set(GIT_EXECUTABLE "${GIT_VERSION_GIT_EXECUTABLE}")
    else()
        # Find Git or bail out
        find_package(Git)
        if(NOT GIT_FOUND)
            message(FATAL_ERROR "Git not found")
        endif(NOT GIT_FOUND)
    endif()

    # build the git describe command
    set(GIT_COMMAND ${GIT_EXECUTABLE} describe --tags)
    if(DEFINED GIT_VERSION_MATCH)
        list(APPEND GIT_COMMAND "--match=${GIT_VERSION_MATCH}")
    endif()
    if(NOT GIT_VERSION_NO_DIRTY)
        list(APPEND GIT_COMMAND "--dirty")
    endif()

    # Git describe
    execute_process(
        COMMAND           ${GIT_COMMAND}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE   git_result
        OUTPUT_VARIABLE   git_describe
        ERROR_VARIABLE    git_error
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )

    if(NOT git_result EQUAL 0)
        include("generated/${PROJECT_NAME}GitVersion.cmake" OPTIONAL RESULT_VARIABLE VERSION_READ)
        if(NOT VERSION_READ STREQUAL NOTFOUND)
            message(STATUS "Read version info from cache file")
            return()
        endif()

        message(WARNING "Failed to execute Git, setting dummy values for versions: ${git_error}")
        set(GIT_HASH "unknown" PARENT_SCOPE)
        set(GIT_VERSION "0.0.0" PARENT_SCOPE)
        set(GIT_DESCRIBE "0.0.0-dirty" ${git_describe} PARENT_SCOPE)
        set(GIT_VERSION_MAJOR "0" PARENT_SCOPE)
        set(GIT_VERSION_MINOR "0" PARENT_SCOPE)
        set(GIT_VERSION_PATCH "0" PARENT_SCOPE)
        set(GIT_COMMITS_SINCE_TAG "0" PARENT_SCOPE)
        set(GIT_DIRTY ON PARENT_SCOPE)
        return()
    endif()

    # https://regex101.com/r/vsg8Pj/1
    if(git_describe MATCHES "([0-9]+)[.]?([0-9]+)?[.]?([0-9]+)?-?([0-9]+)?-?g?([a-f0-9]+)?(-dirty)?$")
        set(version_major     "${CMAKE_MATCH_1}")
        set(version_minor     "${CMAKE_MATCH_2}")
        set(version_patch     "${CMAKE_MATCH_3}")
        set(commits_since_tag "${CMAKE_MATCH_4}")
        set(git_describe_hash "${CMAKE_MATCH_5}")
    else()
        message(FATAL_ERROR "Git tag isn't valid semantic version: [${git_tag}]")
    endif()

    if("${version_minor}" STREQUAL "")
        set(version_minor 0)
    endif()

    if("${version_patch}" STREQUAL "")
        set(version_patch 0)
    endif()

    if("${commits_since_tag}" STREQUAL "")
        set(commits_since_tag 0)
    endif()

    if("${CMAKE_MATCH_6}" STREQUAL "-dirty")
        set(is_dirty ON)
    else()
        set(is_dirty OFF)
    endif()

    # get full hash of last commit
    execute_process(
        COMMAND           git rev-parse HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE   git_result
        OUTPUT_VARIABLE   git_hash
        ERROR_VARIABLE    git_error
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if(NOT git_result EQUAL 0)
        message(FATAL_ERROR "Failed to get git hash: ${git_error}")
    endif()

    # Construct the version variables
    set(version ${version_major}.${version_minor}.${version_patch})

    # Set parent scope variables
    set(GIT_HASH              ${git_hash} PARENT_SCOPE)
    set(GIT_VERSION           ${version} PARENT_SCOPE)
    set(GIT_DESCRIBE          ${git_describe} PARENT_SCOPE)
    set(GIT_VERSION_MAJOR     ${version_major} PARENT_SCOPE)
    set(GIT_VERSION_MINOR     ${version_minor} PARENT_SCOPE)
    set(GIT_VERSION_PATCH     ${version_patch} PARENT_SCOPE)
    set(GIT_COMMITS_SINCE_TAG ${commits_since_tag} PARENT_SCOPE)
    set(GIT_DIRTY             ${is_dirty} PARENT_SCOPE)

    # Log the results
    if(GIT_VERSION_LOG)
        message(STATUS "GitVersion:")
        message(STATUS "  GIT_VERSION: ${version}")
        message(STATUS "  GIT_HASH: ${git_hash}")
        message(STATUS "  GIT_DESCRIBE: ${git_describe}")
        message(STATUS "  GIT_COMMITS_SINCE_TAG: ${commits_since_tag}")
        message(STATUS "  GIT_DIRTY: ${is_dirty}")
    endif()
    configure_file(cmake/VersionInfo.cmake.in "${CMAKE_CURRENT_SOURCE_DIR}/generated/${PROJECT_NAME}GitVersion.cmake" @ONLY)
endfunction()