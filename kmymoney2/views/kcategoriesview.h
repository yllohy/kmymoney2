/***************************************************************************
                          kcategoriesview.h  -  description
                             -------------------
    begin                : Sun Jan 20 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
                           (C) 2005 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KCATEGORIESVIEW_H
#define KCATEGORIESVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/kmymoneyaccounttree.h>
#include <kmymoney/mymoneyutils.h>

#include "kcategoriesviewdecl.h"

/**
  * @author Michael Edwardes, Thomas Baumgart
  */

class KCategoriesView : public KCategoriesViewDecl
{
  Q_OBJECT
public:
  KCategoriesView(QWidget *parent=0, const char *name=0);
  virtual ~KCategoriesView();


public slots:
  void slotLoadAccounts(void);

  /**
    * Override the base class behaviour to include all updates that
    * happened in the meantime.
    */
  void show(void);

protected:
  void loadAccounts(void);
  bool loadSubAccounts(KMyMoneyAccountTreeItem* parent, const QCStringList& accountList);

protected slots:
  void slotUpdateProfit(void);

private:
  /**
    * This method returns an icon according to the account type
    * passed in the argument @p type.
    *
    * @param type account type as defined in MyMoneyAccount::accountTypeE
    */
  const QPixmap accountImage(const MyMoneyAccount::accountTypeE type) const;

signals:
  /**
    * This signal serves as proxy for KMyMoneyAccountTree::selectObject()
    */
  void selectObject(const MyMoneyObject&);

  /**
    * This signal serves as proxy for
    * KMyMoneyAccountTree::openContextMenu(const MyMoneyObject&)
    */
  void openContextMenu(const MyMoneyObject& obj);

  /**
    * This signal will be emitted when the left mouse button is double
    * clicked (actually the KDE executed setting is used) on an account.
    */
  void openObject(const MyMoneyObject& obj);

  /**
    * This signal is emitted, when the user selected to reparent the
    * account @p acc to be a subordinate account of @p parent.
    *
    * @param acc const reference to account to be reparented
    * @param parent const reference to new parent account
    */
  void reparent(const MyMoneyAccount& acc, const MyMoneyAccount& parent);

private:
  QMap<QCString, MyMoneyAccount>      m_accountMap;
  QMap<QCString, MyMoneySecurity>     m_securityMap;
  QMap<QCString, unsigned long>       m_transactionCountMap;

  KMyMoneyAccountTreeItem*            m_incomeItem;
  KMyMoneyAccountTreeItem*            m_expenseItem;

  /// set if a view needs to be reloaded during show()
  bool                                m_needReload;
};

#endif
