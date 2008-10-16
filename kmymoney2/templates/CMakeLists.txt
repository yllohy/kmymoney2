INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############


INSTALL(DIRECTORY C de_AT de_CH de_DE dk el_GR en_GB en_US 
		  es_ES es_MX fr_CA fr_CH fr_FR gl_ES hu_HU it
		  jp nl_NL pt_BR pt_PT ro_RO ru_SU sk tr_TR 
                  zh_CN zh_HK zh_TW
	DESTINATION share/apps/kmymoney2/templates
	PATTERN "CVS" EXCLUDE
	PATTERN "*Make*" EXCLUDE
)

