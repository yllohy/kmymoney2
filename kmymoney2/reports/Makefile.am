noinst_LIBRARIES = libreports.a

INCLUDES = $(all_includes) -I..

libreports_a_METASOURCES = AUTO

libreports_a_SOURCES = pivottable.cpp querytable.cpp reportaccount.cpp

EXTRA_DIST = pivottable.cpp pivottable.h querytable.cpp querytable.h reportaccount.cpp reportaccount.h

if CPPUNIT
check_LIBRARIES = libreportstest.a

libreportstest_a_SOURCES = reportstestcommon.cpp pivottabletest.cpp querytabletest.cpp
endif
