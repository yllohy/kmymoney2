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

#include <qdialog.h>
#include <klocale.h>

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneymoney.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"

#include "knewaccountdlgdecl.h"
#include "../views/kbanklistitem.h"
#include <qcstring.h>

// This dialog lets you create/edit an account.
// Use the second constructor to edit an account.
class KNewAccountDlg : public KNewAccountDlgDecl  {
   Q_OBJECT

private:
  MyMoneyAccount m_account;
  MyMoneyAccount m_parentAccount;
  MyMoneyFile *m_file;
  bool m_bSelectedParentAccount;
  KAccountListItem *m_foundItem;
  bool m_bFoundItem;

  void initParentWidget(const QString&);
  void showSubAccounts(QCStringList accounts, KAccountListItem *parentItem, MyMoneyFile *file, const QString&);
  void loadInstitutions(const QString&);

public:
	KNewAccountDlg(MyMoneyAccount& account, MyMoneyFile* file, bool isEditing, QWidget *parent=0, const char *name=0, const char *title=0);
	~KNewAccountDlg();
  MyMoneyAccount account(void);
  const MyMoneyAccount parentAccount(void);

protected:
  void resizeEvent(QResizeEvent* e);

protected slots:
  void okClicked();
  void slotSelectionChanged(QListViewItem *item);
  void slotSubAccountsToggled(bool on);
  void slotNewClicked();
};

#endif

