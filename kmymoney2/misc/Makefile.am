KDE_OPTIONS = noautodist

SUBDIRS = 

EXTRA_DIST = financequote.pl

LOCAL_DIR = kmymoney2/misc

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)
	for file in $(EXTRA_DIST); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR); \
	done

uninstall-local:
	for file in $(EXTRA_DIST); do \
	  rm -f $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)/$$file; \
	done

