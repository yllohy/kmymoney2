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
#include <kpopupmenu.h>

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneytransaction.h"

#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneytable.h"

#include "ktransactionviewdecl.h"
#include "../widgets/kmymoneyhlayout.h"

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
  //MyMoneyFile *m_filePointer;
  //MyMoneyBank m_bankIndex;
  QCString m_accountIndex;
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

  unsigned m_debitWidth;
  unsigned m_creditWidth;
  unsigned m_balanceWidth;


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
  KPushButton* m_split;
	kMyMoneyLineEdit* m_memo;
  kMyMoneyHLayout*	m_hlayout;

  KPopupMenu* m_contextMenu;

  bool m_bEditingTransaction;

  /// keeps a copy of the transaction that will be edited
  /// and is used when the user rejects any modification (Cancel button)
  /// This member is set in setInputData(const MyMoneyTransaction) and
  /// used in cancelClicked().
  //MyMoneyTransaction m_originalTransaction;

  QMap<unsigned int, QCString> m_transactionMap;

  bool m_bSignals;

  /* Is it read only?
     This will be false for asset and liability accounts,
     true for income/expense accounts.
  */
  bool m_bReadOnly;

  void updateTransactionList(int start, int col=-1);

  void createInputWidgets();
	void loadPayees();
  void clearInputData();
  void setInputData(const QCString& transaction);
  void updateInputLists(void);
  /** No descriptions */
  MyMoneyAccount* getAccount();
  /** No descriptions */
  void hideWidgets();

  /** Need descriptions !*/
//  MyMoneyBank *getBank(void);
  /** Setup initial width for the amount fields */
  void initAmountWidth(void);

  /* These could go in MyMoneyUtils or something? */
  MyMoneyMoney transactionAmount(const QCString& trans);
  MyMoneyPayee transactionPayee(const QCString& trans);
  MyMoneyAccount transactionCategory(const QCString& trans);
  MyMoneySplit::reconcileFlagE transactionReconcileFlag(const QCString& trans);
  void setTransactionAmount(const QCString& trans, const MyMoneyMoney& amount);
  void setTransactionPayeeId(const QCString& trans, const QCString& payeeId);
  void setTransactionCategory(const QCString& trans, const QCString& accountId);
  void setTransactionReconcileFlag(const QCString& trans, MyMoneySplit::reconcileFlagE flag);
  bool transactionIsDebit(const QCString& trans);

public:
	KTransactionView(QWidget *parent=0, const char *name=0);
	~KTransactionView();

	void init(const QCString&, bool readOnly);
	void clear(void);
  void refresh(void);
  void show(void);
  void setSignals(bool enable);

protected:
  void resizeEvent(QResizeEvent*);

protected slots:
  void enterClicked();
  void slotFocusChange(int row, int col, int button, const QPoint & mousePos);
  void slotContextMenu(int row, int col, int button, const QPoint & mousePos);
  void slotEditSplit();
//  void editClicked();
  void cancelClicked();
  void deleteClicked();
  void slotTransactionDelete();
  void slotTransactionUnReconciled();
  void slotTransactionCleared();
	void slotPayeeCompleted();
	void slotMethodCompleted();
  void viewTypeActivated(int num);
  void slotCategoryActivated(int pos);

signals:
 void transactionListChanged();
  void viewTypeSearchActivated();
  void viewTypeNormalActivated();
  void signalViewActivated();

public slots: // Public slots
  /** No descriptions */
  void slotNextTransaction();
};

#endif
