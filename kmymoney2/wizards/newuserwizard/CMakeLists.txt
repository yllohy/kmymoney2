INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )

SET (libnewuserwizard_a_SOURCES 
	knewuserwizard.cpp
	)

SET (libnewuserwizard_a_UI
	kgeneralpagedecl.ui kcurrencypagedecl.ui kpasswordpagedecl.ui
        kintropagedecl.ui kaccountpagedecl.ui kpreferencepagedecl.ui 
        kfilepagedecl.ui
	)

KMM_ADD_UI_FILES(libnewuserwizard_a_SOURCES ${libnewuserwizard_a_UI})

KDE3_AUTOMOC(${libnewuserwizard_a_SOURCES})

KDE3_ADD_MOC_FILES(libnewuserwizard_a_SOURCES knewuserwizard_p.h)

ADD_LIBRARY(newuserwizard STATIC ${libnewuserwizard_a_SOURCES})
ADD_DEPENDENCIES(newuserwizard wizardpages)

########### install files ###############





