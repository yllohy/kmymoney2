INCLUDES = $(all_includes) -I..
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kmm_ofximport.la

# Srcs for the plugin
kmm_ofximport_la_SOURCES = ofximporterplugin.cpp

# Libs needed by the plugin
kmm_ofximport_la_LIBADD = @ofx_libs@ ../libkmm_plugin.la ../libkmm_plugin.la $(top_srcdir)/kmymoney2/mymoney/libkmm_mymoney.la

# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
kmm_ofximport_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) $(LIB_KDECORE) $(LIB_QT)

# Install the desktop file needed to detect the plugin
kde_services_DATA = kmm_ofximport.desktop

noinst_HEADERS = ofximporterplugin.h

