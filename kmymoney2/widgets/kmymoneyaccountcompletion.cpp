/***************************************************************************
                          kmymoneyaccountcompletion.cpp  -  description
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


#include "kmymoneyaccountcompletion.h"

#define MAX_ITEMS   16

kMyMoneyAccountCompletion::kMyMoneyAccountCompletion(QWidget *parent, const char *name ) :
  kMyMoneyCompletion(parent, name)
{
  // Default is to show all accounts
  m_typeList << MyMoneyAccount::Checkings;
  m_typeList << MyMoneyAccount::Savings;
  m_typeList << MyMoneyAccount::Cash;
  m_typeList << MyMoneyAccount::AssetLoan;
  m_typeList << MyMoneyAccount::CertificateDep;
  m_typeList << MyMoneyAccount::Investment;
  m_typeList << MyMoneyAccount::MoneyMarket;
  m_typeList << MyMoneyAccount::Asset;
  m_typeList << MyMoneyAccount::Currency;
  m_typeList << MyMoneyAccount::CreditCard;
  m_typeList << MyMoneyAccount::Loan;
  m_typeList << MyMoneyAccount::Liability;
  m_typeList << MyMoneyAccount::Income;
  m_typeList << MyMoneyAccount::Expense;

  m_accountSelector = new kMyMoneyAccountSelector(this, 0, 0, false);

  connectSignals(static_cast<QWidget*> (m_accountSelector), m_accountSelector->listView());
}

kMyMoneyAccountCompletion::~kMyMoneyAccountCompletion()
{
}

const int kMyMoneyAccountCompletion::loadList(const QString& baseName, const QValueList<QCString>& accountIdList, const bool clear)
{
  m_typeList.clear();
  m_baseName = baseName;
  m_accountIdList = accountIdList;

  return m_accountSelector->loadList(baseName, accountIdList, clear);
}

void kMyMoneyAccountCompletion::show(void)
{
  int  count;

  if(m_typeList.isEmpty()) {
    count = loadList(m_baseName, m_accountIdList);
  } else {
    count = loadList(m_typeList);

    // make sure we increase the count by the account groups
    if((m_typeList.contains(MyMoneyAccount::Checkings)
      + m_typeList.contains(MyMoneyAccount::Savings)
      + m_typeList.contains(MyMoneyAccount::Cash)
      + m_typeList.contains(MyMoneyAccount::AssetLoan)
      + m_typeList.contains(MyMoneyAccount::CertificateDep)
      + m_typeList.contains(MyMoneyAccount::Investment)
      + m_typeList.contains(MyMoneyAccount::MoneyMarket)
      + m_typeList.contains(MyMoneyAccount::Asset)
      + m_typeList.contains(MyMoneyAccount::Currency)) > 0)
      ++count;

    if((m_typeList.contains(MyMoneyAccount::CreditCard)
      + m_typeList.contains(MyMoneyAccount::Loan)
      + m_typeList.contains(MyMoneyAccount::Liability)) > 0)
      ++count;

    if((m_typeList.contains(MyMoneyAccount::Income)) > 0)
      ++count;

    if((m_typeList.contains(MyMoneyAccount::Expense)) > 0)
      ++count;

  }
  if(!m_id.isEmpty())
    m_accountSelector->setSelected(m_id);

  adjustSize(count);

  kMyMoneyCompletion::show();
}

void kMyMoneyAccountCompletion::slotMakeCompletion(const QString& txt)
{
  // if(txt.isEmpty() || txt.length() == 0)
  //  return;

  QString account(txt);
  int pos = txt.findRev(':');
  if(pos != -1) {
    account = txt.mid(pos+1);
  }

  if(m_parent && m_parent->isVisible() && !isVisible())
    show();

  int count = m_accountSelector->slotMakeCompletion(account);

  if(count != 0) {
    // don't forget the four group lines
    adjustSize(count+4);
  } else {
    hide();
  }
}

