AC_DEFUN([AC_LIBOFX],
[
ofx_libs=""
ofx_importerplugin=""

  AC_CHECK_HEADER([OpenSP/macros.h], [
    AC_CHECK_LIB(ofx, libofx_proc_file,[have_new_ofx="yes"], [have_new_ofx="no"])

    if test "$have_new_ofx" == "yes"; then
      AC_DEFINE_UNQUOTED(HAVE_NEW_OFX, "1", [whether OFX >= 0.7.0 is present])
      LIBS="-lofx $LIBS"
      ofx_libs="-lofx"
      ofx_importerplugin="ofximport"
    fi
 ], [], [])
AC_SUBST(ofx_libs)
AC_SUBST(ofx_importerplugin)
])

