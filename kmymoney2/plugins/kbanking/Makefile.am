INCLUDES = $(all_includes) -I..
METASOURCES = AUTO

kde_module_LTLIBRARIES = kmm_kbanking.la
kmm_kbanking_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) $(LIB_KDECORE) $(LIB_QT)
kmm_kbanking_la_SOURCES = mymoneybanking.cpp
kmm_kbanking_la_LIBADD = @kbanking_libs@

pluginsdir = $(kde_datadir)/kmm_kbanking
plugins_DATA = kmm_kbanking.rc

kde_services_DATA = kmm_kbanking.desktop

noinst_HEADERS = mymoneybanking.h

