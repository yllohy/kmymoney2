/***************************************************************************
                          kmymoneyregistercheckings.cpp  -  description
                             -------------------
    begin                : Thu Jul 18 2002
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

// ----------------------------------------------------------------------------
// QT Includes

#include "klocale.h"

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyregistercheckings.h"
#include "../mymoney/mymoneyfile.h"
#include "../views/kledgerview.h"

kMyMoneyRegisterCheckings::kMyMoneyRegisterCheckings(QWidget *parent, const char *name )
  : kMyMoneyRegister(3, parent,name)
{
  setNumCols(7);
  setCurrentCell(0, 1);
  horizontalHeader()->setClickEnabled(true);
  horizontalHeader()->setLabel(0, i18n("Nr."));
  horizontalHeader()->setLabel(1, i18n("Date"));
  horizontalHeader()->setLabel(2, i18n("Payee"));
  horizontalHeader()->setLabel(3, i18n("C"));
  horizontalHeader()->setLabel(4, i18n("Payment"));
  horizontalHeader()->setLabel(5, i18n("Deposit"));
  horizontalHeader()->setLabel(6, i18n("Balance"));
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

kMyMoneyRegisterCheckings::~kMyMoneyRegisterCheckings()
{
}

void kMyMoneyRegisterCheckings::paintCell(QPainter *p, int row, int col, const QRect& r,
                                 bool selected, const QColorGroup& cg)
{
  setTransactionRow(row);
  
  int align = Qt::AlignVCenter;
  QString txt;
  if(m_transaction != 0) {
    switch (col) {
      case 0:
        align |= Qt::AlignRight;
        switch(m_transactionRow) {
          case 0:
            txt = m_split.number();
            break;
        }
        break;

      case 1:
        align |= Qt::AlignLeft;
        switch(m_transactionRow) {
          case 0:
            txt = KGlobal::locale()->formatDate(m_transaction->postDate(), true);
            break;

          case 1:
            txt = m_split.action();
            break;
        }
        break;
        
      case 2:
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
              if(m_transaction->isLoanPayment()) {
                txt = QString(i18n("Loan payment"));
              } else if(m_transaction->splitCount() > 2)
                txt = QString(i18n("Splitted transaction"));
              else {
                MyMoneySplit split = m_transaction->splitByAccount(m_transaction->splitId(), false);
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
        break;
        
      case 3:
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
        break;

      case 4:
        switch(m_transactionRow) {
          case 0:
            align |= Qt::AlignRight;
            if(m_split.value() < 0)
              txt = (-m_split.value()).formatMoney();
            break;
        }
        break;
        
      case 5:
        switch(m_transactionRow) {
          case 0:
            align |= Qt::AlignRight;
            if(m_split.value() >= 0)
              txt = (m_split.value()).formatMoney();
            break;
        }
        break;
        
      case 6:
        switch(m_transactionRow) {
          case 0:
            align |= Qt::AlignRight;
            txt = m_balance.formatMoney();
            if(m_balance < 0)
              p->setPen(QColor(255, 0, 0));
            break;
        }
        break;
    }
  }
  
  // do general stuff
  kMyMoneyRegister::paintCell(p, row, col, r, selected, cg, txt, align);
}

void kMyMoneyRegisterCheckings::adjustColumn(int col)
{
  QHeader *topHeader = horizontalHeader();
  QFontMetrics fontMetrics(m_headerFont);

  int w = topHeader->fontMetrics().width( topHeader->label( col ) ) + 10;
  if ( topHeader->iconSet( col ) )
    w += topHeader->iconSet( col )->pixmap().width();
  w = QMAX( w, 20 );

  // scan through the transactions
  for ( int i = (numRows()/m_rpt)-1; i >= 0; --i ) {
    QString txt;
    KMyMoneyTransaction *t = m_parent->transaction(i);
    MyMoneyMoney amount;
    int nw;
    
    if(t != NULL) {
      switch(col) {
        default:
          break;

        case 0:
          txt = t->splitById(t->splitId()).number();
          nw = fontMetrics.width(txt);
          w = QMAX( w, nw );
          break;
        
        case 1:
          txt = KGlobal::locale()->formatDate(t->postDate(), true)+"  ";
          nw = fontMetrics.width(txt);
          w = QMAX( w, nw );
          break;
        
        case 4:
          amount = t->splitById(t->splitId()).value();
          if(amount < 0) {
            txt = amount.formatMoney();
            nw = fontMetrics.width(txt);
            w = QMAX( w, nw );
          }
          break;
          
        case 5:
          amount = t->splitById(t->splitId()).value();
          if(amount >= 0) {
            txt = amount.formatMoney();
            nw = fontMetrics.width(txt);
            w = QMAX( w, nw );
          }
          break;
          
      }
    }
  }
  setColumnWidth( col, w );
}

// This must be implemented here, as QTable::eventFilter is not virtual :-(

bool kMyMoneyRegisterCheckings::eventFilter(QObject* o, QEvent* e)
{
  return kMyMoneyRegister::eventFilter(o, e);
}
