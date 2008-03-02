KDE_OPTIONS = noautodist

INCLUDES = $(all_includes) -I.. -I$(top_srcdir) -I. -I$(top_srcdir)/libkdchart

noinst_LIBRARIES = libreports.a
libreports_a_METASOURCES = AUTO

libreports_a_SOURCES = pivottable.cpp querytable.cpp reportaccount.cpp kreportchartview.cpp

noinst_HEADERS = kreportchartview.h kreportsviewtest.h pivottable.h pivottabletest.h querytable.h querytabletest.h reportaccount.h reportdebug.h reportstestcommon.h kreportchartview.h reporttable.h

if CPPUNIT
check_LIBRARIES = libreportstest.a

libreportstest_a_SOURCES = reportstestcommon.cpp pivottabletest.cpp querytabletest.cpp
endif
