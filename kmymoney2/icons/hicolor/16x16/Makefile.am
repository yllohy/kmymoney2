SUBDIRS = actions 

LOCAL_DIR=kmymoney2/icons/hicolor/16x16

uninstall-local:
	for i in $(SUBDIRS); do \
	  rmdir $(kde_datadir)/$(LOCAL_DIR)/$$i; \
	done
