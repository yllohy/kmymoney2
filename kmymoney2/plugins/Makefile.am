METASOURCES = AUTO
INCLUDES = -I$(top_srcdir)/src $(all_includes)

SUBDIRS = interfaces @ofx_importerplugin@ @kbanking_plugin@

# The library containing the plugin base class
lib_LTLIBRARIES = libkmm_plugin.la
libkmm_plugin_la_SOURCES = kmymoneyplugin.cpp viewinterface.cpp statementinterface.cpp
libkmm_plugin_la_LDFLAGS = $(all_libraries) -version-info 0:0:0

#definition of the service type
kde_servicetypes_DATA = kmymoneyplugin.desktop

# make sure this lib is build before any subdirectory
BUILT_SOURCES = libkmm_plugin.la

pluginsincludedir = $(includedir)/kmymoney
pluginsinclude_HEADERS = kmymoneyplugin.h viewinterface.h statementinterface.h
