INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(kmm_kbanking_PART_SRCS
   mymoneybanking.cpp
   )

KDE3_AUTOMOC(${kmm_kbanking_PART_SRCS})

KDE3_ADD_KPART(kmm_kbanking ${kmm_kbanking_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_kbanking  ${QT_AND_KDECORE_LIBS} kmm_plugin kmm_mymoney kdeui )

INSTALL(TARGETS kmm_kbanking 
	DESTINATION lib/kde3)


########### install files ###############

INSTALL(FILES kmm_kbanking.rc
	DESTINATION share/apps/kmm_kbanking 
)
INSTALL(FILES kmm_kbanking.desktop
	DESTINATION share/services 
)



#original Makefile.am contents follow:

#KDE_OPTIONS = noautodist
#
#INCLUDES = $(all_includes) -I..
#METASOURCES = AUTO
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kmm_kbanking.la
#
## Srcs for the plugin
#kmm_kbanking_la_SOURCES = mymoneybanking.cpp
#
## Libs needed by the plugin
#kmm_kbanking_la_LIBADD = @kbanking_libs@ ../libkmm_plugin.la $(top_builddir)/kmymoney2/mymoney/libkmm_mymoney.la
#
## LD flags for the plugin
## -module says: this is a module, i.e. something you're going to dlopen
## so e.g. it has no version number like a normal shared lib would have.
#kmm_kbanking_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT)
#
## rc file containing the GUI for the plugin
#pluginsdir = $(kde_datadir)/kmm_kbanking
#plugins_DATA = kmm_kbanking.rc
#
## Install the desktop file needed to detect the plugin
#kde_services_DATA = kmm_kbanking.desktop
#
#noinst_HEADERS = mymoneybanking.h
#
#EXTRA_DIST = kmm_kbanking.desktop kmm_kbanking.rc
