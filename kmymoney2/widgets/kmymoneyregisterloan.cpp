/***************************************************************************
                          kmymoneyregisterloan.cpp  -  description
                             -------------------
    begin                : Sat Sep 13 2003
    copyright            : (C) 2003 by Thomas Baumgart
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

#include "kmymoneyregisterloan.h"
#include "../mymoney/mymoneyfile.h"
#include "../views/kledgerview.h"

kMyMoneyRegisterLoan::kMyMoneyRegisterLoan(QWidget *parent, const char *name ) :
  kMyMoneyRegister(3, parent, name)
{
  setNumCols(6);
  setCurrentCell(0, 1);
  horizontalHeader()->setClickEnabled(true);
  horizontalHeader()->setLabel(0, i18n("Date"));
  horizontalHeader()->setLabel(1, i18n("Payment"));
  horizontalHeader()->setLabel(2, i18n("Payee"));
  horizontalHeader()->setLabel(3, i18n("Amount"));
  horizontalHeader()->setLabel(4, i18n("Amortization"));
  horizontalHeader()->setLabel(5, i18n("Balance"));
  setLeftMargin(0);
  verticalHeader()->hide();
  setColumnStretchable(0, false);
  setColumnStretchable(1, false);
  setColumnStretchable(2, false);
  setColumnStretchable(3, false);
  setColumnStretchable(4, false);
  setColumnStretchable(5, false);

  horizontalHeader()->setResizeEnabled(false);
  horizontalHeader()->setMovingEnabled(false);

  // never show horizontal scroll bars
  setHScrollBarMode(QScrollView::AlwaysOff);
}

kMyMoneyRegisterLoan::~kMyMoneyRegisterLoan()
{
}

void kMyMoneyRegisterLoan::paintCell(QPainter *p, int row, int col, const QRect& r,
                                     bool selected, const QColorGroup& cg)
{
  setTransactionRow(row);

  int align = Qt::AlignVCenter;
  QString txt;
  if(m_transaction != 0) {
    switch (col) {
      case 0:                         // Date
        align |= Qt::AlignLeft;
        switch(m_transactionRow) {
          case 0:
            txt = KGlobal::locale()->formatDate(m_transaction->postDate(), true);
            break;
        }
        break;

      case 1:                         // Nr(No.)
        align |= Qt::AlignRight;
        switch(m_transactionRow) {
          case 0:
            txt = m_split.number();
            break;
        }
        break;

      case 2:                         // Payee
        align |= Qt::AlignLeft;
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
              QValueList<MyMoneySplit>::ConstIterator it;
              for(it = m_transaction->splits().begin(); it != m_transaction->splits().end(); ++it) {
                if((*it).action() == MyMoneySplit::ActionAmortization
                && (*it).id() != m_split.id()) {
                  MyMoneyAccount acc = MyMoneyFile::instance()->account((*it).accountId());
                  txt = i18n("Transfer to/from %1").arg(acc.name());
                }
              }
            } catch(MyMoneyException *e) {
              delete e;
            }
            break;

          case 2:
            txt = m_split.memo();
            break;
        }
        break;

      case 3:                         // Amount
        switch(m_transactionRow) {
          case 0:
            try {
              QValueList<MyMoneySplit>::ConstIterator it;
              for(it = m_transaction->splits().begin(); it != m_transaction->splits().end(); ++it) {
                if((*it).action() == MyMoneySplit::ActionAmortization
                && (*it).id() != m_split.id()) {
                  align |= Qt::AlignRight;
                  if((*it).value().isNegative())
                    txt = (-(*it).value()).formatMoney();
                  else
                    txt = ((*it).value()).formatMoney();
                }
              }

            } catch(MyMoneyException *e) {
              delete e;
            }
            break;
        }
        break;

      case 4:                         // Amortization
        switch(m_transactionRow) {
          case 0:
            align |= Qt::AlignRight;
            if(m_split.value().isNegative())
              txt = (-m_split.value()).formatMoney();
            else
              txt = (m_split.value()).formatMoney();
            break;
        }
        break;

      case 5:                         // Balance
        switch(m_transactionRow) {
          case 0:
            align |= Qt::AlignRight;
            txt = m_balance.formatMoney();
            if(m_balance.isNegative())
              p->setPen(QColor(255, 0, 0));
            break;
        }
        break;
    }
  }

  // do general stuff
  kMyMoneyRegister::paintCell(p, row, col, r, selected, cg, txt, align);
}

void kMyMoneyRegisterLoan::adjustColumn(int col)
{
  QHeader *topHeader = horizontalHeader();
  QFontMetrics cellFontMetrics(m_cellFont);

  int w = topHeader->fontMetrics().width( topHeader->label( col ) ) + 10;
  if ( topHeader->iconSet( col ) )
    w += topHeader->iconSet( col )->pixmap().width();
  w = QMAX( w, 20 );

  // scan through the transactions
  for ( int i = (numRows()/m_rpt)-1; i >= 0; --i ) {
    KMyMoneyTransaction *t = m_parent->transaction(i);
    MyMoneyMoney amount;
    int nw = 0;

    QString txt;
    if(t != NULL) {
      switch(col) {
        case 0:
          txt = KGlobal::locale()->formatDate(QDate(6999,12,29), true)+"  ";
          nw = cellFontMetrics.width(txt);
          i = 0; // save some time
          break;

        case 1:
          txt = t->splitById(t->splitId()).number();
          nw = cellFontMetrics.width(txt+"  ");
          break;

        case 3:
          amount = t->splitById(t->splitId()).value();
          txt = amount.formatMoney();
          nw = cellFontMetrics.width(txt+"  ");
          break;

        case 4:
          amount = t->splitById(t->splitId()).value();
          txt = amount.formatMoney();
          nw = cellFontMetrics.width(txt+"  ");
          break;

        case 5:
          amount = m_parent->balance(i);
          txt = amount.formatMoney();
          nw = cellFontMetrics.width(txt+"  ");
          break;
      }
      w = QMAX( w, nw );
    }
  }
  setColumnWidth( col, w );
}

// This must be implemented here, as QTable::eventFilter is not virtual :-(

bool kMyMoneyRegisterLoan::eventFilter(QObject* o, QEvent* e)
{
  return kMyMoneyRegister::eventFilter(o, e);
}

#include "kmymoneyregisterloan.moc"
