/***************************************************************************
                          kmymoneyregistersearch.cpp  -  description
                             -------------------
    begin                : Sun Aug 10 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyregistersearch.h"
#include "../views/kledgerview.h"

kMyMoneyRegisterSearch::kMyMoneyRegisterSearch(QWidget *parent, const char *name )
 : kMyMoneyRegister(3, parent, name)
{
  setNumCols(6);
  setCurrentCell(0, 1);
  horizontalHeader()->setClickEnabled(true);
  horizontalHeader()->setLabel(0, i18n("Nr."));
  horizontalHeader()->setLabel(1, i18n("Date"));
  horizontalHeader()->setLabel(2, i18n("Account"));
  horizontalHeader()->setLabel(3, i18n("Payee"));
  horizontalHeader()->setLabel(4, i18n("Payment"));
  horizontalHeader()->setLabel(5, i18n("Deposit"));
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

kMyMoneyRegisterSearch::~kMyMoneyRegisterSearch()
{
}

void kMyMoneyRegisterSearch::paintCell(QPainter *p, int row, int col, const QRect& r,
                                 bool selected, const QColorGroup& cg)
{
  setTransactionRow(row);

  int align = Qt::AlignVCenter;
  QString txt = " ";

  // do general stuff
  kMyMoneyRegister::paintCell(p, row, col, r, selected, cg, txt, align);
}

// This must be implemented here, as QTable::eventFilter is not virtual :-(

bool kMyMoneyRegisterSearch::eventFilter(QObject* o, QEvent* e)
{
  return kMyMoneyRegister::eventFilter(o, e);
}

void kMyMoneyRegisterSearch::adjustColumn(int col)
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

      case 1: // date
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
