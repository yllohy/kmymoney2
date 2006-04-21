noinst_LIBRARIES = libnewuserwizard.a
KDE_OPTIONS = noautodist

INCLUDES = $(all_includes)  -I$(top_srcdir) -I. @LIBCURL_CPPFLAGS@ 

libnewuserwizard_a_METASOURCES = AUTO

UI_FILES = kgeneralpagedecl.ui

libnewuserwizard_a_SOURCES = knewuserwizard.cpp kgeneralpagedecl.ui kcurrencypagedecl.ui kpasswordpagedecl.ui

EXTRA_DIST = kgeneralpagedecl.ui kcurrencypagedecl.ui kpasswordpagedecl.ui

DISTCLEANFILES= kgeneralpagedecl.cpp kgeneralpagedecl.h kcurrencypagedecl.cpp kcurrencypagedecl.h kpasswordpagedecl.cpp kpasswordpagedecl.h

noinst_HEADERS = knewuserwizard.h knewuserwizard_p.h knewuserwizard_p.h

SUBDIRS = 

messages: rc.cpp
