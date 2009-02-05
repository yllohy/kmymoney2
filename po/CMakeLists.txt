INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )

FILE(GLOB PO_FILES *.po)
ADD_TRANSLATIONS(KMyMoney ${PO_FILES})


add_custom_target(message-stats
  COMMAND "sh" "${CMAKE_CURRENT_SOURCE_DIR}/message-stats.sh" "${CMAKE_CURRENT_SOURCE_DIR}")

########### install files ###############



#original Makefile.am contents follow:

######## kdevelop will overwrite this part!!! (begin)##########
#
#
######## kdevelop will overwrite this part!!! (end)############
#POFILES = AUTO
#
#CONFIG_CLEAN_FILES = $(GMOFILES)
#
#message-stats:
#	@echo "<?xml version='1.0' standalone='yes'?>"
#	@echo "<!DOCTYPE issuelist>"
#	@echo "<translist>"
#	@for i in ${srcdir}/*.po; do \
#	  ${srcdir}/stats.pl $$i; \
#	done;
#	@echo "</translist>"
