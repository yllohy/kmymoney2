####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = home.html kmymoney2.css 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/home.html $(kde_datadir)/kmymoney2/html/home.html
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/kmymoney2.css $(kde_datadir)/kmymoney2/html/kmymoney2.css

uninstall-local:
	-rm -f $(kde_datadir)/kmymoney2/html/home.html
	-rm -f $(kde_datadir)/kmymoney2/html/kmymoney2.css

####### kdevelop will overwrite this part!!! (end)############
