PROJECT(KMyMoney2)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )
INCLUDE_DIRECTORIES( ${KMyMoney_SOURCE_DIR}/libkdchart )

ADD_SUBDIRECTORY( mymoney ) 
ADD_SUBDIRECTORY( views )
ADD_SUBDIRECTORY( plugins ) 
ADD_SUBDIRECTORY( dialogs ) 
ADD_SUBDIRECTORY( widgets ) 
ADD_SUBDIRECTORY( converter ) 
ADD_SUBDIRECTORY( reports ) 
ADD_SUBDIRECTORY( wizards )
ADD_SUBDIRECTORY( pics ) 
ADD_SUBDIRECTORY( html ) 
ADD_SUBDIRECTORY( templates ) 

INCLUDE_DIRECTORIES( ${Dialogs_BINARY_DIR} )

SET(_HEADERS 
    export.h
    kmymoneyutils.h
    kmymoneyglobalsettings.h
)

# here we call the module wich creates us the given links
KMM_CREATE_LINKS( ${_HEADERS})

KMM_CREATE_LINKS_BIN( kmymoneysettings.h )

########### next target ###############

SET(kmymoney2_SRCS
   kmymoneyutils.cpp
   kstartuplogo.cpp
   kmymoney2.cpp
   main.cpp
   kmymoneyglobalsettings.cpp
   )

SET( kmymoney2_DCOP_SKEL_SRCS
   kmymoney2.h
   )

KDE3_ADD_DCOP_SKELS(kmymoney2_SRCS ${kmymoney2_DCOP_SKEL_SRCS})

SET( kmymoney2_DCOP_STUB_SRCS
   kmymoney2.h
   )

KDE3_ADD_DCOP_STUBS(kmymoney2_SRCS ${kmymoney2_DCOP_STUB_SRCS})

SET( kmymoney2_KCFG_SRCS
   kmymoneysettings.kcfgc
   )

KDE3_ADD_KCFG_FILES(kmymoney2_SRCS ${kmymoney2_KCFG_SRCS})

KDE3_AUTOMOC(${kmymoney2_SRCS})

KDE3_ADD_EXECUTABLE(kmymoney2 ${kmymoney2_SRCS})

TARGET_LINK_LIBRARIES(kmymoney2  ${QT_AND_KDECORE_LIBS} 
			views reports newuserwizard newaccountwizard
			wizardpages dialogs settings widgets
			converter storage kmm_mymoney kgpgfile
			interfaces kmm_plugin dialogs kmm_kdchart
			khtml kdeui kabc  )

INSTALL( TARGETS kmymoney2 
	 DESTINATION bin)


########### next target ###############

SET(kmymoneytest_SRCS
   kmymoneytest.cpp
   kmymoneyutils.cpp
   kmymoneyglobalsettings.cpp
   )

KDE3_AUTOMOC(${kmymoneytest_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(kmymoneytest ${kmymoneytest_SRCS})

TARGET_LINK_LIBRARIES(kmymoneytest  ${QT_AND_KDECORE_LIBS} )

ENDIF(KDE3_BUILD_TESTS)

########### install files ###############

INSTALL(FILES kmymoney2.kcfg
	DESTINATION share/config.kcfg 
)
INSTALL(FILES kmymoney2ui.rc tips
	DESTINATION share/apps/kmymoney2 
)
INSTALL(FILES ${_HEADERS}
	DESTINATION include/kmymoney
)
INSTALL(FILES kmymoney2.desktop
	DESTINATION share/applications/kde
)
INSTALL(DIRECTORY icons/hicolor
	DESTINATION share/apps/kmymoney2/icons
	PATTERN "CVS" EXCLUDE
	PATTERN "*Make*" EXCLUDE
)
INSTALL(FILES misc/financequote.pl
	DESTINATION share/apps/kmymoney2/misc
)
# Application Icons
INSTALL(FILES hi16-app-kmymoney2.png
	DESTINATION share/icons/hicolor/16x16/apps
)
INSTALL(FILES mimetype_kmy_16x16.png
	DESTINATION share/icons/hicolor/16x16/mimetypes
)
INSTALL(FILES hi32-app-kmymoney2.png
	DESTINATION share/icons/hicolor/32x32/apps
)
INSTALL(FILES mimetype_kmy_32x32.png
	DESTINATION share/icons/hicolor/32x32/mimetypes
)
INSTALL(FILES hi48-app-kmymoney2.png
	DESTINATION share/icons/hicolor/48x48/apps
)
INSTALL(FILES mimetype_kmy_48x48.png
	DESTINATION share/icons/hicolor/48x48/mimetypes
)
INSTALL(FILES hi64-app-kmymoney2.png
	DESTINATION share/icons/hicolor/68x68/apps
)
INSTALL(FILES mimetype_kmy_64x64.png
	DESTINATION share/icons/hicolor/64x64/mimetypes
)
INSTALL(FILES hi128-app-kmymoney2.png
	DESTINATION share/icons/hicolor/128x128/apps
)
INSTALL(FILES mimetype_kmy_128x128.png
	DESTINATION share/icons/hicolor/128x128/mimetypes
)
INSTALL(FILES lo16-app-kmymoney2.png
	DESTINATION share/icons/locolor/16x16/apps
)
INSTALL(FILES lo32-app-kmymoney2.png
	DESTINATION share/icons/locolor/32x32/apps
)
