/***************************************************************************
                          kledgerview.h  -  description
                             -------------------
    begin                : Sun Jul 14 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

#ifndef KLEDGERVIEW_H
#define KLEDGERVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qtable.h>
#include <qvaluelist.h>
#include <qvaluevector.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../widgets/kmymoneyregister.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneytransaction.h"
#include "../mymoney/mymoneyobserver.h"

class kMyMoneyRegister;

/**
  *@author Thomas Baumgart
  */

class KTransactionPtrVector : public QPtrVector<MyMoneyTransaction> {
public:
  enum TransactionSortE {
    SortEntryDate = 0,
    SortPostDate,
  };

  KTransactionPtrVector() { m_sortType = SortPostDate; };
  ~KTransactionPtrVector() {};

protected:
  int compareItems(KTransactionPtrVector::Item d1, KTransactionPtrVector::Item d2);

private:
  TransactionSortE  m_sortType;
};

class KLedgerView : public QWidget, MyMoneyObserver  {
   Q_OBJECT
public: 
	KLedgerView(QWidget *parent=0, const char *name=0);
	virtual ~KLedgerView();

  void setCurrentAccount(const QCString& accountId);
  void update(const QCString& accountId);

  MyMoneyTransaction* const transaction(const int idx) const;
  const MyMoneyMoney& balance(const int idx) const;
  const QCString accountId(void) { return m_account.id(); }

public slots:
  /**
    * refresh the current view
    */
  virtual void refreshView(void);

protected:
  void loadAccount(void);
  void filterTransactions(void);
  void sortTransactions(void);

protected:
  QDate m_dateStart;
  kMyMoneyRegister *m_register;
  MyMoneyAccount m_account;
  QValueList<MyMoneyTransaction> m_transactionList;
  QValueVector<MyMoneyMoney> m_balance;

  KTransactionPtrVector m_transactionPtr;
};

#endif
