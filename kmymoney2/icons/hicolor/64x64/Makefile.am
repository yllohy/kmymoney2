SUBDIRS = apps 

LOCAL_DIR=kmymoney2/icons/hicolor/64x64

uninstall-local:
	for i in $(SUBDIRS); do \
	  rmdir $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)/$$i; \
	done
