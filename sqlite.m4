# sqlite3.m4
# ----------
#
# Copyright 2008 by Thomas Baumgart
#
# License: See file COPYING
#
# Checks for the necessity to build our own qt-sqlite3 support library
# and the presence of the sqlite3 development headers.
#
# Supports the following options:
#
# --enable-sqlite3
# --disable-sqlite3
#
# If none of them is present, the detection is automatic. If the development
# headers are not found for automatic detection, the generation will be
# disabled w/o error. If --enable-sqlite3 is provided in the same stage,
# an error is given if the sqlite3 development files are not installed.
#
# The following variables are provided via AC_SUBST:
#
# SQLITE3 - contains the subdirectory to visit for compilation
# LIBSQLITE3 - contains the full pathname for the Qt plugin driver


AC_DEFUN([AC_SQLITE3], [
  AC_MSG_CHECKING(if the SQLITE3 support is desired)
  AC_ARG_ENABLE(sqlite3,
    AC_HELP_STRING([--enable-sqlite3],[build SQLITE3 support library (default=auto)]),
    enable_sqlite3="$enableval",
    enable_sqlite3="auto")
  AC_MSG_RESULT($enable_sqlite3)

  if test ! $enable_sqlite3 = no; then
    # determine name and path of sqlite3 plugin library
    qtlib=""
    if test ! "lib${kdelibsuff}" = "lib"; then
      qtlib=".lib64"
    fi 
    LIBSQLITE3=${QTDIR}/plugins/sqldrivers/libsqlite3${qtlib}.so

    # do the checks
    if test $enable_sqlite3 = auto; then
      AC_MSG_CHECKING(if the SQLITE3 support is already present)
      if test ! -e ${LIBSQLITE3}; then
        result=no
      else
        # add check for local qt-sqlite3 directory here
        result=yes
        enable_sqlite3=no
        # in case a previous run unpacked the SQLITE3 support stuff
        # it is pretty sure that we have build the existing support
        # if that's the case, we just enable it again
        if test -d qt-sqlite3-0.2; then
          enable_sqlite3=auto
        fi
      fi
      AC_MSG_RESULT($result)
    fi

    # we only need to check for the headers in case we need to build
    if test ! $enable_sqlite3 = no; then
      # now check for the presence of SQLITE libraries
      AC_CHECK_HEADER([sqlite3.h], [enable_sqlite3=yes],
        [
          if test $enable_sqlite3 = auto; then
            enable_sqlite3=no
          else
            AC_MSG_ERROR(SQLITE development files not found)
          fi
        ])
    fi
  fi

  if test $enable_sqlite3 = yes; then
    rm -rf qt-sqlite3-0.2
    gunzip -c `dirname -- ${0}`/23011-qt-sqlite3-0.2.tar.gz | tar -xf -
    cd qt-sqlite3-0.2
    ${QTDIR}/bin/qmake QMAKE=${QTDIR}/bin/qmake
    sed -ie s/^install:.*$/install:/ Makefile
    sed -ie s/^uninstall:.*$/uninstall:/ Makefile
    cd ..
    SQLITE3=qt-sqlite3-0.2
    AC_SUBST(SQLITE3)
    AC_SUBST(LIBSQLITE3) 
  fi
])
