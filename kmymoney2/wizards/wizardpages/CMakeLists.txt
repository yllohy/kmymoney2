PROJECT(Wizardpages)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############
SET(libwizardpages_a_SOURCES
  accounts.cpp currency.cpp userinfo.cpp
  )

SET (libwizardpages_a_UI
  accountsdecl.ui currencydecl.ui userinfodecl.ui
  )

KDE3_AUTOMOC(${libwizardpages_a_SOURCES})

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
KMM_ADD_UI_FILES(libwizardpages_a_SOURCES ${libwizardpages_a_UI}
  DEPENDS kmymoney)

ADD_LIBRARY(wizardpages STATIC ${libwizardpages_a_SOURCES})
