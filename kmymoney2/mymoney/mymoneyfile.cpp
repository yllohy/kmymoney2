/***************************************************************************
                          mymoneyfile.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
                           (C) 2002 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
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

#include <qstring.h>
#include <qdatetime.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#ifndef HAVE_CONFIG_H
#define VERSION "UNKNOWN"
#else
#include "config.h"
#endif

// include the following line to get a 'cout' for debug purposes
// #include <iostream>

// unsigned int MyMoneyFile::fileVersionRead;
// unsigned int MyMoneyFile::fileVersionWrite;

MyMoneyFile::MyMoneyFile(IMyMoneyStorage *storage)
{
  m_storage = storage;
}

MyMoneyFile::~MyMoneyFile()
{
}

void MyMoneyFile::addInstitution(MyMoneyInstitution& institution)
{
  // perform some checks to see that the institution stuff is OK. For
  // now we assume that the institution must have a name, the ID is not set
  // and it does not have a parent (MyMoneyFile).

  if(institution.name().length() == 0
  || institution.id().length() != 0
  || institution.file() != 0)
    throw new MYMONEYEXCEPTION("Not a new institution");

  m_storage->addInstitution(institution);
}

void MyMoneyFile::modifyInstitution(const MyMoneyInstitution& institution)
{
  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->modifyInstitution(institution);

  addNotification(institution.id());
}

void MyMoneyFile::modifyTransaction(const MyMoneyTransaction& transaction)
{
  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  // get the current setting of this transaction
  MyMoneyTransaction tr = MyMoneyFile::transaction(transaction.id());
  QValueList<MyMoneySplit>::ConstIterator it_s;

  // and mark all accounts that are referenced
  for(it_s = tr.splits().begin(); it_s != tr.splits().end(); ++it_s) {
    notifyAccountTree((*it_s).accountId());
  }

  // perform modification
  m_storage->modifyTransaction(transaction);

  // and mark all accounts that are referenced
  tr = transaction;
  for(it_s = tr.splits().begin(); it_s != tr.splits().end(); ++it_s) {
    notifyAccountTree((*it_s).accountId());
  }
}

void MyMoneyFile::modifyAccount(const MyMoneyAccount& account)
{
  // check that it's not one of the standard account groups
  if(isStandardAccount(account.id()))
    throw new MYMONEYEXCEPTION("Unable to modify the standard account groups");

  MyMoneyAccount acc = MyMoneyFile::account(account.id());
  if(account.accountType() != acc.accountType())
    throw new MYMONEYEXCEPTION("Unable to change account type");

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  // the account can be moved to another one, so we notify
  // the old one as well
  addNotification(account.institutionId());

  m_storage->modifyAccount(account);

  notifyAccountTree(account.id());
}

const MyMoneyAccount::accountTypeE MyMoneyFile::accountGroup(MyMoneyAccount::accountTypeE type) const
{
  switch(type) {
    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Currency:
    case MyMoneyAccount::Investment:
    case MyMoneyAccount::MoneyMarket:
    case MyMoneyAccount::CertificateDep:
      return MyMoneyAccount::Asset;

    case MyMoneyAccount::CreditCard:
    case MyMoneyAccount::Loan:
      return MyMoneyAccount::Liability;

    default:
      return type;
  }
}

void MyMoneyFile::reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent)
{
  // check that it's not one of the standard account groups
  if(isStandardAccount(account.id()))
    throw new MYMONEYEXCEPTION("Unable to reparent the standard account groups");

  if(accountGroup(account.accountType()) == accountGroup(parent.accountType())) {
    // automatically notify all observers once this routine is done
    MyMoneyNotifier notifier(this);

    // remember current account tree
    notifyAccountTree(account.id());

    m_storage->reparentAccount(account, parent);

    // and also keep the new one
    notifyAccountTree(account.id());

  } else
    throw new MYMONEYEXCEPTION("Unable to reparent to different account type");
}

const MyMoneyInstitution& MyMoneyFile::institution(const QCString& id) const
{
  return m_storage->institution(id);
}

const MyMoneyAccount& MyMoneyFile::account(const QCString& id) const
{
  return m_storage->account(id);
}

void MyMoneyFile::removeTransaction(const MyMoneyTransaction& transaction)
{
  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  // get the current setting of this transaction
  MyMoneyTransaction tr = MyMoneyFile::transaction(transaction.id());
  QValueList<MyMoneySplit>::ConstIterator it_s;

  // and mark all accounts that are referenced
  for(it_s = tr.splits().begin(); it_s != tr.splits().end(); ++it_s) {
    notifyAccountTree((*it_s).accountId());
  }

  m_storage->removeTransaction(transaction);
}


const bool MyMoneyFile::hasActiveSplits(const QCString& id) const
{
  return m_storage->hasActiveSplits(id);
}

const bool MyMoneyFile::isStandardAccount(const QCString& id) const
{
  return m_storage->isStandardAccount(id);
}

void MyMoneyFile::removeAccount(const MyMoneyAccount& account)
{
  MyMoneyAccount parent;
  MyMoneyAccount acc;

  // check that the account and it's parent exist
  // this will throw an exception if the id is unknown
  acc = MyMoneyFile::account(account.id());
  parent = MyMoneyFile::account(account.parentAccountId());

  // check that it's not one of the standard account groups
  if(isStandardAccount(account.id()))
    throw new MYMONEYEXCEPTION("Unable to remove the standard account groups");

  if(hasActiveSplits(account.id())) {
    throw new MYMONEYEXCEPTION("Unable to remove account with active splits");
  }

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  // collect all sub-ordinate accounts for notification
  QCStringList::ConstIterator it;
  for(it = acc.accountList().begin(); it != acc.accountList().end(); ++it)
    notifyAccountTree(*it);
  notifyAccountTree(parent.id());

  m_storage->removeAccount(account);
}

void MyMoneyFile::removeInstitution(const MyMoneyInstitution& institution)
{
  m_storage->removeInstitution(institution);
}

void MyMoneyFile::addAccount(MyMoneyAccount& account, MyMoneyAccount& parent)
{
  MyMoneyInstitution institution;

  // perform some checks to see that the account stuff is OK. For
  // now we assume that the account must have a name, has no
  // transaction and sub-accounts and parent account
  // it's own ID is not set and it does not have a pointer to (MyMoneyFile)

  if(account.name().length() == 0
  || account.id().length() != 0
  || account.transactionList().count() != 0
  || account.accountList().count() != 0
  || account.parentAccountId() != ""
  || account.accountType() == MyMoneyAccount::UnknownAccountType
  || account.file() != 0)
    throw new MYMONEYEXCEPTION("Adding invalid account");

  // make sure, that the parent account exists
  // if not, an exception is thrown. If it exists,
  // get a copy of the current data
  MyMoneyFile::account(parent.id());

  // FIXME: make sure, that the parent has the same type
  // I left it out here because I don't know, if there is
  // a tight coupling between e.g. checking accounts and the
  // class asset. It certainly does not make sense to create an
  // expense account under an income account. Maybe it does, I don't know.

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  // if an institution is set, verify that it exists
  if(account.institutionId().length() != 0) {
    // check the presence of the institution. if it
    // does not exist, an exception is thrown
    institution = MyMoneyFile::institution(account.institutionId());
  }


  if(account.openingDate() == QDate()) {
    account.setOpeningDate(QDate::currentDate());
  }

  account.setParentAccountId(parent.id());

  m_storage->newAccount(account);
  m_storage->addAccount(parent, account);

  if(account.institutionId().length() != 0)
    m_storage->addAccount(institution, account);

  // parse the complete account tree and collect all
  // account and institution ids
  notifyAccountTree(account.id());
}

void MyMoneyFile::addTransaction(MyMoneyTransaction& transaction)
{
  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  // perform some checks to see that the transaction stuff is OK. For
  // now we assume that
  // * no ids are assigned
  // * the pointer to the MyMoneyFile object is 0
  // * the date valid (must not be empty)
  // * the referenced accounts in the splits exist

  // first perform all the checks
  if(transaction.id() != ""
  || transaction.file() != 0
  || !transaction.postDate().isValid())
    throw new MYMONEYEXCEPTION("invalid transaction to be added");

  // now check the splits
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    // the following line will throw an exception if the
    // account does not exist
    MyMoneyFile::account((*it_s).accountId());
  }

  // then add the transaction to the file global pool
  m_storage->addTransaction(transaction);

  // scan the splits again to update notification list
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    notifyAccountTree((*it_s).accountId());
  }
}

const MyMoneyTransaction& MyMoneyFile::transaction(const QCString& id) const
{
  return m_storage->transaction(id);
}

const MyMoneyTransaction& MyMoneyFile::transaction(const QCString& account, const int idx) const
{
  return m_storage->transaction(account, idx);
}

void MyMoneyFile::addPayee(const QString& newPayee, const QString& address, const QString& postcode, const QString& telephone, const QString& email)
{
/*
  if (newPayee.isEmpty() || newPayee==QString::null)
    return;

  bool found=false;
  MyMoneyPayee *payee;
  for ( payee=m_payeeList.first(); payee!=0; payee=m_payeeList.next()) {
    if (payee->name() == newPayee)
      found = true;
  }

  if (!found) {
    MyMoneyPayee *np = new MyMoneyPayee(newPayee, address, postcode, telephone, email);
    m_payeeList.append(np);
    m_dirty=true;
  }
*/
}

void MyMoneyFile::removePayee(const QString name)
{
/*
  MyMoneyPayee *payee;
  for ( payee=m_payeeList.first(); payee!=0; payee=m_payeeList.next()) {
    if (payee->name() == name) {
      m_payeeList.remove();
      m_dirty=true;
    }
  }
*/
}

/*
const int MyMoneyFile::readStream(QDataStream& s)
{
  // process version and magic number to get the version information
  // files written by previous versions had the following layout:
  //
  //   QString with program version info
  //   Q_INT32 with magic code
  //
  // Newer files will use a sligthly different layout as follows:
  //
  //   QByteArray with the magic eight byte contents 'KMyMoney'
  //   Q_INT32 with file version information
  //

  // first read a four byte Q_INT32
  Q_INT32 len;
  QString prog_version("");

  s >> len;
  if(len < 30) {            // this seems to be a valid maximum length
    Q_UINT8 c, r;           // for a program version
    while(len) {
      s >> r;
      s >> c;
      prog_version += QChar(c, r);
      len -= 2;
    }
  } else {                  // check if it's the magic sequence
    // we've already read half of the magic code in, so we just
    // read the second half as well and see if it matches the new
    // magic code 'KMyMoney'
    Q_INT32 len1;
    s >> len1;
    if(len != MAGIC_0_50
    || len1 != MAGIC_0_51)
      return UNKNOWN_FILE_TYPE;
  }

  // next we read a Q_INT32 file version code
  fileVersionRead = 0;
  s >> fileVersionRead;
  if(fileVersionRead != VERSION_0_3_3
  && fileVersionRead != VERSION_0_4_0
  && fileVersionRead != VERSION_0_5_0)
    return UNKNOWN_FILE_FORMAT;

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
  if(fileVersionRead == VERSION_0_3_3) {
    qDebug("\nConverting from old 0.3.3 release\n\tRemoving old file::name field");
    QString temp_delete;
    s >> temp_delete;
  }

  s >> m_userName
    >> m_userStreet
    >> m_userTown
    >> m_userCounty
    >> m_userPostcode
    >> m_userTelephone
    >> m_userEmail
    >> m_creationDate
    >> m_lastAccessDate
    >> m_lastModificationDate;

  // process list of institutions (only version prior to 0.5.0)
  if(fileVersionRead == VERSION_0_3_3
  || fileVersionRead == VERSION_0_4_0) {
    // we don't need that list here, so we just skip it
    Q_INT32 bankCount;
    QString buffer;
    s >> bankCount;
    while(bankCount--)
      s >> buffer;
  }

  // read list of categories
  MyMoneyCategory category;
  s >> tmp;
  for (unsigned int i=0; i < tmp; i++) {
    s >> category;
    addCategory(category.isIncome(), category.name(), category.minorCategories());
  }

  // read list of payees
  MyMoneyPayee payee;
  s >> tmp;
  for (unsigned int i=0; i < tmp; i++) {
    s >> payee;
    addPayee(payee.name(), payee.address(), payee.postcode(), payee.telephone(), payee.email());
  }

  switch(fileVersionRead) {
    case VERSION_0_3_3:
    case VERSION_0_4_0:
      readOldFormat(s);
      break;
    case VERSION_0_5_0:
      // readNewFormat(s);
      break;
  }

  // file is yet unchanged
  clearDirty();
  // we're done
  return OK;
}

void MyMoneyFile::readOldFormat(QDataStream& s)
{
  Q_UINT32 institutionCount;
  Q_UINT32 accountCount;
  Q_UINT32 transactionCount;

  MyMoneyCheckingAccount checkingAccount;

  s >> institutionCount;
  for(unsigned int i=0; i < institutionCount; ++i) {
    MyMoneyInstitution institution;
    QString institutionID;

    // read institution data from file
    s >> institution;
    // since we don't have an id yet, we use the standard procedure
    institutionID = addInstitution(institution);

    s >> accountCount;
    for(unsigned j=0; j < accountCount; ++j) {
      // in old file versions, there are only checking accounts
      QString name, number, desc;
      MyMoneyAccount::accountTypeE accType;
      MyMoneyAccount *acc;

      // read the next three values here, because the type
      // is the fourth that we need to distinguish the type
      // of account
      s >> name >> desc >> number;
      s >> (Q_INT32 &) accType;

      switch(accType) {
        case MyMoneyAccount::Checkings:
          s >> checkingAccount;
          acc = &checkingAccount;
          break;

        default:
          qDebug("Unknown account type %d in MyMoneyFile", accType);
          acc = &checkingAccount;
          break;
      }

      // now load the account with the info already read in
      acc->setName(name);
      acc->setNumber(number);
      acc->setDescription(desc);

      // and don't forget other stuff
      acc->setInstitution(institutionID);

      // the account info is completely read, now add the
      // account to the global file pool

      QString accountID;
      accountID = addAccount(acc);

      s >> transactionCount;
      for ( unsigned int k = 0; k < transactionCount; k++) {
        // the old file storage only knows about checking transactions
        MyMoneyCheckingTransaction t;
        QList<MyMoneyTransaction> list;

        // read the data from file
        s >> t;

        // Only add those transactions that are the tied to
        // the account. The right side of a transfer transaction
        // is skipped here. It will be kept as a single transaction
        // in the new engine and reconstructed later on, when all
        // account information is available.
        if(t.method() != MyMoneyCheckingTransaction::Deposit
        || t.categoryMajor()[0] != '<') {
          QString tid;

          t.setSrcAccount(accountID);
          list.append(&t);
          tid = addTransaction(list);
          if(tid == "")
            qDebug("Unable to add transaction in MyMoneyFile::readOldFormat");
        }
      }
    }
  }

  // All relevant information is read from the file. Now we
  // need to adjust a few things:
  //
  // 1 - change all account references from names to ids
  //     This is the case for all transfers, where the categoryMajor
  //     still has the form '<' account-name '>'

  QMap<QString, MyMoneyTransaction *>::Iterator tit;
  QMap<QString, MyMoneyAccount*>::Iterator ait;

  cout << "TransactionList" << endl;
  for(tit = m_transactionList.begin(); tit != m_transactionList.end(); ++tit) {
    MyMoneyCheckingTransaction *p;
    p = static_cast<MyMoneyCheckingTransaction *> (*tit);
    cout << "  key: " << (*tit)->key() << endl;
    if(p->method() == MyMoneyCheckingTransaction::Transfer) {
      QString name = (*tit)->categoryMajor();
      name = name.remove(0,1);
      name = name.remove(name.length()-1,1);
      for(ait = m_accountList.begin(); ait != m_accountList.end(); ++ait) {
        if((*ait)->name() == name) {
          (*tit)->setDstAccount((*ait)->id());
          cout << "    setting dst to " << (*ait)->id() << endl;
        }
      }
    }
  }

  // Update all accounts
  for(ait = m_accountList.begin(); ait != m_accountList.end(); ++ait) {
    (*ait)->refreshTransactionList();
  }
}
*/

const QValueList<MyMoneyAccount> MyMoneyFile::accountList(void) const
{
  return m_storage->accountList();
}

const QValueList<MyMoneyInstitution> MyMoneyFile::institutionList(void) const
{
  return m_storage->institutionList();
}

// general get functions
const QString MyMoneyFile::userName(void) const { return m_storage->userName(); }
const QString MyMoneyFile::userStreet(void) const { return m_storage->userStreet(); }
const QString MyMoneyFile::userTown(void) const { return m_storage->userTown(); }
const QString MyMoneyFile::userCounty(void) const { return m_storage->userCounty(); }
const QString MyMoneyFile::userPostcode(void) const { return m_storage->userPostcode(); }
const QString MyMoneyFile::userTelephone(void) const { return m_storage->userTelephone(); }
const QString MyMoneyFile::userEmail(void) const { return m_storage->userEmail(); }

// general set functions
void MyMoneyFile::setUserName(const QString& val) { m_storage->setUserName(val); }
void MyMoneyFile::setUserStreet(const QString& val) { m_storage->setUserStreet(val); }
void MyMoneyFile::setUserTown(const QString& val) { m_storage->setUserTown(val); }
void MyMoneyFile::setUserCounty(const QString& val) { m_storage->setUserCounty(val); }
void MyMoneyFile::setUserPostcode(const QString& val) { m_storage->setUserPostcode(val); }
void MyMoneyFile::setUserTelephone(const QString& val) { m_storage->setUserTelephone(val); }
void MyMoneyFile::setUserEmail(const QString& val) { m_storage->setUserEmail(val); }

bool MyMoneyFile::dirty(void) const
{
  return m_storage->dirty();
}

const unsigned int MyMoneyFile::accountCount(void) const
{
  return m_storage->accountCount();
}

const MyMoneyAccount MyMoneyFile::liability(void) const
{
  return m_storage->liability();
}

const MyMoneyAccount MyMoneyFile::asset(void) const
{
  return m_storage->asset();
}

const MyMoneyAccount MyMoneyFile::expense(void) const
{
  return m_storage->expense();
}

const MyMoneyAccount MyMoneyFile::income(void) const
{
  return m_storage->income();
}

const unsigned int MyMoneyFile::transactionCount(void) const
{
  return m_storage->transactionCount();
}

const unsigned int MyMoneyFile::institutionCount(void) const
{
  return m_storage->institutionCount();
}

const MyMoneyMoney MyMoneyFile::balance(const QCString& id) const
{
  return m_storage->balance(id);
}

const MyMoneyMoney MyMoneyFile::totalBalance(const QCString& id) const
{
  return m_storage->totalBalance(id);
}

void MyMoneyFile::attach(const QCString& id, MyMoneyObserver* observer)
{
  QMap<QCString, MyMoneyFileSubject>::Iterator it_s;

  // make sure an entry for the subject with the id exists
  m_subjects[id];

  it_s = m_subjects.find(id);
  (*it_s).attach(observer);
}

void MyMoneyFile::detach(const QCString& id, MyMoneyObserver* observer)
{
  QMap<QCString, MyMoneyFileSubject>::Iterator it_s;

  it_s = m_subjects.find(id);
  if(it_s != m_subjects.end())
    (*it_s).detach(observer);
}

void MyMoneyFile::notify(const QCString& id) const
{
  QMap<QCString, MyMoneyFileSubject>::ConstIterator it_s;

  it_s = m_subjects.find(id);
  if(it_s != m_subjects.end())
    (*it_s).notify(id);
}

void MyMoneyFile::notify(void)
{
  QMap<QCString, bool>::ConstIterator it;
  for(it = m_notificationList.begin(); it != m_notificationList.end(); ++it)
    notify(it.key());

  clearNotification();
}

void MyMoneyFile::notifyAccountTree(const QCString& id)
{
  QCString accId = id;
  MyMoneyAccount acc;

  do {
    addNotification(accId);
    acc = account(accId);
    addNotification(acc.institutionId());
    accId = acc.parentAccountId();
  } while(accId != "");
}

void MyMoneyFile::addNotification(const QCString& id)
{
  if(id != "")
    m_notificationList[id] = true;
}

void MyMoneyFile::clearNotification()
{
  // reset list to be empty
  m_notificationList.clear();
}

