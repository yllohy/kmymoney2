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

/*
 * This file is currently not used anymore, but kept here for reference purposes
 */
#if 0

#include <klocale.h>

#include "kmymoneyfile.h"
#include "../mymoney/storage/mymoneyseqaccessmgr.h"

KMyMoneyFile::KMyMoneyFile()
{
  // m_file = MyMoneyFile::instance();
  m_storage = new MyMoneySeqAccessMgr;
  // m_file->attachStorage(m_storage);
  m_open = false;  // lie a little bit for now
}

/*
KMyMoneyFile::KMyMoneyFile(const QString&)
{
}
*/

KMyMoneyFile::~KMyMoneyFile()
{
  if(m_storage) {
    MyMoneyFile::instance()->detachStorage(m_storage);
    delete m_storage;
  }

  // if(m_file)
  //   delete m_file;
}

/*
KMyMoneyFile *KMyMoneyFile::instance()
{
  if (_instance == 0) {
    _instance = new KMyMoneyFile;
  }

  return _instance;
}

MyMoneyFile* KMyMoneyFile::file()
{
  return m_file;
}
*/

MyMoneySeqAccessMgr* KMyMoneyFile::storage()
{
  return m_storage;
}

void KMyMoneyFile::reset()
{
/*
  delete m_storage;
  delete m_file;
  m_storage = new MyMoneySeqAccessMgr;
  m_file = new MyMoneyFile(m_storage);
*/
}

void KMyMoneyFile::open()
{
  if(m_storage != 0)
    close();

  m_storage = new MyMoneySeqAccessMgr;
  MyMoneyFile::instance()->attachStorage(m_storage);
  m_open = true;
}

void KMyMoneyFile::close()
{
  if(m_storage != 0) {
    MyMoneyFile::instance()->detachStorage(m_storage);
    delete m_storage;
    m_storage = 0;
  }
  m_open = false;
}

bool KMyMoneyFile::isOpen()
{
  return m_open;
}

const QString KMyMoneyFile::accountTypeToString(const MyMoneyAccount::accountTypeE accountType)
{
  QString returnString;

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

const MyMoneyAccount::accountTypeE KMyMoneyFile::stringToAccountType(const QString& type)
{
  MyMoneyAccount::accountTypeE rc = MyMoneyAccount::UnknownAccountType;

  if(type == i18n("Checkings"))
    rc = MyMoneyAccount::Checkings;
  else if(type == i18n("Savings"))
    rc = MyMoneyAccount::Savings;
  else if(type == i18n("Cash"))
    rc = MyMoneyAccount::Cash;
  else if(type == i18n("Certificate of Deposit"))
    rc = MyMoneyAccount::CertificateDep;
  else if(type == i18n("Investment"))
    rc = MyMoneyAccount::Investment;
  else if(type == i18n("Money Market"))
    rc = MyMoneyAccount::MoneyMarket;
  else if(type == i18n("Asset"))
    rc = MyMoneyAccount::Asset;
  else if(type == i18n("Liability"))
    rc = MyMoneyAccount::Liability;
  else if(type == i18n("Currency"))
    rc = MyMoneyAccount::Currency;
  else if(type == i18n("Income"))
    rc = MyMoneyAccount::Income;
  else if(type == i18n("Expense"))
    rc = MyMoneyAccount::Expense;

  return rc;
}
#endif
