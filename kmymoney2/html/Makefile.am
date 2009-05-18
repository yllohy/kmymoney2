KDE_OPTIONS = noautodist

SUBDIRS = images

HOMEPAGES = home_pt_BR.html home_de.html home_es.html home_fr.html home_gl.html home.html home_it.html home_nl.html home_ro.html home_ru.html home_tr.html home_pt.html

WHATSNEWPAGES = whats_new_pt_BR.html whats_new_de.html whats_new_es.html whats_new_fr.html whats_new_gl.html whats_new.html whats_new_it.html whats_new_nl.html whats_new_ro.html whats_new_ru.html whats_new_tr.html

EXTRA_DIST = $(HOMEPAGES) $(WHATSNEWPAGES) kmymoney2.css welcome.css

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

