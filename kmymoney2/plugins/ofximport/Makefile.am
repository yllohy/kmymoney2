KDE_OPTIONS = noautodist

INCLUDES = $(all_includes) -I.. -I. -I${srcdir}/dialogs -Idialogs @LIBXMLPP_CFLAGS@ @LIBCURL_CPPFLAGS@
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kmm_ofximport.la

# Srcs for the plugin
kmm_ofximport_la_SOURCES = ofximporterplugin.cpp ofxpartner.cpp nodeparser.cpp

# Libs needed by the plugin
kmm_ofximport_la_LIBADD = @OFX_LIBS@ dialogs/libdialogs.la ../libkmm_plugin.la ../libkmm_plugin.la $(top_builddir)/kmymoney2/mymoney/libkmm_mymoney.la

# LD flags for the plugin
# -module says: this is a module, i.e. something you're going to dlopen
# so e.g. it has no version number like a normal shared lib would have.
kmm_ofximport_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) $(LIB_KHTML) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_QT) -L../.libs

# rc file containing the GUI for the plugin
pluginsdir = $(kde_datadir)/kmm_ofximport
plugins_DATA = kmm_ofximport.rc

# Install the desktop file needed to detect the plugin
kde_services_DATA = kmm_ofximport.desktop

noinst_HEADERS = ofximporterplugin.h nodeparser.h ofxpartner.h

EXTRA_DIST = kmm_ofximport.desktop kmm_ofximport.rc

SUBDIRS = dialogs
