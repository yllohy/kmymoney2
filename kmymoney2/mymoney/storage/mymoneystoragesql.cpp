/***************************************************************************
                          mymoneystoragesql.cpp
                          ---------------------
    begin                : 11 November 2005
    copyright            : (C) 2005 by Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
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
#include <qstringlist.h>
#include <qiodevice.h>
#include <qcstring.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragesql.h"
#include "../../kmymoneyutils.h"

//************************ Constructor/Destructor *****************************
MyMoneyStorageSql::MyMoneyStorageSql (const QString& driverName, IMyMoneySerialize *storage)
  : QSqlDatabase (driverName, QString("kmmdatabase")) {
  m_majorVersion = 0;
  m_minorVersion = 1;
  m_progressCallback = 0;
  m_storage = storage;
}

int MyMoneyStorageSql::open(const KURL& url, int mode, bool clear) {
try {
  // create the database connection
  QString dbName = url.path().right(url.path().length() - 1); // remove separator slash
  setDatabaseName(dbName);
  setHostName(url.host());
  setUserName(url.user());
  setPassword(url.pass());

  switch (mode) {
    case IO_ReadOnly:    // OpenDatabase menu entry (or open last file)
    case IO_ReadWrite:   // Save menu entry with database open
      if (!QSqlDatabase::open()) {
        buildError(QSqlQuery(), "opening database");
        return(1);
      }
      return (createTables()); // check all tables are present, create if not (we may add tables at some time)
    case IO_WriteOnly:   // SaveAs Database - if exists, must be empty, if not will create
      if (!QSqlDatabase::open()) {
        if (createDatabase(url) != 0) {
          return(1);
        } else {
          if (!QSqlDatabase::open()) {
            buildError(QSqlQuery(), "opening new database");
            return(1);
          } else {
            return(createTables());
          }
        }
      } else {
        createTables();
        if (clear) {
          clean();
          return (0);
        } else {
          return (isEmpty());
        }
      }
  }
  qFatal ("oops in mymoneystoragesql.cpp open");
  return (1);
} catch (QString& s) {
    qDebug("%s",s.data());
    return (1);
}
}

int MyMoneyStorageSql::createDatabase (const KURL& url) {
  if (driverName() != "QMYSQL3") {
    m_error = QString(tr("Cannot currently create database for driver %1")).arg(driverName());
    return (1);
  }
// create the database (only works for mysql at present)
  QString dbName = url.path().right(url.path().length() - 1); // remove separator slash
  QSqlDatabase *maindb = QSqlDatabase::addDatabase(driverName());
  maindb->setDatabaseName ("mysql");
  maindb->setHostName (url.host());
  maindb->setUserName (url.user());
  maindb->setPassword (url.pass());
  maindb->open();
  QSqlQuery qm = (maindb->exec());
  QString qs = QString("CREATE DATABASE %1;").arg(dbName);
  qm.prepare (qs);
  if (!qm.exec()) {
    buildError (qm, "Error in create database %1").arg(dbName);
    return (1);
  }
  QSqlDatabase::removeDatabase (maindb);
  return (0);
}

int MyMoneyStorageSql::createTables () {
  // check tables, create if required
  QMapConstIterator<QString, MyMoneyDbTable> tt = m_db.begin();
  while (tt != m_db.end()) {
    if (!tables().contains(tt.key())) createTable (tt.data());
    ++tt;
  }
  return (0); // any errors will be caught by exception handling
}

void MyMoneyStorageSql::createTable (const MyMoneyDbTable& t) {
// create the tables
  QString qs = QString("CREATE TABLE %1 (").arg(t.name());
  QValueList<MyMoneyDbField>::const_iterator it = t.begin();
  while (it != t.end()) {
    qs = qs +  (*it).name() + " " +
        (*it).type();
    if ((*it).isPrimaryKey()) qs += " PRIMARY KEY";
    if ((*it).isNotNull()) qs += " NOT NULL";
    qs += ", ";
    ++it;
  }
  qs = qs.left(qs.length() -2) + ");";
  QSqlQuery q(this);
  q.prepare (qs);
  if (!q.exec()) throw buildError(q, QString ("creating table %1").arg(t.name()));
}

int MyMoneyStorageSql::isEmpty () {
  // check all tables are empty
  QMapConstIterator<QString, MyMoneyDbTable> tt = m_db.begin();
  int recordCount = 0;
  while ((tt != m_db.end()) && (recordCount == 0)) {
    QSqlQuery q(this);
    q.prepare (QString("select count(*) from %1;").arg((*tt).name()));
    if (!q.exec()) throw buildError(q, "getting record count");
    if (!q.next()) throw buildError(q, "retrieving record count");
    recordCount += q.value(0).toInt();
    ++tt;
  }

  if (recordCount != 0) {
    return (-1); // not empty
  } else {
    return (0);
  }
}

void MyMoneyStorageSql::clean() {
// delete all existing records
  QMapConstIterator<QString, MyMoneyDbTable> it = m_db.begin();
  while (it != m_db.end()) {
    QSqlQuery q(this);
    q.prepare(QString("DELETE from %1;").arg(it.key()));
    if (!q.exec()) throw buildError(q, QString ("cleaning database"));
    ++it;
  }
}

//////////////////////////////////////////////////////////////////

bool MyMoneyStorageSql::readFile(void) {
  try {
  readFileInfo();
  readInstitutions();
  readPayees();
  readCurrencies();
  readSecurities();
  readAccounts();
  readTransactions();
  readSchedules();
  readPrices();
  readReports();
  
  m_storage->rebuildAccountBalances();
  // this seems to be nonsense, but it clears the dirty flag
  // as a side-effect.
  m_storage->setLastModificationDate(m_storage->lastModificationDate());
  m_storage = NULL;
  // make sure the progress bar is not shown any longer
  signalProgress(-1, -1);
  return true;
  } catch (QString& s) {
    return false;
  }
}

bool MyMoneyStorageSql::writeFile(void) {
  // initialize record counts and hi ids
  m_institutions = m_accounts = m_payees = m_transactions = m_splits
      = m_securities = m_prices = m_currencies = m_schedules  = m_reports = m_kvps = 0;
  m_hiIdInstitutions = m_hiIdPayees = m_hiIdAccounts = m_hiIdTransactions =
      m_hiIdSchedules = m_hiIdSecurities = 0;
  try{
  writeInstitutions ();
  writePayees();
  writeAccounts();
  writeTransactions();
  writeSchedules();
  writeSecurities();
  writePrices();
  writeCurrencies();
  writeReports();
  writeFileInfo();
  // this seems to be nonsense, but it clears the dirty flag
  // as a side-effect.
  m_storage->setLastModificationDate(m_storage->lastModificationDate());
  m_storage = NULL;
  // make sure the progress bar is not shown any longer
  signalProgress(-1, -1);
  return true;
} catch (QString& s) {
  return false;
}
}

//------------------------------ Write SQL routines ----------------------------------------
// **** Institutions ****
void MyMoneyStorageSql::writeInstitutions() {
  // first, get a list of what's on the database
  // anything not in the list needs to be inserted
  // anything which is will be updated and removed from the list
  // anything left over at the end will need to be deleted
  // this is an expensive and inconvenient way to do things; find a better way
  // one way would be to build the lists when reading the db
  // unfortunately this object does not persiste between read and write
  // it would also be nice if we could tell which objects had been updated since we read them in
  QValueList<QString> dbList;
  QSqlQuery q(this);
  q.prepare("SELECT id FROM kmmInstitutions;");
  if (!q.exec()) throw buildError (q, "building Institution list");
  while (q.next()) dbList.append(q.value(0).toString());

  const QValueList<MyMoneyInstitution> list = m_storage->institutionList();
  QValueList<MyMoneyInstitution>::ConstIterator it;
  signalProgress(0, list.count(), "Writing Institutions...");
  for(it = list.begin(); it != list.end(); ++it) {
    if (dbList.contains((*it).id())) {
      q.prepare (m_db.m_tables["kmmInstitutions"].updateString());
      dbList.remove ((*it).id());
    } else {
      q.prepare (m_db.m_tables["kmmInstitutions"].insertString());
    }
    writeInstitution(*it, q);
    m_hiIdInstitutions = calcHighId(m_hiIdInstitutions, (*it).id());
    signalProgress (++m_institutions, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare("DELETE FROM kmmInstitutions WHERE id = :id");
      q.bindValue(":id", (*it));
      if (!q.exec()) throw buildError(q, "deleting Institution");
      deleteKeyValuePairs("OFXSETTINGS", (*it));
      ++it;
    }
  }
}

void MyMoneyStorageSql::writeInstitution(const MyMoneyInstitution& i, QSqlQuery& q) {
  q.bindValue(":id", i.id());
  q.bindValue(":name", i.name());
  q.bindValue(":manager", i.manager());
  q.bindValue(":routingCode", i.sortcode());
  q.bindValue(":addressStreet", i.street());
  q.bindValue(":addressCity", i.city());
  q.bindValue(":addressZipcode", i.postcode());
  q.bindValue(":telephone", i.telephone());
  if (!q.exec()) throw buildError(q, QString("writing Institution"));
}

// **** Payees ****
void MyMoneyStorageSql::writePayees() {
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  QSqlQuery q(this);
  q.prepare("SELECT id FROM kmmPayees;");
  if (!q.exec()) throw buildError (q, "building Payee list");
  while (q.next()) dbList.append(q.value(0).toString());

  QValueList<MyMoneyPayee> list = m_storage->payeeList();
  MyMoneyPayee user(QCString("USER"), m_storage->user());
  list.prepend(user);
  signalProgress(0, list.count(), "Writing Payees...");
  QValueList<MyMoneyPayee>::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it) {
    if (dbList.contains((*it).id())) {
      q.prepare (m_db.m_tables["kmmPayees"].updateString());
      dbList.remove ((*it).id());
    } else {
      q.prepare (m_db.m_tables["kmmPayees"].insertString());
    }
    writePayee(*it, q);
    m_hiIdPayees = calcHighId(m_hiIdPayees, (*it).id());
    signalProgress(++m_payees, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare("DELETE FROM kmmPayees WHERE id = :id");
      q.bindValue(":id", (*it));
      if (!q.exec()) throw buildError(q, "deleting Payee");
      ++it;
    }
  }
}

void MyMoneyStorageSql::writePayee(const MyMoneyPayee& p, QSqlQuery& q) {
  q.bindValue(":id", p.id());
  q.bindValue(":name", p.name());
  q.bindValue(":reference", p.reference());
  q.bindValue(":email", p.email());
  q.bindValue(":addressStreet", p.address());
  q.bindValue(":addressCity", p.city());
  q.bindValue(":addressZipcode", p.postcode());
  q.bindValue(":addressState", p.state());
  q.bindValue(":telephone", p.telephone());
  if (!q.exec()) throw buildError(q, QString ("writing Payee"));
}

// **** Accounts ****
void MyMoneyStorageSql::writeAccounts() {
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  QSqlQuery q(this);
  q.prepare("SELECT id FROM kmmAccounts;");
  if (!q.exec()) throw buildError (q, "building Account list");
  while (q.next()) dbList.append(q.value(0).toString());

  const QValueList<MyMoneyAccount> list = m_storage->accountList();
  QValueList<MyMoneyAccount>::ConstIterator it;
  signalProgress(0, list.count(), "Writing Accounts...");
  if (dbList.isEmpty()) { // new table, insert standard accounts
    q.prepare (m_db.m_tables["kmmAccounts"].insertString());
  } else {
    q.prepare (m_db.m_tables["kmmAccounts"].updateString());
  }
  writeAccount(m_storage->asset(), q);
  writeAccount(m_storage->liability(), q);
  writeAccount(m_storage->expense(), q);
  writeAccount(m_storage->income(), q);
  writeAccount(m_storage->equity(), q);
  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i) {
    if (dbList.contains((*it).id())) {
      q.prepare (m_db.m_tables["kmmAccounts"].updateString());
      dbList.remove ((*it).id());
    } else {
      q.prepare (m_db.m_tables["kmmAccounts"].insertString());
    }
    writeAccount(*it, q);
    m_hiIdAccounts = calcHighId(m_hiIdAccounts, (*it).id());
    signalProgress(++m_accounts, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      if (!dynamic_cast<IMyMoneyStorage*>(m_storage)->isStandardAccount(QCString(*it))) {
        q.prepare("DELETE FROM kmmAccounts WHERE id = :id");
        q.bindValue(":id", (*it));
        if (!q.exec()) throw buildError(q, "deleting Account");
        deleteKeyValuePairs("ACCOUNT", (*it));
      }
      ++it;
    }
  }
}

void MyMoneyStorageSql::writeAccount(const MyMoneyAccount& acc, QSqlQuery& q) {
  q.bindValue(":id", acc.id());
  q.bindValue(":institutionId", acc.institutionId());
  q.bindValue(":parentId", acc.parentAccountId());
  q.bindValue(":lastReconciled", acc.lastReconciliationDate());
  q.bindValue(":lastModified", acc.lastModified());
  q.bindValue(":openingDate", acc.openingDate());
  q.bindValue(":accountNumber", acc.number());
  q.bindValue(":accountType", acc.accountType());
  q.bindValue(":accountTypeString", KMyMoneyUtils::accountTypeToString(acc.accountType()));
  if (acc.accountType() == MyMoneyAccount::Stock) {
    q.bindValue(":isStockAccount", "Y");
  } else {
    q.bindValue(":isStockAccount", "N");
  }
  q.bindValue(":accountName", acc.name());
  q.bindValue(":description", acc.description());
  q.bindValue(":currencyId", acc.currencyId());
  IMyMoneyStorage *i = MyMoneyFile::instance()->storage();
  q.bindValue(":balance", i->balance(acc.id(), QDate()).toString());
  q.bindValue(":balanceFormatted", i->balance(acc.id(), QDate()).formatMoney());
  if (!q.exec()) throw buildError(q, QString("writing Account"));

  //Add in Key-Value Pairs for accounts.
  writeKeyValuePairs("ACCOUNT", acc.id(), acc.pairs());
}

// **** Transactions and Splits ****
void MyMoneyStorageSql::writeTransactions() {
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  QSqlQuery q(this);
  q.prepare("SELECT id FROM kmmTransactions WHERE txType = 'N';");
  if (!q.exec()) throw buildError (q, "building Transaction list");
  while (q.next()) dbList.append(q.value(0).toString());

  MyMoneyTransactionFilter filter;
  const QValueList<MyMoneyTransaction> list = m_storage->transactionList(filter);
  signalProgress(0, list.count(), "Writing Transactions...");
  QValueList<MyMoneyTransaction>::ConstIterator it;
  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i) {
    if (dbList.contains((*it).id())) {
      q.prepare (m_db.m_tables["kmmTransactions"].updateString());
      dbList.remove ((*it).id());
    } else {
      q.prepare (m_db.m_tables["kmmTransactions"].insertString());
    }
    writeTransaction((*it).id(), *it, q, "N");
    m_hiIdTransactions = calcHighId(m_hiIdTransactions, (*it).id());
    signalProgress(++m_transactions, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      deleteTransaction(*it);
      ++it;
    }
  }
}

void MyMoneyStorageSql::deleteTransaction(const QString& id) {
  QSqlQuery q(this);
  q.prepare("DELETE FROM kmmTransactions WHERE id = :id");
  q.bindValue(":id", id);
  if (!q.exec()) throw buildError(q, "deleting Transaction");
  q.prepare("DELETE FROM kmmSplits WHERE transactionId = :id");
  q.bindValue(":id", id);
  if (!q.exec()) throw buildError(q, "deleting Splits");
  deleteKeyValuePairs("TRANSACTION", id);
}

void MyMoneyStorageSql::writeTransaction(const QString& txId, const MyMoneyTransaction& tx, QSqlQuery& q, const QString& type) {
  q.bindValue(":id", txId);
  q.bindValue(":txType", type);
  q.bindValue(":postDate", tx.postDate());
  q.bindValue(":memo", tx.memo());
  q.bindValue(":entryDate", tx.entryDate());
  q.bindValue(":currencyId", tx.commodity());
  q.bindValue(":bankId", tx.bankID());
  if (!q.exec()) throw buildError(q, QString("writing Transaction"));

  QValueList<MyMoneySplit> splitList = tx.splits();
  writeSplits(txId, type, splitList);

  //Add in Key-Value Pairs for transactions.
  writeKeyValuePairs("TRANSACTION", txId, tx.pairs());
}

void MyMoneyStorageSql::writeSplits(const QString& txId, const QString& type, const QValueList<MyMoneySplit>& splitList) {
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<unsigned int> dbList;
  QSqlQuery q(this);
  q.prepare("SELECT splitId FROM kmmSplits where transactionId = :id;");
  q.bindValue(":id", txId);
  if (!q.exec()) throw buildError (q, "building Split list");
  while (q.next()) dbList.append(q.value(0).toUInt());

  QValueList<MyMoneySplit>::const_iterator it;
  unsigned int i;
  for(it = splitList.begin(), i = 0; it != splitList.end(); ++it, ++i) {
    if (dbList.contains(i)) {
      q.prepare (m_db.m_tables["kmmSplits"].updateString());
      dbList.remove (i);
    } else {
      q.prepare (m_db.m_tables["kmmSplits"].insertString());
    }
    writeSplit(txId, (*it), type, i, q);
    m_splits++;
  }

  if (!dbList.isEmpty()) {
    QValueList<unsigned int>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare("DELETE FROM kmmSplits WHERE transactionId = :txId AND splitId = :splitId");
      q.bindValue(":txId", txId);
      q.bindValue(":splitId", *it);
      if (!q.exec()) throw buildError(q, "deleting Splits");
      ++it;
    }
  }
}

void MyMoneyStorageSql::writeSplit(const QString& txId, const MyMoneySplit& split,
                                   const QString& type, const int splitId, QSqlQuery& q) {
  q.bindValue(":transactionId", txId);
  q.bindValue(":txType", type);
  q.bindValue(":splitId", splitId);
  q.bindValue(":payeeId", split.payeeId());
  q.bindValue(":reconcileDate", split.reconcileDate());
  q.bindValue(":action", split.action());
  q.bindValue(":reconcileFlag", split.reconcileFlag());
  q.bindValue(":value", split.value().toString());
  q.bindValue(":valueFormatted", split.value().formatMoney());
  q.bindValue(":shares", split.shares().toString());
  q.bindValue(":sharesFormatted", split.shares().formatMoney());
  q.bindValue(":memo", split.memo());
  q.bindValue(":accountId", split.accountId());
  q.bindValue(":checkNumber", split.number());
  if (!q.exec()) throw buildError(q, QString("writing Split"));
}

// **** Schedules ****
void MyMoneyStorageSql::writeSchedules() {
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  QSqlQuery q(this);
  q.prepare("SELECT id FROM kmmSchedules;");
  if (!q.exec()) throw buildError (q, "building Schedule list");
  while (q.next()) dbList.append(q.value(0).toString());

  const QValueList<MyMoneySchedule> list = m_storage->scheduleList();
  QValueList<MyMoneySchedule>::ConstIterator it;
  signalProgress(0, list.count(), "Writing Schedules...");
  for(it = list.begin(); it != list.end(); ++it) {
    bool insert = true;
    if (dbList.contains((*it).id())) {
      q.prepare (m_db.m_tables["kmmSchedules"].updateString());
      dbList.remove ((*it).id());
      insert = false;
    } else {
      q.prepare (m_db.m_tables["kmmSchedules"].insertString());
    }
    writeSchedule(*it, q);
    m_hiIdSchedules = calcHighId(m_hiIdSchedules, (*it).id());
    signalProgress(++m_schedules, 0);

    //store the transaction data for this task. q will be insert or update, as for schedule itself
    if (!insert) {
      q.prepare (m_db.m_tables["kmmTransactions"].updateString());
    } else {
      q.prepare (m_db.m_tables["kmmTransactions"].insertString());
    }
    writeTransaction((*it).id(), (*it).transaction(), q, "S");
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare("DELETE FROM kmmSchedules WHERE id = :id");
      q.bindValue(":id", (*it));
      if (!q.exec()) throw buildError(q, "deleting Schedule");
      q.prepare("DELETE FROM kmmSchedulePaymentHistory WHERE schedId = :id");
      q.bindValue(":id", (*it));
      if (!q.exec()) throw buildError(q, "deleting Schedule Payment History");
      deleteTransaction(*it);
      ++it;
    }
  }
}

void MyMoneyStorageSql::writeSchedule(const MyMoneySchedule& sch, QSqlQuery& q) {
  q.bindValue(":id", sch.id());
  q.bindValue(":name", sch.name());
  q.bindValue(":type", sch.type());
  q.bindValue(":typeString", KMyMoneyUtils::scheduleTypeToString(sch.type()));
  q.bindValue(":occurence", sch.occurence());
  q.bindValue(":occurenceString", KMyMoneyUtils::occurenceToString(sch.occurence()));
  q.bindValue(":paymentType", sch.paymentType());
  q.bindValue(":paymentTypeString", KMyMoneyUtils::paymentMethodToString(sch.paymentType()));
  q.bindValue(":startDate", sch.startDate());
  q.bindValue(":endDate", sch.endDate());
  if (sch.isFixed()) {
    q.bindValue(":fixed", "Y");
  } else {
    q.bindValue(":fixed", "N");
  }
  if (sch.autoEnter()) {
    q.bindValue(":autoEnter", "Y");
  } else {
    q.bindValue(":autoEnter", "N");
  }
  q.bindValue(":lastPayment", sch.lastPayment());
  q.bindValue(":nextPaymentDue", sch.nextPayment(sch.lastPayment()));
  q.bindValue(":weekendOption", sch.weekendOption());
  q.bindValue(":weekendOptionString", ""); // weekendOptionToString?
  if (!q.exec()) throw buildError(q, QString("writing Schedules"));

  //store the payment history for this scheduled task.
  //easiest way is to delete all and re-insert; it's not a high use table
  q.prepare("DELETE FROM kmmSchedulePaymentHistory WHERE schedId = :id;");
  q.bindValue(":id", sch.id());
  if (!q.exec()) throw buildError(q, QString("deleting  Schedule Payment History"));

  QValueList<QDate> payments = sch.recordedPayments();
  QValueList<QDate>::Iterator it;
  for (it=payments.begin(); it!=payments.end(); ++it) {
    q.prepare (m_db.m_tables["kmmSchedulePaymentHistory"].insertString());
    q.bindValue(":schedId", sch.id());
    q.bindValue(":payDate", (*it));
    if (!q.exec()) throw buildError(q, QString("writing Schedule Payment History"));
  }
}

// **** Securities ****
void MyMoneyStorageSql::writeSecurities() {
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  QSqlQuery q(this);
  q.prepare("SELECT id FROM kmmSecurities;");
  if (!q.exec()) throw buildError (q, "building security list");
  while (q.next()) dbList.append(q.value(0).toString());

  const QValueList<MyMoneySecurity> securityList = m_storage->securityList();
  signalProgress(0, securityList.count(), "Writing Securities...");
  for(QValueList<MyMoneySecurity>::ConstIterator it = securityList.begin(); it != securityList.end(); ++it) {
    if (dbList.contains((*it).id())) {
      q.prepare (m_db.m_tables["kmmSecurities"].updateString());
      dbList.remove ((*it).id());
    } else {
      q.prepare (m_db.m_tables["kmmSecurities"].insertString());
    }
    writeSecurity((*it), q);
    m_hiIdSecurities = calcHighId(m_hiIdSecurities, (*it).id());
    signalProgress(++m_securities, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare("DELETE FROM kmmSecurities WHERE id = :id");
      q.bindValue(":id", (*it));
      if (!q.exec()) throw buildError(q, "deleting Security");
      q.prepare("DELETE FROM kmmPrices WHERE fromId = :id OR toId = :id");
      q.bindValue(":fromId", (*it));
      q.bindValue(":toId", (*it));
      if (!q.exec()) throw buildError(q, "deleting Security");
      deleteKeyValuePairs("SECURITY", (*it));
       ++it;
    }
  }
}

void MyMoneyStorageSql::writeSecurity(const MyMoneySecurity& security, QSqlQuery& q) {
  q.bindValue(":id", security.id());
  q.bindValue(":name", security.name());
  q.bindValue(":symbol", security.tradingSymbol());
  q.bindValue(":type", static_cast<int>(security.securityType()));
  q.bindValue(":typeString", KMyMoneyUtils::securityTypeToString(security.securityType()));
  q.bindValue(":smallestAccountFraction", security.smallestAccountFraction());
  q.bindValue(":tradingCurrency", security.tradingCurrency());
  q.bindValue(":tradingMarket", security.tradingMarket());
  if (!q.exec()) throw buildError(q, QString ("writing Securities"));

  //Add in Key-Value Pairs for security
  writeKeyValuePairs("SECURITY", security.id(), security.pairs());
}

// **** Prices ****
void MyMoneyStorageSql::writePrices() {
  // due to difficulties in matching and determining deletes
  //easiest way is to delete all and re-insert
  QSqlQuery q(this);
  q.prepare("DELETE FROM kmmPrices");
  if (!q.exec()) throw buildError(q, QString("deleting Prices"));

  const MyMoneyPriceList list = m_storage->priceList();
  signalProgress(0, list.count(), "Writing Prices...");
  MyMoneyPriceList::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it)   {
    writePricePair(it.key().first, it.key().second, *it);
  }
}

void MyMoneyStorageSql::writePricePair(const QString& from, const QString& to,
    const MyMoneyPriceEntries& p) {
  MyMoneyPriceEntries::ConstIterator it;
  for(it = p.begin(); it != p.end(); ++it) {
    writePrice (from, to, *it);
    signalProgress(++m_prices, 0);
  }
}

void MyMoneyStorageSql::writePrice(const QString& from, const QString& to, const MyMoneyPrice& p) {
  QSqlQuery q(this);
  q.prepare (m_db.m_tables["kmmPrices"].insertString());
  q.bindValue(":fromId", from);
  q.bindValue(":toId", to);
  q.bindValue(":priceDate", p.date());
  q.bindValue(":price", p.rate().toString());
  q.bindValue(":priceFormatted", p.rate().formatMoney());
  q.bindValue(":priceSource", p.source());
  if (!q.exec()) throw buildError(q, QString("writing Prices"));
}

// **** Currencies ****
void MyMoneyStorageSql::writeCurrencies() {
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  QSqlQuery q(this);
  q.prepare("SELECT ISOCode FROM kmmCurrencies;");
  if (!q.exec()) throw buildError (q, "building Currency list");
  while (q.next()) dbList.append(q.value(0).toString());

  const QValueList<MyMoneySecurity> currencyList = m_storage->currencyList();
  signalProgress(0, currencyList.count(), "Writing Currencies...");
  for(QValueList<MyMoneySecurity>::ConstIterator it = currencyList.begin(); it != currencyList.end(); ++it) {
    if (dbList.contains((*it).id())) {
      q.prepare (m_db.m_tables["kmmCurrencies"].updateString());
      dbList.remove ((*it).id());
    } else {
      q.prepare (m_db.m_tables["kmmCurrencies"].insertString());
    }
    writeCurrency((*it), q);
    signalProgress(++m_currencies, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare("DELETE FROM kmmCurrencies WHERE ISOCode = :ISOCode");
      q.bindValue(":ISOCode", (*it));
      if (!q.exec()) throw buildError(q, "deleting Currency");
      ++it;
    }
  }
}

void MyMoneyStorageSql::writeCurrency(const MyMoneySecurity& currency, QSqlQuery& q) {
  q.bindValue(":ISOcode", currency.id());
  q.bindValue(":name", currency.name());
  q.bindValue(":type", static_cast<int>(currency.securityType()));
  q.bindValue(":typeString", KMyMoneyUtils::securityTypeToString(currency.securityType()));
  // writing the symbol as three short ints is a PITA, but the
  // problem is that database drivers have incompatible ways of declaring UTF8
  QString symbol = currency.tradingSymbol() + "   ";
  q.bindValue(":symbol1", symbol.mid(0,1).unicode()->unicode());
  q.bindValue(":symbol2", symbol.mid(1,1).unicode()->unicode());
  q.bindValue(":symbol3", symbol.mid(2,1).unicode()->unicode());
  q.bindValue(":symbolString", symbol);
  q.bindValue(":partsPerUnit", currency.partsPerUnit());
  q.bindValue(":smallestCashFraction", currency.smallestCashFraction());
  q.bindValue(":smallestAccountFraction", currency.smallestAccountFraction());
  if (!q.exec()) throw buildError(q, QString("writing Currencies"));
}

void MyMoneyStorageSql::writeReports() {
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  QSqlQuery q(this);
  q.prepare("SELECT name FROM kmmReportConfig;");
  if (!q.exec()) throw buildError (q, "building Report list");
  while (q.next()) dbList.append(q.value(0).toString());

  QValueList<MyMoneyReport> list = m_storage->reportList();
  signalProgress(0, list.count(), "Writing Reports...");
  QValueList<MyMoneyReport>::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it){
    if (dbList.contains((*it).name())) {
      q.prepare (m_db.m_tables["kmmReportConfig"].updateString());
      dbList.remove ((*it).name());
    } else {
      q.prepare (m_db.m_tables["kmmReportConfig"].insertString());
    }
    QDomDocument d; // create a dummy XML document
    QDomElement e = d.createElement("REPORTS");
    d.appendChild (e);
    (*it).writeXML(d, e); // write the XML to document
    q.bindValue(":name", (*it).name());
    q.bindValue(":XML", d.toString());
    if (!q.exec()) throw buildError(q, QString("writing Reports"));
    signalProgress(++m_reports, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare("DELETE FROM kmmReportConfig WHERE name = :name");
      q.bindValue(":name", (*it));
      if (!q.exec()) throw buildError(q, "deleting Report");
      ++it;
    }
  }
}

void MyMoneyStorageSql::writeFileInfo() {
  QSqlQuery q(this);
  q.prepare ("SELECT * FROM kmmFileInfo;");
  if (!q.exec()) throw buildError(q, "checking fileinfo");
  QString qs;
  if (q.next())
    qs = m_db.m_tables["kmmFileInfo"].updateString();
  else
    qs = (m_db.m_tables["kmmFileInfo"].insertString());
  q.prepare(qs);
  q.bindValue(":version", QString("%1.%2").arg(m_majorVersion).arg(m_minorVersion));
  q.bindValue(":created", m_storage->creationDate());
  q.bindValue(":lastModified", m_storage->lastModificationDate());
  q.bindValue(":baseCurrency", m_storage->pairs()["kmm-baseCurrency"]);
  q.bindValue(":institutions", m_institutions);
  q.bindValue(":accounts", m_accounts);
  q.bindValue(":payees", m_payees);
  q.bindValue(":transactions", m_transactions);
  q.bindValue(":splits", m_splits);
  q.bindValue(":securities", m_securities);
  q.bindValue(":prices", m_prices);
  q.bindValue(":currencies", m_currencies);
  q.bindValue(":schedules", m_schedules);
  q.bindValue(":reports", m_reports);
  q.bindValue(":kvps", m_kvps);
  q.bindValue(":dateRangeStart", QDate());
  q.bindValue(":dateRangeEnd", QDate());
  q.bindValue(":hiInstitutionId", (m_storage->institutionId() > m_hiIdInstitutions ? m_storage->institutionId() : m_hiIdInstitutions));
  q.bindValue(":hiPayeeId", (m_storage->payeeId() > m_hiIdPayees ? m_storage->payeeId() : m_hiIdPayees));
  q.bindValue(":hiAccountId", (m_storage->accountId() > m_hiIdAccounts ? m_storage->accountId() : m_hiIdAccounts));
  q.bindValue(":hiTransactionId", (m_storage->transactionId() > m_hiIdTransactions ? m_storage->transactionId() : m_hiIdTransactions));
  q.bindValue(":hiScheduleId", (m_storage->scheduleId() > m_hiIdSchedules ? m_storage->scheduleId() : m_hiIdSchedules));
  q.bindValue(":hiSecurityId", (m_storage->securityId() > m_hiIdSecurities ? m_storage->securityId() : m_hiIdSecurities));
  q.bindValue(":hiReportId", (m_storage->reportId() > m_hiIdReports ? m_storage->reportId() : m_hiIdReports));
  q.bindValue(":encryptData", m_encryptData);
  q.bindValue(":updateInProgress", "N");
  if (!q.exec()) throw buildError(q, QString("writing FileInfo"));
  //
  writeKeyValuePairs("STORAGE", "", m_storage->pairs());
}

// **** Key/value pairs ****
void MyMoneyStorageSql::writeKeyValuePairs(const QString& kvpType, const QString& kvpId, const QMap<QCString,  QString>& pairs) {
  QMap<QCString, QString>::const_iterator it;
  for(it = pairs.begin(); it != pairs.end(); ++it) {
    writeKeyValuePair (kvpType, kvpId, it.key(), it.data());
  }
}

void MyMoneyStorageSql::writeKeyValuePair (const QString& kvpType, const QString& kvpId, const QString& kvpKey, const QString& kvpData) {
  QSqlQuery q(this);
  q.prepare (m_db.m_tables["kmmKeyValuePairs"].insertString());
  q.bindValue(":kvpType", kvpType);
  q.bindValue(":kvpId", kvpId);
  q.bindValue(":kvpKey", kvpKey);
  q.bindValue(":kvpData", kvpData);
  if (!q.exec()) throw buildError(q, QString("writing KVP"));
  m_kvps++;
}

void MyMoneyStorageSql::deleteKeyValuePairs (const QString& kvpType, const QString& kvpId) {
  QSqlQuery q(this);
  q.prepare("DELETE FROM kmmKeyValuePairs WHERE kvpType = :kvpType AND kvpId = :kvpId;");
  q.bindValue(":kvpType", kvpType);
  //qDebug("bound %s", kvpType.latin1());
  q.bindValue(":kvpId", kvpId);
  //qDebug("bound %s", kvpId.latin1());
  if (!q.exec()) throw buildError(q, "deleting kvp for %1 %2").arg(kvpType).arg(kvpId);
}
//******************************** read SQL routines **************************************
#define CASE(a) if ((*ft).name() == #a)
#define GETSTRING q.value(i).toString()
#define GETCSTRING q.value(i).toCString()
#define GETDATE q.value(i).toDate()
#define GETINT q.value(i).toInt()
#define GETULL q.value(i).toULongLong()

void MyMoneyStorageSql::readFileInfo(void) {
  signalProgress(0, 18, QObject::tr("Loading file information..."));
  MyMoneyDbTable t = m_db.m_tables["kmmFileInfo"];
  QSqlQuery q(this);
  q.prepare (t.selectAllString());
  if (!q.exec()) throw buildError(q, QString("reading FileInfo"));
  if (!q.next()) throw buildError(q, QString("retrieving FileInfo"));
  QValueList<MyMoneyDbField>::const_iterator ft = t.begin();
  int i = 0;
  while (ft != t.end()) {
    CASE(version)  ;// check version == current version...
    CASE(created) m_storage->setCreationDate(QDate::fromString(GETSTRING, Qt::ISODate));
    CASE(lastModified) m_storage->setLastModificationDate(QDate::fromString(GETSTRING, Qt::ISODate));
    CASE(hiInstitutionId) m_storage->loadInstitutionId(GETINT);
    CASE(hiPayeeId) m_storage->loadPayeeId(GETINT);
    CASE(hiAccountId) m_storage->loadAccountId(GETINT);
    CASE(hiTransactionId) m_storage->loadTransactionId(GETINT);
    CASE(hiScheduleId) m_storage->loadScheduleId(GETINT);
    CASE(hiSecurityId) m_storage->loadSecurityId(GETINT);
    CASE(hiReportId  ) m_storage->loadReportId(GETINT);
    CASE(institutions) m_institutions = GETULL;
    CASE(accounts    ) m_accounts = GETULL;
    CASE(payees      ) m_payees = GETULL;
    CASE(transactions) m_transactions = GETULL;
    CASE(splits      ) m_splits = GETULL;
    CASE(securities  ) m_securities = GETULL;
    CASE(currencies  ) m_currencies = GETULL;
    CASE(schedules   ) m_schedules = GETULL;
    CASE(prices      ) m_prices = GETULL;
    CASE(kvps        ) m_kvps = GETULL;
    CASE(encryptData) m_encryptData = GETSTRING;
    ++ft; ++i;
    signalProgress(i,0);
  }
  m_storage->setPairs(readKeyValuePairs("STORAGE", "").pairs());
}

void MyMoneyStorageSql::readInstitutions(void) {
  signalProgress(0, m_institutions, QObject::tr("Loading institutions..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmInstitutions"];
  QSqlQuery q(this);
  q.prepare (t.selectAllString());
  if (!q.exec()) throw buildError(q, QString("reading Institution"));
  while (q.next()) {
    QValueList<MyMoneyDbField>::const_iterator ft = t.begin();
    int i = 0;
    QString iid;
    MyMoneyInstitution inst;
    while (ft != t.end()) {
      CASE(id) iid = GETSTRING;
      CASE(name) inst.setName(GETSTRING);
      CASE(manager) inst.setManager(GETSTRING);
      CASE(routingCode) inst.setSortcode(GETSTRING);
      CASE(addressStreet) inst.setStreet(GETSTRING);
      CASE(addressCity) inst.setCity(GETSTRING);
      CASE(addressZipcode) inst.setPostcode(GETSTRING);
      CASE(telephone)  inst.setTelephone(GETSTRING);
      ++ft; ++i;
    }
    // get list of subaccounts
    QSqlQuery sq(this);
    sq.prepare (QString("SELECT id from kmmAccounts where institutionId = :id"));
    sq.bindValue(":id", iid);
    if (!sq.exec()) throw buildError(q, QString("reading Institution AccountList"));
    QStringList aList;
    while (sq.next()) aList.append(sq.value(0).toString());
    for (QStringList::Iterator it = aList.begin(); it != aList.end(); ++it)
      inst.addAccountId(QCString(*it));

    m_storage->loadInstitution(MyMoneyInstitution(QCString(iid), inst));
    signalProgress (++progress, 0);
  }
}

void MyMoneyStorageSql::readPayees(void) {
  signalProgress(0, m_payees, QObject::tr("Loading payees..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmPayees"];
  QSqlQuery q(this);
  q.prepare (t.selectAllString());
  if (!q.exec()) throw buildError(q, QString("reading Payee"));
  while (q.next()) {
    QValueList<MyMoneyDbField>::const_iterator ft = t.begin();
    int i = 0;
    QCString pid;
    MyMoneyPayee payee;
    while (ft != t.end()) {
      CASE(id) pid = GETCSTRING;
      CASE(name) payee.setName(GETSTRING);
      CASE(reference) payee.setReference(GETSTRING);
      CASE(email) payee.setEmail(GETSTRING);
      CASE(addressStreet) payee.setAddress(GETSTRING);
      CASE(addressCity) payee.setCity(GETSTRING);
      CASE(addressZipcode) payee.setPostcode(GETSTRING);
      CASE(addressState) payee.setState(GETSTRING);
      CASE(telephone) payee.setTelephone(GETSTRING);
      ++ft; ++i;
    }
    if (pid == "USER") {
      m_storage->setUser(payee);
    } else {
      m_storage->loadPayee(MyMoneyPayee(QCString(pid), payee));
    }
    signalProgress(++progress, 0);
  }
}

void MyMoneyStorageSql::readAccounts(void) {
  signalProgress(0, m_accounts, QObject::tr("Loading accounts..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmAccounts"];
  QSqlQuery q(this);
  q.prepare (t.selectAllString());
  if (!q.exec()) throw buildError(q, QString("reading Account"));
  while (q.next()) {
    QValueList<MyMoneyDbField>::const_iterator ft = t.begin();
    int i = 0;
    QCString aid;
    MyMoneyAccount acc;
    while (ft != t.end()) {
      CASE(id) aid = GETCSTRING;
      CASE(institutionId) acc.setInstitutionId(GETCSTRING);
      CASE(parentId) acc.setParentAccountId(GETCSTRING);
      CASE(lastReconciled) acc.setLastReconciliationDate(GETDATE);
      CASE(lastModified) acc.setLastModified(GETDATE);
      CASE(openingDate) acc.setOpeningDate(GETDATE);
      CASE(accountNumber) acc.setNumber(GETSTRING);
      CASE(accountType) acc.setAccountType(static_cast<MyMoneyAccount::accountTypeE>(GETINT));
      CASE(accountName) acc.setName(GETSTRING);
      CASE(description) acc.setDescription(GETSTRING);
      CASE(currencyId) acc.setCurrencyId(GETCSTRING);
      ++ft; ++i;
    }
  // get list of subaccounts
    QSqlQuery sq(this);
    sq.prepare (QString("SELECT id from kmmAccounts where parentId = :id"));
    sq.bindValue(":id", aid);
    if (!sq.exec()) throw buildError(q, QString("reading subAccountList"));
    QStringList aList;
    while (sq.next()) aList.append(sq.value(0).toString());
    for (QStringList::Iterator it = aList.begin(); it != aList.end(); ++it)
      acc.addAccountId(QCString(*it));

    // Process any key value pair
    acc.setPairs(readKeyValuePairs("ACCOUNT", aid).pairs());

    m_storage->loadAccount(MyMoneyAccount(QCString(aid), acc));
    signalProgress(++progress, 0);
  }
}

void MyMoneyStorageSql::readTransactions(void) {
  signalProgress(0, m_transactions, QObject::tr("Loading transactions..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmTransactions"];
  QSqlQuery q(this);
  q.prepare (QString(t.selectAllString(false) + " WHERE txType = 'N' ORDER BY id;"));
  if (!q.exec()) throw buildError(q, QString("reading Transaction"));
  MyMoneyDbTable ts = m_db.m_tables["kmmSplits"];
  QSqlQuery qs(this);
  qs.prepare (QString(ts.selectAllString(false) + " WHERE txType = 'N' ORDER BY transactionId, splitId;"));
  if (!qs.exec()) throw buildError(qs, QString("reading Splits"));
  QString splitTxId = "ZZZ";
  MyMoneySplit s;
  if (qs.next()) {
    splitTxId = qs.value(0).toString();
    readSplit (s, qs, ts);
  } else {
    splitTxId = "ZZZ";
  }
  while (q.next()) {
    MyMoneyTransaction tx;
    QString txId;
    tx.setId(QCString(txId));
    QValueList<MyMoneyDbField>::const_iterator ft = t.begin();
    int i = 0;
    while (ft != t.end()) {
      CASE(id) txId = GETSTRING;
      CASE(postDate) tx.setPostDate(GETDATE);
      CASE(memo) tx.setMemo(GETSTRING);
      CASE(entryDate) tx.setEntryDate(GETDATE);
      CASE(currencyId) tx.setCommodity(GETCSTRING);
      CASE(bankId) tx.setBankID(GETSTRING);
      ++ft; ++i;
    }
    while (txId == splitTxId) {
      tx.addSplit (s);
      if (qs.next()) {
      splitTxId = qs.value(0).toString();
      readSplit (s, qs, ts);
      } else {
        splitTxId = "ZZZ";
      }
    }
  // Process any key value pair
    tx.setPairs(readKeyValuePairs("TRANSACTION", txId).pairs());
    m_storage->loadTransaction(MyMoneyTransaction(QCString(txId), tx));
    signalProgress(++progress, 0);
  }
}

void MyMoneyStorageSql::readSplit (MyMoneySplit& s, const QSqlQuery& q, const MyMoneyDbTable& t) {
  s.setId(QCString(""));
  QValueList<MyMoneyDbField>::const_iterator ft = t.begin();
  int i = 0;
  while (ft != t.end()) {
    CASE(payeeId) s.setPayeeId(GETCSTRING);
    CASE(reconcileDate) s.setReconcileDate(GETDATE);
    CASE(action) s.setAction(GETCSTRING);
    CASE(reconcileFlag) s.setReconcileFlag(static_cast<MyMoneySplit::reconcileFlagE>(GETINT));
    CASE(value) s.setValue(MyMoneyMoney(GETSTRING));
    CASE(shares) s.setShares(MyMoneyMoney(GETSTRING));
    CASE(memo) s.setMemo(GETSTRING);
    CASE(accountId) s.setAccountId(GETCSTRING);
    CASE(checkNumber) s.setNumber(GETSTRING);
    ++ft; ++i;
  }
  return;
}

void MyMoneyStorageSql::readSchedules(void) {
  signalProgress(0, m_schedules, QObject::tr("Loading schedules..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmSchedules"];
  QSqlQuery q(this);
  q.prepare (QString(t.selectAllString(false) + " ORDER BY id;"));
  if (!q.exec()) throw buildError(q, QString("reading Schedules"));
  while (q.next()) {
    QValueList<MyMoneyDbField>::const_iterator ft = t.begin();
    int i = 0;
    MyMoneySchedule s;
    QString boolChar;
    while (ft != t.end()) {
      CASE(id) s.setId(GETCSTRING);
      CASE(name)  s.setName (GETSTRING);
      CASE(type)  s.setType (static_cast<MyMoneySchedule::typeE>(GETINT));
      CASE(occurence)  s.setOccurence (static_cast<MyMoneySchedule::occurenceE>(GETINT));
      CASE(paymentType)  s.setPaymentType (static_cast<MyMoneySchedule::paymentTypeE>(GETINT));
      CASE(startDate)  s.setStartDate (GETDATE);
      CASE(endDate)  s.setEndDate (GETDATE);
      CASE(fixed) {boolChar = GETSTRING; s.setFixed (boolChar == "Y");}
      CASE(autoEnter)  {boolChar = GETSTRING; s.setAutoEnter (boolChar == "Y");}
      CASE(lastPayment)  s.setLastPayment (GETDATE);
      CASE(weekendOption)  s.setWeekendOption (static_cast<MyMoneySchedule::weekendOptionE>(GETINT));
      ++ft; ++i;
    }
    // read the associated transaction
    MyMoneyDbTable t = m_db.m_tables["kmmTransactions"];
    QSqlQuery q(this);
    q.prepare (QString(t.selectAllString(false) + " WHERE id = :id;"));
    q.bindValue(":id", s.id());
    if (!q.exec()) throw buildError(q, QString("reading Scheduled Transaction"));
    if (!q.next()) throw buildError (q, QString("retrieving scheduled transaction"));
    MyMoneyTransaction tx;
    tx.setId(s.id());
    ft = t.begin();
    i = 0;
    while (ft != t.end()) {
      CASE(postDate) tx.setPostDate(GETDATE);
      CASE(memo) tx.setMemo(GETSTRING);
      CASE(entryDate) tx.setEntryDate(GETDATE);
      CASE(currencyId) tx.setCommodity(GETCSTRING);
      CASE(bankId) tx.setBankID(GETSTRING);
      ++ft; ++i;
    }

    MyMoneyDbTable ts = m_db.m_tables["kmmSplits"];
    QSqlQuery qs(this);
    qs.prepare (QString(ts.selectAllString(false) + " WHERE transactionId = :id ORDER BY splitId;"));
    qs.bindValue(":id", s.id());
    if (!qs.exec()) throw buildError(qs, QString("reading Scheduled Splits"));
    while (qs.next()) {
      MyMoneySplit sp;
      readSplit (sp, qs, ts);
      tx.addSplit (sp);
    }
    // Process any key value pair
    tx.setPairs(readKeyValuePairs("TRANSACTION", s.id()).pairs());
    s.setTransaction(tx);

    // read in the recorded payments
    QSqlQuery sq(this);
    sq.prepare (QString("SELECT payDate from kmmSchedulePaymentHistory where schedId = :id"));
    sq.bindValue(":id", s.id());
    if (!sq.exec()) throw buildError(q, QString("reading schedule payment history"));
    while (sq.next()) s.recordPayment (sq.value(0).toDate());

    m_storage->loadSchedule(s);
    signalProgress(++progress, 0);
  }
}

void MyMoneyStorageSql::readSecurities(void) {
  signalProgress(0, m_securities, QObject::tr("Loading securities..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmSecurities"];
  QSqlQuery q(this);
  q.prepare (QString(t.selectAllString(false) + " ORDER BY id;"));
  if (!q.exec()) throw buildError(q, QString("reading Securities"));
  while (q.next()) {
    QValueList<MyMoneyDbField>::const_iterator ft = t.begin();
    int i = 0;
    MyMoneySecurity e;
    QCString eid;
    int saf;
    while (ft != t.end()) {
      CASE(id) eid = GETSTRING;
      CASE(name) e.setName(GETSTRING);
      CASE(symbol) e.setTradingSymbol(GETSTRING);
      CASE(type) e.setSecurityType(static_cast<MyMoneySecurity::eSECURITYTYPE>(GETINT));
      CASE(smallestAccountFraction) saf = GETINT;
      CASE(tradingCurrency) e.setTradingCurrency(GETCSTRING);
      CASE(tradingMarket) e.setTradingMarket(GETSTRING);
      ++ft; ++i;
    }
    if(e.tradingCurrency().isEmpty())
      e.setTradingCurrency(QCString(m_storage->pairs()["kmm-baseCurrency"]));
    if(saf == 0)
      saf = 100;
    e.setSmallestAccountFraction(saf);

  // Process any key value pairs
    e.setPairs(readKeyValuePairs("SECURITY", eid).pairs());
  //tell the storage objects we have a new security object.
    m_storage->loadSecurity(MyMoneySecurity(eid,e));
    signalProgress(++progress, 0);
  }
}

void MyMoneyStorageSql::readPrices(void) {
  signalProgress(0, m_prices, QObject::tr("Loading prices..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmPrices"];
  QSqlQuery q(this);
  q.prepare (t.selectAllString());
  if (!q.exec()) throw buildError(q, QString("reading Prices"));
  while (q.next()) {
    QValueList<MyMoneyDbField>::const_iterator ft = t.begin();
    int i = 0;
    QCString from;
    QCString to;
    QDate date;
    MyMoneyMoney rate;
    QString source;
    while (ft != t.end()) {
      CASE(fromId) from = GETCSTRING;
      CASE(toId) to = GETCSTRING;
      CASE(priceDate) date = GETDATE;
      CASE(price) rate = GETSTRING;
      CASE(priceSource) source = GETSTRING;
      ++ft; ++i;
    }
    m_storage->addPrice(MyMoneyPrice(from, to,  date, rate, source));
    signalProgress(++progress, 0);
  }
}

void MyMoneyStorageSql::readCurrencies(void) {
  signalProgress(0, m_currencies, QObject::tr("Loading currencies..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmCurrencies"];
  QSqlQuery q(this);
  q.prepare (QString(t.selectAllString(false) + " ORDER BY ISOcode;"));
  if (!q.exec()) throw buildError(q, QString("reading Currencies"));
  while (q.next()) {
    QValueList<MyMoneyDbField>::const_iterator ft = t.begin();
    int i = 0;
    QCString id;
    MyMoneySecurity c;
    QChar symbol[3];
    while (ft != t.end()) {
      CASE(ISOcode) id = GETCSTRING;
      CASE(name) c.setName(GETSTRING);
      CASE(type) c.setSecurityType(static_cast<MyMoneySecurity::eSECURITYTYPE>(GETINT));
      CASE(symbol1) symbol[0] = QChar(GETINT);
      CASE(symbol2) symbol[1] = QChar(GETINT);
      CASE(symbol3) symbol[2] = QChar(GETINT);
      CASE(partsPerUnit) c.setPartsPerUnit(GETINT);
      CASE(smallestCashFraction) c.setSmallestCashFraction(GETINT);
      CASE(smallestAccountFraction) c.setSmallestAccountFraction(GETINT);
      ++ft; ++i;
    }
    c.setTradingSymbol(QString(symbol, 3).stripWhiteSpace());
    m_storage->loadCurrency(MyMoneySecurity(id, c));
    signalProgress(++progress, 0);
  }
}

void MyMoneyStorageSql::readReports(void) {
  signalProgress(0, m_reports, QObject::tr("Loading reports..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmReportConfig"];
  QSqlQuery q(this);
  q.prepare (QString(t.selectAllString(true)));
  if (!q.exec()) throw buildError(q, QString("reading reports"));
  while (q.next()) {
    QValueList<MyMoneyDbField>::const_iterator ft = t.begin();
    int i = 0;
    QDomDocument d;
    while (ft != t.end()) {
      CASE(XML) d.setContent(GETSTRING, false);
      ++ft; ++i;
    }
    QDomNode child = d.firstChild();
    child = child.firstChild();
    MyMoneyReport report;
    if (report.read(child.toElement())) m_storage->loadReport(report);
    signalProgress(++progress, 0);
  }
}

MyMoneyKeyValueContainer MyMoneyStorageSql::readKeyValuePairs (const QString kvpType, const QString& kvpId) {
  MyMoneyKeyValueContainer list;
  QSqlQuery q(this);
  q.prepare ("SELECT kvpKey, kvpData from kmmKeyValuePairs where kvpType = :type and kvpId = :id;");
  q.bindValue(":type", kvpType);
  q.bindValue(":id", kvpId);
  if (!q.exec()) throw buildError(q, QString("reading Kvp for %1 %2").arg(kvpType).arg(kvpId));
  while (q.next()) list.setValue(QCString(q.value(0).toString()), q.value(1).toString());
  return (list);
}

//****************************************************
const long long unsigned MyMoneyStorageSql::calcHighId
     (const long long unsigned i, const QString& id) {
  QString nid = id;
  long long unsigned high = nid.replace(QRegExp("[A-Z]*"), "").toULongLong();
  return (high > i ? high : i);
}

void MyMoneyStorageSql::setProgressCallback(void(*callback)(int, int, const QString&)) {
  m_progressCallback = callback;
}

void MyMoneyStorageSql::signalProgress(int current, int total, const QString& msg) {
  if(m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}

// **************************** Error display routine *******************************
QString& MyMoneyStorageSql::buildError (const QSqlQuery& q, const QString& message) {
  QString s = QString("Error in %1").arg(message);
  QSqlError e = lastError();
  s += QString ("\nDriver = %1, Host = %2, User = %3, Database = %4")
      .arg(driverName()).arg(hostName()).arg(userName()).arg(databaseName());
  s += QString ("\nDriver Error: %1").arg(e.driverText());
  s += QString ("\nDatabase Error No %1: %2").arg(e.number()).arg(e.databaseText());
  e = q.lastError();
  s += QString ("\nExecuted: %1").arg(q.executedQuery());
  s += QString ("\nQuery error No %1: %2").arg(e.number()).arg(e.text());
  m_error = s;
  return (m_error);
}

// ************************* Build table descriptions ****************************
MyMoneyDbDef::MyMoneyDbDef () {
  FileInfo();
  Institutions();
  Payees();
  Accounts();
  Transactions();
  Splits();
  KeyValuePairs();
  Schedules();
  SchedulePaymentHistory();
  Securities();
  Prices();
  Currencies();
  Reports();
}

/* PRIMARYKEY - this field is a unique key field on which the db will create an index
   NOTNULL - this field should never be null
   ISKEY - where there is no primary key, these fields can be used to uniquely identify a record
Default is that all 3 are false */

#define PRIMARYKEY true
#define NOTNULL true
#define ISKEY true

void MyMoneyDbDef::FileInfo(void){
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("version", "varchar(16)"));
  fields.append(MyMoneyDbField("created", "date"));
  fields.append(MyMoneyDbField("lastModified", "date"));
  fields.append(MyMoneyDbField("baseCurrency", "char(3)"));
  fields.append(MyMoneyDbField("institutions", "bigint unsigned"));
  fields.append(MyMoneyDbField("accounts", "bigint unsigned"));
  fields.append(MyMoneyDbField("payees", "bigint unsigned"));
  fields.append(MyMoneyDbField("transactions", "bigint unsigned"));
  fields.append(MyMoneyDbField("splits", "bigint unsigned"));
  fields.append(MyMoneyDbField("securities", "bigint unsigned"));
  fields.append(MyMoneyDbField("prices", "bigint unsigned"));
  fields.append(MyMoneyDbField("currencies", "bigint unsigned"));
  fields.append(MyMoneyDbField("schedules", "bigint unsigned"));
  fields.append(MyMoneyDbField("reports", "bigint unsigned"));
  fields.append(MyMoneyDbField("kvps", "bigint unsigned"));
  fields.append(MyMoneyDbField("dateRangeStart", "date"));
  fields.append(MyMoneyDbField("dateRangeEnd", "date"));
  fields.append(MyMoneyDbField("hiInstitutionId", "bigint unsigned"));
  fields.append(MyMoneyDbField("hiPayeeId", "bigint unsigned"));
  fields.append(MyMoneyDbField("hiAccountId", "bigint unsigned"));
  fields.append(MyMoneyDbField("hiTransactionId", "bigint unsigned"));
  fields.append(MyMoneyDbField("hiScheduleId", "bigint unsigned"));
  fields.append(MyMoneyDbField("hiSecurityId", "bigint unsigned"));
  fields.append(MyMoneyDbField("hiReportId", "bigint unsigned"));
  fields.append(MyMoneyDbField("encryptData", "varchar(255)"));
  fields.append(MyMoneyDbField("updateInProgress", "char(1)"));
  MyMoneyDbTable t("kmmFileInfo", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Institutions(void){
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("id", "varchar(32)", PRIMARYKEY, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("name", "text", false, NOTNULL));
  fields.append(MyMoneyDbField("manager", "text"));
  fields.append(MyMoneyDbField("routingCode", "text"));
  fields.append(MyMoneyDbField("addressStreet", "text"));
  fields.append(MyMoneyDbField("addressCity", "text"));
  fields.append(MyMoneyDbField("addressZipcode", "text"));
  fields.append(MyMoneyDbField("telephone", "text"));
  MyMoneyDbTable t("kmmInstitutions", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Payees(void){
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("id", "varchar(32)",  PRIMARYKEY, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("name", "text"));
  fields.append(MyMoneyDbField("reference", "text"));
  fields.append(MyMoneyDbField("email", "text"));
  fields.append(MyMoneyDbField("addressStreet", "text"));
  fields.append(MyMoneyDbField("addressCity", "text"));
  fields.append(MyMoneyDbField("addressZipcode", "text"));
  fields.append(MyMoneyDbField("addressState", "text"));
  fields.append(MyMoneyDbField("telephone", "text"));
  MyMoneyDbTable t("kmmPayees", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Accounts(void){
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("id", "varchar(32)",  PRIMARYKEY, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("institutionId", "varchar(32)"));
  fields.append(MyMoneyDbField("parentId", "varchar(32)"));
  fields.append(MyMoneyDbField("lastReconciled", "datetime"));
  fields.append(MyMoneyDbField("lastModified", "datetime"));
  fields.append(MyMoneyDbField("openingDate", "date"));
  fields.append(MyMoneyDbField("accountNumber", "text"));
  fields.append(MyMoneyDbField("accountType", "varchar(16)", false, NOTNULL));
  fields.append(MyMoneyDbField("accountTypeString", "text"));
  fields.append(MyMoneyDbField("isStockAccount", "char(1)"));
  fields.append(MyMoneyDbField("accountName", "text"));
  fields.append(MyMoneyDbField("description", "text"));
  fields.append(MyMoneyDbField("currencyId", "varchar(32)"));
  fields.append(MyMoneyDbField("balance", "text"));
  fields.append(MyMoneyDbField("balanceFormatted", "text"));
  MyMoneyDbTable t("kmmAccounts", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Transactions(void){
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("id", "varchar(32)", PRIMARYKEY, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("txType", "char(1)"));
  fields.append(MyMoneyDbField("postDate", "datetime"));
  fields.append(MyMoneyDbField("memo", "text"));
  fields.append(MyMoneyDbField("entryDate", "datetime"));
  fields.append(MyMoneyDbField("currencyId", "char(3)"));
  fields.append(MyMoneyDbField("bankId", "text"));
  MyMoneyDbTable t("kmmTransactions", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Splits(void){
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("transactionId", "varchar(32)",  false, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("txType", "char(1)"));
  fields.append(MyMoneyDbField("splitId", "smallint unsigned",  false, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("payeeId", "varchar(32)"));
  fields.append(MyMoneyDbField("reconcileDate", "datetime"));
  fields.append(MyMoneyDbField("action", "varchar(16)"));
  fields.append(MyMoneyDbField("reconcileFlag", "char(1)"));
  fields.append(MyMoneyDbField("value", "text", false, NOTNULL));
  fields.append(MyMoneyDbField("valueFormatted", "text"));
  fields.append(MyMoneyDbField("shares", "text", false, NOTNULL));
  fields.append(MyMoneyDbField("sharesFormatted", "text"));
  fields.append(MyMoneyDbField("memo", "text"));
  fields.append(MyMoneyDbField("accountId", "varchar(32)", false, NOTNULL));
  fields.append(MyMoneyDbField("checkNumber", "varchar(16)"));
  MyMoneyDbTable t("kmmSplits", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::KeyValuePairs(void){
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("kvpType", "varchar(16)", false, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("kvpId", "varchar(32)", false, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("kvpKey", "varchar(255)", false, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("kvpData", "text"));
  MyMoneyDbTable t("kmmKeyValuePairs", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Schedules(void){
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("id", "varchar(32)", PRIMARYKEY, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("name", "text", false, NOTNULL));
  fields.append(MyMoneyDbField("type", "tinyint unsigned", false, NOTNULL));
  fields.append(MyMoneyDbField("typeString", "text"));
  fields.append(MyMoneyDbField("occurence", "smallint unsigned", false, NOTNULL));
  fields.append(MyMoneyDbField("occurenceString", "text"));
  fields.append(MyMoneyDbField("paymentType", "tinyint unsigned"));
  fields.append(MyMoneyDbField("paymentTypeString", "longtext"));
  fields.append(MyMoneyDbField("startDate", "date", false, NOTNULL));
  fields.append(MyMoneyDbField("endDate", "date"));
  fields.append(MyMoneyDbField("fixed", "char(1)", false, NOTNULL));
  fields.append(MyMoneyDbField("autoEnter", "char(1)", false, NOTNULL));
  fields.append(MyMoneyDbField("lastPayment", "date"));
  fields.append(MyMoneyDbField("nextPaymentDue", "date"));
  fields.append(MyMoneyDbField("weekendOption", "tinyint unsigned", false, NOTNULL));
  fields.append(MyMoneyDbField("weekendOptionString", "text"));
  MyMoneyDbTable t("kmmSchedules", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::SchedulePaymentHistory(void){
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("schedId", "varchar(32)", false, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("payDate", "date", false,  NOTNULL, ISKEY));
  MyMoneyDbTable t("kmmSchedulePaymentHistory", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Securities(void){
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("id", "varchar(32)", PRIMARYKEY, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("name", "text", false, NOTNULL));
  fields.append(MyMoneyDbField("symbol", "text"));
  fields.append(MyMoneyDbField("type", "smallint unsigned", false, NOTNULL));
  fields.append(MyMoneyDbField("typeString", "text"));
  fields.append(MyMoneyDbField("smallestAccountFraction", "varchar(24)"));
  fields.append(MyMoneyDbField("tradingMarket", "text"));
  fields.append(MyMoneyDbField("tradingCurrency", "char(3)"));
  MyMoneyDbTable t("kmmSecurities", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Prices(void){
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("fromId", "varchar(32)", false, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("toId", "varchar(32)",  false, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("priceDate", "date", false, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("price", "text", false, NOTNULL));
  fields.append(MyMoneyDbField("priceFormatted", "text"));
  fields.append(MyMoneyDbField("priceSource", "text"));
  MyMoneyDbTable t("kmmPrices", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Currencies(void){
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("ISOcode", "char(3)", PRIMARYKEY, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("name", "text", false, NOTNULL));
  fields.append(MyMoneyDbField("type", "smallint unsigned"));
  fields.append(MyMoneyDbField("typeString", "text"));
  fields.append(MyMoneyDbField("symbol1", "smallint unsigned"));
  fields.append(MyMoneyDbField("symbol2", "smallint unsigned"));
  fields.append(MyMoneyDbField("symbol3", "smallint unsigned"));
  fields.append(MyMoneyDbField("symbolString", "varchar(255)"));
  fields.append(MyMoneyDbField("partsPerUnit", "varchar(24)"));
  fields.append(MyMoneyDbField("smallestCashFraction", "varchar(24)"));
  fields.append(MyMoneyDbField("smallestAccountFraction", "varchar(24)"));
  MyMoneyDbTable t("kmmCurrencies", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Reports(void) {
  QValueList<MyMoneyDbField> fields;
  fields.append(MyMoneyDbField("name", "varchar(255)", false, NOTNULL, ISKEY));
  fields.append(MyMoneyDbField("XML", "longtext"));
  MyMoneyDbTable t("kmmReportConfig", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

// function to write create SQL to a stream
void MyMoneyDbDef::generateSQL (QTextStream& s) {
  QMapConstIterator<QString, MyMoneyDbTable> tt = m_tables.begin();
  while (tt != m_tables.end()) {
    QString qs = QString("CREATE TABLE %1 (\n").arg(tt.key());
    QValueList<MyMoneyDbField>::const_iterator it = tt.data().begin();
    while (it != tt.data().end()) {
      qs = qs + "    " + (*it).name() + " " +
          (*it).type();
      if ((*it).isPrimaryKey()) qs += " PRIMARY KEY";
      if ((*it).isNotNull()) qs += " NOT NULL";
      qs += ",\n";
      ++it;
    }
    qs = qs.left(qs.length() -2) + '\n' + ");";
    s << qs << '\n';
    ++tt;
  }
}

//*****************************************************************************
void MyMoneyDbTable::buildSQLStrings (void) {
  // build fixed SQL strings for this table
  // build the insert string with placeholders for each field
  QString qs = QString("INSERT INTO %1 VALUES (").arg(m_name);
  QValueList<MyMoneyDbField>::const_iterator ft = m_fields.begin();
  while (ft != m_fields.end()) {
    qs += QString(":%1, ").arg((*ft).name());
    ++ft;
  }
  m_insertString = qs.left(qs.length() - 2) + ");";
  // build a 'select all' string (select * is deprecated)
  // don't terminate with semicolon coz we may want a where or order clause
  qs = QString("SELECT ");
  ft = m_fields.begin();
  while (ft != m_fields.end()) {
    qs += QString("%1, ").arg((*ft).name());
    ++ft;
  }
  m_selectAllString = qs.left(qs.length() - 2) + " FROM " + name();
  // build an update string; key fields go in the where clause
  qs = QString("UPDATE " + name() + " SET ");
  QString ws = QString();
  ft = m_fields.begin();
  while (ft != m_fields.end()) {
    if ((*ft).isKey()) {
      if (!ws.isEmpty()) ws += " AND ";
      ws += QString("%1 = :%2").arg((*ft).name()).arg((*ft).name());
    } else {
      qs += QString("%1 = :%2, ").arg((*ft).name()).arg((*ft).name());
    }
    ++ft;
  }
  qs = qs.left(qs.length() - 2);
  if (!ws.isEmpty()) qs += QString(" WHERE " + ws);
  m_updateString = qs + ";";

}
