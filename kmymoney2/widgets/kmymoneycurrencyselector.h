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

#include <kmymoney/mymoneyfile.h>

/**
  * @author Thomas Baumgart
  */

class KMyMoneySecuritySelector : public KComboBox
{
  Q_OBJECT
public:
  enum displayItemE {
    Symbol = 0,
    FullName
  };

  enum displayTypeE {
    TypeCurrencies = 0x01,
    TypeSecurities = 0x02,
    TypeAll        = 0x03
  };

  KMyMoneySecuritySelector(QWidget *parent=0, const char *name=0);
  KMyMoneySecuritySelector(displayTypeE type = TypeAll, QWidget *parent=0, const char *name=0);
  virtual ~KMyMoneySecuritySelector();

  const MyMoneySecurity& security(void) const;
  void setSecurity(const MyMoneySecurity& currency);
  void selectDisplayItem(KMyMoneySecuritySelector::displayItemE item);
  void setDisplayOnly(const bool disp);

  void update(const QCString& id);

public slots:
  void slotSetInitialSecurity(void);

private:
  MyMoneySecurity m_currency;
  displayItemE    m_displayItem;
  int             m_selectedItemId;
  bool            m_displayOnly;
  displayTypeE    m_displayType;
  QValueList<MyMoneySecurity> m_list;
};

class KMyMoneyCurrencySelector : public KMyMoneySecuritySelector
{
public:
  KMyMoneyCurrencySelector(QWidget *parent=0, const char *name=0);
  virtual ~KMyMoneyCurrencySelector() {}
};

#endif
