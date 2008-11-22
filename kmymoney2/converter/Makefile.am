KDE_OPTIONS = noautodist

INCLUDES = $(all_includes) -I$(top_srcdir) -I. -I$(top_srcdir)/kmymoney2 -I$(top_builddir)/kmymoney2 @LIBXMLPP_CFLAGS@ @LIBCURL_CPPFLAGS@

instdir=$(includedir)/kmymoney

noinst_LIBRARIES = libconverter.a
libconverter_a_METASOURCES = AUTO

libconverter_a_SOURCES = mymoneyqifreader.cpp mymoneyqifwriter.cpp mymoneyqifprofile.cpp mymoneytemplate.cpp mymoneystatementreader.cpp webpricequote.cpp mymoneygncreader.cpp

EXTRA_DIST = 

inst_HEADERS = mymoneytemplate.h

noinst_HEADERS = imymoneyreader.h mymoneyqifprofile.h mymoneyqifreader.h mymoneyqifwriter.h mymoneystatementreader.h webpricequote.h mymoneygncreader.h convertertest.h

if CPPUNIT
check_LIBRARIES = libconvertertest.a

libconvertertest_a_SOURCES = convertertest.cpp
endif


