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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpopupmenu.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneymoney.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneytransaction.h"
class kMyMoneyCategory;
class kMyMoneyEdit;
class kMyMoneyLineEdit;

#include "ksplittransactiondlgdecl.h"

/**
  * @author Thomas Baumgart
  * @todo Add account (hierarchy) upon new category
  */

class KSplitTransactionDlg : public kSplitTransactionDlgDecl  {
  Q_OBJECT

public: 
  KSplitTransactionDlg(const MyMoneyTransaction& t,
                       const MyMoneyAccount& acc,
                       const bool amountValid,
                       const bool deposit,
                       const MyMoneyMoney& calculatedValue = 0,
                       QWidget* parent = 0, const char* name = 0);

  virtual ~KSplitTransactionDlg();

  /**
    * Using this method, an external object can retrieve the result
    * of the dialog.
    *
    * @return MyMoneyTransaction based on the transaction passes during
    *         the construction of this object and modified using the
    *         dialog.
    */
  const MyMoneyTransaction& transaction(void) const { return m_transaction; };

protected:
  void resizeEvent(QResizeEvent*);

private:
  /**
    * This method sets the initial width for the amount field. The
    * width will be updated within updateSplit().
    */
  void initAmountWidth(void);

  /**
    * This method updates the display of the sums below the register
    */
  void updateSums(void);

  /**
    * This method creates the necessary input widgets in a specific row
    * of the register. They can be destroyed using destroyInputWidgets().
    *
    * @param row row of the register to place the widgets
    */
  void createInputWidgets(const unsigned row);

  /**
    * This method destroys all input widgets created with createInputWidgets().
    */
  void destroyInputWidgets(void);

  /**
    * This method resizes the transaction table depending on the visual size
    * and the number of entries. Updates m_numExtraLines.
    */
  void updateTransactionTableSize(void);

  /**
    * This method updates parts or all of the register. The area is controlled
    * via the parameters @p start and @p col.
    *
    * @param start row to be updated. -1 will update all rows. -1 is the default
    * @param col column to be updated. -1 will update all columns. -1 is the default
    */
  void updateSplit(int start = -1, int col = -1);

  /**
    * This method shows or hides the input widgets according to the argument
    * @p show. Uses createInputWidgets() and destroyInputWidgets() to dynamically
    * create and destroy the widgets. Updates m_editRow.
    *
    * @param row the row that should be updated
    * @param show true will create and show, false will hide and destroy the widgets
    */
  void showWidgets(int row, bool show = true);

  /**
    * This method hides and destroys the input widgets. It is provided for
    * convenience and calls showWidgets() to do the job.
    */
  void hideWidgets() { showWidgets(0, false); }

  /**
    * This method calculates the difference between the split that references
    * the account passed as argument to the constructor of this object and
    * all the other splits shown in the register of this dialog.
    *
    * @return difference as MyMoneyMoney object
    */
  MyMoneyMoney diffAmount(void);

  /**
    * This method calculates the sum of the splits shown in the register
    * of this dialog.
    *
    * @return sum of splits as MyMoneyMoney object
    */
  MyMoneyMoney splitsValue(void);

  /**
    * This method deletes a split referenced by the argument @p row.
    *
    * @param row index of the split to be deleted from the transaction
    */ 
  void deleteSplit(int row);

protected slots:
  void slotFinishClicked();
  void slotCancelClicked();
  void slotClearAllClicked();

  /// called upon mouse click, to see where to set the focus
  void slotFocusChange(int row, int col, int button, const QPoint & mousePos);

  /// move the selection bar around
  /// @param key the key that gives the direction
  void slotNavigationKey(int key);

  /**
    * This method is provided for convenience. It actually calls:
    *
    * slotStartEdit(transactionsTable->currentRow(), 0, Qt::LeftButton, QPoint(0, 0))
    */
  void slotStartEdit(void);

  /**
    * This method starts the edit phase of a split. It creates and preloads
    * the edit widgets with either the current selected split or an empty
    * split if a new one should be added.
    *
    * @param row The row of the table the widgets should be shown
    * @param col unused but provided to match the double-click signal
    * @param button The mouse button used
    * @param point The point, the click was issued
    */
  void slotStartEdit(int row, int col, int button, const QPoint&  point);

  /**
    * This methods stops editing a split and modifies/adds it
    * in/to the the transaction. This is supplied
    * for convenience as wrapper to slotEndEdit(int key).
    */
  void slotEndEdit(void) { slotEndEdit(Qt::Key_Down); }

  /**
    * This methods stops editing a split and modifies/adds it
    * in/to the the transaction. The argument @p key is used
    * to pass along an information, which direction should be taken
    * to select the next transaction. Possible interpreted values are:
    *
    * - Qt::Key_Up - select previoius split
    * - Qt::Key_Down - select next split
    *
    * @param key direction in which the next split should be selected
    */
  void slotEndEdit(int key);

  /**
    * This method stops editing the current split and destroys the widgets.
    * The previous values of this split will be loaded to the register again.
    */
  void slotQuitEdit(void);

  /**
    * This method ensures, that the user wants to delete the split
    * specified by @p row. It calls deleteSplit() to actually remove
    * the split from the transaction.
    *
    * @param row index of the split to be deleted into the list of splits
    */
  void slotDeleteSplit(int row);

  /**
    * This method is provided for convenience to delete the current
    * selected split. It calls slotDeleteSplit(int row) to do the job.
    */
  void slotDeleteSplit(void);

  /**
    * Called when the category field has been changed.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param name const reference to the name of the category
    */
  virtual void slotCategoryChanged(const QCString& id);
  // virtual void slotCategoryChanged(const QString& name);

  /**
    * Called when the amount field has been changed by the user.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param amount const reference to the amount value
    */
  virtual void slotAmountChanged(const QString& amount);

  /**
    * Called when the memo field has been changed.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param memo const reference to the new memo text
    */
  virtual void slotMemoChanged(const QString &memo);

private slots:
  /// used internally to setup the initial size of all widgets
  void initSize(void);

private:
  /**
    * This method returns a list of all splits not referencing the
    * current account (the one's that need to be displayed in the register
    * of the dialog).
    *
    * @return const QValueList<MyMoneySplit> containing all splits except the
    *               one that references the current account
    */
  const QValueList<MyMoneySplit> getSplits(void) const;

private:

  /**
    * This member keeps a copy of the current selected transaction
    */
  MyMoneyTransaction m_transaction;

  /**
    * This member keeps a copy of the currently edited split
    */
  MyMoneySplit m_split;

  /**
    * This member keeps a copy of the currently selected account
    */
  MyMoneyAccount m_account;

  /**
    * This member keeps the actual width required for the amount field
    */
  unsigned      m_amountWidth;

  /**
    * flag that shows that the amount specified in the constructor
    * should be used as fix value (true) or if it can be changed (false)
    */
  bool          m_amountValid;

  /**
    * This member contains the number of table rows that are required to fill
    * the widget if less splits are entered. This number is adjusted
    * during the execution of updateTransactionTableSize().
    */
  int   m_numExtraLines;

  /**
    * This member contains a pointer to the input widget for the category.
    * The widget will be created and destroyed dynamically in createInputWidgets()
    * and destroyInputWidgets().
    */
  kMyMoneyCategory* m_editCategory;

  /**
    * This member contains a pointer to the input widget for the memo.
    * The widget will be created and destroyed dynamically in createInputWidgets()
    * and destroyInputWidgets().
    */
  kMyMoneyLineEdit* m_editMemo;

  /**
    * This member contains a pointer to the input widget for the amount.
    * The widget will be created and destroyed dynamically in createInputWidgets()
    * and destroyInputWidgets().
    */
  kMyMoneyEdit*  m_editAmount;

  /**
    * The row that is currently edited. If -1, no row is selected
    */
  long m_editRow;

  /**
    * This member keeps a pointer to the context menu
    */
  KPopupMenu* m_contextMenu;

  /// keeps the id of the delete entry in the context menu
  int   m_contextMenuDelete;

  /**
    * This member keeps track if the current transaction is of type
    * deposit (true) or withdrawal (false).
    */
  bool  m_isDeposit;

  /**
    * This member keeps the amount that will be assigned to all the
    * splits that are marked 'will be calculated'.
    */
  MyMoneyMoney m_calculatedValue;
};

#endif
