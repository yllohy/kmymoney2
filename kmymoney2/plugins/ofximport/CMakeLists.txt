INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(kmm_ofximport_PART_SRCS
   ofximporterplugin.cpp
   )

KDE3_AUTOMOC(${kmm_ofximport_PART_SRCS})

KDE3_ADD_KPART(kmm_ofximport ${kmm_ofximport_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_ofximport  ${QT_AND_KDECORE_LIBS} kmm_plugin kmm_plugin kmm_mymoney )

INSTALL(TARGETS kmm_ofximport 
	DESTINATION lib/kde3
)


########### install files ###############

INSTALL_FILES( /share/services FILES  kmm_ofximport.desktop )



#original Makefile.am contents follow:

#KDE_OPTIONS = noautodist
#
#INCLUDES = $(all_includes) -I..
#METASOURCES = AUTO
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kmm_ofximport.la
#
## Srcs for the plugin
#kmm_ofximport_la_SOURCES = ofximporterplugin.cpp
#
## Libs needed by the plugin
#kmm_ofximport_la_LIBADD = @OFX_LIBS@ ../libkmm_plugin.la ../libkmm_plugin.la $(top_builddir)/kmymoney2/mymoney/libkmm_mymoney.la
#
## LD flags for the plugin
## -module says: this is a module, i.e. something you're going to dlopen
## so e.g. it has no version number like a normal shared lib would have.
#kmm_ofximport_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) $(LIB_KDECORE) $(LIB_QT)
#
## Install the desktop file needed to detect the plugin
#kde_services_DATA = kmm_ofximport.desktop
#
#noinst_HEADERS = ofximporterplugin.h
#
#EXTRA_DIST = kmm_ofximport.desktop
