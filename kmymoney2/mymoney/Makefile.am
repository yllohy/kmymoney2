KDE_OPTIONS = noautodist

#
# The LIBVERSION setting controls the libtool versioning system for shared
# libraries. It is constructed out of the triplet CURRENT:REVISION:AGE.
#
# Here are the rules on howto update the version info:
#
#  1. Update the version information only immediately before a public
#     release of your software.  More frequent updates are unnecessary,
#     and only guarantee that the current interface number gets larger
#     faster.
#
#  2. If the library source code has changed at all since the last
#     update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
#
#  3. If any interfaces have been added, removed, or changed since the
#     last update, increment CURRENT, and set REVISION to 0.
#
#  4. If any interfaces have been added since the last public release,
#     then increment AGE.
#
#  5. If any interfaces have been removed since the last public release,
#     then set AGE to 0.
#
# The above information is copied from 'info libtool'.

LIBVERSION=1:1:1

INCLUDES = $(all_includes)  -I$(top_srcdir) -I.
lib_LTLIBRARIES = libkmm_mymoney.la

libkmm_mymoney_la_METASOURCES = AUTO

libkmm_mymoney_la_SOURCES = mymoneymoney.cpp mymoneyfinancialcalculator.cpp mymoneytransactionfilter.cpp mymoneykeyvaluecontainer.cpp mymoneyobserver.cpp mymoneysubject.cpp mymoneysplit.cpp mymoneyinstitution.cpp mymoneyexception.cpp mymoneyinvesttransaction.cpp mymoneyutils.cpp mymoneysecurity.cpp mymoneytransaction.cpp mymoneyscheduled.cpp mymoneypayee.cpp mymoneyfile.cpp mymoneycategory.cpp mymoneyaccount.cpp mymoneyreport.cpp mymoneystatement.cpp mymoneyprice.cpp
libkmm_mymoney_la_LDFLAGS = $(all_libraries) -version-info $(LIBVERSION)

SUBDIRS = storage 

instdir=$(includedir)/kmymoney
inst_HEADERS = mymoneyaccount.h mymoneycategory.h mymoneyexception.h mymoneyfile.h mymoneyfinancialcalculator.h mymoneyinstitution.h mymoneyinvesttransaction.h mymoneykeyvaluecontainer.h mymoneymoney.h mymoneyobserver.h mymoneypayee.h mymoneyprice.h mymoneyreport.h mymoneyscheduled.h mymoneysecurity.h mymoneysplit.h mymoneystatement.h mymoneysubject.h mymoneytransactionfilter.h mymoneytransaction.h mymoneyutils.h

noinst_HEADERS = autotest.h mymoneyaccounttest.h mymoneyfinancialcalculatortest.h mymoneykeyvaluecontainertest.h mymoneyexceptiontest.h mymoneyfiletest.h mymoneyinstitutiontest.h mymoneymoneytest.h mymoneyobservertest.h mymoneyscheduletest.h mymoneysplittest.h mymoneysecuritytest.h mymoneytransactiontest.h

if CPPUNIT
check_LIBRARIES = libmymoneytest.a

libmymoneytest_a_SOURCES = mymoneytransactiontest.cpp mymoneysplittest.cpp mymoneymoneytest.cpp mymoneyfiletest.cpp mymoneyaccounttest.cpp mymoneyexceptiontest.cpp mymoneyinstitutiontest.cpp mymoneykeyvaluecontainertest.cpp mymoneyscheduletest.cpp mymoneyfinancialcalculatortest.cpp mymoneysecuritytest.cpp
endif

