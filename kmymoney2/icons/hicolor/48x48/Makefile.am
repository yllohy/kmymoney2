SUBDIRS = apps 

LOCAL_DIR=kmymoney2/icons/hicolor/48x48

uninstall-local:
	for i in $(SUBDIRS); do \
	  rm -rf $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)/$$i; \
	done
