####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libstorage.a

INCLUDES = $(all_includes)

libstorage_a_METASOURCES = AUTO

libstorage_a_SOURCES = imymoneystorageformat.cpp mymoneystoragexml.cpp mymoneystoragedump.cpp mymoneystoragebin.cpp mymoneyseqaccessmgr.cpp imymoneystorage.cpp imymoneyserialize.cpp 


EXTRA_DIST = imymoneyserialize.cpp imymoneyserialize.h imymoneystorage.cpp imymoneystorage.h mymoneyseqaccessmgr.cpp mymoneyseqaccessmgr.h mymoneystoragebin.cpp mymoneystoragebin.h mymoneystoragedump.cpp mymoneystoragedump.h mymoneystoragexml.h mymoneystoragexml.cpp imymoneystorageformat.cpp imymoneystorageformat.h mymoneyxmlparser.h 

####### kdevelop will overwrite this part!!! (end)############

if CPPUNIT
EXTRA_libstorage_a_SOURCES = mymoneyseqaccessmgrtest.cpp mymoneystoragebintest.cpp
libstorage_a_LIBADD = mymoneyseqaccessmgrtest.o mymoneystoragebintest.o
endif
