/***************************************************************************
                          kmymoneyaccountcompletion.h  -  description
                             -------------------
    begin                : Mon Apr 26 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#ifndef KMYMONEYACCOUNTCOMPLETION_H
#define KMYMONEYACCOUNTCOMPLETION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
class QListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountselector.h"
#include "../widgets/kmymoneycompletion.h"

/**
  * @author Thomas Baumgart
  */

class kMyMoneyAccountCompletion : public kMyMoneyCompletion
{
  Q_OBJECT
public:

  kMyMoneyAccountCompletion(QWidget *parent=0, const char *name=0);
  virtual ~kMyMoneyAccountCompletion();

  /**
    * Re-implemented for internal reasons.  API is unaffected.
    */
  virtual void show();

  /**
    * This method loads the set of accounts into the widget
    * as defined by the parameter @p accountIdList. @p accountIdList is
    * a QValueList of account ids.
    *
    * @param baseName QString which should be used as group text
    * @param accountIdList QValueList of QCString account ids
    *                 which should be loaded into the widget
    * @param clear if true (default) clears the widget before populating
    * @return This method returns the number of accounts loaded into the list
    */
  const int loadList(const QString& baseName, const QValueList<QCString>& accountIdList, const bool clear = true);

  /**
    * This method loads the set of accounts into the widget
    * as defined by the parameter @p typeList. @p typeList is
    * a list of ints representing different account types.
    * See MyMoneyAccount::accountTypeE for possible values.
    *
    * @param typeList QValueList conatining the account types to be displayed
    *
    * @return This method returns the number of accounts loaded into the list
    */
  const int loadList(QValueList<int> typeList);

  const int loadList(void) { return loadList(m_typeList); };

  /**
    * This method sets the current account with id @p id as
    * the current selection.
    *
    * @param id id of account to be selected
    */
  void setSelected(const QCString& id) { m_id = id; m_accountSelector->setSelected(id, true); };

  const QCStringList accountList(const QValueList<MyMoneyAccount::accountTypeE>& list = QValueList<MyMoneyAccount::accountTypeE>()) const { return m_accountSelector->accountList(list); };

  void removeAccount(const QCString& id) { m_accountSelector->removeAccount(id); }

  /**
    * This method returns the list of selected account id's. If
    * no account is selected, the list is empty.
    *
    * @return list of selected accounts
    */
  const QCStringList selectedAccounts(void) const { return m_accountSelector->selectedAccounts(); };

public slots:
  void slotMakeCompletion(const QString& txt);

private:
  kMyMoneyAccountSelector*      m_accountSelector;
  QValueList<int>               m_typeList;
  QValueList<QCString>          m_accountIdList;
  QString                       m_baseName;
};

#endif
