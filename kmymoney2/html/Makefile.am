####### kdevelop will overwrite this part!!! (begin)##########

KDE_OPTIONS = noautodist

EXTRA_DIST = home.html kmymoney2.css welcome.css whats_new.html home_fr.fr.html home_de.de.html home_ru.ru.html home_br.pt_BR.html

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/
	$(INSTALL_DATA) $(srcdir)/home.html $(kde_datadir)/kmymoney2/html
	$(INSTALL_DATA) $(srcdir)/whats_new.html $(kde_datadir)/kmymoney2/html
	$(INSTALL_DATA) $(srcdir)/kmymoney2.css $(kde_datadir)/kmymoney2/html/kmymoney2.css
	$(INSTALL_DATA) $(srcdir)/home_fr.fr.html $(kde_datadir)/kmymoney2/html/home_fr.fr.html
	$(INSTALL_DATA) $(srcdir)/home_de.de.html $(kde_datadir)/kmymoney2/html/home_de.de.html
	$(INSTALL_DATA) $(srcdir)/home_ru.ru.html $(kde_datadir)/kmymoney2/html/home_ru.ru.html
	$(INSTALL_DATA) $(srcdir)/home_br.pt_BR.html $(kde_datadir)/kmymoney2/html/home_br.pt_BR.html
	$(INSTALL_DATA) $(srcdir)/welcome.css $(kde_datadir)/kmymoney2/html
	$(mkinstalldirs) $(kde_datadir)/kmymoney2/html/images
	$(INSTALL_DATA) $(srcdir)/images/*.png $(kde_datadir)/kmymoney2/html/images
	$(INSTALL_DATA) $(srcdir)/images/*.gif $(kde_datadir)/kmymoney2/html/images

uninstall-local:
	-rm -f $(kde_datadir)/kmymoney2/html/home.html
	-rm -f $(kde_datadir)/kmymoney2/html/whats_new.html
	-rm -f $(kde_datadir)/kmymoney2/html/kmymoney2.css
	-rm -f $(kde_datadir)/kmymoney2/html/home_fr.fr.html
	-rm -f $(kde_datadir)/kmymoney2/html/home_de.de.html
	-rm -f $(kde_datadir)/kmymoney2/html/home_ru.ru.html
	-rm -f $(kde_datadir)/kmymoney2/html/home_br.pt_BR.html
	-rm -f $(kde_datadir)/kmymoney2/html/welcome.css
	-rm -f $(kde_datadir)/kmymoney2/html/images/*

####### kdevelop will overwrite this part!!! (end)############
