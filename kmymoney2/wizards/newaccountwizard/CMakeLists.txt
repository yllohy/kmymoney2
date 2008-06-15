INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )

SET (libnewaccountwizard_a_SOURCES 
	knewaccountwizard.cpp 
        )

SET (libnewaccountwizard_a_UI
	kinstitutionpagedecl.ui kaccounttypepagedecl.ui kbrokeragepagedecl.ui kschedulepagedecl.ui
	kgeneralloaninfopagedecl.ui kloandetailspagedecl.ui kloanpaymentpagedecl.ui
	kloanschedulepagedecl.ui kloanpayoutpagedecl.ui kaccountsummarypagedecl.ui
        khierarchypagedecl.ui
	)

KMM_ADD_UI_FILES(libnewaccountwizard_a_SOURCES ${libnewaccountwizard_a_UI})

KDE3_AUTOMOC(${libnewaccountwizard_a_SOURCES})

KDE3_ADD_MOC_FILES(libnewaccountwizard_a_SOURCES knewaccountwizard_p.h)

ADD_LIBRARY(newaccountwizard STATIC ${libnewaccountwizard_a_SOURCES})
ADD_DEPENDENCIES(newaccountwizard dialogs)

#ADD_DEPENDENCIES(newaccountwizard settings)

########### install files ###############


