/***************************************************************************
                          kmymoneyfile.cpp  -  description
                             -------------------
    begin                : Mon Jun 10 2002
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
#include <klocale.h>
#include "kmymoneyfile.h"

KMyMoneyFile *KMyMoneyFile::m_instance = 0L;
MyMoneyFile *KMyMoneyFile::m_file = 0L;
MyMoneySeqAccessMgr *KMyMoneyFile::m_storage = 0L;
bool KMyMoneyFile::m_open = false;

KMyMoneyFile::KMyMoneyFile()
{
}

KMyMoneyFile::KMyMoneyFile(const QString&)
{
}

KMyMoneyFile::~KMyMoneyFile()
{
}

KMyMoneyFile *KMyMoneyFile::instance()
{
  if (!m_instance)
  {
    m_instance = new KMyMoneyFile;
    m_instance->m_storage = new MyMoneySeqAccessMgr;
    m_instance->m_file = new MyMoneyFile(m_storage);
  }

  return m_instance;
}

MyMoneyFile* KMyMoneyFile::file()
{
  return m_file;
}

MyMoneySeqAccessMgr* KMyMoneyFile::storage()
{
  return m_storage;
}

void KMyMoneyFile::reset()
{
  delete m_instance->m_storage;
  delete m_instance->m_file;
  m_instance->m_storage = new MyMoneySeqAccessMgr;
  m_instance->m_file = new MyMoneyFile(m_storage);
}

QString KMyMoneyFile::accountTypeToString(MyMoneyAccount::accountTypeE accountType)
{
  QString returnString;

  // FIXME: We need to localise these strings
  switch (accountType)
  {
    case MyMoneyAccount::Checkings:
      returnString = i18n("Checkings");
      break;
    case MyMoneyAccount::Savings:
      returnString = i18n("Savings");
      break;
    case MyMoneyAccount::Cash:
      returnString = i18n("Cash");
      break;
    case MyMoneyAccount::CertificateDep:
      returnString = i18n("Certificate of Deposit");
      break;
    case MyMoneyAccount::Investment:
      returnString = i18n("Investment");
      break;
    case MyMoneyAccount::MoneyMarket:
      returnString = i18n("Money Market");
      break;
    case MyMoneyAccount::Asset:
      returnString = i18n("Asset");
      break;
    case MyMoneyAccount::Liability:
      returnString = i18n("Liability");
      break;
    case MyMoneyAccount::Currency:
      returnString = i18n("Currency");
      break;
    case MyMoneyAccount::Income:
      returnString = i18n("Income");
      break;
    case MyMoneyAccount::Expense:
      returnString = i18n("Expense");
      break;
    default:
      returnString = i18n("Unknown");
  }

  return returnString;
}

void KMyMoneyFile::open()
{
  reset();
  m_open = true;
}

void KMyMoneyFile::close()
{
  reset();
  m_open = false;
}

bool KMyMoneyFile::isOpen()
{
  /* keeping this debug here for now because it
    highlights the infinite loop bug on exit
  */
  qDebug("returning open %d", m_open);
  return m_open;
}
