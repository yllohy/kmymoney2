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
// #include "mymoneycurrency.h"
#include "mymoneyfile.h"
#include "mymoneyreport.h"
#ifndef HAVE_CONFIG_H
#define VERSION "UNKNOWN"
#else
#include "config.h"
#endif

#define CATEGORY_SEPERATOR ":"

const QCString MyMoneyFile::NotifyClassAccount = "MyMoneyFile::NotifyAccount";
const QCString MyMoneyFile::NotifyClassPayee = "MyMoneyFile::NotifyPayee";
const QCString MyMoneyFile::NotifyClassPayeeSet = "MyMoneyFile::NotifyPayeeSet";
const QCString MyMoneyFile::NotifyClassInstitution = "MyMoneyFile::NotifyInstitution";
const QCString MyMoneyFile::NotifyClassAccountHierarchy = "MyMoneyFile::NotifyAccountHierarchy";
const QCString MyMoneyFile::NotifyClassSchedule = "MyMoneyFile::NotifySchedule";
const QCString MyMoneyFile::NotifyClassAnyChange = "MyMoneyFile::NotifyAnyChange";
const QCString MyMoneyFile::NotifyClassCurrency = "MyMoneyFile::NotifyCurrency";
const QCString MyMoneyFile::NotifyClassSecurity = "MyMoneyFile::NotifySecurity";
const QCString MyMoneyFile::NotifyClassReport = "MyMoneyFile::NotifyReport";
const QCString MyMoneyFile::NotifyClassPrice = "MyMoneyFile::NotifyPrice";

const QString MyMoneyFile::OpeningBalancesPrefix = "Opening Balances";

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
  m_suspendNotify = false;
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

void MyMoneyFile::detachStorage(IMyMoneyStorage* const /* storage */)
{
  m_storage = 0;
}

void MyMoneyFile::addInstitution(MyMoneyInstitution& institution)
{
  // perform some checks to see that the institution stuff is OK. For
  // now we assume that the institution must have a name, the ID is not set
  // and it does not have a parent (MyMoneyFile).

  if(institution.name().length() == 0
  || institution.id().length() != 0)
    throw new MYMONEYEXCEPTION("Not a new institution");

  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->addInstitution(institution);

  addNotification(NotifyClassInstitution);
}

void MyMoneyFile::modifyInstitution(const MyMoneyInstitution& institution)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->modifyInstitution(institution);

  addNotification(institution.id());
  addNotification(NotifyClassInstitution);
}

void MyMoneyFile::modifyTransaction(const MyMoneyTransaction& transaction)
{
  checkStorage();

  const MyMoneyTransaction* t = &transaction;
  MyMoneyTransaction tCopy;

  // now check the splits
  bool loanAccountAffected = false;
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    // the following line will throw an exception if the
    // account does not exist
    MyMoneyAccount acc = MyMoneyFile::account((*it_s).accountId());
    if((acc.accountType() == MyMoneyAccount::Loan
    || acc.accountType() == MyMoneyAccount::AssetLoan)
    && ((*it_s).action() == MyMoneySplit::ActionTransfer))
      loanAccountAffected = true;
  }

  // change transfer splits between asset/liability and loan accounts
  // into amortization splits
  if(loanAccountAffected) {
    tCopy = transaction;
    for(it_s = tCopy.splits().begin(); it_s != tCopy.splits().end(); ++it_s) {
      if((*it_s).action() == MyMoneySplit::ActionTransfer) {
        MyMoneyAccount acc = MyMoneyFile::account((*it_s).accountId());

        if(acc.accountGroup() == MyMoneyAccount::Asset
        || acc.accountGroup() == MyMoneyAccount::Liability) {
          MyMoneySplit s = (*it_s);
          s.setAction(MyMoneySplit::ActionAmortization);
          tCopy.modifySplit(s);
          t = &tCopy;
        }
      }
    }
  }

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  // get the current setting of this transaction
  MyMoneyTransaction tr = MyMoneyFile::transaction(transaction.id());

  // and mark all accounts that are referenced
  for(it_s = tr.splits().begin(); it_s != tr.splits().end(); ++it_s) {
    notifyAccountTree((*it_s).accountId());
    if(!(*it_s).payeeId().isEmpty()) {
      addNotification((*it_s).payeeId());
      addNotification(NotifyClassPayee);
    }
  }

  // perform modification
  m_storage->modifyTransaction(*t);

  // and mark all accounts that are referenced
  for(it_s = t->splits().begin(); it_s != t->splits().end(); ++it_s) {
    notifyAccountTree((*it_s).accountId());
    if(!(*it_s).payeeId().isEmpty()) {
      addNotification((*it_s).payeeId());
      addNotification(NotifyClassPayee);
    }
  }
  addNotification(NotifyClassAccount);
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

  // if the account was moved to another insitution, we notify
  // the old one as well as the new one and the structure change
  if(acc.institutionId() != account.institutionId()) {
    addNotification(acc.institutionId());
    addNotification(account.institutionId());
    addNotification(NotifyClassInstitution);
  }

  m_storage->modifyAccount(account);

  notifyAccountTree(account.id());
  addNotification(NotifyClassAccount);
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
    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Stock:
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

  if(accountGroup(account.accountType()) == accountGroup(parent.accountType())
  || (account.accountType() == MyMoneyAccount::Income && parent.accountType() == MyMoneyAccount::Expense)
  || (account.accountType() == MyMoneyAccount::Expense && parent.accountType() == MyMoneyAccount::Income)) {

    if(account.accountType() == MyMoneyAccount::Stock && parent.accountType() != MyMoneyAccount::Investment)
      throw new MYMONEYEXCEPTION("Unable to reparent Stock to non-investment account");

    if(parent.accountType() == MyMoneyAccount::Investment && account.accountType() != MyMoneyAccount::Stock)
      throw new MYMONEYEXCEPTION("Unable to reparent non-stock to investment account");

    // automatically notify all observers once this routine is done
    MyMoneyNotifier notifier(this);

    // remember current account tree
    notifyAccountTree(account.id());

    m_storage->reparentAccount(account, parent);

    // and also keep the new one
    notifyAccountTree(account.id());
    addNotification(NotifyClassAccount);
    addNotification(NotifyClassAccountHierarchy);

  } else
    throw new MYMONEYEXCEPTION("Unable to reparent to different account type");
}

const MyMoneyInstitution& MyMoneyFile::institution(const QCString& id) const
{
  checkStorage();

  return m_storage->institution(id);
}

const MyMoneyAccount MyMoneyFile::account(const QCString& id) const
{
  checkStorage();

  MyMoneyAccount acc;
  acc = m_storage->account(id);
  ensureDefaultCurrency(acc);
  return acc;
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
    if(!(*it_s).payeeId().isEmpty()) {
      addNotification((*it_s).payeeId());
      addNotification(NotifyClassPayee);
    }
  }
  addNotification(NotifyClassAccount);

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

  // also notify the pseudo account class
  addNotification(NotifyClassAccount);
  addNotification(NotifyClassAccountHierarchy);
}

void MyMoneyFile::removeInstitution(const MyMoneyInstitution& institution)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  addNotification(institution.id());

  m_storage->removeInstitution(institution);

  addNotification(NotifyClassInstitution);
}

void MyMoneyFile::addAccount(MyMoneyAccount& account, MyMoneyAccount& parent)
{
  checkStorage();

  MyMoneyInstitution institution;

  // perform some checks to see that the account stuff is OK. For
  // now we assume that the account must have a name, has no
  // transaction and sub-accounts and parent account
  // it's own ID is not set and it does not have a pointer to (MyMoneyFile)

  if(account.name().length() == 0)
    throw new MYMONEYEXCEPTION("Account has no name");

  if(account.id().length() != 0)
    throw new MYMONEYEXCEPTION("New account must have no id");

// removed with MyMoneyAccount::Transaction
//  || account.transactionList().count() != 0
  if(account.accountList().count() != 0)
    throw new MYMONEYEXCEPTION("New account must have no sub-accounts");

  if(!account.parentAccountId().isEmpty())
    throw new MYMONEYEXCEPTION("New account must have no parent-id");

  if(account.accountType() == MyMoneyAccount::UnknownAccountType)
    throw new MYMONEYEXCEPTION("Account has invalid type");

  // make sure, that the parent account exists
  // if not, an exception is thrown. If it exists,
  // get a copy of the current data
  MyMoneyFile::account(parent.id());

#if 0
  // make sure that no account with the same name and type exist
  // asset andliability as well as income and expense are considered
  // the same type in this aspect.
  switch(account.accountGroup()) {
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
      if(!categoryToAccount(account.name()).isEmpty())
        throw new MYMONEYEXCEPTION("Account with that name already exists");
      // for now, we only support income/expense accounts in the base currency
      account.setCurrencyId(baseCurrency().id());
      break;

    default:
      if(!nameToAccount(account.name()).isEmpty())
        throw new MYMONEYEXCEPTION("Account with that name already exists");
      break;
  }
#endif

  // FIXME: make sure, that the parent has the same type
  // I left it out here because I don't know, if there is
  // a tight coupling between e.g. checking accounts and the
  // class asset. It certainly does not make sense to create an
  // expense account under an income account. Maybe it does, I don't know.

  // We enforce, that a stock account can never be a parent and
  // that the parent for a stock account must be an investment. Also,
  // an investment cannot have another investment account as it's parent
  if(parent.accountType() == MyMoneyAccount::Stock)
    throw new MYMONEYEXCEPTION("Stock account cannot be parent account");

  if(account.accountType() == MyMoneyAccount::Stock
  && parent.accountType() != MyMoneyAccount::Investment)
    throw new MYMONEYEXCEPTION("Stock account must have investment account as parent ");

  if(account.accountType() != MyMoneyAccount::Stock
  && parent.accountType() == MyMoneyAccount::Investment)
    throw new MYMONEYEXCEPTION("Investment account can only have stock accounts as children");

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  // if an institution is set, verify that it exists
  if(account.institutionId().length() != 0) {
    // check the presence of the institution. if it
    // does not exist, an exception is thrown
    institution = MyMoneyFile::institution(account.institutionId());
  }


  if(!account.openingDate().isValid()) {
    account.setOpeningDate(QDate::currentDate());
  }

  account.setParentAccountId(parent.id());

  MyMoneyMoney openingBalance(account.openingBalance());
  account.setOpeningBalance(MyMoneyMoney(0,1));

  m_storage->addAccount(account);
  m_storage->addAccount(parent, account);

  if(account.institutionId().length() != 0)
    m_storage->addAccount(institution, account);

  createOpeningBalanceTransaction(account, openingBalance);

  // parse the complete account tree and collect all
  // account and institution ids and also the pseudo account class
  notifyAccountTree(account.id());
  addNotification(NotifyClassAccount);
  addNotification(NotifyClassAccountHierarchy);
}

void MyMoneyFile::createOpeningBalanceTransaction(const MyMoneyAccount& acc, const MyMoneyMoney& balance)
{
  // if the opening balance is not zero, we need
  // to create the respective transaction
  if(!balance.isZero()) {
    MyMoneySecurity currency = security(acc.currencyId());
    MyMoneyAccount openAcc = openingBalanceAccount(currency);

    if(openAcc.openingDate() > acc.openingDate()) {
      openAcc.setOpeningDate(acc.openingDate());
      modifyAccount(openAcc);
    }

    MyMoneyTransaction t;
    MyMoneySplit s;

    t.setPostDate(acc.openingDate());
    t.setCommodity(acc.currencyId());

    s.setAccountId(acc.id());
    s.setShares(balance);
    s.setValue(balance);
    t.addSplit(s);

    s.setId(QCString());
    s.setAccountId(openAcc.id());
    s.setShares(-balance);
    s.setValue(-balance);
    t.addSplit(s);

    addTransaction(t);
  }
}

const MyMoneyAccount MyMoneyFile::openingBalanceAccount(const MyMoneySecurity& security)
{
  if(!security.isCurrency())
    throw new MYMONEYEXCEPTION("Opening balance for non currencies not supported");

  MyMoneyAccount acc;
  QRegExp match(QString("^%1").arg(MyMoneyFile::OpeningBalancesPrefix));

  QValueList<MyMoneyAccount> accounts;
  QValueList<MyMoneyAccount>::Iterator it;

  accounts = accountList(equity().accountList(), true);

  for(it = accounts.begin(); it != accounts.end(); ++it) {
    if(match.search((*it).name()) != -1) {
      if((*it).currencyId() == security.id()) {
        acc = *it;
        break;
      }
    }
  }

  if(acc.id().isEmpty()) {
    return createOpeningBalanceAccount(security);
  }

  return acc;
}

const MyMoneyAccount MyMoneyFile::createOpeningBalanceAccount(const MyMoneySecurity& security)
{
  MyMoneyAccount acc;
  QString name = MyMoneyFile::OpeningBalancesPrefix;
  if(security.id() != baseCurrency().id()) {
    name += QString(" (%1)").arg(security.id());
  }
  acc.setName(name);
  acc.setAccountType(MyMoneyAccount::Equity);
  acc.setCurrencyId(security.id());

  MyMoneyAccount parent = equity();
  this->addAccount(acc, parent);
  return acc;
}

void MyMoneyFile::addTransaction(MyMoneyTransaction& transaction)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  // perform some checks to see that the transaction stuff is OK. For
  // now we assume that
  // * no ids are assigned
  // * the date valid (must not be empty)
  // * the referenced accounts in the splits exist

  // first perform all the checks
  if(!transaction.id().isEmpty())
    throw new MYMONEYEXCEPTION("Unable to add transaction with id set");
  if(!transaction.postDate().isValid())
    throw new MYMONEYEXCEPTION("Unable to add transaction with invalid postdate");

  // now check the splits
  bool loanAccountAffected = false;
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    // the following line will throw an exception if the
    // account does not exist
    MyMoneyAccount acc = MyMoneyFile::account((*it_s).accountId());
    if(acc.accountType() == MyMoneyAccount::Loan
    || acc.accountType() == MyMoneyAccount::AssetLoan)
      loanAccountAffected = true;
  }

  // change transfer splits between asset/liability and loan accounts
  // into amortization splits
  if(loanAccountAffected) {
    for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
      if((*it_s).action() == MyMoneySplit::ActionTransfer) {
        MyMoneyAccount acc = MyMoneyFile::account((*it_s).accountId());

        if(acc.accountGroup() == MyMoneyAccount::Asset
        || acc.accountGroup() == MyMoneyAccount::Liability) {
          MyMoneySplit s = (*it_s);
          s.setAction(MyMoneySplit::ActionAmortization);
          transaction.modifySplit(s);
        }
      }
    }
  }

  // check that we have a commodity
  if(transaction.commodity().isEmpty()) {
    transaction.setCommodity(baseCurrency().id());
  }

  // then add the transaction to the file global pool
  m_storage->addTransaction(transaction);

  // scan the splits again to update notification list
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    notifyAccountTree((*it_s).accountId());
    if(!(*it_s).payeeId().isEmpty()) {
      addNotification((*it_s).payeeId());
      addNotification(NotifyClassPayee);
    }
  }
  addNotification(NotifyClassAccount);
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

void MyMoneyFile::addPayee(MyMoneyPayee& payee)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->addPayee(payee);

  addNotification(NotifyClassPayee);
  addNotification(NotifyClassPayeeSet);
}

const MyMoneyPayee MyMoneyFile::payee(const QCString& id) const
{
  checkStorage();

  return m_storage->payee(id);
}

const MyMoneyPayee MyMoneyFile::payeeByName(const QString& name) const
{
  checkStorage();

  return m_storage->payeeByName(name);
}

void MyMoneyFile::modifyPayee(const MyMoneyPayee& payee)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->modifyPayee(payee);

  addNotification(NotifyClassPayee);
}

void MyMoneyFile::removePayee(const MyMoneyPayee& payee)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->removePayee(payee);

  addNotification(NotifyClassPayee);
  addNotification(NotifyClassPayeeSet);
}

const QValueList<MyMoneyAccount> MyMoneyFile::accountList(const QCStringList& idlist, const bool recursive) const
{
  checkStorage();

  return m_storage->accountList(idlist, recursive);
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

void MyMoneyFile::setDirty(void) const
{
  checkStorage();

  m_storage->setDirty();
}

const unsigned int MyMoneyFile::accountCount(void) const
{
  checkStorage();

  return m_storage->accountCount();
}

void MyMoneyFile::ensureDefaultCurrency(MyMoneyAccount& acc) const
{
  if(acc.currencyId().isEmpty()) {
    if(!baseCurrency().id().isEmpty())
      acc.setCurrencyId(baseCurrency().id());
    // m_storage->modifyAccount(acc);
  }
}

const MyMoneyAccount MyMoneyFile::liability(void) const
{
  checkStorage();

  MyMoneyAccount acc = m_storage->liability();
  ensureDefaultCurrency(acc);
  return acc;
}

const MyMoneyAccount MyMoneyFile::asset(void) const
{
  checkStorage();

  MyMoneyAccount acc = m_storage->asset();
  ensureDefaultCurrency(acc);
  return acc;
}

const MyMoneyAccount MyMoneyFile::expense(void) const
{
  checkStorage();

  MyMoneyAccount acc = m_storage->expense();
  ensureDefaultCurrency(acc);
  return acc;
}

const MyMoneyAccount MyMoneyFile::income(void) const
{
  checkStorage();

  MyMoneyAccount acc = m_storage->income();
  ensureDefaultCurrency(acc);
  return acc;
}

const MyMoneyAccount MyMoneyFile::equity(void) const
{
  checkStorage();

  MyMoneyAccount acc = m_storage->equity();
  ensureDefaultCurrency(acc);
  return acc;
}

const unsigned int MyMoneyFile::transactionCount(const QCString& account) const
{
  checkStorage();

  return m_storage->transactionCount(account);
}

const QMap<QCString, unsigned long> MyMoneyFile::transactionCountMap(void) const
{
  checkStorage();

  return m_storage->transactionCountMap();
}

const unsigned int MyMoneyFile::institutionCount(void) const
{
  checkStorage();

  return m_storage->institutionCount();
}

const MyMoneyMoney MyMoneyFile::balance(const QCString& id, const QDate& date) const
{
  checkStorage();

  return m_storage->balance(id, date);
}

const MyMoneyMoney MyMoneyFile::totalBalance(const QCString& id, const QDate& date) const
{
  checkStorage();

  return m_storage->totalBalance(id, date);
}

void MyMoneyFile::warningMissingRate(const QCString& fromId, const QCString& toId) const
{
  MyMoneySecurity from, to;
  try {
    from = security(fromId);
    to = security(toId);
    qWarning("Missing price info for conversion from %s to %s", from.name().latin1(), to.name().latin1());

  } catch(MyMoneyException *e) {
    qFatal("Missing security caught in MyMoneyFile::warningMissingRate(): %s(%ld) %s", e->file().data(), e->line(), e->what().data());
    delete e;
  }
}

const bool MyMoneyFile::accountValueValid(const QCString& id) const
{
  bool result = true;
  try {
    // if the balance is zero, we don't care about conversion. It must be 0
    // and thus the currency / security doesn't matter. It will remain 0
    // and thus the value is valid.
    if(!balance(id).isZero()) {
      MyMoneyAccount acc;

      acc = account(id);

      if(acc.currencyId() != baseCurrency().id()) {
        // if the currency of the account differs from the base currency,
        // we have to distinguish between 'monetary' and 'stock' accounts
        // here. For monetary accounts, we assume that there is a direct price
        // information available (currency exchange rate). For stock this is
        // different, as stocks might be traded in a different currency than
        // the base currency. So we convert the price twice in this case.

        if(acc.accountType() == MyMoneyAccount::Stock) {
          MyMoneySecurity security = this->security(acc.currencyId());
          result = price(acc.currencyId(), security.tradingCurrency()).isValid();
          if(!result) {
            warningMissingRate(acc.currencyId(), security.tradingCurrency());
          }
          // if the trading currency differs from the base currency, we
          // calculate the conversion between these two currencies now.
          if(result == true && security.tradingCurrency() != baseCurrency().id()) {
            result = price(security.tradingCurrency(), baseCurrency().id()).isValid();
            if(!result) {
              warningMissingRate(security.tradingCurrency(), baseCurrency().id());
            }
          }
        } else {
          result = price(acc.currencyId(), baseCurrency().id()).isValid();
          if(!result) {
            warningMissingRate(acc.currencyId(), baseCurrency().id());
          }
        }
      }
    }
  } catch(MyMoneyException *e) {
    qDebug("MyMoneyFile::totalValueValid: %s thrown in %s line %ld",
      e->what().data(), e->file().data(), e->line());
    delete e;
  }
  return result;
}

const bool MyMoneyFile::totalValueValid(const QCString& id) const
{
  QCStringList accounts;
  QCStringList::ConstIterator it_a;

  bool result = accountValueValid(id);
  try {
    MyMoneyAccount acc;

    acc = this->account(id);
    accounts = acc.accountList();

    for(it_a = accounts.begin(); result == true && it_a != accounts.end(); ++it_a) {
      result = totalValueValid(*it_a);
    }
  } catch(MyMoneyException *e) {
    qDebug("MyMoneyFile::totalValueValid: %s thrown in %s line %ld",
      e->what().data(), e->file().data(), e->line());
    delete e;
  }
  return result;
}

const MyMoneyMoney MyMoneyFile::accountValue(const QCString& id) const
{
  MyMoneyMoney result(balance(id));
  try {
    MyMoneyAccount acc;

    acc = this->account(id);

    // we don't have to convert anything, if we're already on base currency
    if(acc.currencyId() != baseCurrency().id()) {
      // if the currency of the account differs from the base currency,
      // we have to distinguish between 'monetary' and 'stock' accounts
      // here. For monetary accounts, we assume that there is a direct price
      // information available (currency exchange rate). For stock this is
      // different, as stocks might be traded in a different currency than
      // the base currency. So we convert the price twice in this case.

      if(acc.accountType() == MyMoneyAccount::Stock) {
        MyMoneySecurity security = this->security(acc.currencyId());
        MyMoneyPrice price = this->price(acc.currencyId(), security.tradingCurrency());
        result = result * price.rate();
        // if the trading currency differs from the base currency, we
        // calculate the conversion between these two currencies now.
        if(security.tradingCurrency() != baseCurrency().id()) {
          price = this->price(security.tradingCurrency(), baseCurrency().id());
          result = result * price.rate();
        }
      } else {
        MyMoneyPrice price = this->price(acc.currencyId(), baseCurrency().id());
        result = result * price.rate();
      }
    }
  } catch(MyMoneyException *e) {
    qDebug("MyMoneyFile::accountValue: %s thrown in %s line %ld",
      e->what().data(), e->file().data(), e->line());
    delete e;
  }
  return result;
}

const MyMoneyMoney MyMoneyFile::totalValue(const QCString& id) const
{
  QCStringList accounts;
  QCStringList::ConstIterator it_a;

  MyMoneyMoney result(accountValue(id));
  try {
    MyMoneyAccount acc;

    acc = account(id);
    accounts = acc.accountList();

    for(it_a = accounts.begin(); it_a != accounts.end(); ++it_a) {
      result += totalValue(*it_a);
    }
  } catch(MyMoneyException *e) {
    qDebug("MyMoneyFile::totalValue: %s thrown in %s line %ld",
      e->what().data(), e->file().data(), e->line());
    delete e;
  }
  return result;
}

void MyMoneyFile::attach(const QCString& id, MyMoneyObserver* observer)
{
  // qDebug("attach 0x%08lX for %s", (unsigned long) observer, id.data());
  QMap<QCString, MyMoneyFileSubject>::Iterator it_s;

  // make sure an entry for the subject with the id exists
  m_subjects[id];

  it_s = m_subjects.find(id);
  (*it_s).attach(observer);
}

void MyMoneyFile::detach(const QCString& id, MyMoneyObserver* observer)
{
  // qDebug("detach 0x%08lX for %s", (unsigned long) observer, id.data());
  QMap<QCString, MyMoneyFileSubject>::Iterator it_s;

  it_s = m_subjects.find(id);
  if(it_s != m_subjects.end())
    (*it_s).detach(observer);
}

void MyMoneyFile::notify(const QCString& id)
{
  QMap<QCString, MyMoneyFileSubject>::Iterator it_s;

  it_s = m_subjects.find(id);
  if(it_s != m_subjects.end()) {
    (*it_s).notify(id);
  }
}

void MyMoneyFile::notify(void)
{
  if(!m_suspendNotify) {
    QMap<QCString, bool>::ConstIterator it;
    // keep a local copy so that the called update()
    // members can modify the original list
    QMap<QCString, bool> list = m_notificationList;
    for(it = list.begin(); it != list.end(); ++it) {
      notify(it.key());
    }

    if(list.count() > 0)
      notify(NotifyClassAnyChange);

    clearNotification();
  }
}

void MyMoneyFile::notifyAccountTree(const QCString& id)
{
  checkStorage();

  QCString accId = id;
  MyMoneyAccount acc;

  for(;;) {
    addNotification(accId);
    if(isStandardAccount(accId))
      break;
    acc = account(accId);
    addNotification(acc.institutionId());
    accId = acc.parentAccountId();
  }
}

void MyMoneyFile::addNotification(const QCString& id)
{
  if(!id.isEmpty())
    m_notificationList[id] = true;
}

void MyMoneyFile::clearNotification()
{
  // reset list to be empty
  m_notificationList.clear();
}

const QValueList<MyMoneyTransaction> MyMoneyFile::transactionList(MyMoneyTransactionFilter& filter) const
{
  checkStorage();

  return m_storage->transactionList(filter);
}

const QValueList<MyMoneyPayee> MyMoneyFile::payeeList(void) const
{
  checkStorage();

  return m_storage->payeeList();
}

const QString MyMoneyFile::accountToCategory(const QCString& accountId) const
{
  MyMoneyAccount acc;
  QString rc;

  acc = account(accountId);
  do {
    if(!rc.isEmpty())
      rc = QString(CATEGORY_SEPERATOR) + rc;
    rc = acc.name() + rc;
    acc = account(acc.parentAccountId());
  } while(!isStandardAccount(acc.id()));

  return rc;
}

const QCString MyMoneyFile::categoryToAccount(const QString& category) const
{
  QCString id;

  // search the category in the expense accounts and if it is not found, try
  // to locate it in the income accounts
  id = locateSubAccount(MyMoneyFile::instance()->expense(), category);
  if(id.isEmpty())
    id = locateSubAccount(MyMoneyFile::instance()->income(), category);

  return id;
}

const QCString MyMoneyFile::nameToAccount(const QString& name) const
{
  QCString id;

  // search the category in the asset accounts and if it is not found, try
  // to locate it in the liability accounts
  id = locateSubAccount(MyMoneyFile::instance()->asset(), name);
  if(id.isEmpty())
    id = locateSubAccount(MyMoneyFile::instance()->liability(), name);

  return id;
}

const QString MyMoneyFile::parentName(const QString& name) const
{
  return name.section(CATEGORY_SEPERATOR, 0, -2);
}

const QCString MyMoneyFile::locateSubAccount(const MyMoneyAccount& base, const QString& category) const
{
  MyMoneyAccount nextBase;
  QString level, remainder;
  level = category.section(CATEGORY_SEPERATOR, 0, 0);
  remainder = category.section(CATEGORY_SEPERATOR, 1);

  QCStringList list = base.accountList();
  QCStringList::ConstIterator it_a;

  for(it_a = list.begin(); it_a != list.end(); ++it_a) {
    nextBase = account(*it_a);
    if(nextBase.name().lower() == level.lower()) {
      if(remainder.isEmpty()) {
        return nextBase.id();
      }
      return locateSubAccount(nextBase, remainder);
    }
  }
  return QCString();
}

const QString MyMoneyFile::value(const QCString& key) const
{
  checkStorage();

  return m_storage->value(key);
}

void MyMoneyFile::setValue(const QCString& key, const QString& val)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->setValue(key, val);

  addNotification(NotifyClassAnyChange);
}

void MyMoneyFile::deletePair(const QCString& key)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->deletePair(key);

  addNotification(NotifyClassAnyChange);
}

void MyMoneyFile::addSchedule(MyMoneySchedule& sched)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->addSchedule(sched);

  addNotification(NotifyClassSchedule);
}

void MyMoneyFile::modifySchedule(const MyMoneySchedule& sched)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->modifySchedule(sched);

  addNotification(NotifyClassSchedule);
}

void MyMoneyFile::removeSchedule(const MyMoneySchedule& sched)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->removeSchedule(sched);

  addNotification(NotifyClassSchedule);
}

const MyMoneySchedule MyMoneyFile::schedule(const QCString& id) const
{
  checkStorage();

  return m_storage->schedule(id);
}

const QValueList<MyMoneySchedule> MyMoneyFile::scheduleList(
                          const QCString& accountId,
                          const MyMoneySchedule::typeE type,
                          const MyMoneySchedule::occurenceE occurence,
                          const MyMoneySchedule::paymentTypeE paymentType,
                          const QDate& startDate,
                          const QDate& endDate,
                          const bool overdue) const
{
  checkStorage();

  return m_storage->scheduleList(accountId, type, occurence, paymentType, startDate, endDate, overdue);
}


const QStringList MyMoneyFile::consistencyCheck(void)
{
  QValueList<MyMoneyAccount> list;
  QValueList<MyMoneyAccount>::Iterator it_a;
  QCStringList accountRebuild;
  QCStringList::ConstIterator it_c;

  MyMoneyAccount parent;
  MyMoneyAccount child;
  MyMoneyAccount toplevel;
  MyMoneyAccount::accountTypeE group;

  QCString parentId;
  QStringList rc;

  int problemCount = 0;
  QString problemAccount;

  // check that we have a storage object
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  // get the current list of accounts
  list = accountList();
  // add the standard accounts
  list << MyMoneyFile::instance()->asset();
  list << MyMoneyFile::instance()->liability();
  list << MyMoneyFile::instance()->income();
  list << MyMoneyFile::instance()->expense();

  for(it_a = list.begin(); it_a != list.end(); ++it_a) {
    // no more checks for standard accounts
    if(isStandardAccount((*it_a).id())) {
      continue;
    }

    group = accountGroup((*it_a).accountType());
    switch(group) {
      case MyMoneyAccount::Asset:
        toplevel = asset();
        break;
      case MyMoneyAccount::Liability:
        toplevel = liability();
        break;
      case MyMoneyAccount::Expense:
        toplevel = expense();
        break;
      case MyMoneyAccount::Income:
        toplevel = income();
        break;
      default:
        qWarning("%s:%d This should never happen!", __FILE__ , __LINE__);
        break;
    }

    // check that the parent exists
    parentId = (*it_a).parentAccountId();
    try {
      parent = account(parentId);
      if(group != accountGroup(parent.accountType())) {
        problemCount++;
        if(problemAccount != (*it_a).name()) {
          problemAccount = (*it_a).name();
          rc << QString("* Problem with account '%1'").arg(problemAccount);
        }
        // the parent belongs to a different group, so we reconnect to the
        // master group account (asset, liability, etc) to which this account
        // should belong and update it in the engine.
        rc << QString("  * Parent account '%1' belongs to a different group.").arg(parent.name());
        rc << QString("    New parent account is the top level account '%1'.").arg(toplevel.name());
        (*it_a).setParentAccountId(toplevel.id());

        // make sure to rebuild the sub-accounts of the top account
        // and the one we removed this account from
        if(accountRebuild.contains(toplevel.id()) == 0)
          accountRebuild << toplevel.id();
        if(accountRebuild.contains(parent.id()) == 0)
          accountRebuild << parent.id();
      }
    } catch(MyMoneyException *e) {
      delete e;
      // apparently, the parent does not exist anymore. we reconnect to the
      // master group account (asset, liability, etc) to which this account
      // should belong and update it in the engine.
      problemCount++;
      if(problemAccount != (*it_a).name()) {
        problemAccount = (*it_a).name();
        rc << QString("* Problem with account '%1'").arg(problemAccount);
      }
      rc << QString("  * The parent with id %1 does not exist anymore.").arg(parentId);
      rc << QString("    New parent account is the top level account '%1'.").arg(toplevel.name());
      (*it_a).setParentAccountId(toplevel.id());

      addNotification((*it_a).id());

      // make sure to rebuild the sub-accounts of the top account
      if(accountRebuild.contains(toplevel.id()) == 0)
        accountRebuild << toplevel.id();
    }

    // now check that all the children exist and have the correct type
    for(it_c = (*it_a).accountList().begin(); it_c != (*it_a).accountList().end(); ++it_c) {
      // check that the child exists
      try {
        child = account(*it_c);
      } catch(MyMoneyException *e) {
        problemCount++;
        if(problemAccount != (*it_a).name()) {
          problemAccount = (*it_a).name();
          rc << QString("* Problem with account '%1'").arg(problemAccount);
        }
        rc << QString("  * Child account with id %1 does not exist anymore.").arg(*it_c);
        rc << "    The child account list will be reconstructed.";
        if(accountRebuild.contains((*it_a).id()) == 0)
          accountRebuild << (*it_a).id();
      }
    }
    // if the account was modified, we need to update it in the engine
    if(!(m_storage->account((*it_a).id()) == (*it_a))) {
      try {
        m_storage->modifyAccount(*it_a, true);
        addNotification((*it_a).id());
      } catch (MyMoneyException *e) {
        delete e;
        rc << "  * Unable to update account data in engine";
        return rc;
      }
    }
  }

  if(accountRebuild.count() != 0) {
    rc << "* Reconstructing the child lists for";
  }

  // clear the affected lists
  for(it_a = list.begin(); it_a != list.end(); ++it_a) {
    if(accountRebuild.contains((*it_a).id())) {
      rc << QString("  %1").arg((*it_a).name());
      // clear the account list
      for(it_c = (*it_a).accountList().begin(); it_c != (*it_a).accountList().end();) {
        (*it_a).removeAccountId(*it_c);
        it_c = (*it_a).accountList().begin();
      }
    }
  }

  // reconstruct the lists
  for(it_a = list.begin(); it_a != list.end(); ++it_a) {
    QValueList<MyMoneyAccount>::Iterator it;
    parentId = (*it_a).parentAccountId();
    if(accountRebuild.contains(parentId)) {
      for(it = list.begin(); it != list.end(); ++it) {
        if((*it).id() == parentId) {
          (*it).addAccountId((*it_a).id());
          break;
        }
      }
    }
  }

  // update the engine objects
  for(it_a = list.begin(); it_a != list.end(); ++it_a) {
    if(accountRebuild.contains((*it_a).id())) {
      try {
        m_storage->modifyAccount(*it_a, true);
        notifyAccountTree((*it_a).id());
      } catch (MyMoneyException *e) {
        delete e;
        rc << QString("  * Unable to update account data for account %1 in engine").arg((*it_a).name());
      }
    }
  }

  addNotification(NotifyClassAccount);
  addNotification(NotifyClassAccountHierarchy);

  // add more checks here

  if(problemCount == 0)
    rc << "Finish! Data is consistent.";
  else
    rc << QString("Finish! %1 problems corrected. Data is consistent.")
            .arg(QString::number(problemCount));

  return rc;
}

QCString MyMoneyFile::createCategory(const MyMoneyAccount& base, const QString& name)
{
  MyMoneyAccount parent = base;
  QString categoryText;

  if(base.id() != expense().id() && base.id() != income().id())
    throw MYMONEYEXCEPTION("Invalid base category");

  QStringList subAccounts = QStringList::split(CATEGORY_SEPERATOR, name);
  QStringList::Iterator it;
  for (it = subAccounts.begin(); it != subAccounts.end(); ++it)
  {
    MyMoneyAccount categoryAccount;

    categoryAccount.setName(*it);
    categoryAccount.setAccountType(base.accountType());

    if (it == subAccounts.begin())
      categoryText += *it;
    else
      categoryText += (":" + *it);

    // Only create the account if it doesn't exist
    try
    {
      QCString categoryId = categoryToAccount(categoryText);
      if (categoryId.isEmpty())
        addAccount(categoryAccount, parent);
      else
      {
        categoryAccount = account(categoryId);
      }
    }
    catch (MyMoneyException *e)
    {
      qDebug("Unable to add account %s, %s, %s: %s",
        categoryAccount.name().latin1(),
        parent.name().latin1(),
        categoryText.latin1(),
        e->what().latin1());
      delete e;
    }

    parent = categoryAccount;
  }

  return categoryToAccount(name);
}

QValueList<MyMoneySchedule> MyMoneyFile::scheduleListEx( int scheduleTypes,
                                              int scheduleOcurrences,
                                              int schedulePaymentTypes,
                                              QDate startDate,
                                              const QCStringList& accounts) const
{
  checkStorage();

  return m_storage->scheduleListEx(scheduleTypes, scheduleOcurrences, schedulePaymentTypes, startDate, accounts);
}

void MyMoneyFile::suspendNotify(const bool state)
{
  bool prevState = m_suspendNotify;
  m_suspendNotify = state;

  if(state == false && prevState == true)
    notify();
  // qDebug("Notification turned %s", state ? "off" : "on");
}


void MyMoneyFile::addSecurity(MyMoneySecurity& security)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->addSecurity(security);
  addNotification(NotifyClassSecurity);
}

void MyMoneyFile::modifySecurity(const MyMoneySecurity& security)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->modifySecurity(security);
  addNotification(security.id());
  addNotification(NotifyClassSecurity);
}

void MyMoneyFile::removeSecurity(const MyMoneySecurity& security)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->removeSecurity(security);
  addNotification(NotifyClassSecurity);
}

const MyMoneySecurity MyMoneyFile::security(const QCString& id) const
{
  if(id.isEmpty())
    return baseCurrency();

  checkStorage();

  MyMoneySecurity e = m_storage->security(id);
  if(e.id().isEmpty())
    e = m_storage->currency(id);
  return e;
}

const QValueList<MyMoneySecurity> MyMoneyFile::securityList(void) const
{
  checkStorage();

  return m_storage->securityList();
}

void MyMoneyFile::addCurrency(const MyMoneySecurity& currency)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->addCurrency(currency);
  addNotification(NotifyClassCurrency);
}

void MyMoneyFile::modifyCurrency(const MyMoneySecurity& currency)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->modifyCurrency(currency);
  addNotification(NotifyClassCurrency);
}

void MyMoneyFile::removeCurrency(const MyMoneySecurity& currency)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->removeCurrency(currency);
  addNotification(NotifyClassCurrency);
}

const MyMoneySecurity MyMoneyFile::currency(const QCString& id) const
{
  if(id.isEmpty())
    return baseCurrency();

  checkStorage();
  return m_storage->currency(id);
}

const QValueList<MyMoneySecurity> MyMoneyFile::currencyList(void) const
{
  checkStorage();

  return m_storage->currencyList();
}

const MyMoneySecurity MyMoneyFile::baseCurrency(void) const
{
  QCString id = QCString(value("kmm-baseCurrency"));
  if(id.isEmpty())
    return MyMoneySecurity();

  return currency(id);
}

void MyMoneyFile::setBaseCurrency(const MyMoneySecurity& curr)
{
  // make sure the currency exists
  MyMoneySecurity c = currency(curr.id());

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  setValue("kmm-baseCurrency", curr.id());

  addNotification(NotifyClassCurrency);
}

void MyMoneyFile::addPrice(const MyMoneyPrice& price)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->addPrice(price);

  addNotification(NotifyClassPrice);
  addNotification(price.from());
  addNotification(price.to());
}

void MyMoneyFile::removePrice(const MyMoneyPrice& price)
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  addNotification(NotifyClassPrice);
  addNotification(price.from());
  addNotification(price.to());

  m_storage->removePrice(price);
}

const MyMoneyPrice MyMoneyFile::price(const QCString& fromId, const QCString& toId, const QDate& date, const bool exactDate) const
{
  checkStorage();

  QCString to(toId);
  if(to.isEmpty())
    to = value("kmm-baseCurrency");
  // if some id is missing, we can return an empty price object
  if(fromId.isEmpty() || to.isEmpty())
    return MyMoneyPrice();

  MyMoneyPrice rc = m_storage->price(fromId, to, date, exactDate);
  if(!rc.isValid()) {
    rc = m_storage->price(to, fromId, date, exactDate);
  }
  return rc;
}

const MyMoneyPriceList MyMoneyFile::priceList(void) const
{
  checkStorage();

  return m_storage->priceList();
}

const bool MyMoneyFile::hasAccount(const QCString& id, const QString& name) const
{
  checkStorage();

  MyMoneyAccount acc = m_storage->account(id);
  QCStringList list = acc.accountList();
  QCStringList::ConstIterator it;
  bool rc = false;
  for(it = list.begin(); rc == false && it != list.end(); ++it) {
    MyMoneyAccount a = m_storage->account(*it);
    if(a.name() == name)
      rc = true;
  }
  return rc;
}

const QValueList<MyMoneyReport> MyMoneyFile::reportList( void ) const
{
  checkStorage();

  return m_storage->reportList();
}

void MyMoneyFile::addReport( MyMoneyReport& report )
{
  checkStorage();

  m_storage->addReport( report );
}

void MyMoneyFile::modifyReport( const MyMoneyReport& report )
{
  checkStorage();

  // automatically notify all observers once this routine is done
  MyMoneyNotifier notifier(this);

  m_storage->modifyReport( report );

  addNotification(NotifyClassReport);
}

unsigned MyMoneyFile::countReports(void) const
{
  checkStorage();

  return m_storage->countReports();
}

MyMoneyReport MyMoneyFile::report( const QCString& id ) const
{
  checkStorage();

  return m_storage->report(id);
}

void MyMoneyFile::removeReport(const MyMoneyReport& report)
{
  checkStorage();
  MyMoneyNotifier notifier(this);

  m_storage->removeReport(report);

  addNotification(NotifyClassReport);
}

const bool MyMoneyFile::isReferenced(const MyMoneySecurity& security) const
{
  // check the account list
  QValueList<MyMoneyAccount> list_a = accountList();
  QValueList<MyMoneyAccount>::ConstIterator it_a;
  for(it_a = list_a.begin(); it_a != list_a.end(); ++it_a) {
    if((*it_a).currencyId() == security.id())
      return true;
  }

  // check the security list
  QValueList<MyMoneySecurity> list_s = securityList();
  list_s += currencyList();
  QValueList<MyMoneySecurity>::ConstIterator it_s;
  for(it_s = list_s.begin(); it_s != list_s.end(); ++it_s) {
    // don't check myself
    if((*it_s).id() == security.id())
      continue;
    if((*it_s).tradingCurrency() == security.id())
      return true;
  }

  // check the price list
  MyMoneyPriceList list_p = priceList();
  MyMoneyPriceList::ConstIterator it_p;
  for(it_p = list_p.begin(); it_p != list_p.end(); ++it_p) {
    if(it_p.key().first == security.id()
    || it_p.key().second == security.id())
      return true;
  }
  return false;
}
