dnl
dnl AC_CHECK_LIB_WPROLOGUE(LIBRARY, FUNCTION,
dnl              [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
dnl              [OTHER-LIBRARIES], [PROLOGUE])
dnl ------------------------------------------------------
dnl This macro is the same as AC_CHECK_LIB with the
dnl addition of the PROLOGUE variable.  If present, the
dnl prolog is inserted prior to the main() function in
dnl the compiled test code.
dnl
AC_DEFUN([AC_CHECK_LIB_WPROLOGUE],
[m4_ifval([$3], , [AH_CHECK_LIB([$1])])dnl
AS_LITERAL_IF([$1],
              [AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$1_$2])],
              [AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$1''_$2])])dnl
AC_CACHE_CHECK([for $2 in -l$1], ac_Lib,
[ac_check_lib_save_LIBS=$LIBS
LIBS="-l$1 $5 $LIBS"
AC_LINK_IFELSE([AC_LANG_CALL([$6], [$2])],
               [AS_VAR_SET(ac_Lib, yes)],
               [AS_VAR_SET(ac_Lib, no)])
LIBS=$ac_check_lib_save_LIBS])
AS_IF([test AS_VAR_GET(ac_Lib) = yes],
      [m4_default([$3], [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_LIB$1))
  LIBS="-l$1 $LIBS"
])],
      [$4])dnl
AS_VAR_POPDEF([ac_Lib])dnl
])# AC_CHECK_LIB_WPROLOGUE

AC_DEFUN([AC_LIBOFX],
[
  AC_CHECK_HEADER([OpenSP/macros.h], [
    AC_CHECK_LIB(ofx, libofx_proc_file,[have_new_ofx="yes"], [have_new_ofx="no"])

    if test "$have_new_ofx" != "yes"; then
      AC_CHECK_LIB_WPROLOGUE(ofx, ofx_proc_file, [], [], [], [[void ofx_proc_security_cb() {} void ofx_proc_transaction_cb() {} void ofx_proc_statement_cb() {} void ofx_proc_status_cb() {} void ofx_proc_account_cb() {}]])
    else
      AC_DEFINE_UNQUOTED(HAVE_NEW_OFX, "1", [whether new OFX is present])
      LIBS="-lofx $LIBS"
    fi
 ], [], [])
])

