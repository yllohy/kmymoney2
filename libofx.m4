AC_DEFUN([AC_LIBOFX],
[
AC_MSG_CHECKING(if the OFX importer plugin is desired)
AC_ARG_ENABLE(ofxplugin,
  [  --enable-ofxplugin      enable OFX support (default=yes)],
  enable_ofxplugin="$enableval",
  enable_ofxplugin="yes")
AC_MSG_RESULT($enable_ofxplugin)

AC_MSG_CHECKING(if OFX direct connect is desired)
AC_ARG_ENABLE(ofxbanking,
  [  --enable-ofxbanking     enable OFX direct connect (default=no)],
  enable_ofxbanking="$enableval",
  enable_ofxbanking="no")
AC_MSG_RESULT($enable_ofxbanking)

ofx_libs=""
ofx_importerplugin=""

if test "$enable_ofxplugin" != "no"; then
  AC_CHECK_HEADER([OpenSP/macros.h], [
    AC_CHECK_LIB(ofx, libofx_proc_file,[have_new_ofx="yes"], [have_new_ofx="no"])
    if test "$have_new_ofx" == "yes"; then
      ofx_importerplugin="ofximport"
      ofx_libs="-lofx"
    fi
 ], [], [])
fi

if test "$enable_ofxbanking" != "no"; then
  AC_CHECK_HEADER([OpenSP/macros.h], [
    AC_CHECK_LIB(ofx, libofx_proc_file,[have_new_ofx="yes"], [have_new_ofx="no"])
    if test "$have_new_ofx" == "yes"; then
      AC_DEFINE_UNQUOTED(USE_OFX_DIRECTCONNECT, "1", [whether to use OFX directconnect])
      LIBS="-lofx $LIBS"
    fi
 ], [], [])
fi
AC_SUBST(ofx_libs)
AC_SUBST(ofx_importerplugin)
])

