/***************************************************************************
                          kmymoneycurrencyselector.h  -  description
                             -------------------
    begin                : Tue Apr 6 2004
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

#ifndef KMYMONEYCURRENCYSELECTOR_H
#define KMYMONEYCURRENCYSELECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyobserver.h"

/**
  * @author Thomas Baumgart
  */

class kMyMoneyCurrencySelector : public KComboBox, MyMoneyObserver
{
  Q_OBJECT
public: 
  kMyMoneyCurrencySelector(QWidget *parent=0, const char *name=0);
  ~kMyMoneyCurrencySelector();

  const MyMoneyCurrency currency(void) const;
  void setCurrency(const MyMoneyCurrency& currency);

  void update(const QCString& id);
  
private:
  MyMoneyCurrency   m_currency;
};

#endif
