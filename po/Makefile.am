####### kdevelop will overwrite this part!!! (begin)##########


####### kdevelop will overwrite this part!!! (end)############
POFILES = AUTO

CONFIG_CLEAN_FILES = $(GMOFILES)

message-stats:
	@echo "<?xml version='1.0' standalone='yes'?>"
	@echo "<!DOCTYPE issuelist>"
	@echo "<translist>"
	@for i in ${srcdir}/*.po; do \
	  ${srcdir}/stats.pl $$i; \
	done;
	@echo "</translist>"
