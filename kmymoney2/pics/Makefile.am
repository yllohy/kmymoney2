####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = toolbar 

EXTRA_DIST = account-folder.xpm bank-folder.xpm startlogo.png 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/pics/
	$(INSTALL_DATA) $(srcdir)/account-folder.xpm $(kde_datadir)/kmymoney2/pics/account-folder.xpm
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/pics/
	$(INSTALL_DATA) $(srcdir)/bank-folder.xpm $(kde_datadir)/kmymoney2/pics/bank-folder.xpm
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/pics/
	$(INSTALL_DATA) $(srcdir)/startlogo.png $(kde_datadir)/kmymoney2/pics/startlogo.png

uninstall-local:
	-rm -f $(kde_datadir)/kmymoney2/pics/account-folder.xpm
	-rm -f $(kde_datadir)/kmymoney2/pics/bank-folder.xpm
	-rm -f $(kde_datadir)/kmymoney2/pics/startlogo.png

####### kdevelop will overwrite this part!!! (end)############
