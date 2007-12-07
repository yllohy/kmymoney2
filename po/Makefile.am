####### kdevelop will overwrite this part!!! (begin)##########


####### kdevelop will overwrite this part!!! (end)############
POFILES = AUTO

CONFIG_CLEAN_FILES = $(GMOFILES)

message-stats:
	@echo "<?xml version='1.0' standalone='yes'?>"
	@echo "<!DOCTYPE issuelist>"
	@echo "<translist>"
	@echo "<translation><flag>C</flag>"
	@echo "<cvsrevision>"
	@cvs stat kmymoney2.pot | grep Working | cut -d: -f2 | sed -e 's/\ *//'
	@echo "</cvsrevision>"
	@echo -n "<potrevision>"
	@R=`grep POT kmymoney2.pot | cut -d' ' -f 2`; \
	echo "$$R</potrevision>"
	@echo "</translation>"
	
	@for i in ${srcdir}/*.po; do \
	  ${srcdir}/stats.pl $$i; \
	done;
	@echo "</translist>"
