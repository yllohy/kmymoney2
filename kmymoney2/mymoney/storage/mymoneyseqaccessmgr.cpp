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

MyMoneySeqAccessMgr::MyMoneySeqAccessMgr()
{
  m_nextAccountID = 0;
  m_nextInstitutionID = 0;
  m_nextTransactionID = 0;
  m_nextPayeeID = 0;
  m_userName =
  m_userStreet =
  m_userTown =
  m_userCounty =
  m_userPostcode =
  m_userTelephone =
  m_userEmail = "";
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

  MyMoneyBalanceCacheItem balance;

  m_balanceCache[STD_ACC_LIABILITY] = balance;
  m_balanceCache[STD_ACC_ASSET] = balance;
  m_balanceCache[STD_ACC_EXPENSE] = balance;
  m_balanceCache[STD_ACC_INCOME] = balance;
}

MyMoneySeqAccessMgr::~MyMoneySeqAccessMgr()
{
}

const bool MyMoneySeqAccessMgr::isStandardAccount(const QCString& id) const
{
  return id == STD_ACC_LIABILITY
      || id == STD_ACC_ASSET
      || id == STD_ACC_EXPENSE
      || id == STD_ACC_INCOME;
}

void MyMoneySeqAccessMgr::setAccountName(const QCString& id, const QString& name)
{
  if(!isStandardAccount(id))
    throw new MYMONEYEXCEPTION("Only standard accounts can be modified using setAccountName()");

  m_accountList[id].setName(name);
}

const MyMoneyAccount& MyMoneySeqAccessMgr::account(const QCString id) const
{
  QMap<QCString, MyMoneyAccount>::ConstIterator pos;

  // locate the account and if present, return it's data
  pos = m_accountList.find(id);
  if(pos != m_accountList.end())
    return (*pos);

  // throw an exception, if it does not exist
  QString msg = "Unknown account id '" + id + "'";
  throw new MYMONEYEXCEPTION(msg);
}

const QValueList<MyMoneyAccount> MyMoneySeqAccessMgr::accountList(void) const
{
  QValueList<MyMoneyAccount> list;
  QMap<QCString, MyMoneyAccount>::ConstIterator it;

  for(it = m_accountList.begin(); it != m_accountList.end(); ++it) {
    if(!isStandardAccount((*it).id()))
      list.append(*it);
  }
  return list;
}

void MyMoneySeqAccessMgr::newAccount(MyMoneyAccount& account)
{
  // create the account
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

const MyMoneyPayee MyMoneySeqAccessMgr::payee(const QCString& id) const
{
  QMap<QCString, MyMoneyPayee>::ConstIterator it;

  it = m_payeeList.find(id);
  if(it == m_payeeList.end())
    throw new MYMONEYEXCEPTION("Unknown payee");

  return *it;
}

void MyMoneySeqAccessMgr::modifyPayee(const MyMoneyPayee& payee)
{
  QMap<QCString, MyMoneyPayee>::Iterator it;

  it = m_payeeList.find(payee.id());
  if(it == m_payeeList.end())
    throw new MYMONEYEXCEPTION("Unknown payee");

  touch();
  *it = payee;
}

void MyMoneySeqAccessMgr::removePayee(const MyMoneyPayee& payee)
{
  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
  QValueList<MyMoneySplit>::ConstIterator it_s;
  QMap<QCString, MyMoneyPayee>::Iterator it_p;

  it_p = m_payeeList.find(payee.id());
  if(it_p == m_payeeList.end())
    throw new MYMONEYEXCEPTION("Unknown payee");

  // scan all transactions to check if the payee is still referenced
  for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    // scan all splits of this transaction
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      if((*it_s).payeeId() == payee.id())
        throw new MYMONEYEXCEPTION("Cannot remove payee that is referenced");
    }
  }

  m_payeeList.remove(it_p);
  touch();
}

const QValueList<MyMoneyPayee> MyMoneySeqAccessMgr::payeeList(void) const
{
  QValueList<MyMoneyPayee> list;
  QMap<QCString, MyMoneyPayee>::ConstIterator it;

  for(it = m_payeeList.begin(); it != m_payeeList.end(); ++it) {
    list.append(*it);
  }
  return list;
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

void MyMoneySeqAccessMgr::addTransaction(MyMoneyTransaction& transaction, const bool skipAccountUpdate)
{
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
    // the following lines will throw an exception if the
    // account or payee do not exist
    account((*it_s).accountId());
    if((*it_s).payeeId() != "")
      payee((*it_s).payeeId());
  }

  MyMoneyTransaction newTransaction(nextTransactionID(), transaction);
  QCString key = transactionKey(newTransaction);

  m_transactionList[key] = newTransaction;
  m_transactionKeys[newTransaction.id()] = key;

  transaction = newTransaction;

  // mark file as changed
  touch();

  if(!skipAccountUpdate) {
    // now update those accounts that need a reference
    // to this transaction
    QValueList<MyMoneySplit>::ConstIterator it;
    for(it = transaction.splits().begin(); it != transaction.splits().end(); ++it) {
      QMap<QCString, MyMoneyAccount>::Iterator acc;
      acc = m_accountList.find((*it).accountId());
      if(acc != m_accountList.end()) {
        (*acc).touch();
        refreshAccountTransactionList(acc);
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

void MyMoneySeqAccessMgr::refreshAllAccountTransactionLists(void)
{
/* removed with MyMoneyAccount::Transaction
  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
  QMap<QCString, MyMoneyAccount>::Iterator it_a;
  QValueList<MyMoneySplit>::ConstIterator it_s;

  for(it_a = m_accountList.begin(); it_a != m_accountList.end(); ++it_a)
    (*it_a).clearTransactions();

  // scan all transactions
  for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    // scan all splits of this transaction
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      MyMoneyAccount::Transaction val((*it_t).id() ,(*it_s).value());
      it_a = m_accountList.find((*it_s).accountId());
      (*it_a).addTransaction(val);
    }
  }
*/
}

void MyMoneySeqAccessMgr::refreshAccountTransactionList(QMap<QCString, MyMoneyAccount>::Iterator acc) const
{
/* removed with MyMoneyAccount::Transaction
  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
  QValueList<MyMoneySplit>::ConstIterator it_s;

  (*acc).clearTransactions();

  // scan all transactions
  for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    // scan all splits of this transaction
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      // is it a split in our account?
      if((*it_s).accountId() == (*acc).id()) {
        MyMoneyAccount::Transaction val((*it_t).id() ,(*it_s).value());
        (*acc).addTransaction(val);
      }
    }
  }
*/
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
  QValueList<MyMoneyInstitution> list;
  QMap<QCString, MyMoneyInstitution>::ConstIterator it;

  for(it = m_institutionList.begin(); it != m_institutionList.end(); ++it) {
    list.append(*it);
  }
  return list;
}

void MyMoneySeqAccessMgr::modifyAccount(const MyMoneyAccount& account)
{
  QMap<QCString, MyMoneyAccount>::Iterator pos;

  // locate the account in the file global pool
  pos = m_accountList.find(account.id());
  if(pos != m_accountList.end()) {
    // check if the new info is based on the old one.
    // this is the case, when the file and the id
    // as well as the type are equal.
    if((*pos).parentAccountId() == account.parentAccountId()
    && (*pos).accountType() == account.accountType()) {
      // if it points to a different institution, then update both
      if((*pos).institutionId() != account.institutionId()) {
        // check if new institution exists
        if(account.institutionId() != "")
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
  if(transaction.id() == ""
//  || transaction.file() != this
  || !transaction.postDate().isValid())
    throw new MYMONEYEXCEPTION("invalid transaction to be modified");

  // now check the splits
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    // the following lines will throw an exception if the
    // account or payee do not exist
    account((*it_s).accountId());
    if((*it_s).payeeId() != "")
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
    modifiedAccounts[(*it_s).accountId()] = true;
  }
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
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
      refreshAccountTransactionList(acc);
      invalidateBalanceCache((*acc).id());
    }
  }
}

void MyMoneySeqAccessMgr::reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent)
{
  reparentAccount(account, parent, true);
}

void MyMoneySeqAccessMgr::reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent, const bool sendNotification)
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

  newParent = m_accountList.find(parent.id());
  childAccount = m_accountList.find(account.id());

  if(account.parentAccountId().length() != 0)
    (*oldParent).removeAccountId(account.id());

  (*newParent).addAccountId(account.id());
  (*childAccount).setParentAccountId(parent.id());

  parent = *newParent;
  account = *childAccount;

  // mark file as changed
  touch();
}

void MyMoneySeqAccessMgr::removeTransaction(const MyMoneyTransaction& transaction)
{
  QMap<QCString, bool> modifiedAccounts;

  // first perform all the checks
  if(transaction.id() == "")
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
  }

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
      refreshAccountTransactionList(acc);
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

  // check if the new info is based on the old one.
  // this is the case, when the file and the id
  // as well as the type are equal.
  if((*it_a).file() == account.file()
  && (*it_a).id() == account.id()
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
        acc.setInstitutionId("");
        try {
          modifyAccount(acc);
        } catch(MyMoneyException *e) {
          qDebug(e->what());
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

const QValueList<MyMoneyTransaction> MyMoneySeqAccessMgr::transactionList(const QCString& account) const
{
  QValueList<MyMoneyTransaction> list;
  QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
  QValueList<MyMoneySplit>::ConstIterator it_s;

  // scan all transactions
  for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    bool append = true;

    if(account.length() != 0) {
      // scan all splits of this transaction
      for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
        // is it a split in our account?
        if((*it_s).accountId() == account) {
          // since a transaction can only have one split referencing
          // each account, we're done with the splits here!
          break;
        }
      }
      // reset flag, if no split contains the account id
      if(it_s == (*it_t).splits().end())
        append = false;

    }

    if(append == true)
      list.append(*it_t);
  }
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
  list = transactionList(account);
  if(idx < 0 || idx >= static_cast<int> (list.count()))
    throw new MYMONEYEXCEPTION("Unknown idx for transaction");

  return transaction(list[idx].id());
}

const MyMoneyMoney MyMoneySeqAccessMgr::balance(const QCString& id)
{
  MyMoneyMoney result(0);
  MyMoneyAccount acc;

  if(m_balanceCache[id].valid == true)
    result = m_balanceCache[id].balance;

  else {
    acc = account(id);

  /* removed with MyMoneyAccount::Transaction
    return acc.balance();
  */

    // new implementation if the above code does not work anymore
    result += acc.openingBalance();

    QValueList<MyMoneyTransaction> list;
    QValueList<MyMoneyTransaction>::ConstIterator it;
    MyMoneySplit split;
    list = transactionList(id);

    for(it = list.begin(); it != list.end(); ++it) {
      try {
        split = (*it).split(id);
        result += split.value();

      } catch(MyMoneyException *e) {
        // account is not referenced within this transaction
        delete e;
      }
    }

    MyMoneyBalanceCacheItem balance(result);
    m_balanceCache[id] = balance;
  }
  return result;
}

const MyMoneyMoney MyMoneySeqAccessMgr::totalBalance(const QCString& id)
{
  QCStringList accounts;
  QCStringList::ConstIterator it_a;

  MyMoneyMoney result(balance(id));

  MyMoneyAccount acc;

  acc = account(id);
  accounts = acc.accountList();

  for(it_a = accounts.begin(); it_a != accounts.end(); ++it_a) {
    result += balance(*it_a);
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
  MyMoneyAccount  acc;

  try {
    m_balanceCache[id].valid = false;
    if(!isStandardAccount(id)) {
      acc = account(id);
      invalidateBalanceCache(acc.parentAccountId());
    }
  } catch (MyMoneyException *e) {
    delete e;
  }
}