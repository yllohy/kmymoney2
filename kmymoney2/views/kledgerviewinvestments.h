/***************************************************************************
                          kledgerviewinvestments.h  -  description
                             -------------------
    begin                : Sun Mar 7 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#ifndef KLEDGERVIEWINVESTMENTS_H
#define KLEDGERVIEWINVESTMENTS_H

#include <kledgerview.h>

/**
  *@author Kevin Tambascio
  */

class KLedgerViewInvestments : public KLedgerView
{
  Q_OBJECT
public: 
  KLedgerViewInvestments(QWidget *parent = NULL, const char *name = NULL);
  ~KLedgerViewInvestments();

protected slots:
  virtual void slotReconciliation();

protected:
  virtual void fillForm();
  virtual void fillSummary();
  virtual void showWidgets();
  virtual void hideWidgets();
  virtual void reloadEditWidgets(const MyMoneyTransaction& t);
};

#endif
