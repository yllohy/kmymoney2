
LOCAL_DIR=kmymoney2/icons/hicolor/22x22/actions

FILES = bank.png categories.png pay_edit.png transaction_export.png transaction_find.png transaction_import.png reconcile.png backup.png close_window.png delete.png personal_data.png transaction.png view_info.png account_add.png account_open.png account.png account-types_asset.png account-types_liability.png account-types_expense.png account-types_income.png institution_add.png unreconciled.png

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)
	for file in $(FILES); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR); \
	done


uninstall-local:
	for file in $(FILES); do \
	  rm -f $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)/$$file; \
	done

