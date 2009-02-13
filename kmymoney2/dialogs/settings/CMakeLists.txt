INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


ADD_CUSTOM_COMMAND(
	 OUTPUT ${KMyMoney2_BINARY_DIR}/kmymoneysettings.h  ${KMyMoney2_BINARY_DIR}/kmymoneysettings.cpp
	 DEPENDS ${KMyMoney2_SOURCE_DIR}/kmymoney2.kcfg ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc
	 COMMAND ${KDE3_KCFGC_EXECUTABLE} -d ${KMyMoney2_BINARY_DIR}/ ${KMyMoney2_SOURCE_DIR}/kmymoney2.kcfg ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc
         )


SET (libsettings_a_SOURCES 
	 ksettingsgeneral.cpp 
	 ksettingsregister.cpp 
	 ksettingsgpg.cpp 
	 ksettingscolors.cpp 
	 ksettingsfonts.cpp 
	 ksettingsschedules.cpp 
	 ksettingsonlinequotes.cpp 
	 ksettingshome.cpp
	 ksettingsforecast.cpp
         ksettingsplugins.cpp
	 ${KMyMoney2_BINARY_DIR}/kmymoneysettings.h
	)

SET (libsettings_a_UI 
	ksettingsgeneraldecl.ui ksettingsregisterdecl.ui ksettingsgpgdecl.ui
	ksettingscolorsdecl.ui ksettingsfontsdecl.ui ksettingsschedulesdecl.ui
	ksettingsonlinequotesdecl.ui ksettingshomedecl.ui ksettingsforecastdecl.ui
	)

KMM_ADD_UI_FILES(libsettings_a_SOURCES ${libsettings_a_UI})

KDE3_AUTOMOC(${libsettings_a_SOURCES})

ADD_LIBRARY(settings STATIC ${libsettings_a_SOURCES})

# Manually added dependency to kmymoney shared lib. 
# This cause libkmymoney to be build before settings.
ADD_DEPENDENCIES(settings kmymoney)

########### install files ###############



