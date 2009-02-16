# Find whether Sqlite is installed (Qt4 package).
# If not, extract our own copy of the sqlite library.
#
# Variables:
#  SQLITE_FOUND - system has Sqlite
#  SQLITE_INCLUDE_DIR - the Sqlite include directory
#  SQLITE_LIBRARIES - Link these to use Sqlite
#  SQLITE_DEFINITIONS - Compiler switches required for using Sqlite

if ( SQLITE_INCLUDE_DIR AND SQLITE_LIBRARIES )
   # in cache already
   SET(Sqlite_FIND_QUIETLY TRUE)
endif ( SQLITE_INCLUDE_DIR AND SQLITE_LIBRARIES )

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if( NOT WIN32 )
  find_package(PkgConfig)

  pkg_check_modules(PC_SQLITE sqlite3)

  set(SQLITE_DEFINITIONS ${PC_SQLITE_CFLAGS_OTHER})
endif( NOT WIN32 )

FIND_PATH(SQLITE_INCLUDE_DIR NAMES sqlite3.h
  PATHS
  ${PC_SQLITE_INCLUDEDIR}
  ${PC_SQLITE_INCLUDE_DIRS}
)

FIND_LIBRARY(SQLITE_LIBRARIES NAMES sqlite3
  PATHS
  ${PC_SQLITE_LIBDIR}
  ${PC_SQLITE_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sqlite DEFAULT_MSG SQLITE_INCLUDE_DIR SQLITE_LIBRARIES )

MARK_AS_ADVANCED(SQLITE_INCLUDE_DIR SQLITE_LIBRARIES )

SET(SQLITE_LIB_DIRS ${PC_SQLITE_LIBDIR} ${PC_SQLITE_LIBRARY_DIRS})

# if the Qt4 libraries are not installed, extract and use our copy
IF(NOT SQLITE_FOUND)
  message(STATUS
    "Sqlite3: Qt4-libraries not found. Using our fallback copy.")
  SET(QSQLITE3_DIR ${CMAKE_BINARY_DIR}/qt-sqlite3-0.2)
  IF(NOT EXISTS ${QSQLITE3_DIR})
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf
      ${CMAKE_SOURCE_DIR}/23011-qt-sqlite3-0.2.tar.gz
      ${CMAKE_BINARY_DIR})
  ENDIF(NOT EXISTS ${QSQLITE3_DIR})

  GET_FILENAME_COMPONENT(QT_BIN_DIR ${QT_MOC_EXECUTABLE} PATH)

  FILE(GLOB_RECURSE QSQLITE3_SOURCES "${QSQLITE3_DIR}/*.cpp")
  ADD_LIBRARY(qsqlite3 SHARED ${QSQLITE3_SOURCES})
  SET_TARGET_PROPERTIES(qsqlite3 PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${QSQLITE3_DIR}/sqldrivers/
    ARCHIVE_OUTPUT_DIRECTORY ${QSQLITE3_DIR}/sqldrivers/)

  SET(SQLITE_LIB_DIRS ${QT_BIN_DIR}/../plugins/sqldrivers/)

  INSTALL(TARGETS qsqlite3
    DESTINATION ${SQLITE_LIB_DIRS})

  SET(SQLITE_LIBRARIES "qsqlite3")
  SET(SQLITE_FOUND TRUE)
ENDIF(NOT SQLITE_FOUND)
