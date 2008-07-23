INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )
INCLUDE_DIRECTORIES( ${KMyMoney_SOURCE_DIR}/libkdchart )

SET (libviews_a_SOURCES 
	kreportsview.cpp kmymoneytransaction.cpp kgloballedgerview.cpp
	kmymoneyfile.cpp kinvestmentlistitem.cpp kinvestmentview.cpp
	kscheduledlistitem.cpp  
	kscheduledview.cpp kpayeesview.cpp 
	khomeview.cpp  kcategoriesview.cpp
	kmymoneyview.cpp  kaccountsview.cpp
	kinstitutionsview.cpp kbudgetview.cpp
	kforecastview.cpp
	)

SET (libviews_a_HEADER
	kmymoneyview.h
)

SET (libviews_a_UI
kaccountsviewdecl.ui kcategoriesviewdecl.ui kinvestmentviewdecl.ui
kpayeesviewdecl.ui kscheduledviewdecl.ui kinstitutionsviewdecl.ui
kbudgetviewdecl.ui kforecastviewdecl.ui
)

KMM_CREATE_LINKS( ${libviews_a_HEADER} )

KDE3_AUTOMOC(${libviews_a_SOURCES})

KMM_ADD_UI_FILES(libviews_a_SOURCES ${libviews_a_UI} )

ADD_LIBRARY(views STATIC ${libviews_a_SOURCES})
ADD_DEPENDENCIES(views dialogs)

########### install files ###############

INSTALL(FILES ${libviews_a_HEADER}
	DESTINATION include/kmymoney 
)


