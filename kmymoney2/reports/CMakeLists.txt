INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )
INCLUDE_DIRECTORIES( ${KMyMoney_SOURCE_DIR}/libkdchart)

SET (libreports_a_SOURCES
	pivottable.cpp querytable.cpp reportaccount.cpp 
	kreportchartview.cpp pivotgrid.cpp listtable.cpp
        objectinfotable.cpp
    )

KDE3_AUTOMOC(${libreports_a_SOURCES})

ADD_LIBRARY(reports STATIC ${libreports_a_SOURCES})
ADD_DEPENDENCIES(reports storage)

########### install files ###############

