/***************************************************************************
                          ksplittransactiondlg.h  -  description
                             -------------------
    begin                : Thu Jan 10 2002
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

#ifndef KSPLITTRANSACTIONDLG_H
#define KSPLITTRANSACTIONDLG_H

#include <kpopupmenu.h>
#include <kiconloader.h>

#include "../mymoney/mymoneymoney.h"
#include "../mymoney/mymoneysplittransaction.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneybank.h"
#include "../mymoney/mymoneyaccount.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"

#include "ksplittransactiondlgdecl.h"

/**
  *@author Thomas Baumgart
  */

class KSplitTransactionDlg : public kSplitTransactionDlgDecl  {
  Q_OBJECT

public: 
	KSplitTransactionDlg( QWidget* parent,  const char* name,
                        MyMoneyFile* const filePointer,
                        MyMoneyBank* const bankPointer,
                        MyMoneyAccount* const accountPointer,
                        MyMoneyMoney* amount, const bool amountValid = false);
	~KSplitTransactionDlg();

  /// get a pointer to the first split transaction
  /// @return pointer to the first MyMoneySplitTransaction in the list
  MyMoneySplitTransaction* firstTransaction(void);

  /// get a pointer to the next split transaction
  /// @return pointer to the next MyMoneySplitTransaction in the list
  MyMoneySplitTransaction* nextTransaction(void);

  /// add a MyMoneySplitTransaction to the dialog
  /// @param split pointer to split transaction
  void addTransaction(MyMoneySplitTransaction* const split);

protected:
  void resizeEvent(QResizeEvent*);

private:
  /// Setup initial width for the amount fields
  void initAmountWidth(void);

  /// Update the display of the sums
  void updateSums(void);

  /// create input widgets
  void createInputWidgets(void);

  /// resize the transaction table depending on the visual size
  /// and the number of entries. Updates m_numExtraLines.
  void updateTransactionTableSize(void);

  /// called when the transaction table needs to be updated
  ///
  /// @param start row to be updated. -1 (default) will update all rows
  /// @param col the column to be updated. -1 (default) will update all cols
  void updateTransactionList(int start = -1, int col = -1);

  /// updates a single transaction with the values of the input widgets
  void updateTransaction(MyMoneySplitTransaction *);

  /// enable (default) or disable input widgets
  /// @param show true will enable, false will disable the widgets
  void showWidgets(int row, bool show = true);

  /// disable input widgets (provided for convenience)
  void hideWidgets() { showWidgets(0, false); }

  /// calculate the difference to the given transaction amount (if any)
  /// @return the difference between sum of the splits to the transactions amount
  MyMoneyMoney diffAmount(void);

  /// calculate the sum of the splits
  /// @return the sum of all split transactions
  MyMoneyMoney splitsAmount(void);

  /// loads the lists required for input
  void updateInputLists(void);

  /// creates a new split transaction at the end of the list. The
  /// amount is updated to current difference value.
  /// @param row the row in the table that will hold the transaction
  void createSplitTransaction(int row);

  /// stop editing a transaction, enter it and skip the next startEdit event
  /// if the parameter is true. Do not skip that, if the parameter is false.
  /// @param skipNextStart provided to be able to trick the focus selection
  /// of the kMyMoneySplitTable. Default is false.
  void endEdit(bool skipNextStart = false);

  /// delete a split transaction from the list. No user interaction.
  /// @param row the row to be deleted
  void deleteSplitTransaction(int row);

protected slots:
  void slotFinishClicked();
  void slotCancelClicked();
  void slotClearAllClicked();

  /// called upon mouse click, to see where to set the focus
  void slotFocusChange(int row, int col, int button, const QPoint & mousePos);

  /// called upon double click on a line
  void slotStartEdit(int row, int col, int button, const QPoint&  point);

  /// move the selection bar around
  /// @param key the key that gives the direction
  void slotNavigationKey(int key);

  /// start editing a transaction
  void slotStartEdit(void);

  /// stop editing a transaction, enter it and skip the next startEdit event
  /// this is used to avoid automatic entry into the next transaction when
  /// editing the transaction is ended with a TAB key in the m_amount field.
  /// This is somewhat ugly, but I did not find another way to override the
  /// focus selection stuff.
  void slotEndEditTab(void) { endEdit(true); }

  /// stop editing a transaction and enter it. This is supplied
  /// for convenience as wrapper to slotEndEdit(bool)
  void slotEndEdit(void) { endEdit(false); }

  /// stop editing a transaction and discard it
  void slotQuitEdit(void);

  /// delete a split transaction from the list. The user
  /// will be asked if he wants to proceed.
  /// @param row the row to be deleted
  void slotDeleteSplitTransaction(int row);

  /// delete the currently selected transaction. Wrapper
  /// function for slotDeleteSplitTransaction(int row).
  void slotDeleteSplitTransaction(void);

private slots:
  /// used internally to setup the initial size of all widgets
  void initSize(void);

private:

  /// keeps a pointer to the file for global data retrieval
  MyMoneyFile*  const m_filePointer;

  /// keeps a pointer to the currently selected bank
  MyMoneyBank* const m_bankPointer;

  /// keeps a pointer to the currently selected account
  MyMoneyAccount* const m_accountPointer;

  /// keeps the actual width required for the amount field
  unsigned      m_amountWidth;

  /// the initial amount entered into the transaction register
  MyMoneyMoney* m_amountTransaction;

  /// flag if an amount for the transaction was specified
  bool          m_amountValid;

  /// the number of table rows that are required to fill
  /// the widget if less splits are entered. This number is adjusted
  /// during resize.
  int   m_numExtraLines;

  /// pointer input widget for category. the widget will be
  /// created in createInputWidgets()
  kMyMoneyCombo* m_category;

  /// pointer to input widget for memo. the widget will be
  /// created in createInputWidgets()
  kMyMoneyLineEdit* m_memo;

  /// pointer to input widget for amount. the widget will be
  /// created in createInputWidgets()
  kMyMoneyEdit*  m_amount;

  /// the dialog local list of splits
  QList<MyMoneySplitTransaction> m_splitList;

  /// flag that is set if a new split has been created and can
  /// be removed when discarded
  bool  m_createdNewSplit;

  /// flag that is set to skip the next call to StartEdit. This flag is set,
  /// when a transaction is left using the TAB key. The same TAB key event
  /// would otherwise start the next transaction.
  bool  m_skipStartEdit;

  /// pointer to the context menu
  KPopupMenu* m_contextMenu;

  /// keeps the id of the delete entry in the context menu
  int   m_contextMenuDelete;
};

#endif
