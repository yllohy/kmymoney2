
AC_DEFUN([AC_LIBOFX],
[
AC_MSG_CHECKING(if the OFX importer plugin is desired)
AC_ARG_ENABLE(ofxplugin,
  AC_HELP_STRING([--enable-ofxplugin],[enable OFX importer plugin (default=auto)]),
  [enable_ofxplugin="$enableval"],
  [enable_ofxplugin="auto"])
AC_MSG_RESULT($enable_ofxplugin)

AC_MSG_CHECKING(if OFX direct connect is desired)
AC_ARG_ENABLE(ofxbanking,
  AC_HELP_STRING([--enable-ofxbanking],[enable OFX direct connect (default=auto)]),
  [enable_ofxbanking="$enableval"],
  [enable_ofxbanking="auto"])
AC_MSG_RESULT($enable_ofxbanking)

# make sure we include the plugin even if not mentioned explicitly
if test "$enable_ofxbanking" != "no" -a "$enable_ofxplugin" != "yes"; then
  enable_ofxplugin=$enable_ofxbanking
fi
 
if test "$enable_ofxplugin" != "no" -o "$enable_ofxbanking" != "no"; then
  ac_save_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $all_includes $USER_INCLUDES"
  AC_CHECK_HEADER([OpenSP/macros.h],
   [],
   [
    if test "$enable_ofxplugin" != "auto" -o "$enable_ofxbanking" != "auto"; then
      AC_MSG_ERROR([cannot find OpenSP headers. Please ensure you have OpenSP installed.])
    fi
    AC_MSG_RESULT([cannot find OpenSP headers. Skipping OFX support])
    enable_ofxbanking=no
    enable_ofxplugin=no
   ])
  if test "$enable_ofxplugin" != "no"; then
    CFLAGS="$ac_save_CFLAGS"
    PKG_CHECK_MODULES(OFX,libofx >= 0.8.2)
  fi
fi
if test "$enable_ofxplugin" != "no"; then
 OFX_IMPORTERPLUGIN="ofximport" 
 enable_ofxplugin=yes
fi

if test "$enable_ofxbanking" != "no"; then
  PKG_CHECK_MODULES(LIBXMLPP, libxml++-1.0 >= 1.0.1,
    [
      AC_DEFINE(HAVE_LIBXMLPP, 1, [Defined if libxml++ is available])
      AC_DEFINE_UNQUOTED(USE_OFX_DIRECTCONNECT, 1, [whether to use OFX directconnect])
    ],
    [PKG_CHECK_MODULES(LIBXMLPP,libxml++-2.6 >= 1.0.1,
	[
          AC_DEFINE(HAVE_LIBXMLPP, 1, [Defined if libxml++ is available])
          AC_DEFINE_UNQUOTED(USE_OFX_DIRECTCONNECT, 1, [whether to use OFX directconnect])
        ],
        [
          if test "$enable_ofxbanking" != "auto"; then
            AC_MSG_ERROR([libxml++-1.0 or libxml++-2.6 is required for OFX Direct Connect.])
          fi
          enable_ofxbanking=no
          enable_ofxplugin=no
        ])
    ])

  if test "$enable_ofxbanking" != "no"; then
    LIBCURL_CHECK_CONFIG([yes],[7.9.7])
    OFX_LIBS="$OFX_LIBS $LIBXMLPP_LIBS $LIBCURL"
    enable_ofxbanking=yes
  fi
fi
AM_CONDITIONAL(OFXBANKING, test "$enable_ofxbanking" != "no" )
AC_SUBST(OFX_LIBS)
AC_SUBST(OFX_IMPORTERPLUGIN)
])

