
AC_DEFUN([AC_LIBOFX],
[
AC_MSG_CHECKING(if the OFX importer plugin is desired)
AC_ARG_ENABLE(ofxplugin,
  AC_HELP_STRING([--enable-ofxplugin],[enable OFX importer plugin (default=no)]),
  enable_ofxplugin="$enableval",
  enable_ofxplugin="no")
AC_MSG_RESULT($enable_ofxplugin)

AC_MSG_CHECKING(if OFX direct connect is desired)
AC_ARG_ENABLE(ofxbanking,
  AC_HELP_STRING([--enable-ofxbanking],[enable OFX direct connect (default=no)]),
  enable_ofxbanking="$enableval",
  enable_ofxbanking="no")
AC_MSG_RESULT($enable_ofxbanking)

if test "$enable_ofxplugin" != "no"; then
  AC_CHECK_HEADER([OpenSP/macros.h],[],[AC_MSG_ERROR([cannot find OpenSP headers. Please ensure you have OpenSP installed.])])
  PKG_CHECK_MODULES(OFX,libofx >= 0.8.1)
 OFX_IMPORTERPLUGIN="ofximport" 
fi

if test "$enable_ofxbanking" != "no"; then
  AC_CHECK_HEADER([OpenSP/macros.h],[],[AC_MSG_ERROR([cannot find OpenSP headers. Please ensure you have OpenSP installed.])])
  PKG_CHECK_MODULES(OFX,libofx >= 0.8.1)
  AC_DEFINE_UNQUOTED(USE_OFX_DIRECTCONNECT, 1, [whether to use OFX directconnect])
  PKG_CHECK_MODULES(LIBXMLPP,libxml++-1.0 >= 1.0.1,
	[AC_DEFINE(HAVE_LIBXMLPP, 1, [Defined if libxml++ is available])],
        [AC_MSG_ERROR([libxml++ is required for OFX Direct Connect.])])

  LIBCURL_CHECK_CONFIG([yes],[7.9.7])
  LIBS="$OFX_LIBS $LIBXMLPP_LIBS $LIBCURL $LIBS"
fi
AM_CONDITIONAL(OFXBANKING, test "$enable_ofxbanking" != "no" )
AC_SUBST(OFX_LIBS)
AC_SUBST(OFX_IMPORTERPLUGIN)
])


