KDE_OPTIONS = noautodist

SUBDIRS = images

EXTRA_DIST = home.html kmymoney2.css welcome.css whats_new.html home_fr.fr.html home_de.de.html home_ru.ru.html home_br.pt_BR.html home_gl.html whats_new_gl.html whats_new_de.html

LOCAL_DIR = kmymoney2/html

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)
	for file in $(EXTRA_DIST); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR); \
	done

uninstall-local:
	for file in $(EXTRA_DIST); do \
	  rm -f $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)/$$file; \
	done

