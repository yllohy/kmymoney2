/***************************************************************************
                             currency.h
                             -------------------
    begin                : Fri Jun  1 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CURRENCY_H
#define CURRENCY_H

// ----------------------------------------------------------------------------
// QT Includes

class KListViewItem;

// ----------------------------------------------------------------------------
// Project Includes

#include "currencydecl.h"
#include <kmymoney/mymoneysecurity.h>

/**
  * @author Thomas Baumgart
  */
class Currency : public CurrencyDecl
{
  Q_OBJECT
public:
  Currency(QWidget* parent = 0, const char* name = 0);
  QListViewItem* insertCurrency(const MyMoneySecurity& sec);
  void selectCurrency(const MyMoneySecurity& sec);
  QString selectedCurrency(void) const;
};

#endif
