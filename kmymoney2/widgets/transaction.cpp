/***************************************************************************
                          transaction.cpp  -  description
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qpainter.h>
#include <qpalette.h>
// FIXME remove tabbar
// #include <qtabbar.h>
#include <qwidgetlist.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/transaction.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/mymoneysplit.h>
#include <kmymoney/mymoneyobjectcontainer.h>
#include <kmymoney/mymoneypayee.h>
#include <kmymoney/register.h>
#include <kmymoney/kmymoneycategory.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/transactionform.h>
#include <kmymoney/kmymoneylineedit.h>
#include <kmymoney/kmymoneypayee.h>

#include "../kmymoneyglobalsettings.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

Transaction::Transaction(Register *parent, MyMoneyObjectContainer* objects, const MyMoneyTransaction& transaction, const MyMoneySplit& split) :
  RegisterItem(parent),
  m_transaction(transaction),
  m_split(split),
  m_objects(objects),
  m_uniqueId(m_transaction.id()+m_split.id()),
  m_selected(false),
  m_focus(false),
  m_erronous(false),
  m_form(0)
// FIXME remove tabbar
  // m_action(MaxAction)
{
  // load the payee
  if(!m_split.payeeId().isEmpty()) {
    m_payee = m_objects->payee(m_split.payeeId()).name();
  }

  // load the currency
  if(!m_transaction.id().isEmpty())
    m_splitCurrencyId = objects->account(m_split.accountId()).currencyId();

  // check if transaction is errnous or not
  m_erronous = m_transaction.splitSum() != MyMoneyMoney(0);
}

void Transaction::setFocus(bool focus, bool updateLens)
{
  if(focus != m_focus) {
    m_focus = focus;
    if(updateLens) {
      if(KMyMoneySettings::ledgerLens() || !KMyMoneySettings::transactionForm() || KMyMoneySettings::showRegisterDetailed()) {
        if(focus)
          setNumRowsRegister(3);
        else
          setNumRowsRegister(KMyMoneySettings::showRegisterDetailed() ? 3 : 1);
      }
    }
  }
}

void Transaction::markAsErronous(QPainter* painter, int row, int col, const QRect& r)
{
#if 0
  // the code for the old corner
  int h = m_parent->rowHeightHint() / 3;
  painter->save();
  QPointArray a(3);
  a.setPoint(0, r.topRight().x()-h, 0);
  a.setPoint(1, r.topRight().x(), 0);
  a.setPoint(2, r.topRight().x(), h);
  painter->setPen(Qt::NoPen);
  painter->setBrush(QBrush(KMyMoneySettings::listErronousTransactionColor(), Qt::SolidPattern));
  painter->drawPolygon(a);
  painter->restore();
#endif

  const int m = 4;  // margin
  int h = m_parent->rowHeightHint() - (2*m);
  QRect cr(QPoint(r.topRight().x() - h - m, m), QSize(h, h));
  painter->save();
  painter->setPen(Qt::NoPen);
  painter->setBrush(QBrush(KMyMoneySettings::listErronousTransactionColor(), Qt::SolidPattern));
  painter->drawEllipse(cr);
  painter->setBrush(QBrush(QColor("white"), Qt::SolidPattern));
  int eh = h/10;
  QRect er(QPoint(r.topRight().x() - h/2 - eh/2 - m, m + eh), QSize(eh, 5*eh));
  painter->drawRect(er);
  er = QRect(QPoint(r.topRight().x() - h/2 - eh/2 - m, m + (7*eh)), QSize(eh, eh));
  painter->drawRect(er);
  painter->restore();
}

void Transaction::paintRegisterCell(QPainter* painter, int row, int col, const QRect& r, bool /*selected*/, const QColorGroup& cg)
{

  if(m_focus) {
    QPen oldPen = painter->pen();
    QPen newPen = oldPen;
    newPen.setWidth(0);

    painter->setFont(KMyMoneyGlobalSettings::listCellFont());
    painter->setPen(newPen);
    painter->setPen(cg.foreground());
    painter->setPen(Qt::DotLine);
    // for the first Row, we need to paint the top
    QPoint start, end;
#if 0
    if(row == 0) {
      start = QPoint(r.x(), r.y() + 1);
      end = QPoint(r.x() + r.width(), r.y() + 1);
      if(col == 0) {
        start.rx()++;
      } else if(col == m_parent->lastCol()) {
        end.rx()--;
      }
      // painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    // for the last Row, we need to paint the bottom
    if(row == numRows() - 1) {
      start = QPoint(r.x(), r.y() + r.height() - 1);
      end = QPoint(r.x() + r.width(), r.y() + r.height() - 1);
      if(col == 0) {
        start.rx()++;
      } else if(col == m_parent->lastCol()) {
        end.rx()--;
      }
      // painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    // for the first col, we need to paint the left
    if(col == 0) {
      start = QPoint(r.x() + 1, r.y());
      end = QPoint(r.x() + 1, r.y() + r.height());
      if(row == 0) {
        start.ry()++;
      } else if(row == numRows()-1) {
        end.ry()--;
      }
      //painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    // for the last col, we need to paint the left
    if(col == m_parent->lastCol()) {
      start = QPoint(r.x() + r.width() - 1, r.y());
      end = QPoint(r.x() + r.width() - 1, r.y() + r.height());
      if(row == 0) {
        start.ry()++;
      } else if(row == numRows()-1) {
        end.ry()--;
      }
      //painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
#endif
    if(row == 0) {
      start = QPoint(r.x(), r.y());
      end = QPoint(r.x() + r.width(), r.y() + 1);
      if(col == 0) {
        start.rx()++;
      } else if(col == m_parent->lastCol()) {
        end.rx()--;
      }
      // painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    // for the last Row, we need to paint the bottom
    if(row == numRowsRegister() - 1) {
      start = QPoint(r.x(), r.y() + r.height() - 2);
      end = QPoint(r.x() + r.width(), r.y() + r.height() - 2);
      if(col == 0) {
        start.rx()++;
      } else if(col == m_parent->lastCol()) {
        end.rx()--;
      }
      // painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    // for the first col, we need to paint the left
    if(col == 0) {
      start = QPoint(r.x() + 1, r.y());
      end = QPoint(r.x() + 1, r.y() + r.height());
      if(row == 0) {
        start.ry()++;
      } else if(row == numRowsRegister()-1) {
        end.ry()--;
      }
      //painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    // for the last col, we need to paint the left
    if(col == m_parent->lastCol()) {
      start = QPoint(r.x() + r.width() - 1, r.y());
      end = QPoint(r.x() + r.width() - 1, r.y() + r.height());
      if(row == 0) {
        start.ry()++;
      } else if(row == numRowsRegister()-1) {
        end.ry()--;
      }
      //painter->drawLine(start, end);
      painter->drawWinFocusRect(QRect(start, end));
    }
    painter->setPen(oldPen);
  }
}

int Transaction::formRowHeight(int /*row*/)
{
  return 1;
}

void Transaction::setupForm(QTable* form)
{
  m_form = form;
  form->setUpdatesEnabled(false);
  form->verticalHeader()->setUpdatesEnabled(false);
  form->horizontalHeader()->setUpdatesEnabled(false);

  form->setNumRows(numRowsForm());
  form->setNumCols(numColsForm());

  // Force all cells to have some text (so that paintCell is called for each cell)
  for(int r = 0; r < numRowsForm(); ++r) {
    form->setRowHeight(r, formRowHeight(r));
    for(int c = 0; c < numColsForm(); ++c) {
      form->setText(r, c, "x");
      if(form->columnWidth(c) == 0) {
        form->setColumnWidth(c, 10);
      }
    }
  }
  form->horizontalHeader()->setUpdatesEnabled(true);
  form->verticalHeader()->setUpdatesEnabled(true);
  form->setUpdatesEnabled(true);
}

void Transaction::setupPalette(QMap<QString, QWidget*>& editWidgets)
{
  // make sure, we're using the right palette
  QPalette palette = m_parent->palette();
  QMap<QString, QWidget*>::iterator it_w;
  for(it_w = editWidgets.begin(); it_w != editWidgets.end(); ++it_w) {
    if(*it_w)
      (*it_w)->setPalette(palette);
  }
}

QWidget* Transaction::focusWidget(QWidget* w) const
{
  if(w) {
    while(w->focusProxy())
      w = w->focusProxy();
  }
  return w;
}

void Transaction::arrangeWidget(QTable* tbl, int row, int col, QWidget* w) const
{
  if(w)
    tbl->setCellWidget(row, col, w);
}

bool Transaction::haveNumberField(void) const
{
  bool rc = true;
  switch(m_objects->account(m_split.accountId()).accountGroup()) {
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Equity:
      rc = KMyMoneySettings::alwaysShowNrField();
      break;

    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::CreditCard:
      break;

    default:
      rc = false;
      break;
  }
  return rc;
}

StdTransaction::StdTransaction(Register *parent, MyMoneyObjectContainer* objects, const MyMoneyTransaction& transaction, const MyMoneySplit& split) :
  Transaction(parent, objects, transaction, split),
  m_formRowHeight(-1)
{
  try {
    m_categoryHeader = i18n("Category");
    switch(transaction.splitCount()) {
      default:
        m_category = i18n("Split transaction (category replacement)", "Split transaction");
        break;

      case 0: // the empty transaction
      case 1:
        break;

      case 2:
        setupFormHeader(m_transaction.splitByAccount(m_split.accountId(), false).accountId());
        break;
    }
  } catch(MyMoneyException *e) {
    kdDebug(2) << "Problem determining the category for transaction '" << m_transaction.id() << "'. Reason: " << e->what()  << "\n";
    delete e;
  }
  m_rowsForm = 5;
}

// FIXME remove tabbar
#if 0
void StdTransaction::setupAction(void)
{
  if(m_split.action() == MyMoneySplit::ActionCheck)
    m_action = ActionCheck;
  else if(m_split.action() == MyMoneySplit::ActionATM)
    m_action = ActionAtm;
  else {
    // if at least one split is referencing an income or
    // expense account, we will not call it a transfer
    QValueList<MyMoneySplit>::const_iterator it_s;
    for(it_s = m_transaction.splits().begin(); it_s != m_transaction.splits().end(); ++it_s) {
      if((*it_s).accountId() == m_split.accountId())
        continue;
      MyMoneyAccount acc = m_objects->account((*it_s).accountId());
      if(acc.accountGroup() == MyMoneyAccount::Income
      || acc.accountGroup() == MyMoneyAccount::Expense) {
        // otherwise, we have to determine between deposit and withdrawal
        m_action = m_split.shares().isNegative() ? ActionWithdrawal : ActionDeposit;
        return;
      }
    }
    // otherwise, it's a transfer
    m_action = ActionTransfer;
    return;
  }
}
#endif

void StdTransaction::setupFormHeader(const QCString& id)
{
  m_category = m_objects->accountToCategory(id);
  switch(m_objects->account(id).accountGroup()) {
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
      m_categoryHeader = m_split.shares().isNegative() ? i18n("Transfer to") : i18n("Transfer from");
      break;

    default:
      m_categoryHeader = i18n("Category");
      break;
  }
}

int StdTransaction::formRowHeight(int /*row*/)
{
  if(m_formRowHeight < 0) {
    // determine the height of the objects in the table
    kMyMoneyDateInput dateInput;
    // FIXME make sure the category has the split button activated
    kMyMoneyCategory category;

    m_formRowHeight = QMAX(dateInput.sizeHint().height(), category.sizeHint().height());
  }
  return m_formRowHeight;
}

void StdTransaction::setupForm(QTable* _form)
{
  KMyMoneyTransactionForm::TransactionForm* form = dynamic_cast<KMyMoneyTransactionForm::TransactionForm*> (_form);

  Transaction::setupForm(form);

// FIXME remove tabbar
#if 0
  if(m_action == MaxAction)
    setupAction();
#endif

  QTableItem* memo = form->item(2, 1);
  memo->setSpan(3, 1);

// FIXME remove tabbar
#if 0
  // setup the tab bar for view mode (all tabs enabled)
  QTabBar* bar = form->tabBar();
  bar->blockSignals(true);
  for(int i = ActionCheck; i < MaxAction; ++i) {
    bar->setTabEnabled(i, true);
  }
  bar->setCurrentTab(m_action);
  bar->blockSignals(false);
#endif
}

void StdTransaction::paintFormCell(QPainter* painter, int row, int col, const QRect& /*r*/, bool /*selected*/, const QColorGroup& _cg)
{
  if(!m_form)
    return;

  QRect cellRect = m_form->cellRect(row, col);

  QRect textRect(cellRect);
  textRect.setX(1);
  textRect.setY(1);
  textRect.setWidth(textRect.width()-2);
  textRect.setHeight(textRect.height()-2);

  painter->fillRect(cellRect, _cg.background());
  painter->setPen(_cg.text());

  QString txt;
  int align = Qt::AlignVCenter;
  bool editField = formCellText(txt, align, row, col);

  if(editField) {
    painter->fillRect(textRect, _cg.base());
  }
  // make sure, we clear the cell
  if(txt.isEmpty())
    painter->drawText(textRect, align, " ");
  else
    painter->drawText(textRect, align, txt);

}

bool StdTransaction::formCellText(QString& txt, int& align, int row, int col)
{
  // if(m_transaction != MyMoneyTransaction()) {
    switch(row) {
      case 0:
        switch(col) {
          case LabelColumn1:
            align |= Qt::AlignLeft;
            txt = m_split.shares().isNegative() ? i18n("Pay to") : i18n("From");
            break;

          case ValueColumn1:
            align |= Qt::AlignLeft;
            txt = m_payee;
            break;

          case LabelColumn2:
            align |= Qt::AlignLeft;
            if(haveNumberField())
              txt = i18n("Number");
            break;

          case ValueColumn2:
            align |= Qt::AlignRight;
            if(haveNumberField())
              txt = m_split.number();
            break;
        }
        break;

      case 1:
        switch(col) {
          case 0:
            align |= Qt::AlignLeft;
            txt = m_categoryHeader;
            break;

          case 1:
            align |= Qt::AlignLeft;
            txt = m_category;
            if(m_transaction != MyMoneyTransaction()) {
              if(txt.isEmpty() && !m_split.value().isZero())
                txt = i18n("*** UNASSIGNED ***");
            }
            break;

          case 2:
            align |= Qt::AlignLeft;
            txt = i18n("Date");
            break;

          case 3:
            align |= Qt::AlignRight;
            if(m_transaction != MyMoneyTransaction())
              txt = KGlobal::locale()->formatDate(m_transaction.postDate(), true);
            break;
        }
        break;

      case 2:
        switch(col) {
          case 0:
            align |= Qt::AlignLeft;
            txt = i18n("Memo");
            break;

          case 1:
            align &= ~Qt::AlignVCenter;
            align |= Qt::AlignTop;
            align |= Qt::AlignLeft;
            if(m_transaction != MyMoneyTransaction())
              txt = m_split.memo().section('\n', 0, 2);
            break;

          case 2:
            align |= Qt::AlignLeft;
            txt = i18n("Amount");
            break;

          case 3:
            align |= Qt::AlignRight;
            if(m_transaction != MyMoneyTransaction())
              txt = (m_split.value(m_transaction.commodity(), m_splitCurrencyId).abs()).formatMoney();
            break;
        }
        break;

      case 4:
        switch(col) {
          case 2:
            align |= Qt::AlignLeft;
            txt = i18n("Status");
            break;

          case 3:
            align |= Qt::AlignRight;
            switch(m_split.reconcileFlag()) {
              case MyMoneySplit::NotReconciled:
                txt = i18n("Reconcile state 'Not reconciled'", "Not reconciled");
                break;
              case MyMoneySplit::Cleared:
                txt = i18n("Reconcile state 'Cleared'", "Cleared");
                break;
              case MyMoneySplit::Reconciled:
                txt = i18n("Reconcile state 'Reconciled'", "Reconciled");
                break;
              case MyMoneySplit::Frozen:
                txt = i18n("Reconcile state 'Frozen'", "Frozen");
                break;
              default:
                if(m_transaction != MyMoneyTransaction())
                  txt = i18n("Unknown");
                break;
            }
        }
    }
  // }
  if(col == ValueColumn2 && row == 0) {
    return haveNumberField();
  }
  return (col == 1 && row < 3) || (col == 3 && row != 3);
}

void StdTransaction::paintRegisterCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& _cg)
{
  int align = Qt::AlignVCenter;

  QColorGroup cg(_cg);
  if(m_alternate)
    cg.setColor(QColorGroup::Base, KMyMoneySettings::listColor());
  else
    cg.setColor(QColorGroup::Base, KMyMoneySettings::listBGColor());

  if(m_transaction.value("Imported").lower() == "true") {
    cg.setColor(QColorGroup::Base, KMyMoneySettings::importedTransactionColor());
  }
  if(m_transaction.value("MatchSelected").lower() == "true") {
    cg.setColor(QColorGroup::Base, KMyMoneySettings::matchedTransactionColor());
  }

  QRect cellRect(r);
  cellRect.setX(0);
  cellRect.setY(0);
  cellRect.setWidth(m_parent->columnWidth(col));
  cellRect.setHeight(m_parent->rowHeight(m_startRow + row));

  QRect textRect(cellRect);
  textRect.setX(2);
  textRect.setY(0);
  textRect.setWidth(textRect.width()-4);
  // textRect.setHeight(m_parent->rowHeight(m_startRow + row));

  if(m_selected) {
    QBrush backgroundBrush(cg.highlight());
    painter->fillRect(cellRect, backgroundBrush);
    painter->setPen(cg.highlightedText());
  } else {
    QBrush backgroundBrush(cg.base());
    painter->fillRect(cellRect, backgroundBrush);
    painter->setPen(cg.text());
  }

  // do we need to switch to the error color?
  if(m_erronous && m_parent->markErronousTransactions()) {
    painter->setPen(KMyMoneySettings::listErronousTransactionColor());
  }

  QString txt;
  if(m_transaction != MyMoneyTransaction()) {
    switch(row) {
      case 0:
        switch(col) {
          case NumberColumn:
            align |= Qt::AlignLeft;
            if(haveNumberField())
              txt = m_split.number();
            break;

          case DateColumn:
            align |= Qt::AlignLeft;
            txt = KGlobal::locale()->formatDate(m_transaction.postDate(), true);
            break;

          case DetailColumn:
            align |= Qt::AlignLeft;
            txt = m_payee;
            if(txt.isEmpty() && m_rowsRegister < 3) {
              txt = m_split.memo();
              // remove empty lines
              txt.replace("\n\n", "\n");
              // replace '\n' with ", "
              txt.replace('\n', ", ");
            }
            if(txt.isEmpty() && m_rowsRegister < 2) {
              txt = m_category;
            }
            if(m_erronous)
              markAsErronous(painter, row, col, cellRect);
            break;

          case MarkColumn:
            align |= Qt::AlignHCenter;
            switch(m_split.reconcileFlag()) {
              case MyMoneySplit::NotReconciled:
                break;
              case MyMoneySplit::Cleared:
                txt = i18n("Reconcile flag C", "C");
                break;
              case MyMoneySplit::Reconciled:
                txt = i18n("Reconcile flag R", "R");
                break;
              case MyMoneySplit::Frozen:
                txt = i18n("Reconcile flag F", "F");
                break;
              default:
                txt = i18n("Flag for unknown reconciliation state", "?");
                break;
            }
            break;

          case PaymentColumn:
            align |= Qt::AlignRight;
            if(m_split.value().isNegative()) {
              txt = (-m_split.value(m_transaction.commodity(), m_splitCurrencyId)).formatMoney();
            }
            break;

          case DepositColumn:
            align |= Qt::AlignRight;
            if(!m_split.value().isNegative()) {
              txt = m_split.value(m_transaction.commodity(), m_splitCurrencyId).formatMoney();
            }
            break;

          case BalanceColumn:
            align |= Qt::AlignRight;
            txt = m_balance;
            break;

          default:
            break;
        }
        break;

      case 1:
        switch(col) {
          case DetailColumn:
            align |= Qt::AlignLeft;
            txt = m_category;
            if(txt.isEmpty() && !m_split.value().isZero()) {
              painter->setPen(KMyMoneySettings::listErronousTransactionColor());
              txt = i18n("*** UNASSIGNED ***");
            }
            break;

          default:
            break;
        }
        break;

      case 2:
        switch(col) {
          case DetailColumn:
            align |= Qt::AlignLeft;
            txt = m_split.memo();
            // remove empty lines
            txt.replace("\n\n", "\n");
            // replace '\n' with ", "
            txt.replace('\n', ", ");
            break;

          default:
            break;
        }
        break;
    }
  }

  // make sure, we clear the cell
  if(txt.isEmpty())
    painter->drawText(textRect, align, " ");
  else
    painter->drawText(textRect, align, txt);

    // if a grid is selected, we paint it right away
  if (KMyMoneySettings::showGrid()) {
    painter->save();
    painter->setPen(KMyMoneySettings::listGridColor());
    if(col != 0)
      painter->drawLine(cellRect.x(), 0, cellRect.x(), cellRect.height()-1);
    if(row == numRowsRegister()-1)
      painter->drawLine(cellRect.x(), cellRect.height()-1, cellRect.width(), cellRect.height()-1);
    painter->restore();
  }


  // take care of standard stuff (e.g. focus)
  Transaction::paintRegisterCell(painter, row, col, cellRect, selected, cg);
}

void StdTransaction::arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets)
{
  if(!m_form || !m_parent)
    return;

  setupPalette(editWidgets);

  arrangeWidget(m_form, 0, LabelColumn1, editWidgets["cashflow"]);
  arrangeWidget(m_form, 0, ValueColumn1, editWidgets["payee"]);
  arrangeWidget(m_form, 1, ValueColumn1, editWidgets["category"]->parentWidget());
  arrangeWidget(m_form, 2, ValueColumn1, editWidgets["memo"]);
  if(haveNumberField())
    arrangeWidget(m_form, 0, ValueColumn2, editWidgets["number"]);
  arrangeWidget(m_form, 1, ValueColumn2, editWidgets["postdate"]);
  arrangeWidget(m_form, 2, ValueColumn2, editWidgets["amount"]);
  arrangeWidget(m_form, 4, ValueColumn2, editWidgets["status"]);
  arrangeWidget(m_form, 1, LabelColumn1, editWidgets["categoryLabel"]);

  // get rid of the hints. we don't need them for the form
  QMap<QString, QWidget*>::iterator it;
  for(it = editWidgets.begin(); it != editWidgets.end(); ++it) {
    KMyMoneyCombo* combo = dynamic_cast<KMyMoneyCombo*>(*it);
    kMyMoneyLineEdit* edit = dynamic_cast<kMyMoneyLineEdit*>(*it);
    KMyMoneyPayee* payee = dynamic_cast<KMyMoneyPayee*>(*it);
    if(combo)
      combo->setHint(QString());
    if(edit)
      edit->setHint(QString());
    if(payee)
      payee->setHint(QString());
  }
}

void StdTransaction::tabOrderInForm(QWidgetList& tabOrderWidgets) const
{
  // cashflow direction
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(0, LabelColumn1)));
  // payee
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(0, ValueColumn1)));
  // make sure to have the category field and the split button as seperate tab order widgets
  // ok, we have to have some internal knowledge about the KMyMoneyCategory object, but
  // it's one of our own widgets, so we actually don't care. Just make sure, that we don't
  // go haywire when someone changes the KMyMoneyCategory object ...
  QWidget* w = m_form->cellWidget(1, ValueColumn1);
  tabOrderWidgets.append(focusWidget(w));
  w = dynamic_cast<QWidget*>(w->child("splitButton"));
  if(w)
    tabOrderWidgets.append(w);
  // memo
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(2, ValueColumn1)));
  // number
  if(haveNumberField()) {
    if((w = focusWidget(m_form->cellWidget(0, ValueColumn2))))
      tabOrderWidgets.append(w);
  }
  // date
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(1, ValueColumn2)));
  // amount
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(2, ValueColumn2)));
  // state
  tabOrderWidgets.append(focusWidget(m_form->cellWidget(4, ValueColumn2)));
}

void StdTransaction::arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets)
{
  if(!m_parent)
    return;

  setupPalette(editWidgets);

  if(haveNumberField())
    arrangeWidget(m_parent, m_startRow+0, NumberColumn, editWidgets["number"]);
  arrangeWidget(m_parent, m_startRow + 0, DateColumn, editWidgets["postdate"]);
  arrangeWidget(m_parent, m_startRow + 0, DetailColumn, editWidgets["payee"]);
  arrangeWidget(m_parent, m_startRow + 1, DetailColumn, editWidgets["category"]->parentWidget());
  arrangeWidget(m_parent, m_startRow + 2, DetailColumn, editWidgets["memo"]);
  arrangeWidget(m_parent, m_startRow + 0, PaymentColumn, editWidgets["payment"]);
  arrangeWidget(m_parent, m_startRow + 0, DepositColumn, editWidgets["deposit"]);
  arrangeWidget(m_parent, m_startRow + 1, PaymentColumn, editWidgets["status"]);

  // increase the height of the row containing the memo widget
  m_parent->setRowHeight(m_startRow+2, m_parent->rowHeightHint() * 3);
}

void StdTransaction::tabOrderInRegister(QWidgetList& tabOrderWidgets) const
{
  QWidget* w;
  // number
  if(haveNumberField()) {
    if((w = focusWidget(m_parent->cellWidget(m_startRow + 0, NumberColumn))))
      tabOrderWidgets.append(w);
  }

  // date
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, DateColumn)));
  // payee
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, DetailColumn)));
  // make sure to have the category field and the split button as seperate tab order widgets
  // ok, we have to have some internal knowledge about the KMyMoneyCategory object, but
  // it's one of our own widgets, so we actually don't care. Just make sure, that we don't
  // go haywire when someone changes the KMyMoneyCategory object ...
  w = m_parent->cellWidget(m_startRow + 1, DetailColumn);
  tabOrderWidgets.append(focusWidget(w));
  w = dynamic_cast<QWidget*>(w->child("splitButton"));
  if(w)
    tabOrderWidgets.append(w);
  // memo
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 2, DetailColumn)));
  // payment
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, PaymentColumn)));
  // deposit
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 0, DepositColumn)));
  // status
  tabOrderWidgets.append(focusWidget(m_parent->cellWidget(m_startRow + 1, PaymentColumn)));
}

bool StdTransaction::maybeTip(const QPoint& cpos, int row, int col, QRect& r, QString& msg)
{
  if(col != DetailColumn || row != 0)
    return false;

  if(!m_erronous)
    return false;

  int h = m_parent->rowHeightHint();
  r = m_parent->cellGeometry(m_startRow + row, col);
  // qDebug("r is %d,%d,%d,%d", r.x(), r.y(), r.width(), r.height());
  r.setBottomLeft(QPoint(r.x() + (r.width() - h), r.y() + h));
  // qDebug("r is %d,%d,%d,%d", r.x(), r.y(), r.width(), r.height());
  // qDebug("p is in r = %d", r.contains(cpos));
  if(r.contains(cpos)) {
    if(m_transaction.splits().count() < 2) {
      msg = QString("<qt>%1</qt>").arg(i18n("Transaction is missing a category assignment."));
    } else {
      msg = QString("<qt>%1</qt>").arg(i18n("The transaction has a missing assignment of <b>%1</b>.").arg(m_transaction.splitSum().abs().formatMoney()));
    }
    return true;
  }
  return false;
}
