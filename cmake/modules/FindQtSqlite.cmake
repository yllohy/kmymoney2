# Find whether Qt-Sqlite3 is installed
# And if not, make sure our tarball gets extracted and compiled
#
# Variables:
#  QT_SQLITE_FOUND - system has Qt-Sqlite3
#  QT_SQLITE_FALLBACK - our fallback library is used

FIND_PACKAGE(Sqlite)

GET_FILENAME_COMPONENT(QT_BIN_DIR ${QT_MOC_EXECUTABLE} PATH)
GET_FILENAME_COMPONENT(QT_DIR     ${QT_BIN_DIR}        PATH)
SET(QT_SQLITE3_LIB_DIR ${QT_DIR}/plugins/sqldrivers)

# Look for libsqlite3.lib[64].so in ${QT_SQLITE3_LIB_DIR}
FIND_LIBRARY(QT_SQLITE3_LIB NAMES sqlite3.lib64 sqlite3.lib
  HINTS ${QT_SQLITE3_LIB_DIR})

if(QT_SQLITE3_LIB)
  message(STATUS "Found Qt-Sqlite3 library: ${QT_SQLITE3_LIB}")
  SET(QT_SQLITE_FOUND TRUE)
  SET(QT_SQLITE_FALLBACK FALSE)
else(QT_SQLITE3_LIB)
  if(SQLITE_FOUND)
    SET(QSQLITE3_DIR ${CMAKE_BINARY_DIR}/qt-sqlite3-0.2)
    IF(NOT EXISTS ${QSQLITE3_DIR})
      message(STATUS "Qt-Sqlite3 not found in ${QT_SQLITE3_LIB_DIR}
      No problem, extracting and compiling our fallback library.")
      EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E tar xzf
        ${CMAKE_SOURCE_DIR}/23011-qt-sqlite3-0.2.tar.gz
        ${CMAKE_BINARY_DIR})
    ENDIF(NOT EXISTS ${QSQLITE3_DIR})

    GET_FILENAME_COMPONENT(QT_BIN_DIR ${QT_MOC_EXECUTABLE} PATH)

    FILE(GLOB_RECURSE QSQLITE3_SOURCES "${QSQLITE3_DIR}/*.cpp")

    ADD_LIBRARY(qsqlite3 SHARED ${QSQLITE3_SOURCES})
    SET_TARGET_PROPERTIES(qsqlite3 PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY ${QSQLITE3_DIR}/sqldrivers/)

    INSTALL(TARGETS qsqlite3
      DESTINATION ${QT_SQLITE3_LIB_DIR}
      COMPONENT qsqlite3)
    ADD_CUSTOM_TARGET(install-qsqlite3 ${CMAKE_COMMAND}
      -DCMAKE_INSTALL_COMPONENT=qsqlite3 -P cmake_install.cmake)

    SET(QT_SQLITE_FOUND TRUE)
    SET(QT_SQLITE_FALLBACK TRUE)
  else(SQLITE_FOUND)
    message(STATUS "Qt-Sqlite3 not found, but also Sqlite3 is missing. Sqlite3 support disabled.")
    SET(QT_SQLITE_FOUND FALSE)
    SET(QT_SQLITE_FALLBACK FALSE)
  endif(SQLITE_FOUND)
endif(QT_SQLITE3_LIB)
