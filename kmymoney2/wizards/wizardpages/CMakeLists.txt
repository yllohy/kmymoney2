PROJECT(wizardpages)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )

########### next target ###############
SET(libwizardpages_a_SOURCES
 userinfo.cpp  currency.cpp  accounts.cpp
)

SET (libwizardpages_a_UI
     userinfodecl.ui currencydecl.ui accountsdecl.ui
)

KDE3_AUTOMOC(${libwizardpages_a_SOURCES})

KMM_ADD_UI_FILES(libwizardpages_a_SOURCES ${libwizardpages_a_UI} )


ADD_LIBRARY(wizardpages STATIC ${libwizardpages_a_SOURCES})
#ADD_DEPENDENCIES(wizardpages dialogs)

########### install files ###############

