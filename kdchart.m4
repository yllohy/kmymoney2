dnl
dnl Somewhat lame macro to check for kdchart.  All it really does is
dnl set "HAVE_KDCHART" if you enable it, because I couldn't figure out
dnl how to check for it
dnl

AC_DEFUN([AC_KDCHART],
[
AC_MSG_CHECKING([if charts are desired for reports (requires KDChart)])
AC_ARG_ENABLE(charts,
  AC_HELP_STRING([--enable-charts],[Enable charts in reports (default=no)]),
  [ 
	enable_charts="$enableval" 
	AC_DEFINE_UNQUOTED(HAVE_KDCHART, "1", [[whether kdchart is available]]) 
  	LIBS="-lkdchart $LIBS"
  ],
  enable_charts="no")
AC_MSG_RESULT($enable_charts)
])
