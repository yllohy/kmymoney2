INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET (libnewaccountwizard_a_SOURCES 
  knewaccountwizard.cpp 
  )

SET (libnewaccountwizard_a_UI
  kaccountsummarypagedecl.ui kaccounttypepagedecl.ui
  kbrokeragepagedecl.ui kgeneralloaninfopagedecl.ui
  khierarchypagedecl.ui kinstitutionpagedecl.ui
  kloandetailspagedecl.ui kloanpaymentpagedecl.ui
  kloanpayoutpagedecl.ui kloanschedulepagedecl.ui kschedulepagedecl.ui
	)

# The handling of these ui files depends
# on libkmymoney.so (the widgets library)
KMM_ADD_UI_FILES(libnewaccountwizard_a_SOURCES ${libnewaccountwizard_a_UI}
  DEPENDS kmymoney)

KDE3_AUTOMOC(${libnewaccountwizard_a_SOURCES})

KDE3_ADD_MOC_FILES(libnewaccountwizard_a_SOURCES knewaccountwizard_p.h)

ADD_LIBRARY(newaccountwizard STATIC ${libnewaccountwizard_a_SOURCES})
