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

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kled.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "konlinebankingstatus.h"
#include <kmymoney/mymoneykeyvaluecontainer.h>
#include <kmymoney/mymoneyaccount.h>

KOnlineBankingStatus::KOnlineBankingStatus(const MyMoneyAccount& acc, QWidget *parent, const char *name) :
  KOnlineBankingStatusDecl(parent,name)
{
  m_ledOnlineStatus->off();
#ifdef USE_OFX_DIRECTCONNECT
  // Set up online banking settings if applicable
  MyMoneyKeyValueContainer settings = acc.onlineBankingSettings();
  m_textOnlineStatus->setText(i18n("STATUS: Enabled & configured"));
  m_ledOnlineStatus->on();

  QString account = i18n("ACCOUNT: %1").arg(settings.value("accountid"));
  QString bank = i18n("BANK/BROKER: %1").arg(settings.value("bankname"));
  QString bankid = QString("%1 %2").arg(settings.value("bankid")).arg(settings.value("branchid"));
  if ( bankid.length() > 1 )
    bank += QString(" (%1)").arg(bankid);
  m_textBank->setText(bank);
  m_textOnlineAccount->setText(account);

#else
  m_textOnlineStatus->setText(i18n("STATUS: Disabled.  No online banking services are available"));
#endif
}

KOnlineBankingStatus::~KOnlineBankingStatus()
{
}

#ifdef USE_OFX_DIRECTCONNECT
#endif

#include "konlinebankingstatus.moc"

