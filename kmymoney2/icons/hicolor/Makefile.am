SUBDIRS = 16x16 22x22 32x32 48x48 64x64

LOCAL_DIR=kmymoney2/icons/hicolor

uninstall-local:
	for i in $(SUBDIRS); do \
	  rmdir $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)/$$i; \
	done
