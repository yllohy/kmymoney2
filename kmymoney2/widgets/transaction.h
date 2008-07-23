/***************************************************************************
                          transaction.h  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TRANSACTION_H
#define TRANSACTION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qpalette.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/registeritem.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/mymoneysplit.h>
#include <kmymoney/mymoneysecurity.h>
#include <kmymoney/selectedtransaction.h>
#include <kmymoney/mymoneyaccount.h>

class QTable;
class TransactionEditor;
class TransactionEditorContainer;

namespace KMyMoneyTransactionForm {
  class TransactionForm;
}; // namespace

namespace KMyMoneyRegister {

// keep the following list in sync with code in the constructor
// of KMyMoneyRegister::Register in register.cpp
typedef enum {
  NumberColumn = 0,
  DateColumn,
  AccountColumn,
  SecurityColumn,
  DetailColumn,
  ReconcileFlagColumn,
  PaymentColumn,
  DepositColumn,
  QuantityColumn,
  PriceColumn,
  ValueColumn,
  BalanceColumn,
  // insert new values above this line
  MaxColumns
} Column;

class Transaction : public RegisterItem
{
public:
  Transaction(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~Transaction() {}

  virtual const char* className(void) { return "Transaction"; }

  bool isSelectable(void) const { return true; }
  bool isSelected(void) const { return m_selected; }
  void setSelected(bool selected);

  bool canHaveFocus(void) const { return true; }
  bool hasFocus(void) const { return m_focus; }
  bool hasEditorOpen(void) const { return m_inEdit; }

  void setFocus(bool focus, bool updateLens = true);

  bool isErronous(void) const { return m_erronous; }

  virtual const QDate& sortPostDate(void) const { return m_transaction.postDate(); }
  virtual const QDate& sortEntryDate(void) const { return m_transaction.entryDate(); }
  virtual const QString& sortPayee(void) const { return m_payee; }
  virtual const MyMoneyMoney& sortValue(void) const { return m_split.shares(); }
  virtual const QString& sortNumber(void) const { return m_split.number(); }
  virtual const QCString& sortEntryOrder(void) const { return m_uniqueId; }
  virtual CashFlowDirection sortType(void) const { return m_split.shares().isPositive() ? Deposit : Payment; }
  virtual const QString& sortCategory(void) const { return m_category; }
  virtual MyMoneySplit::reconcileFlagE sortReconcileState(void) const { return m_split.reconcileFlag(); }

  virtual const QCString& id(void) const { return m_uniqueId; }
  const MyMoneyTransaction& transaction(void) const { return m_transaction; }
  const MyMoneySplit& split(void) const { return m_split; }

  void setBalance(const QString& balance) { m_balance = balance; }
  const QString& balance(void) const { return m_balance; }

  virtual int rowHeightHint(void) const;

  /**
    * This method sets the general paramaters required for the painting of a cell
    * in the register. These are:
    *
    * - background color (alternating)
    * - background color (imported transaction)
    * - background color (matched transaction)
    * - background color (selected transaction)
    * - cellRect (area covering the cell)
    * - textRect (area covering the text)
    * - color of the pen to do the painting of text and lines
    *
    * @param painter pointer to the QPainter object
    * @param row vertical index of cell in register
    * @param col horizontal index of cell in register
    * @param cellRect ref to QRect object receiving the area information for the cell
    * @param textRect ref to QRect object receiving the area information for the text
    * @param cg ref to QColorGroup object receiving the color information to be used
    */
  virtual bool paintRegisterCellSetup(QPainter* painter, int& row, int& col, QRect& cellRect, QRect& textRect, QColorGroup& cg, QBrush& brush);

  /**
    * paints the focus if the current cell defined by (@a row, @a col) has the focus.
    *
    * @param painter pointer to the QPainter object
    * @param row vertical index of cell in register
    * @param col horizontal index of cell in register
    * @param r area covering the cell
    * @param cg the color definitions to be used
    */
  void paintRegisterCellFocus(QPainter* painter, int row, int col, const QRect& r, const QColorGroup& cg);

  /**
    * paints a cell of the register for the transaction. Uses paintRegisterCellSetup(), paintRegisterCellText()
    * paintRegisterGrid() and paintRegisterCellFocus() to actually do the job.
    *
    * @param painter pointer to the QPainter object
    * @param row vertical index of cell in register
    * @param col horizontal index of cell in register
    * @param r area covering the cell
    * @param selected unused but kept for compatibility
    * @param cg the color definitions to be used
    *
    */
  virtual void paintRegisterCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);
  virtual void paintRegisterCellText(QPainter* painter, int row, int col, const QRect& r, const QColorGroup& cg, int align, const QString& txt);
  virtual void paintRegisterCellBackground(QPainter* painter, int row, int col, const QRect& r, const QBrush& backgroundBrush);
  virtual void paintRegisterGrid(QPainter* painter, int row, int col, const QRect& r, const QColorGroup& cg) const;

  virtual void paintFormCell(QPainter* /* painter */, int /* row */, int /* col */, const QRect& /* r */, bool /* selected */, const QColorGroup& /* cg */);

  virtual bool formCellText(QString& /* txt */, int& /* align */, int /* row */, int /* col */, QPainter* /* painter */) { return false; }
  virtual void registerCellText(QString& /* txt */, int& /* align */, int /* row */, int /* col */, QPainter* /* painter */) {}
  virtual int registerColWidth(int /* col */, const QFontMetrics& /* cellFontMetrics */) { return 0; }

  /**
    * Helper method for the above method.
    */
  void registerCellText(QString& txt, int row, int col);

  virtual int formRowHeight(int row);
  virtual int formRowHeight(void) const;

  virtual void setupForm(KMyMoneyTransactionForm::TransactionForm* form);
  virtual void setupFormPalette(QMap<QString, QWidget*>& editWidgets);
  virtual void setupRegisterPalette(QMap<QString, QWidget*>& editWidgets);
  virtual void loadTab(KMyMoneyTransactionForm::TransactionForm* form) = 0;

  virtual void arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets) = 0;
  virtual void arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets) = 0;
  virtual void tabOrderInForm(QWidgetList& tabOrderWidgets) const = 0;
  virtual void tabOrderInRegister(QWidgetList& tabOrderWidgets) const = 0;

  QWidget* focusWidget(QWidget*) const;
  void arrangeWidget(QTable* tbl, int row, int col, QWidget* w) const;

  bool haveNumberField(void) const;

  bool matches(const QString&) const;

  /**
    * Checks if the mouse hovered over an area that has a tooltip associated with it.
    * The mouse position is given in relative coordinates to the @a startRow and the
    * @a row and @a col of the item are also passed as relative values.
    *
    * If a tooltip shall be shown, this method presets the rectangle @a r with the
    * area in register coordinates and @a msg with the string that will be passed
    * to QToolTip::tip. @a true is returned in this case.
    *
    * If no tooltip is available, @a false will be returned.
    */
  virtual bool maybeTip(const QPoint& relpos, int row, int col, QRect& r, QString& msg);

  /**
    * This method returns the number of register rows required for a certain
    * item in expanded (@p expanded equals @a true) or collapsed (@p expanded
    * is @a false) mode.
    *
    * @param expanded returns number of maximum rows required for this item to
    *                 display all information (used for ledger lens and register
    *                 edit mode) or the minimum number of rows required.
    * @return number of rows required for mode selected by @p expanded
    */
  virtual int numRowsRegister(bool expanded) const = 0;

  virtual int numRowsRegister(void) const = 0;

  void leaveEditMode(void);
  void startEditMode(void);

  /**
    * This method creates an editor for the transaction
    */
  virtual TransactionEditor* createEditor(TransactionEditorContainer* regForm, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate) = 0;

  virtual void setVisible(bool visible);

  virtual void setShowBalance(bool showBalance);

  /**
    * Return information if @a row should be shown (@a true )
    * or hidden (@a false ) in the form. Default is true.
    */
  virtual bool showRowInForm(int row) const { Q_UNUSED(row) return true; }

  /**
    * Control visibility of @a row in the transaction form.
    * Only row 0 has an effect, others return @a true.
    */
  virtual void setShowRowInForm(int row, bool show) { Q_UNUSED(row); Q_UNUSED(show) }

protected:
  virtual void markAsErronous(QPainter* p, int row, int col, const QRect& r);

  /**
    * This method converts m_split.reconcileFlag() into a readable string
    *
    * @param text Return textual representation e.g. "Cleared" (@a true) or just
    *             a flag e.g. "C" (@a false). Defaults to textual representation.
    * @return Textual representation or flag as selected via @p text of the
    *         reconciliation state of the split
    */
  QString reconcileState(bool text = true) const;

  /**
    * Helper method to reduce a multi line memo text into a single line.
    *
    * @param txt QString that will receive the single line memo text
    * @param split const reference to the split to take the memo from
    */
  void singleLineMemo(QString& txt, const MyMoneySplit& split) const;

  virtual void setupPalette(const QPalette& palette, QMap<QString, QWidget*>& editWidgets);

protected:
  MyMoneyTransaction      m_transaction;
  MyMoneySplit            m_split;
  MyMoneyAccount          m_account;
  QTable*                 m_form;
  QString                 m_category;
  QString                 m_payee;
  QString                 m_payeeHeader;
  QString                 m_balance;
  QString                 m_categoryHeader;
  QCString                m_splitCurrencyId;
  QCString                m_uniqueId;
  int                     m_formRowHeight;
  bool                    m_selected;
  bool                    m_focus;
  bool                    m_erronous;
  bool                    m_inEdit;
  bool                    m_inRegisterEdit;
  bool                    m_showBalance;
};

class StdTransaction : public Transaction
{
public:
  StdTransaction(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~StdTransaction() {}

  virtual const char* className(void) { return "StdTransaction"; }
  virtual void paintRegisterCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);

  bool formCellText(QString& txt, int& align, int row, int col, QPainter* painter = 0);
  void registerCellText(QString& txt, int& align, int row, int col, QPainter* painter = 0);

  int registerColWidth(int col, const QFontMetrics& cellFontMetrics);
  void setupForm(KMyMoneyTransactionForm::TransactionForm* form);
  void loadTab(KMyMoneyTransactionForm::TransactionForm* form);

  int numColsForm(void) const { return 4; }

  void arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets);
  void arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets);
  void tabOrderInForm(QWidgetList& tabOrderWidgets) const;
  void tabOrderInRegister(QWidgetList& tabOrderWidgets) const;

  int numRowsRegister(bool expanded) const;

  /**
    * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister()
    */
  int numRowsRegister(void) const { return RegisterItem::numRowsRegister(); }

  TransactionEditor* createEditor(TransactionEditorContainer* regForm, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate);

  /**
    * Return information if @a row should be shown (@a true )
    * or hidden (@a false ) in the form. Default is true.
    */
  virtual bool showRowInForm(int row) const;

  /**
    * Control visibility of @a row in the transaction form.
    * Only row 0 has an effect, others return @a true.
    */
  virtual void setShowRowInForm(int row, bool show);

protected:
  void setupFormHeader(const QCString& id);

private:
  bool m_showAccountRow;
};

class InvestTransaction : public Transaction
{
public:
  InvestTransaction(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~InvestTransaction() {}

  virtual const QString& sortSecurity(void) const { return m_security.name(); }
  virtual const char* className(void) { return "InvestTransaction"; }

  // virtual void paintRegisterCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);

  bool formCellText(QString& txt, int& align, int row, int col, QPainter* painter = 0);
  void registerCellText(QString& txt, int& align, int row, int col, QPainter* painter = 0);

  int registerColWidth(int col, const QFontMetrics& cellFontMetrics);
  void setupForm(KMyMoneyTransactionForm::TransactionForm* form);

  /**
    * provide NOP here as the investment transaction form does not supply a tab
    */
  void loadTab(KMyMoneyTransactionForm::TransactionForm* /* form */) {}

  int numColsForm(void) const { return 4; }

  void arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets);
  void arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets);
  void tabOrderInForm(QWidgetList& tabOrderWidgets) const;
  void tabOrderInRegister(QWidgetList& tabOrderWidgets) const;

  int numRowsRegister(bool expanded) const;

  /**
    * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister()
    */
  int numRowsRegister(void) const { return RegisterItem::numRowsRegister(); }

  TransactionEditor* createEditor(TransactionEditorContainer* regForm, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate);

  void splits(MyMoneySplit& assetAccountSplit, QValueList<MyMoneySplit>& interestSplits, QValueList<MyMoneySplit>& feeSplits) const;

protected:
  bool haveShares(void) const;
  bool haveFees(void) const;
  bool haveInterest(void) const;
  bool havePrice(void) const;
  bool haveAmount(void) const;
  bool haveAssetAccount(void) const;
  bool haveSplitRatio(void) const;

  /**
    * Returns textual representation of the activity identified
    * by @p type.
    *
    * @param txt reference to QString where to store the result
    * @param type activity represented as investTransactionTypeE
    */
  void activity(QString& txt, MyMoneySplit::investTransactionTypeE type) const;

private:
  QValueList<MyMoneySplit>  m_feeSplits;
  QValueList<MyMoneySplit>  m_interestSplits;
  MyMoneySplit              m_assetAccountSplit;
  MyMoneySecurity           m_security;
  MyMoneySecurity           m_currency;
  MyMoneySplit::investTransactionTypeE    m_transactionType;
  QString                   m_feeCategory;
  QString                   m_interestCategory;
  MyMoneyMoney              m_feeAmount;
  MyMoneyMoney              m_interestAmount;
  MyMoneyMoney              m_totalAmount;
};

}; // namespace

#endif
// vim:cin:si:ai:et:ts=2:sw=2:

