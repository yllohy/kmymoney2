/***************************************************************************
                          mymoneyfile.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneyfile.h"
#ifndef HAVE_CONFIG_H
#define VERSION "UNKNOWN"
#else
#include "config.h"
#endif

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <qfile.h>
#include <qdatastream.h>

#include "mymoneyaccount.h"

#define MAGIC 0x00000006  // File revision number

MyMoneyFile::MyMoneyFile(void)
{
//  m_banks.setAutoDelete(true);
//  m_categoryList.setAutoDelete(true);
//  m_payeeList.setAutoDelete(true);
  m_dirty = false;
  m_initialised = m_containsBanks = m_containsAccounts = m_containsTransactions = false;
}

MyMoneyFile::MyMoneyFile(const QString& szname, const QString& usern, const QString& userStreet,
 const QString& userTown, const QString& userCounty, const QString& userPostcode, const QString& userTelephone,
 const QString& userEmail, const QDate& createDate)
{
//  m_banks.setAutoDelete(true);
//  m_categoryList.setAutoDelete(true);
//  m_payeeList.setAutoDelete(true);

  m_moneyName = szname;
  m_userName = usern;
  m_userStreet = userStreet;
  m_userTown = userTown;
  m_userCounty = userCounty;
  m_userPostcode = userPostcode;
  m_userTelephone = userTelephone;
  m_userEmail = userEmail;
  m_createdDate = createDate;
  m_lastAccess = createDate;
  m_lastModify = createDate;
  m_dirty = false;
  m_passwordProtected=false;
  m_encrypted=false;
  m_initialised = m_containsBanks = m_containsAccounts = m_containsTransactions = false;
}

MyMoneyFile::~MyMoneyFile()
{
}

QString MyMoneyFile::userAddress(void)
{
  // Return an address containing concatenated strings
  QString m_userAddress;
  m_userAddress = m_userStreet + "\n";
  m_userAddress += m_userTown + "\n";
  m_userAddress += m_userCounty + "\n";
  m_userAddress += m_userPostcode + "\n";
  m_userAddress += m_userTelephone + "\n";
  m_userAddress += m_userEmail + "\n";
  return m_userAddress;
}

int MyMoneyFile::saveAllData(const QString& fileName)
{
  QFile f( fileName );
  f.open( IO_WriteOnly );
  QDataStream s( &f );

  // Write a header with a "magic number" and a version
  QString ver(VERSION);
  s << ver;
  s << (Q_INT32)MAGIC;
  if (m_passwordProtected)
    s << (Q_INT32)1;
  else
    s << (Q_INT32)0;
  if (m_encrypted)
    s << (Q_INT32)1;
  else
    s << (Q_INT32)0;
  s << m_password;

  // Is updated if the user chooses to save the file
  m_lastModify = QDate::currentDate();

  // Simple Data
  s << m_moneyName
    << m_userName
    << m_userStreet
    << m_userTown
    << m_userCounty
    << m_userPostcode
    << m_userTelephone
    << m_userEmail
    << m_createdDate
    << m_lastAccess
    << m_lastModify;

  // Read through the three lists writing the count and then the objects
  // in turn
  s << m_bankNames.count();
  for (QStringList::Iterator it = m_bankNames.begin(); it != m_bankNames.end(); ++it)
    s << (*it);

  s << m_categoryList.count();
  MyMoneyCategory *data;
  for (data=m_categoryList.first(); data != 0; data = m_categoryList.next()) {
    s << *data;
  }

  s << m_payeeList.count();
  MyMoneyPayee *payee;
  for ( payee=m_payeeList.first(); payee!=0; payee=m_payeeList.next()) {
    s << *payee;
  }

//  unsigned int bankIdx=0, accountIdx=0;

  // Tell the objects to save themselves
//  QListIterator<MyMoneyBank> it = bankIterator();
  s << (Q_INT32)bankCount();
  MyMoneyBank *bank;
  //for ( ; it.current(); ++it) {
  for (bank=bankFirst(); bank!=0; bank=bankNext()) {
//    s << (*it.current());
    s << *bank;

    //QListIterator<MyMoneyAccount> it2 = accountIterator(*it.current());
//    s << (Q_INT32)accountCount(*it.current());
    s << (Q_INT32)bank->accountCount();
    MyMoneyAccount *account;
    for (account=bank->accountFirst(); account!=0; account=bank->accountNext()) {
    //for ( ; it2.current(); ++it2) {
//      s << (*it2.current());
      s << *account;

      //QListIterator<MyMoneyTransaction> it3 = transactionIterator(*it.current(), *it2.current());
//      s << (Q_INT32)transactionCount(*it.current(), *it2.current());
      s << (Q_INT32)account->transactionCount();
      MyMoneyTransaction *transaction;
      for (transaction=account->transactionFirst(); transaction!=0; transaction=account->transactionNext()) {
      //for ( ; it3.current(); ++it3) {
//        s << (*it3.current());
        s << *transaction;
      }
//      accountIdx++;
    }
//    bankIdx++;
  }

  f.close();
  m_dirty = false;
  return 0;
}

int MyMoneyFile::readAllData(const QString& fileName)
{
  // Returns an error code in the form of an int.
  // TODO: add descriptive error returns.
  QFile f( fileName );
  f.open( IO_ReadOnly );
  if (!f.isOpen())
    return 3;

  QDataStream s( &f );

  QString version;
  s >> version;
/* In the future I'll have a check for compatible versions, for now rely on the MAGIC...
  if (version != VERSION) {
    // error code
    return 1;
  }
*/
  Q_INT32 magic;
  s >> magic;

  if (magic != MAGIC) {
    return 2;
  }

  Q_INT32 tmp;
  s >> tmp;
  if (tmp==1)
    m_passwordProtected = true;
  else
    m_passwordProtected = false;
  s >> tmp;
  if (tmp==1)
    m_encrypted = true;
  else
    m_encrypted = false;

  // if (m_encrypted ==true) {
  //   All data that follows needs to be decrypted
  // }
  s >> m_password;

  // Simple Data
  s >> m_moneyName
    >> m_userName
    >> m_userStreet
    >> m_userTown
    >> m_userCounty
    >> m_userPostcode
    >> m_userTelephone
    >> m_userEmail
    >> m_createdDate
    >> m_lastAccess
    >> m_lastModify;

  m_lastAccess = QDate::currentDate();

  Q_UINT32 bankNames_count=0, categoryList_count=0, payTo_count=0, bank_count=0, account_count=0, transaction_count=0;

  QString buffer;
  QString buffer2;

  // Reading is quite simple as each object 'knows' how to read into itself
  s >> bankNames_count;
  for (unsigned int i=0; i<bankNames_count; i++) {
    s >> buffer;
    addBankName(buffer);
  }

  MyMoneyCategory category;
  s >> categoryList_count;
  for (unsigned int i=0; i<categoryList_count; i++) {
    s >> category;
    addCategory(category.isIncome(), category.name(), category.minorCategories());
  }

  MyMoneyPayee *payee=0;
  s >> payTo_count;
  for (unsigned int i=0; i<payTo_count; i++) {
    if (payee==0) {
      payee = new MyMoneyPayee;
    }
    s >> (*payee);
    addPayee(payee->name(), payee->address(), payee->postcode(), payee->telephone(), payee->email());
    delete payee;
    payee=0;
  }

  MyMoneyBank bankl;
  MyMoneyBank *bankWrite;
  MyMoneyAccount account;
  MyMoneyAccount *accountWrite;
  MyMoneyTransaction transaction;

  s >> bank_count;
  for ( unsigned int j=0; j<bank_count; j++ ) {
    m_containsBanks = true;
    s >> bankl;
    addBank(bankl.name(), bankl.sortCode(), bankl.city(), bankl.street(), bankl.postcode(), bankl.telephone(), bankl.manager());

    bankWrite = bank(bankl);
    if (!bankWrite) {
      qDebug("Unable to get bankWrite");
      return -9;
    }

    s >> account_count;
    for ( unsigned int k=0; k<account_count; k++ ) {
      m_containsAccounts=true;
      s >> account;
      bankWrite->newAccount(account.name(), account.accountNumber(), account.accountType(), account.description(), account.lastReconcile());

      accountWrite = bankWrite->account(account);
      if (!accountWrite) {
        qDebug("Unabel to get accountWrite");
        return -9;
      }

      s >> transaction_count;
      for ( unsigned int l=0; l<transaction_count; l++) {
        m_containsTransactions=true;
        s >> transaction;
        accountWrite->addTransaction(transaction.method(), transaction.number(), transaction.memo(),
            transaction.amount(), transaction.date(), transaction.categoryMajor(), transaction.categoryMinor(),
            transaction.atmBankName(), transaction.payee(), transaction.accountFrom(), transaction.accountTo(),
            transaction.state());
      }
    }
  }

  f.close();
  return 0;
}

void MyMoneyFile::resetAllData(void)
{
  // Just empty out all data
  QListIterator<MyMoneyBank> it(m_banks);
  for ( ; it.current(); ++it ) {
    MyMoneyBank *bank = it.current();
    bank->clear();
  }

  m_moneyName = QString::null;
  m_userName = QString::null;
  m_userStreet = QString::null;
  m_userTown = QString::null;
  m_userCounty = QString::null;
  m_userPostcode = QString::null;
  m_userTelephone = QString::null;
  m_userEmail = QString::null;
  m_bankNames.clear();
  m_createdDate = QDate::currentDate();
  m_lastAccess = QDate::currentDate();
  m_lastModify = QDate::currentDate();
  m_categoryList.clear();
  m_initialised = m_containsBanks = m_containsAccounts = m_containsTransactions = false;
}

MyMoneyBank* MyMoneyFile::bank(const MyMoneyBank& bank)
{
  unsigned int pos;

  if (findBankPosition(bank, pos)) {
    return m_banks.at(pos);
  }
  return 0;
}

MyMoneyBank* MyMoneyFile::bankFirst(void)
{
  return m_banks.first();
}

MyMoneyBank* MyMoneyFile::bankNext(void)
{
  return m_banks.next();
}

MyMoneyBank* MyMoneyFile::bankLast(void)
{
  return m_banks.last();
}

unsigned int MyMoneyFile::bankCount(void)
{
  return m_banks.count();
}

bool MyMoneyFile::removeBank(const MyMoneyBank& bank)
{
  if (m_banks.count()<=1)
    m_containsBanks=false;

  unsigned int pos;
  if (findBankPosition(bank, pos))
    return m_banks.remove(pos);
  return false;
}
/*
bool MyMoneyFile::addAccount(const QString& name, const QString& number, MyMoneyAccount::accountTypeE type,
  const QString& description, const QDate& lastReconcile, const MyMoneyBank& bank)
{
  if (m_containsAccounts==false)
    m_containsAccounts=true;

  unsigned int pos=0;
  if (findBankPosition(bank, pos)) {
    return m_banks.at(pos)->newAccount(name, number, type, description, lastReconcile);
  }
  return false;
}

bool MyMoneyFile::addTransaction(const long id, MyMoneyTransaction::transactionMethod methodType, const QString& number, const QString& memo,
  const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
  const QString& fromTo, const QString& bankFrom, const QString& bankTo, MyMoneyTransaction::stateE state, const MyMoneyBank& bank, const MyMoneyAccount& account)
{
  unsigned int pos;
  MyMoneyAccount *accountTmp=0;
  if (findBankPosition(bank, pos)) {
    if ((accountTmp=m_banks.at(pos)->account(account))) {
      if (m_containsTransactions==false)
        m_containsTransactions=true;

        return accountTmp->addTransaction(id, methodType, number, memo, amount, date, categoryMajor, categoryMinor, atmName,
          fromTo, bankFrom, bankTo, state);
    }
  }
  return false;
}
*/
void MyMoneyFile::addBankName(const QString& val)
{
  m_bankNames.append(val);
}

void MyMoneyFile::addMajorCategory(const bool income, const QString& val)
{
  MyMoneyCategory *tst;
  for (tst=m_categoryList.first(); tst!=0; tst=m_categoryList.next()) {
    if (tst->name() == val) { // Already in, return true
      if (tst->isIncome() != income)
        tst->setIncome(income);
      return;
    }
  }

  MyMoneyCategory *data = new MyMoneyCategory(income, val);
  m_categoryList.append(data);
  m_dirty=true;
}

void MyMoneyFile::addMinorCategory(const bool income, const QString& major, const QString& minor)
{
  MyMoneyCategory *data;
  for (data=m_categoryList.first(); data!=0; data=m_categoryList.next()) {
    if (data->name() == major) {
      data->addMinorCategory(minor);
      if (data->isIncome() != income)
        data->setIncome(income);
      m_dirty=true;
      return;
    }
  }

  MyMoneyCategory *newData = new MyMoneyCategory(income, major, minor);
  m_categoryList.append(newData);
  m_dirty=true;
}

void MyMoneyFile::addCategory(const bool income, const QString& major, QStringList& minors)
{
  MyMoneyCategory *data;
  for (data=m_categoryList.first(); data!=0; data=m_categoryList.next()) {
    if (data->name() == major) {
      data->addMinorCategory(minors);
      m_dirty=true;
      if (data->isIncome() != income)
        data->setIncome(income);
      return;
    }
  }

  MyMoneyCategory *newData = new MyMoneyCategory(income, major, minors);
  m_categoryList.append(newData);
  m_dirty=true;
}

void MyMoneyFile::addCategory(const bool income, const QString& major, const QString& minor)
{
  addMinorCategory(income, major, minor);
}

QListIterator<MyMoneyCategory> MyMoneyFile::categoryIterator(void)
{
  QListIterator<MyMoneyCategory> it(m_categoryList);
  return it;
}

void MyMoneyFile::removeMajorCategory(const QString& major)
{
  MyMoneyCategory *data;
  for (data=m_categoryList.first(); data!=0; data=m_categoryList.next()) {
    if (data->name() == major) {
      data->removeAllMinors();
      m_categoryList.remove(m_categoryList.at());
      m_dirty=true;
    }
  }
}

void MyMoneyFile::removeMinorCategory(const QString& major, const QString& minor)
{
  MyMoneyCategory *data;
  for (data=m_categoryList.first(); data!=0; data=m_categoryList.next()) {
    if (data->name() == major) {
      data->removeMinorCategory(minor);
      m_dirty=true;
    }
  }
}

void MyMoneyFile::renameMajor(const QString& oldName, const QString& newName)
{
  MyMoneyCategory *data;
  for (data=m_categoryList.first(); data!=0; data=m_categoryList.next()) {
    if (data->name() == oldName) {
      data->setName(newName);
      m_dirty = true;
    }
  }
}

void MyMoneyFile::renameMinor(const QString& major, const QString& oldName, const QString& newName)
{
  MyMoneyCategory *data;
  for (data=m_categoryList.first(); data!=0; data=m_categoryList.next()) {
    if (data->name() == major) {
      if (data->renameMinorCategory(oldName, newName))
        m_dirty=true;
    }
  }
}

void MyMoneyFile::addPayee(const QString& newPayee, QString address, QString postcode, QString telephone, QString email)
{
  bool found=false;
  MyMoneyPayee *payee;
  for ( payee=m_payeeList.first(); payee!=0; payee=m_payeeList.next()) {
    if (payee->name() == newPayee)
      found = true;
  }

  if (!found) {
    MyMoneyPayee *np = new MyMoneyPayee(newPayee, address, postcode, telephone, email);
    m_payeeList.append(np);
  }
}

QListIterator<MyMoneyPayee> MyMoneyFile::payeeIterator(void)
{
  QListIterator<MyMoneyPayee> it(m_payeeList);
  return it;
}

bool MyMoneyFile::addBank(const QString& name, const QString& sortCode, const QString& city,
  const QString& street, const QString& postcode, const QString& telephone, const QString& manager)
{
  MyMoneyBank *bank = new MyMoneyBank(name, sortCode, city, street, postcode, telephone, manager);
  m_banks.append(bank);
  if (m_containsBanks==false)
    m_containsBanks=true;
  return true;
}

MyMoneyFile::MyMoneyFile(const MyMoneyFile& right)
{
  m_moneyName = right.m_moneyName;
  m_userName = right.m_userName;
  m_userStreet = right.m_userStreet;
  m_userTown = right.m_userTown;
  m_userCounty = right.m_userCounty;
  m_userPostcode = right.m_userPostcode;
  m_userTelephone = right.m_userTelephone;
  m_userEmail = right.m_userEmail;
  m_createdDate = right.m_createdDate;
  m_lastAccess = right.m_lastAccess;
  m_lastModify = right.m_lastModify;
  m_dirty = right.m_dirty;
  m_passwordProtected = right.m_passwordProtected;
  m_encrypted = right.m_encrypted;
  m_password = right.m_password;
  m_banks.clear();
  m_banks = right.m_banks;
  m_bankNames.clear();
  m_bankNames = right.m_bankNames;
  m_payeeList.clear();
  m_payeeList = right.m_payeeList;
  m_categoryList.clear();
  m_categoryList = right.m_categoryList;
}

MyMoneyFile& MyMoneyFile::operator = (const MyMoneyFile& right)
{
  m_moneyName = right.m_moneyName;
  m_userName = right.m_userName;
  m_userStreet = right.m_userStreet;
  m_userTown = right.m_userTown;
  m_userCounty = right.m_userCounty;
  m_userPostcode = right.m_userPostcode;
  m_userTelephone = right.m_userTelephone;
  m_userEmail = right.m_userEmail;
  m_createdDate = right.m_createdDate;
  m_lastAccess = right.m_lastAccess;
  m_lastModify = right.m_lastModify;
  m_dirty = right.m_dirty;
  m_passwordProtected = right.m_passwordProtected;
  m_encrypted = right.m_encrypted;
  m_password = right.m_password;
  m_banks.clear();
  m_banks = right.m_banks;
  m_bankNames.clear();
  m_bankNames = right.m_bankNames;
  m_payeeList.clear();
  m_payeeList = right.m_payeeList;
  m_categoryList.clear();
  m_categoryList = right.m_categoryList;
  return *this;
}

bool MyMoneyFile::findBankPosition(const MyMoneyBank& bank, unsigned int& pos)
{
  int k=0;

  QListIterator<MyMoneyBank> it(m_banks);
  for ( k=0; it.current(); ++it, k++) {
    if (*it.current()==bank) {
      pos=k;
      return true;
    }
  }
  pos=k;
  return false;
}

void MyMoneyFile::init(void)
{
  m_initialised=true;
}

bool MyMoneyFile::isInitialised(void)
{
  return m_initialised;
}

bool MyMoneyFile::containsBanks(void)
{
  qDebug("STUB: in MyMoneyFile::containsBanks");
  return m_containsBanks;
}

bool MyMoneyFile::containsAccounts(void)
{
  qDebug("STUB: in MyMoneyFile::containsAccounts");
  return m_containsAccounts;
}

bool MyMoneyFile::containsTransactions(void)
{
  qDebug("STUB: in MyMoneyFile::containsTransactions");
  return m_containsTransactions;
}
