/***************************************************************************
                          kmainview.h
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

#ifndef KMAINVIEW_H
#define KMAINVIEW_H

#include <qwidget.h>

#include "kbanksview.h"
//#include "kaccountsview.h"
#include "ktransactionview.h"

// This class handles the tabs.
class KMainView : public QWidget  {
   Q_OBJECT

public:
  enum viewType { None, BankList, TransactionList };

private:
  KBanksView *banksView;
  KTransactionView *transactionView;
  viewType m_showing;

public: 
	KMainView(QWidget *parent=0, const char *name=0);
	~KMainView();
	void clear(void);
	void refreshBankView(MyMoneyFile file);
	void refreshTransactionView(void);
	MyMoneyBank currentBank(bool& success);
	MyMoneyAccount currentAccount(bool& success);
//	MyMoneyTransaction currentTransaction(bool& success);
//  void switchInputFocus(MyMoneyTransaction::transactionMethod method);
//  void setInputDataRead(const MyMoneyTransaction trans);
//  void setInputDataWrite(const MyMoneyTransaction trans);
  void viewBankList(void);
  void viewTransactionList(void);
  viewType viewing(void) { return m_showing; }
  void showInputBox(bool val);
  void initTransactionView(MyMoneyFile *file, const MyMoneyBank bank, const MyMoneyAccount account,
    QList<MyMoneyTransaction> *theList,
    KTransactionView::viewingType type
    );
  /** No descriptions */
  KTransactionView* getTransactionView();
	
protected:
  void resizeEvent(QResizeEvent *e);

protected slots:
  void slotBRightMouseClick(const MyMoneyBank, bool);
  void slotARightMouseClick(const MyMoneyAccount, bool);
//  void slotEnterPressed(const MyMoneyTransaction& trans, bool);
  void slotTransactionListChanged();
  void slotBankSelected();
  void slotAccountSelected();

signals:
  void bankRightMouseClick(const MyMoneyBank, bool);
  void accountRightMouseClick(const MyMoneyAccount, bool);
//  void enterBtnClicked(const MyMoneyTransaction&, bool);
  void transactionListChanged();
  void bankSelected();
  void accountSelected();
};

#endif
