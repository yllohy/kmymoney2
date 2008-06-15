INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )

SET (libinterfaces_a_SOURCES
	kmmviewinterface.cpp kmmstatementinterface.cpp kmmimportinterface.cpp
	)

KDE3_AUTOMOC(${libinterfaces_a_SOURCES})

ADD_LIBRARY(interfaces STATIC ${libinterfaces_a_SOURCES})
ADD_DEPENDENCIES(interfaces kmymoney)

########### install files ###############

