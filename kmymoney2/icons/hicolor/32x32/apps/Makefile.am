
LOCAL_DIR = kmymoney2/icons/hicolor/32x32/apps

FILES = accounts.png account-types_asset.png account-types_cash.png account-types_checking.png account-types_credit-card.png account-types_investments.png account-types_liability.png account-types_loan.png account-types_savings.png categories.png home.png institutions.png investments.png ledger.png payee.png report.png schedule.png onlinebanking.png budget.png account-types_reconciled.png

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)
	for file in $(FILES); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR); \
	done


uninstall-local:
	for file in $(FILES); do \
	  rm -f $(DESTDIR)$(kde_datadir)/$(LOCAL_DIR)/$$file; \
	done


