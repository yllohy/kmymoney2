####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = home.html kmymoney2.css home_fr.fr.html 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/home.html $(kde_datadir)/kmymoney2/html/home.html
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/kmymoney2.css $(kde_datadir)/kmymoney2/html/kmymoney2.css
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/home_fr.fr.html $(kde_datadir)/kmymoney2/html/home_fr.fr.html

uninstall-local:
	-rm -f $(kde_datadir)/kmymoney2/html/home.html
	-rm -f $(kde_datadir)/kmymoney2/html/kmymoney2.css
	-rm -f $(kde_datadir)/kmymoney2/html/home_fr.fr.html

####### kdevelop will overwrite this part!!! (end)############
