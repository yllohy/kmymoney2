####### kdevelop will overwrite this part!!! (begin)##########

KDE_OPTIONS = noautodist

EXTRA_DIST = startlogo.png titlelabel_background.png 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/pics/
	$(INSTALL_DATA) $(srcdir)/startlogo.png $(kde_datadir)/kmymoney2/pics/startlogo.png
	$(INSTALL_DATA) $(srcdir)/titlelabel_background.png $(kde_datadir)/kmymoney2/pics/titlelabel_background.png

uninstall-local:
	-rm -f $(kde_datadir)/kmymoney2/pics/startlogo.png
	-rm -f $(kde_datadir)/kmymoney2/pics/titlelabel_background.png

####### kdevelop will overwrite this part!!! (end)############

