SUBDIRS = hicolor 

LOCAL_DIR=kmymoney2/icons

uninstall-local:
	for i in $(SUBDIRS); do \
	  rmdir $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)/$$i; \
	done
