/***************************************************************************
                          kmymoneyregisterinvestment.cpp  -  description
                             -------------------
    begin                : Mon Jul 12 2004
    copyright            : (C) 2004 by Thomas Baumgart
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

// ----------------------------------------------------------------------------
// QT Includes

#include "klocale.h"

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyregisterinvestment.h"
#include "../mymoney/mymoneyfile.h"
#include "../views/kledgerview.h"

kMyMoneyRegisterInvestment::kMyMoneyRegisterInvestment(QWidget *parent, const char *name )
  : kMyMoneyRegister(4, parent, name)
{
  setNumCols(7);
  setCurrentCell(0, 1);
  horizontalHeader()->setClickEnabled(true);
  horizontalHeader()->setLabel(0, i18n("Date"));
  horizontalHeader()->setLabel(1, i18n("Equity"));
  horizontalHeader()->setLabel(2, i18n("Activity"));
  horizontalHeader()->setLabel(3, i18n("C"));
  horizontalHeader()->setLabel(4, i18n("Amount"));
  horizontalHeader()->setLabel(5, i18n("Price"));
  horizontalHeader()->setLabel(6, i18n("Value"));
  setLeftMargin(0);
  verticalHeader()->hide();
  setColumnStretchable(0, false);
  setColumnStretchable(1, false);
  setColumnStretchable(2, false);
  setColumnStretchable(3, false);
  setColumnStretchable(4, false);
  setColumnStretchable(5, false);
  setColumnStretchable(6, false);

  horizontalHeader()->setResizeEnabled(false);
  horizontalHeader()->setMovingEnabled(false);

  // never show horizontal scroll bars
  setHScrollBarMode(QScrollView::AlwaysOff);
}

kMyMoneyRegisterInvestment::~kMyMoneyRegisterInvestment()
{
}

void kMyMoneyRegisterInvestment::paintCell(QPainter *p, int row, int col, const QRect& r,
                                 bool selected, const QColorGroup& cg)
{
  QCString splitCurrency;

  setTransactionRow(row);

  int align = Qt::AlignVCenter;
  QString txt;
  if(m_transaction != 0) {
    switch (col) {
      case 0:
        align |= Qt::AlignLeft;
        switch(m_transactionRow) {
          case 0:
            txt = KGlobal::locale()->formatDate(m_transaction->postDate(), true);
            break;
/*
          case 1:
            txt = m_action[MyMoneySplit::ActionWithdrawal];
            if(KLedgerView::transactionType(*m_transaction) == KLedgerView::Transfer) {
              txt = m_action[MyMoneySplit::ActionTransfer];
            } else if(KLedgerView::transactionDirection(m_split) == KLedgerView::Credit) {
              txt = m_action[MyMoneySplit::ActionDeposit];
            } else if( m_split.action() == MyMoneySplit::ActionCheck){
              txt = m_action[MyMoneySplit::ActionCheck];
            } else if(m_split.action() == MyMoneySplit::ActionATM) {
              txt = m_action[MyMoneySplit::ActionATM];
            }

            break;
*/
        }
        break;

      case 2:
        align |= Qt::AlignLeft;
/*
        switch(m_transactionRow) {
          case 0:
            try {
              txt = MyMoneyFile::instance()->payee(m_split.payeeId()).name();
            } catch(MyMoneyException *e) {
              delete e;
            }
            break;

          case 1:
            try {
              if(m_transaction->isLoanPayment()) {
                txt = QString(i18n("Loan payment"));
              } else if(m_transaction->splitCount() > 2)
                txt = QString(i18n("Split transaction"));
              else {
                MyMoneySplit split = m_transaction->splitByAccount(m_split.accountId(), false);
                txt = MyMoneyFile::instance()->accountToCategory(split.accountId());
              }
            } catch(MyMoneyException *e) {
              delete e;
            }
            break;

          case 2:
            txt = m_split.memo();
            break;
        }
*/
        break;

      case 3:
/*
        switch(m_transactionRow) {
          case 0:
            align |= Qt::AlignHCenter;
            switch(m_split.reconcileFlag()) {
              case MyMoneySplit::Cleared:
                txt = i18n("C");
                break;
              case MyMoneySplit::Reconciled:
              case MyMoneySplit::Frozen:
                txt = i18n("R");
                break;
              case MyMoneySplit::NotReconciled:
                break;
            }
            break;
        }
*/
        break;

      case 4:
/*
        switch(m_transactionRow) {
          case 0:
            align |= Qt::AlignRight;
            if(m_split.value() < 0) {
              splitCurrency = MyMoneyFile::instance()->account(m_split.accountId()).currencyId();
              txt = (-m_split.value(m_transaction->commodity(), splitCurrency)).formatMoney();
            }
            break;
        }
*/
        break;

      case 5:
/*
        switch(m_transactionRow) {
          case 0:
            align |= Qt::AlignRight;
            if(m_split.value() >= 0) {
              splitCurrency = MyMoneyFile::instance()->account(m_split.accountId()).currencyId();
              txt = m_split.value(m_transaction->commodity(), splitCurrency).formatMoney();
            }
            break;
        }
*/
        break;

      case 6:
/*
        switch(m_transactionRow) {
          case 0:
            align |= Qt::AlignRight;
            txt = m_balance.formatMoney();
            if(m_balance < 0)
              p->setPen(QColor(255, 0, 0));
            break;
        }
*/
        break;
    }
  }

  // do general stuff
  kMyMoneyRegister::paintCell(p, row, col, r, selected, cg, txt, align);
}

void kMyMoneyRegisterInvestment::adjustColumn(int col)
{
  QHeader *topHeader = horizontalHeader();
  QFontMetrics cellFontMetrics(m_cellFont);

  int w = topHeader->fontMetrics().width( topHeader->label( col ) ) + 10;
  if ( topHeader->iconSet( col ) )
    w += topHeader->iconSet( col )->pixmap().width();
  w = QMAX( w, 20 );

  // check for date column
  if(col == 0) {
    QString txt = KGlobal::locale()->formatDate(QDate(6999,12,29), true);
    int nw = cellFontMetrics.width(txt+"  ");
    w = QMAX( w, nw );
  }

#if 0
  // scan through the transactions
  for ( int i = (numRows()/m_rpt)-1; i >= 0; --i ) {
    QString txt;
    KMyMoneyTransaction *t = m_parent->transaction(i);
    MyMoneyMoney amount;
    int nw = 0;

    if(t != NULL) {
      MyMoneySplit split = t->splitById(t->splitId());
      switch(col) {
        default:
          break;

        case 0:
          txt = t->splitById(t->splitId()).number();
          nw = cellFontMetrics.width(txt+"  ");
          break;

        case 1:
          txt = m_action[split.action()];
          nw = cellFontMetrics.width(txt+"  ");
          break;

        case 4:
          amount = t->splitById(t->splitId()).value();
          if(amount < 0) {
            txt = amount.formatMoney();
            nw = cellFontMetrics.width(txt+"  ");
          }
          break;

        case 5:
          amount = t->splitById(t->splitId()).value();
          if(amount >= 0) {
            txt = amount.formatMoney();
            nw = cellFontMetrics.width(txt+"  ");
          }
          break;

        case 6:
          amount = m_parent->balance(i);
          txt = amount.formatMoney();
          nw = cellFontMetrics.width(txt+"  ");
          break;
      }
      w = QMAX( w, nw );
    }
  }
  setColumnWidth( col, w );
#endif
}

// This must be implemented here, as QTable::eventFilter is not virtual :-(

bool kMyMoneyRegisterInvestment::eventFilter(QObject* o, QEvent* e)
{
  return kMyMoneyRegister::eventFilter(o, e);
}
