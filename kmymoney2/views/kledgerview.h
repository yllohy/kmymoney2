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
#include "../widgets/kmymoneytransactionform.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneydateinput.h"

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

  /**
    * This method is used to convert a string used in the
    * context of the user interface to an action string
    * used in the mymoney engine
    *
    * @param action QString reference of action string
    * @return QCString with normalized action string
    */
  const QCString str2action(const QString& action) const;

  /**
    * This method is used to convert an action string used
    * within the engine to a form usable on the user interface.
    *
    * @param action QCString reference to normalized action string
    * @return QString with translated action string
    */
  const QString action2str(const QCString& action) const;

  /**
    * This method is called to fill the transaction form
    * with the data of the currently selected transaction
    * in m_register. It must be overridden by any derived
    * class.
    */
  virtual void fillForm(void) = 0;

public slots:
  /**
    * refresh the current view
    */
  virtual void refreshView(void);
  void slotRegisterClicked(int row, int col, int button, const QPoint &mousePos);

  /**
    * Called when the user changes the visibility
    * setting of the transaction form
    */
  void slotShowTransactionForm(bool show);

  /**
    * Called when a new payee entry has been edited
    * This routine will call the payee dialog and possibly add
    * the payee to the MyMoneyFile object
    */
  void slotNewPayee(const QString& payee);

protected:
  void loadAccount(void);
  void filterTransactions(void);
  void sortTransactions(void);

protected:
  kMyMoneyRegister *m_register;
  kMyMoneyTransactionForm *m_form;

  QDate m_dateStart;
  MyMoneyAccount m_account;

  QValueList<MyMoneyTransaction> m_transactionList;
  QValueVector<MyMoneyMoney> m_balance;

  KTransactionPtrVector m_transactionPtrVector;
  MyMoneyTransaction *m_transactionPtr;
  MyMoneyTransaction m_transaction;

  MyMoneySplit m_split;

  kMyMoneyPayee*        m_editPayee;
  kMyMoneyCategory*     m_editCategory;
  kMyMoneyLineEdit*     m_editMemo;
  kMyMoneyEdit*         m_editAmount;
  kMyMoneyLineEdit*     m_editNr;
  kMyMoneyDateInput*    m_editDate;

signals:
  void transactionSelected(void);

};

#endif
