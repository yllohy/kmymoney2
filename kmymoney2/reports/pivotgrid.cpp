/***************************************************************************
                          pivotgrid.cpp
                             -------------------
    begin                : Mon May 17 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : <ace.j@hotpop.com>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
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
#include <qlayout.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <qdragobject.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qfile.h>
#include <qdom.h>

// ----------------------------------------------------------------------------
// KDE Includes
// This is just needed for i18n() and weekStartDay().
// Once I figure out how to handle i18n
// without using this macro directly, I'll be freed of KDE dependency.  This
// is a minor problem because we use these terms when rendering to HTML,
// and a more major problem because we need it to translate account types
// (e.g. MyMoneyAccount::Checkings) into their text representation.  We also
// use that text representation in the core data structure of the report. (Ace)

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcalendarsystem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "pivottable.h"
#include "reportdebug.h"
#include "kreportchartview.h"
#include "../kmymoneyglobalsettings.h"

#include <kmymoney/kmymoneyutils.h>

namespace reports {

  const unsigned PivotOuterGroup::m_kDefaultSortOrder = 100;

PivotCell PivotCell::operator += (const PivotCell& right)
{
  const MyMoneyMoney& r = static_cast<const MyMoneyMoney&>(right);
  *this += r;
  m_postSplit = m_postSplit * right.m_stockSplit;
  m_stockSplit = m_stockSplit * right.m_stockSplit;
  m_postSplit += right.m_postSplit;
  m_cellUsed |= right.m_cellUsed;
  return *this;
}

PivotCell PivotCell::operator += (const MyMoneyMoney& value)
{
  m_cellUsed |= !value.isZero();
  if(m_stockSplit != MyMoneyMoney(1,1))
    m_postSplit += value;
  else
    MyMoneyMoney::operator += (value);
  return *this;
}

PivotCell PivotCell::stockSplit(const MyMoneyMoney& factor)
{
  PivotCell s;
  s.m_stockSplit = factor;
  return s;
}

const QString PivotCell::formatMoney(int fraction, bool showThousandSeparator) const
{
  return formatMoney("", MyMoneyMoney::denomToPrec(fraction), showThousandSeparator);
}

const QString PivotCell::formatMoney(const QString& currency, const int prec, bool showThousandSeparator) const
{
  // construct the result
  MyMoneyMoney res = (*this * m_stockSplit) + m_postSplit;
  return res.formatMoney(currency, prec, showThousandSeparator);
}

MyMoneyMoney PivotCell::calculateRunningSum(const MyMoneyMoney& runningSum)
{
  MyMoneyMoney::operator += (runningSum);
  MyMoneyMoney::operator = ((*this * m_stockSplit) + m_postSplit);
  m_postSplit = MyMoneyMoney(0,1);
  m_stockSplit = MyMoneyMoney(1,1);
  return *this;
}

MyMoneyMoney PivotCell::cellBalance(const MyMoneyMoney& _balance)
{
  MyMoneyMoney balance(_balance);
  balance += *this;
  balance = (balance * m_stockSplit) + m_postSplit;
  return balance;
}




} // namespace
// vim:cin:si:ai:et:ts=2:sw=2:
