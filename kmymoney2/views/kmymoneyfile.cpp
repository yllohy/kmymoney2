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

// KMyMoneyFile *KMyMoneyFile::_instance = 0;

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
    m_storage = 0;
  }
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

