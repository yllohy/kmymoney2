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
#include <qvaluelist.h>

// ----------------------------------------------------------------------------
// KDE Includes

// #include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "storage/mymoneyseqaccessmgr.h"
#include "mymoneyfile.h"
#ifndef HAVE_CONFIG_H
#define VERSION "UNKNOWN"
#else
#include "config.h"
#endif

// include the following line to get a 'cout' for debug purposes
// #include <iostream>
MyMoneyFile* MyMoneyFile::_instance = 0;

MyMoneyFile* const MyMoneyFile::instance()
{
  if(_instance == 0) {
    _instance = new MyMoneyFile;
  }
  return _instance;
}

MyMoneyFile::MyMoneyFile()
{
  m_storage = 0;
}

MyMoneyFile::~MyMoneyFile()
{
  _instance = 0;
}

MyMoneyFile::MyMoneyFile(IMyMoneyStorage *storage)
{
  m_storage = 0;
  attachStorage(storage);
}

void MyMoneyFile::attachStorage(IMyMoneyStorage* const storage)
{
  if(m_storage != 0)
    throw new MYMONEYEXCEPTION("Storage already attached");

  if(storage == 0)
    throw new MYMONEYEXCEPTION("Storage must not be 0");

  m_storage = storage;
}

void MyMoneyFile::detachStorage(IMyMoneyStorage* const storage)
{
  m_storage = 0;
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

  checkStorage();

  m_storage->addInstitution(institution);
}

void MyMoneyFile::modifyInstitution(const MyMoneyInstitution& institution)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->modifyInstitution(institution);

  addNotification(institution.id());
}

void MyMoneyFile::modifyTransaction(const MyMoneyTransaction& transaction)
{
  checkStorage();

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
  checkStorage();

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
  checkStorage();

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
  checkStorage();

  return m_storage->institution(id);
}

const MyMoneyAccount& MyMoneyFile::account(const QCString& id) const
{
/*  It would be nice if this works, but it causes a seg fault.
    because we're returning a reference to a temporary.
	MyMoneyAccount account;
  if (m_storage->isStandardAccount(id))
	{
	  if (id == STD_ACC_LIABILITY)
		  account = liability();
		else if (id == STD_ACC_ASSET)
			account = asset();
		else if (id == STD_ACC_EXPENSE)
			account = expense();
	  else if (id == STD_ACC_INCOME)
			account = income();
		return account;
	}
*/	
  checkStorage();

  return m_storage->account(id);
}

void MyMoneyFile::removeTransaction(const MyMoneyTransaction& transaction)
{
  checkStorage();

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
  checkStorage();

  return m_storage->hasActiveSplits(id);
}

const bool MyMoneyFile::isStandardAccount(const QCString& id) const
{
  checkStorage();

  return m_storage->isStandardAccount(id);
}

void MyMoneyFile::setAccountName(const QCString& id, const QString& name) const
{
  checkStorage();

  m_storage->setAccountName(id, name);
}

void MyMoneyFile::removeAccount(const MyMoneyAccount& account)
{
  checkStorage();

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
  checkStorage();

  m_storage->removeInstitution(institution);
}

void MyMoneyFile::addAccount(MyMoneyAccount& account, MyMoneyAccount& parent)
{
  checkStorage();

  MyMoneyInstitution institution;

  // perform some checks to see that the account stuff is OK. For
  // now we assume that the account must have a name, has no
  // transaction and sub-accounts and parent account
  // it's own ID is not set and it does not have a pointer to (MyMoneyFile)

  if(account.name().length() == 0
  || account.id().length() != 0
// removed with MyMoneyAccount::Transaction
//  || account.transactionList().count() != 0
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
  checkStorage();

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
  checkStorage();

  return m_storage->transaction(id);
}

const MyMoneyTransaction& MyMoneyFile::transaction(const QCString& account, const int idx) const
{
  checkStorage();

  return m_storage->transaction(account, idx);
}

void MyMoneyFile::addPayee(MyMoneyPayee& payee) const
{
  checkStorage();

  m_storage->addPayee(payee);
}

const MyMoneyPayee MyMoneyFile::payee(const QCString& id) const
{
  checkStorage();

  return m_storage->payee(id);
}

void MyMoneyFile::modifyPayee(const MyMoneyPayee& payee) const
{
  checkStorage();

  m_storage->modifyPayee(payee);
}

void MyMoneyFile::removePayee(const MyMoneyPayee& payee) const
{
  checkStorage();

  m_storage->removePayee(payee);
}


const QValueList<MyMoneyAccount> MyMoneyFile::accountList(void) const
{
  checkStorage();

  return m_storage->accountList();
}

const QValueList<MyMoneyInstitution> MyMoneyFile::institutionList(void) const
{
  checkStorage();

  return m_storage->institutionList();
}

// general get functions
const QString MyMoneyFile::userName(void) const { checkStorage(); return m_storage->userName(); }
const QString MyMoneyFile::userStreet(void) const { checkStorage(); return m_storage->userStreet(); }
const QString MyMoneyFile::userTown(void) const { checkStorage(); return m_storage->userTown(); }
const QString MyMoneyFile::userCounty(void) const { checkStorage(); return m_storage->userCounty(); }
const QString MyMoneyFile::userPostcode(void) const { checkStorage(); return m_storage->userPostcode(); }
const QString MyMoneyFile::userTelephone(void) const { checkStorage(); return m_storage->userTelephone(); }
const QString MyMoneyFile::userEmail(void) const { checkStorage(); return m_storage->userEmail(); }

// general set functions
void MyMoneyFile::setUserName(const QString& val) { checkStorage(); m_storage->setUserName(val); }
void MyMoneyFile::setUserStreet(const QString& val) { checkStorage(); m_storage->setUserStreet(val); }
void MyMoneyFile::setUserTown(const QString& val) { checkStorage(); m_storage->setUserTown(val); }
void MyMoneyFile::setUserCounty(const QString& val) { checkStorage(); m_storage->setUserCounty(val); }
void MyMoneyFile::setUserPostcode(const QString& val) { checkStorage(); m_storage->setUserPostcode(val); }
void MyMoneyFile::setUserTelephone(const QString& val) { checkStorage(); m_storage->setUserTelephone(val); }
void MyMoneyFile::setUserEmail(const QString& val) { checkStorage(); m_storage->setUserEmail(val); }

bool MyMoneyFile::dirty(void) const
{
  checkStorage();

  return m_storage->dirty();
}

const unsigned int MyMoneyFile::accountCount(void) const
{
  checkStorage();

  return m_storage->accountCount();
}

const MyMoneyAccount MyMoneyFile::liability(void) const
{
  checkStorage();

  return m_storage->liability();
}

const MyMoneyAccount MyMoneyFile::asset(void) const
{
  checkStorage();

  return m_storage->asset();
}

const MyMoneyAccount MyMoneyFile::expense(void) const
{
  checkStorage();

  return m_storage->expense();
}

const MyMoneyAccount MyMoneyFile::income(void) const
{
  checkStorage();

  return m_storage->income();
}

const unsigned int MyMoneyFile::transactionCount(const QCString& account) const
{
  checkStorage();

  return m_storage->transactionCount(account);
}

const unsigned int MyMoneyFile::institutionCount(void) const
{
  checkStorage();

  return m_storage->institutionCount();
}

const MyMoneyMoney MyMoneyFile::balance(const QCString& id) const
{
  checkStorage();

  return m_storage->balance(id);
}

const MyMoneyMoney MyMoneyFile::totalBalance(const QCString& id) const
{
  checkStorage();

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
  if(it_s != m_subjects.end()) {
    (*it_s).notify(id);
  }
}

void MyMoneyFile::notify(void)
{
  QMap<QCString, bool>::ConstIterator it;
  for(it = m_notificationList.begin(); it != m_notificationList.end(); ++it) {
    notify(it.key());
  }

  clearNotification();
}

void MyMoneyFile::notifyAccountTree(const QCString& id)
{
  checkStorage();

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

const QValueList<MyMoneyTransaction> MyMoneyFile::transactionList(const QCString& account) const
{
  checkStorage();

  return m_storage->transactionList(account);
}

const QValueList<MyMoneyPayee> MyMoneyFile::payeeList(void) const
{
  checkStorage();

  return m_storage->payeeList();
}
