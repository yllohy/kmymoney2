/***************************************************************************
                          stdtransactiondownloaded.cpp
                             -------------------
    begin                : Sun May 11 2008
    copyright            : (C) 2008 by Thomas Baumgart
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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <stdtransactiondownloaded.h>
#include <kmymoney/kmymoneyglobalsettings.h>
#include <kmymoney/register.h>

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

StdTransactionDownloaded::StdTransactionDownloaded(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  StdTransaction(parent, transaction, split, uniqueId)
{
}

bool StdTransactionDownloaded::paintRegisterCellSetup(QPainter* painter, int& row, int& col, QRect& cellRect, QRect& textRect, QColorGroup& cg, QBrush& brush)

{
  bool rc = Transaction::paintRegisterCellSetup(painter, row, col, cellRect, textRect, cg, brush);
  // if not selected paint in selected background color
  if(!isSelected()) {
    cg.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::importedTransactionColor());
    brush = QBrush(cg.base());
  }
  return rc;
}

InvestTransactionDownloaded::InvestTransactionDownloaded(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
    InvestTransaction(parent, transaction, split, uniqueId)
{
}

bool InvestTransactionDownloaded::paintRegisterCellSetup(QPainter* painter, int& row, int& col, QRect& cellRect, QRect& textRect, QColorGroup& cg, QBrush& brush)

{
  bool rc = Transaction::paintRegisterCellSetup(painter, row, col, cellRect, textRect, cg, brush);
  // if not selected paint in selected background color
  if(!isSelected()) {
    cg.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::importedTransactionColor());
    brush = QBrush(cg.base());
  }
  return rc;
}

