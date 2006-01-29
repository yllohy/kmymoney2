/***************************************************************************
                          imymoneystoragestream.cpp  -  description
                             -------------------
    begin                : Sun May 5 2002
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

#include "mymoneyseqaccessmgr.h"
#include "../mymoneytransactionfilter.h"
#include <qvaluevector.h>

const bool MyMoneyBalanceCacheItem::operator ==(const MyMoneyBalanceCacheItem & right) const
{
  return ((balance == right.balance)
        && (valid == right.valid));
}

MyMoneySeqAccessMgr::MyMoneySeqAccessMgr()
{
  m_nextAccountID = 0;
  m_nextInstitutionID = 0;
  m_nextTransactionID = 0;
  m_nextPayeeID = 0;
  m_nextScheduleID = 0;
  m_nextSecurityID = 0;
  m_nextReportID = 0;
  m_nextBudgetID = 0;
  m_user = MyMoneyPayee();
  m_dirty = false;
  m_institutionList.clear();
  m_accountList.clear();
  m_creationDate = QDate::currentDate();

  // setup standard accounts
  MyMoneyAccount acc_l;
  acc_l.setAccountType(MyMoneyAccount::Liability);
  acc_l.setName("Liability");
  MyMoneyAccount acc_a;
  acc_a.setAccountType(MyMoneyAccount::Asset);
  acc_a.setName("Asset");
  MyMoneyAccount acc_e;
  acc_e.setAccountType(MyMoneyAccount::Expense);
  acc_e.setName("Expense");
  MyMoneyAccount acc_i;
  acc_i.setAccountType(MyMoneyAccount::Income);
  acc_i.setName("Income");
  MyMoneyAccount acc_q;
  acc_q.setAccountType(MyMoneyAccount::Equity);
  acc_q.setName("Equity");

  MyMoneyAccount* a;
  a = new MyMoneyAccount(STD_ACC_LIABILITY, acc_l);
  m_accountList[STD_ACC_LIABILITY] = *a;
  delete a;

  a = new MyMoneyAccount(STD_ACC_ASSET, acc_a);
  m_accountList[STD_ACC_ASSET] = *a;
  delete a;

  a = new MyMoneyAccount(STD_ACC_EXPENSE, acc_e);
  m_accountList[STD_ACC_EXPENSE] = *a;
  delete a;

  a = new MyMoneyAccount(STD_ACC_INCOME, acc_i);
  m_accountList[STD_ACC_INCOME] = *a;
  delete a;

  a = new MyMoneyAccount(STD_ACC_EQUITY, acc_q);
  m_accountList[STD_ACC_EQUITY] = *a;
  delete a;

  MyMoneyBalanceCacheItem balance;

  m_balanceCache.clear();
  m_balanceCache[STD_ACC_LIABILITY] = balance;
  m_balanceCache[STD_ACC_ASSET] = balance;
  m_balanceCache[STD_ACC_EXPENSE] = balance;
  m_balanceCache[STD_ACC_INCOME] = balance;
  m_balanceCache[STD_ACC_EQUITY] = balance;
}

MyMoneySeqAccessMgr::~MyMoneySeqAccessMgr()
{
}

MyMoneySeqAccessMgr* const MyMoneySeqAccessMgr::duplicate(void)
{
  MyMoneySeqAccessMgr* that = new MyMoneySeqAccessMgr();
  *that = *this;
  return that;
}

const bool MyMoneySeqAccessMgr::isStandardAccount(const QCString& id) const
{
  return id == STD_ACC_LIABILITY
      || id == STD_ACC_ASSET
      || id == STD_ACC_EXPENSE
      || id == STD_ACC_INCOME
      || id == STD_ACC_EQUITY;
}

void MyMoneySeqAccessMgr::setAccountName(const QCString& id, const QString& name)
{
  if(!isStandardAccount(id))
    throw new MYMONEYEXCEPTION("Only standard accounts can be modified using setAccountName()");

  m_accountList[id].setName(name);
}

const MyMoneyAccount& MyMoneySeqAccessMgr::account(const QCString& id) const
{
  // locate the account and if present, return it's data
  if(m_accountList.find(id) != m_accountList.end())
    return m_accountList[id];

  // throw an exception, if it does not exist
  QString msg = "Unknown account id '" + id + "'";
  throw new MYMONEYEXCEPTION(msg);
}

const QValueList<MyMoneyAccount> MyMoneySeqAccessMgr::accountList(const QCStringList& idlist, const bool recursive) const
{
  QValueList<MyMoneyAccount> list;
  QMap<QCString, MyMoneyAccount>::ConstIterator it;

  for(it = m_accountList.begin(); it != m_accountList.end(); ++it) {
    if(!isStandardAccount((*it).id())) {
      if(idlist.isEmpty() || idlist.findIndex((*it).id()) != -1) {
        list.append(*it);
        if(!idlist.isEmpty() && recursive == true) {
          list += accountList((*it).accountList());
        }
      }
    }
  }
  return list;
}

void MyMoneySeqAccessMgr::addAccount(MyMoneyAccount& account)
{
  // we do not allow an opening balance different from 0 anymore
  // This used to work in older releases
  if(!account.openingBalance().isZero()) {
    // FIXME we should really generate an exception here. For now,
    // we only print a warning and hope that only old binary files
    // still have this feature. Once we get rid of the binary
    // reader we can safely throw the exception
    // throw new MYMONEYEXCEPTION("Cannot create account with opening balance != 0");
    qWarning("Createing accounts with opening balance != 0 in the engine is deprecated!");
  }

  // create the account.
  MyMoneyAccount newAccount(nextAccountID(), account);
  m_accountList[newAccount.id()] = newAccount;
  touch();
  account = newAccount;
}

void MyMoneySeqAccessMgr::addPayee(MyMoneyPayee& payee)
{
  // create the payee
  MyMoneyPayee newPayee(nextPayeeID(), payee);
  m_payeeList[newPayee.id()] = newPayee;
  touch();
  payee = newPayee;
}

const MyMoneyPayee& MyMoneySeqAccessMgr::payee(const QCString& id) const
{
  QMap<QCString, MyMoneyPayee>::ConstIterator it;

  it = m_payeeList.find(id);
  if(it == m_payeeList.end())
    throw new MYMONEYEXCEPTION("Unknown payee '" + id + "'");

  return *it;
}

const MyMoneyPayee& MyMoneySeqAccessMgr::payeeByName(const QString& payee) const
{
  if(payee.isEmpty())
    return MyMoneyPayee::null;

  QMap<QCString, MyMoneyPayee>::ConstIterator it_p;

  for(it_p = m_payeeList.begin(); it_p != m_payeeList.end(); ++it_p) {
    if((*it_p).name() == payee) {
      return *it_p;
    }
  }

  throw new MYMONEYEXCEPTION("Unknown payee '" + payee + "'");
}

void MyMoneySeqAccessMgr::modifyPayee(const MyMoneyPayee& payee)
{
  QMap<QCString, MyMoneyPayee>::Iterator it;

  it = m_payeeList.find(payee.id());
  if(it == m_payeeList.end()) {
    QString msg = "Unknown payee '" + payee.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }
  touch();
  *it = payee;
}

void MyMoneySeqAccessMgr::removePayee(const MyMoneyPayee& payee)
{
  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
  QValueList<MyMoneySplit>::ConstIterator it_s;
  QMap<QCString, MyMoneyPayee>::Iterator it_p;

  it_p = m_payeeList.find(payee.id());
  if(it_p == m_payeeList.end()) {
    QString msg = "Unknown payee '" + payee.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  // scan all transactions to check if the payee is still referenced
  for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    // scan all splits of this transaction
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      if((*it_s).payeeId() == payee.id())
        throw new MYMONEYEXCEPTION("Cannot remove payee that is referenced");
    }
  }
  // FIXME: check referential integrity in schedules

  m_payeeList.remove(it_p);
  touch();
}

const QValueList<MyMoneyPayee> MyMoneySeqAccessMgr::payeeList(void) const
{
  return m_payeeList.values();
}


void MyMoneySeqAccessMgr::addAccount(MyMoneyAccount& parent, MyMoneyAccount& account)
{
  QMap<QCString, MyMoneyAccount>::Iterator theParent;
  QMap<QCString, MyMoneyAccount>::Iterator theChild;

  theParent = m_accountList.find(parent.id());
  if(theParent == m_accountList.end()) {
    QString msg = "Unknown parent account '";
    msg += parent.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  theChild = m_accountList.find(account.id());
  if(theChild == m_accountList.end()) {
    QString msg = "Unknown child account '";
    msg += account.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  (*theParent).addAccountId(account.id());
  parent = *theParent;

  (*theChild).setParentAccountId(parent.id());
  account = *theChild;

  MyMoneyBalanceCacheItem balance;
  m_balanceCache[account.id()] = balance;

  touch();
}

void MyMoneySeqAccessMgr::addAccount(MyMoneyInstitution& institution, MyMoneyAccount& account)
{
  QMap<QCString, MyMoneyInstitution>::Iterator theInstitution;
  QMap<QCString, MyMoneyAccount>::Iterator theAccount;

  theInstitution = m_institutionList.find(institution.id());
  theAccount = m_accountList.find(account.id());

  if(theInstitution == m_institutionList.end())
    throw new MYMONEYEXCEPTION("Unknown institution");

  if(theAccount == m_accountList.end())
    throw new MYMONEYEXCEPTION("Unknown account");

  (*theInstitution).addAccountId(account.id());
  institution = *theInstitution;

  (*theAccount).setInstitutionId(institution.id());
  account = *theAccount;

  touch();
}

void MyMoneySeqAccessMgr::addInstitution(MyMoneyInstitution& institution)
{
  MyMoneyInstitution newInstitution(nextInstitutionID(), institution);

  m_institutionList[newInstitution.id()] = newInstitution;

  // mark file as changed
  touch();

  // return new data
  institution = newInstitution;
}

const unsigned int MyMoneySeqAccessMgr::transactionCount(const QCString& account) const
{
  unsigned int cnt = 0;

  if(account.length() == 0) {
    cnt = m_transactionList.count();

  } else {
    QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
    QValueList<MyMoneySplit>::ConstIterator it_s;

    // scan all transactions
    for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {

      // scan all splits of this transaction
      for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
        // is it a split in our account?
        if((*it_s).accountId() == account) {
          // since a transaction can only have one split referencing
          // each account, we're done with the splits here!
          break;
        }
      }
      // if no split contains the account id, continue with the
      // next transaction
      if(it_s == (*it_t).splits().end())
        continue;

      // otherwise count it
      ++cnt;
    }
  }
  return cnt;
}

const QMap<QCString, unsigned long> MyMoneySeqAccessMgr::transactionCountMap(void) const
{
  QMap<QCString, unsigned long> map;
  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
  QValueList<MyMoneySplit>::ConstIterator it_s;

  // scan all transactions
  for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    // scan all splits of this transaction
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      map[(*it_s).accountId()]++;
    }
  }
  return map;
}

const unsigned int MyMoneySeqAccessMgr::institutionCount(void) const
{
  return m_institutionList.count();
}

const unsigned int MyMoneySeqAccessMgr::accountCount(void) const
{
  return m_accountList.count();
}

const QCString MyMoneySeqAccessMgr::nextPayeeID(void)
{
  QCString id;
  id.setNum(++m_nextPayeeID);
  id = "P" + id.rightJustify(PAYEE_ID_SIZE, '0');
  return id;
}

const QCString MyMoneySeqAccessMgr::nextInstitutionID(void)
{
  QCString id;
  id.setNum(++m_nextInstitutionID);
  id = "I" + id.rightJustify(INSTITUTION_ID_SIZE, '0');
  return id;
}

const QCString MyMoneySeqAccessMgr::nextAccountID(void)
{
  QCString id;
  id.setNum(++m_nextAccountID);
  id = "A" + id.rightJustify(ACCOUNT_ID_SIZE, '0');
  return id;
}

const QCString MyMoneySeqAccessMgr::nextTransactionID(void)
{
  QCString id;
  id.setNum(++m_nextTransactionID);
  id = "T" + id.rightJustify(TRANSACTION_ID_SIZE, '0');
  return id;
}

const QCString MyMoneySeqAccessMgr::nextScheduleID(void)
{
  QCString id;
  id.setNum(++m_nextScheduleID);
  id = "SCH" + id.rightJustify(SCHEDULE_ID_SIZE, '0');
  return id;
}

const QCString MyMoneySeqAccessMgr::nextSecurityID(void)
{
  QCString id;
  id.setNum(++m_nextSecurityID);
  id = "E" + id.rightJustify(SECURITY_ID_SIZE, '0');
  return id;
}


void MyMoneySeqAccessMgr::addTransaction(MyMoneyTransaction& transaction, const bool skipAccountUpdate)
{
  // perform some checks to see that the transaction stuff is OK. For
  // now we assume that
  // * no ids are assigned
  // * the date valid (must not be empty)
  // * the referenced accounts in the splits exist

  // first perform all the checks
  if(!transaction.id().isEmpty())
    throw new MYMONEYEXCEPTION("transaction already contains an id");
  if(!transaction.postDate().isValid())
    throw new MYMONEYEXCEPTION("invalid post date");

  // now check the splits
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    // the following lines will throw an exception if the
    // account or payee do not exist
    account((*it_s).accountId());
    if(!(*it_s).payeeId().isEmpty())
      payee((*it_s).payeeId());
  }

  MyMoneyTransaction newTransaction(nextTransactionID(), transaction);
  QCString key = transactionKey(newTransaction);

  m_transactionList[key] = newTransaction;
  m_transactionKeys[newTransaction.id()] = key;

  transaction = newTransaction;

  // adjust the balance of all affected accounts
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s)
    m_accountList[(*it_s).accountId()].adjustBalance((*it_s).shares());

  // mark file as changed
  touch();

  if(!skipAccountUpdate) {
    // now update those accounts that need a reference
    // to this transaction
    for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
      QMap<QCString, MyMoneyAccount>::Iterator acc;
      acc = m_accountList.find((*it_s).accountId());
      if(acc != m_accountList.end()) {
        (*acc).touch();
        invalidateBalanceCache((*acc).id());
      }
    }
  }
}

const QCString MyMoneySeqAccessMgr::transactionKey(const MyMoneyTransaction& t) const
{
  QCString year, month, day;

  year = year.setNum(t.postDate().year()).rightJustify(YEAR_SIZE, '0');
  month = month.setNum(t.postDate().month()).rightJustify(MONTH_SIZE, '0');
  day = day.setNum(t.postDate().day()).rightJustify(DAY_SIZE, '0');

  return year + "-" + month + "-" + day + "-" + t.id();
}

void MyMoneySeqAccessMgr::touch(void)
{
  m_dirty = true;
  m_lastModificationDate = QDate::currentDate();
}

const bool MyMoneySeqAccessMgr::hasActiveSplits(const QCString& id) const
{
  QMap<QCString, MyMoneyTransaction>::ConstIterator it;

  for(it = m_transactionList.begin(); it != m_transactionList.end(); ++it) {
    if((*it).accountReferenced(id)) {
      return true;
    }
  }
  return false;
}

const MyMoneyInstitution& MyMoneySeqAccessMgr::institution(const QCString& id) const
{
  QMap<QCString, MyMoneyInstitution>::ConstIterator pos;

  pos = m_institutionList.find(id);
  if(pos != m_institutionList.end())
    return *pos;
  throw new MYMONEYEXCEPTION("unknown institution");
}

const QValueList<MyMoneyInstitution> MyMoneySeqAccessMgr::institutionList(void) const
{
  return m_institutionList.values();
}

void MyMoneySeqAccessMgr::modifyAccount(const MyMoneyAccount& account, const bool skipCheck)
{
  QMap<QCString, MyMoneyAccount>::Iterator pos;

  if(!account.openingBalance().isZero())
    throw new MYMONEYEXCEPTION("Cannot modify account with opening balance != 0");

  // locate the account in the file global pool
  pos = m_accountList.find(account.id());
  if(pos != m_accountList.end()) {
    // check if the new info is based on the old one.
    // this is the case, when the file and the id
    // as well as the type are equal.
    if((*pos).parentAccountId() == account.parentAccountId()
    && (*pos).accountType() == account.accountType()
    || skipCheck == true) {
      // if it points to a different institution, then update both
      if((*pos).institutionId() != account.institutionId()) {
        // check if new institution exists
        if(!account.institutionId().isEmpty())
          institution(account.institutionId());

        QMap<QCString, MyMoneyInstitution>::Iterator oldInst, newInst;
        oldInst = m_institutionList.find((*pos).institutionId());
        newInst = m_institutionList.find(account.institutionId());

        if(oldInst != m_institutionList.end()) {
          (*oldInst).removeAccountId(account.id());
        }
        if(newInst != m_institutionList.end()) {
          (*newInst).addAccountId(account.id());
        }
      }

      // update information in account list
      m_accountList[account.id()] = account;

      // invalidate cached balance
      invalidateBalanceCache(account.id());

      // mark file as changed
      touch();

    } else
      throw new MYMONEYEXCEPTION("Invalid information for update");

  } else
    throw new MYMONEYEXCEPTION("Unknown account id");
}

void MyMoneySeqAccessMgr::modifyInstitution(const MyMoneyInstitution& institution)
{
  QMap<QCString, MyMoneyInstitution>::Iterator pos;

  // locate the institution in the file global pool
  pos = m_institutionList.find(institution.id());
  if(pos != m_institutionList.end()) {
    *pos = institution;

    // mark file as changed
    touch();

  } else
    throw new MYMONEYEXCEPTION("unknown institution");
}

void MyMoneySeqAccessMgr::modifyTransaction(const MyMoneyTransaction& transaction)
{
  QMap<QCString, bool> modifiedAccounts;

  // perform some checks to see that the transaction stuff is OK. For
  // now we assume that
  // * ids are assigned
  // * the pointer to the MyMoneyFile object is not 0
  // * the date valid (must not be empty)
  // * the splits must have valid account ids

  // first perform all the checks
  if(transaction.id().isEmpty()
//  || transaction.file() != this
  || !transaction.postDate().isValid())
    throw new MYMONEYEXCEPTION("invalid transaction to be modified");

  // now check the splits
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    // the following lines will throw an exception if the
    // account or payee do not exist
    account((*it_s).accountId());
    if(!(*it_s).payeeId().isEmpty())
      payee((*it_s).payeeId());
  }

  // new data seems to be ok. find old version of transaction
  // in our pool. Throw exception if unknown.
  if(!m_transactionKeys.contains(transaction.id()))
    throw new MYMONEYEXCEPTION("invalid transaction id");

  QCString oldKey = m_transactionKeys[transaction.id()];
  if(!m_transactionList.contains(oldKey))
    throw new MYMONEYEXCEPTION("invalid transaction key");

  QMap<QCString, MyMoneyTransaction>::Iterator it_t;

  it_t = m_transactionList.find(oldKey);
  if(it_t == m_transactionList.end())
    throw new MYMONEYEXCEPTION("invalid transaction key");

  // mark all accounts referenced in old and new transaction data
  // as modified
  for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
    m_accountList[(*it_s).accountId()].adjustBalance(-((*it_s).shares()));
    modifiedAccounts[(*it_s).accountId()] = true;
  }
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    m_accountList[(*it_s).accountId()].adjustBalance((*it_s).shares());
    modifiedAccounts[(*it_s).accountId()] = true;
  }

  // remove old transaction from lists
  m_transactionList.remove(it_t);

  // add new transaction to lists
  QCString newKey = transactionKey(transaction);
  m_transactionList[newKey] = transaction;
  m_transactionKeys[transaction.id()] = newKey;

  // mark file as changed
  touch();

  // now update the accounts
  QMap<QCString, bool>::ConstIterator it_a;
  for(it_a = modifiedAccounts.begin(); it_a != modifiedAccounts.end(); ++it_a) {
    QMap<QCString, MyMoneyAccount>::Iterator acc;
    acc = m_accountList.find(it_a.key());
    if(acc != m_accountList.end()) {
      (*acc).touch();
      invalidateBalanceCache((*acc).id());
    }
  }
}

void MyMoneySeqAccessMgr::reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent)
{
  reparentAccount(account, parent, true);
}

void MyMoneySeqAccessMgr::reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent, const bool /* sendNotification */)
{
  QMap<QCString, MyMoneyAccount>::Iterator oldParent;
  QMap<QCString, MyMoneyAccount>::Iterator newParent;
  QMap<QCString, MyMoneyAccount>::Iterator childAccount;

  // verify that accounts exist. If one does not,
  // an exception is thrown
  MyMoneySeqAccessMgr::account(account.id());
  MyMoneySeqAccessMgr::account(parent.id());
  if(account.parentAccountId().length() != 0) {
    MyMoneySeqAccessMgr::account(account.parentAccountId());
    oldParent = m_accountList.find(account.parentAccountId());
  }

  if(account.accountType() == MyMoneyAccount::Stock && parent.accountType() != MyMoneyAccount::Investment)
    throw new MYMONEYEXCEPTION("Cannot move a stock acocunt into a non-investment account");

  newParent = m_accountList.find(parent.id());
  childAccount = m_accountList.find(account.id());

  if(account.parentAccountId().length() != 0)
    (*oldParent).removeAccountId(account.id());

  (*newParent).addAccountId(account.id());
  (*childAccount).setParentAccountId(parent.id());

#if 0
  // make sure the type is the same as the new parent. This does not work for stock and investment
  if(account.accountType() != MyMoneyAccount::Stock && account.accountType() != MyMoneyAccount::Investment)
    (*childAccount).setAccountType((*newParent).accountType());
#endif

  parent = *newParent;
  account = *childAccount;

  // mark file as changed
  touch();
}

void MyMoneySeqAccessMgr::removeTransaction(const MyMoneyTransaction& transaction)
{
  QMap<QCString, bool> modifiedAccounts;

  // first perform all the checks
  if(transaction.id().isEmpty())
    throw new MYMONEYEXCEPTION("invalid transaction to be deleted");

  QMap<QCString, QCString>::Iterator it_k;
  QMap<QCString, MyMoneyTransaction>::Iterator it_t;

  it_k = m_transactionKeys.find(transaction.id());
  if(it_k == m_transactionKeys.end())
    throw new MYMONEYEXCEPTION("invalid transaction to be deleted");

  it_t = m_transactionList.find(*it_k);
  if(it_t == m_transactionList.end())
    throw new MYMONEYEXCEPTION("invalid transaction key");

  QValueList<MyMoneySplit>::ConstIterator it_s;

  // scan the splits and collect all accounts that need
  // to be updated after the removal of this transaction
  for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
    modifiedAccounts[(*it_s).accountId()] = true;
    m_accountList[(*it_s).accountId()].adjustBalance(-((*it_s).shares()));
  }

  // FIXME: check if any split is frozen and throw exception

  // remove the transaction from the two lists
  m_transactionList.remove(it_t);
  m_transactionKeys.remove(it_k);

  // now update all the accounts that were referenced
  QMap<QCString, bool>::ConstIterator it_a;
  for(it_a = modifiedAccounts.begin(); it_a != modifiedAccounts.end(); ++it_a) {
    QMap<QCString, MyMoneyAccount>::Iterator acc;
    acc = m_accountList.find(it_a.key());
    if(acc != m_accountList.end()) {
      (*acc).touch();
      invalidateBalanceCache((*acc).id());
    }
  }

  // mark file as changed
  touch();
}

void MyMoneySeqAccessMgr::removeAccount(const MyMoneyAccount& account)
{
  MyMoneyAccount parent;

  // check that the account and it's parent exist
  // this will throw an exception if the id is unknown
  MyMoneySeqAccessMgr::account(account.id());
  parent = MyMoneySeqAccessMgr::account(account.parentAccountId());

  // check that it's not one of the standard account groups
  if(isStandardAccount(account.id()))
    throw new MYMONEYEXCEPTION("Unable to remove the standard account groups");

  if(hasActiveSplits(account.id())) {
    throw new MYMONEYEXCEPTION("Unable to remove account with active splits");
  }

  // re-parent all sub-ordinate accounts to the parent of the account
  // to be deleted. First round check that all accounts exist, second
  // round do the re-parenting.
  QCStringList::ConstIterator it;
  for(it = account.accountList().begin(); it != account.accountList().end(); ++it) {
    MyMoneySeqAccessMgr::account(*it);
  }

  // if one of the accounts did not exist, an exception had been
  // thrown and we would not make it until here.

  QMap<QCString, MyMoneyAccount>::Iterator it_a;
  QMap<QCString, MyMoneyAccount>::Iterator it_p;

  // locate the account in the file global pool

  it_a = m_accountList.find(account.id());
  if(it_a == m_accountList.end())
    throw new MYMONEYEXCEPTION("Internal error: account not found in list");

  it_p = m_accountList.find(parent.id());
  if(it_p == m_accountList.end())
    throw new MYMONEYEXCEPTION("Internal error: parent account not found in list");

  // FIXME: check referential integrity for the account to be removed

  // check if the new info is based on the old one.
  // this is the case, when the file and the id
  // as well as the type are equal.
  if((*it_a).id() == account.id()
  && (*it_a).institutionId() == account.institutionId()
  && (*it_a).accountType() == account.accountType()) {

    // second round over sub-ordinate accounts: do re-parenting
    // but only if the list contains at least one entry
    if((*it_a).accountList().count() > 0) {
      while((*it_a).accountList().count() > 0) {
        it = (*it_a).accountList().begin();
        MyMoneyAccount acc(MyMoneySeqAccessMgr::account(*it));
        reparentAccount(acc, parent, false);
      }
    }
    // remove account from parent's list
    (*it_p).removeAccountId(account.id());

    // remove account from the global account pool
    m_accountList.remove(it_a);

    // remove account from institution's list
    QMap<QCString, MyMoneyInstitution>::Iterator theInstitution;

    theInstitution = m_institutionList.find(account.institutionId());
    if(theInstitution != m_institutionList.end()) {
      (*theInstitution).removeAccountId(account.id());
    }

    // remove from balance list
    m_balanceCache.remove(account.id());
    invalidateBalanceCache(parent.id());

    // mark file as changed
    touch();
  }
}

void MyMoneySeqAccessMgr::removeInstitution(const MyMoneyInstitution& institution)
{
  QMap<QCString, MyMoneyInstitution>::Iterator it_i;

  it_i = m_institutionList.find(institution.id());
  if(it_i != m_institutionList.end()) {
    if((*it_i).accountCount() != 0) {
      QCStringList accounts = (*it_i).accountList();
      QCStringList::Iterator it_a;
      for(it_a = accounts.begin(); it_a != accounts.end(); ++it_a) {
        MyMoneyAccount acc = account(*it_a);
        acc.setInstitutionId(QCString());
        try {
          modifyAccount(acc);
        } catch(MyMoneyException *e) {
          qDebug("%s", e->what().data());
          delete e;
        }
      }
    }
    m_institutionList.remove(it_i);

    // mark file as changed
    touch();
  } else
    throw new MYMONEYEXCEPTION("invalid institution");
}

void MyMoneySeqAccessMgr::transactionList(QValueList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const
{
  list.clear();

  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;

#if 0
  if(!filter.filterSet().allFilter) {
    list = m_transactionList.values();

  } else {
#endif
    for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
      // This code is used now. It adds the transaction to the list for
      // each matching split exactly once. This allows to show information
      // about different splits in the same register view (e.g. search result)
      //
      // I have no idea, if this has some impact on the functionality. So far,
      // I could not see it.  (ipwizard 9/5/2003)
      if(filter.match(*it_t, this)) {
        unsigned int cnt = filter.matchingSplits().count();
        if(cnt > 1) {
          for(unsigned i=0; i < cnt; ++i)
            list.append(*it_t);
        } else {
          list.append(*it_t);
        }
      }
    }
//  }
}

QValueList<MyMoneyTransaction> MyMoneySeqAccessMgr::transactionList(MyMoneyTransactionFilter& filter) const
{
  QValueList<MyMoneyTransaction> list;
  transactionList(list, filter);
  return list;
}

const MyMoneyTransaction& MyMoneySeqAccessMgr::transaction(const QCString& id) const
{
  // get the full key of this transaction, throw exception
  // if it's invalid (unknown)
  if(!m_transactionKeys.contains(id))
    throw new MYMONEYEXCEPTION("invalid transaction id");

  // check if this key is in the list, throw exception if not
  QCString key = m_transactionKeys[id];
  if(!m_transactionList.contains(key))
    throw new MYMONEYEXCEPTION("invalid transaction key");

  return m_transactionList[key];
}

const MyMoneyTransaction& MyMoneySeqAccessMgr::transaction(const QCString& account, const int idx) const
{
/* removed with MyMoneyAccount::Transaction
  QMap<QCString, MyMoneyAccount>::ConstIterator acc;

  // find account object in list, throw exception if unknown
  acc = m_accountList.find(account);
  if(acc == m_accountList.end())
    throw new MYMONEYEXCEPTION("unknown account id");

  // get the transaction info from the account
  MyMoneyAccount::Transaction t = (*acc).transaction(idx);

  // return the transaction, throw exception if not found
  return transaction(t.transactionID());
*/

  // new implementation if the above code does not work anymore
  QValueList<MyMoneyTransaction> list;
  MyMoneyAccount acc = m_accountList[account];
  MyMoneyTransactionFilter filter;

  if(acc.accountGroup() == MyMoneyAccount::Income
  || acc.accountGroup() == MyMoneyAccount::Expense)
    filter.addCategory(account);
  else
    filter.addAccount(account);

  list = transactionList(filter);
  if(idx < 0 || idx >= static_cast<int> (list.count()))
    throw new MYMONEYEXCEPTION("Unknown idx for transaction");

  return transaction(list[idx].id());
}

const MyMoneyMoney MyMoneySeqAccessMgr::balance(const QCString& id, const QDate& date)
{
  MyMoneyMoney result(0);
  MyMoneyAccount acc;

  if(!date.isValid() && account(id).accountType() != MyMoneyAccount::Stock) {
    if(m_accountList.find(id) != m_accountList.end())
      return m_accountList[id].balance();
    return MyMoneyMoney(0);
  }
  if(m_balanceCache[id].valid == false || date != m_balanceCacheDate) {
    QMap<QCString, MyMoneyMoney> balances;
    QMap<QCString, MyMoneyMoney>::ConstIterator it_b;

    m_balanceCache.clear();
    m_balanceCacheDate = date;

    QValueList<MyMoneyTransaction> list;
    QValueList<MyMoneyTransaction>::ConstIterator it_t;
    QValueList<MyMoneySplit>::ConstIterator it_s;

    MyMoneyTransactionFilter filter;
    filter.setDateFilter(QDate(), date);
    filter.setReportAllSplits(false);
    list = transactionList(filter);

    for(it_t = list.begin(); it_t != list.end(); ++it_t) {
      for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s){
        const QCString& aid = (*it_s).accountId();
        if((*it_s).action() == MyMoneySplit::ActionSplitShares) {
          balances[aid] = balances[aid] * (*it_s).shares();
        } else {
          balances[aid] += (*it_s).value((*it_t).commodity(), m_accountList[aid].currencyId());
        }
      }
    }

    // fill the found balances into the cache
    for(it_b = balances.begin(); it_b != balances.end(); ++it_b) {
      MyMoneyBalanceCacheItem balance(*it_b);
      m_balanceCache[it_b.key()] = balance;
    }

    // fill all accounts w/o transactions to zero
    QMap<QCString, MyMoneyAccount>::ConstIterator it_a;
    for(it_a = m_accountList.begin(); it_a != m_accountList.end(); ++it_a) {
      if(m_balanceCache[(*it_a).id()].valid == false) {
        MyMoneyBalanceCacheItem balance(MyMoneyMoney(0,1));
        m_balanceCache[(*it_a).id()] = balance;
      }
    }
  }

  if(m_balanceCache[id].valid == true)
    result = m_balanceCache[id].balance;
  else
    qDebug("Cache mishit should never happen at this point");

  return result;
}

const MyMoneyMoney MyMoneySeqAccessMgr::totalBalance(const QCString& id, const QDate& date)
{
  QCStringList accounts;
  QCStringList::ConstIterator it_a;

  MyMoneyMoney result(balance(id, date));

  accounts = account(id).accountList();

  for(it_a = accounts.begin(); it_a != accounts.end(); ++it_a) {
    result += totalBalance(*it_a, date);
  }

  return result;
}

/**
  * this was intended to move all splits from one account
  * to another. This somehow is strange to undo because many
  * changes to different objects are made within one single call.
  * I kept the source here but commented it out. If we ever need
  * the functionality, we can turn it back on. BTW: the stuff is untested ;-)
  */
/*
const unsigned int MyMoneyFile::moveSplits(const QString& oldAccount, const QString& newAccount)
{
  QMap<QString, MyMoneyTransaction>::Iterator it_t;
  QValueList<MyMoneySplit>::ConstIterator it_s;
  unsigned int cnt = 0;

  // scan all transactions
  for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    // scan all splits of this transaction
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      // is it a split in our account?
      if((*it_s).account() == oldAccount) {
        MyMoneySplit s = *it_s;
        s.setAccount(newAccount);
        (*it_t).modifySplit(s);
        ++cnt;
      }
    }
  }

  if(cnt != 0) {
    // now update all the accounts that were referenced
    QMap<QString, MyMoneyAccount>::Iterator acc;
    acc = m_accountList.find(oldAccount);
    if(acc != m_accountList.end()) {
      (*acc).touch();
      refreshAccountTransactionList(acc);
    }
    acc = m_accountList.find(newAccount);
    if(acc != m_accountList.end()) {
      (*acc).touch();
      refreshAccountTransactionList(acc);
    }

    // mark file as changed
    m_dirty = true;
  }
  return cnt;
}
*/

void MyMoneySeqAccessMgr::invalidateBalanceCache(const QCString& id)
{
  if(!id.isEmpty()) {
    try {
      m_balanceCache[id].valid = false;
      if(!isStandardAccount(id)) {
        invalidateBalanceCache(account(id).parentAccountId());
      }
    } catch (MyMoneyException *e) {
      delete e;
    }
  }
}

void MyMoneySeqAccessMgr::loadAccount(const MyMoneyAccount& acc)
{
  if(acc.id() == asset().id()
  || acc.id() == liability().id()
  || acc.id() == expense().id()
  || acc.id() == income().id()
  || acc.id() == equity().id()) {
    m_accountList[acc.id()] = acc;
    return;
  }

  QMap<QCString, MyMoneyAccount>::ConstIterator it;

  it = m_accountList.find(acc.id());
  if(it != m_accountList.end()) {
    QString msg = "Duplicate account  '";
    msg += acc.id() + "' during loadAccount()";
    throw new MYMONEYEXCEPTION(msg);
  }
  m_accountList[acc.id()] = acc;

  // FIXME: Since usage of openingBalance is deprecated, this whole code can go eventually
  if(acc.accountType() != MyMoneyAccount::Stock
  && acc.accountType() != MyMoneyAccount::Investment)
    m_balanceCache[acc.id()] = MyMoneyBalanceCacheItem(acc.openingBalance());
}

void MyMoneySeqAccessMgr::loadTransaction(const MyMoneyTransaction& tr)
{
  QMap<QCString, MyMoneyTransaction>::ConstIterator it;
  QCString key = transactionKey(tr);

  it = m_transactionList.find(key);
  if(it != m_transactionList.end()) {
    QString msg = "Duplicate transaction  '";
    msg += tr.id() + "' during loadTransaction()";
    throw new MYMONEYEXCEPTION(msg);
  }
  m_transactionList[key] = tr;
  m_transactionKeys[tr.id()] = key;

  // (acejones) Cacheing the balance at load time doesn't work with Split
  // Shares transactions.  That's because the DATE matters with split shares,
  // and there's no guarantee these transactions are loaded in date order.
  //
  // There must be a more elegant way to handle this, but I don't know the
  // caching system well enough.  Instead, I'll just invalidate the cache
  // ALWAYS :-/
  //
  // (ipwizard) For stock accounts that might be ok, but for all others it
  // doesn't matter and it speeds up initial loading of the file by
  // magnitudes
  QValueList<MyMoneySplit> list = tr.splits();
  QValueList<MyMoneySplit>::ConstIterator it_s;

  for(it_s = list.begin(); it_s != list.end(); ++it_s) {
    const QCString& id((*it_s).accountId());
    if(account(id).accountType() != MyMoneyAccount::Stock) {
      m_balanceCache[id] = MyMoneyBalanceCacheItem(balance(id, QDate()) + (*it_s).value(tr.commodity(), account(id).currencyId()));
    }
  }
}

void MyMoneySeqAccessMgr::loadInstitution(const MyMoneyInstitution& inst)
{
  QMap<QCString, MyMoneyInstitution>::ConstIterator it;

  it = m_institutionList.find(inst.id());
  if(it != m_institutionList.end()) {
    QString msg = "Duplicate institution  '";
    msg += inst.id() + "' during loadInstitution()";
    throw new MYMONEYEXCEPTION(msg);
  }
  m_institutionList[inst.id()] = inst;
}

void MyMoneySeqAccessMgr::loadPayee(const MyMoneyPayee& payee)
{
  QMap<QCString, MyMoneyPayee>::ConstIterator it;

  it = m_payeeList.find(payee.id());
  if(it != m_payeeList.end()) {
    QString msg = "Duplicate payee  '";
    msg += payee.id() + "' during loadPayee()";
    throw new MYMONEYEXCEPTION(msg);
  }
  m_payeeList[payee.id()] = payee;
}

void MyMoneySeqAccessMgr::loadSecurity(const MyMoneySecurity& security)
{
  QMap<QCString, MyMoneySecurity>::ConstIterator it;

  it = m_securitiesList.find(security.id());
  if(it != m_securitiesList.end())
  {
    QString msg = "Duplicate security  '";
    msg += security.id() + "' during loadSecurity()";
    throw new MYMONEYEXCEPTION(msg);
  }
  m_securitiesList[security.id()] = security;
  //qDebug("loadSecurity: Security list size is %d, this=%8p", m_equitiesList.size(), (void*)this);
}

void MyMoneySeqAccessMgr::loadCurrency(const MyMoneySecurity& currency)
{
  QMap<QCString, MyMoneySecurity>::ConstIterator it;

  it = m_currencyList.find(currency.id());
  if(it != m_currencyList.end())
  {
    throw new MYMONEYEXCEPTION(QString("Duplicate currency '%1' during loadCurrency()").arg(currency.id().data()));
  }
  m_currencyList[currency.id()] = currency;
}

void MyMoneySeqAccessMgr::loadAccountId(const unsigned long id)
{
  m_nextAccountID = id;
}

void MyMoneySeqAccessMgr::loadTransactionId(const unsigned long id)
{
  m_nextTransactionID = id;
}

void MyMoneySeqAccessMgr::loadPayeeId(const unsigned long id)
{
  m_nextPayeeID = id;
}

void MyMoneySeqAccessMgr::loadInstitutionId(const unsigned long id)
{
  m_nextInstitutionID = id;
}

void MyMoneySeqAccessMgr::loadSecurityId(const unsigned long id)
{
  m_nextSecurityID = id;
}

void MyMoneySeqAccessMgr::loadReportId(const unsigned long id)
{
  m_nextReportID = id;
}

void MyMoneySeqAccessMgr::loadBudgetId(const unsigned long id)
{
  m_nextBudgetID = id;
}

const QString MyMoneySeqAccessMgr::value(const QCString& key) const
{
  return MyMoneyKeyValueContainer::value(key);
}

void MyMoneySeqAccessMgr::setValue(const QCString& key, const QString& val)
{
  MyMoneyKeyValueContainer::setValue(key, val);
  touch();
}

void MyMoneySeqAccessMgr::deletePair(const QCString& key)
{
  MyMoneyKeyValueContainer::deletePair(key);
  touch();
}

QMap<QCString, QString> MyMoneySeqAccessMgr::pairs(void) const
{
  return MyMoneyKeyValueContainer::pairs();
}

void MyMoneySeqAccessMgr::setPairs(const QMap<QCString, QString>& list)
{
  MyMoneyKeyValueContainer::setPairs(list);
  touch();
}

void MyMoneySeqAccessMgr::addSchedule(MyMoneySchedule& sched)
{
  // first perform all the checks
  if(!sched.id().isEmpty())
    throw new MYMONEYEXCEPTION("schedule already contains an id");

  // The following will throw an exception when it fails
  sched.validate(false);

  sched.setId(nextScheduleID());

  m_scheduleList[sched.id()] = sched;

  // mark file as changed
  touch();
}

void MyMoneySeqAccessMgr::modifySchedule(const MyMoneySchedule& sched)
{
  QMap<QCString, MyMoneySchedule>::Iterator it;

  it = m_scheduleList.find(sched.id());
  if(it == m_scheduleList.end()) {
    QString msg = "Unknown schedule '" + sched.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  *it = sched;
  // mark file as changed
  touch();
}

void MyMoneySeqAccessMgr::removeSchedule(const MyMoneySchedule& sched)
{
  QMap<QCString, MyMoneySchedule>::Iterator it;

  it = m_scheduleList.find(sched.id());
  if(it == m_scheduleList.end()) {
    QString msg = "Unknown schedule '" + sched.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  // FIXME: check referential integrity for loan accounts

  m_scheduleList.remove(it);
  touch();
}

const MyMoneySchedule MyMoneySeqAccessMgr::schedule(const QCString& id) const
{
  QMap<QCString, MyMoneySchedule>::ConstIterator pos;

  // locate the schedule and if present, return it's data
  pos = m_scheduleList.find(id);
  if(pos != m_scheduleList.end())
    return (*pos);

  // throw an exception, if it does not exist
  QString msg = "Unknown schedule id '" + id + "'";
  throw new MYMONEYEXCEPTION(msg);
}

const QValueList<MyMoneySchedule> MyMoneySeqAccessMgr::scheduleList(
                          const QCString& accountId,
                          const MyMoneySchedule::typeE type,
                          const MyMoneySchedule::occurenceE occurence,
                          const MyMoneySchedule::paymentTypeE paymentType,
                          const QDate& startDate,
                          const QDate& endDate,
                          const bool overdue) const
{
  QMap<QCString, MyMoneySchedule>::ConstIterator pos;
  QValueList<MyMoneySchedule> list;

  // qDebug("scheduleList()");

  for(pos = m_scheduleList.begin(); pos != m_scheduleList.end(); ++pos) {
    // qDebug("  '%s'", (*pos).id().data());

    if(type != MyMoneySchedule::TYPE_ANY) {
      if(type != (*pos).type()) {
        continue;
      }
    }

    if(occurence != MyMoneySchedule::OCCUR_ANY) {
      if(occurence != (*pos).occurence()) {
        continue;
      }
    }

    if(paymentType != MyMoneySchedule::STYPE_ANY) {
      if(paymentType != (*pos).paymentType()) {
        continue;
      }
    }

    if(!accountId.isEmpty()) {
      MyMoneyTransaction t = (*pos).transaction();
      QValueList<MyMoneySplit>::ConstIterator it;
      QValueList<MyMoneySplit> splits;
      splits = t.splits();
      for(it = splits.begin(); it != splits.end(); ++it) {
        if((*it).accountId() == accountId)
          break;
      }
      if(it == splits.end()) {
        continue;
      }
    }

    if(startDate.isValid() && endDate.isValid()) {
      if((*pos).paymentDates(startDate, endDate).count() == 0) {
        continue;
      }
    }

    if(startDate.isValid() && !endDate.isValid()) {
      if(!(*pos).nextPayment(startDate.addDays(-1)).isValid()) {
        continue;
      }
    }

    if(!startDate.isValid() && endDate.isValid()) {
      if((*pos).startDate() > endDate) {
        continue;
      }
    }

    if(overdue) {
      if (!(*pos).isOverdue())
        continue;
/*
      QDate nextPayment = (*pos).nextPayment((*pos).lastPayment());
      if(!nextPayment.isValid())
        continue;
      if(nextPayment >= QDate::currentDate())
        continue;
*/
    }

    // qDebug("Adding '%s'", (*pos).name().latin1());
    list << *pos;
  }
  return list;
}

void MyMoneySeqAccessMgr::loadSchedule(const MyMoneySchedule& sched)
{
  QMap<QCString, MyMoneySchedule>::ConstIterator it;

  it = m_scheduleList.find(sched.id());
  if(it != m_scheduleList.end()) {
    QString msg = "Duplicate scheduled '";
    msg += sched.id() + "' during loadSchedule()";
    throw new MYMONEYEXCEPTION(msg);
  }
  m_scheduleList[sched.id()] = sched;
}

void MyMoneySeqAccessMgr::loadScheduleId(const unsigned long id)
{
  m_nextScheduleID = id;
}

QValueList<MyMoneySchedule> MyMoneySeqAccessMgr::scheduleListEx(int scheduleTypes,
                                                                int scheduleOcurrences,
                                                                int schedulePaymentTypes,
                                                                QDate date,
                                                                const QCStringList& accounts) const
{
//  qDebug("scheduleListEx");

  QMap<QCString, MyMoneySchedule>::ConstIterator pos;
  QValueList<MyMoneySchedule> list;

  if (!date.isValid())
    return list;

  for(pos = m_scheduleList.begin(); pos != m_scheduleList.end(); ++pos)
  {
    if (scheduleTypes && !(scheduleTypes & (*pos).type()))
      continue;

    if (scheduleOcurrences && !(scheduleOcurrences & (*pos).occurence()))
      continue;

    if (schedulePaymentTypes && !(schedulePaymentTypes & (*pos).paymentType()))
      continue;

    if((*pos).paymentDates(date, date).count() == 0)
      continue;

    if ((*pos).isFinished())
      continue;

    if ((*pos).hasRecordedPayment(date))
      continue;

    if (accounts.count() > 0)
    {
      if (accounts.contains((*pos).account().id()))
        continue;
    }

//    qDebug("\tAdding '%s'", (*pos).name().latin1());
    list << *pos;
  }

  return list;
}

void MyMoneySeqAccessMgr::addSecurity(MyMoneySecurity& security)
{
  // create the account
  MyMoneySecurity newSecurity(nextSecurityID(), security);

  m_securitiesList[newSecurity.id()] = newSecurity;

  touch();
  security = newSecurity;
}

void MyMoneySeqAccessMgr::modifySecurity(const MyMoneySecurity& security)
{
  QMap<QCString, MyMoneySecurity>::ConstIterator it;

  it = m_securitiesList.find(security.id());
  if(it == m_securitiesList.end())
  {
    QString msg = "Unknown security  '";
    msg += security.id() + "' during modifySecurity()";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_securitiesList[security.id()] = security;
  touch();
}

void MyMoneySeqAccessMgr::removeSecurity(const MyMoneySecurity& security)
{
  QMap<QCString, MyMoneySecurity>::ConstIterator it;

  // FIXME: check referential integrity

  it = m_securitiesList.find(security.id());
  if(it == m_securitiesList.end())
  {
    QString msg = "Unknown security  '";
    msg += security.id() + "' during removeSecurity()";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_securitiesList.erase(security.id());
  touch();
}

const MyMoneySecurity MyMoneySeqAccessMgr::security(const QCString& id) const
{
  QMap<QCString, MyMoneySecurity>::ConstIterator it = m_securitiesList.find(id);
  if(it != m_securitiesList.end())
  {
    return it.data();
  }

  return MyMoneySecurity();
}

const QValueList<MyMoneySecurity> MyMoneySeqAccessMgr::securityList(void) const
{
  //qDebug("securityList: Security list size is %d, this=%8p", m_equitiesList.size(), (void*)this);
  return m_securitiesList.values();
}

void MyMoneySeqAccessMgr::addCurrency(const MyMoneySecurity& currency)
{
  QMap<QCString, MyMoneySecurity>::Iterator it;

  it = m_currencyList.find(currency.id());
  if(it != m_currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot add currency with existing id %1").arg(currency.id().data()));
  }

  m_currencyList[currency.id()] = currency;
  touch();
}

void MyMoneySeqAccessMgr::modifyCurrency(const MyMoneySecurity& currency)
{
  QMap<QCString, MyMoneySecurity>::Iterator it;

  it = m_currencyList.find(currency.id());
  if(it == m_currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot modify currency with unknown id %1").arg(currency.id().data()));
  }

  *it = currency;
  touch();
}

void MyMoneySeqAccessMgr::removeCurrency(const MyMoneySecurity& currency)
{
  QMap<QCString, MyMoneySecurity>::Iterator it;

  // FIXME: check referential integrity

  it = m_currencyList.find(currency.id());
  if(it == m_currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot remove currency with unknown id %1").arg(currency.id().data()));
  }

  m_currencyList.remove(it);
  touch();
}

const MyMoneySecurity MyMoneySeqAccessMgr::currency(const QCString& id) const
{
  if(id.isEmpty()) {

  }
  QMap<QCString, MyMoneySecurity>::ConstIterator it;

  it = m_currencyList.find(id);
  if(it == m_currencyList.end()) {
    MyMoneyException* e = new MYMONEYEXCEPTION(QString("Cannot retrieve currency with unknown id '%1'").arg(id.data()));
    qDebug("THROW: %s in %s:%ld", e->what().data(), e->file().data(), e->line());
    throw e;
  }

  return *it;
}

const QValueList<MyMoneySecurity> MyMoneySeqAccessMgr::currencyList(void) const
{
  return m_currencyList.values();
}

const QValueList<MyMoneyReport> MyMoneySeqAccessMgr::reportList(void) const
{
  return m_reportList.values();
}

void MyMoneySeqAccessMgr::addReport( MyMoneyReport& report )
{
  if(!report.id().isEmpty())
    throw new MYMONEYEXCEPTION("transaction already contains an id");

  report.setId( nextReportID() );

  m_reportList[report.id()] = report;

  touch();

}

void MyMoneySeqAccessMgr::loadReport( const MyMoneyReport& report )
{
  // in order to be 'loaded', this report must have an ID, and it must not already
  // be in the list.
  if ( report.id().isEmpty() )
    throw new MYMONEYEXCEPTION(QString("Unable to load report %1 with unassigned ID during loadReport()").arg(report.name()));
  if ( m_reportList.contains(report.id()) )
    throw new MYMONEYEXCEPTION(QString("Duplicate report %1 during loadReport()").arg(report.id()));

  m_reportList[report.id()] = report;
}

void MyMoneySeqAccessMgr::modifyReport( const MyMoneyReport& report )
{
  QMap<QCString, MyMoneyReport>::Iterator it;

  it = m_reportList.find(report.id());
  if(it == m_reportList.end()) {
    QString msg = "Unknown report '" + report.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }
  touch();

  *it = report;
}

const QCString MyMoneySeqAccessMgr::nextReportID(void)
{
  QCString id;
  id.setNum(++m_nextReportID);
  id = "R" + id.rightJustify(REPORT_ID_SIZE, '0');
  return id;
}

unsigned MyMoneySeqAccessMgr::countReports(void) const
{
  return m_reportList.count();
}

MyMoneyReport MyMoneySeqAccessMgr::report( const QCString& _id ) const
{
  return m_reportList[_id];
}

void MyMoneySeqAccessMgr::removeReport( const MyMoneyReport& report )
{
  QMap<QCString, MyMoneyReport>::Iterator it;

  it = m_reportList.find(report.id());
  if(it == m_reportList.end()) {
    QString msg = "Unknown report '" + report.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_reportList.remove(it);
  touch();
}

const QValueList<MyMoneyBudget> MyMoneySeqAccessMgr::budgetList(void) const
{
  return m_budgetList.values();
}


void MyMoneySeqAccessMgr::addBudget( MyMoneyBudget& budget )
{
  if(!budget.id().isEmpty())
    throw new MYMONEYEXCEPTION("transaction already contains an id");

  budget.setId( nextBudgetID() );

  m_budgetList[budget.id()] = budget;

  touch();

}

void MyMoneySeqAccessMgr::loadBudget( const MyMoneyBudget& budget )
{
  // in order to be 'loaded', this budget must have an ID, and it must not already
  // be in the list.
  if ( budget.id().isEmpty() )
    throw new MYMONEYEXCEPTION(QString("Unable to load budget %1 with unassigned ID during loadBudget()").arg(budget.name()));
  if ( m_budgetList.contains(budget.id()) )
    throw new MYMONEYEXCEPTION(QString("Duplicate budget %1 during loadBudget()").arg(budget.id()));

  m_budgetList[budget.id()] = budget;
}

void MyMoneySeqAccessMgr::modifyBudget( const MyMoneyBudget& budget )
{
  QMap<QCString, MyMoneyBudget>::Iterator it;

  it = m_budgetList.find(budget.id());
  if(it == m_budgetList.end()) {
    QString msg = "Unknown budget '" + budget.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }
  touch();

  *it = budget;
}

const QCString MyMoneySeqAccessMgr::nextBudgetID(void)
{
  QCString id;
  id.setNum(++m_nextBudgetID);
  id = "B" + id.rightJustify(BUDGET_ID_SIZE, '0');
  return id;
}

unsigned MyMoneySeqAccessMgr::countBudgets(void) const
{
  return m_budgetList.count();
}

MyMoneyBudget MyMoneySeqAccessMgr::budget( const QCString& _id ) const
{
  return m_budgetList[_id];
}

void MyMoneySeqAccessMgr::removeBudget( const MyMoneyBudget& budget )
{
  QMap<QCString, MyMoneyBudget>::Iterator it;

  it = m_budgetList.find(budget.id());
  if(it == m_budgetList.end()) {
    QString msg = "Unknown budget '" + budget.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_budgetList.remove(it);
  touch();
}

void MyMoneySeqAccessMgr::addPrice(const MyMoneyPrice& price)
{
  MyMoneyPriceEntries::ConstIterator it;
  it = m_priceList[MyMoneySecurityPair(price.from(), price.to())].find(price.date());

  // do not replace, if the information did not change.
  if(it != m_priceList[MyMoneySecurityPair(price.from(), price.to())].end()) {
    if((*it).rate() == price.rate()
    && (*it).source() == price.source())
      return;
  }

  m_priceList[MyMoneySecurityPair(price.from(), price.to())][price.date()] = price;
  touch();
}

void MyMoneySeqAccessMgr::removePrice(const MyMoneyPrice& price)
{
  m_priceList[MyMoneySecurityPair(price.from(), price.to())].remove(price.date());
  touch();
}

const MyMoneyPriceList& MyMoneySeqAccessMgr::priceList(void) const
{
  return m_priceList;
}

MyMoneyPrice MyMoneySeqAccessMgr::price(const QCString& fromId, const QCString& toId, const QDate& _date, const bool exactDate) const
{
  MyMoneyPrice rc;
  MyMoneyPriceEntries::ConstIterator it;
  QDate date(_date);

  if(!date.isValid())
    date = QDate::currentDate();

  for(it = m_priceList[MyMoneySecurityPair(fromId, toId)].begin(); it != m_priceList[MyMoneySecurityPair(fromId, toId)].end(); ++it) {
    if(date < it.key())
      break;

    if(exactDate) {
      if(date == it.key())
        rc = *it;

    } else {
      if(date >= it.key())
        rc = *it;
    }
  }
  return rc;
}

void MyMoneySeqAccessMgr::clearCache(void)
{
  m_balanceCache.clear();
}

void MyMoneySeqAccessMgr::rebuildAccountBalances(void)
{
  // reset the balance of all accounts to 0
  QMap<QCString, MyMoneyAccount>::iterator it_a;
  for(it_a = m_accountList.begin(); it_a != m_accountList.end(); ++it_a)
    (*it_a).setBalance(MyMoneyMoney(0));

  // now scan over all transactions and all splits and setup the balances
  QMap<QCString, MyMoneyTransaction>::const_iterator it_t;
  for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    const QValueList<MyMoneySplit>& splits = (*it_t).splits();
    QValueList<MyMoneySplit>::const_iterator it_s = splits.begin();
    for(; it_s != splits.end(); ++it_s ) {
      if(!(*it_s).shares().isZero()) {
        const QCString& id = (*it_s).accountId();
        // locate the account and if present, update data
        if(m_accountList.find(id) != m_accountList.end()) {
          m_accountList[id].adjustBalance((*it_s).shares());
        }
      }
    }
  }
}

bool MyMoneySeqAccessMgr::isReferenced(const MyMoneyObject& obj) const
{
  bool rc = false;
  const QCString& id = obj.id();
  QMap<QCString, MyMoneyTransaction>::const_iterator it_t;
  QMap<QCString, MyMoneyAccount>::const_iterator it_a;
  QMap<QCString, MyMoneyInstitution>::const_iterator it_i;
  QMap<QCString, MyMoneyPayee>::const_iterator it_p;
  QMap<QCString, MyMoneyReport>::const_iterator it_r;
  QMap<QCString, MyMoneyBudget>::const_iterator it_b;
  QMap<QCString, MyMoneySchedule>::const_iterator it_sch;
  QMap<QCString, MyMoneySecurity>::const_iterator it_sec;
  MyMoneyPriceList::const_iterator it_pr;

  // FIXME optimize the list of objects we have to checks
  //       with a bit of knowledge of the internal structure, we
  //       could optimize the number of objects we check for references

  // Scan all engine objects for a reference
  for(it_t = m_transactionList.begin(); !rc && it_t != m_transactionList.end(); ++it_t) {
    rc = (*it_t).hasReferenceTo(id);
  }
  for(it_a = m_accountList.begin(); !rc && it_a != m_accountList.end(); ++it_a) {
    rc = (*it_a).hasReferenceTo(id);
  }
  for(it_i = m_institutionList.begin(); !rc && it_i != m_institutionList.end(); ++it_i) {
    rc = (*it_i).hasReferenceTo(id);
  }
  for(it_p = m_payeeList.begin(); !rc && it_p != m_payeeList.end(); ++it_p) {
    rc = (*it_p).hasReferenceTo(id);
  }
  for(it_r = m_reportList.begin(); !rc && it_r != m_reportList.end(); ++it_r) {
    rc = (*it_r).hasReferenceTo(id);
  }
  for(it_b = m_budgetList.begin(); !rc && it_b != m_budgetList.end(); ++it_b) {
    rc = (*it_b).hasReferenceTo(id);
  }
  for(it_sch = m_scheduleList.begin(); !rc && it_sch != m_scheduleList.end(); ++it_sch) {
    rc = (*it_sch).hasReferenceTo(id);
  }
  for(it_sec = m_securitiesList.begin(); !rc && it_sec != m_securitiesList.end(); ++it_sec) {
    rc = (*it_sec).hasReferenceTo(id);
  }
  for(it_sec = m_currencyList.begin(); !rc && it_sec != m_currencyList.end(); ++it_sec) {
    rc = (*it_sec).hasReferenceTo(id);
  }
  // within the pricelist we don't have to scan each entry. Checking the QPair
  // members of the MyMoneySecurityPair is enough as they are identical to the
  // two security ids
  for(it_pr = m_priceList.begin(); !rc && it_pr != m_priceList.end(); ++it_pr) {
    rc = (it_pr.key().first == id) || (it_pr.key().second == id);
  }
  return rc;
}
// vim:cin:si:ai:et:ts=2:sw=2:
