INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


ADD_CUSTOM_COMMAND(
	 OUTPUT ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.h  ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.cpp
	 DEPENDS ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.h ${KMyMoney2_SOURCE_DIR}/kmymoney2.kcfg ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc
	 COMMAND ${KDE3_KCFGC_EXECUTABLE} -d ${KMyMoney2_SOURCE_DIR}/ ${KMyMoney2_SOURCE_DIR}/kmymoney2.kcfg ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc
         )

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





