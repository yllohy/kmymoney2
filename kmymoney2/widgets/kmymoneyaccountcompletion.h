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
    * as defined by the parameter @p typeMask. @p typeMask is
    * a bit mask. See KMyMoneyUtils::categoryTypeE for
    * possible values.
    *
    * If multiple sets should be displayed, several KMyMoneyUtils::categoryTypeE values
    * can be logically OR-ed.
    *
    * @param typeMask bitmask defining which types of accounts
    *                 should be loaded into the completion list
    * @return This method returns the number of accounts loaded into the list
    */
  int loadList(KMyMoneyUtils::categoryTypeE typeMask) { m_accountType = typeMask; return m_accountSelector->loadList(typeMask); };

  int loadList(void) { return loadList(m_accountType); };

  /**
    * This method sets the current account with id @p id as
    * the current selection.
    *
    * @param id id of account to be selected
    */
  void setSelected(const QCString& id) { m_id = id; };

  const QCStringList accountList(const QValueList<MyMoneyAccount::accountTypeE>& list = QValueList<MyMoneyAccount::accountTypeE>()) const { return m_accountSelector->accountList(list); };

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
  KMyMoneyUtils::categoryTypeE  m_accountType;
};

#endif
