KDE_OPTIONS = noautodist

INCLUDES = $(all_includes)

noinst_LIBRARIES = libstorage.a
libstorage_a_METASOURCES = AUTO

libstorage_a_SOURCES = mymoneystoragegnc.cpp imymoneystorageformat.cpp mymoneystoragexml.cpp mymoneystoragedump.cpp mymoneystoragebin.cpp mymoneyseqaccessmgr.cpp imymoneystorage.cpp imymoneyserialize.cpp mymoneystorageanon.cpp

instdir=$(includedir)/kmymoney
inst_HEADERS = imymoneystorage.h imymoneyserialize.h imymoneystorageformat.h

noinst_HEADERS = mymoneyseqaccessmgr.h mymoneystorageanon.h mymoneystoragebin.h mymoneystoragedump.h mymoneystoragegnc.h mymoneystoragexml.h mymoneyseqaccessmgrtest.h mymoneystoragebintest.h

if CPPUNIT
check_LIBRARIES = libstoragetest.a

libstoragetest_a_SOURCES = mymoneyseqaccessmgrtest.cpp mymoneystoragebintest.cpp
endif
