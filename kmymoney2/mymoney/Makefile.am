####### kdevelop will overwrite this part!!! (begin)##########

INCLUDES = $(all_includes)
lib_LTLIBRARIES = libkmm_mymoney.la

libkmm_mymoney_la_METASOURCES = AUTO

libkmm_mymoney_la_SOURCES = mymoneyfinancialcalculator.cpp mymoneytransactionfilter.cpp mymoneykeyvaluecontainer.cpp mymoneyobserver.cpp mymoneysubject.cpp mymoneysplit.cpp mymoneyinstitution.cpp mymoneyexception.cpp mymoneyinvesttransaction.cpp mymoneyutils.cpp mymoneysecurity.cpp mymoneytransaction.cpp mymoneyscheduled.cpp mymoneypayee.cpp mymoneymoney.cpp mymoneyfile.cpp mymoneycategory.cpp mymoneyaccount.cpp mymoneyonlinepriceupdate.cpp mymoneyreport.cpp mymoneystatement.cpp mymoneyprice.cpp
libkmm_mymoney_la_LDFLAGS = $(all_libraries) -version-info 0:0:0

SUBDIRS = storage 

EXTRA_DIST = mymoneyaccount.cpp mymoneyaccount.h mymoneycategory.cpp mymoneycategory.h mymoneyfile.cpp mymoneyfile.h mymoneymoney.cpp mymoneymoney.h mymoneypayee.cpp mymoneypayee.h mymoneyscheduled.cpp mymoneyscheduled.h mymoneytransaction.cpp mymoneytransaction.h mymoneysecurity.cpp mymoneysecurity.h mymoneyutils.h mymoneyutils.cpp mymoneyinvesttransaction.cpp mymoneyinvesttransaction.h mymoneyexception.cpp mymoneyexception.h mymoneyinstitution.cpp mymoneyinstitution.h mymoneysplit.cpp mymoneysplit.h mymoneysubject.cpp mymoneysubject.h mymoneyobserver.cpp mymoneyobserver.h mymoneykeyvaluecontainer.cpp mymoneykeyvaluecontainer.h mymoneytransactionfilter.cpp mymoneytransactionfilter.h mymoneyfinancialcalculator.cpp mymoneyfinancialcalculator.h mymoneycurrency.cpp mymoneycurrency.h mymoneyprice.cpp mymoneyprice.h

####### kdevelop will overwrite this part!!! (end)############

if CPPUNIT
check_LIBRARIES = libmymoneytest.a

libmymoneytest_a_SOURCES = mymoneytransactiontest.cpp mymoneysplittest.cpp mymoneymoneytest.cpp mymoneyfiletest.cpp mymoneyaccounttest.cpp mymoneyexceptiontest.cpp mymoneyinstitutiontest.cpp mymoneykeyvaluecontainertest.cpp mymoneyscheduletest.cpp mymoneyfinancialcalculatortest.cpp mymoneysecuritytest.cpp
endif

