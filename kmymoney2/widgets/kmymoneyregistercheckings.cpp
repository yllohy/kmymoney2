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
  QString txt = " ";
  if(m_transaction != 0) {
    switch (col) {
      case 0:
        align |= Qt::AlignRight;
        switch(m_transactionRow) {
          case 0:
            txt = m_split.number();
            if(txt.isEmpty())
              txt = " ";
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
            if(txt.isEmpty())
              txt = " ";
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
              if(m_transaction->splitCount() > 2)
                txt = QString(i18n("Splitted transaction"));
              else {
                MyMoneySplit split = m_transaction->split(m_parent->accountId(m_transaction), false);
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
/*
  // do general stuff
  kMyMoneyRegister::paintCell(p, row, col, r, selected, cg, cellTxt, cellAlign);

  const bool lastLine = m_ledgerLens && m_transactionIndex == m_currentTransactionIndex
                         ? m_transactionRow == maxRpt() - 1
                         : m_transactionRow == m_rpt-1;

  int align = Qt::AlignVCenter;

  // if a grid is selected, we paint it right away
  if (m_showGrid) {
    p->setPen(m_gridColor);
    p->drawLine(m_cellRect.x(), 0, m_cellRect.x(), m_cellRect.height()-1);
    if(lastLine)
      p->drawLine(m_cellRect.x(), m_cellRect.height()-1, m_cellRect.width(), m_cellRect.height()-1);
  }

  // if we paint something, that we don't know (yet), we're done
  // this applies to the very last line of the ledger which always
  // shows an empty line for new transactions to be added.
  // In case this is the current date row, we still draw the marker
  if(m_transaction == NULL) {
    if(m_transactionIndex == m_currentDateIndex && m_transactionRow == 0) {
      p->setPen(m_gridColor);
      p->drawLine(m_cellRect.x(), 0, m_cellRect.width(), 0);
      p->drawLine(m_cellRect.x(), 1, m_cellRect.width(), 1);
    }
    return;
  }

  QColor textColor(m_textColor);
  // if it's an erronous transaction, set it to error color (which toggles ;-)  )
  if(m_transaction->splitCount() < 2
  || m_transaction->splitSum() != 0) {
    textColor = m_errorColor;
  }
  p->setPen(textColor);

  // now the specific stuff for checking accounts

  QString txt;

  switch (col) {
    case 0:
      align |= Qt::AlignRight;
      switch(m_transactionRow) {
        case 0:
          txt = m_split.number();
          if(txt.isEmpty())
            txt = " ";
          break;
        case 1:
        case 2:
          txt = " "; // for now keep it empty
          break;
      }
      // tricky fall through here!

    case 1:
      if(txt.isEmpty()) {
        align |= Qt::AlignLeft;
        switch(m_transactionRow) {
          case 0:
            txt = KGlobal::locale()->formatDate(m_transaction->postDate(), true);
            break;

          case 1:
          txt = m_split.action();
          if(txt.isEmpty())
            txt = " ";
          break;

          case 2:
            txt = " ";
            break;
        }
      }
      // tricky fall through here!

    case 2:
      if(txt.isEmpty()) {
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
              if(m_transaction->splitCount() > 2)
                txt = QString(i18n("Splitted transaction"));
              else {
                MyMoneySplit split = m_transaction->split(m_view->accountId(), false);
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
      }
      p->drawText(m_textRect, align, txt);
      if(m_transactionIndex == m_currentDateIndex && m_transactionRow == 0) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 0, m_cellRect.width(), 0);
        p->drawLine(m_cellRect.x(), 1, m_cellRect.width(), 1);
        p->setPen(textColor);
      }
      break;
    case 3:
      if(txt.isEmpty()) {
        txt = " ";
        switch(m_transactionRow) {
          case 0:
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
      }
      p->drawText(m_textRect, Qt::AlignCenter | Qt::AlignVCenter, txt);
      if(m_transactionIndex == m_currentDateIndex && m_transactionRow == 0) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 0, m_cellRect.width(), 0);
        p->drawLine(m_cellRect.x(), 1, m_cellRect.width(), 1);
        p->setPen(textColor);
      }
      break;

    case 4:
        switch(m_transactionRow) {
          case 0:
            if(m_split.value() < 0)
              txt = (-m_split.value()).formatMoney();
            else
              txt = " ";    // make sure cell stays empty
            break;

          case 1:
          case 2:
            txt = " ";
            break;
        }
      // tricky fall through here!

    case 5:
      if(txt.isEmpty()) {
        switch(m_transactionRow) {
          case 0:
            if(m_split.value() >= 0)
              txt = (m_split.value()).formatMoney();
            else
              txt = " ";    // make sure cell stays empty
            break;

          case 1:
          case 2:
            txt = " ";
            break;
        }
      }
      // tricky fall through here!
    case 6:
      if(txt.isEmpty()) {
        switch(m_transactionRow) {
          case 0:
            txt = m_balance.formatMoney();
            if(m_balance < 0)
              p->setPen(QColor(255, 0, 0));
            break;

          case 1:
          case 2:
            txt = " ";
            break;
        }
      }
      p->drawText(m_textRect, Qt::AlignRight | Qt::AlignVCenter, txt);
      if(m_transactionIndex == m_currentDateIndex && m_transactionRow == 0) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 0, m_cellRect.width(), 0);
        p->drawLine(m_cellRect.x(), 1, m_cellRect.width(), 1);
        p->setPen(textColor);
      }
      break;
  }
*/
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
    switch(col) {
      default:
        break;

      case 1:
        QString txt;
        MyMoneyTransaction *t = m_parent->transaction(i);
        if(t != NULL) {
          txt = KGlobal::locale()->formatDate(t->postDate(), true)+"  ";
          int nw = fontMetrics.width(txt);
          w = QMAX( w, nw );
        }
        break;
    }
  }
  setColumnWidth( col, w );
}

// This must be implemented here, as QTable::eventFilter is not virtual :-(

bool kMyMoneyRegisterCheckings::eventFilter(QObject* o, QEvent* e)
{
  return kMyMoneyRegister::eventFilter(o, e);
}
