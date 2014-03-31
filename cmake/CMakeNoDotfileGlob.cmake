
# ------------------------------------------------------------------------------
# Macro: NO_DOTFILE_GLOB
# Globs on each regular expression in the ARGN array and removes matches that
# have a directory component that starts with dot.  From the icetray macro of
# the same name (originally from boost, I think).
# ------------------------------------------------------------------------------
MACRO (NO_DOTFILE_GLOB where_to)
  FOREACH (regex ${ARGN})
    #
    #  if not a regex, do absolute/relative
    #
    IF (NOT ${regex} MATCHES "\\*")
#      MESSAGE("not a regex: >>> ${regex}")
      IF (${regex} MATCHES "^/.*")
        SET (rawglob ${regex})
      ELSE (${regex} MATCHES "^/.*")
        SET (rawglob ${CMAKE_CURRENT_SOURCE_DIR}/${regex})
      ENDIF (${regex} MATCHES "^/.*")
      #
      #  else, not a regex, take it literally as absolute/relative
      #
    ELSE (NOT ${regex} MATCHES "\\*")
      IF (${regex} MATCHES "^/.*")
        FILE (GLOB rawglob ${regex})
      ELSE (${regex} MATCHES "^/.*")
        FILE (GLOB rawglob ${CMAKE_CURRENT_SOURCE_DIR}/${regex})
      ENDIF (${regex} MATCHES "^/.*")
    ENDIF (NOT ${regex} MATCHES "\\*")
#    MESSAGE("RAWGLOB=${rawglob}")
    IF (rawglob)
      FILTER_OUT(".*/\\\\.+.*" "${rawglob}" filterglob)
      IF (filterglob)
        LIST (APPEND ${where_to} ${filterglob})
      ENDIF (filterglob)
    ENDIF (rawglob)
  ENDFOREACH (regex ${ARGN})
ENDMACRO (NO_DOTFILE_GLOB where_to)

