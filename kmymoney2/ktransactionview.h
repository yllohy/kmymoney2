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
#include <qlayout.h>

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
#include "widgets/kmymoneylineedit.h"
#include "widgets/kmymoneycombo.h"
#include "widgets/kmymoneyedit.h"
#include "widgets/kmymoneytable.h"

#include "ktransactionviewdecl.h"
#include "widgets/kmymoneyhlayout.h"

// This class handles the transaction 'view'.
// It handles the resize event, the totals widgets
// and the KTransactionListView itself
class KTransactionView : public KTransactionViewDecl  {
  Q_OBJECT
public:
  enum viewingType { NORMAL, // All transactions view
          SUBSET // Sub set view
          };
private:
  MyMoneyFile *m_filePointer;
  MyMoneyBank m_bankIndex;
  MyMoneyAccount m_accountIndex;
  long m_index;
  bool useall;
  bool usedate;
  bool userow;
	long lastcheck;
	int m_currentcol;
	int m_currentrow;
    int m_currentbutton;
    QPoint m_currentpos;
  viewingType m_viewType;


	kMyMoneyDateInput*  m_date;
	kMyMoneyCombo* m_method;
	kMyMoneyLineEdit* m_number;
  kMyMoneyCombo* m_payee;
  kMyMoneyEdit* m_payment;
	kMyMoneyEdit* m_withdrawal;
  kMyMoneyCombo* m_category;
	KPushButton* m_enter;
  KPushButton* m_cancel;
	KPushButton* m_delete;
	kMyMoneyLineEdit* m_memo;
    kMyMoneyHLayout*	m_hlayout;

  QList<MyMoneyTransaction> *m_transactions;

  void updateTransactionList(int start, int col=-1);

  void createInputWidgets();
	void loadPayees();
  void clearInputData();
  void setInputData(const MyMoneyTransaction transaction);
  void updateInputLists(void);
  /** No descriptions */
  MyMoneyAccount* getAccount();
  /** No descriptions */
  void hideWidgets();

public:
	KTransactionView(QWidget *parent=0, const char *name=0);
	~KTransactionView();

	void init(MyMoneyFile *file, MyMoneyBank bank, MyMoneyAccount account, QList<MyMoneyTransaction> *transList, viewingType type);
	void clear(void);
  void refresh(void);

protected:
  void resizeEvent(QResizeEvent*);

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
  void viewTypeActivated(int num);

signals:
 void transactionListChanged();
  void viewTypeSearchActivated();
  void viewTypeNormalActivated();

public slots: // Public slots
  /** No descriptions */
  void slotNextTransaction();
};

#endif
