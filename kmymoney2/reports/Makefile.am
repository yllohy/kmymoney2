KDE_OPTIONS = noautodist

INCLUDES = $(all_includes) -I.. -I$(top_srcdir) -I. -I$(top_srcdir)/libkdchart

noinst_LIBRARIES = libreports.a
libreports_a_METASOURCES = AUTO

libreports_a_SOURCES = pivotgrid.cpp pivottable.cpp querytable.cpp reportaccount.cpp kreportchartview.cpp

noinst_HEADERS = kreportchartview.h kreportsviewtest.h pivotgrid.h pivottable.h pivottabletest.h pivotgridtest.h querytable.h querytabletest.h reportaccount.h reportdebug.h reportstestcommon.h kreportchartview.h reporttable.h

if CPPUNIT
check_LIBRARIES = libreportstest.a

libreportstest_a_SOURCES = reportstestcommon.cpp pivottabletest.cpp pivotgridtest.cpp querytabletest.cpp
endif
