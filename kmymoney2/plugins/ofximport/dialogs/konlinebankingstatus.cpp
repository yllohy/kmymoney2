/***************************************************************************
                          konlinebankingstatus.cpp
                             -------------------
    begin                : Wed Apr 16 2008
    copyright            : (C) 2008 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


// ----------------------------------------------------------------------------
// System Includes

// ----------------------------------------------------------------------------
// QT Includes

#include <qlabel.h>
#include <qpushbutton.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kled.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "konlinebankingstatus.h"
#include <kmymoney/mymoneykeyvaluecontainer.h>
#include <kmymoney/mymoneyaccount.h>
#include "mymoneyofxconnector.h"  // for definition of LIBOFX_IS_VERSION

static QMap<QString, QString> appMap;
static QMap<QString, QString> verMap;
static QString defaultAppId("QWIN:1700");

KOnlineBankingStatus::KOnlineBankingStatus(const MyMoneyAccount& acc, QWidget *parent, const char *name) :
  KOnlineBankingStatusDecl(parent,name)
{
  m_ledOnlineStatus->off();
#ifdef USE_OFX_DIRECTCONNECT
// http://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-intuit-products/
// http://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-microsoft-money/

  // Quicken
  appMap[i18n("Quicken Windows 2005")] = "QWIN:1400";
  appMap[i18n("Quicken Windows 2006")] = "QWIN:1500";
  appMap[i18n("Quicken Windows 2007")] = "QWIN:1600";
  appMap[i18n("Quicken Windows 2008")] = "QWIN:1700";

  // MS-Money
  appMap[i18n("MS-Money 2003")] = "Money:1100";
  appMap[i18n("MS-Money 2004")] = "Money:1200";
  appMap[i18n("MS-Money 2005")] = "Money:1400";
  appMap[i18n("MS-Money 2006")] = "Money:1500";
  appMap[i18n("MS-Money 2007")] = "Money:1600";
  appMap[i18n("MS-Money Plus")] = "Money:1700";

  // Set up online banking settings if applicable
  MyMoneyKeyValueContainer settings = acc.onlineBankingSettings();
  m_textOnlineStatus->setText(i18n("Enabled & configured"));
  m_ledOnlineStatus->on();

  QString account = settings.value("accountid");
  QString bank = settings.value("bankname");
  QString bankid = QString("%1 %2").arg(settings.value("bankid")).arg(settings.value("branchid"));
  if ( bankid.length() > 1 )
    bank += QString(" (%1)").arg(bankid);
  m_textBank->setText(bank);
  m_textOnlineAccount->setText(account);

  QString appId = settings.value("appId");
  // default to Quicken 2008
  if(appId.isEmpty()) {
    appId = defaultAppId;
  }

  loadApplicationButton(appId);
  
#else
  m_textOnlineStatus->setText(i18n("Disabled. No online banking services are available"));
#endif
}

KOnlineBankingStatus::~KOnlineBankingStatus()
{
}

void KOnlineBankingStatus::loadApplicationButton(const QString& appId)
{
  m_applicationCombo->clear();
  m_applicationCombo->insertStringList(appMap.keys());

  QMap<QString, QString>::const_iterator it_a;
  for(it_a = appMap.begin(); it_a != appMap.end(); ++it_a) {
    if(*it_a == appId)
      break;
  }
  if(it_a != appMap.end()) {
    m_applicationCombo->setCurrentItem(it_a.key());
  }

#if LIBOFX_IS_VERSION(0,9,0)
#else
  // This feature does not work with libOFX < 0.9 so
  // we just make disable the button in this case
  m_applicationCombo->setDisabled(true);
#endif
}

const QString& KOnlineBankingStatus::appId(void) const
{
  QString app = m_applicationCombo->currentText();
  if(appMap[app] != defaultAppId)
    return appMap[app];
  return QString::null;
}

#ifdef USE_OFX_DIRECTCONNECT
#endif

#include "konlinebankingstatus.moc"

