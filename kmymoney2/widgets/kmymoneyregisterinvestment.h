/***************************************************************************
                          kmymoneyregisterinvestment.h  -  description
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

#ifndef KMYMONEYREGISTERINVESTMENT_H
#define KMYMONEYREGISTERINVESTMENT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../widgets/kmymoneyregister.h"
#include "../mymoney/mymoneysecurity.h"

/**
  * @author Thomas Baumgart
  */

class kMyMoneyRegisterInvestment : public kMyMoneyRegister
{
   Q_OBJECT
public:
  kMyMoneyRegisterInvestment(QWidget *parent=0, const char *name=0);
  ~kMyMoneyRegisterInvestment();

  bool eventFilter(QObject* o, QEvent* e);

  virtual const int maxRpt(void) const { return 3; };

public slots:
  void adjustColumn(int col);

protected:
  void paintCell(QPainter *p, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);

private:
  /**
    * This member variable holds the id of the last transaction processed.
    */
  QCString        m_lastTransactionId;
  MyMoneySplit    m_accountSplit;
  MyMoneySplit    m_feeSplit;
  MyMoneySplit    m_interestSplit;
  MyMoneySecurity m_security;
};

#endif
