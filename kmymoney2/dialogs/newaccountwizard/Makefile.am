noinst_LIBRARIES = libnewaccountwizard.a
KDE_OPTIONS = noautodist

INCLUDES = $(all_includes)  -I../wizardpages -I.. -I$(top_srcdir) -I. @LIBCURL_CPPFLAGS@ 

libnewaccountwizard_a_METASOURCES = AUTO

UI_FILES = 

libnewaccountwizard_a_SOURCES = knewaccountwizard.cpp kinstitutionpagedecl.ui kaccounttypepagedecl.ui kbrokeragepagedecl.ui kschedulepagedecl.ui kgeneralloaninfopagedecl.ui kloandetailspagedecl.ui kloanpaymentpagedecl.ui kloanschedulepagedecl.ui kloanpayoutpagedecl.ui kaccountsummarypagedecl.ui

EXTRA_DIST = kinstitutionpagedecl.ui kaccounttypepagedecl.ui kbrokeragepagedecl.ui kschedulepagedecl.ui kgeneralloaninfopagedecl.ui kloandetailspagedecl.ui kloanpaymentpagedecl.ui kloanschedulepagedecl.ui kloanpayoutpagedecl.ui kaccountsummarypagedecl.ui

DISTCLEANFILES= kinstitutionpagedecl.cpp kinstitutionpagedecl.h kaccounttypepagedecl.cpp kaccounttypepagedecl.h kbrokeragepagedecl.cpp kbrokeragepagedecl.h kschedulepagedecl.cpp kschedulepagedecl.h kgeneralloaninfopagedecl.cpp kgeneralloaninfopagedecl.h kloandetailspagedecl.cpp kloandetailspagedecl.h kloanpaymentpagedecl.cpp kloanpaymentpagedecl.h kloanschedulepagedecl.cpp kloanschedulepagedecl.h kloanpayoutpagedecl.cpp kloanpayoutpagedecl.h kaccountsummarypagedecl.cpp kaccountsummarypagedecl.h

noinst_HEADERS = knewaccountwizard.h knewaccountwizard_p.h

BUILT_SOURCES = dialogs

.PHONY: dialogs
dialogs:
	$(MAKE) -C .. ksplittransactiondlgdecl.h

SUBDIRS = 

messages: rc.cpp
