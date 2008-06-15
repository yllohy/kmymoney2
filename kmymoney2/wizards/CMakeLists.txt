Project(Wizards)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR}/wizardpages )

ADD_SUBDIRECTORY( wizardpages )
ADD_SUBDIRECTORY( newuserwizard ) 
ADD_SUBDIRECTORY( newaccountwizard )

########### next target ###############

#SET(libwizards_a_SOURCES
#	dummy.cpp
#)

#ADD_LIBRARY(wizards STATIC ${libwizards_a_SOURCES})
#TARGET_LINK_LIBRARIES(wizards  
#		      wizardpages newaccountwizard newuserwizard 
#)
#ADD_DEPENDENCIES(wizards wizardpages newaccountwizard newuserwizard)
########### install files ###############

#INSTALL_FILES( /include/kmymoney FILES  )

