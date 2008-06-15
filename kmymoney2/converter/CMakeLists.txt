
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


SET (libconverter_a_SOURCES 
	mymoneyqifreader.cpp mymoneyqifwriter.cpp mymoneyqifprofile.cpp
	mymoneytemplate.cpp mymoneystatementreader.cpp 
	webpricequote.cpp mymoneygncreader.cpp
	)

KDE3_AUTOMOC(${libconverter_a_SOURCES})

KMM_CREATE_LINKS( mymoneytemplate.h )

ADD_LIBRARY(converter STATIC ${libconverter_a_SOURCES})
ADD_DEPENDENCIES(converter dialogs)

########### install files ###############

INSTALL(FILES mymoneytemplate.h
	DESTINATION include/kmymoney
)



