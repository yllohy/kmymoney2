INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(storage_HEADERS
  imymoneystorage.h imymoneyserialize.h imymoneystorageformat.h
  )

KMM_CREATE_LINKS(${storage_HEADERS})

SET(storage_a_SOURCES
  imymoneystorageformat.cpp mymoneystoragexml.cpp mymoneystoragedump.cpp
  mymoneyseqaccessmgr.cpp imymoneystorage.cpp imymoneyserialize.cpp
  mymoneystorageanon.cpp mymoneystoragesql.cpp mymoneydatabasemgr.cpp
  )

KMM_KDE3_ADD_KCFG_FILES(storage_a_SOURCES
  ${KMyMoney2_SOURCE_DIR} ${KMyMoney2_SOURCE_DIR}/kmymoneysettings.kcfgc )

# This library is actually not needed. It is built just for
# convenience, and then linked into kmm_mymoney.
ADD_LIBRARY( kmm_storage STATIC ${storage_a_SOURCES} )

########### install files ###############

INSTALL(FILES ${storage_HEADERS} 
	DESTINATION include/kmymoney )
