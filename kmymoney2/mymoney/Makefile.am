####### kdevelop will overwrite this part!!! (begin)##########

INCLUDES = $(all_includes)

libmymoney_a_METASOURCES = AUTO

noinst_LIBRARIES = libmymoney.a

libmymoney_a_SOURCES = mymoneyinvesttransaction.cpp mymoneyequitylist.cpp mymoneyutils.cpp mymoneyequity.cpp mymoneytransactionbase.cpp mymoneysplittransaction.cpp mymoneytransaction.cpp mymoneyscheduled.cpp mymoneypayee.cpp mymoneymoney.cpp mymoneyfile.cpp mymoneycategory.cpp mymoneybank.cpp mymoneyaccount.cpp 


EXTRA_DIST = mymoneyaccount.cpp mymoneyaccount.h mymoneybank.cpp mymoneybank.h mymoneycategory.cpp mymoneycategory.h mymoneyfile.cpp mymoneyfile.h mymoneymoney.cpp mymoneymoney.h mymoneypayee.cpp mymoneypayee.h mymoneyscheduled.cpp mymoneyscheduled.h mymoneytransaction.cpp mymoneytransaction.h readme-mymoney.txt mymoney_config.h mymoneysplittransaction.cpp mymoneysplittransaction.h mymoneytransactionbase.cpp mymoneytransactionbase.h mymoneyequity.cpp mymoneyequity.h mymoneyutils.h mymoneyutils.cpp mymoneyequitylist.cpp mymoneyequitylist.h mymoneyinvesttransaction.cpp mymoneyinvesttransaction.h 

####### kdevelop will overwrite this part!!! (end)############

CXXFLAGS += -fexceptions

TESTS = autotest
check_PROGRAMS = $(TESTS)

# INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include

autotest_SOURCES= mymoneymoney.cpp \
                   autotest.cpp \
                   mymoneymoney.h \
                   mymoneymoneytest.h

autotest_LDADD = $(LIB_KDECORE) $(LIB_QT) $(LIB_CPPUNIT)
autotest_LDFLAGS = $(all_libraries) $(kde_RPATH)


