ADD_SUBDIRECTORY( interfaces ) 

if(USE_OFX_DIRECTCONNECT)
  ADD_SUBDIRECTORY( ofximport )
endif(USE_OFX_DIRECTCONNECT)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

SET(kmm_plugin_LIB_SRCS
  importinterface.cpp kmymoneyplugin.cpp pluginloader.cpp
  statementinterface.cpp viewinterface.cpp)

KDE3_AUTOMOC(${kmm_plugin_LIB_SRCS})

SET(plugins_HEADERS
  importinterface.h kmymoneyplugin.h pluginloader.h
  statementinterface.h viewinterface.h)

# here we call the module wich creates us the given links
KMM_CREATE_LINKS(${plugins_HEADERS})

ADD_LIBRARY(kmm_plugin SHARED ${kmm_plugin_LIB_SRCS})
TARGET_LINK_LIBRARIES(kmm_plugin ${QT_AND_KDECORE_LIBS} kdeui kio kutils)

#SET_TARGET_PROPERTIES(kmm_plugin PROPERTIES VERSION 4.2.0 SOVERSION 4 )
SET_TARGET_PROPERTIES(kmm_plugin PROPERTIES VERSION 0.0.0 SOVERSION 0 )

########### install files ###############

INSTALL(TARGETS kmm_plugin 
	DESTINATION lib)

INSTALL(FILES ${plugins_HEADERS}
	DESTINATION include/kmymoney)

INSTALL(FILES kmymoneyplugin.desktop kmymoneyimporterplugin.desktop
  DESTINATION share/servicetypes)
