/***************************************************************************
                          kpayeesview.h  -  description
                             -------------------
    begin                : Thu Jan 24 2002
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

#ifndef KPAYEESVIEW_H
#define KPAYEESVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kpayeesviewdecl.h"
#include "kledgerview.h"
#include "../mymoney/mymoneyobserver.h"
#include "../mymoney/mymoneypayee.h"

/**
  *@author Michael Edwardes
  */

class KPayeesView : public KPayeesViewDecl, MyMoneyObserver
{
   Q_OBJECT
public: 
  KPayeesView(QWidget *parent=0, const char *name=0);
  ~KPayeesView();
  void show();
  void update(const QCString &id);
  void refreshView(void) { refresh(); };

  /**
    * This method is used to suppress updates for specific times
    * (e.g. during creation of a new MyMoneyFile object when the
    * default accounts are loaded). The behaviour of update() is
    * controlled with the parameter.
    *
    * @param suspend Suspend updates or not. Possible values are
    *
    * @li true updates are suspended
    * @li false updates will be performed immediately
    *
    * When a true/false transition of the parameter between
    * calls to this method is detected,
    * refresh() will be invoked once automatically.
    */
  void suspendUpdate(const bool suspend);

public slots:
  void slotSelectPayeeAndTransaction(const QCString& payeeId, const QCString& accountId, const QCString& transactionId);

protected:
  void resizeEvent(QResizeEvent*);

protected slots:
  /**
    * This method loads the m_transactionList, clears
    * the m_TransactionPtrVector and rebuilds and sorts
    * it according to the current settings. Then it
    * loads the m_transactionView with the transaction data.
    */
  void showTransactions(void);

  void payeeHighlighted(const QString&);
  void slotAddClicked();
  void slotPayeeTextChanged(const QString& text);
  void slotUpdateClicked();
  void slotDeleteClicked();

  void slotTransactionDoubleClicked(QListViewItem *);

private slots:
  void rearrange(void);

private:
  void readConfig(void);
  void writeConfig(void);

  void refresh(void);

signals:
  void signalViewActivated();
  void transactionSelected(const QCString& accountId, const QCString& transactionId);

private:
  MyMoneyPayee m_payee;
  QString m_lastPayee;

  /**
    * This member holds a list of all transactions
    */
  QValueList<MyMoneyTransaction> m_transactionList;

  /**
    * This member keeps a vector of pointers to all visible (filtered)
    * transaction in m_transactionList in sorted order. Sorting is done
    * in KTransactionPtrVector::compareItems
    */
  KTransactionPtrVector m_transactionPtrVector;

  /**
    * This member holds the state of the toggle switch used
    * to suppress updates due to MyMoney engine data changes
    */
  bool m_suspendUpdate;

};

#endif
