KDE_OPTIONS = noautodist

DISTCLEANFILES = kmymoney2.1.gz

EXTRA_DIST = credits.docbook details-accounts.docbook  details-categories.docbook details-files.docbook details-formats.docbook  details-hbciofx.docbook details-impexp.docbook details-institutions.docbook details-integrity.docbook details-investments.docbook details-ledgers.docbook details-loans.docbook details-payees.docbook details-reconciliation.docbook details-reports.docbook details-schedules.docbook details-search.docbook details-settings.docbook details-transactions.docbook details-widgets.docbook firsttime.docbook index.docbook introduction.docbook otherdoc.docbook views-accounts.docbook views-categories.docbook views-home.docbook views-investments.docbook views-ledgers.docbook views-outbox.docbook views-payees.docbook views-schedules.docbook views-institutions.docbook views-reports.docbook blank_icon.png mainwindow.png newaccount.png newaccount_icon.png newbank.png newbank_icon.png startdialog.png transwindow.png kmymoney2.1

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



