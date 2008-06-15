INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )

# creating the kmymoneysettings files 
ADD_CUSTOM_COMMAND(OUTPUT ${KMyMoney2_BINARY_DIR}/kmymoneysettings.h
         COMMAND ${KDE3_KCFGC_EXECUTABLE}
         ARGS  -d ${KMyMoney2_BINARY_DIR} ${KMyMoney2_SOURCE_DIR}/kmymoney2.kcfg ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc
         DEPENDS ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc ${KMyMoney2_SOURCE_DIR}/kmymoney2.kcfg  )

SET(storage_HEADERS
    imymoneystorage.h
    imymoneyserialize.h
    imymoneystorageformat.h
   )

KMM_CREATE_LINKS(${storage_HEADERS})

SET(storage_a_SOURCES
    imymoneystorageformat.cpp mymoneystoragexml.cpp mymoneystoragedump.cpp
    mymoneyseqaccessmgr.cpp imymoneystorage.cpp imymoneyserialize.cpp
    mymoneystorageanon.cpp mymoneystoragesql.cpp mymoneydatabasemgr.cpp
    ${KMyMoney2_BINARY_DIR}/kmymoneysettings.h
)


ADD_LIBRARY(storage STATIC ${storage_a_SOURCES})


########### install files ###############

INSTALL(FILES ${storage_HEADERS} 
	DESTINATION include/kmymoney 
)


