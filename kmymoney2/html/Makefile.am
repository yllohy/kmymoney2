####### kdevelop will overwrite this part!!! (begin)##########

KDE_OPTIONS = noautodist

EXTRA_DIST = home.html kmymoney2.css home_fr.fr.html home_de.de.html home_ru.ru.html home_br.pt_BR.html

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/home.html $(kde_datadir)/kmymoney2/html/home.html
	$(INSTALL_DATA) $(srcdir)/kmymoney2.css $(kde_datadir)/kmymoney2/html/kmymoney2.css
	$(INSTALL_DATA) $(srcdir)/home_fr.fr.html $(kde_datadir)/kmymoney2/html/home_fr.fr.html
	$(INSTALL_DATA) $(srcdir)/home_de.de.html $(kde_datadir)/kmymoney2/html/home_de.de.html
	$(INSTALL_DATA) $(srcdir)/home_ru.ru.html $(kde_datadir)/kmymoney2/html/home_ru.ru.html
	$(INSTALL_DATA) $(srcdir)/home_br.pt_BR.html $(kde_datadir)/kmymoney2/html/home_br.pt_BR.html
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/images
	$(INSTALL_DATA) $(srcdir)/images/* $(kde_datadir)/kmymoney2/html/images
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/css
	$(INSTALL_DATA) $(srcdir)/css/welcome.css $(kde_datadir)/kmymoney2/html/css

uninstall-local:
	-rm -f $(kde_datadir)/kmymoney2/html/home.html
	-rm -f $(kde_datadir)/kmymoney2/html/kmymoney2.css
	-rm -f $(kde_datadir)/kmymoney2/html/home_fr.fr.html
	-rm -f $(kde_datadir)/kmymoney2/html/home_de.de.html
	-rm -f $(kde_datadir)/kmymoney2/html/home_ru.ru.html
	-rm -f $(kde_datadir)/kmymoney2/html/home_br.pt_BR.html
	-rm -f $(kde_datadir)/kmymoney2/html/css/welcome.css
	-rm -f $(kde_datadir)/kmymoney2/html/images/*

####### kdevelop will overwrite this part!!! (end)############
