ADD_SUBDIRECTORY( interfaces ) 

if(USE_OFX_DIRECTCONNECT)
  ADD_SUBDIRECTORY( ofximport )
endif(USE_OFX_DIRECTCONNECT)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(kmm_plugin_LIB_SRCS
   kmymoneyplugin.cpp
   viewinterface.cpp
   statementinterface.cpp
   importinterface.cpp
   )

KDE3_AUTOMOC(${kmm_plugin_LIB_SRCS})

SET(plugins_HEADERS
    kmymoneyplugin.h
    viewinterface.h
    statementinterface.h
    importinterface.h
    )

# here we call the module wich creates us the given links
KMM_CREATE_LINKS( ${plugins_HEADERS})

ADD_LIBRARY(kmm_plugin SHARED ${kmm_plugin_LIB_SRCS})

TARGET_LINK_LIBRARIES(kmm_plugin  ${QT_AND_KDECORE_LIBS} )

SET_TARGET_PROPERTIES(kmm_plugin PROPERTIES VERSION 4.2.0 SOVERSION 4 )

INSTALL(TARGETS kmm_plugin 
	DESTINATION lib
)

########### install files ###############

INSTALL(FILES  ${plugins_HEADERS}
	DESTINATION include/kmymoney
)
INSTALL(FILES  kmymoneyplugin.desktop kmymoneyimporterplugin.desktop
        DESTINATION share/servicetypes
)


