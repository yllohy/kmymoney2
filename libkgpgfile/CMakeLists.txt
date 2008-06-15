INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(kgpgfile_STAT_SRCS
   kgpgfile.cpp
   )

KMM_CREATE_LINKS( kgpgfile.h )

KDE3_AUTOMOC(${kgpgfile_STAT_SRCS})

ADD_LIBRARY(kgpgfile STATIC ${kgpgfile_STAT_SRCS})


########### install files ###############

INSTALL(FILES ${kgpgfile_STAT_SRCS}
	DESTINATION include/kmymoney
)

