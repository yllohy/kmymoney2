dnl
dnl Somewhat lame macro to check for kdchart.  All it really does is
dnl set "HAVE_KDCHART" if you enable it, because I couldn't figure out
dnl how to check for it
dnl

AC_DEFUN([AC_KDCHART],
[
AC_MSG_CHECKING([if charts are desired for reports])
AC_ARG_ENABLE(charts,
  AC_HELP_STRING([--enable-charts],[Enable charts in reports (default=no)]),
  [
    enable_charts="$enableval" 
    AC_MSG_RESULT($enable_charts)
    AC_MSG_CHECKING([if KDChart library is available])
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
    kde_ldflags_safe="$LDFLAGS"
    kde_libs_safe="$LIBS"
    kde_cxxflags_safe="$CXXFLAGS"

    LDFLAGS="$LDFLAGS -L$qt_libdir $all_libraries $USER_LDFLAGS $KDE_MT_LDFLAGS"
    LIBS="-lkdchart $LIBS $LIBQT $KDE_MT_LIBS"
    CXXFLAGS="$CXXFLAGS -I$prefix/include -I$kde_includes $all_includes"

    AC_TRY_LINK(
      [#include <KDChartWidget.h>],
      [
        QWidget *w = 0;
        char *name = 0;
        KDChartWidget chart(w,name);
      ],
      [
        AC_DEFINE_UNQUOTED(HAVE_KDCHART, "1", [Define if you have libkdchart]) 
      ],
      [
        enable_charts="no"
      ]
    )

    LDFLAGS=$kde_ldflags_safe
    LIBS=$kde_libs_safe
    CXXFLAGS=$kde_cxxflags_safe
    AC_LANG_RESTORE

    AC_MSG_RESULT($enable_charts)
    if test "x$enable_charts" = "xyes"; then
      LIBS="-lkdchart $LIBS"
    fi
  ],
  [
    enable_charts="no"
    AC_MSG_RESULT($enable_charts)
  ])
])
