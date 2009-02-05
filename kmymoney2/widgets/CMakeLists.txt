PROJECT(widgets)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

########### custom command #############

# create the makekdewidgets file out of the makewidgets.in
CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/makekdewidgets.in
  ${CMAKE_CURRENT_BINARY_DIR}/makekdewidgets @ONLY)

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kmmwidgets.cpp
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kmymoney.widgets 
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/makekdewidgets -g KMyMoney
  -n CustomWidgetPlugin -o kmmwidgets.cpp -i kmymoney
  ${CMAKE_CURRENT_SOURCE_DIR}/kmymoney.widgets 
  )

########### create links ###############
SET(kmymoney_STAT_HEADERS
  register.h registeritem.h transaction.h kmymoneydateinput.h kmymoneyedit.h
  kmymoneytitlelabel.h kmymoneyaccountselector.h kmymoneycategory.h
  kmymoneyaccounttree.h kmymoneycurrencyselector.h kguiutils.h kmymoneywizard.h
  kmymoneyaccounttreebudget.h kmymoneyaccountcombo.h selectedtransaction.h
  transactionform.h transactioneditorcontainer.h kmymoneylineedit.h
  kmymoneychecklistitem.h kmymoneylistviewitem.h kmymoneyselector.h
  kmymoneyaccountcompletion.h kmymoneycompletion.h kmymoneycombo.h
  kbudgetvalues.h kaccounttemplateselector.h stdtransactiondownloaded.h
  stdtransactionmatched.h kmymoneyaccounttreebase.h scheduledtransaction.h
  kmymoneyaccounttreeforecast.h 
  )

KMM_CREATE_LINKS( ${kmymoney_STAT_HEADERS} )

KMM_CREATE_LINKS_BIN( transactionsortoption.h )

########### next target (kmymoney.la) SHARED #################

SET (libkmymoney_la_SOURCES
  kmmwidgets.cpp kmymoneycompletion.cpp kmymoneyaccountcombo.cpp
  kmymoneyaccountcompletion.cpp kmymoneycurrencyselector.cpp
  kmymoneyaccountselector.cpp kmymoneydatetbl.cpp kmymoneycalculator.cpp
  kmymoneycategory.cpp kmymoneycombo.cpp kmymoneylineedit.cpp kmymoneyedit.cpp
  kmymoneydateinput.cpp kmymoneyaccounttree.cpp kmymoneytitlelabel.cpp
  kguiutils.cpp kmymoneyaccounttreebudget.cpp register.cpp registeritem.cpp
  transaction.cpp transactionform.cpp kmymoneychecklistitem.cpp
  kmymoneylistviewitem.cpp kmymoneyselector.cpp kbudgetvalues.cpp
  kaccounttemplateselector.cpp kmymoneyaccounttreebase.cpp
  kmymoneyaccounttreeforecast.cpp scheduledtransaction.cpp
  stdtransactiondownloaded.cpp stdtransactionmatched.cpp 
  )

KDE3_AUTOMOC(${libkmymoney_la_SOURCES})

SET (libkmymoney_la_UI
  kbudgetvaluesdecl.ui transactionsortoption.ui kaccounttemplateselectordecl.ui
  )
KDE3_ADD_UI_FILES(libkmymoney_la_SOURCES ${libkmymoney_la_UI} )  

SET( kmymoney2_KCFG_SRCS ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc )
KMM_KDE3_ADD_KCFG_FILES( libkmymoney_la_SOURCES
  ${KMyMoney2_SOURCE_DIR} ${kmymoney2_KCFG_SRCS} )

ADD_LIBRARY(kmymoney SHARED ${libkmymoney_la_SOURCES})
# kmm_mymoney and kdeui added manually
TARGET_LINK_LIBRARIES(kmymoney ${QT_AND_KDECORE_LIBRARIES} kmm_mymoney kdeui)
#SET_TARGET_PROPERTIES(kmymoney PROPERTIES
#		      COMPILE_FLAGS "-U_CHECK_MEMORY -DKMM_DESIGNER")

############# next target (widgets) STATIC ###################

SET(libwidgets_a_SOURCES
  kmymoneycompletion.cpp kmymoneyaccountcombo.cpp kmymoneyaccountcompletion.cpp
  kmymoneycurrencyselector.cpp kmymoneypriceview.cpp kmymoneyaccountselector.cpp
  kmymoneybriefschedule.cpp kmymoneyscheduleddatetbl.cpp kmymoneydatetbl.cpp
  kmymoneyscheduledcalendar.cpp kmymoneycalendar.cpp kmymoneycalculator.cpp
  kmymoneycategory.cpp kmymoneycombo.cpp kmymoneylineedit.cpp kmymoneyedit.cpp
  kmymoneydateinput.cpp kmymoneyonlinequoteconfig.cpp kmymoneygpgconfig.cpp
  kmymoneytitlelabel.cpp kguiutils.cpp kmymoneywizard.cpp
  kmymoneyaccounttreebudget.cpp register.cpp registeritem.cpp transaction.cpp
  transactionform.cpp transactioneditorcontainer.cpp kmymoneychecklistitem.cpp
  kmymoneylistviewitem.cpp kmymoneyselector.cpp kmymoneyaccounttree.cpp
  klistviewsearchline.cpp registersearchline.cpp kbudgetvalues.cpp
  kmymoneyforecastlistviewitem.cpp kaccounttemplateselector.cpp
  selectedtransaction.cpp stdtransactiondownloaded.cpp stdtransactionmatched.cpp
  kmymoneyaccounttreebase.cpp scheduledtransaction.cpp
  kmymoneyaccounttreeforecast.cpp
  )

KDE3_AUTOMOC( ${libwidgets_a_SOURCES} )
KDE3_ADD_MOC_FILES(libwidgets_a_SOURCES kmymoneywizard_p.h)

SET(libwidgets_a_UI
  kschedulebriefwidget.ui kmymoneyreportcontroldecl.ui
  kmymoneyreportconfigtab1decl.ui kmymoneyreportconfigtab2decl.ui
  kmymoneyreportconfigtab3decl.ui kmymoneyreportconfigtabchartdecl.ui
  kmymoneyonlinequoteconfigdecl.ui kmymoneygpgconfigdecl.ui 
  )

KDE3_ADD_UI_FILES(libwidgets_a_SOURCES
  ${libkmymoney_la_UI})
KMM_ADD_UI_FILES(libwidgets_a_SOURCES
  ${libwidgets_a_UI} )

ADD_LIBRARY(widgets STATIC ${libwidgets_a_SOURCES})
ADD_DEPENDENCIES(widgets kmymoney)

########### install files ###############

INSTALL(FILES ${kmymoney_STAT_HEADERS} 
	DESTINATION include/kmymoney )
