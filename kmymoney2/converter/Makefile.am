####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libconverter.a

INCLUDES = $(all_includes)

libconverter_a_METASOURCES = AUTO

libconverter_a_SOURCES = mymoneyqifreader.cpp mymoneyqifwriter.cpp mymoneyqifprofile.cpp mymoneytemplate.cpp mymoneystatementreader.cpp mymoneyofxstatement.cpp webpricequote.cpp

EXTRA_DIST = mymoneyqifprofile.h mymoneyqifprofile.cpp mymoneyqifwriter.cpp mymoneyqifwriter.h mymoneyqifreader.cpp mymoneyqifreader.h mymoneytemplate.cpp mymoneytemplate.h webpricequote.h

####### kdevelop will overwrite this part!!! (end)############

noinst_HEADERS = mymoneyofxstatement.h webpricequote.h

if CPPUNIT
check_LIBRARIES = libconvertertest.a

libconvertertest_a_SOURCES = convertertest.cpp
endif
