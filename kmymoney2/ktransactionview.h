/***************************************************************************
                          ktransactionview.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KTRANSACTIONVIEW_H
#define KTRANSACTIONVIEW_H

#include <qwidget.h>
#include <qlabel.h>

#include <klocale.h>
#include <klistview.h>

#include "mymoney/mymoneyfile.h"
#include "mymoney/mymoneybank.h"
#include "mymoney/mymoneyaccount.h"
#include "mymoney/mymoneytransaction.h"

#include "ktransactionviewdecl.h"

// This class handles the transaction 'view'.
// It handles the resize event, the totals widgets
// and the KTransactionListView itself
class KTransactionView : public KTransactionViewDecl  {
  Q_OBJECT
private:
  MyMoneyTransaction::transactionMethod m_focus;
  MyMoneyFile *m_filePointer;
  MyMoneyBank m_bankIndex;
  MyMoneyAccount m_accountIndex;
  long m_index;
  QList<MyMoneyTransaction> m_transactions;
  bool m_inEditMode;
//  MyMoneyTransaction *m_editTransaction;
  bool m_showingInputBox;

  void setInputData(const MyMoneyTransaction transaction);
  void updateInputLists(void);
  void updateTransactionList(int start, int col=-1);
  void viewMode(void);
  void editMode(void);

public:
	KTransactionView(QWidget *parent=0, const char *name=0);
	~KTransactionView();
//	MyMoneyTransaction currentTransaction(bool&);
	void init(MyMoneyFile *file, MyMoneyBank bank, MyMoneyAccount account);
	void clear(void);
  void showInputBox(bool val);
  void refresh(void);

protected slots:
  void enterClicked();
  void slotFocusChange(int row, int col, int button, const QPoint & mousePos);
  void slotTabSelected(QWidget *tab);
  void slotMajorCombo(const QString&);
  void transactionCellEdited(int row, int col);
  void editClicked();
  void cancelClicked();
  void newClicked();
  void slotTransactionDelete();
  void slotTransactionUnReconciled();
  void slotTransactionCleared();

signals:
//  void enterBtnClicked(const MyMoneyTransaction& , bool inNew);
  void transactionListChanged();
};

#endif
