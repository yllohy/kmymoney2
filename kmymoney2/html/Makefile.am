####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = home.html kmymoney2.css home_fr.fr.html home_de.de.html home_ru.ru.html 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/home.html $(kde_datadir)/kmymoney2/html/home.html
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/kmymoney2.css $(kde_datadir)/kmymoney2/html/kmymoney2.css
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/home_fr.fr.html $(kde_datadir)/kmymoney2/html/home_fr.fr.html
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/home_de.de.html $(kde_datadir)/kmymoney2/html/home_de.de.html
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/home_ru.ru.html $(kde_datadir)/kmymoney2/html/home_ru.ru.html

uninstall-local:
	-rm -f $(kde_datadir)/kmymoney2/html/home.html
	-rm -f $(kde_datadir)/kmymoney2/html/kmymoney2.css
	-rm -f $(kde_datadir)/kmymoney2/html/home_fr.fr.html
	-rm -f $(kde_datadir)/kmymoney2/html/home_de.de.html
	-rm -f $(kde_datadir)/kmymoney2/html/home_ru.ru.html

####### kdevelop will overwrite this part!!! (end)############
