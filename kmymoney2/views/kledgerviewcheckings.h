/***************************************************************************
                          kledgerviewcheckings.h  -  description
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

#ifndef KLEDGERVIEWCHECKINGS_H
#define KLEDGERVIEWCHECKINGS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qtabbar.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerview.h"
#include "../widgets/kmymoneyregistercheckings.h"

/**
  *@author Thomas Baumgart
  */

class KLedgerViewCheckings : public KLedgerView  {
   Q_OBJECT
public: 
	KLedgerViewCheckings(QWidget *parent=0, const char *name=0);
	~KLedgerViewCheckings();

  void show();

  void fillForm(void);

public slots:
  /**
    * refresh the current view
    */
  virtual void refreshView(void);

  void slotTypeSelected(int transactionType);

protected:
  void resizeEvent(QResizeEvent*);

private:
  int transactionType(const MyMoneySplit& split) const;

private:
  QTab* m_tabCheck;
  QTab* m_tabDeposit;
  QTab* m_tabTransfer;
  QTab* m_tabWithdrawal;
  QTab* m_tabAtm;
};

#endif
