SUBDIRS = apps 

LOCAL_DIR=kmymoney2/icons/hicolor/32x32

uninstall-local:
	for i in $(SUBDIRS); do \
	  rmdir $(kde_datadir)/$(LOCAL_DIR)/$$i; \
	done
