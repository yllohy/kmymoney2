/***************************************************************************
                          kendingbalancedlg.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KENDINGBALANCEDLG_H
#define KENDINGBALANCEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qdatetime.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../widgets/kmymoneyedit.h"
#include "../mymoney/mymoneymoney.h"
#include "../widgets/kmymoneydateinput.h"
#include "kendingbalancedlgdecl.h"

// This dialog lets the user selected an ending balance.
// It is designed to be used in conjunction with KReconcileDlg.
class KEndingBalanceDlg : public KEndingBalanceDlgDecl  {
   Q_OBJECT

public:
	KEndingBalanceDlg(const MyMoneyMoney& prevBal, const MyMoneyMoney& endingGuess, const QDate& statementDate, QWidget *parent=0, const char *name=0);
	~KEndingBalanceDlg();

  const MyMoneyMoney endingBalance(void) const { return m_endingBalance; };
  const MyMoneyMoney previousBalance(void) const { return m_previousBalance; };
  const QDate endingDate(void) const { return m_endingDate; };

protected:

protected slots:
  void okClicked();

private:
  MyMoneyMoney m_endingBalance;
  MyMoneyMoney m_previousBalance;
  QDate m_endingDate;
};

#endif
