####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = home.html kmymoney2.css 

install-data-local:
	$(mkinstalldirs) $(datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/home.html $(datadir)/kmymoney2/html/home.html
	$(mkinstalldirs) $(datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/kmymoney2.css $(datadir)/kmymoney2/html/kmymoney2.css

uninstall-local:
	-rm -f $(datadir)/kmymoney2/html/home.html
	-rm -f $(datadir)/kmymoney2/html/kmymoney2.css

####### kdevelop will overwrite this part!!! (end)############
