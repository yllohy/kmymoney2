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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/registeritem.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/mymoneysplit.h>

class MyMoneyObjectContainer;
class QTable;

namespace KMyMoneyTransactionForm {
  class TransactionForm;
}; // namespace

namespace KMyMoneyRegister {

typedef enum {
  NumberColumn = 0,
  DateColumn,
  AccountColumn,
  DetailColumn,
  SecurityColumn,
  ActivityColumn,
  ReconcileFlagColumn,
  PaymentColumn,
  DepositColumn,
  BalanceColumn,
  AmountColumn,
  PriceColumn,
  ValueColumn,
  // insert new values above this line
  MaxColumns
} Column;


class Transaction : public RegisterItem
{
public:
  Transaction(Register* parent, MyMoneyObjectContainer* objects, const MyMoneyTransaction& transaction, const MyMoneySplit& split);
  virtual ~Transaction() {};

  bool isSelectable(void) const { return true; }
  bool isSelected(void) const { return m_selected; }
  void setSelected(bool selected) { m_selected = selected; }

  bool canHaveFocus(void) const { return true; }
  bool hasFocus(void) const { return m_focus; }
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

  virtual const QCString& id(void) { return m_transaction.id(); }
  const MyMoneyTransaction& transaction(void) const { return m_transaction; }
  const MyMoneySplit& split(void) const { return m_split; }

  void setBalance(const QString& balance) { m_balance = balance; }
  const QString& balance(void) const { return m_balance; }

  void paintRegisterCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);
  virtual void paintFormCell(QPainter* /* painter */, int /* row */, int /* col */, const QRect& /* r */, bool /* selected */, const QColorGroup& /* cg */) {}
  virtual bool formCellText(QString& /* txt */, int& /* align */, int /* row */, int /* col */) { return false; }
  virtual int formRowHeight(int row);

  virtual void setupForm(KMyMoneyTransactionForm::TransactionForm* form);
  virtual void setupPalette(QMap<QString, QWidget*>& editWidgets);
  virtual void loadTab(KMyMoneyTransactionForm::TransactionForm* form) = 0;

// FIXME remove tabbar
#if 0
  virtual void setupAction(void) = 0;
  Action action(void) const { return m_action; }
#endif

  virtual void arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets) = 0;
  virtual void arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets) = 0;
  virtual void tabOrderInForm(QWidgetList& tabOrderWidgets) const = 0;
  virtual void tabOrderInRegister(QWidgetList& tabOrderWidgets) const = 0;

  QWidget* focusWidget(QWidget*) const;
  void arrangeWidget(QTable* tbl, int row, int col, QWidget* w) const;

  MyMoneyObjectContainer* objects(void) const { return m_objects; }
  bool haveNumberField(void) const;

protected:
  virtual void markAsErronous(QPainter* p, int row, int col, const QRect& r);

protected:
  MyMoneyTransaction      m_transaction;
  MyMoneySplit            m_split;
  MyMoneyObjectContainer* m_objects;
  QString                 m_category;
  QString                 m_payee;
  QString                 m_balance;
  QCString                m_splitCurrencyId;
  QCString                m_uniqueId;
  bool                    m_selected;
  bool                    m_focus;
  bool                    m_erronous;
  QTable*                 m_form;
// FIXME remove tabbar
  // Action                  m_action;
};

class StdTransaction : public Transaction
{
public:
  StdTransaction(Register* parent, MyMoneyObjectContainer* objects, const MyMoneyTransaction& transaction, const MyMoneySplit& split);
  virtual ~StdTransaction() {};

  void paintRegisterCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);
  void paintFormCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);

  bool formCellText(QString& txt, int& align, int row, int col);
  int formRowHeight(int row);
  void setupForm(KMyMoneyTransactionForm::TransactionForm* form);
  void loadTab(KMyMoneyTransactionForm::TransactionForm* form);

  int numColsForm(void) const { return 4; }

// FIXME remove tabbar
  // void setupAction(void);

  void arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets);
  void arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets);
  void tabOrderInForm(QWidgetList& tabOrderWidgets) const;
  void tabOrderInRegister(QWidgetList& tabOrderWidgets) const;

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

private:
  void setupFormHeader(const QCString& id);

private:
  QString                m_categoryHeader;
  int                    m_formRowHeight;

};

}; // namespace

#endif
// vim:cin:si:ai:et:ts=2:sw=2:
