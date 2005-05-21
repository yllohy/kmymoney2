KDE_OPTIONS = noautodist

DOCBOOK_FILES = credits.docbook details-accounts.docbook  details-categories.docbook details-files.docbook details-formats.docbook  details-hbciofx.docbook details-impexp.docbook details-institutions.docbook details-integrity.docbook details-investments.docbook details-ledgers.docbook details-loans.docbook details-payees.docbook details-reconciliation.docbook details-reports.docbook details-schedules.docbook details-search.docbook details-settings.docbook details-transactions.docbook details-widgets.docbook firsttime.docbook index.docbook introduction.docbook otherdoc.docbook views-accounts.docbook views-categories.docbook views-home.docbook views-investments.docbook views-ledgers.docbook views-outbox.docbook views-payees.docbook views-schedules.docbook views-institutions.docbook views-reports.docbook whatsnew.docbook details-currencies.docbook

SCREENSHOT_FILES = ledgers.png

EXTRA_DIST = kmymoney2.1 $(DOCBOOK_FILES)

MOSTLYCLEANFILES = index.cache.bz2 kmymoney2.1.gz

BUILT_SOURCES = index.cache.bz2 kmymoney2.1.gz

KDE_LANG = en
KDE_DOCS = kmymoney2 - $(DOCBOOK_FILES) $(SCREENSHOT_FILES)

index.cache.bz2: $(DOCBOOK_FILES) $(KDE_XSL_STYLESHEET) $(SCREENSHOT_FILES)
	if test -n "$(MEINPROC)"; then echo $(MEINPROC) --check --cache index.cache.bz2 --stylesheet $(KDE_XSL_STYLESHEET) $(srcdir)/index.docbook; $(MEINPROC) --check --cache index.cache.bz2 --stylesheet $(KDE_XSL_STYLESHEET) $(srcdir)/index.docbook; fi


# The automatic KDE install installs some files,
# that we don't need, so we remove them immediately.
# Also install the man pages on this system
.PHONY: install-docs uninstall-docs

install-data-hook: kmymoney2.1.gz install-docs
	-rm -f $(DESTDIR)$(kde_htmldir)/$(KDE_LANG)/kmymoney2/kmymoney2.1
	$(mkinstalldirs) $(DESTDIR)$(mandir)/man1/
	$(INSTALL_DATA) kmymoney2.1.gz $(DESTDIR)$(mandir)/man1/kmymoney2.1.gz

kmymoney2.1.gz: kmymoney2.1
	gzip -9 -c -N $(top_srcdir)/$(subdir)/kmymoney2.1 > kmymoney2.1.gz

uninstall-hook: uninstall-docs
	-rm -f $(DESTDIR)$(mandir)/man1/kmymoney2.1.gz

#
# The following two rules are copied from an 'am_edit' generated
# version. Since we do not use the automatic generated version to
# avoid distribution of unwanted files, we add them here manually
#
install-docs: 
	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/$(KDE_LANG)/kmymoney2
	@if test -f index.cache.bz2; then \
	echo $(INSTALL_DATA) index.cache.bz2 $(DESTDIR)$(kde_htmldir)/$(KDE_LANG)/kmymoney2/; \
	$(INSTALL_DATA) index.cache.bz2 $(DESTDIR)$(kde_htmldir)/$(KDE_LANG)/kmymoney2/; \
	elif test -f  $(srcdir)/index.cache.bz2; then \
	echo $(INSTALL_DATA) $(srcdir)/index.cache.bz2 $(DESTDIR)$(kde_htmldir)/$(KDE_LANG)/kmymoney2/; \
	$(INSTALL_DATA) $(srcdir)/index.cache.bz2 $(DESTDIR)$(kde_htmldir)/$(KDE_LANG)/kmymoney2/; \
	fi
	-rm -f $(DESTDIR)$(kde_htmldir)/$(KDE_LANG)/kmymoney2/common
	$(LN_S) $(kde_libs_htmldir)/$(KDE_LANG)/common $(DESTDIR)$(kde_htmldir)/$(KDE_LANG)/kmymoney2/common

uninstall-docs:
	-rm -rf $(kde_htmldir)/$(KDE_LANG)/kmymoney2


