####### kdevelop will overwrite this part!!! (begin)##########

KDE_OPTIONS = noautodist

EXTRA_DIST = backarrow.png background.png bc.png bl.png bottomleft.png bottomright.png br.png center.png filenew.png kmymoneydata.png konqueror.png lc.png manual.png messagebox_info.png original-trans_logo.png rc.png rightborder.png spacer.png tc.png title-center.png title-left.png title-right.png tl.png topleft.png topright.png trans_logo.png tr.png spacer.gif shadow.png currencyflourish.png top-shadow.png arrow.png bg-texture.png about_kde.png

LOCAL_DIR = kmymoney2/html/images

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kmymoney2/html/images
	for file in $(EXTRA_DIST); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR); \
	done

uninstall-local:
	for file in $(EXTRA_DIST); do \
	  rm -f $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)/$$file; \
	done

####### kdevelop will overwrite this part!!! (end)############
