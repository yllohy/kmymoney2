/***************************************************************************
                          knewaccountdlg.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWACCOUNTDLG_H
#define KNEWACCOUNTDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qcstring.h>

// ----------------------------------------------------------------------------
// KDE Headers

#ifdef HAVE_KDCHART
#include <KDChartWidget.h>
#include <KDChartTable.h>
#include <KDChartParams.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneymoney.h>
#include <kmymoney/kmymoneyedit.h>

#include "../dialogs/knewaccountdlgdecl.h"
class KMyMoneyAccountTreeItem;
namespace reports {
  class KReportChartView;
};

/**
  * This dialog lets you create/edit an account.
  */
class KNewAccountDlg : public KNewAccountDlgDecl
{
  Q_OBJECT

private:
  MyMoneyAccount m_account;
  MyMoneyAccount m_parentAccount;
  bool m_bSelectedParentAccount;

  KMyMoneyAccountTreeItem *m_parentItem;
  KMyMoneyAccountTreeItem *m_accountItem;
  bool m_categoryEditor;
  bool m_isEditing;

  int m_idPropFutureValue;
  int m_idPropLastValue;
  int m_idPropMaxCredit;
  int m_idPropMinBalance;

  void initParentWidget(QCString parentId, const QCString& accountId);
  void showSubAccounts(QCStringList accounts, KMyMoneyAccountTreeItem *parentItem, const QCString& parentId, const QCString& accountId);
  void loadVatAccounts(void);
  void storeKVP(const QCString& key, kMyMoneyEdit* widget);
  void storeKVP(const QCString& key, KLineEdit* widget);
  void storeKVP(const QCString& key, const QString& text, const QString& value);
  void loadKVP(const QCString& key, kMyMoneyEdit* widget);
  void loadKVP(const QCString& key, KLineEdit* widget);

public:
  /**
    * This is the constructor of the dialog. The parameters define the environment
    * in which the dialog will be used. Depending on the environment, certain rules
    * apply and will be handled by the dialog.
    *
    * @param account The original data to be used to create the account. In case
    *                of @p isEditing is false, the account id, the parent account id
    *                and the list of all child accounts will be cleared.
    * @param isEditing If @p false, rules for new account creation apply.
    *                  If @p true, rules for account editing apply
    * @param categoryEditor If @p false, rules for asset/liability accounts apply.
    *                       If @p true, rules for income/expense account apply.
    * @param parent Pointer to parent object (passed to QDialog). Default is 0.
    * @param name Name of the object (passed to QDialog). Default is 0.
    * @param title Caption of the object (passed to QDialog). Default is empty string.
    */
  KNewAccountDlg(const MyMoneyAccount& account, bool isEditing, bool categoryEditor, QWidget *parent=0, const char *name=0, const QString& title=QString::null);

  /**
    * This method returns the edited account object.
    */
  const MyMoneyAccount& account(void);

  /**
    * This method returns the parent account of the edited account object.
    */
  const MyMoneyAccount& parentAccount(void);

  void setOpeningBalance(const MyMoneyMoney& balance);

  const MyMoneyMoney openingBalance(void) const { return m_openingBalanceEdit->value(); };

  void setOpeningBalanceShown(bool shown);

  /**
   * This method adds an additional tab pointed to with @a w to the tab widget.
   * This tab is usually defined by a plugin (eg. online banking). If @a w is
   * zero, this is a NOP. @a name is used as the text to be placed on the tab.
   */
  void addTab(QWidget* w, const QString& name);

protected:
  void resizeEvent(QResizeEvent* e);
  void displayOnlineBankingStatus(void);
  void adjustEditWidgets(kMyMoneyEdit* dst, kMyMoneyEdit* src, char mode, int corr);

protected slots:
  void okClicked();
  void slotSelectionChanged(QListViewItem *item);
  void slotAccountTypeChanged(const QString& type);
  void slotVatChanged(bool);
  void slotVatAssignmentChanged(bool);
  void slotNewClicked(void);
  void slotCheckFinished(void);
  void slotLoadInstitutions(const QString&);
  void slotAdjustMinBalanceAbsoluteEdit(const QString&);
  void slotAdjustMinBalanceEarlyEdit(const QString&);
  void slotAdjustMaxCreditAbsoluteEdit(const QString&);
  void slotAdjustMaxCreditEarlyEdit(const QString&);

private slots:
  void timerDone(void);

};

#endif

