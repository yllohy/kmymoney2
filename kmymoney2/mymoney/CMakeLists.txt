ADD_SUBDIRECTORY( storage )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

SET(kmm_mymoney_LIB_SRCS
  mymoneymoney.cpp mymoneyfinancialcalculator.cpp mymoneytransactionfilter.cpp
  mymoneyobject.cpp mymoneykeyvaluecontainer.cpp mymoneyobserver.cpp
  mymoneysubject.cpp mymoneysplit.cpp mymoneyinstitution.cpp
  mymoneyexception.cpp mymoneyinvesttransaction.cpp mymoneyutils.cpp
  mymoneysecurity.cpp mymoneytransaction.cpp mymoneyscheduled.cpp
  mymoneypayee.cpp mymoneyfile.cpp mymoneycategory.cpp mymoneyaccount.cpp
  mymoneyreport.cpp mymoneystatement.cpp mymoneyprice.cpp mymoneybudget.cpp
  mymoneyobjectcontainer.cpp mymoneyforecast.cpp
  )


SET(mymoney_HEADERS
  mymoneyobject.h mymoneyaccount.h mymoneycategory.h mymoneyexception.h
  mymoneyfile.h mymoneyfinancialcalculator.h mymoneyinstitution.h
  mymoneyinvesttransaction.h mymoneykeyvaluecontainer.h mymoneymoney.h
  mymoneyobserver.h mymoneypayee.h mymoneyprice.h mymoneyreport.h
  mymoneyscheduled.h mymoneysecurity.h mymoneysplit.h mymoneystatement.h
  mymoneysubject.h mymoneytransactionfilter.h mymoneytransaction.h
  mymoneyutils.h mymoneybudget.h mymoneyobjectcontainer.h mymoneyforecast.h
  )

KDE3_AUTOMOC(${kmm_mymoney_LIB_SRCS})

KMM_CREATE_LINKS( ${mymoney_HEADERS} )

ADD_LIBRARY(kmm_mymoney SHARED ${kmm_mymoney_LIB_SRCS})

TARGET_LINK_LIBRARIES(kmm_mymoney ${QT_AND_KDECORE_LIBS} storage)

SET_TARGET_PROPERTIES(kmm_mymoney PROPERTIES VERSION 5.0.0 SOVERSION 5 )

########### install files ###############

INSTALL(TARGETS kmm_mymoney
	DESTINATION lib )

INSTALL(FILES ${mymoney_HEADERS}
	DESTINATION include/kmymoney )

