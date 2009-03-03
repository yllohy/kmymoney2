INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/dialogs
  ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )

ADD_SUBDIRECTORY( dialogs )

########### next target ###############

SET(kmm_ofximport_PART_SRCS
  ofximporterplugin.cpp ofxpartner.cpp)

KDE3_AUTOMOC(${kmm_ofximport_PART_SRCS})

KDE3_ADD_KPART(kmm_ofximport ${kmm_ofximport_PART_SRCS})

TARGET_LINK_LIBRARIES(kmm_ofximport ${QT_AND_KDECORE_LIBS} kmm_plugin
  kmm_mymoney kmm_ofximport_dialogs)

########### install files ###############

INSTALL(TARGETS kmm_ofximport 
  DESTINATION lib/kde3)

INSTALL(FILES kmm_ofximport.desktop
  DESTINATION share/services)
