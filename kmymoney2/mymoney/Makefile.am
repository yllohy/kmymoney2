KDE_OPTIONS = noautodist

INCLUDES = $(all_includes)
lib_LTLIBRARIES = libkmm_mymoney.la

libkmm_mymoney_la_METASOURCES = AUTO

libkmm_mymoney_la_SOURCES = mymoneyfinancialcalculator.cpp mymoneytransactionfilter.cpp mymoneykeyvaluecontainer.cpp mymoneyobserver.cpp mymoneysubject.cpp mymoneysplit.cpp mymoneyinstitution.cpp mymoneyexception.cpp mymoneyinvesttransaction.cpp mymoneyutils.cpp mymoneysecurity.cpp mymoneytransaction.cpp mymoneyscheduled.cpp mymoneypayee.cpp mymoneymoney.cpp mymoneyfile.cpp mymoneycategory.cpp mymoneyaccount.cpp mymoneyreport.cpp mymoneystatement.cpp mymoneyprice.cpp
libkmm_mymoney_la_LDFLAGS = $(all_libraries) -version-info 0:0:0

SUBDIRS = storage 

instdir=$(includedir)/kmymoney
inst_HEADERS = mymoneyaccount.h mymoneycategory.h mymoneycurrency.h mymoneyexception.h mymoneyfile.h mymoneyfinancialcalculator.h mymoneyinstitution.h mymoneyinvesttransaction.h mymoneykeyvaluecontainer.h mymoneymoney.h mymoneyobserver.h mymoneypayee.h mymoneyprice.h mymoneyreport.h mymoneyscheduled.h mymoneysecurity.h mymoneysplit.h mymoneystatement.h mymoneysubject.h mymoneytransactionfilter.h mymoneytransaction.h mymoneyutils.h

noinst_HEADERS = autotest.h mymoneyaccounttest.h mymoneyfinancialcalculatortest.h mymoneykeyvaluecontainertest.h mymoneyexceptiontest.h mymoneyfiletest.h mymoneyinstitutiontest.h mymoneymoneytest.h mymoneyobservertest.h mymoneyscheduletest.h mymoneysplittest.h mymoneysecuritytest.h mymoneytransactiontest.h

if CPPUNIT
check_LIBRARIES = libmymoneytest.a

libmymoneytest_a_SOURCES = mymoneytransactiontest.cpp mymoneysplittest.cpp mymoneymoneytest.cpp mymoneyfiletest.cpp mymoneyaccounttest.cpp mymoneyexceptiontest.cpp mymoneyinstitutiontest.cpp mymoneykeyvaluecontainertest.cpp mymoneyscheduletest.cpp mymoneyfinancialcalculatortest.cpp mymoneysecuritytest.cpp
endif

