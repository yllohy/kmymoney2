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
#include <typeinfo>
#include <algorithm>

#include "mymoneydatabasemgr.h"
#include "../mymoneytransactionfilter.h"
#include "../mymoneycategory.h"

#define TRY try {
#define CATCH } catch (MyMoneyException *e) {
#define PASS } catch (MyMoneyException *e) { throw; }

MyMoneyDatabaseMgr::MyMoneyDatabaseMgr() :
m_creationDate (QDate::currentDate ()),
m_lastModificationDate (QDate::currentDate ()),
m_sql (0)
{ }

MyMoneyDatabaseMgr::~MyMoneyDatabaseMgr()
{ }

  // general get functions
const MyMoneyPayee MyMoneyDatabaseMgr::user(void) const
{ return m_user; }

const QDate MyMoneyDatabaseMgr::creationDate(void) const
{ return m_creationDate; }

const QDate MyMoneyDatabaseMgr::lastModificationDate(void) const
{ return m_lastModificationDate; }

unsigned int MyMoneyDatabaseMgr::currentFixVersion(void) const
{ return CURRENT_FIX_VERSION; }

unsigned int MyMoneyDatabaseMgr::fileFixVersion(void) const
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
const QString MyMoneyDatabaseMgr::value(const QString& key) const
{
  return MyMoneyKeyValueContainer::value(key);
}

void MyMoneyDatabaseMgr::setValue(const QString& key, const QString& val)
{
  MyMoneyKeyValueContainer::setValue(key, val);
}

void MyMoneyDatabaseMgr::deletePair(const QString& key)
{
  MyMoneyKeyValueContainer::deletePair(key);
}

const QMap<QString, QString> MyMoneyDatabaseMgr::pairs(void) const
{
  return MyMoneyKeyValueContainer::pairs();
}

void MyMoneyDatabaseMgr::setPairs(const QMap<QString, QString>& list)
{
  MyMoneyKeyValueContainer::setPairs(list);
}

MyMoneyDatabaseMgr const * MyMoneyDatabaseMgr::duplicate(void)
{
  MyMoneyDatabaseMgr* that = new MyMoneyDatabaseMgr();
  *that = *this;
  return that;
}

void MyMoneyDatabaseMgr::addAccount(MyMoneyAccount& account)
{
  if (m_sql) {
    // create the account.
    MyMoneyAccount newAccount(nextAccountID(), account);

    m_sql->addAccount(newAccount);
    account = newAccount;
  }
}

void MyMoneyDatabaseMgr::addAccount(MyMoneyAccount& parent, MyMoneyAccount& account)
{
  QMap<QString, MyMoneyAccount> accountList;
  QStringList accountIdList;
  QMap<QString, MyMoneyAccount>::ConstIterator theParent;
  QMap<QString, MyMoneyAccount>::ConstIterator theChild;

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
  if (m_sql) {
    // create the payee
    MyMoneyPayee newPayee(nextPayeeID(), payee);

    m_sql->addPayee(newPayee);
    payee = newPayee;
  }
}

const MyMoneyPayee MyMoneyDatabaseMgr::payee(const QString& id) const
{
  QMap<QString, MyMoneyPayee>::ConstIterator it;
  QMap<QString, MyMoneyPayee> payeeList = m_sql->fetchPayees(QString(id));
  it = payeeList.find(id);
  if(it == payeeList.end())
    throw new MYMONEYEXCEPTION("Unknown payee '" + id + "'");

  return *it;
}

const MyMoneyPayee MyMoneyDatabaseMgr::payeeByName(const QString& payee) const
{
  if(payee.isEmpty())
    return MyMoneyPayee::null;

  QMap<QString, MyMoneyPayee> payeeList;

  TRY
  payeeList = m_sql->fetchPayees();
  PASS

  QMap<QString, MyMoneyPayee>::ConstIterator it_p;

  for(it_p = payeeList.begin(); it_p != payeeList.end(); ++it_p) {
    if((*it_p).name() == payee) {
      return *it_p;
    }
  }

  throw new MYMONEYEXCEPTION("Unknown payee '" + payee + "'");
}

void MyMoneyDatabaseMgr::modifyPayee(const MyMoneyPayee& payee)
{
  QMap<QString, MyMoneyPayee> payeeList = m_sql->fetchPayees(QString(payee.id()), true);
  QMap<QString, MyMoneyPayee>::ConstIterator it;

  it = payeeList.find(payee.id());
  if(it == payeeList.end()) {
    QString msg = "Unknown payee '" + payee.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_sql->modifyPayee(payee);
}

void MyMoneyDatabaseMgr::removePayee(const MyMoneyPayee& payee)
{
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  QValueList<MyMoneySplit>::ConstIterator it_s;
  QMap<QString, MyMoneyPayee> payeeList = m_sql->fetchPayees(QString(payee.id()));
  QMap<QString, MyMoneyPayee>::ConstIterator it_p;

  it_p = payeeList.find(payee.id());
  if(it_p == payeeList.end()) {
    QString msg = "Unknown payee '" + payee.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  // scan all transactions to check if the payee is still referenced
  MyMoneyTransactionFilter f;
  f.addPayee(payee.id());

  QMap<QString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions(f); // make sure they're all here

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
  if (m_sql)
    return m_sql->fetchPayees().values();
  else
    return QValueList<MyMoneyPayee> ();
}

const MyMoneyAccount MyMoneyDatabaseMgr::account(const QString& id) const
{
  if (m_sql)
  {
    QMap <QString, MyMoneyAccount> accountList = m_sql->fetchAccounts(QString(id));
    QMap <QString, MyMoneyAccount>::ConstIterator pos = accountList.find(id);

    // locate the account and if present, return it's data
    if(pos != accountList.end())
      return *pos;
  }

  // throw an exception, if it does not exist
  QString msg = "Unknown account id '" + id + "'";
  throw new MYMONEYEXCEPTION(msg);
}

bool MyMoneyDatabaseMgr::isStandardAccount(const QString& id) const
{
  return id == STD_ACC_LIABILITY
      || id == STD_ACC_ASSET
      || id == STD_ACC_EXPENSE
      || id == STD_ACC_INCOME
      || id == STD_ACC_EQUITY;
}

void MyMoneyDatabaseMgr::setAccountName(const QString& id, const QString& name)
{
  if(!isStandardAccount(id))
    throw new MYMONEYEXCEPTION("Only standard accounts can be modified using setAccountName()");

  if (m_sql) {
    startTransaction();
    MyMoneyAccount acc = m_sql->fetchAccounts(QString(id), true) [id];
    acc.setName(name);
    m_sql->modifyAccount(acc);
    commitTransaction();
  }
}

void MyMoneyDatabaseMgr::addInstitution(MyMoneyInstitution& institution)
{
  if (m_sql) {
    MyMoneyInstitution newInstitution(nextInstitutionID(), institution);

    // mark file as changed
    m_sql->addInstitution (newInstitution);

    // return new data
    institution = newInstitution;
  }
}

const QString MyMoneyDatabaseMgr::nextPayeeID(void)
{
  QString id;
  if (m_sql) {
    id.setNum(ulong(m_sql->incrementPayeeId()));
    id = "P" + id.rightJustify(PAYEE_ID_SIZE, '0');
  }
  return id;
}

const QString MyMoneyDatabaseMgr::nextInstitutionID(void)
{
  QString id;
  if (m_sql) {
    id.setNum(ulong(m_sql->incrementInstitutionId()));
    id = "I" + id.rightJustify(INSTITUTION_ID_SIZE, '0');
  }
  return id;
}

const QString MyMoneyDatabaseMgr::nextAccountID(void)
{
  QString id;
  if (m_sql) {
    id.setNum(ulong(m_sql->incrementAccountId()));
    id = "A" + id.rightJustify(ACCOUNT_ID_SIZE, '0');
  }
  return id;
}

const QString MyMoneyDatabaseMgr::nextBudgetID(void)
{
  QString id;
  if (m_sql) {
    id.setNum(ulong(m_sql->incrementBudgetId()));
    id = "B" + id.rightJustify(BUDGET_ID_SIZE, '0');
  }
  return id;
}

const QString MyMoneyDatabaseMgr::nextReportID(void)
{
  QString id;
  if (m_sql) {
    id.setNum(ulong(m_sql->incrementReportId()));
    id = "R" + id.rightJustify(REPORT_ID_SIZE, '0');
  }
  return id;
}

const QString MyMoneyDatabaseMgr::nextTransactionID(void)
{
  QString id;
  if (m_sql) {
    id.setNum(ulong(m_sql->incrementTransactionId()));
    id = "T" + id.rightJustify(TRANSACTION_ID_SIZE, '0');
  }
  return id;
}

const QString MyMoneyDatabaseMgr::nextScheduleID(void)
{
  QString id;
  if (m_sql) {
    id.setNum(ulong(m_sql->incrementScheduleId()));
    id = "SCH" + id.rightJustify(SCHEDULE_ID_SIZE, '0');
  }
  return id;
}

const QString MyMoneyDatabaseMgr::nextSecurityID(void)
{
  QString id;
  if (m_sql) {
    id.setNum(ulong(m_sql->incrementSecurityId()));
    id = "E" + id.rightJustify(SECURITY_ID_SIZE, '0');
  }
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
  QString key = newTransaction.uniqueSortKey();

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

bool MyMoneyDatabaseMgr::hasActiveSplits(const QString& id) const
{
  QMap<QString, MyMoneyTransaction>::ConstIterator it;

  MyMoneyTransactionFilter f(id);
  QMap<QString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions(f);

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
//const MyMoneyMoney MyMoneyDatabaseMgr::balance(const QString& id, const QDate& date);

const MyMoneyMoney MyMoneyDatabaseMgr::totalBalance(const QString& id, const QDate& date) const
{
  QStringList accounts;
  QStringList::ConstIterator it_a;

  MyMoneyMoney result; //(balance(id, date));

  accounts = MyMoneyFile::instance()->account(id).accountList();
  for (it_a = accounts.begin(); it_a != accounts.end(); ++it_a) {
    accounts += MyMoneyFile::instance()->account(*it_a).accountList();
  }
  std::list <QString> tempList (accounts.begin(), accounts.end());
  tempList.sort();;
  tempList.unique();

  accounts = QStringList(tempList);

  QMap<QString, MyMoneyMoney> balanceMap = m_sql->fetchBalance(accounts, date);
  for (QMap<QString, MyMoneyMoney>::ConstIterator it_b = balanceMap.begin(); it_b != balanceMap.end(); ++it_b) {
    result += it_b.data();
  }

  return result;
}

const MyMoneyInstitution MyMoneyDatabaseMgr::institution(const QString& id) const
{
  QMap<QString, MyMoneyInstitution>::ConstIterator pos;
  QMap<QString, MyMoneyInstitution> institutionList = m_sql->fetchInstitutions(QString(id));

  pos = institutionList.find(id);
  if(pos != institutionList.end())
    return *pos;
  throw new MYMONEYEXCEPTION("unknown institution");
}

bool MyMoneyDatabaseMgr::dirty(void) const
{ return false; }

void MyMoneyDatabaseMgr::setDirty(void)
{}

unsigned int MyMoneyDatabaseMgr::accountCount(void) const
{
  return m_sql->getRecCount("kmmAccounts");
}

const QValueList<MyMoneyInstitution> MyMoneyDatabaseMgr::institutionList(void) const
{
  if (m_sql) {
    return m_sql->fetchInstitutions().values();
  } else {
    return QValueList<MyMoneyInstitution> ();
  }
}

void MyMoneyDatabaseMgr::modifyAccount(const MyMoneyAccount& account, const bool skipCheck)
{
  QMap<QString, MyMoneyAccount>::ConstIterator pos;

  // locate the account in the file global pool
  startTransaction();
  QMap<QString, MyMoneyAccount> accountList = m_sql->fetchAccounts (QString(account.id()), true);
  pos = accountList.find(account.id());
  if(pos != accountList.end()) {
    // check if the new info is based on the old one.
    // this is the case, when the file and the id
    // as well as the type are equal.
    if(((*pos).parentAccountId() == account.parentAccountId()
    && (*pos).accountType() == account.accountType())
    || skipCheck == true) {
      // make sure that all the referenced objects exist
      if(!account.institutionId().isEmpty())
        institution(account.institutionId());

      QValueList<QString>::ConstIterator it_a;
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
  QMap<QString, MyMoneyInstitution> institutionList = m_sql->fetchInstitutions(QString(institution.id()));
  QMap<QString, MyMoneyInstitution>::ConstIterator pos;

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
  QMap<QString, bool> modifiedAccounts;

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

//  QString oldKey = m_transactionKeys[transaction.id()];
  QMap <QString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions("('" + QString(transaction.id()) + "')");
//  if(transactionList.size() != 1)
//    throw new MYMONEYEXCEPTION("invalid transaction key");

  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;

//  it_t = transactionList.find(oldKey);
  it_t = transactionList.begin();
  if(it_t == transactionList.end())
    throw new MYMONEYEXCEPTION("invalid transaction key");

  // mark all accounts referenced in old and new transaction data
  // as modified
  QMap<QString, MyMoneyAccount> accountList = m_sql->fetchAccounts();
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
 // QString newKey = transaction.uniqueSortKey();
//  m_sql->insertTransaction(newKey, transaction);
  //m_transactionKeys.modify(transaction.id(), newKey);

  // mark file as changed
  m_sql->modifyTransaction(transaction);
}

void MyMoneyDatabaseMgr::reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent)
{
  if(account.accountType() == MyMoneyAccount::Stock && parent.accountType() != MyMoneyAccount::Investment)
    throw new MYMONEYEXCEPTION("Cannot move a stock acocunt into a non-investment account");

  QStringList accountIdList;
  QMap<QString, MyMoneyAccount>::ConstIterator oldParent;
  QMap<QString, MyMoneyAccount>::ConstIterator newParent;
  QMap<QString, MyMoneyAccount>::ConstIterator childAccount;

  // verify that accounts exist. If one does not,
  // an exception is thrown
  accountIdList << account.id() << parent.id();
  MyMoneyDatabaseMgr::account(account.id());
  MyMoneyDatabaseMgr::account(parent.id());

  if(!account.parentAccountId().isEmpty()) {
    accountIdList << account.parentAccountId();
  }

  startTransaction();
  QMap<QString, MyMoneyAccount> accountList = m_sql->fetchAccounts(accountIdList, true);

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

  m_sql->modifyAccount(parent);
  m_sql->modifyAccount(account);
  commitTransaction();
}

void MyMoneyDatabaseMgr::removeTransaction(const MyMoneyTransaction& transaction)
{
  QMap<QString, bool> modifiedAccounts;

  // first perform all the checks
  if(transaction.id().isEmpty())
    throw new MYMONEYEXCEPTION("invalid transaction to be deleted");

  QMap<QString, QString>::ConstIterator it_k;
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;

//  it_k = m_transactionKeys.find(transaction.id());
//  if(it_k == m_transactionKeys.end())
//    throw new MYMONEYEXCEPTION("invalid transaction to be deleted");

  QMap <QString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions("('" + QString(transaction.id()) + "')");
//  it_t = transactionList.find(*it_k);
  it_t = transactionList.begin();
  if(it_t == transactionList.end())
    throw new MYMONEYEXCEPTION("invalid transaction key");

  QValueList<MyMoneySplit>::ConstIterator it_s;

  // scan the splits and collect all accounts that need
  // to be updated after the removal of this transaction
  QMap<QString, MyMoneyAccount> accountList = m_sql->fetchAccounts();
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

unsigned int MyMoneyDatabaseMgr::transactionCount(const QString& account) const
{ return (m_sql->transactionCount(account)); }

const QMap<QString, unsigned long> MyMoneyDatabaseMgr::transactionCountMap(void) const
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
  if (m_sql) list = m_sql->fetchTransactions(filter).values();
  PASS
}

void MyMoneyDatabaseMgr::transactionList(QValueList<QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const
{
  list.clear();
  MyMoneyMap<QString, MyMoneyTransaction> transactionList;
  TRY
  if (m_sql) transactionList = m_sql->fetchTransactions(filter);
  PASS

  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  QMap<QString, MyMoneyTransaction>::ConstIterator txEnd = transactionList.end();

  for(it_t = transactionList.begin(); it_t != txEnd; ++it_t) {
    if(filter.match(*it_t)) {
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
  QStringList::ConstIterator it;
  for(it = account.accountList().begin(); it != account.accountList().end(); ++it) {
    MyMoneyDatabaseMgr::account(*it);
  }

  // if one of the accounts did not exist, an exception had been
  // thrown and we would not make it until here.

  QStringList accountIdList;
  accountIdList << parent.id() << account.id();
  startTransaction();
  QMap<QString, MyMoneyAccount> accountList = m_sql->fetchAccounts(accountIdList, true);

  QMap<QString, MyMoneyAccount>::ConstIterator it_a;
  QMap<QString, MyMoneyAccount>::ConstIterator it_p;

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
      for(it = (*it_a).accountList().begin(); it != (*it_a).accountList().end(); ++it) {
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
  QMap<QString, MyMoneyInstitution> institutionList = m_sql->fetchInstitutions(QString(institution.id()));
  QMap<QString, MyMoneyInstitution>::ConstIterator it_i;

  it_i = institutionList.find(institution.id());
  if(it_i != institutionList.end()) {
    // mark file as changed
    m_sql->removeInstitution(institution);
  } else
    throw new MYMONEYEXCEPTION("invalid institution");
}

const MyMoneyTransaction MyMoneyDatabaseMgr::transaction(const QString& id) const
{
  // get the full key of this transaction, throw exception
  // if it's invalid (unknown)
  //if(!m_transactionKeys.contains(id))
  //  throw new MYMONEYEXCEPTION("invalid transaction id");

  // check if this key is in the list, throw exception if not
  //QString key = m_transactionKeys[id];
  QMap <QString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions("('" + QString(id) + "')");

  //there should only be one transaction in the map, if it was found, so check the size of the map
  //return the first element.
  //if(!transactionList.contains(key))
  if(!transactionList.size())
    throw new MYMONEYEXCEPTION("invalid transaction key");

  return transactionList.begin().data();
}

const MyMoneyMoney MyMoneyDatabaseMgr::balance(const QString& id, const QDate& date) const
{
  QStringList idList;
  idList.append(id);
  QMap<QString,MyMoneyMoney> tempMap = m_sql->fetchBalance(idList, date);

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
  QMap<QString, MyMoneyAccount> accountList = m_sql->fetchAccounts(/*QString(id)*/);
  //QMap<QString, MyMoneyAccount>::const_iterator accpos = accountList.find(id);
  if (date_ != QDate()) qDebug ("request balance for %s at %s", id.data(), date_.toString(Qt::ISODate).latin1());
//  if(!date_.isValid() && MyMoneyFile::instance()->account(id).accountType() != MyMoneyAccount::Stock) {
//    if(accountList.find(id) != accountList.end())
//      return accountList[id].balance();
//    return MyMoneyMoney(0);
//  }
  if(/*m_balanceCache[id].valid == false || date != m_balanceCacheDate) || */ m_sql != 0) {
    QMap<QString, MyMoneyMoney> balances;
    QMap<QString, MyMoneyMoney>::ConstIterator it_b;
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
        const QString aid = (*it_s).accountId();
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
//      QMap<QString, MyMoneyAccount>::ConstIterator it_a;
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

const MyMoneyTransaction MyMoneyDatabaseMgr::transaction(const QString& account, const int idx) const
{
/* removed with MyMoneyAccount::Transaction
  QMap<QString, MyMoneyAccount>::ConstIterator acc;

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

unsigned int MyMoneyDatabaseMgr::institutionCount(void) const
{
  return m_sql->getRecCount("kmmInstitutions");
}

void MyMoneyDatabaseMgr::accountList(QValueList<MyMoneyAccount>& list) const
{
  QMap <QString, MyMoneyAccount> accountList;
  if (m_sql) accountList  = m_sql->fetchAccounts();
  QMap<QString, MyMoneyAccount>::ConstIterator it;
  QMap<QString, MyMoneyAccount>::ConstIterator accEnd = accountList.end();
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
  QMap<QString, MyMoneySecurity> securitiesList = m_sql->fetchSecurities(QString(security.id()), true);
  QMap<QString, MyMoneySecurity>::ConstIterator it;

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
  QMap<QString, MyMoneySecurity> securitiesList = m_sql->fetchSecurities(QString(security.id()));
  QMap<QString, MyMoneySecurity>::ConstIterator it;

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

const MyMoneySecurity MyMoneyDatabaseMgr::security(const QString& id) const
{
  QMap<QString, MyMoneySecurity> securitiesList = m_sql->fetchSecurities(QString(id));
  QMap<QString, MyMoneySecurity>::ConstIterator it = securitiesList.find(id);
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

const MyMoneyPrice MyMoneyDatabaseMgr::price(const QString& fromId, const QString& toId, const QDate& _date, const bool exactDate) const
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

  if (m_sql) {
    sched = MyMoneySchedule (nextScheduleID(), sched);

    m_sql->addSchedule(sched);
  }
}

void MyMoneyDatabaseMgr::modifySchedule(const MyMoneySchedule& sched)
{
  QMap<QString, MyMoneySchedule> scheduleList = m_sql->fetchSchedules(QString(sched.id()));
  QMap<QString, MyMoneySchedule>::ConstIterator it;

  it = scheduleList.find(sched.id());
  if(it == scheduleList.end()) {
    QString msg = "Unknown schedule '" + sched.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_sql->modifySchedule(sched);
}

void MyMoneyDatabaseMgr::removeSchedule(const MyMoneySchedule& sched)
{
  QMap<QString, MyMoneySchedule> scheduleList = m_sql->fetchSchedules(QString(sched.id()));
  QMap<QString, MyMoneySchedule>::ConstIterator it;

  it = scheduleList.find(sched.id());
  if(it == scheduleList.end()) {
    QString msg = "Unknown schedule '" + sched.id() + "'";
    throw new MYMONEYEXCEPTION(msg);
  }

  // FIXME: check referential integrity for loan accounts

  m_sql->removeSchedule(sched);
}

const MyMoneySchedule MyMoneyDatabaseMgr::schedule(const QString& id) const
{
  QMap<QString, MyMoneySchedule> scheduleList = m_sql->fetchSchedules(QString(id));
  QMap<QString, MyMoneySchedule>::ConstIterator pos;

  // locate the schedule and if present, return it's data
  pos = scheduleList.find(id);
  if(pos != scheduleList.end())
    return (*pos);

  // throw an exception, if it does not exist
  QString msg = "Unknown schedule id '" + id + "'";
  throw new MYMONEYEXCEPTION(msg);
}

const QValueList<MyMoneySchedule> MyMoneyDatabaseMgr::scheduleList(const QString& accountId,
                                     const MyMoneySchedule::typeE type,
                                     const MyMoneySchedule::occurenceE occurence,
                                     const MyMoneySchedule::paymentTypeE paymentType,
                                     const QDate& startDate,
                                     const QDate& endDate,
                                     const bool overdue) const
{
  QMap<QString, MyMoneySchedule> scheduleList;
  if (m_sql) scheduleList = m_sql->fetchSchedules();
  QMap<QString, MyMoneySchedule>::ConstIterator pos;
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
                                              const QStringList& accounts) const
{
//  qDebug("scheduleListEx");
  QMap<QString, MyMoneySchedule> scheduleList = m_sql->fetchSchedules();
  QMap<QString, MyMoneySchedule>::ConstIterator pos;
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
  if (m_sql) {
    QMap<QString, MyMoneySecurity> currencyList = m_sql->fetchCurrencies(QString(currency.id()));
    QMap<QString, MyMoneySecurity>::ConstIterator it;

    it = currencyList.find(currency.id());
    if(it != currencyList.end()) {
      throw new MYMONEYEXCEPTION(QString("Cannot add currency with existing id %1").arg(currency.id().data()));
    }

    m_sql->addCurrency(currency);
  }
}

void MyMoneyDatabaseMgr::modifyCurrency(const MyMoneySecurity& currency)
{
  QMap<QString, MyMoneySecurity> currencyList = m_sql->fetchCurrencies(QString(currency.id()));
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = currencyList.find(currency.id());
  if(it == currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot modify currency with unknown id %1").arg(currency.id().data()));
  }

  m_sql->modifyCurrency(currency);
}

void MyMoneyDatabaseMgr::removeCurrency(const MyMoneySecurity& currency)
{
  QMap<QString, MyMoneySecurity> currencyList = m_sql->fetchCurrencies(QString(currency.id()));
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  // FIXME: check referential integrity

  it = currencyList.find(currency.id());
  if(it == currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot remove currency with unknown id %1").arg(currency.id().data()));
  }

  m_sql->removeCurrency(currency);
}

const MyMoneySecurity MyMoneyDatabaseMgr::currency(const QString& id) const
{
  if(id.isEmpty()) {

  }
  QMap<QString, MyMoneySecurity> currencyList = m_sql->fetchCurrencies(QString(id));
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = currencyList.find(id);
  if(it == currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot retrieve currency with unknown id '%1'").arg(id.data()));
  }

  return *it;
}

const QValueList<MyMoneySecurity> MyMoneyDatabaseMgr::currencyList(void) const
{
  if (m_sql) {
    return m_sql->fetchCurrencies().values();
  } else {
    return QValueList<MyMoneySecurity> ();
  }
}

const QValueList<MyMoneyReport> MyMoneyDatabaseMgr::reportList( void ) const
{
  if (m_sql) {
    return m_sql->fetchReports().values();
  } else {
    return QValueList<MyMoneyReport> ();
  }
}

void MyMoneyDatabaseMgr::addReport( MyMoneyReport& report )
{
  if(!report.id().isEmpty())
    throw new MYMONEYEXCEPTION("transaction already contains an id");

  m_sql->addReport(MyMoneyReport (nextReportID(), report));
}

void MyMoneyDatabaseMgr::modifyReport( const MyMoneyReport& report )
{
  QMap<QString, MyMoneyReport> reportList = m_sql->fetchReports(QString(report.id()));
  QMap<QString, MyMoneyReport>::ConstIterator it;

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

const MyMoneyReport MyMoneyDatabaseMgr::report( const QString& id ) const
{
  return m_sql->fetchReports(QString(id))[id];
}

void MyMoneyDatabaseMgr::removeReport(const MyMoneyReport& report)
{
  QMap<QString, MyMoneyReport> reportList = m_sql->fetchReports(QString(report.id()));
  QMap<QString, MyMoneyReport>::ConstIterator it;

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
  QMap<QString, MyMoneyBudget> budgets = m_sql->fetchBudgets();
  QMap<QString, MyMoneyBudget>::ConstIterator it_p;

  for(it_p = budgets.begin(); it_p != budgets.end(); ++it_p) {
    if((*it_p).name() == budget) {
      return *it_p;
    }
  }

  throw new MYMONEYEXCEPTION("Unknown budget '" + budget + "'");
}

void MyMoneyDatabaseMgr::modifyBudget( const MyMoneyBudget& budget )
{
  //QMap<QString, MyMoneyBudget>::ConstIterator it;

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

MyMoneyBudget MyMoneyDatabaseMgr::budget( const QString& id ) const
{
  return m_sql->fetchBudgets(QString(id)) [id];
}

void MyMoneyDatabaseMgr::removeBudget(const MyMoneyBudget& budget)
{
//  QMap<QString, MyMoneyBudget>::ConstIterator it;
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
    isReferencedHelper(const QString& id)
      : m_id (id)
    {}

    inline bool operator() (const MyMoneyObject& obj) const
    { return obj.hasReferenceTo(m_id); }

  private:
    QString m_id;
};

bool MyMoneyDatabaseMgr::isReferenced(const MyMoneyObject& obj, const MyMoneyFileBitArray& skipCheck) const
{
  bool rc = false;
  const QString& id = obj.id();

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
      //QMap <QString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions(f);
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
    QMap<QString, MyMoneyReport> reportList = m_sql->fetchReports();
    rc = (reportList.end() != std::find_if(reportList.begin(), reportList.end(),  isReferencedHelper(id)));
  }
  if(!skipCheck[RefCheckBudget] && !rc) {
    QMap<QString, MyMoneyBudget> budgets = m_sql->fetchBudgets();
    rc = (budgets.end() != std::find_if(budgets.begin(), budgets.end(),  isReferencedHelper(id)));
  }
  if(!skipCheck[RefCheckSchedule] && !rc) {
    QMap<QString, MyMoneySchedule> scheduleList = m_sql->fetchSchedules();
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
{ if (m_sql) m_sql->startCommitUnit ("databasetransaction"); }

bool MyMoneyDatabaseMgr::commitTransaction(void)
{
  if (m_sql)
    return m_sql->endCommitUnit ("databasetransaction");
  return false;
}

void MyMoneyDatabaseMgr::rollbackTransaction(void)
{ if (m_sql) m_sql->cancelCommitUnit ("databasetransaction"); }

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

bool MyMoneyDatabaseMgr::isDuplicateTransaction(const QString& /*id*/) const
{
  //FIXME: figure out the real id from the key and check the DB.
//return m_transactionKeys.contains(id);
  return false;
}

void MyMoneyDatabaseMgr::loadAccounts(const QMap<QString, MyMoneyAccount>& /*map*/)
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

void MyMoneyDatabaseMgr::loadTransactions(const QMap<QString, MyMoneyTransaction>& /*map*/)
{
//  m_transactionList = map;
//FIXME: update the database.

//  // now fill the key map
//  QMap<QString, QString> keys;
//  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
//  for(it_t = map.begin(); it_t != map.end(); ++it_t) {
//    keys[(*it_t).id()] = it_t.key();
//  }
//  m_transactionKeys = keys;
}

void MyMoneyDatabaseMgr::loadInstitutions(const QMap<QString, MyMoneyInstitution>& /*map*/)
{
//  m_institutionList = map;
//FIXME: update the database.

//  // now fill the key map
//  QMap<QString, QString> keys;
//  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
//  for(it_t = map.begin(); it_t != map.end(); ++it_t) {
//    keys[(*it_t).id()] = it_t.key();
//  }
//  m_transactionKeys = keys;
}

void MyMoneyDatabaseMgr::loadPayees(const QMap<QString, MyMoneyPayee>& /*map*/)
{
//  m_payeeList = map;
}

void MyMoneyDatabaseMgr::loadSchedules(const QMap<QString, MyMoneySchedule>& /*map*/)
{
//  m_scheduleList = map;
}

void MyMoneyDatabaseMgr::loadSecurities(const QMap<QString, MyMoneySecurity>& /*map*/)
{
//  m_securitiesList = map;
}

void MyMoneyDatabaseMgr::loadCurrencies(const QMap<QString, MyMoneySecurity>& /*map*/)
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

void MyMoneyDatabaseMgr::loadReports( const QMap<QString, MyMoneyReport>& /*reports*/ )
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

void MyMoneyDatabaseMgr::loadBudgets( const QMap<QString, MyMoneyBudget>& /*budgets*/ )
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

void MyMoneyDatabaseMgr::loadPrices(const MyMoneyPriceList& list)
{
  Q_UNUSED(list);
}

unsigned long MyMoneyDatabaseMgr::accountId(void) const
{ return m_sql->getNextAccountId(); }

unsigned long MyMoneyDatabaseMgr::transactionId(void) const
{ return m_sql->getNextTransactionId(); }

unsigned long MyMoneyDatabaseMgr::payeeId(void) const
{ return m_sql->getNextPayeeId(); }

unsigned long MyMoneyDatabaseMgr::institutionId(void) const
{ return m_sql->getNextInstitutionId(); }

unsigned long MyMoneyDatabaseMgr::scheduleId(void) const
{ return m_sql->getNextScheduleId(); }

unsigned long MyMoneyDatabaseMgr::securityId(void) const
{ return m_sql->getNextSecurityId(); }

unsigned long MyMoneyDatabaseMgr::reportId(void) const
{ return m_sql->getNextReportId(); }

unsigned long MyMoneyDatabaseMgr::budgetId(void) const
{ return m_sql->getNextBudgetId(); }

void MyMoneyDatabaseMgr::loadAccountId(const unsigned long id)
{
  m_sql->loadAccountId(id);
}

void MyMoneyDatabaseMgr::loadTransactionId(const unsigned long id)
{
  m_sql->loadTransactionId(id);
}

void MyMoneyDatabaseMgr::loadPayeeId(const unsigned long id)
{
  m_sql->loadPayeeId(id);
}

void MyMoneyDatabaseMgr::loadInstitutionId(const unsigned long id)
{
  m_sql->loadInstitutionId(id);
}

void MyMoneyDatabaseMgr::loadScheduleId(const unsigned long id)
{
  m_sql->loadScheduleId(id);
}

void MyMoneyDatabaseMgr::loadSecurityId(const unsigned long id)
{
  m_sql->loadSecurityId(id);
}

void MyMoneyDatabaseMgr::loadReportId(const unsigned long id)
{
  m_sql->loadReportId(id);
}

void MyMoneyDatabaseMgr::loadBudgetId(const unsigned long id)
{
  m_sql->loadBudgetId(id);
}

void MyMoneyDatabaseMgr::rebuildAccountBalances(void)
{
  startTransaction();
  QMap<QString, MyMoneyAccount> accountMap = m_sql->fetchAccounts(QStringList(), true);

  QMap<QString, MyMoneyMoney> balanceMap = m_sql->fetchBalance(accountMap.keys(), QDate());

  for (QMap<QString, MyMoneyMoney>::const_iterator it_b = balanceMap.begin();
         it_b != balanceMap.end(); ++it_b) {
    accountMap[it_b.key()].setBalance(it_b.data());
  }

  for (QMap<QString, MyMoneyAccount>::const_iterator it_a = accountMap.begin();
         it_a != accountMap.end(); ++it_a) {
    m_sql->modifyAccount(it_a.data());
  }
  commitTransaction();
}

#undef TRY
#undef CATCH
#undef PASS
