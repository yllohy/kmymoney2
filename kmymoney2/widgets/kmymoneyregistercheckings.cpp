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
  : kMyMoneyRegister(parent,name)
{
}

kMyMoneyRegisterCheckings::~kMyMoneyRegisterCheckings()
{
}

void kMyMoneyRegisterCheckings::paintCell(QPainter *p, int row, int col, const QRect& r,
                                 bool selected, const QColorGroup& cg)
{
  // do general stuff
  kMyMoneyRegister::paintCell(p, row, col, r, selected, cg);

  const bool lastLine = m_transactionRow == m_rpt-1;
  int align = Qt::AlignVCenter;

  // if a grid is selected, we paint it right away
  if (m_showGrid) {
    p->setPen(m_gridColor);
    p->drawLine(m_cellRect.x(), 0, m_cellRect.x(), m_cellRect.height()-1);
    if(lastLine)
      p->drawLine(m_cellRect.x(), m_cellRect.height()-1, m_cellRect.width(), m_cellRect.height()-1);
    p->setPen(m_textColor);
  }

  // if we paint something, that we don't know (yet), we're done
  // this applies to the very last line of the ledger which always
  // shows an empty line for new transactions to be added.
  if(m_transaction == NULL)
    return;

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

/*
      // now do the painting
      if (m_showGrid) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 0, m_cellRect.x(), m_cellRect.height()-1);
        if(lastLine)
          p->drawLine(m_cellRect.x(), m_cellRect.height()-1, m_cellRect.width(), m_cellRect.height()-1);
        p->setPen(m_textColor);
      }
*/
/*
      if(m_transactionRow > 0 && col == 2) {
        int intMemoStart = m_cellRect.width() / 2;
        rr3.setX(2);
        rr3.setY(0);
        rr3.setWidth(intMemoStart-4);
        rr3.setHeight(rowHeight(row));
        // p->drawText(rr3,Qt::AlignLeft | Qt::AlignVCenter,qstringCategory);
        if(m_showGrid) {
          p->setPen(m_gridColor);
          p->drawLine(intMemoStart,0,intMemoStart,m_cellRect.height()-1);
          p->setPen(m_cg.foreground());
        }
        rr3.setX(intMemoStart + 2);
        rr3.setWidth(intMemoStart-4);
        // p->drawText(rr3,Qt::AlignLeft | Qt::AlignVCenter,qstringMemo);

      } else {
*/
        p->drawText(m_textRect, align, txt);
//      }
      if(row == m_currentDateRow) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 1, m_cellRect.width(), 1);
        p->setPen(m_textColor);
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
      if(row == m_currentDateRow) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 1, m_cellRect.width(), 1);
        p->setPen(m_textColor);
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
            break;

          case 1:
          case 2:
            txt = " ";
            break;
        }
      }
/*
      if (m_showGrid) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 0, m_cellRect.x(), m_cellRect.height()-1);
        if(lastLine)
          p->drawLine(m_cellRect.x(), m_cellRect.height()-1, m_cellRect.width(), m_cellRect.height()-1);
        p->drawLine(m_cellRect.x()+m_cellRect.width(), 0, m_cellRect.x()+m_cellRect.width(), m_cellRect.height()-1);
        p->setPen(m_textColor);
      }
*/
      p->drawText(m_textRect, Qt::AlignRight | Qt::AlignVCenter, txt);
      if(row == m_currentDateRow) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 1, m_cellRect.width(), 1);
        p->setPen(m_textColor);
      }
      break;
  }
}

void kMyMoneyRegisterCheckings::adjustColumn(int col)
{
  QHeader *topHeader = horizontalHeader();
  QFontMetrics fontMetrics(m_font);

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
        MyMoneyTransaction *t = m_view->transaction(i);
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

QWidget* kMyMoneyRegisterCheckings::createEditor(int row, int col, bool initFromCell) const
{
  if(!m_inlineEditAvailable)
    return 0;

  return kMyMoneyRegister::createEditor(row, col, initFromCell);
}

// This must be implemented here, as QTable::eventFilter is not virtual :-(

bool kMyMoneyRegisterCheckings::eventFilter(QObject* o, QEvent* e)
{
  return kMyMoneyRegister::eventFilter(o, e);
}
