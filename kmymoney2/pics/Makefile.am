####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = toolbar 

EXTRA_DIST = account-folder.xpm bank-folder.xpm 

install-data-local:
	$(mkinstalldirs) $$(kde_datadir)/kmymoney2/pics/
	$(INSTALL_DATA) account-folder.xpm $$(kde_datadir)/kmymoney2/pics/account-folder.xpm
	$(mkinstalldirs) $$(kde_datadir)/kmymoney2/pics/
	$(INSTALL_DATA) bank-folder.xpm $$(kde_datadir)/kmymoney2/pics/bank-folder.xpm

uninstall-local:
	-rm -f $$(kde_datadir)/kmymoney2/pics/account-folder.xpm
	-rm -f $$(kde_datadir)/kmymoney2/pics/bank-folder.xpm

####### kdevelop will overwrite this part!!! (end)############
