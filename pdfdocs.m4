dnl
dnl check the pdf generation option
dnl if enabled or disabled, directly controlled
dnl
AC_DEFUN([AC_PDF_GENERATION], [
  AC_MSG_CHECKING(if the PDF document generation is desired)
  AC_ARG_ENABLE( pdf-docs,
    [  --enable-pdf-docs       enable generation of PDF documents (default=no)],
    enable_pdfdocs="$enableval",
    enable_pdfdocs="no")
  AC_MSG_RESULT($enable_pdfdocs)

  AM_CONDITIONAL(GENERATE_PDF, test "x$enable_pdfdocs" = "xyes")

  if test "x$enable_pdfdocs" = "xyes"; then
    AC_ARG_WITH(dblatex-dir, [  --with-dblatex-dir=DIR
                          uses dblatex from given dir],
      [lcc_dir="$withval:$withval/bin:$PATH"],
      [lcc_dir="$PATH"])
    AC_PATH_PROG(dblatexbin, dblatex, "", $lcc_dir)
    AC_SUBST(dblatexbin)
    if test "x$dblatexbin" != "x"; then
      AC_CHECK_PROG(found_latex, latex, yes, no)
      AC_CHECK_PROG(found_xsltproc, xsltproc, yes, no)
    fi
  fi

  AM_CONDITIONAL(HAVE_DOCCONVERSIONTOOLS, test "x$enable_pdfdocs" = "xyes" -a "x$found_latex" = "xyes" -a "x$found_xsltproc" = "xyes")
])

