Project(Dialogs)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR}/wizardpages )
INCLUDE_DIRECTORIES( ${KMyMoney_SOURCE_DIR}/libkdchart)

ADD_SUBDIRECTORY( settings ) 
#ADD_SUBDIRECTORY( wizardpages )
#ADD_SUBDIRECTORY( newuserwizard ) 
#ADD_SUBDIRECTORY( newaccountwizard )

########### next target ###############

SET(libdialogs_a_SOURCES
kreportconfigurationfilterdlg.cpp kcurrencycalculator.cpp
kcurrencyeditdlg.cpp keditequityentrydlg.cpp keditloanwizard.cpp
knewloanwizard.cpp kenterscheduledlg.cpp krecentfileitem.cpp
keditscheduledlg.cpp mymoneyqifprofileeditor.cpp kaccountselectdlg.cpp
kupdatestockpricedlg.cpp knewequityentrydlg.cpp kreconcilelistitem.cpp
kstartdlg.cpp kreconciledlg.cpp knewfiledlg.cpp knewbankdlg.cpp
knewaccountdlg.cpp kfindtransactiondlg.cpp kendingbalancedlg.cpp
ksplittransactiondlg.cpp kimportdlg.cpp kexportdlg.cpp
kcsvprogressdlg.cpp kchooseimportexportdlg.cpp kbackupdlg.cpp
kequitypriceupdatedlg.cpp kmymoneypricedlg.cpp
knewinvestmentwizard.cpp knewinvestmentwizard.h
ksecuritylisteditor.cpp kgncimportoptionsdlg.cpp
konlinequoteconfigurationdlg.cpp kgncpricesourcedlg.cpp
kmymoneyfileinfodlg.cpp kselectdatabasedlg.cpp kpayeereassigndlg.cpp
kcategoryreassigndlg.cpp kconfirmmanualenterdlg.cpp
transactioneditor.cpp investtransactioneditor.cpp investactivities.cpp
kbalancechartdlg.cpp knewbudgetdlg.cpp
kselecttransactionsdlg.cpp kmergetransactionsdlg.cpp
kloadtemplatedlg.cpp kmymoneysplittable.cpp kgpgkeyselectiondlg.cpp
transactionmatcher.cpp
)

KDE3_AUTOMOC(${libdialogs_a_SOURCES})

SET(dialogs_HEADERS
  transactioneditor.h investtransactioneditor.h kcurrencycalculator.h
)

KMM_CREATE_LINKS( ${dialogs_HEADERS} )

SET( dialogs_UI
kaccountselectdlgdecl.ui kbackupdlgdecl.ui
kchooseimportexportdlgdecl.ui kconfirmmanualenterdlgdecl.ui
kcsvprogressdlgdecl.ui kcurrencycalculatordecl.ui
kcurrencyeditdlgdecl.ui keditequityentrydecl.ui
kendingbalancedlgdecl.ui kenterscheduledlgdecl.ui
kequitypriceupdatedlgdecl.ui kexportdlgdecl.ui
kfindtransactiondlgdecl.ui kgncimportoptionsdlgdecl.ui
kimportdlgdecl.ui kmymoneypricedlgdecl.ui knewaccountdlgdecl.ui
knewbankdlgdecl.ui knewequityentrydecl.ui knewfiledlgdecl.ui
knewinvestmentwizarddecl.ui knewloanwizarddecl.ui
konlinequoteconfigurationdecl.ui kreconciledlgdecl.ui
ksecuritylisteditordecl.ui ksplitcorrectiondlg.ui
ksplittransactiondlgdecl.ui kupdatestockpricedlgdecl.ui
mymoneyqifprofileeditordecl.ui kgncpricesourcedlgdecl.ui
kmymoneyfileinfodlgdecl.ui kselectdatabasedlgdecl.ui
kpayeereassigndlgdecl.ui kcategoryreassigndlgdecl.ui
konlinebankingsetupdecl.ui knewbudgetdlgdecl.ui
kselecttransactionsdlgdecl.ui ksortoptiondlg.ui
keditscheduledlgdecl.ui kplugindlg.ui kloadtemplatedlgdecl.ui
)

KMM_ADD_UI_FILES(libdialogs_a_SOURCES ${dialogs_UI} )
#KDE3_ADD_UI_FILES(libdialogs_a_SOURCES ${dialogs_UI} )

ADD_LIBRARY(dialogs STATIC ${libdialogs_a_SOURCES})
ADD_DEPENDENCIES(dialogs widgets)

########### install files ###############

INSTALL(FILES  ${dialogs_HEADERS}
	DESTINATION include/kmymoney 
)

