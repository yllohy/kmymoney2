/***************************************************************************
                          ktransactionview.h
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

#ifndef KTRANSACTIONVIEW_H
#define KTRANSACTIONVIEW_H

#include <qwidget.h>
#include <qlabel.h>

#include <kpushbutton.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <klistview.h>

#include "mymoney/mymoneyfile.h"
#include "mymoney/mymoneybank.h"
#include "mymoney/mymoneyaccount.h"
#include "mymoney/mymoneytransaction.h"

#include "widgets/kmymoneydateinput.h"

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

	kMyMoneyDateInput*  m_date;
	KComboBox* m_method;
  KComboBox* m_payee;
  KLineEdit* m_payment;
	KLineEdit* m_withdrawal;
  KComboBox* m_category;
	KPushButton* m_enter;
  KPushButton* m_cancel;
	KPushButton* m_delete;

  QList<MyMoneyTransaction> m_transactions;
  bool m_inEditMode;
//  MyMoneyTransaction *m_editTransaction;
  bool m_showingInputBox;

  void setInputData(const MyMoneyTransaction transaction);
  void clearInputData();
  void updateInputLists(void);
  void updateTransactionList(int start, int col=-1);
  void viewMode(void);
  void editMode(void);

  void createInputWidgets();
	void loadPayees();

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
  void slotMajorCombo(const QString&);
  void transactionCellEdited(int row, int col);
  void editClicked();
  void cancelClicked();
  void newClicked();
  void deleteClicked();
  void slotTransactionDelete();
  void slotTransactionUnReconciled();
  void slotTransactionCleared();

signals:
//  void enterBtnClicked(const MyMoneyTransaction& , bool inNew);
  void transactionListChanged();
};

#endif
