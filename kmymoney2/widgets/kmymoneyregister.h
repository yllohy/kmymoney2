/***************************************************************************
                          kmymoneyregister.h  -  description
                             -------------------
    begin                : Sun Jul 14 2002
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

#ifndef KMYMONEYREGISTER_H
#define KMYMONEYREGISTER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qpainter.h>
#include <qtable.h>
// #include <qfontmetric.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneytransaction.h"
#include "../views/kledgerview.h"

/**
  *@author Thomas Baumgart
  */

class KLedgerView;

class kMyMoneyRegister : public QTable  {
  Q_OBJECT

  friend class QHeader;
  friend class QTableHeader;

public:
	kMyMoneyRegister(QWidget *parent=0, const char *name=0);
	virtual ~kMyMoneyRegister();

  void setView(KLedgerView *view) { m_view = view; };

  /**
    * read setting of configuration from config file
    */
  void readConfig(void);

  void clearCell(int row, int col) {};
  void setItem(int row, int col, QTableItem *) {};
  QTableItem* item(int row, int col) const  { return NULL; };

  void clearCellWidget(int row, int col) {};
  QWidget* cellWidget(int row, int col) const { return NULL; };

public slots:
  /**
    * This method is used to inform the widget about the number
    * of transactions to be displayed.
    *
    * @param transactions number of transactions in this account
    */
  void setTransactionCount(int transactions);

  void setNumRows(int r);

protected:
  void paintCell(QPainter *p, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);

  /**
    * resizeData is re-implemented to avoid creation
    * of huge arrays that are not required
    */
  void resizeData(int len) {};

  void insertWidget(int row, int col, QWidget *) {};

protected:
  int    m_rpt;     // rows per transaction
  KLedgerView*  m_view;

  QFont  m_font;

  QColorGroup m_cg;
  QColor m_color;
  QColor m_textColor;
  QColor m_bgColor;
  QColor m_gridColor;

  bool   m_showGrid;
  bool   m_colorPerTransaction;

  QRect  m_cellRect;
  QRect  m_textRect;

  int    m_lastTransactionIndex;
  int    m_transactionIndex;
  int    m_transactionRow;
  int    m_currentDateRow;
  int    m_currentTransactionRow;

  MyMoneyTransaction const * m_transaction;
  MyMoneyMoney m_balance;
  MyMoneySplit m_split;
};

#endif
