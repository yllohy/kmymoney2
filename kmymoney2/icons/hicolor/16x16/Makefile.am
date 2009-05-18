SUBDIRS = actions 

LOCAL_DIR=kmymoney2/icons/hicolor/16x16

uninstall-local:
	for i in $(SUBDIRS); do \
	  rm -rf $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)/$$i; \
	done
