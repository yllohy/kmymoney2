KDE_OPTIONS = noautodist

DOCBOOK_FILES = credits.docbook details-accounts.docbook  details-categories.docbook details-files.docbook details-formats.docbook  details-hbciofx.docbook details-impexp.docbook details-institutions.docbook details-integrity.docbook details-investments.docbook details-ledgers.docbook details-loans.docbook details-payees.docbook details-reconciliation.docbook details-reports.docbook details-schedules.docbook details-search.docbook details-settings.docbook details-transactions.docbook details-widgets.docbook firsttime.docbook index.docbook introduction.docbook otherdoc.docbook views-accounts.docbook views-categories.docbook views-home.docbook views-investments.docbook views-ledgers.docbook views-outbox.docbook views-payees.docbook views-schedules.docbook views-institutions.docbook views-reports.docbook whatsnew.docbook details-currencies.docbook reference.docbook details-database.docbook makemostof.docbook details-budgets.docbook

SCREENSHOT_FILES = accountdropdown.png accounts_view.png accwiz1.png accwiz2.png accwiz3.png accwiz4.png accwiz5.png accwiz6.png calendarview.png categories.png categories_view.png categoryedit.png confirmenter.png default_prompt.png enterschedule.png finddlg.png home_view.png institutions_view.png investments_view.png kmt.png ledgers.png ledgers_view.png ledgerview-numbered.png mainwindow_numbered.png newfile.png newinst.png new.png newsched_numbered.png outbox_view.png payees_view.png payeeview.png qifopen.png reconcile.png register.png reports_view.png schedcaltypes.png schedpopup.png schedule.png schedule_view.png scheduleview.png tipofday.png transaction_find.png transactionform.png translist.png schedmonthday.png find-account.png find-amount.png find-category.png find-date.png find-details.png find-payee.png find-text.png payee_widget.png category_widget.png value_widget.png date_widget.png institution_view.png institution_options.png account_options.png account_edit.png payee_info.png payee_history.png split_transaction.png cleared_state.png transactionform-off.png ledger_more.png select_currency.png reconcile1.png reconcile2.png reconcile3.png reconcile4.png qif_report.png qifimportverify.png investment-transactionform.png investment-onlineupdate.png investment-currencywarning.png investment-exchangerateeditor.png gnucash-import_options.png gnucash-report.png gnucash-select_price_source.png  currency_newpriceentry.png currency_priceeditor.png investments_summarytab.png qifimport-export.png qifimport-qifprofiledate.png qifimport-qifprofileeditor.png report_configuration-reporttab.png report_configuration-rowscolumns.png reports_view-all.png select_database.png

EXTRA_DIST = kmymoney2.1 $(DOCBOOK_FILES)

MOSTLYCLEANFILES = index.cache.bz2 kmymoney2.1.gz index.xml *.aux *.tex *.log **.out *.pdf errorlog *.ps *.html online-manual.tar.bz2

BUILT_SOURCES = index.cache.bz2 kmymoney2.1.gz $(PDF_TARGET)

CHARSET = 
KDE_LANG = en
KDE_DOCS = kmymoney2 - $(DOCBOOK_FILES) $(SCREENSHOT_FILES)

index.cache.bz2: $(DOCBOOK_FILES) $(KDE_XSL_STYLESHEET) $(SCREENSHOT_FILES)
	if test -n "$(MEINPROC)"; then echo $(MEINPROC) --check --cache index.cache.bz2 --stylesheet $(KDE_XSL_STYLESHEET) $(srcdir)/index.docbook; $(MEINPROC) --check --cache index.cache.bz2 --stylesheet $(KDE_XSL_STYLESHEET) $(srcdir)/index.docbook; fi

index.html: 
	$(MEINPROC) --check --stylesheet `dirname $(KDE_XSL_STYLESHEET)`/kde-nochunk.xsl  $(srcdir)/index.docbook --stdout | sed "s/ISO-8859-1/UTF-8/g" > index.html

.PHONY: show preview get-files clear-files web
preview: index.html

show: preview
	konqueror index.html

web:
	if test x`which splitup-kde-chunk-online`x != xx; then \
		rm -rf online-manual common; \
		mkdir online-manual; \
		mkdir common; \
		$(MEINPROC) --check --stylesheet `dirname $(KDE_XSL_STYLESHEET)`/kde-chunk-online.xsl  $(srcdir)/index.docbook -o online-manual/index.xml; \
		cd online-manual; \
		splitup-kde-chunk-online; \
		cd ..; \
		make get-files; \
		for file in $(SCREENSHOT_FILES); do \
			cp $$file online-manual; \
		done; \
		make clear-files; \
		cp $(kde_libs_htmldir)/$(KDE_LANG)/common/* common; \
		tar -cjf online-manual.tar.bz2 common online-manual; \
		rm -rf index.xml online-manual common; \
	fi;

# if we're in a VPATH environment, we need to copy all files over
# to the build directory. Otherwise, we'll get into trouble. Of course
# we delete them afterwards.
get-files:
	if test -n "$(VPATH)" ; then \
		if test x$(VPATH) != x.; then \
			for file in $(DOCBOOK_FILES); do \
				cp $(srcdir)/$$file .; \
				if test -n $(CHARSET); then \
					recode utf-8..$(CHARSET) $$file; \
				fi \
			done; \
			for file in $(SCREENSHOT_FILES); do \
				cp $(srcdir)/$$file .; \
			done; \
		fi \
	fi

clear-files:
	if test -n "$(VPATH)" ; then \
		if test x$(VPATH) != x.; then \
			for file in $(DOCBOOK_FILES) $(SCREENSHOT_FILES); do rm $$file; done;  \
		fi \
	fi

if HAVE_DOCCONVERSIONTOOLS
PDF_TARGET = kmymoney-user.pdf

kmymoney-user.pdf: $(DOCBOOK_FILES) $(SCREENSHOT_FILES)
	$(MAKE) get-files
	@dblatexbin@ -b pdftex --style kdestyle -o kmymoney-user.pdf \
		-P latex.output.revhistory=0  -P newtbl.use=1 \
		-P imagedata.default.scale=pagebound \
		-P literal.width.ignore=1 \
		-I $(kde_libs_htmldir)/en \
		-X \
		index.docbook
	$(MAKE) clear-files
endif




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
	-rm -rf $(DESTDIR)$(kde_htmldir)/$(KDE_LANG)/kmymoney2


