noinst_LTLIBRARIES = libdialogs.la
KDE_OPTIONS = noautodist

INCLUDES = $(all_includes)  -I$(top_srcdir) -I. @LIBCURL_CPPFLAGS@ @LIBXMLPP_CFLAGS@

libdialogs_la_METASOURCES = AUTO

libdialogs_la_SOURCES = konlinebankingsetupdecl.ui konlinebankingstatusdecl.ui konlinebankingsetupwizard.cpp konlinebankingstatus.cpp kofxdirectconnectdlgdecl.ui kofxdirectconnectdlg.cpp mymoneyofxconnector.cpp

EXTRA_DIST = konlinebankingsetupdecl.ui konlinebankingstatusdecl.ui kofxdirectconnectdlgdecl.ui

DISTCLEANFILES= konlinebankingsetupdecl.h konlinebankingsetupdecl.cpp konlinebankingstatusdecl.h konlinebankingstatusdecl.cpp kofxdirectconnectdlgdecl.h kofxdirectconnectdlgdecl.cpp

noinst_HEADERS = konlinebankingsetupwizard.h konlinebankingstatus.h kofxdirectconnectdlg.h mymoneyofxconnector.h

messages: rc.cpp
