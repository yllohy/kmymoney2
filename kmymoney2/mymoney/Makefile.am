####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libmymoney.a

INCLUDES = $(all_includes)

libmymoney_a_METASOURCES = AUTO

libmymoney_a_SOURCES = mymoneytransactionfilter.cpp mymoneykeyvaluecontainer.cpp mymoneyobserver.cpp mymoneysubject.cpp mymoneysplit.cpp mymoneyinstitution.cpp mymoneyexception.cpp mymoneyinvesttransaction.cpp mymoneyequitylist.cpp mymoneyutils.cpp mymoneyequity.cpp mymoneytransaction.cpp mymoneyscheduled.cpp mymoneypayee.cpp mymoneymoney.cpp mymoneyfile.cpp mymoneycategory.cpp mymoneyaccount.cpp 

SUBDIRS = storage 

EXTRA_DIST = mymoneyaccount.cpp mymoneyaccount.h mymoneycategory.cpp mymoneycategory.h mymoneyfile.cpp mymoneyfile.h mymoneymoney.cpp mymoneymoney.h mymoneypayee.cpp mymoneypayee.h mymoneyscheduled.cpp mymoneyscheduled.h mymoneytransaction.cpp mymoneytransaction.h mymoneyequity.cpp mymoneyequity.h mymoneyutils.h mymoneyutils.cpp mymoneyequitylist.cpp mymoneyequitylist.h mymoneyinvesttransaction.cpp mymoneyinvesttransaction.h mymoneyexception.cpp mymoneyexception.h mymoneyinstitution.cpp mymoneyinstitution.h mymoneysplit.cpp mymoneysplit.h mymoneysubject.cpp mymoneysubject.h mymoneyobserver.cpp mymoneyobserver.h mymoneykeyvaluecontainer.cpp mymoneykeyvaluecontainer.h mymoneytransactionfilter.cpp mymoneytransactionfilter.h 

####### kdevelop will overwrite this part!!! (end)############

DISTCLEANFILES=autotest

TESTS = autotest
check_PROGRAMS = $(TESTS)

INCLUDES += -Istorage


if CPPUNIT
autotest_SOURCES = autotest.cpp \
		mymoneytransactiontest.cpp \
                mymoneysplittest.cpp \
                mymoneymoneytest.cpp \
                mymoneyfiletest.cpp \
                mymoneyaccounttest.cpp \
		mymoneyexceptiontest.cpp \
                mymoneyinstitutiontest.cpp \
		mymoneykeyvaluecontainertest.cpp \
		mymoneyscheduletest.cpp

autotest_DEPENDENCIES = all-recursive
autotest_LDADD = $(LIB_KDECORE) $(LIB_QT) $(CPPUNIT_LIBS) storage/libstorage.a libmymoney.a
else
autotest_SOURCES= autotest.cpp 
endif

autotest_LDFLAGS = $(all_libraries) $(kde_RPATH)

