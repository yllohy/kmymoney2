SUBDIRS = hicolor 

LOCAL_DIR=kmymoney2/icons

uninstall-local:
	for i in $(SUBDIRS); do \
	  rm -rf $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)/$$i; \
	done
