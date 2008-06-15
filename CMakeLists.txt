# Setting the name of the main project
PROJECT(KMyMoney)

# The cmake version we need
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# Search the KDE3 packages and set the 
FIND_PACKAGE(KDE3 REQUIRED)

FIND_PACKAGE(Perl REQUIRED)
IF (PERL_FOUND)
	MESSAGE(STATUS "Perl found (${PERL_EXECUTABLE})")
ENDIF (PERL_FOUND)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

#set a variable to install the html doc correctly
SET(KDE3HTMLDIR share/doc/HTML)

FIND_PACKAGE(MSGFMT REQUIRED)
IF(MSGFMT_FOUND)
	MESSAGE(STATUS "Program msgfmt found (${MSGFMT_EXECUTABLE})")
ENDIF(MSGFMT_FOUND)

FIND_PACKAGE(MEINPROC REQUIRED)
IF(MEINPROC_FOUND)
	MESSAGE(STATUS "Program meinproc found (${MEINPROC_EXECUTABLE})")
ENDIF(MEINPROC_FOUND)

#SET(CMAKE_VERBOSE_MAKEFILE ON)

# check for JPEG
FIND_PACKAGE(JPEG)
IF (JPEG_FOUND)
  SET(HAVE_LIBJPEG 1)
ENDIF (JPEG_FOUND)

# check for CURL
FIND_PACKAGE(CURL)
IF (CURL_FOUND)
  SET(HAVE_LIBCURL 1)
ENDIF (CURL_FOUND)

# check for PNGL
FIND_PACKAGE(PNG)
IF (PNG_FOUND)
  SET(HAVE_LIBPNG 1)
ENDIF (PNG_FOUND)

# check for Xml2
FIND_PACKAGE(LibXml2)
IF (LIBXML2_FOUND)
  SET(HAVE_LIBXML2 1)
ENDIF (LIBXML2_FOUND)

# check for THREAD model
Find_PACKAGE(Threads)
IF (CMAKE_USE_PTHREADS_INIT)
  SET(HAVE_LIBPTHREAD 1)
ENDIF (CMAKE_USE_PTHREADS_INIT)

# check for ZLIB
FIND_PACKAGE(ZLIB)
IF (ZLIB_FOUND)
  SET(HAVE_LIBZ 1)
ENDIF (ZLIB_FOUND)


INCLUDE(KMyMoneyMacros)

ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE3_DEFINITIONS})
# some definitions for the compiler
ADD_DEFINITIONS( -DKMM_DEBUG=0 -DHAVE_CONFIG_H -fexceptions)

# check some  functions
INCLUDE (CheckFunctionExists) 
CHECK_FUNCTION_EXISTS(atoll HAVE_ATOLL)
CHECK_FUNCTION_EXISTS(vsnprintf HAVE_VSNPRINTF)
CHECK_FUNCTION_EXISTS(strtoll HAVE_STRTOLL)

SET(CMAKE_REQUIRED_FLAGS "-std=c99")
SET(CMAKE_REQUIRED_LIBRARIES m)
CHECK_FUNCTION_EXISTS(round HAVE_ROUND)

# check for function in header
INCLUDE(CheckSymbolExists)
SET(CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE=500 -D_BSD_SOURCE)
CHECK_SYMBOL_EXISTS(res_init "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_RES_INIT)

# check for res_init with prototype
INCLUDE(CheckPrototypeExists)
CHECK_PROTOTYPE_EXISTS(res_init "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_RES_INIT_PROTO)

# special handling of crypt() check
# ----------------------- crypt() ---------------------------------
SET(CMAKE_REQUIRED_LIBRARIES crypt) 
CHECK_FUNCTION_EXISTS(crypt HAVE_CRYPT)
# -----------------------------------------------------------------

# check some include files exists
INCLUDE (CheckIncludeFile)
CHECK_INCLUDE_FILE("stdint.h" HAVE_STDINT_H)
CHECK_INCLUDE_FILE("dlfcn.h" HAVE_DLFCN_H)
CHECK_INCLUDE_FILE("inttypes.h" HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE("memory.h" HAVE_MEMORY_H)
CHECK_INCLUDE_FILE("stdlib.h" HAVE_STDLIB_H)
CHECK_INCLUDE_FILE("strings.h" HAVE_STRINGS_H)
CHECK_INCLUDE_FILE("string.h" HAVE_STRING_H)
CHECK_INCLUDE_FILE("sys/bitypes.h" HAVE_SYS_BITYPES_H)
CHECK_INCLUDE_FILE("sys/stat.h" HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/types.h" HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE("unistd.h" HAVE_UNISTD_H)

#check sizes (TODO --> size in config.h not hard coded!!!! )
INCLUDE(CheckTypeSize)
CHECK_TYPE_SIZE(char* SIZEOF_CHAR_P)
CHECK_TYPE_SIZE(int SIZEOF_INT)
CHECK_TYPE_SIZE(long SIZEOF_LONG)
CHECK_TYPE_SIZE(short SIZEOF_SHORT)
CHECK_TYPE_SIZE(size_t SIZEOF_SIZE_T)
CHECK_TYPE_SIZE("unsigned long" SIZEOF_UNSIGNED_LONG)

# create the config.h file out of the config.h.cmake
CONFIGURE_FILE  (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

# the RPATH to be used when installing
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

LINK_DIRECTORIES(${KDE3_LIB_DIR})

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )

ADD_SUBDIRECTORY( libkgpgfile ) 
ADD_SUBDIRECTORY( kmymoney2 ) 
ADD_SUBDIRECTORY( po ) 
ADD_SUBDIRECTORY( doc ) 
ADD_SUBDIRECTORY( libkdchart )

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

ADD_CUSTOM_TARGET(messages
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/admin/extract-merge.sh
)

# later will include
# ADD_SUBDIRECTORY( developer-doc ) 

########### install files ###############





