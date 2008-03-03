noinst_LIBRARIES = libwizardpages.a
KDE_OPTIONS = noautodist

INCLUDES = $(all_includes)  -I$(top_srcdir) -I. @LIBCURL_CPPFLAGS@ 

libwizardpages_a_METASOURCES = AUTO

libwizardpages_a_SOURCES = userinfodecl.ui userinfo.cpp currencydecl.ui currency.cpp accountsdecl.ui accounts.cpp

EXTRA_DIST = userinfodecl.ui currencydecl.ui accountsdecl.ui

DISTCLEANFILES= userinfodecl.cpp userinfodecl.h currencydecl.cpp currencydecl.h accountsdecl.cpp accountsdecl.h

noinst_HEADERS = userinfo.h currency.h accounts.h

SUBDIRS = 

messages: rc.cpp
