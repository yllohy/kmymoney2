####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libmymoney.a

INCLUDES = $(all_includes)

libmymoney_a_METASOURCES = AUTO

libmymoney_a_SOURCES = mymoneysplit.cpp mymoneyinstitution.cpp mymoneyexception.cpp mymoneyinvesttransaction.cpp mymoneyequitylist.cpp mymoneyutils.cpp mymoneyequity.cpp mymoneytransaction.cpp mymoneyscheduled.cpp mymoneypayee.cpp mymoneymoney.cpp mymoneyfile.cpp mymoneycategory.cpp mymoneyaccount.cpp 

SUBDIRS = storage 

EXTRA_DIST = mymoneyaccount.cpp mymoneyaccount.h mymoneycategory.cpp mymoneycategory.h mymoneyfile.cpp mymoneyfile.h mymoneymoney.cpp mymoneymoney.h mymoneypayee.cpp mymoneypayee.h mymoneyscheduled.cpp mymoneyscheduled.h mymoneytransaction.cpp mymoneytransaction.h readme-mymoney.txt mymoney_config.h mymoneyequity.cpp mymoneyequity.h mymoneyutils.h mymoneyutils.cpp mymoneyequitylist.cpp mymoneyequitylist.h mymoneyinvesttransaction.cpp mymoneyinvesttransaction.h mymoneyexception.cpp mymoneyexception.h mymoneyinstitution.cpp mymoneyinstitution.h mymoneysplit.cpp mymoneysplit.h 

####### kdevelop will overwrite this part!!! (end)############
TESTS = autotest
check_PROGRAMS = $(TESTS)

INCLUDES += -Istorage

autotest_SOURCES= autotest.cpp \
                mymoneytransactiontest.h \
                mymoneysplittest.h \
                mymoneyaccounttest.h \
                mymoneyinstitutiontest.h \
                mymoneyfiletest.h \
                mymoneymoneytest.h \
                mymoneyexceptiontest.h \
                storage/mymoneytseqaccessmgrtest.h \
                storage/mymoneystoragebintest.h

autotest_DEPENDENCIES = all-recursive

autotest_LDADD = $(LIB_KDECORE) $(LIB_QT) $(LIB_CPPUNIT) storage/libstorage.a libmymoney.a
autotest_LDFLAGS = $(all_libraries) $(kde_RPATH)

