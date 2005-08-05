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

  AM_CONDITIONAL(GENERATE_PDF, test x$enable_pdfdocs == xyes)

  if test x$enable_pdfdocs == xyes; then
    dnl html2ps and ps2pdf
    AC_CHECK_PROG(found_html2ps, html2ps, yes, no)
    AC_CHECK_PROG(found_ps2pdf, ps2pdf14, yes, no)
    AC_CHECK_PROG(found_jade, jade, yes, no)
    AC_CHECK_PROG(found_pdfjadetex, pdfjadetex, yes, no)
  fi

  AM_CONDITIONAL(HAVE_HTMLCONVERSIONTOOLS, test x$enable_pdfdocs == xyes -a x$found_ps2pdf = xyes -a x$found_html2ps == xyes)
  AM_CONDITIONAL(HAVE_DSSSLCONVERSIONTOOLS, test x$enable_pdfdocs == xyes -a x$found_jade = xyes -a x$found_pdfjadetex == xyes)
])

