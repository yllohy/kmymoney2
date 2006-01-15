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

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneymoney.h>

#include "../dialogs/knewaccountdlgdecl.h"
class KMyMoneyAccountTreeItem;

/**
  * This dialog lets you create/edit an account.
  */
class KNewAccountDlg : public KNewAccountDlgDecl  {
   Q_OBJECT

private:
  MyMoneyAccount m_account;
  MyMoneyAccount m_parentAccount;
  bool m_bSelectedParentAccount;

  KMyMoneyAccountTreeItem *m_parentItem;
  KMyMoneyAccountTreeItem *m_accountItem;
  bool m_categoryEditor;
  bool m_isEditing;

  void initParentWidget(QCString parentId, const QCString& accountId);
  void showSubAccounts(QCStringList accounts, KMyMoneyAccountTreeItem *parentItem, const QCString& parentId, const QCString& accountId);
  void loadInstitutions(const QString&);
  void loadVatAccounts(void);

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
    * @param title Caption of the object (passed to QDialog). Default is 0.
    */
  KNewAccountDlg(const MyMoneyAccount& account, bool isEditing, bool categoryEditor, QWidget *parent=0, const char *name=0, const char *title=0);
  ~KNewAccountDlg();

  /**
    * This method returns the edited account object.
    */
  const MyMoneyAccount& account(void);

  /**
    * This method returns the parent account of the edited account object.
    */
  const MyMoneyAccount& parentAccount(void);

protected:
  void resizeEvent(QResizeEvent* e);
  void displayOnlineBankingStatus(void);

protected slots:
  void okClicked();
  void slotSelectionChanged(QListViewItem *item);
  void slotAccountTypeChanged(const QString& type);
  void slotVatChanged(bool);
  void slotVatAssignmentChanged(bool);
  void slotNewClicked();
  void slotCheckFinished(void);
  void slotOnlineSetupClicked();

private slots:
  void timerDone(void);

};

#endif

