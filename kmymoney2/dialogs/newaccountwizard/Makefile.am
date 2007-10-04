noinst_LIBRARIES = libnewaccountwizard.a
KDE_OPTIONS = noautodist

INCLUDES = $(all_includes)  -I../wizardpages -I$(top_srcdir) -I. @LIBCURL_CPPFLAGS@ 

libnewaccountwizard_a_METASOURCES = AUTO

UI_FILES = 

libnewaccountwizard_a_SOURCES = knewaccountwizard.cpp kinstitutionpagedecl.ui kaccounttypepagedecl.ui kopeningpagedecl.ui kschedulepagedecl.ui

EXTRA_DIST = kinstitutionpagedecl.ui kaccounttypepagedecl.ui kopeningpagedecl.ui kschedulepagedecl.ui

DISTCLEANFILES= kinstitutionpagedecl.cpp kinstitutionpagedecl.h kaccounttypepagedecl.cpp kaccounttypepagedecl.h kopeningpagedecl.cpp kopeningpagedecl.h kschedulepagedecl.cpp kschedulepagedecl.h

noinst_HEADERS = knewaccountwizard.h knewaccountwizard_p.h

SUBDIRS = 

messages: rc.cpp
