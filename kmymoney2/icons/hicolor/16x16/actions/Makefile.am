
LOCAL_DIR=kmymoney2/icons/hicolor/16x16/actions

FILES = bank.png categories.png pay_edit.png transaction_export.png transaction_find.png transaction_import.png reconcile.png backup.png close_window.png delete.png personal_data.png transaction.png view_info.png account_add.png account_open.png account.png account-type_asset.png account-type_liability.png account-type_expense.png account-type_income.png institution_add.png unreconciled.png

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/$(LOCAL_DIR)
	for file in $(FILES); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(kde_datadir)/$(LOCAL_DIR); \
	done


uninstall-local:
	for file in $(FILES); do \
	  rm -f $(kde_datadir)/$(LOCAL_DIR)/$$file; \
	done

