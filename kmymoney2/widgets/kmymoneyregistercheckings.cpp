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

  // now the specific stuff for checking accounts
  QRect rr3 = r;

  QString txt;

  switch (col) {
    case 0:
      txt = " "; // for now keep it empty
      // txt = m_transaction->???
      // tricky fall through here!

    case 1:
      if(txt.isEmpty()) {
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
                MyMoneyAccount account = MyMoneyFile::instance()->account(split.accountId());
                txt = account.name();
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

      // now do the painting
      if (m_showGrid) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 0, m_cellRect.x(), m_cellRect.height()-1);
        if(lastLine)
          p->drawLine(m_cellRect.x(), m_cellRect.height()-1, m_cellRect.width(), m_cellRect.height()-1);
        p->setPen(m_cg.foreground());
      }
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
        p->drawText(m_textRect, Qt::AlignLeft | Qt::AlignVCenter, txt);
//      }
      if(row == m_currentDateRow) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 1, m_cellRect.width(), 1);
        p->setPen(m_cg.foreground());
      }
      break;
    case 3:
      if (m_showGrid) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 0, m_cellRect.x(), m_cellRect.height()-1);
        if(lastLine)
          p->drawLine(m_cellRect.x(), m_cellRect.height()-1, m_cellRect.width(), m_cellRect.height()-1);
        p->setPen(m_cg.foreground());
      }
      if(txt.isEmpty()) {
        switch(m_transactionRow) {
          case 0:
            break;
        }
      }
      p->drawText(m_textRect, Qt::AlignCenter | Qt::AlignVCenter, txt);
      if(row == m_currentDateRow) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 1, m_cellRect.width(), 1);
        p->setPen(m_cg.foreground());
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
      if (m_showGrid) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 0, m_cellRect.x(), m_cellRect.height()-1);
        if(lastLine)
          p->drawLine(m_cellRect.x(), m_cellRect.height()-1, m_cellRect.width(), m_cellRect.height()-1);
        p->drawLine(m_cellRect.x()+m_cellRect.width(), 0, m_cellRect.x()+m_cellRect.width(), m_cellRect.height()-1);
        p->setPen(m_cg.foreground());
      }
      p->drawText(m_textRect, Qt::AlignRight | Qt::AlignVCenter, txt);
      if(row == m_currentDateRow) {
        p->setPen(m_gridColor);
        p->drawLine(m_cellRect.x(), 1, m_cellRect.width(), 1);
        p->setPen(m_cg.foreground());
      }
      break;
  }
}

void kMyMoneyRegisterCheckings::adjustColumn(int col)
{
  QHeader *topHeader = horizontalHeader();

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
        txt = KGlobal::locale()->formatDate(m_view->transaction(i)->postDate(), true);
        w = QMAX( w, fontMetrics().width(txt) );
        break;
    }
  }
  setColumnWidth( col, w );
}

