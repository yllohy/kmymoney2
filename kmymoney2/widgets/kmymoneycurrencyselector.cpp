/***************************************************************************
                          kmymoneycurrencyselector.cpp  -  description
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qpixmap.h>
#include <qbitmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycurrencyselector.h"

KMyMoneySecuritySelector::KMyMoneySecuritySelector(QWidget *parent, const char *name ) :
  KComboBox(parent, name),
  m_displayItem(FullName),
  m_displayOnly(false),
  m_displayType(TypeAll)
{
  // update(QString());
}

KMyMoneySecuritySelector::KMyMoneySecuritySelector(displayTypeE type, QWidget *parent, const char *name ) :
  KComboBox(parent,name),
  m_displayItem(FullName),
  m_displayOnly(false),
  m_displayType(type)
{
  // update(QString());
}

KMyMoneySecuritySelector::~KMyMoneySecuritySelector()
{
}

void KMyMoneySecuritySelector::selectDisplayItem(KMyMoneySecuritySelector::displayItemE item)
{
  m_displayItem = item;
  update(QString());
}

void KMyMoneySecuritySelector::update(const QString& id)
{
  MyMoneySecurity curr = MyMoneyFile::instance()->baseCurrency();
  QString baseCurrency = curr.id();

  if(!id.isEmpty())
    curr = m_currency;

  this->clear();
  m_list.clear();
  if(m_displayType & TypeCurrencies)
    m_list += MyMoneyFile::instance()->currencyList();
  if(m_displayType & TypeSecurities)
    m_list += MyMoneyFile::instance()->securityList();

  // sort
  qHeapSort(m_list);

  QValueList<MyMoneySecurity>::ConstIterator it;

  // construct a transparent 16x16 pixmap
  QPixmap empty(16, 16);
  empty.setMask(QBitmap(16, 16, true));

  int itemId = 0;
  int m_selectedItemId = 0;
  for(it = m_list.begin(); it != m_list.end(); ++it) {
    QString display;
    switch(m_displayItem) {
      default:
      case FullName:
        if((*it).isCurrency()) {
          display = QString("%2 (%1)").arg((*it).id()).arg((*it).name());
        } else
          display = QString("%2 (%1)").arg((*it).tradingSymbol()).arg((*it).name());
        break;
        break;

      case Symbol:
        if((*it).isCurrency())
          display = (*it).id();
        else
          display = (*it).tradingSymbol();
        break;
    }
    if((*it).id() == baseCurrency) {
      insertItem(QPixmap( locate("icon","hicolor/16x16/apps/kmymoney2.png")),
                          display, itemId);
    } else {
      insertItem(empty, display, itemId);
    }

    if(curr.id() == (*it).id()) {
      m_selectedItemId = itemId;
      m_currency = (*it);
    }

    itemId++;
  }
  setCurrentItem(m_selectedItemId);
}

void KMyMoneySecuritySelector::setDisplayOnly(const bool disp)
{
  if(disp == m_displayOnly)
    return;

  switch(disp) {
    case true:
      connect(this, SIGNAL(activated(int)), this, SLOT(slotSetInitialCurrency()));
      break;
    case false:
      disconnect(this, SIGNAL(activated(int)), this, SLOT(slotSetInitialCurrency()));
      break;
  }
  m_displayOnly = disp;
}

void KMyMoneySecuritySelector::slotSetInitialSecurity(void)
{
  setCurrentItem(m_selectedItemId);
}

const MyMoneySecurity& KMyMoneySecuritySelector::security(void) const
{
  return m_list[currentItem()];
}

void KMyMoneySecuritySelector::setSecurity(const MyMoneySecurity& currency)
{
  m_currency = currency;
  update(QString("x"));
}

KMyMoneyCurrencySelector::KMyMoneyCurrencySelector(QWidget *parent, const char *name ) :
  KMyMoneySecuritySelector(TypeCurrencies, parent, name)
{
}

#include "kmymoneycurrencyselector.moc"
