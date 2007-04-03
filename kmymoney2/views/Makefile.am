KDE_OPTIONS = noautodist

INCLUDES = -I$(top_srcdir) $(all_includes) 

noinst_LIBRARIES = libviews.a
libviews_a_METASOURCES = AUTO

libviews_a_SOURCES = kreportsview.cpp kmymoneytransaction.cpp kgloballedgerview.cpp kmymoneyfile.cpp kinvestmentlistitem.cpp kinvestmentviewdecl.ui kinvestmentview.cpp kscheduledlistitem.cpp kscheduledviewdecl.ui kscheduledview.cpp kpayeesview.cpp kpayeesviewdecl.ui khomeview.cpp kcategoriesviewdecl.ui kcategoriesview.cpp kmymoneyview.cpp kaccountsviewdecl.ui kaccountsview.cpp kinstitutionsviewdecl.ui kinstitutionsview.cpp kbudgetview.cpp kbudgetviewdecl.ui

# oldviews
# kledgerview.cpp kledgerviewinvestments.cpp kledgerviewliability.cpp kledgerviewloan.cpp kledgerviewcash.cpp kledgerviewasset.cpp kledgerviewcreditcard.cpp kledgerviewsavings.cpp kledgerviewcheckings.cpp

DISTCLEANFILES= kaccountsviewdecl.cpp kcategoriesviewdecl.cpp kinvestmentviewdecl.cpp kpayeesviewdecl.cpp kscheduledviewdecl.cpp kaccountsviewdecl.h kcategoriesviewdecl.h kinvestmentviewdecl.h kpayeesviewdecl.h kscheduledviewdecl.h kinstitutionsviewdecl.cpp kinstitutionsviewdecl.h kbudgetviewdecl.cpp kbudgetviewdecl.h

EXTRA_DIST = kaccountsviewdecl.ui kcategoriesviewdecl.ui kinvestmentviewdecl.ui kpayeesviewdecl.ui kscheduledviewdecl.ui kinstitutionsviewdecl.ui kbudgetviewdecl.ui

instdir=$(includedir)/kmymoney
inst_HEADERS = kmymoneyview.h

noinst_HEADERS = kaccountsview.h kcategoriesview.h kgloballedgerview.h khomeview.h kinvestmentlistitem.h kinvestmentview.h kmymoneyfile.h kmymoneytransaction.h kpayeesview.h kreportsview.h kscheduledlistitem.h kscheduledview.h kinstitutionsview.h kbudgetview.h

# old noinst_HEADERS
# kledgerviewasset.h kledgerviewcash.h kledgerviewcheckings.h kledgerviewcreditcard.h kledgerviewliability.h kledgerviewloan.h kledgerviewsavings.h kledgerview.h kledgerviewinvestments.h

messages: rc.cpp
