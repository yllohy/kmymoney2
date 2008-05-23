KDE_OPTIONS = noautodist

INCLUDES = $(all_includes) -I$(top_srcdir) -I. -I$(top_builddir)/kmymoney2/dialogs

noinst_LIBRARIES = libstorage.a
libstorage_a_METASOURCES = AUTO

libstorage_a_SOURCES = imymoneystorageformat.cpp mymoneystoragexml.cpp mymoneystoragedump.cpp mymoneyseqaccessmgr.cpp mymoneydatabasemgr.cpp  imymoneystorage.cpp imymoneyserialize.cpp mymoneystorageanon.cpp mymoneystoragesql.cpp

instdir=$(includedir)/kmymoney
inst_HEADERS = imymoneystorage.h imymoneyserialize.h imymoneystorageformat.h

noinst_HEADERS = mymoneyseqaccessmgr.h mymoneydatabasemgr.h mymoneystorageanon.h mymoneystoragedump.h mymoneystoragexml.h mymoneyseqaccessmgrtest.h mymoneystoragesql.h mymoneystoragebin.h mymoneymap.h mymoneymaptest.h

if CPPUNIT
check_LIBRARIES = libstoragetest.a

libstoragetest_a_SOURCES = mymoneyseqaccessmgrtest.cpp mymoneymaptest.cpp
endif
