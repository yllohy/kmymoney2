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
#include "widgets/kmymoneymethodcombo.h"
#include "widgets/kmymoneypayeecombo.h"
#include "widgets/kmymoneylineedit.h"
#include "widgets/kmymoneycategorycombo.h"

#include "ktransactionviewdecl.h"

// This class handles the transaction 'view'.
// It handles the resize event, the totals widgets
// and the KTransactionListView itself
class KTransactionView : public KTransactionViewDecl  {
  Q_OBJECT
private:
  MyMoneyFile *m_filePointer;
  MyMoneyBank m_bankIndex;
  MyMoneyAccount m_accountIndex;
  long m_index;
  bool useall;
  bool usedate;
  bool userow;
	long lastcheck;


	kMyMoneyDateInput*  m_date;
	KComboBox* m_method;
	KLineEdit* m_number;
  QComboBox* m_payee;
  KLineEdit* m_payment;
	KLineEdit* m_withdrawal;
  KComboBox* m_category;
	KPushButton* m_enter;
  KPushButton* m_cancel;
	KPushButton* m_delete;

  QList<MyMoneyTransaction> m_transactions;

  void updateTransactionList(int start, int col=-1);

  void createInputWidgets();
	void loadPayees();
  void clearInputData();
  void setInputData(const MyMoneyTransaction transaction);
  void updateInputLists(void);
  /** No descriptions */
  MyMoneyAccount* getAccount();

public:
	KTransactionView(QWidget *parent=0, const char *name=0);
	~KTransactionView();

	void init(MyMoneyFile *file, MyMoneyBank bank, MyMoneyAccount account);
	void clear(void);
  void showInputBox(bool val);
  void refresh(void);

protected slots:
  void enterClicked();
  void slotFocusChange(int row, int col, int button, const QPoint & mousePos);
//  void editClicked();
  void cancelClicked();
  void deleteClicked();
  void slotTransactionDelete();
  void slotTransactionUnReconciled();
  void slotTransactionCleared();
	void slotPayeeCompleted();
	void slotMethodCompleted();

signals:
 void transactionListChanged();
};

#endif
