KDE_OPTIONS = noautodist

DISTCLEANFILES = kmymoney2.1.gz

EXTRA_DIST = index.docbook blank_icon.png mainwindow.png newaccount.png newaccount_icon.png newbank.png newbank_icon.png startdialog.png transwindow.png kmymoney2.1 

KDE_LANG = en
KDE_DOCS = kmymoney2

# the automatic KDE install installs this file as well,
# but we don't need it, so we remove immediately
# and install it with the man pages on this system
install-data-hook: kmymoney2.1.gz
	-rm -f $(DESTDIR)$(kde_htmldir)/$(KDE_LANG)/kmymoney2/kmymoney2.1
	$(mkinstalldirs) $(DESTDIR)$(mandir)/man1/
	$(INSTALL_DATA) $(srcdir)/kmymoney2.1.gz $(DESTDIR)$(mandir)/man1/kmymoney2.1.gz

kmymoney2.1.gz: kmymoney2.1
	gzip -9 -c -N kmymoney2.1 > kmymoney2.1.gz

uninstall-hook:
	-rm -f $(DESTDIR)$(mandir)/man1/kmymoney2.1.gz



