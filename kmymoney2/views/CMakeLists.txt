INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

############# next target (views) STATIC ###################
SET(libviews_a_SOURCES
  kaccountsview.cpp
  kbudgetview.cpp
  kcategoriesview.cpp
  kforecastview.cpp
  kgloballedgerview.cpp
  khomeview.cpp
  kinstitutionsview.cpp
  kinvestmentlistitem.cpp
  kinvestmentview.cpp
  kmymoneyfile.cpp
  kmymoneytransaction.cpp
  kmymoneyview.cpp
  kpayeesview.cpp
  kreportsview.cpp
  kscheduledlistitem.cpp
  kscheduledview.cpp
  )

SET(libviews_a_HEADER kmymoneyview.h)

SET(libviews_a_UI
  kaccountsviewdecl.ui
  kbudgetviewdecl.ui
  kcategoriesviewdecl.ui
  kforecastviewdecl.ui
  kinstitutionsviewdecl.ui
  kinvestmentviewdecl.ui
  kpayeesviewdecl.ui
  kscheduledviewdecl.ui
  )

KMM_CREATE_LINKS( ${libviews_a_HEADER} )

KDE3_AUTOMOC(${libviews_a_SOURCES})

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
KMM_ADD_UI_FILES(libviews_a_SOURCES ${libviews_a_UI} DEPENDS kmymoney)

ADD_LIBRARY(views STATIC ${libviews_a_SOURCES})

# Must build knewbudgetdlgdecl.h from knewbudgetdlgdecl.ui
# before budgetview.cpp can be compiled.
ADD_DEPENDENCIES(views dialogs)

########### install files ###############

INSTALL(FILES ${libviews_a_HEADER}
  DESTINATION include/kmymoney)
