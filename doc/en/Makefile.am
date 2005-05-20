KDE_OPTIONS = noautodist

DISTCLEANFILES = kmymoney2.1.gz

DOCBOOK_FILES = credits.docbook details-accounts.docbook  details-categories.docbook details-files.docbook details-formats.docbook  details-hbciofx.docbook details-impexp.docbook details-institutions.docbook details-integrity.docbook details-investments.docbook details-ledgers.docbook details-loans.docbook details-payees.docbook details-reconciliation.docbook details-reports.docbook details-schedules.docbook details-search.docbook details-settings.docbook details-transactions.docbook details-widgets.docbook firsttime.docbook index.docbook introduction.docbook otherdoc.docbook views-accounts.docbook views-categories.docbook views-home.docbook views-investments.docbook views-ledgers.docbook views-outbox.docbook views-payees.docbook views-schedules.docbook views-institutions.docbook views-reports.docbook whatsnew.docbook details-currencies.docbook

EXTRA_DIST = kmymoney2.1 $(DOCBOOK_FILES)

KDE_LANG = en
KDE_DOCS = kmymoney2

index.cache.bz2: $(DOCBOOK_FILES) $(KDE_XSL_STYLESHEET)
	if test -n "$(MEINPROC)"; then echo $(MEINPROC) --check --cache index.cache.bz2 $(srcdir)/index.docbook; $(MEINPROC) --check --cache index.cache.bz2 $(srcdir)/index.docbook; fi


# The automatic KDE install installs some files,
# that we don't need, so we remove them immediately.
# Also install the man pages on this system
install-data-hook: kmymoney2.1.gz
	-rm -f $(DESTDIR)$(kde_htmldir)/$(KDE_LANG)/kmymoney2/kmymoney2.1
	$(mkinstalldirs) $(DESTDIR)$(mandir)/man1/
	$(INSTALL_DATA) kmymoney2.1.gz $(DESTDIR)$(mandir)/man1/kmymoney2.1.gz

kmymoney2.1.gz: kmymoney2.1
	gzip -9 -c -N $(top_srcdir)/$(subdir)/kmymoney2.1 > kmymoney2.1.gz

uninstall-hook:
	-rm -f $(DESTDIR)$(mandir)/man1/kmymoney2.1.gz



