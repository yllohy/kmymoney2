/***************************************************************************
                          mymoneydatabasemgr.cpp
                             -------------------
    begin                : June 5 2007
    copyright            : (C) 2007 by Fernando Vilas
    email                : Fernando Vilas <fvilas@iname.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "mymoneydatabasemgr.h"
#include "../mymoneytransactionfilter.h"
#include "../mymoneycategory.h"

#define TRY try {
#define CATCH } catch (MyMoneyException *e) {
#define PASS } catch (MyMoneyException *e) { throw; }

MyMoneyDatabaseMgr::MyMoneyDatabaseMgr() :
m_lastModificationDate (QDate::currentDate ()),
m_sql (0)
{ }

MyMoneyDatabaseMgr::~MyMoneyDatabaseMgr()
{
  //close();
}

  // general get functions
const MyMoneyPayee MyMoneyDatabaseMgr::user(void) const
{ return m_user; }

const QDate MyMoneyDatabaseMgr::creationDate(void) const
{ return m_creationDate; }

const QDate MyMoneyDatabaseMgr::lastModificationDate(void) const
{ return m_lastModificationDate; }

const unsigned int MyMoneyDatabaseMgr::currentFixVersion(void) const
{ return m_sql->currentVersion(); }

const unsigned int MyMoneyDatabaseMgr::fileFixVersion(void) const
{ return m_fileFixVersion; }

  // general set functions
void MyMoneyDatabaseMgr::setUser(const MyMoneyPayee& user)
{
  m_user = user;
  if (m_sql != 0) m_sql->modifyUserInfo(user);
}

void MyMoneyDatabaseMgr::setFileFixVersion(const unsigned int v)
{ m_fileFixVersion = v; }

  // methods provided by MyMoneyKeyValueContainer
const QString MyMoneyDatabaseMgr::value(const QCString& key) const
{
  return MyMoneyKeyValueContainer::value(key);
}

void MyMoneyDatabaseMgr::setValue(const QCString& key, const QString& val)
{
  MyMoneyKeyValueContainer::setValue(key, val);
}

void MyMoneyDatabaseMgr::deletePair(const QCString& key)
{
  MyMoneyKeyValueContainer::deletePair(key);
}

const QMap<QCString, QString> MyMoneyDatabaseMgr::pairs(void) const
{
  return MyMoneyKeyValueContainer::pairs();
}

void MyMoneyDatabaseMgr::setPairs(const QMap<QCString, QString>& list)
{
  MyMoneyKeyValueContainer::setPairs(list);
}

MyMoneyDatabaseMgr* const MyMoneyDatabaseMgr::duplicate(void)
{
  MyMoneyDatabaseMgr* that = new MyMoneyDatabaseMgr();
  *that = *this;
  return that;
}

void MyMoneyDatabaseMgr::addAccount(MyMoneyAccount& account)
{
  // create the account.
  MyMoneyAccount newAccount(nextAccountID(), account);

  m_sql->addAccount(newAccount);
  account = newAccount;
}

void MyMoneyDatabaseMgr::addAccount(MyMoneyAccount& parent, MyMoneyAccount& account)
{
  QMap<QCString, MyMoneyAccount> accountList;
  QStringList accountIdList;
  QMap<QCString, MyMoneyAccount>::ConstIterator theParent;
  QMap<QCString, MyMoneyAccount>::ConstIterator theChild;

  accountIdList << parent.id() << account.id();
  startTransaction();
  accountList = m_sql->fetchAccounts(accountIdList, true);

  theParent = accountList.find(parent.id());
  if(theParent == accountList.end()) {
    QString msg = "Unknown parent account '";
    msg += parent.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  theChild = accountList.find(account.id());
  if(theChild == accountList.end()) {
    QString msg = "Unknown child account '";
    msg += account.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  MyMoneyAccount acc = *theParent;
  acc.addAccountId(account.id());
  parent = acc;

  acc = *theChild;
  acc.setParentAccountId(parent.id());
  account = acc;

//FIXME:  MyMoneyBalanceCacheItem balance;
//FIXME:  m_balanceCache[account.id()] = balance;

  m_sql->modifyAccount(parent);
  m_sql->modifyAccount(account);
  commitTransaction();
}

void MyMoneyDatabaseMgr::addPayee(MyMoneyPayee& payee)
{
  // create the payee
  MyMoneyPayee newPayee(nextPayeeID(), payee);

  m_sql->addPayee(newPayee);
  payee = newPayee;
}

const MyMoneyPayee MyMoneyDatabaseMgr::payee(const QCString& id) const
{
  QMap<QCString, MyMoneyPayee>::ConstIterator it;
  QMap<QCString, MyMoneyPayee> payeeList = m_sql->fetchPayees(QString(id));
  it = payeeList.find(id);
  if(it == payeeList.end())
    throw new MYMONEYEXCEPTION("Unknown payee '" + id + "'");

  return *it;
}

const MyMoneyPayee MyMoneyDatabaseMgr::payeeByName(const QString& payee) const
{
  if(payee.isEmpty())
    return MyMoneyPayee::null;

  QMap<QCString, MyMoneyPayee> payeeList;

  TRY
  payeeList = m_sql->fetchPayees();
  PASS

  QMap<QCString, MyMoneyPayee>::ConstIterator it_p;

  for(it_p = payeeList.begin(); it_p != payeeList.end(); ++it_p) {
    if((*it_p).name() == payee) {
      return *it_p;
    }
  }

  throw new MYMONEYEXCEPTION("Unknown payee '" + payee + "'");
}

void MyMoneyDatabaseMgr::modifyPayee(const MyMoneyPayee& payee)
{
  QMap<QCString, MyMoneyPayee> payeeList = m_sql->fetchPayees(QString(payee.id()), true);
  QMap<QCString, MyMoneyPayee>::ConstIterator it;

  it = payeeList.find(payee.id());
  if(it == payeeList.end()) {
    QString msg = "Unknown payee '" + payee.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_sql->modifyPayee(payee);
}

void MyMoneyDatabaseMgr::removePayee(const MyMoneyPayee& payee)
{
  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
  QValueList<MyMoneySplit>::ConstIterator it_s;
  QMap<QCString, MyMoneyPayee> payeeList = m_sql->fetchPayees(QString(payee.id()));
  QMap<QCString, MyMoneyPayee>::ConstIterator it_p;

  it_p = payeeList.find(payee.id());
  if(it_p == payeeList.end()) {
    QString msg = "Unknown payee '" + payee.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  // scan all transactions to check if the payee is still referenced
  MyMoneyTransactionFilter f;
  f.addPayee(payee.id());

  QMap<QCString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions(f); // make sure they're all here

  for(it_t = transactionList.begin(); it_t != transactionList.end(); ++it_t) {
    // scan all splits of this transaction
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      if((*it_s).payeeId() == payee.id())
        throw new MYMONEYEXCEPTION("Cannot remove payee that is referenced");
    }
  }
  // FIXME: check referential integrity in schedules

  m_sql->removePayee(payee);
}

const QValueList<MyMoneyPayee> MyMoneyDatabaseMgr::payeeList(void) const
{
  return m_sql->fetchPayees().values();
}

const MyMoneyAccount MyMoneyDatabaseMgr::account(const QCString& id) const
{
  QMap <QCString, MyMoneyAccount> accountList = m_sql->fetchAccounts(QString(id));
  QMap <QCString, MyMoneyAccount>::ConstIterator pos = accountList.find(id);

  // locate the account and if present, return it's data
  if(pos != accountList.end())
    return *pos;

  // throw an exception, if it does not exist
  QString msg = "Unknown account id '" + id + "'";
  throw new MYMONEYEXCEPTION(msg);
}

const bool MyMoneyDatabaseMgr::isStandardAccount(const QCString& id) const
{
  return id == STD_ACC_LIABILITY
      || id == STD_ACC_ASSET
      || id == STD_ACC_EXPENSE
      || id == STD_ACC_INCOME
      || id == STD_ACC_EQUITY;
}

void MyMoneyDatabaseMgr::setAccountName(const QCString& id, const QString& name)
{
  if(!isStandardAccount(id))
    throw new MYMONEYEXCEPTION("Only standard accounts can be modified using setAccountName()");

  startTransaction();
  MyMoneyAccount acc = m_sql->fetchAccounts(QString(id), true) [id];
  acc.setName(name);
  m_sql->modifyAccount(acc);
  commitTransaction();
}

void MyMoneyDatabaseMgr::addInstitution(MyMoneyInstitution& institution)
{
  MyMoneyInstitution newInstitution(nextInstitutionID(), institution);

  // mark file as changed
  m_sql->addInstitution (newInstitution);

  // return new data
  institution = newInstitution;
}

const QCString MyMoneyDatabaseMgr::nextPayeeID(void)
{
  QCString id;
  id.setNum(ulong(m_sql->incrementPayeeId()));
  id = "P" + id.rightJustify(PAYEE_ID_SIZE, '0');
  return id;
}

const QCString MyMoneyDatabaseMgr::nextInstitutionID(void)
{
  QCString id;
  id.setNum(ulong(m_sql->incrementInstitutionId()));
  id = "I" + id.rightJustify(INSTITUTION_ID_SIZE, '0');
  return id;
}

const QCString MyMoneyDatabaseMgr::nextAccountID(void)
{
  QCString id;
  id.setNum(ulong(m_sql->incrementAccountId()));
  id = "A" + id.rightJustify(ACCOUNT_ID_SIZE, '0');
  return id;
}

const QCString MyMoneyDatabaseMgr::nextBudgetID(void)
{
  QCString id;
  id.setNum(ulong(m_sql->incrementBudgetId()));
  id = "B" + id.rightJustify(BUDGET_ID_SIZE, '0');
  return id;
}

const QCString MyMoneyDatabaseMgr::nextReportID(void)
{
  QCString id;
  id.setNum(ulong(m_sql->incrementReportId()));
  id = "R" + id.rightJustify(REPORT_ID_SIZE, '0');
  return id;
}

const QCString MyMoneyDatabaseMgr::nextTransactionID(void)
{
  QCString id;
  id.setNum(ulong(m_sql->incrementTransactionId()));
  id = "T" + id.rightJustify(TRANSACTION_ID_SIZE, '0');
  return id;
}

const QCString MyMoneyDatabaseMgr::nextScheduleID(void)
{
  QCString id;
  id.setNum(ulong(m_sql->incrementScheduleId()));
  id = "SCH" + id.rightJustify(SCHEDULE_ID_SIZE, '0');
  return id;
}

const QCString MyMoneyDatabaseMgr::nextSecurityID(void)
{
  QCString id;
  id.setNum(ulong(m_sql->incrementSecurityId()));
  id = "E" + id.rightJustify(SECURITY_ID_SIZE, '0');
  return id;
}

void MyMoneyDatabaseMgr::addTransaction(MyMoneyTransaction& transaction, const bool skipAccountUpdate)
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
  QCString key = newTransaction.uniqueSortKey();

  m_sql->addTransaction(newTransaction);

  transaction = newTransaction;

  // adjust the balance of all affected accounts
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
    acc.adjustBalance((*it_s));
    if(!skipAccountUpdate) {
      acc.touch();
//FIXME:      invalidateBalanceCache(acc.id());
    }
    m_sql->modifyAccount(acc);
  }
}

const bool MyMoneyDatabaseMgr::hasActiveSplits(const QCString& id) const
{
  QMap<QCString, MyMoneyTransaction>::ConstIterator it;

  MyMoneyTransactionFilter f(id);
  QMap<QCString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions(f);

  for(it = transactionList.begin(); it != transactionList.end(); ++it) {
    if((*it).accountReferenced(id)) {
      return true;
    }
  }
  return false;
}

  /**
    * This method is used to return the actual balance of an account
    * without it's sub-ordinate accounts. If a @p date is presented,
    * the balance at the beginning of this date (not including any
    * transaction on this date) is returned. Otherwise all recorded
    * transactions are included in the balance.
    *
    * @param id id of the account in question
    * @param date return balance for specific date
    * @return balance of the account as MyMoneyMoney object
    */
//const MyMoneyMoney MyMoneyDatabaseMgr::balance(const QCString& id, const QDate& date);

const MyMoneyMoney MyMoneyDatabaseMgr::totalBalance(const QCString& id, const QDate& date) const
{
  QCStringList accounts;
  QCStringList::ConstIterator it_a;

  MyMoneyMoney result; //(balance(id, date));

  accounts = MyMoneyFile::instance()->account(id).accountList();
  for (it_a = accounts.begin(); it_a != accounts.end(); ++it_a) {
    accounts += MyMoneyFile::instance()->account(*it_a).accountList();
  }
  std::list <QCString> tempList (accounts.begin(), accounts.end());
  tempList.sort();;
  tempList.unique();

  accounts = QCStringList(tempList);

  QMap<QCString, MyMoneyMoney> balanceMap = m_sql->fetchBalance(accounts, date);
  for (QMap<QCString, MyMoneyMoney>::ConstIterator it_b = balanceMap.begin(); it_b != balanceMap.end(); ++it_b) {
    result += it_b.data();
  }

  return result;
}

const MyMoneyInstitution MyMoneyDatabaseMgr::institution(const QCString& id) const
{
  QMap<QCString, MyMoneyInstitution>::ConstIterator pos;
  QMap<QCString, MyMoneyInstitution> institutionList = m_sql->fetchInstitutions(QString(id));

  pos = institutionList.find(id);
  if(pos != institutionList.end())
    return *pos;
  throw new MYMONEYEXCEPTION("unknown institution");
}

bool MyMoneyDatabaseMgr::dirty(void) const
{ return false; }

void MyMoneyDatabaseMgr::setDirty(void)
{}

const unsigned int MyMoneyDatabaseMgr::accountCount(void) const
{
  return m_sql->getRecCount("kmmAccounts");
}

const QValueList<MyMoneyInstitution> MyMoneyDatabaseMgr::institutionList(void) const
{ return m_sql->fetchInstitutions().values(); }

void MyMoneyDatabaseMgr::modifyAccount(const MyMoneyAccount& account, const bool skipCheck)
{
  QMap<QCString, MyMoneyAccount>::ConstIterator pos;

  // locate the account in the file global pool
  startTransaction();
  QMap<QCString, MyMoneyAccount> accountList = m_sql->fetchAccounts (QString(account.id()), true);
  pos = accountList.find(account.id());
  if(pos != accountList.end()) {
    // check if the new info is based on the old one.
    // this is the case, when the file and the id
    // as well as the type are equal.
    if((*pos).parentAccountId() == account.parentAccountId()
    && (*pos).accountType() == account.accountType()
    || skipCheck == true) {
      // make sure that all the referenced objects exist
      if(!account.institutionId().isEmpty())
        institution(account.institutionId());

      QValueList<QCString>::ConstIterator it_a;
      for(it_a = account.accountList().begin(); it_a != account.accountList().end(); ++it_a) {
        this->account(*it_a);
      }

      // update information in account list
      //m_accountList.modify(account.id(), account);

      // invalidate cached balance
//FIXME:      invalidateBalanceCache(account.id());

      // mark file as changed
      m_sql->modifyAccount(account);
      commitTransaction();
    } else {
      rollbackTransaction();
      throw new MYMONEYEXCEPTION("Invalid information for update");
    }

  } else {
    rollbackTransaction();
    throw new MYMONEYEXCEPTION("Unknown account id");
  }
}

void MyMoneyDatabaseMgr::modifyInstitution(const MyMoneyInstitution& institution)
{
  QMap<QCString, MyMoneyInstitution> institutionList = m_sql->fetchInstitutions(QString(institution.id()));
  QMap<QCString, MyMoneyInstitution>::ConstIterator pos;

  // locate the institution in the file global pool
  pos = institutionList.find(institution.id());
  if(pos != institutionList.end()) {
    m_sql->modifyInstitution(institution);
  } else
    throw new MYMONEYEXCEPTION("unknown institution");
}

  /**
    * This method is used to update a specific transaction in the
    * transaction pool of the MyMoneyFile object
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to transaction to be changed
    */
void MyMoneyDatabaseMgr::modifyTransaction(const MyMoneyTransaction& transaction)
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
    MyMoneyFile::instance()->account((*it_s).accountId());
    if(!(*it_s).payeeId().isEmpty())
      MyMoneyFile::instance()->payee((*it_s).payeeId());
  }

  // new data seems to be ok. find old version of transaction
  // in our pool. Throw exception if unknown.
//  if(!m_transactionKeys.contains(transaction.id()))
//    throw new MYMONEYEXCEPTION("invalid transaction id");

//  QCString oldKey = m_transactionKeys[transaction.id()];
  QMap <QCString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions("('" + QString(transaction.id()) + "')");
//  if(transactionList.size() != 1)
//    throw new MYMONEYEXCEPTION("invalid transaction key");

  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;

//  it_t = transactionList.find(oldKey);
  it_t = transactionList.begin();
  if(it_t == transactionList.end())
    throw new MYMONEYEXCEPTION("invalid transaction key");

  // mark all accounts referenced in old and new transaction data
  // as modified
  QMap<QCString, MyMoneyAccount> accountList = m_sql->fetchAccounts();
  for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
    MyMoneyAccount acc = accountList[(*it_s).accountId()];
    acc.adjustBalance((*it_s), true);
    acc.touch();
//FIXME:    invalidateBalanceCache(acc.id());
    //m_accountList.modify(acc.id(), acc);
    m_sql->modifyAccount(acc);
    //modifiedAccounts[(*it_s).accountId()] = true;
  }
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    MyMoneyAccount acc = accountList[(*it_s).accountId()];
    acc.adjustBalance((*it_s));
    acc.touch();
//FIXME:    invalidateBalanceCache(acc.id());
    //m_accountList.modify(acc.id(), acc);
    m_sql->modifyAccount(acc);
    //modifiedAccounts[(*it_s).accountId()] = true;
  }

  // remove old transaction from lists
//  m_sql->removeTransaction(oldKey);

  // add new transaction to lists
 // QCString newKey = transaction.uniqueSortKey();
//  m_sql->insertTransaction(newKey, transaction);
  //m_transactionKeys.modify(transaction.id(), newKey);

  // mark file as changed
  m_sql->modifyTransaction(transaction);
}

void MyMoneyDatabaseMgr::reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent)
{
  QStringList accountIdList;
  QMap<QCString, MyMoneyAccount>::ConstIterator oldParent;
  QMap<QCString, MyMoneyAccount>::ConstIterator newParent;
  QMap<QCString, MyMoneyAccount>::ConstIterator childAccount;

  // verify that accounts exist. If one does not,
  // an exception is thrown
  accountIdList << account.id() << parent.id();
  MyMoneyDatabaseMgr::account(account.id());
  MyMoneyDatabaseMgr::account(parent.id());

  if(!account.parentAccountId().isEmpty()) {
    accountIdList << account.parentAccountId();
  }

  if(account.accountType() == MyMoneyAccount::Stock && parent.accountType() != MyMoneyAccount::Investment)
    throw new MYMONEYEXCEPTION("Cannot move a stock acocunt into a non-investment account");

  startTransaction();
  QMap<QCString, MyMoneyAccount> accountList = m_sql->fetchAccounts(accountIdList, true);

  if(!account.parentAccountId().isEmpty()) {
    MyMoneyDatabaseMgr::account(account.parentAccountId());
    oldParent = accountList.find(account.parentAccountId());
  }

  newParent = accountList.find(parent.id());
  childAccount = accountList.find(account.id());

  MyMoneyAccount acc;
  if(!account.parentAccountId().isEmpty()) {
    acc = (*oldParent);
    acc.removeAccountId(account.id());
    m_sql->modifyAccount(acc);
  }

  parent = (*newParent);
  parent.addAccountId(account.id());

  account = (*childAccount);
  account.setParentAccountId(parent.id());

#if 0
  // make sure the type is the same as the new parent. This does not work for stock and investment
  if(account.accountType() != MyMoneyAccount::Stock && account.accountType() != MyMoneyAccount::Investment)
    (*childAccount).setAccountType((*newParent).accountType());
#endif

  m_sql->modifyAccount(parent);
  m_sql->modifyAccount(account);
  commitTransaction();
}

void MyMoneyDatabaseMgr::removeTransaction(const MyMoneyTransaction& transaction)
{
  QMap<QCString, bool> modifiedAccounts;

  // first perform all the checks
  if(transaction.id().isEmpty())
    throw new MYMONEYEXCEPTION("invalid transaction to be deleted");

  QMap<QCString, QCString>::ConstIterator it_k;
  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;

//  it_k = m_transactionKeys.find(transaction.id());
//  if(it_k == m_transactionKeys.end())
//    throw new MYMONEYEXCEPTION("invalid transaction to be deleted");

  QMap <QCString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions("('" + QString(transaction.id()) + "')");
//  it_t = transactionList.find(*it_k);
  it_t = transactionList.begin();
  if(it_t == transactionList.end())
    throw new MYMONEYEXCEPTION("invalid transaction key");

  QValueList<MyMoneySplit>::ConstIterator it_s;

  // scan the splits and collect all accounts that need
  // to be updated after the removal of this transaction
  QMap<QCString, MyMoneyAccount> accountList = m_sql->fetchAccounts();
  for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
    MyMoneyAccount acc = accountList[(*it_s).accountId()];
//    modifiedAccounts[(*it_s).accountId()] = true;
    acc.adjustBalance((*it_s), true);
    acc.touch();
    m_sql->modifyAccount(acc);
//FIXME:    invalidateBalanceCache(acc.id());
  }

  // FIXME: check if any split is frozen and throw exception

  // remove the transaction from the two lists
  //m_transactionList.remove(*it_k);
//  m_transactionKeys.remove(transaction.id());

  // mark file as changed
  m_sql->removeTransaction(transaction);
}

const unsigned int MyMoneyDatabaseMgr::transactionCount(const QCString& account) const
{ return (m_sql->transactionCount(account)); }

const QMap<QCString, unsigned long> MyMoneyDatabaseMgr::transactionCountMap(void) const
{ return (m_sql->transactionCountMap()); }

const QValueList<MyMoneyTransaction> MyMoneyDatabaseMgr::transactionList(MyMoneyTransactionFilter& filter) const
{
  QValueList<MyMoneyTransaction> list;
  transactionList(list, filter);
  return list;
}

void MyMoneyDatabaseMgr::transactionList(QValueList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const
{
  list.clear();

  TRY
  list = m_sql->fetchTransactions(filter).values();
  PASS
}

void MyMoneyDatabaseMgr::transactionList(QValueList<QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const
{
  list.clear();
  MyMoneyMap<QCString, MyMoneyTransaction> transactionList;
  TRY
  transactionList = m_sql->fetchTransactions(filter);
  PASS

  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
  QMap<QCString, MyMoneyTransaction>::ConstIterator txEnd = transactionList.end();

  for(it_t = transactionList.begin(); it_t != txEnd; ++it_t) {
    if(filter.match(*it_t, this)) {
      QValueList<MyMoneySplit>::const_iterator it_s;
      for(it_s = filter.matchingSplits().begin(); it_s != filter.matchingSplits().end(); ++it_s) {
        list.append(qMakePair(*it_t, *it_s));
      }
    }
  }
}

void MyMoneyDatabaseMgr::removeAccount(const MyMoneyAccount& account)
{
  MyMoneyAccount parent;

  // check that the account and it's parent exist
  // this will throw an exception if the id is unknown
  MyMoneyDatabaseMgr::account(account.id());
  parent = MyMoneyDatabaseMgr::account(account.parentAccountId());

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
    MyMoneyDatabaseMgr::account(*it);
  }

  // if one of the accounts did not exist, an exception had been
  // thrown and we would not make it until here.

  QStringList accountIdList;
  accountIdList << parent.id() << account.id();
  startTransaction();
  QMap<QCString, MyMoneyAccount> accountList = m_sql->fetchAccounts(accountIdList, true);

  QMap<QCString, MyMoneyAccount>::ConstIterator it_a;
  QMap<QCString, MyMoneyAccount>::ConstIterator it_p;

  // locate the account in the file global pool

  it_a = accountList.find(account.id());
  if(it_a == accountList.end())
    throw new MYMONEYEXCEPTION("Internal error: account not found in list");

  it_p = accountList.find(parent.id());
  if(it_p == accountList.end())
    throw new MYMONEYEXCEPTION("Internal error: parent account not found in list");

  if(!account.institutionId().isEmpty())
    throw new MYMONEYEXCEPTION("Cannot remove account still attached to an institution");

  // FIXME: check referential integrity for the account to be removed

  // check if the new info is based on the old one.
  // this is the case, when the file and the id
  // as well as the type are equal.
  if((*it_a).id() == account.id()
  && (*it_a).accountType() == account.accountType()) {

    // second round over sub-ordinate accounts: do re-parenting
    // but only if the list contains at least one entry
    // FIXME: move this logic to MyMoneyFile
    if((*it_a).accountList().count() > 0) {
      while((*it_a).accountList().count() > 0) {
        it = (*it_a).accountList().begin();
        MyMoneyAccount acc(MyMoneyDatabaseMgr::account(*it));
        reparentAccount(acc, parent);//, false);
      }
    }
    // remove account from parent's list
    parent.removeAccountId(account.id());
    m_sql->modifyAccount(parent);

    // remove account from the global account pool
    //m_accountList.remove(account.id());

    // remove from balance list
//FIXME:    m_balanceCache.remove(account.id());
//FIXME:    invalidateBalanceCache(parent.id());

    m_sql->removeAccount(account);
  }
  commitTransaction();
}

void MyMoneyDatabaseMgr::removeInstitution(const MyMoneyInstitution& institution)
{
  QMap<QCString, MyMoneyInstitution> institutionList = m_sql->fetchInstitutions(QString(institution.id()));
  QMap<QCString, MyMoneyInstitution>::ConstIterator it_i;

  it_i = institutionList.find(institution.id());
  if(it_i != institutionList.end()) {
    // mark file as changed
    m_sql->removeInstitution(institution);
  } else
    throw new MYMONEYEXCEPTION("invalid institution");
}

const MyMoneyTransaction MyMoneyDatabaseMgr::transaction(const QCString& id) const
{
  // get the full key of this transaction, throw exception
  // if it's invalid (unknown)
  //if(!m_transactionKeys.contains(id))
  //  throw new MYMONEYEXCEPTION("invalid transaction id");

  // check if this key is in the list, throw exception if not
  //QCString key = m_transactionKeys[id];
  QMap <QCString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions("('" + QString(id) + "')");

  //there should only be one transaction in the map, if it was found, so check the size of the map
  //return the first element.
  //if(!transactionList.contains(key))
  if(!transactionList.size())
    throw new MYMONEYEXCEPTION("invalid transaction key");

  return transactionList.begin().data();
}

const MyMoneyMoney MyMoneyDatabaseMgr::balance(const QCString& id, const QDate& date) const
{
  QCStringList idList;
  idList.append(id);
  QMap<QCString,MyMoneyMoney> tempMap = m_sql->fetchBalance(idList, date);

  MyMoneyMoney returnValue = tempMap[id];
  if (returnValue != MyMoneyMoney()) {
    return returnValue;
  }

//DEBUG
  QDate date_ (date);
  //if (date_ == QDate()) date_ = QDate::currentDate();
// END DEBUG

  MyMoneyMoney result(0);
  MyMoneyAccount acc;
  QMap<QCString, MyMoneyAccount> accountList = m_sql->fetchAccounts(/*QString(id)*/);
  //QMap<QCString, MyMoneyAccount>::const_iterator accpos = accountList.find(id);
  if (date_ != QDate()) qDebug ("request balance for %s at %s", id.data(), date_.toString(Qt::ISODate).latin1());
  if(!date_.isValid() && MyMoneyFile::instance()->account(id).accountType() != MyMoneyAccount::Stock) {
    if(accountList.find(id) != accountList.end())
      return accountList[id].balance();
    return MyMoneyMoney(0);
  }
  if(/*m_balanceCache[id].valid == false || date != m_balanceCacheDate) || */ m_sql != 0) {
    QMap<QCString, MyMoneyMoney> balances;
    QMap<QCString, MyMoneyMoney>::ConstIterator it_b;
//FIXME:    if (date != m_balanceCacheDate) {
//FIXME:      m_balanceCache.clear();
//FIXME:      m_balanceCacheDate = date;
//FIXME:    }

    QValueList<MyMoneyTransaction>::ConstIterator it_t;
    QValueList<MyMoneyTransaction>::ConstIterator txEnd;
    QValueList<MyMoneySplit>::ConstIterator it_s;

    MyMoneyTransactionFilter filter;
    filter.addAccount(id);
    filter.setDateFilter(QDate(), date_);
    filter.setReportAllSplits(false);
    QValueList<MyMoneyTransaction> list = transactionList(filter);

    txEnd = list.end();
    for(it_t = list.begin(); it_t != txEnd; ++it_t) {
      for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s){
        const QCString aid = (*it_s).accountId();
        if((*it_s).action() == MyMoneySplit::ActionSplitShares) {
          balances[aid] = balances[aid] * (*it_s).shares();
        } else {
          balances[aid] += (*it_s).value((*it_t).commodity(), accountList[aid].currencyId());
        }
      }
    }

    // fill the found balances into the cache
//FIXME:    for(it_b = balances.begin(); it_b != balances.end(); ++it_b) {
//FIXME:      MyMoneyBalanceCacheItem balance(*it_b);
//FIXME:      m_balanceCache[it_b.key()] = balance;
//FIXME:    }

    // fill all accounts w/o transactions to zero
//    if (m_sql != 0) {
//      QMap<QCString, MyMoneyAccount>::ConstIterator it_a;
//      for(it_a = m_accountList.begin(); it_a != m_accountList.end(); ++it_a) {
//FIXME:        if(m_balanceCache[(*it_a).id()].valid == false) {
//FIXME:          MyMoneyBalanceCacheItem balance(MyMoneyMoney(0,1));
//FIXME:          m_balanceCache[(*it_a).id()] = balance;
//FIXME:        }
//      }
//    }

  result = balances[id];

  }

//FIXME:  if(m_balanceCache[id].valid == true)
//FIXME:    result = m_balanceCache[id].balance;
//FIXME:  else
//FIXME:    qDebug("Cache mishit should never happen at this point");

  return result;
}

const MyMoneyTransaction MyMoneyDatabaseMgr::transaction(const QCString& account, const int idx) const
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
  //MyMoneyAccount acc = m_accountList[account];
  MyMoneyAccount acc = m_sql->fetchAccounts(QString(account)) [account];
  MyMoneyTransactionFilter filter;

  if(acc.accountGroup() == MyMoneyAccount::Income
  || acc.accountGroup() == MyMoneyAccount::Expense)
    filter.addCategory(account);
  else
    filter.addAccount(account);

  transactionList(list, filter);
  if(idx < 0 || idx >= static_cast<int> (list.count()))
    throw new MYMONEYEXCEPTION("Unknown idx for transaction");

  return transaction(list[idx].id());
}

const unsigned int MyMoneyDatabaseMgr::institutionCount(void) const
{
  return m_sql->getRecCount("kmmInstitutions");
}

void MyMoneyDatabaseMgr::accountList(QValueList<MyMoneyAccount>& list) const
{
  QMap <QCString, MyMoneyAccount> accountList;
  if (m_sql) accountList  = m_sql->fetchAccounts();
  QMap<QCString, MyMoneyAccount>::ConstIterator it;
  QMap<QCString, MyMoneyAccount>::ConstIterator accEnd = accountList.end();
  for(it = accountList.begin(); it != accEnd; ++it) {
    if(!isStandardAccount((*it).id())) {
      list.append(*it);
    }
  }
}

const MyMoneyAccount MyMoneyDatabaseMgr::liability(void) const
{ return MyMoneyFile::instance()->account(STD_ACC_LIABILITY); }

const MyMoneyAccount MyMoneyDatabaseMgr::asset(void) const
{ return MyMoneyFile::instance()->account(STD_ACC_ASSET); }

const MyMoneyAccount MyMoneyDatabaseMgr::expense(void) const
{ return MyMoneyFile::instance()->account(STD_ACC_EXPENSE); }

const MyMoneyAccount MyMoneyDatabaseMgr::income(void) const
{ return MyMoneyFile::instance()->account(STD_ACC_INCOME); }

const MyMoneyAccount MyMoneyDatabaseMgr::equity(void) const
{ return MyMoneyFile::instance()->account(STD_ACC_EQUITY); }

void MyMoneyDatabaseMgr::addSecurity(MyMoneySecurity& security)
{
  // create the account
  MyMoneySecurity newSecurity(nextSecurityID(), security);

  m_sql->addSecurity(newSecurity);
  security = newSecurity;
}

void MyMoneyDatabaseMgr::modifySecurity(const MyMoneySecurity& security)
{
  QMap<QCString, MyMoneySecurity> securitiesList = m_sql->fetchSecurities(QString(security.id()), true);
  QMap<QCString, MyMoneySecurity>::ConstIterator it;

  it = securitiesList.find(security.id());
  if(it == securitiesList.end())
  {
    QString msg = "Unknown security  '";
    msg += security.id() + "' during modifySecurity()";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_sql->modifySecurity(security);
}

void MyMoneyDatabaseMgr::removeSecurity(const MyMoneySecurity& security)
{
  QMap<QCString, MyMoneySecurity> securitiesList = m_sql->fetchSecurities(QString(security.id()));
  QMap<QCString, MyMoneySecurity>::ConstIterator it;

  // FIXME: check referential integrity

  it = securitiesList.find(security.id());
  if(it == securitiesList.end())
  {
    QString msg = "Unknown security  '";
    msg += security.id() + "' during removeSecurity()";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_sql->removeSecurity(security);
}

const MyMoneySecurity MyMoneyDatabaseMgr::security(const QCString& id) const
{
  QMap<QCString, MyMoneySecurity> securitiesList = m_sql->fetchSecurities(QString(id));
  QMap<QCString, MyMoneySecurity>::ConstIterator it = securitiesList.find(id);
  if(it != securitiesList.end())
  {
    return it.data();
  }

  return MyMoneySecurity();
}

const QValueList<MyMoneySecurity> MyMoneyDatabaseMgr::securityList(void) const
{ return m_sql->fetchSecurities().values(); }

void MyMoneyDatabaseMgr::addPrice(const MyMoneyPrice& price)
{
  MyMoneyPriceEntries::ConstIterator it;
  MyMoneyPriceList priceList = m_sql->fetchPrices();
  it = priceList[MyMoneySecurityPair(price.from(), price.to())].find(price.date());
  // do not replace, if the information did not change.
  if(it != priceList[MyMoneySecurityPair(price.from(), price.to())].end()) {
    if((*it).rate((*it).to()) == price.rate(price.to())
    && (*it).source() == price.source())
      return;
  }

  m_sql->addPrice(price);
}

void MyMoneyDatabaseMgr::removePrice(const MyMoneyPrice& price)
{
  m_sql->removePrice(price);
}

const MyMoneyPrice MyMoneyDatabaseMgr::price(const QCString& fromId, const QCString& toId, const QDate& _date, const bool exactDate) const
{
  return m_sql->fetchSinglePrice(fromId, toId, _date, exactDate);
}

const MyMoneyPriceList MyMoneyDatabaseMgr::priceList(void) const
{ return m_sql->fetchPrices(); }

void MyMoneyDatabaseMgr::addSchedule(MyMoneySchedule& sched)
{
  // first perform all the checks
  if(!sched.id().isEmpty())
    throw new MYMONEYEXCEPTION("schedule already contains an id");

  // The following will throw an exception when it fails
  sched.validate(false);

  sched = MyMoneySchedule (nextScheduleID(), sched);

  m_sql->addSchedule(sched);
}

void MyMoneyDatabaseMgr::modifySchedule(const MyMoneySchedule& sched)
{
  QMap<QCString, MyMoneySchedule> scheduleList = m_sql->fetchSchedules(QString(sched.id()));
  QMap<QCString, MyMoneySchedule>::ConstIterator it;

  it = scheduleList.find(sched.id());
  if(it == scheduleList.end()) {
    QString msg = "Unknown schedule '" + sched.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_sql->modifySchedule(sched);
}

void MyMoneyDatabaseMgr::removeSchedule(const MyMoneySchedule& sched)
{
  QMap<QCString, MyMoneySchedule> scheduleList = m_sql->fetchSchedules(QString(sched.id()));
  QMap<QCString, MyMoneySchedule>::ConstIterator it;

  it = scheduleList.find(sched.id());
  if(it == scheduleList.end()) {
    QString msg = "Unknown schedule '" + sched.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  // FIXME: check referential integrity for loan accounts

  m_sql->removeSchedule(sched);
}

const MyMoneySchedule MyMoneyDatabaseMgr::schedule(const QCString& id) const
{
  QMap<QCString, MyMoneySchedule> scheduleList = m_sql->fetchSchedules(QString(id));
  QMap<QCString, MyMoneySchedule>::ConstIterator pos;

  // locate the schedule and if present, return it's data
  pos = scheduleList.find(id);
  if(pos != scheduleList.end())
    return (*pos);

  // throw an exception, if it does not exist
  QString msg = "Unknown schedule id '" + id + "'";
  throw new MYMONEYEXCEPTION(msg);
}

const QValueList<MyMoneySchedule> MyMoneyDatabaseMgr::scheduleList(const QCString& accountId,
                                     const MyMoneySchedule::typeE type,
                                     const MyMoneySchedule::occurenceE occurence,
                                     const MyMoneySchedule::paymentTypeE paymentType,
                                     const QDate& startDate,
                                     const QDate& endDate,
                                     const bool overdue) const
{
  QMap<QCString, MyMoneySchedule> scheduleList = m_sql->fetchSchedules();
  QMap<QCString, MyMoneySchedule>::ConstIterator pos;
  QValueList<MyMoneySchedule> list;

  // qDebug("scheduleList()");

  for(pos = scheduleList.begin(); pos != scheduleList.end(); ++pos) {
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

const QValueList<MyMoneySchedule> MyMoneyDatabaseMgr::scheduleListEx( int scheduleTypes,
                                              int scheduleOcurrences,
                                              int schedulePaymentTypes,
                                              QDate startDate,
                                              const QCStringList& accounts) const
{
//  qDebug("scheduleListEx");
  QMap<QCString, MyMoneySchedule> scheduleList = m_sql->fetchSchedules();
  QMap<QCString, MyMoneySchedule>::ConstIterator pos;
  QValueList<MyMoneySchedule> list;

  if (!startDate.isValid())
    return list;

  for(pos = scheduleList.begin(); pos != scheduleList.end(); ++pos)
  {
    if (scheduleTypes && !(scheduleTypes & (*pos).type()))
      continue;

    if (scheduleOcurrences && !(scheduleOcurrences & (*pos).occurence()))
      continue;

    if (schedulePaymentTypes && !(schedulePaymentTypes & (*pos).paymentType()))
      continue;

    if((*pos).paymentDates(startDate, startDate).count() == 0)
      continue;

    if ((*pos).isFinished())
      continue;

    if ((*pos).hasRecordedPayment(startDate))
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

void MyMoneyDatabaseMgr::addCurrency(const MyMoneySecurity& currency)
{
  QMap<QCString, MyMoneySecurity> currencyList = m_sql->fetchCurrencies(QString(currency.id()));
  QMap<QCString, MyMoneySecurity>::ConstIterator it;

  it = currencyList.find(currency.id());
  if(it != currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot add currency with existing id %1").arg(currency.id().data()));
  }

  m_sql->addCurrency(currency);
}

void MyMoneyDatabaseMgr::modifyCurrency(const MyMoneySecurity& currency)
{
  QMap<QCString, MyMoneySecurity> currencyList = m_sql->fetchCurrencies(QString(currency.id()));
  QMap<QCString, MyMoneySecurity>::ConstIterator it;

  it = currencyList.find(currency.id());
  if(it == currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot modify currency with unknown id %1").arg(currency.id().data()));
  }

  m_sql->modifyCurrency(currency);
}

void MyMoneyDatabaseMgr::removeCurrency(const MyMoneySecurity& currency)
{
  QMap<QCString, MyMoneySecurity> currencyList = m_sql->fetchCurrencies(QString(currency.id()));
  QMap<QCString, MyMoneySecurity>::ConstIterator it;

  // FIXME: check referential integrity

  it = currencyList.find(currency.id());
  if(it == currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot remove currency with unknown id %1").arg(currency.id().data()));
  }

  m_sql->removeCurrency(currency);
}

const MyMoneySecurity MyMoneyDatabaseMgr::currency(const QCString& id) const
{
  if(id.isEmpty()) {

  }
  QMap<QCString, MyMoneySecurity> currencyList = m_sql->fetchCurrencies(QString(id));
  QMap<QCString, MyMoneySecurity>::ConstIterator it;

  it = currencyList.find(id);
  if(it == currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot retrieve currency with unknown id '%1'").arg(id.data()));
  }

  return *it;
}

const QValueList<MyMoneySecurity> MyMoneyDatabaseMgr::currencyList(void) const
{
  return m_sql->fetchCurrencies().values();
}

const QValueList<MyMoneyReport> MyMoneyDatabaseMgr::reportList( void ) const
{
  return m_sql->fetchReports().values();
}

void MyMoneyDatabaseMgr::addReport( MyMoneyReport& report )
{
  if(!report.id().isEmpty())
    throw new MYMONEYEXCEPTION("transaction already contains an id");

  m_sql->addReport(MyMoneyReport (nextReportID(), report));
}

void MyMoneyDatabaseMgr::modifyReport( const MyMoneyReport& report )
{
  QMap<QCString, MyMoneyReport> reportList = m_sql->fetchReports(QString(report.id()));
  QMap<QCString, MyMoneyReport>::ConstIterator it;

  it = reportList.find(report.id());
  if(it == reportList.end()) {
    QString msg = "Unknown report '" + report.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_sql->modifyReport(report);
}

unsigned MyMoneyDatabaseMgr::countReports( void ) const
{
  return m_sql->getRecCount("kmmReports");
}

const MyMoneyReport MyMoneyDatabaseMgr::report( const QCString& id ) const
{
  return m_sql->fetchReports(QString(id))[id];
}

void MyMoneyDatabaseMgr::removeReport(const MyMoneyReport& report)
{
  QMap<QCString, MyMoneyReport> reportList = m_sql->fetchReports(QString(report.id()));
  QMap<QCString, MyMoneyReport>::ConstIterator it;

  it = reportList.find(report.id());
  if(it == reportList.end()) {
    QString msg = "Unknown report '" + report.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_sql->removeReport(report);
}

const QValueList<MyMoneyBudget> MyMoneyDatabaseMgr::budgetList( void ) const
{
  return m_sql->fetchBudgets().values();
}

void MyMoneyDatabaseMgr::addBudget( MyMoneyBudget& budget )
{
  MyMoneyBudget newBudget(nextBudgetID(), budget);
  m_sql->addBudget(newBudget);
}

const MyMoneyBudget MyMoneyDatabaseMgr::budgetByName(const QString& budget) const
{
  QMap<QCString, MyMoneyBudget> budgets = m_sql->fetchBudgets();
  QMap<QCString, MyMoneyBudget>::ConstIterator it_p;

  for(it_p = budgets.begin(); it_p != budgets.end(); ++it_p) {
    if((*it_p).name() == budget) {
      return *it_p;
    }
  }

  throw new MYMONEYEXCEPTION("Unknown budget '" + budget + "'");
}

void MyMoneyDatabaseMgr::modifyBudget( const MyMoneyBudget& budget )
{
  //QMap<QCString, MyMoneyBudget>::ConstIterator it;

  //it = m_budgetList.find(budget.id());
  //if(it == m_budgetList.end()) {
  //  QString msg = "Unknown budget '" + budget.id() + "'";
  //  throw new MYMONEYEXCEPTION(msg);
  //}
  //m_budgetList.modify(budget.id(), budget);

  startTransaction();
  if (m_sql->fetchBudgets(QString(budget.id()), true).empty()) {
    QString msg = "Unknown budget '" + budget.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }
  m_sql->modifyBudget(budget);
  commitTransaction();
}

unsigned MyMoneyDatabaseMgr::countBudgets( void ) const
{
  return m_sql->getRecCount("kmmBudgetConfig");
}

MyMoneyBudget MyMoneyDatabaseMgr::budget( const QCString& id ) const
{
  return m_sql->fetchBudgets(QString(id)) [id];
}

void MyMoneyDatabaseMgr::removeBudget(const MyMoneyBudget& budget)
{
//  QMap<QCString, MyMoneyBudget>::ConstIterator it;
//
//  it = m_budgetList.find(budget.id());
//  if(it == m_budgetList.end()) {
//    QString msg = "Unknown budget '" + budget.id() + "'";
//    throw new MYMONEYEXCEPTION(msg);
//  }
//
  m_sql->removeBudget(budget);
}

void MyMoneyDatabaseMgr::clearCache(void)
{
  //m_balanceCache.clear();
}

class isReferencedHelper {
  public:
    isReferencedHelper(const QCString& id)
      : m_id (id)
    {}

    inline bool operator() (const MyMoneyObject& obj) const
    { return obj.hasReferenceTo(m_id); }

  private:
    QCString m_id;
};

bool MyMoneyDatabaseMgr::isReferenced(const MyMoneyObject& obj, const MyMoneyFileBitArray& skipCheck) const
{
  bool rc = false;
  const QCString& id = obj.id();

  MyMoneyPriceList::const_iterator it_pr;

  MyMoneyPriceList::const_iterator priceEnd;

  // FIXME optimize the list of objects we have to checks
  //       with a bit of knowledge of the internal structure, we
  //       could optimize the number of objects we check for references

  // Scan all engine objects for a reference
  if(!skipCheck[RefCheckTransaction]) {
    bool skipTransactions = false;
    MyMoneyTransactionFilter f;
    if (typeid(obj) == typeid(MyMoneyAccount)) {
      f.addAccount(obj.id());
    } else if (typeid(obj) == typeid(MyMoneyCategory)) {
      f.addCategory(obj.id());
    } else if (typeid(obj) == typeid(MyMoneyPayee)) {
      f.addPayee(obj.id());
    } // if it's anything else, I guess we just read everything
    //FIXME: correction, transactions can only have a reference to an account or payee,
    //             so, read nothing.
    else {
      skipTransactions = true;
    }
    if (! skipTransactions) {
      //QMap <QCString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions(f);
      //rc = (transactionList.end() != std::find_if(transactionList.begin(), transactionList.end(),  isReferencedHelper(id)));
      //if (rc != m_sql->isReferencedByTransaction(obj.id()))
      //  qDebug ("Transaction match inconsistency.");
      rc = m_sql->isReferencedByTransaction(obj.id());
    }
  }

  if(!skipCheck[RefCheckAccount] && !rc) {
    QValueList<MyMoneyAccount> accountList;
    MyMoneyFile::instance()->accountList(accountList);
    rc = (accountList.end() != std::find_if(accountList.begin(), accountList.end(),  isReferencedHelper(id)));
  }
  if(!skipCheck[RefCheckInstitution] && !rc) {
    QValueList<MyMoneyInstitution> institutionList;
    MyMoneyFile::instance()->institutionList(institutionList);
    rc = (institutionList.end() != std::find_if(institutionList.begin(), institutionList.end(),  isReferencedHelper(id)));
  }
  if(!skipCheck[RefCheckPayee] && !rc) {
    QValueList<MyMoneyPayee> payeeList = MyMoneyFile::instance()->payeeList();
    rc = (payeeList.end() != std::find_if(payeeList.begin(), payeeList.end(),  isReferencedHelper(id)));
  }
  if(!skipCheck[RefCheckReport] && !rc) {
    QMap<QCString, MyMoneyReport> reportList = m_sql->fetchReports();
    rc = (reportList.end() != std::find_if(reportList.begin(), reportList.end(),  isReferencedHelper(id)));
  }
  if(!skipCheck[RefCheckBudget] && !rc) {
    QMap<QCString, MyMoneyBudget> budgets = m_sql->fetchBudgets();
    rc = (budgets.end() != std::find_if(budgets.begin(), budgets.end(),  isReferencedHelper(id)));
  }
  if(!skipCheck[RefCheckSchedule] && !rc) {
    QMap<QCString, MyMoneySchedule> scheduleList = m_sql->fetchSchedules();
    rc = (scheduleList.end() != std::find_if(scheduleList.begin(), scheduleList.end(),  isReferencedHelper(id)));
  }
  if(!skipCheck[RefCheckSecurity] && !rc) {
    QValueList<MyMoneySecurity> securitiesList = MyMoneyFile::instance()->securityList();
    rc = (securitiesList.end() != std::find_if(securitiesList.begin(), securitiesList.end(),  isReferencedHelper(id)));
  }
  if(!skipCheck[RefCheckCurrency] && !rc) {
    QValueList<MyMoneySecurity> currencyList = m_sql->fetchCurrencies().values();
    rc = (currencyList.end() != std::find_if(currencyList.begin(), currencyList.end(),  isReferencedHelper(id)));
  }
  // within the pricelist we don't have to scan each entry. Checking the QPair
  // members of the MyMoneySecurityPair is enough as they are identical to the
  // two security ids
  if(!skipCheck[RefCheckPrice] && !rc) {
    MyMoneyPriceList priceList = m_sql->fetchPrices();
    priceEnd = priceList.end();
    for(it_pr = priceList.begin(); !rc && it_pr != priceEnd; ++it_pr) {
      rc = (it_pr.key().first == id) || (it_pr.key().second == id);
    }
  }
  return rc;
}

void MyMoneyDatabaseMgr::close(void)
{ if (m_sql != 0) m_sql->close(true); }

void MyMoneyDatabaseMgr::startTransaction(void)
{ m_sql->startCommitUnit ("databasetransaction"); }

void MyMoneyDatabaseMgr::commitTransaction(void)
{ m_sql->endCommitUnit ("databasetransaction"); }

void MyMoneyDatabaseMgr::rollbackTransaction(void)
{ m_sql->cancelCommitUnit ("databasetransaction"); }

void MyMoneyDatabaseMgr::setCreationDate(const QDate& val) 
{ m_creationDate = val; }

MyMoneyStorageSql *MyMoneyDatabaseMgr::connectToDatabase(const KURL& url) {
  m_sql = new MyMoneyStorageSql (this, url);
  return m_sql;
}

 void MyMoneyDatabaseMgr::fillStorage()
{ m_sql->fillStorage(); }

void MyMoneyDatabaseMgr::setLastModificationDate(const QDate& val)
{ m_lastModificationDate = val; }

bool MyMoneyDatabaseMgr::isDuplicateTransaction(const QCString& /*id*/) const 
{ 
  //FIXME: figure out the real id from the key and check the DB.
//return m_transactionKeys.contains(id); 
  return false;
}

void MyMoneyDatabaseMgr::loadAccounts(const QMap<QCString, MyMoneyAccount>& /*map*/)
{
//  m_accountList = map;
//FIXME: update the database.
// startTransaction
// DELETE FROM kmmAccounts
// for each account in the map
//    m_sql->addAccount(...)
// commitTransaction
// on error, rollbackTransaction
}

void MyMoneyDatabaseMgr::loadTransactions(const QMap<QCString, MyMoneyTransaction>& /*map*/)
{
//  m_transactionList = map;
//FIXME: update the database.

//  // now fill the key map
//  QMap<QCString, QCString> keys;
//  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
//  for(it_t = map.begin(); it_t != map.end(); ++it_t) {
//    keys[(*it_t).id()] = it_t.key();
//  }
//  m_transactionKeys = keys;
}

void MyMoneyDatabaseMgr::loadInstitutions(const QMap<QCString, MyMoneyInstitution>& /*map*/)
{
//  m_institutionList = map;
//FIXME: update the database.

//  // now fill the key map
//  QMap<QCString, QCString> keys;
//  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
//  for(it_t = map.begin(); it_t != map.end(); ++it_t) {
//    keys[(*it_t).id()] = it_t.key();
//  }
//  m_transactionKeys = keys;
}

void MyMoneyDatabaseMgr::loadPayees(const QMap<QCString, MyMoneyPayee>& /*map*/)
{
//  m_payeeList = map;
}

void MyMoneyDatabaseMgr::loadSchedules(const QMap<QCString, MyMoneySchedule>& /*map*/)
{
//  m_scheduleList = map;
}

void MyMoneyDatabaseMgr::loadSecurities(const QMap<QCString, MyMoneySecurity>& /*map*/)
{
//  m_securitiesList = map;
}

void MyMoneyDatabaseMgr::loadCurrencies(const QMap<QCString, MyMoneySecurity>& /*map*/)
{
//  m_currencyList = map;
//FIXME: update the database.
// startTransaction
// DELETE FROM kmmBudgetConfig
// for each budget in the map
//    m_sql->addBudget(...)
// commitTransaction
// on error, rollbackTransaction
}

void MyMoneyDatabaseMgr::loadReports( const QMap<QCString, MyMoneyReport>& /*reports*/ )
{
//  m_reportList = reports;
//FIXME: update the database.
// startTransaction
// DELETE FROM kmmBudgetConfig
// for each budget in the map
//    m_sql->addBudget(...)
// commitTransaction
// on error, rollbackTransaction
}

void MyMoneyDatabaseMgr::loadBudgets( const QMap<QCString, MyMoneyBudget>& /*budgets*/ )
{
//  m_budgetList = budgets;
//FIXME: update the database.
// startTransaction
// DELETE FROM kmmBudgetConfig
// for each budget in the map
//    m_sql->addBudget(...)
// commitTransaction
// on error, rollbackTransaction
}

const unsigned long MyMoneyDatabaseMgr::accountId(void) const
{ return m_sql->getNextAccountId(); }

const unsigned long MyMoneyDatabaseMgr::transactionId(void) const
{ return m_sql->getNextTransactionId(); }

const unsigned long MyMoneyDatabaseMgr::payeeId(void) const
{ return m_sql->getNextPayeeId(); }

const unsigned long MyMoneyDatabaseMgr::institutionId(void) const
{ return m_sql->getNextInstitutionId(); }

const unsigned long MyMoneyDatabaseMgr::scheduleId(void) const
{ return m_sql->getNextScheduleId(); }

const unsigned long MyMoneyDatabaseMgr::securityId(void) const
{ return m_sql->getNextSecurityId(); }

const unsigned long MyMoneyDatabaseMgr::reportId(void) const
{ return m_sql->getNextReportId(); }

const unsigned long MyMoneyDatabaseMgr::budgetId(void) const
{ return m_sql->getNextBudgetId(); }

void MyMoneyDatabaseMgr::loadAccountId(const unsigned long /*id*/)
{
  //m_nextAccountID = id;
}

void MyMoneyDatabaseMgr::loadTransactionId(const unsigned long /*id*/)
{
//  m_nextTransactionID = id;
}

void MyMoneyDatabaseMgr::loadPayeeId(const unsigned long /*id*/)
{
//  m_nextPayeeID = id;
}

void MyMoneyDatabaseMgr::loadInstitutionId(const unsigned long /*id*/)
{
//  m_nextInstitutionID = id;
}

void MyMoneyDatabaseMgr::loadScheduleId(const unsigned long /*id*/)
{
//  m_nextScheduleID = id;
}

void MyMoneyDatabaseMgr::loadSecurityId(const unsigned long /*id*/)
{
//  m_nextSecurityID = id;
}

void MyMoneyDatabaseMgr::loadReportId(const unsigned long /*id*/)
{
//  m_nextReportID = id;
}

void MyMoneyDatabaseMgr::loadBudgetId(const unsigned long /*id*/)
{
  //m_nextBudgetID = id;
}

void MyMoneyDatabaseMgr::rebuildAccountBalances(void)
{
  startTransaction();
  QMap<QCString, MyMoneyAccount> accountMap = m_sql->fetchAccounts(QStringList(), true);

  QMap<QCString, MyMoneyMoney> balanceMap = m_sql->fetchBalance(accountMap.keys(), QDate());

  for (QMap<QCString, MyMoneyMoney>::const_iterator it_b = balanceMap.begin();
         it_b != balanceMap.end(); ++it_b) {
    accountMap[it_b.key()].setBalance(it_b.data());
  }

  for (QMap<QCString, MyMoneyAccount>::const_iterator it_a = accountMap.begin();
         it_a != accountMap.end(); ++it_a) {
    m_sql->modifyAccount(it_a.data());
  }
  commitTransaction();
}

#undef TRY
#undef CATCH
#undef PASS