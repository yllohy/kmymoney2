METASOURCES = AUTO
INCLUDES = -I$(top_srcdir)/src $(all_includes)
SUBDIRS = interfaces @kbanking_plugin@

noinst_LIBRARIES = libplugins.a

libplugins_a_SOURCES = kmymoneyplugin.cpp viewinterface.cpp statementinterface.cpp

noinst_HEADERS = kmymoneyplugin.h viewinterface.h statementinterface.h

#definition of the service type
kde_servicetypes_DATA = kmymoneyplugin.desktop

