KDE_OPTIONS = noautodist

INCLUDES = $(all_includes) -I$(top_srcdir) -I.

noinst_LIBRARIES = libconverter.a
libconverter_a_METASOURCES = AUTO

libconverter_a_SOURCES = mymoneyqifreader.cpp mymoneyqifwriter.cpp mymoneyqifprofile.cpp mymoneytemplate.cpp mymoneystatementreader.cpp mymoneyofxstatement.cpp webpricequote.cpp mymoneygncreader.cpp

EXTRA_DIST = 

noinst_HEADERS = convertertest.h imymoneyreader.h mymoneyofxstatement.h mymoneyqifprofile.h mymoneyqifreader.h mymoneyqifwriter.h mymoneystatementreader.h mymoneytemplate.h webpricequote.h mymoneygncreader.cpp

if CPPUNIT
check_LIBRARIES = libconvertertest.a

libconvertertest_a_SOURCES = convertertest.cpp
endif
