INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )

########### next target ###############

SET(kmm_ofximport_dialogs_la_SOURCES
  mymoneyofxconnector.cpp
  kofxdirectconnectdlg.cpp
  konlinebankingsetupwizard.cpp
  konlinebankingstatus.cpp
  )

SET(kmm_ofximport_dialogs_UI
  kofxdirectconnectdlgdecl.ui konlinebankingsetupdecl.ui
  konlinebankingstatusdecl.ui)

KDE3_ADD_UI_FILES(kmm_ofximport_dialogs_la_SOURCES
  ${kmm_ofximport_dialogs_UI})

KDE3_AUTOMOC(${kmm_ofximport_dialogs_la_SOURCES})

ADD_LIBRARY(kmm_ofximport_dialogs SHARED ${kmm_ofximport_dialogs_la_SOURCES})

########### install files ###############

INSTALL(TARGETS kmm_ofximport_dialogs
	DESTINATION lib/kde3)
