/***************************************************************************
                          mymoneyqifreader.cpp  -  description
                             -------------------
    begin                : Mon Jan 27 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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
// QT Headers

#include <qfile.h>
#include <qstringlist.h>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyqifreader.h"

MyMoneyQifReader::MyMoneyQifReader() :
  m_tempFile(QString::null, "qif")
{
  m_tempFile.close();
  m_tempFile.setAutoDelete(true);
}

MyMoneyQifReader::~MyMoneyQifReader()
{
}

void MyMoneyQifReader::setFilename(const QString& name)
{
  m_originalFilename = name;
  runFilter();
}

void MyMoneyQifReader::setProfile(const QString& profile)
{
  m_qifProfile.loadProfile("Profile-" + profile);
  runFilter();
}

void MyMoneyQifReader::runFilter(void)
{
/*
  if(m_qifProfile.filterScript() != "") {
    m_filename = m_tempFile.name();
    
  } else
*/
    m_filename = m_originalFilename;
}

const QString MyMoneyQifReader::scanFileForAccount(void)
{
  QString rc;
  QFile qifFile(m_filename);
  QStringList entry;
  if(qifFile.open(IO_ReadOnly)) {
    QTextStream s(&qifFile);
    while(!s.atEnd()) {
      QString line = s.readLine();
      QString type = "!Type:" + m_qifProfile.profileType();
      if(line == type) {
        processMSAccountEntry(readEntry(s));
        if(m_account.name() != "") {
          rc = m_account.name();
          break;
        }
      } else if(line == "!Account") {
        processAccountEntry(readEntry(s));
      }
    }
    qifFile.close();
  }
  return rc;
}

const QStringList MyMoneyQifReader::readEntry(QTextStream& s) const
{
  QStringList entry;
  while(!s.atEnd()) {
    QString line = s.readLine();
    if(line[0] == '^')
      break;
    entry += line;
  }
  return entry;
}

const QString MyMoneyQifReader::extractLine(const QChar id, const QStringList& lines) const
{
  QStringList::ConstIterator it;

  for(it = lines.begin(); it != lines.end(); ++it) {
    if((*it)[0] == id) {
      return (*it).mid(1);
    }
  }
  return QString();
}

void MyMoneyQifReader::processMSAccountEntry(const QStringList& lines)
{
  m_account = MyMoneyAccount();
  if(extractLine('P', lines) == m_qifProfile.openingBalanceText()) {
    QString txt = extractLine('T', lines);
    MyMoneyMoney balance = m_qifProfile.value('T', txt);
    m_account.setOpeningBalance(balance);

    QDate date = m_qifProfile.date(extractLine('D', lines));
    m_account.setOpeningDate(date);

    QString name = extractLine('L', lines);
    if(name.left(1) == m_qifProfile.accountDelimiter().left(1)) {
      name = name.mid(1, name.length()-2);
    }
    m_account.setName(name);
  }  
}

void MyMoneyQifReader::processAccountEntry(const QStringList& lines)
{
  m_account = MyMoneyAccount();
  
  m_account.setName(extractLine('N', lines));
  m_account.setDescription(extractLine('D', lines));
  m_account.setValue("lastStatementBalance", extractLine('$', lines));
  m_account.setValue("lastStatementDate", m_qifProfile.date(extractLine('/', lines)).toString("yyyy-MM-dd"));
  QString type = extractLine('T', lines);
  if(type == m_qifProfile.profileType()) {
    m_account.setAccountType(MyMoneyAccount::Checkings);
  } else if(type == "CCard") {
    m_account.setAccountType(MyMoneyAccount::CreditCard);
  }
}
