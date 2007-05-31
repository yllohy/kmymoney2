/***************************************************************************
                          mymoneystoragesql.cpp
                          ---------------------
    begin                : 11 November 2005
    copyright            : (C) 2005 by Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
                         : Fernando Vilas <fvilas@iname.com>
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
#include <qmessagebox.h>  // FIXME: remove
// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragesql.h"
#include "imymoneyserialize.h"
#include "../../kmymoneyutils.h"
#include <kmymoney/kmymoneyglobalsettings.h>

#define TRY try {
#define CATCH } catch (MyMoneyException *e) {
#define PASS } catch (MyMoneyException *e) { throw; }
#define ECATCH }
#define DBG  // qDebug
//********************** THE CURRENT VERSION OF THE DATABASE LAYOUT **********************
unsigned int MyMoneyDbDef::m_currentVersion = 1;

// subclass QSqlQuery for performance tracing
bool MyMoneySqlQuery::exec () {
  ::timetrace("start sql");
  bool rc = QSqlQuery::exec();
  QString msg("end sql\n%1\n***Query returned %2, row count %3");
  ::timetrace (msg.arg(QSqlQuery::executedQuery()).arg(rc).arg(numRowsAffected()));
  return (rc);
}

//************************ Constructor/Destructor *****************************
MyMoneyStorageSql::MyMoneyStorageSql (IMyMoneySerialize *storage, const KURL& url)
  : QSqlDatabase (url.queryItem("driver"), QString("kmmdatabase")) {
  DBG("*** Entering MyMoneyStorageSql::MyMoneyStorageSql");
  m_majorVersion = 0;
  m_minorVersion = 1;
  m_progressCallback = 0;
  m_displayStatus = false;
  m_storage = storage;
  m_storagePtr = dynamic_cast<IMyMoneyStorage*>(storage);
  m_payeeListRead = false;
  m_transactionListRead = false;
  m_readingPrices = false;
  m_loadAll = false;
  m_override = false;
  m_preferred.setReportAllSplits(false);
  // supported driver types, their qt names and useful names
  // will be used by select dialog (maybe..., else remove)
  m_driverMap["QDB2"] = QString("IBM DB2");
  m_driverMap["QIBASE"] = QString("Borland Interbase");
  m_driverMap["QMYSQL3"] = QString("MySQL");
  m_driverMap["QOCI8"] = QString("Oracle Call Interface, version 8 and 9");
  m_driverMap["QODBC3"] = QString("Open Database Connectivity");
  m_driverMap["QPSQL7"] = QString("PostgreSQL v6.x and v7.x");
  m_driverMap["QSQLITE"] = QString("SQLite Version 2");
  m_driverMap["QTDS7"] = QString("Sybase Adaptive Server and Microsoft SQL Server");
}

int MyMoneyStorageSql::open(const KURL& url, int openMode, bool clear) {
  DBG("*** Entering MyMoneyStorageSql::open");
try {
  int rc = 0;
  QString driverName = url.queryItem("driver");
  if (driverName == "QDB2") m_dbType = Db2;
  else if (driverName == "QIBASE") m_dbType = Interbase;
  else if (driverName == "QMYSQL3") m_dbType = Mysql;
  else if (driverName == "QOCI8") m_dbType = Oracle8;
  else if (driverName == "QODBC3") m_dbType = ODBC3;
  else if (driverName == "QPSQL7") m_dbType = Postgresql;
  else if (driverName == "QSQLITE") m_dbType = Sqlite;
  else if (driverName == "QTDS7") m_dbType = Sybase;
  //get the input options
  QString mode = url.queryItem("mode");
  m_mode = 0;
  if (mode == "single") m_mode = 1;
  if (mode == "multi") m_mode = 2;
  QStringList options = QStringList::split(',', url.queryItem("options"));
  m_loadAll = options.contains("loadAll") || m_mode == 0;
  m_override = options.contains("override");
  // may need to make the following driver dependent
  if (m_dbType == Sqlite) m_startCommitUnitStatement = "BEGIN TRANSACTION;";
  else m_startCommitUnitStatement = "START TRANSACTION;";
  m_endCommitUnitStatement = "COMMIT";
  m_cancelCommitUnitStatement = "ROLLBACK;";

  // create the database connection
  QString dbName = url.path().right(url.path().length() - 1); // remove separator slash
  setDatabaseName(dbName);
  setHostName(url.host());
  setUserName(url.user());
  setPassword(url.pass());
  switch (openMode) {
    case IO_ReadOnly:    // OpenDatabase menu entry (or open last file)
    case IO_ReadWrite:   // Save menu entry with database open
      if (!QSqlDatabase::open()) {
        buildError(MyMoneySqlQuery(), __func__,  "opening database");
        rc = 1;
      }
      rc = createTables(); // check all tables are present, create if not (we may add tables at some time)
      break;
    case IO_WriteOnly:   // SaveAs Database - if exists, must be empty, if not will create
      if (!QSqlDatabase::open()) {
        if (createDatabase(url) != 0) {
          rc = 1;
        } else {
          if (!QSqlDatabase::open()) {
            buildError(MyMoneySqlQuery(), __func__, "opening new database");
            rc = 1;
          } else {
            rc = createTables();
          }
        }
      } else {
        createTables();
        if (clear) {
          clean();
        } else {
          rc = isEmpty();
        }
      }
      break;
    default:
      qFatal (QString("%1 - unknown open mode %2").arg(__func__).arg(openMode));
  }
  if (rc != 0) return (rc);
  // bypass logon check if we are creating a database
  if (openMode == IO_WriteOnly) return(0);
  // check if the database is locked, if not lock it
  readFileInfo();
  if (!m_logonUser.isEmpty() && (!m_override)) {
    m_error = QString
        (i18n("Database apparently in use\nOpened by %1 on %2 at %3.\nOpen anyway?"))
        .arg(m_logonUser)
        .arg(m_logonAt.date().toString(Qt::ISODate))
        .arg(m_logonAt.time().toString("hh.mm.ss"));
    qDebug(m_error);
    close(false);
    rc = -1;
  } else {
    m_logonUser = url.user() + "@" + url.host();
    m_logonAt = QDateTime::currentDateTime();
    writeFileInfo();
  }
  return(rc);
} catch (QString& s) {
    qDebug("%s",s.data());
    return (1);
}
}

void MyMoneyStorageSql::close(bool logoff) {
  DBG("*** Entering MyMoneyStorageSql::close");
  if (logoff) {
    m_logonUser = QString();
    writeFileInfo();
  }
  QSqlDatabase::close();
  QSqlDatabase::removeDatabase(this);
}

int MyMoneyStorageSql::createDatabase (const KURL& url) {
  DBG("*** Entering MyMoneyStorageSql::createDatabase");
  if (driverName() == "QSQLITE") return(0); // not needed for sqlite
  if (driverName() != "QMYSQL3") {
    m_error = QString(i18n("Cannot currently create database for driver %1")).arg(driverName());
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
  MyMoneySqlQuery qm;
  QString qs = QString("CREATE DATABASE %1;").arg(dbName);
  qm.prepare (qs);
  if (!qm.exec()) {
    buildError (qm, __func__, "Error in create database %1").arg(dbName);
    return (1);
  }
  QSqlDatabase::removeDatabase (maindb);
  return (0);
}

int MyMoneyStorageSql::upgradeDb() {
  DBG("*** Entering MyMoneyStorageSql::upgradeDb");
  //signalProgress(0, 1, QObject::tr("Upgrading database..."));
  MyMoneySqlQuery q(this);
  q.prepare ("SELECT version FROM kmmFileInfo;");
  if (!q.exec() || !q.next()) { // must be a new database
    m_majorVersion = m_db.currentVersion();
    return (0);
  }
  m_majorVersion = q.value(0).toString().section('.', 0, 0).toUInt();
  m_minorVersion = q.value(0).toString().section('.', 1, 1).toUInt();
  int rc = 0;
  while ((m_majorVersion < m_db.currentVersion()) && (rc == 0)) {
    switch (m_majorVersion) {
    case 0:
      if ((rc = upgradeToV1()) != 0) return (1);
      ++m_majorVersion;
      break;
    case 1:
      // add indices and fields for v1->v2
      // increment m_majorVersion
      break;
    default:
      qFatal("Unknown version number in database - %d", m_majorVersion);
    }
  }
  // write updated version to DB
  q.prepare (QString("UPDATE kmmFileInfo SET version=%1.%2").arg(m_majorVersion).arg(m_minorVersion));
  if (!q.exec()) {
    buildError (q, __func__, "Error updating db version");
    return (1);
  }
  //signalProgress(-1,-1);
  return (0);
}

int MyMoneyStorageSql::upgradeToV1() {
  DBG("*** Entering MyMoneyStorageSql::upgradeToV1");
  if (m_dbType == Sqlite) qFatal("SQLite upgrade NYI");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  // change kmmSplits pkey to (transactionId, splitId)
  q.prepare ("ALTER TABLE kmmSplits ADD PRIMARY KEY (transactionId, splitId);");
  if (!q.exec()) {
    buildError (q, __func__, "Error updating kmmSplits pkey");
    return (1);
  }
  // change kmmSplits alter checkNumber varchar(32)
  q.prepare (m_db.m_tables["kmmSplits"].modifyColumnString(driverName(), "checkNumber",
             MyMoneyDbColumn("checkNumber", "varchar(32)")));
  if (!q.exec()) {
    buildError (q, __func__, "Error expanding kmmSplits.checkNumber");
    return (1);
  }
  // change kmmSplits add postDate datetime
  q.prepare ("ALTER TABLE kmmSplits ADD COLUMN " +
      MyMoneyDbDatetimeColumn("postDate").generateDDL(driverName()) + ";");
  if (!q.exec()) {
    buildError (q, __func__, "Error adding kmmSplits.postDate");
    return (1);
  }
  // initialize it to same value as transaction (do it the long way round)
  q.prepare ("SELECT id, postDate FROM kmmTransactions WHERE txType = 'N';");
  if (!q.exec()) {
    buildError (q, __func__, "Error priming kmmSplits.postDate");
    return (1);
  }
  QMap<QString, QDateTime> tids;
  while (q.next()) tids[q.value(0).toString()] = q.value(1).toDateTime();
  QMap<QString, QDateTime>::ConstIterator it;
  for (it = tids.begin(); it != tids.end(); ++it) {
    q.prepare ("UPDATE kmmSplits SET postDate=:postDate WHERE transactionId = :id;");
    q.bindValue(":postDate", it.data());
    q.bindValue(":id", it.key());
    if (!q.exec()) {
      buildError (q, __func__, "priming kmmSplits.postDate");
      return(1);
    }
  }
  // add index to kmmKeyValuePairs to (kvpType,kvpId)
  QStringList list;
  list << "kvpType" << "kvpId";
  q.prepare (MyMoneyDbIndex("kmmKeyValuePairs", "kmmKVPtype_id", list, false).generateDDL(driverName()) + ";");
  if (!q.exec()) {
      buildError (q, __func__, "Error adding kmmKeyValuePairs index");
      return (1);
  }
  // add index to kmmSplits to (accountId, txType)
  list.clear();
  list << "accountId" << "txType";
  q.prepare (MyMoneyDbIndex("kmmSplits", "kmmSplitsaccount_type", list, false).generateDDL(driverName()) + ";");
  if (!q.exec()) {
    buildError (q, __func__, "Error adding kmmSplits index");
    return (1);
  }
  // change kmmSchedulePaymentHistory pkey to (schedId, payDate)
  q.prepare ("ALTER TABLE kmmSchedulePaymentHistory ADD PRIMARY KEY (schedId, payDate);");
  if (!q.exec()) {
    buildError (q, __func__, "Error updating kmmSchedulePaymentHistory pkey");
    return (1);
  }
      // change kmmPrices pkey to (fromId, toId, priceDate)
  q.prepare ("ALTER TABLE kmmPrices ADD PRIMARY KEY (fromId, toId, priceDate);");
  if (!q.exec()) {
    buildError (q, __func__, "Error updating kmmPrices pkey");
    return (1);
  }
  // change kmmReportConfig pkey to (name)
  // There wasn't one previously, so no need to drop it.
  q.prepare ("ALTER TABLE kmmReportConfig ADD PRIMARY KEY (name);");
  if (!q.exec()) {
    buildError (q, __func__, "Error updating kmmReportConfig pkey");
    return (1);
  }
  // change kmmFileInfo add budgets unsigned bigint after kvps
  q.prepare ("ALTER TABLE kmmFileInfo ADD COLUMN " +
      MyMoneyDbIntColumn("budgets", MyMoneyDbIntColumn::BIG, false).generateDDL(driverName()) + ";");
  if (!q.exec()) {
    buildError (q, __func__, "Error adding kmmFileInfo.budgets");
    return (1);
  }
  // change kmmFileInfo add hiBudgetId unsigned bigint after hiReportId
  q.prepare ("ALTER TABLE kmmFileInfo ADD COLUMN " +
      MyMoneyDbIntColumn("hiBudgetId", MyMoneyDbIntColumn::BIG, false).generateDDL(driverName()) + ";");
  if (!q.exec()) {
    buildError (q, __func__, "Error adding kmmFileInfo.hiBudgetId");
    return (1);
  }
      // change kmmFileInfo add logonUser
  q.prepare ("ALTER TABLE kmmFileInfo ADD COLUMN " +
      MyMoneyDbColumn("logonUser", "varchar(255)", false).generateDDL(driverName()) + ";");
  if (!q.exec()) {
    buildError (q, __func__, "Error adding kmmFileInfo.logonUser");
    return (1);
  }
      // change kmmFileInfo add logonAt datetime
  q.prepare ("ALTER TABLE kmmFileInfo ADD COLUMN " +
      MyMoneyDbDatetimeColumn("logonAt", false).generateDDL(driverName()) + ";");
  if (!q.exec()) {
    buildError (q, __func__, "Error adding kmmFileInfo.logonAt");
    return (1);
  }
      // change kmmAccounts add transactionCount unsigned bigint as last field
  q.prepare ("ALTER TABLE kmmAccounts ADD COLUMN " +
      MyMoneyDbIntColumn("transactionCount", MyMoneyDbIntColumn::BIG, false).generateDDL(driverName()) + " NOT NULL DEFAULT 0;");
  if (!q.exec()) {
    buildError (q, __func__, "Error adding kmmAccounts.transactionCount");
    return (1);
  }
  // calculate the transaction counts. the application logic defines an account's tx count
  // in such a way as to count multiple splits in a tx which reference the same account as one.
  // this is the only way I can think of to do this which will work in sqlite too.
  // inefficient, but it only gets done once...
  // get a list of all accounts so we'll get a zero value for those without txs
  q.prepare ("SELECT id FROM kmmAccounts");
  if (!q.exec()) {
    buildError (q, __func__, "Error retrieving accounts for transaction counting");
    return(1);
  }
  while (q.next()) {
    m_transactionCountMap[q.value(0).toCString()] = 0;
  }
  q.prepare ("SELECT accountId, transactionId FROM kmmSplits WHERE txType = 'N' ORDER BY 1, 2");
  if (!q.exec()) {
    buildError (q, __func__, "Error retrieving splits for transaction counting");
    return(1);
  }
  QCString lastAcc, lastTx;
  while (q.next()) {
    QCString thisAcc = q.value(0).toCString();
    QCString thisTx = q.value(1).toCString();
    if ((thisAcc != lastAcc) || (thisTx != lastTx)) m_transactionCountMap[thisAcc]++;
    lastAcc = thisAcc;
    lastTx = thisTx;
  }
  QMap<QCString, unsigned long>::Iterator itm;
  for (itm = m_transactionCountMap.begin(); itm != m_transactionCountMap.end(); ++itm) {
    q.prepare("UPDATE kmmAccounts SET transactionCount = :txCount WHERE id = :id;");
    q.bindValue (":txCount", QString::number(itm.data()));
    q.bindValue (":id", itm.key());
    if (!q.exec()) {
      buildError(q, __func__, "Error updating transaction count");
      return (1);
    }
  }
  m_transactionCountMap.clear();
  // there were considerable problems with record counts in V0, so rebuild them
  readFileInfo();
  m_institutions = getRecCount("kmmInstitutions");
  m_accounts = getRecCount("kmmAccounts");
  m_payees = getRecCount("kmmPayees");
  m_transactions = getRecCount("kmmTransactions WHERE txType = 'N'");
  m_splits = getRecCount("kmmSplits");
  m_securities = getRecCount("kmmSecurities");
  m_prices = getRecCount("kmmPrices");
  m_currencies = getRecCount("kmmCurrencies");
  m_schedules = getRecCount("kmmSchedules");
  m_reports = getRecCount("kmmReportConfig");
  m_kvps = getRecCount("kmmKeyValuePairs");
  m_budgets = getRecCount("kmmBudgetConfig");
  writeFileInfo();
  /* if sqlite {
    q.prepare("VACUUM;");
    if (!q.exec()) {
      buildError (q, __func__, "Error vacuuming database");
      return(1);
    }
  }*/
  endCommitUnit(__func__);
  return (0);
}

long long unsigned MyMoneyStorageSql::getRecCount (const QString& table) {
  DBG("*** Entering MyMoneyStorageSql::getRecCount");
  MyMoneySqlQuery q(this);
  q.prepare(QString("SELECT COUNT(*) FROM %1;").arg(table));
  if ((!q.exec()) || (!q.next())) {
    buildError (q, __func__, "error retrieving record count");
    qFatal("Error retrieving record count"); // definitely shouldn't happen
  }
  return (q.value(0).toULongLong());
}

int MyMoneyStorageSql::createTables () {
  DBG("*** Entering MyMoneyStorageSql::createTables");
  // check tables, create if required
  // convert everything to lower case, since SQL standard is case insensitive
  // table and column names (when not delimited), but some DBMSs disagree.
  QStringList lowerTables = tables();
  for (QStringList::iterator i = lowerTables.begin(); i != lowerTables.end(); ++i) {
    (*i) = (*i).lower();
  }

  QMapConstIterator<QString, MyMoneyDbTable> tt = m_db.begin();
  while (tt != m_db.end()) {
    if (!lowerTables.contains(tt.key().lower())) createTable (tt.data());
    ++tt;
  }

  // get the current db version from kmmFileInfo.
  // upgrade if necessary.

  return (upgradeDb()); // any errors will be caught by exception handling
}

void MyMoneyStorageSql::createTable (const MyMoneyDbTable& t) {
  DBG("*** Entering MyMoneyStorageSql::createTable");
// create the tables
  QStringList ql = QStringList::split('\n', t.generateCreateSQL(driverName()));
  MyMoneySqlQuery q(this);
  for (unsigned int i = 0; i < ql.count(); ++i) {
    q.prepare (ql[i]);
    if (!q.exec()) throw buildError (q, __func__, QString ("creating table/index %1").arg(t.name()));
  }
}

int MyMoneyStorageSql::isEmpty () {
  DBG("*** Entering MyMoneyStorageSql::isEmpty");
  // check all tables are empty
  QMapConstIterator<QString, MyMoneyDbTable> tt = m_db.begin();
  int recordCount = 0;
  while ((tt != m_db.end()) && (recordCount == 0)) {
    MyMoneySqlQuery q(this);
    q.prepare (QString("select count(*) from %1;").arg((*tt).name()));
    if (!q.exec()) throw buildError (q, __func__, "getting record count");
    if (!q.next()) throw buildError (q, __func__, "retrieving record count");
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
  DBG("*** Entering MyMoneyStorageSql::clean");
// delete all existing records
  QMapConstIterator<QString, MyMoneyDbTable> it = m_db.begin();
  while (it != m_db.end()) {
    MyMoneySqlQuery q(this);
    q.prepare(QString("DELETE from %1;").arg(it.key()));
    if (!q.exec()) throw buildError (q, __func__, QString ("cleaning database"));
    ++it;
  }
}

//////////////////////////////////////////////////////////////////

bool MyMoneyStorageSql::readFile(void) {
  DBG("*** Entering MyMoneyStorageSql::readFile");
  m_displayStatus = true;
  try {
    readFileInfo();
    readInstitutions();
    if (m_loadAll) {
      readPayees();
    } else {
      QValueList<QCString> user;
      user.append(QCString("USER"));
      readPayees(user);
    }
  //::timetrace("done payees");
    readCurrencies();
  //::timetrace("done currencies");
    readSecurities();
  //::timetrace("done securities");
    readAccounts();
    if (m_loadAll) {
      readTransactions();
    } else {
      if (m_preferred.filterSet().singleFilter.accountFilter) readTransactions (m_preferred);
    }
  //::timetrace("done accounts");
    readSchedules();
  //::timetrace("done schedules");
    readPrices();
  //::timetrace("done prices");
    readReports();
  //::timetrace("done reports");
    readBudgets();
  //::timetrace("done budgets");
    if (m_mode == 0) m_storage->rebuildAccountBalances();
  // this seems to be nonsense, but it clears the dirty flag
  // as a side-effect.
    m_storage->setLastModificationDate(m_storage->lastModificationDate());
    if (m_mode == 0) m_storage = NULL;
  // make sure the progress bar is not shown any longer
    signalProgress(-1, -1);
    m_displayStatus = false;
    //MyMoneySqlQuery::traceOn();
    return true;
  } catch (QString& s) {
    return false;
  }
}

// The following is called from 'SaveAsDatabase'
bool MyMoneyStorageSql::writeFile(void) {
  DBG("*** Entering MyMoneyStorageSql::writeFile");
  // initialize record counts and hi ids
  m_institutions = m_accounts = m_payees = m_transactions = m_splits
      = m_securities = m_prices = m_currencies = m_schedules  = m_reports = m_kvps = m_budgets = 0;
  m_hiIdInstitutions = m_hiIdPayees = m_hiIdAccounts = m_hiIdTransactions =
      m_hiIdSchedules = m_hiIdSecurities = m_hiIdReports = m_hiIdBudgets = 0;
  m_displayStatus = true;
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
  writeBudgets();
  writeFileInfo();
  // this seems to be nonsense, but it clears the dirty flag
  // as a side-effect.
  m_storage->setLastModificationDate(m_storage->lastModificationDate());
  if (m_mode == 0) m_storage = NULL;
  // make sure the progress bar is not shown any longer
  signalProgress(-1, -1);
  m_displayStatus = false;
  return true;
} catch (QString& s) {
  return false;
}
}
// --------------- SQL Transaction (commit unit) handling -----------------------------------
void MyMoneyStorageSql::startCommitUnit (const QString& callingFunction) {
  DBG("*** Entering MyMoneyStorageSql::startCommitUnit");
  if (m_commitUnitStack.isEmpty()) {
    MyMoneySqlQuery q(this);
    q.prepare(m_startCommitUnitStatement);
    if (!q.exec()) throw buildError (q, __func__, "starting commit unit");
  }
  m_commitUnitStack.push(callingFunction);
}

void MyMoneyStorageSql::endCommitUnit (const QString& callingFunction) {
  DBG("*** Entering MyMoneyStorageSql::endCommitUnit");
  if (callingFunction != m_commitUnitStack.top())
    qFatal(QString("%1 - %2 s/be %3").arg(__func__).arg(callingFunction).arg(m_commitUnitStack.top()));
  m_commitUnitStack.pop();
  if (m_commitUnitStack.isEmpty()) {
    MyMoneySqlQuery q(this);
    q.prepare(m_endCommitUnitStatement);
    if (!q.exec()) throw buildError (q, __func__, "ending commit unit");
  }
}

void MyMoneyStorageSql::cancelCommitUnit (const QString& callingFunction) {
  DBG("*** Entering MyMoneyStorageSql::cancelCommitUnit");
  if (callingFunction != m_commitUnitStack.top())
    qDebug(QString("%1 - %2 s/be %3").arg(__func__).arg(callingFunction).arg(m_commitUnitStack.top()));
  if (m_commitUnitStack.isEmpty()) return;
  m_commitUnitStack.clear();
  MyMoneySqlQuery q(this);
  q.prepare(m_cancelCommitUnitStatement);
  if (!q.exec()) throw buildError (q, __func__, "cancelling commit unit");
}

/////////////////////////////////////////////////////////////////////
void MyMoneyStorageSql::fillStorage() {
  DBG("*** Entering MyMoneyStorageSql::fillStorage");
  if (!m_transactionListRead)  // make sure we have loaded everything
    readTransactions();
  if (!m_payeeListRead)
    readPayees();
}

//------------------------------ Write SQL routines ----------------------------------------
// **** Institutions ****
void MyMoneyStorageSql::writeInstitutions() {
  DBG("*** Entering MyMoneyStorageSql::writeInstitutions");
  // first, get a list of what's on the database
  // anything not in the list needs to be inserted
  // anything which is will be updated and removed from the list
  // anything left over at the end will need to be deleted
  // this is an expensive and inconvenient way to do things; find a better way
  // one way would be to build the lists when reading the db
  // unfortunately this object does not persist between read and write
  // it would also be nice if we could tell which objects had been updated since we read them in
  QValueList<QString> dbList;
  MyMoneySqlQuery q(this);
  q.prepare("SELECT id FROM kmmInstitutions;");
  if (!q.exec()) throw buildError (q, __func__, "building Institution list");
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
    signalProgress (++m_institutions, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare("DELETE FROM kmmInstitutions WHERE id = :id");
      q.bindValue(":id", (*it));
      if (!q.exec()) throw buildError (q, __func__, "deleting Institution");
      deleteKeyValuePairs("OFXSETTINGS", (*it));
      ++it;
    }
  }
}

void MyMoneyStorageSql::addInstitution(const MyMoneyInstitution& inst) {
  DBG("*** Entering MyMoneyStorageSql::addInstitution");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmInstitutions"].insertString());
  writeInstitution(inst ,q);
  m_institutions++;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::modifyInstitution(const MyMoneyInstitution& inst) {
  DBG("*** Entering MyMoneyStorageSql::modifyInstitution");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmInstitutions"].updateString());
  deleteKeyValuePairs("OFXSETTINGS", inst.id());
  writeInstitution(inst ,q);
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::removeInstitution(const MyMoneyInstitution& inst) {
  DBG("*** Entering MyMoneyStorageSql::removeInstitution");
  startCommitUnit(__func__);
  deleteKeyValuePairs("OFXSETTINGS", inst.id());
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmInstitutions"].deleteString());
  q.bindValue(":id", inst.id());
  if (!q.exec()) throw buildError (q, __func__, QString("deleting  Institution"));
  m_institutions--;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::writeInstitution(const MyMoneyInstitution& i, MyMoneySqlQuery& q) {
  DBG("*** Entering MyMoneyStorageSql::writeInstitution");
  q.bindValue(":id", i.id());
  q.bindValue(":name", i.name());
  q.bindValue(":manager", i.manager());
  q.bindValue(":routingCode", i.sortcode());
  q.bindValue(":addressStreet", i.street());
  q.bindValue(":addressCity", i.city());
  q.bindValue(":addressZipcode", i.postcode());
  q.bindValue(":telephone", i.telephone());
  if (!q.exec()) throw buildError (q, __func__, QString("writing Institution"));
  m_hiIdInstitutions = calcHighId(m_hiIdInstitutions, i.id());
}

// **** Payees ****
void MyMoneyStorageSql::writePayees() {
  DBG("*** Entering MyMoneyStorageSql::writePayees");
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  MyMoneySqlQuery q(this);
  q.prepare("SELECT id FROM kmmPayees;");
  if (!q.exec()) throw buildError (q, __func__, "building Payee list");
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
    signalProgress(++m_payees, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare(m_db.m_tables["kmmPayees"].deleteString());
      q.bindValue(":id", (*it));
      if (!q.exec()) throw buildError (q, __func__, "deleting Payee");
      m_payees -= q.numRowsAffected();
      ++it;
    }
  }
}

void MyMoneyStorageSql::addPayee(const MyMoneyPayee& payee) {
  DBG("*** Entering MyMoneyStorageSql::addPayee");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmPayees"].insertString());
  writePayee(payee,q);
  m_payees++;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::modifyPayee(const MyMoneyPayee& payee) {
  DBG("*** Entering MyMoneyStorageSql::modifyPayee");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmPayees"].updateString());
  writePayee(payee,q);
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::modifyUserInfo(const MyMoneyPayee& payee) {
  DBG("*** Entering MyMoneyStorageSql::modifyUserInfo");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmPayees"].updateString());
  writePayee(payee,q, true);
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::removePayee(const MyMoneyPayee& payee) {
  DBG("*** Entering MyMoneyStorageSql::removePayee");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmPayees"].deleteString());
  q.bindValue(":id", payee.id());
  if (!q.exec()) throw buildError (q, __func__, QString("deleting  Payee"));
  m_payees--;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::writePayee(const MyMoneyPayee& p, MyMoneySqlQuery& q, bool isUserInfo) {
  DBG("*** Entering MyMoneyStorageSql::writePayee");
  if (isUserInfo) {
    q.bindValue(":id", "USER");
  } else {
    q.bindValue(":id", p.id());
  }
  q.bindValue(":name", p.name());
  q.bindValue(":reference", p.reference());
  q.bindValue(":email", p.email());
  q.bindValue(":addressStreet", p.address());
  q.bindValue(":addressCity", p.city());
  q.bindValue(":addressZipcode", p.postcode());
  q.bindValue(":addressState", p.state());
  q.bindValue(":telephone", p.telephone());
  if (!q.exec()) throw buildError (q, __func__, QString ("writing Payee"));
  if (!isUserInfo) m_hiIdPayees = calcHighId(m_hiIdPayees, p.id());
}

// **** Accounts ****
void MyMoneyStorageSql::writeAccounts() {
  DBG("*** Entering MyMoneyStorageSql::writeAccounts");
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  MyMoneySqlQuery q(this);
  q.prepare("SELECT id FROM kmmAccounts;");
  if (!q.exec()) throw buildError (q, __func__, "building Account list");
  while (q.next()) dbList.append(q.value(0).toString());

  QValueList<MyMoneyAccount> list;
  m_storage->accountList(list);
  QValueList<MyMoneyAccount>::ConstIterator it;
  signalProgress(0, list.count(), "Writing Accounts...");
  if (dbList.isEmpty()) { // new table, insert standard accounts
    q.prepare (m_db.m_tables["kmmAccounts"].insertString());
  } else {
    q.prepare (m_db.m_tables["kmmAccounts"].updateString());
  }
  writeAccount(m_storage->asset(), q); ++m_accounts;
  writeAccount(m_storage->liability(), q); ++m_accounts;
  writeAccount(m_storage->expense(), q); ++m_accounts;
  writeAccount(m_storage->income(), q); ++m_accounts;
  writeAccount(m_storage->equity(), q); ++m_accounts;
  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i) {
    m_transactionCountMap[(*it).id()] = m_storagePtr->transactionCount((*it).id());
    if (dbList.contains((*it).id())) {
      q.prepare (m_db.m_tables["kmmAccounts"].updateString());
      dbList.remove ((*it).id());
    } else {
      q.prepare (m_db.m_tables["kmmAccounts"].insertString());
    }
    writeAccount(*it, q);
    signalProgress(++m_accounts, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      if (!m_storagePtr->isStandardAccount(QCString(*it))) {
        q.prepare("DELETE FROM kmmAccounts WHERE id = :id");
        q.bindValue(":id", (*it));
        if (!q.exec()) throw buildError (q, __func__, "deleting Account");
        deleteKeyValuePairs("ACCOUNT", (*it));
      }
      ++it;
    }
  }
}

void MyMoneyStorageSql::addAccount(const MyMoneyAccount& acc) {
  DBG("*** Entering MyMoneyStorageSql::addAccount");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmAccounts"].insertString());
  writeAccount(acc,q);
  m_accounts++;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::modifyAccount(const MyMoneyAccount& acc) {
  DBG("*** Entering MyMoneyStorageSql::modifyAccount");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmAccounts"].updateString());
  deleteKeyValuePairs("ACCOUNT", acc.id());
  writeAccount(acc,q);
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::removeAccount(const MyMoneyAccount& acc) {
  DBG("*** Entering MyMoneyStorageSql::removeAccount");
  startCommitUnit(__func__);
  deleteKeyValuePairs("ACCOUNT", acc.id());
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmAccounts"].deleteString());
  q.bindValue(":id", acc.id());
  if (!q.exec()) throw buildError (q, __func__, QString("deleting  Account"));
  m_accounts--;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::writeAccount(const MyMoneyAccount& acc, MyMoneySqlQuery& q) {
  DBG("*** Entering MyMoneyStorageSql::writeAccount");
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
  q.bindValue(":balance", m_storagePtr->balance(acc.id(), QDate()).toString());
  q.bindValue(":balanceFormatted", m_storagePtr->balance(acc.id(), QDate()).formatMoney());
  q.bindValue(":transactionCount", Q_ULLONG(m_transactionCountMap[acc.id()]));
  if (!q.exec()) throw buildError (q, __func__, QString("writing Account"));

  //Add in Key-Value Pairs for accounts.
  writeKeyValuePairs("ACCOUNT", acc.id(), acc.pairs());
  m_hiIdAccounts = calcHighId(m_hiIdAccounts, acc.id());
}

// **** Transactions and Splits ****
void MyMoneyStorageSql::writeTransactions() {
  DBG("*** Entering MyMoneyStorageSql::writeTransactions");
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  MyMoneySqlQuery q(this);
  q.prepare("SELECT id FROM kmmTransactions WHERE txType = 'N';");
  if (!q.exec()) throw buildError (q, __func__, "building Transaction list");
  while (q.next()) dbList.append(q.value(0).toString());

  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  QValueList<MyMoneyTransaction> list;
  m_storage->transactionList(list, filter);
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

void MyMoneyStorageSql::addTransaction (const MyMoneyTransaction& tx) {
  DBG("*** Entering MyMoneyStorageSql::addTransaction");
  startCommitUnit(__func__);
  // add the transaction and splits
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmTransactions"].insertString());
  writeTransaction(tx.id(), tx, q, "N");
  m_transactions++;
  // for each split account, update lastMod date, balance, txCount
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = tx.splits().begin(); it_s != tx.splits().end(); ++it_s) {
    MyMoneyAccount acc = m_storagePtr->account((*it_s).accountId());
    m_transactionCountMap[acc.id()]++;
    modifyAccount(acc);
  }
  // in the fileinfo record, update lastMod, txCount, next TxId
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::modifyTransaction (const MyMoneyTransaction& tx) {
  DBG("*** Entering MyMoneyStorageSql::modifyTransaction");
  startCommitUnit(__func__);
  // remove the splits of the old tx from the count table
  MyMoneySqlQuery q(this);
  q.prepare ("SELECT accountId FROM kmmSplits WHERE transactionId = :txId;");
  q.bindValue(":txId", tx.id());
  if (!q.exec()) throw buildError (q, __func__, "retrieving old splits");
  while (q.next()) {
    QCString id = q.value(0).toCString();
    m_transactionCountMap[id]--;
  }
  // add the transaction and splits
  q.prepare (m_db.m_tables["kmmTransactions"].updateString());
  writeTransaction(tx.id(), tx, q, "N");
  // for each split account, update lastMod date, balance, txCount
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = tx.splits().begin(); it_s != tx.splits().end(); ++it_s) {
    MyMoneyAccount acc = m_storagePtr->account((*it_s).accountId());
    m_transactionCountMap[acc.id()]++;
    modifyAccount(acc);
  }
  // in the fileinfo record, update lastMod
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::removeTransaction(const MyMoneyTransaction& tx) {
  DBG("*** Entering MyMoneyStorageSql::removeTransaction");
  startCommitUnit(__func__);
  deleteTransaction(tx.id());
  m_transactions--;

  // for each split account, update lastMod date, balance, txCount
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = tx.splits().begin(); it_s != tx.splits().end(); ++it_s) {
    MyMoneyAccount acc = m_storagePtr->account((*it_s).accountId());
    m_transactionCountMap[acc.id()]--;
    modifyAccount(acc);
  }
  // in the fileinfo record, update lastModDate, txCount
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::deleteTransaction(const QString& id) {
  DBG("*** Entering MyMoneyStorageSql::deleteTransaction");
  MyMoneySqlQuery q(this);
  q.prepare("DELETE FROM kmmSplits WHERE transactionId = :transactionId;");
  q.bindValue(":transactionId", id);
  if (!q.exec()) throw buildError (q, __func__, "deleting Splits");
  m_splits -= q.numRowsAffected();
  deleteKeyValuePairs("TRANSACTION", id);
  q.prepare(m_db.m_tables["kmmTransactions"].deleteString());
  q.bindValue(":id", id);
  if (!q.exec()) throw buildError (q, __func__, "deleting Transaction");
}

void MyMoneyStorageSql::writeTransaction(const QString& txId, const MyMoneyTransaction& tx, MyMoneySqlQuery& q, const QString& type) {
  DBG("*** Entering MyMoneyStorageSql::writeTransaction");
  q.bindValue(":id", txId);
  q.bindValue(":txType", type);
  q.bindValue(":postDate", tx.postDate());
  q.bindValue(":memo", tx.memo());
  q.bindValue(":entryDate", tx.entryDate());
  q.bindValue(":currencyId", tx.commodity());
  q.bindValue(":bankId", tx.bankID());
  if (!q.exec()) throw buildError (q, __func__, QString("writing Transaction"));

  m_txPostDate = tx.postDate(); // FIXME: TEMP till Tom puts date in split object
  QValueList<MyMoneySplit> splitList = tx.splits();
  writeSplits(txId, type, splitList);

  //Add in Key-Value Pairs for transactions.
  deleteKeyValuePairs("TRANSACTION", txId);
  writeKeyValuePairs("TRANSACTION", txId, tx.pairs());
  m_hiIdTransactions = calcHighId(m_hiIdTransactions, tx.id());
}

void MyMoneyStorageSql::writeSplits(const QString& txId, const QString& type, const QValueList<MyMoneySplit>& splitList) {
  DBG("*** Entering MyMoneyStorageSql::writeSplits");
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<unsigned int> dbList;
  MyMoneySqlQuery q(this);
  q.prepare("SELECT splitId FROM kmmSplits where transactionId = :id;");
  q.bindValue(":id", txId);
  if (!q.exec()) throw buildError (q, __func__, "building Split list");
  while (q.next()) dbList.append(q.value(0).toUInt());

  QValueList<MyMoneySplit>::const_iterator it;
  unsigned int i;
  for(it = splitList.begin(), i = 0; it != splitList.end(); ++it, ++i) {
    if (dbList.contains(i)) {
      q.prepare (m_db.m_tables["kmmSplits"].updateString());
      dbList.remove (i);
    } else {
      q.prepare (m_db.m_tables["kmmSplits"].insertString());
      m_splits++;
    }
    writeSplit(txId, (*it), type, i, q);

  }

  if (!dbList.isEmpty()) {
    QValueList<unsigned int>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare("DELETE FROM kmmSplits WHERE transactionId = :txId AND splitId = :splitId");
      q.bindValue(":txId", txId);
      q.bindValue(":splitId", *it);
      if (!q.exec()) throw buildError (q, __func__, "deleting Splits");
      ++it;
    }
  }
}

void MyMoneyStorageSql::writeSplit(const QString& txId, const MyMoneySplit& split,
                                   const QString& type, const int splitId, MyMoneySqlQuery& q) {
  DBG("*** Entering MyMoneyStorageSql::writeSplit");
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
  q.bindValue(":postDate", m_txPostDate); // FIXME: when Tom puts date into split object
  if (!q.exec()) throw buildError (q, __func__, QString("writing Split"));
}

// **** Schedules ****
void MyMoneyStorageSql::writeSchedules() {
  DBG("*** Entering MyMoneyStorageSql::writeSchedules");
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  MyMoneySqlQuery q(this);
  q.prepare("SELECT id FROM kmmSchedules;");
  if (!q.exec()) throw buildError (q, __func__, "building Schedule list");
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
    writeSchedule(*it, q, insert);
    signalProgress(++m_schedules, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::const_iterator it = dbList.begin();
    while (it != dbList.end()) {
      deleteSchedule(*it);
      ++it;
    }
  }
}

void MyMoneyStorageSql::addSchedule(const MyMoneySchedule& sched) {
  DBG("*** Entering MyMoneyStorageSql::addSchedule");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmSchedules"].insertString());
  writeSchedule(sched,q, true);
  m_schedules++;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::modifySchedule(const MyMoneySchedule& sched) {
  DBG("*** Entering MyMoneyStorageSql::modifySchedule");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmSchedules"].updateString());
  writeSchedule(sched,q, false);
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::removeSchedule(const MyMoneySchedule& sched) {
  DBG("*** Entering MyMoneyStorageSql::removeSchedule");
  startCommitUnit(__func__);
  deleteSchedule(sched.id());
  m_schedules--;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::deleteSchedule (const QString& id) {
  DBG("*** Entering MyMoneyStorageSql::deleteSchedule");
  deleteTransaction(id);
  MyMoneySqlQuery q(this);
  q.prepare("DELETE FROM kmmSchedulePaymentHistory WHERE schedId = :id");
  q.bindValue(":id", id);
  if (!q.exec()) throw buildError (q, __func__, "deleting Schedule Payment History");
  q.prepare(m_db.m_tables["kmmSchedules"].deleteString());
  q.bindValue(":id", id);
  if (!q.exec()) throw buildError (q, __func__, "deleting Schedule");
}

void MyMoneyStorageSql::writeSchedule(const MyMoneySchedule& sch, MyMoneySqlQuery& q, bool insert) {
  DBG("*** Entering MyMoneyStorageSql::writeSchedule");
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
  if (!q.exec()) throw buildError (q, __func__, QString("writing Schedules"));

  //store the payment history for this scheduled task.
  //easiest way is to delete all and re-insert; it's not a high use table
  q.prepare("DELETE FROM kmmSchedulePaymentHistory WHERE schedId = :id;");
  q.bindValue(":id", sch.id());
  if (!q.exec()) throw buildError (q, __func__, QString("deleting  Schedule Payment History"));

  QValueList<QDate> payments = sch.recordedPayments();
  QValueList<QDate>::Iterator it;
  for (it=payments.begin(); it!=payments.end(); ++it) {
    q.prepare (m_db.m_tables["kmmSchedulePaymentHistory"].insertString());
    q.bindValue(":schedId", sch.id());
    q.bindValue(":payDate", (*it));
    if (!q.exec()) throw buildError (q, __func__, QString("writing Schedule Payment History"));
  }

    //store the transaction data for this task.
  if (!insert) {
    q.prepare (m_db.m_tables["kmmTransactions"].updateString());
  } else {
    q.prepare (m_db.m_tables["kmmTransactions"].insertString());
  }
  writeTransaction(sch.id(), sch.transaction(), q, "S");

  m_hiIdSchedules = calcHighId(m_hiIdSchedules, sch.id());
}

// **** Securities ****
void MyMoneyStorageSql::writeSecurities() {
  DBG("*** Entering MyMoneyStorageSql::writeSecurities");
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  MyMoneySqlQuery q(this);
  q.prepare("SELECT id FROM kmmSecurities;");
  if (!q.exec()) throw buildError (q, __func__, "building security list");
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
    signalProgress(++m_securities, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare("DELETE FROM kmmSecurities WHERE id = :id");
      q.bindValue(":id", (*it));
      if (!q.exec()) throw buildError (q, __func__, "deleting Security");
      q.prepare("DELETE FROM kmmPrices WHERE fromId = :id OR toId = :id");
      q.bindValue(":fromId", (*it));
      q.bindValue(":toId", (*it));
      if (!q.exec()) throw buildError (q, __func__, "deleting Security");
      deleteKeyValuePairs("SECURITY", (*it));
       ++it;
    }
  }
}

void MyMoneyStorageSql::addSecurity(const MyMoneySecurity& sec) {
  DBG("*** Entering MyMoneyStorageSql::addSecurity");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmSecurities"].insertString());
  writeSecurity(sec,q);
  m_securities++;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::modifySecurity(const MyMoneySecurity& sec) {
  DBG("*** Entering MyMoneyStorageSql::modifySecurity");
  startCommitUnit(__func__);
  deleteKeyValuePairs("SECURITY", sec.id());
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmSecurities"].updateString());
  writeSecurity(sec,q);
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::removeSecurity(const MyMoneySecurity& sec) {
  DBG("*** Entering MyMoneyStorageSql::removeSecurity");
  startCommitUnit(__func__);
  deleteKeyValuePairs("SECURITY", sec.id());
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmSecurities"].deleteString());
  q.bindValue(":id", sec.id());
  if (!q.exec()) throw buildError (q, __func__, QString("deleting  Security"));
  m_securities--;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::writeSecurity(const MyMoneySecurity& security, MyMoneySqlQuery& q) {
  DBG("*** Entering MyMoneyStorageSql::writeSecurity");
  q.bindValue(":id", security.id());
  q.bindValue(":name", security.name());
  q.bindValue(":symbol", security.tradingSymbol());
  q.bindValue(":type", static_cast<int>(security.securityType()));
  q.bindValue(":typeString", KMyMoneyUtils::securityTypeToString(security.securityType()));
  q.bindValue(":smallestAccountFraction", security.smallestAccountFraction());
  q.bindValue(":tradingCurrency", security.tradingCurrency());
  q.bindValue(":tradingMarket", security.tradingMarket());
  if (!q.exec()) throw buildError (q, __func__, QString ("writing Securities"));

  //Add in Key-Value Pairs for security
  writeKeyValuePairs("SECURITY", security.id(), security.pairs());
  m_hiIdSecurities = calcHighId(m_hiIdSecurities, security.id());
}

// **** Prices ****
void MyMoneyStorageSql::writePrices() {
  DBG("*** Entering MyMoneyStorageSql::writePrices");
  // due to difficulties in matching and determining deletes
  // easiest way is to delete all and re-insert
  MyMoneySqlQuery q(this);
  q.prepare("DELETE FROM kmmPrices");
  if (!q.exec()) throw buildError (q, __func__, QString("deleting Prices"));
  m_prices = 0;

  const MyMoneyPriceList list = m_storage->priceList();
  signalProgress(0, list.count(), "Writing Prices...");
  MyMoneyPriceList::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it)   {
    writePricePair(*it);
  }
}

void MyMoneyStorageSql::writePricePair(const MyMoneyPriceEntries& p) {
  DBG("*** Entering MyMoneyStorageSql::writePricePair");
  MyMoneyPriceEntries::ConstIterator it;
  for(it = p.begin(); it != p.end(); ++it) {
    writePrice (*it);
    signalProgress(++m_prices, 0);
  }
}

void MyMoneyStorageSql::addPrice(const MyMoneyPrice& p) {
  DBG("*** Entering MyMoneyStorageSql::addPrice");
  if (m_readingPrices) return;
  // the app always calls addPrice, whether or not there is already one there
  startCommitUnit(__func__);
  bool newRecord = false;
  MyMoneySqlQuery q(this);
  QString s = m_db.m_tables["kmmPrices"].selectAllString(false);
  s += " WHERE fromId = :fromId AND toId = :toId AND priceDate = :priceDate;";
  q.prepare (s);
  q.bindValue(":fromId", p.from());
  q.bindValue(":toId", p.to());
  q.bindValue(":priceDate", p.date());
  if (!q.exec()) throw buildError (q, __func__, QString("finding Price"));
  if (q.next()) {
    q.prepare(m_db.m_tables["kmmPrices"].updateString());
  } else {
    q.prepare(m_db.m_tables["kmmPrices"].insertString());
    m_prices++;
    newRecord = true;
  }
  q.bindValue(":fromId", p.from());
  q.bindValue(":toId", p.to());
  q.bindValue(":priceDate", p.date());
  q.bindValue(":price", p.rate().toString());
  q.bindValue(":priceFormatted", p.rate().formatMoney());
  q.bindValue(":priceSource", p.source());
  if (!q.exec()) throw buildError (q, __func__, QString("writing Price"));

  if (newRecord) writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::removePrice(const MyMoneyPrice& p) {
  DBG("*** Entering MyMoneyStorageSql::removePrice");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmPrices"].deleteString());
  q.bindValue(":fromId", p.from());
  q.bindValue(":toId", p.to());
  q.bindValue(":priceDate", p.date());
  if (!q.exec()) throw buildError (q, __func__, QString("deleting  Price"));
  m_prices--;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::writePrice(const MyMoneyPrice& p) {
  DBG("*** Entering MyMoneyStorageSql::writePrice");
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmPrices"].insertString());
  q.bindValue(":fromId", p.from());
  q.bindValue(":toId", p.to());
  q.bindValue(":priceDate", p.date());
  q.bindValue(":price", p.rate().toString());
  q.bindValue(":priceFormatted", p.rate().formatMoney());
  q.bindValue(":priceSource", p.source());
  if (!q.exec()) throw buildError (q, __func__, QString("writing Prices"));
}

// **** Currencies ****
void MyMoneyStorageSql::writeCurrencies() {
  DBG("*** Entering MyMoneyStorageSql::writeCurrencies");
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  MyMoneySqlQuery q(this);
  q.prepare("SELECT ISOCode FROM kmmCurrencies;");
  if (!q.exec()) throw buildError (q, __func__, "building Currency list");
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
      if (!q.exec()) throw buildError (q, __func__, "deleting Currency");
      ++it;
    }
  }
}

void MyMoneyStorageSql::addCurrency(const MyMoneySecurity& sec) {
  DBG("*** Entering MyMoneyStorageSql::addCurrency");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmCurrencies"].insertString());
  writeCurrency(sec,q);
  m_currencies++;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::modifyCurrency(const MyMoneySecurity& sec) {
  DBG("*** Entering MyMoneyStorageSql::modifyCurrency");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmCurrencies"].updateString());
  writeCurrency(sec,q);
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::removeCurrency(const MyMoneySecurity& sec) {
  DBG("*** Entering MyMoneyStorageSql::removeCurrency");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmCurrencies"].deleteString());
  q.bindValue(":ISOCode", sec.id());
  if (!q.exec()) throw buildError (q, __func__, QString("deleting  Currency"));
  m_currencies--;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::writeCurrency(const MyMoneySecurity& currency, MyMoneySqlQuery& q) {
  DBG("*** Entering MyMoneyStorageSql::writeCurrency");
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
  if (!q.exec()) throw buildError (q, __func__, QString("writing Currencies"));
}


void MyMoneyStorageSql::writeReports() {
  DBG("*** Entering MyMoneyStorageSql::writeReports");
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  MyMoneySqlQuery q(this);
  q.prepare("SELECT name FROM kmmReportConfig;");
  if (!q.exec()) throw buildError (q, __func__, "building Report list");
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
    writeReport(*it, q);
    signalProgress(++m_reports, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare("DELETE FROM kmmReportConfig WHERE name = :name");
      q.bindValue(":name", (*it));
      if (!q.exec()) throw buildError (q, __func__, "deleting Report");
      ++it;
    }
  }
}

void MyMoneyStorageSql::addReport(const MyMoneyReport& rep) {
  DBG("*** Entering MyMoneyStorageSql::addReport");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmReportConfig"].insertString());
  writeReport(rep,q);
  m_reports++;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::modifyReport(const MyMoneyReport& rep) {
  DBG("*** Entering MyMoneyStorageSql::modifyReport");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmReportConfig"].updateString());
  writeReport(rep,q);
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::removeReport(const MyMoneyReport& rep) {
  DBG("*** Entering MyMoneyStorageSql::removeReport");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmReportConfig"].deleteString());
  q.bindValue(":name", rep.name());
  if (!q.exec()) throw buildError (q, __func__, QString("deleting  Report"));
  m_reports--;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::writeReport (const MyMoneyReport& rep, MyMoneySqlQuery& q) {
  DBG("*** Entering MyMoneyStorageSql::writeReport");
  QDomDocument d; // create a dummy XML document
  QDomElement e = d.createElement("REPORTS");
  d.appendChild (e);
  rep.writeXML(d, e); // write the XML to document
  q.bindValue(":name", rep.name());
  q.bindValue(":XML", d.toString());
  if (!q.exec()) throw buildError (q, __func__, QString("writing Reports"));
}

void MyMoneyStorageSql::writeBudgets() {
  DBG("*** Entering MyMoneyStorageSql::writeBudgets");
  // first, get a list of what's on the database (see writeInstitutions)
  QValueList<QString> dbList;
  MyMoneySqlQuery q(this);
  q.prepare("SELECT name FROM kmmBudgetConfig;");
  if (!q.exec()) throw buildError (q, __func__, "building Budget list");
  while (q.next()) dbList.append(q.value(0).toString());

  QValueList<MyMoneyBudget> list = m_storage->budgetList();
  signalProgress(0, list.count(), "Writing Budgets...");
  QValueList<MyMoneyBudget>::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it){
    if (dbList.contains((*it).name())) {
      q.prepare (m_db.m_tables["kmmBudgetConfig"].updateString());
      dbList.remove ((*it).name());
    } else {
      q.prepare (m_db.m_tables["kmmBudgetConfig"].insertString());
    }
    writeBudget(*it, q);
    signalProgress(++m_budgets, 0);
  }

  if (!dbList.isEmpty()) {
    QValueList<QString>::iterator it = dbList.begin();
    while (it != dbList.end()) {
      q.prepare("DELETE FROM kmmBudgetConfig WHERE id = :id");
      q.bindValue(":name", (*it));
      if (!q.exec()) throw buildError (q, __func__, "deleting Budget");
      ++it;
    }
  }
}

void MyMoneyStorageSql::addBudget(const MyMoneyBudget& bud) {
  DBG("*** Entering MyMoneyStorageSql::addBudget");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmBudgetConfig"].insertString());
  writeBudget(bud,q);
  m_budgets++;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::modifyBudget(const MyMoneyBudget& bud) {
  DBG("*** Entering MyMoneyStorageSql::modifyBudget");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmBudgetConfig"].updateString());
  writeBudget(bud,q);
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::removeBudget(const MyMoneyBudget& bud) {
  DBG("*** Entering MyMoneyStorageSql::removeBudget");
  startCommitUnit(__func__);
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmBudgetConfig"].deleteString());
  q.bindValue(":id", bud.id());
  if (!q.exec()) throw buildError (q, __func__, QString("deleting  Budget"));
  m_budgets--;
  writeFileInfo();
  endCommitUnit(__func__);
}

void MyMoneyStorageSql::writeBudget (const MyMoneyBudget& bud, MyMoneySqlQuery& q) {
  DBG("*** Entering MyMoneyStorageSql::writeBudget");
  QDomDocument d; // create a dummy XML document
  QDomElement e = d.createElement("BUDGETS");
  d.appendChild (e);
  bud.writeXML(d, e); // write the XML to document
  q.bindValue(":id", bud.id());
  q.bindValue(":name", bud.name());
  q.bindValue(":start", bud.budgetstart());
  q.bindValue(":XML", d.toString());
  if (!q.exec()) throw buildError (q, __func__, QString("writing Budgets"));
}

void MyMoneyStorageSql::writeFileInfo() {
  DBG("*** Entering MyMoneyStorageSql::writeFileInfo");
  // we have no real way of knowing when these change, so re-write them every time
  deleteKeyValuePairs("STORAGE", "");
  writeKeyValuePairs("STORAGE", "", m_storage->pairs());
  //
  MyMoneySqlQuery q(this);
  q.prepare ("SELECT * FROM kmmFileInfo;");
  if (!q.exec()) throw buildError (q, __func__, "checking fileinfo");
  QString qs;
  if (q.next())
    qs = m_db.m_tables["kmmFileInfo"].updateString();
  else
    qs = (m_db.m_tables["kmmFileInfo"].insertString());
  q.prepare(qs);
  q.bindValue(":version", QString("%1.%2")
      .arg(m_majorVersion)
          .arg((m_storage->fileFixVersion() + 1)));
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
  q.bindValue(":budgets", m_budgets);
  q.bindValue(":dateRangeStart", QDate());
  q.bindValue(":dateRangeEnd", QDate());
  q.bindValue(":hiInstitutionId", (m_storage->institutionId() > m_hiIdInstitutions ? m_storage->institutionId() : m_hiIdInstitutions));
  q.bindValue(":hiPayeeId", (m_storage->payeeId() > m_hiIdPayees ? m_storage->payeeId() : m_hiIdPayees));
  q.bindValue(":hiAccountId", (m_storage->accountId() > m_hiIdAccounts ? m_storage->accountId() : m_hiIdAccounts));
  q.bindValue(":hiTransactionId", (m_storage->transactionId() > m_hiIdTransactions ? m_storage->transactionId() : m_hiIdTransactions));
  q.bindValue(":hiScheduleId", (m_storage->scheduleId() > m_hiIdSchedules ? m_storage->scheduleId() : m_hiIdSchedules));
  q.bindValue(":hiSecurityId", (m_storage->securityId() > m_hiIdSecurities ? m_storage->securityId() : m_hiIdSecurities));
  q.bindValue(":hiReportId", (m_storage->reportId() > m_hiIdReports ? m_storage->reportId() : m_hiIdReports));
  q.bindValue(":hiBudgetId", (m_storage->budgetId() > m_hiIdBudgets ? m_storage->budgetId() : m_hiIdBudgets));
  q.bindValue(":encryptData", m_encryptData);
  q.bindValue(":updateInProgress", "N");
  q.bindValue(":logonUser", m_logonUser);
  q.bindValue(":logonAt", m_logonAt.toString(Qt::ISODate));
  if (!q.exec()) throw buildError (q, __func__, QString("writing FileInfo"));
}

// **** Key/value pairs ****
void MyMoneyStorageSql::writeKeyValuePairs(const QString& kvpType, const QString& kvpId, const QMap<QCString,  QString>& pairs) {
  DBG("*** Entering MyMoneyStorageSql::writeKeyValuePairs");
  QMap<QCString, QString>::const_iterator it;
  for(it = pairs.begin(); it != pairs.end(); ++it) {
    writeKeyValuePair (kvpType, kvpId, it.key(), it.data());
  }
}

void MyMoneyStorageSql::writeKeyValuePair (const QString& kvpType, const QString& kvpId, const QString& kvpKey, const QString& kvpData) {
  DBG("*** Entering MyMoneyStorageSql::writeKeyValuePair");
  MyMoneySqlQuery q(this);
  q.prepare (m_db.m_tables["kmmKeyValuePairs"].insertString());
  q.bindValue(":kvpType", kvpType);
  q.bindValue(":kvpId", kvpId);
  q.bindValue(":kvpKey", kvpKey);
  q.bindValue(":kvpData", kvpData);
  if (!q.exec()) throw buildError (q, __func__, QString("writing KVP"));
  m_kvps++;
}

void MyMoneyStorageSql::deleteKeyValuePairs (const QString& kvpType, const QString& kvpId) {
  DBG("*** Entering MyMoneyStorageSql::deleteKeyValuePairs");
  MyMoneySqlQuery q(this);
  q.prepare ("DELETE FROM kmmKeyValuePairs WHERE kvpType = :kvpType AND kvpId = :kvpId;");
  q.bindValue(":kvpType", kvpType);
  q.bindValue(":kvpId", kvpId);
  if (!q.exec()) throw buildError (q, __func__, QString("deleting kvp for %1 %2").arg(kvpType).arg(kvpId));
  m_kvps -= q.numRowsAffected();
}

//******************************** read SQL routines **************************************
#define CASE(a) if ((*ft)->name() == #a)
#define GETSTRING q.value(i).toString()
#define GETCSTRING q.value(i).toCString()
#define GETDATE q.value(i).toDate()
#define GETINT q.value(i).toInt()
#define GETULL q.value(i).toULongLong()

void MyMoneyStorageSql::readFileInfo(void) {
  DBG("*** Entering MyMoneyStorageSql::readFileInfo");
  signalProgress(0, 18, QObject::tr("Loading file information..."));
  MyMoneyDbTable t = m_db.m_tables["kmmFileInfo"];
  MyMoneySqlQuery q(this);
  q.prepare (t.selectAllString());
  if (!q.exec()) throw buildError (q, __func__, QString("reading FileInfo"));
  if (!q.next()) throw buildError (q, __func__, QString("retrieving FileInfo"));
  MyMoneyDbTable::field_iterator ft = t.begin();
  int i = 0;
  while (ft != t.end()) {
    CASE(version)  setVersion(GETSTRING); // check version == current version...
    CASE(created) m_storage->setCreationDate(QDate::fromString(GETSTRING, Qt::ISODate));
    CASE(lastModified) m_storage->setLastModificationDate(QDate::fromString(GETSTRING, Qt::ISODate));
    CASE(hiInstitutionId) {m_hiIdInstitutions = GETULL; m_storage->loadInstitutionId(m_hiIdInstitutions);};
    CASE(hiPayeeId)       {m_hiIdPayees = GETULL; m_storage->loadPayeeId(m_hiIdPayees);};
    CASE(hiAccountId)     {m_hiIdAccounts = GETULL; m_storage->loadAccountId(m_hiIdAccounts);};
    CASE(hiTransactionId) {m_hiIdTransactions = GETULL; m_storage->loadTransactionId(m_hiIdTransactions);};
    CASE(hiScheduleId)    {m_hiIdSchedules = GETULL; m_storage->loadScheduleId(m_hiIdSchedules);};
    CASE(hiSecurityId)    {m_hiIdSecurities = GETULL; m_storage->loadSecurityId(m_hiIdSecurities);};
    CASE(hiReportId  )    {m_hiIdReports = GETULL; m_storage->loadReportId(m_hiIdReports); };
    CASE(hiBudgetId  )    {m_hiIdBudgets = GETULL; m_storage->loadBudgetId(m_hiIdBudgets); };
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
    CASE(reports     ) m_reports = GETULL;
    CASE(budgets     ) m_budgets = GETULL;
    CASE(encryptData) m_encryptData = GETSTRING;
    CASE(logonUser)  m_logonUser = GETSTRING;
    CASE(logonAt)  m_logonAt = QDateTime::fromString(GETSTRING, Qt::ISODate);
    ++ft; ++i;
    signalProgress(i,0);
  }
  m_storage->setPairs(readKeyValuePairs("STORAGE", QString("")).pairs());
}

void MyMoneyStorageSql::setVersion (const QString& version) {
  DBG("*** Entering MyMoneyStorageSql::setVersion");
  m_majorVersion = version.section('.', 0, 0).toUInt();
  m_minorVersion = version.section('.', 1, 1).toUInt();
  // Okay, I made a cockup by forgetting to include a fixversion in the database
  // design, so we'll use the minor version as fix level (similar to VERSION
  // and FIXVERSION in XML file format). A second mistake was setting minor version to 1
  // in the first place, so we need to subtract one on reading and add one on writing (sigh)!!
  m_storage->setFileFixVersion( m_minorVersion - 1);
}

void MyMoneyStorageSql::readInstitutions(void) {
  DBG("*** Entering MyMoneyStorageSql::readInstitutions");
  signalProgress(0, m_institutions, QObject::tr("Loading institutions..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmInstitutions"];
  MyMoneySqlQuery q(this);
  q.prepare (t.selectAllString());
  if (!q.exec()) throw buildError (q, __func__, QString("reading Institution"));
  while (q.next()) {
    MyMoneyDbTable::field_iterator ft = t.begin();
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
    MyMoneySqlQuery sq(this);
    sq.prepare (QString("SELECT id from kmmAccounts where institutionId = :id"));
    sq.bindValue(":id", iid);
    if (!sq.exec()) throw buildError (q, __func__, QString("reading Institution AccountList"));
    QStringList aList;
    while (sq.next()) aList.append(sq.value(0).toString());
    for (QStringList::Iterator it = aList.begin(); it != aList.end(); ++it)
      inst.addAccountId(QCString(*it));
    TRY
    // FIXME: Adapt to new interface make sure, to take care of the securities as well
    // m_storage->loadInstitution(MyMoneyInstitution(QCString(iid), inst));
    PASS
    signalProgress (++progress, 0);
  }
}

void MyMoneyStorageSql::readPayees (const QCString& id) {
  DBG("*** Entering MyMoneyStorageSql::readPayees");
  QValueList<QCString> list;
  list.append(id);
  readPayees(list);
}

void MyMoneyStorageSql::readPayees(const QValueList<QCString> pid) {
  DBG("*** Entering MyMoneyStorageSql::readPayees");
  if (m_displayStatus) {
    signalProgress(0, m_payees, QObject::tr("Loading payees..."));
  } else {
    if (m_payeeListRead) return;
  }
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmPayees"];
  MyMoneySqlQuery q(this);
  if (pid.isEmpty()) {
    q.prepare (t.selectAllString());
  } else {
    QString whereClause = " where (";
    QString itemConnector = "";
    QValueList<QCString>::ConstIterator it;
    for (it = pid.begin(); it != pid.end(); ++it) {
      whereClause.append(QString("%1id = '%2'").arg(itemConnector).arg(*it));
      itemConnector = " or ";
    }
    whereClause += ")";
    q.prepare (QString(t.selectAllString(false) + whereClause));
  }
  if (!q.exec()) throw buildError (q, __func__, QString("reading Payee"));
  while (q.next()) {
    MyMoneyDbTable::field_iterator ft = t.begin();
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
      TRY
      m_storage->setUser(payee);
      PASS
    } else {
      TRY
        // FIXME: Adapt to new interface make sure, to take care of the securities as well
        // m_storage->loadPayee(MyMoneyPayee(QCString(pid), payee));
      CATCH
        delete e; // ignore duplicates
      ECATCH
    }
    if (m_displayStatus) signalProgress(++progress, 0);
  }
  if (pid.isEmpty()) m_payeeListRead = true;
}

void MyMoneyStorageSql::readAccounts(void) {
  DBG("*** Entering MyMoneyStorageSql::readAccounts");
  signalProgress(0, m_accounts, QObject::tr("Loading accounts..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmAccounts"];
  MyMoneySqlQuery q(this);
  q.prepare (t.selectAllString());
  if (!q.exec()) throw buildError (q, __func__, QString("reading Account"));
  while (q.next()) {
    MyMoneyDbTable::field_iterator ft = t.begin();
    int i = 0;
    QCString aid;
    QString balance;
    MyMoneyAccount acc;
    unsigned long txCount;
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
      CASE(balance) balance = GETSTRING;
      CASE(transactionCount) m_transactionCountMap[aid] = GETULL;
      ++ft; ++i;
    }
  // get list of subaccounts
    MyMoneySqlQuery sq(this);
    sq.prepare (QString("SELECT id from kmmAccounts where parentId = :id"));
    sq.bindValue(":id", aid);
    if (!sq.exec()) throw buildError (q, __func__, QString("reading subAccountList"));
    QStringList aList;
    while (sq.next()) aList.append(sq.value(0).toString());
    for (QStringList::Iterator it = aList.begin(); it != aList.end(); ++it)
      acc.addAccountId(QCString(*it));

    // Process any key value pair
    acc.setPairs(readKeyValuePairs("ACCOUNT", aid).pairs());
    // in database mode, load the balance from the account record
    // else we would need to read all the transactions
    TRY
    if (m_mode > 0) {
      acc.setBalance(MyMoneyMoney(balance));
      // FIXME: Adapt to new interface make sure, to take care of the securities as well
      // m_storage->loadAccount(MyMoneyAccount(QCString(aid), acc));
      if (acc.value("PreferredAccount") == "Yes") m_preferred.addAccount(aid);
    } else  {
      // FIXME: Adapt to new interface make sure, to take care of the securities as well
      // m_storage->loadAccount(MyMoneyAccount(QCString(aid), acc));
    }
    PASS
    signalProgress(++progress, 0);
  }
}

void MyMoneyStorageSql::readTransactions(const QString& tidList, const QString& dateClause) {
  DBG("*** Entering MyMoneyStorageSql::readTransactions");
  if (m_transactionListRead) return; // all list already in memory
  if (m_displayStatus) signalProgress(0, m_transactions, QObject::tr("Loading transactions..."));
  int progress = 0;
  m_payeeList.clear();
  QString whereClause;
  if (tidList.isEmpty()) {
    whereClause = " WHERE txType = 'N' ";
  } else {
    whereClause = " WHERE id IN " + tidList;
  }
  if (!dateClause.isEmpty()) whereClause += QString(" and " + dateClause);
  MyMoneyDbTable t = m_db.m_tables["kmmTransactions"];
  MyMoneySqlQuery q(this);
  q.prepare (QString(t.selectAllString(false) + whereClause + " ORDER BY id;"));
  if (!q.exec()) throw buildError (q, __func__, QString("reading Transaction"));
  MyMoneyDbTable ts = m_db.m_tables["kmmSplits"];
  if (tidList.isEmpty()) {
    whereClause = " WHERE txType = 'N' ";
  } else {
    whereClause = " WHERE transactionId IN " + tidList;
  }
  MyMoneySqlQuery qs(this);
  QString splitQuery = QString(ts.selectAllString(false) + whereClause + " ORDER BY transactionId, splitId;");
  qs.prepare (splitQuery);
  if (!qs.exec()) throw buildError (qs, __func__, "reading Splits");
  QString splitTxId = "ZZZ";
  MyMoneySplit s;
  if (qs.next()) {
    splitTxId = qs.value(0).toString();
    readSplit (s, qs, ts);
  } else {
    splitTxId = "ZZZ";
  }
  QMap <QString, MyMoneyTransaction> txMap;
  QStringList txList;
  while (q.next()) {
    MyMoneyTransaction tx;
    QString txId;
    tx.setId(QCString(txId));
    MyMoneyDbTable::field_iterator ft = t.begin();
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
    txList << txId;
    txMap.insert(txId,tx);
  }
  QMap <QString, MyMoneyKeyValueContainer> kvpMap = readKeyValuePairs("TRANSACTION", txList);

  for (QMapIterator<QString, MyMoneyTransaction> i = txMap.begin();
       i != txMap.end(); ++i) {
         i.data().setPairs(kvpMap[i.key()].pairs());
         TRY
             // FIXME: Adapt to new interface make sure, to take care of the securities as well
             // m_storage->loadTransaction(MyMoneyTransaction(QCString(i.key()), i.data()));
         PASS
             if (m_displayStatus) signalProgress(++progress, 0);
       }

       if (!m_payeeList.isEmpty())
         readPayees(m_payeeList);
       if ((tidList.isEmpty()) && (dateClause.isEmpty())) {
         qDebug("setting full list read");
         m_transactionListRead = true;
       }
}

void MyMoneyStorageSql::readTransactions(const MyMoneyTransactionFilter& filter) {
  DBG("*** Entering MyMoneyStorageSql::readTransactions");
  // analyze the filter
  if (m_transactionListRead) return; // all list already in memory
  // if the filter is restricted to certain accounts/categories
  // check if we already have them all in memory
  QCStringList accounts;
  filter.accounts(accounts);
  filter.categories(accounts);
  QCStringList::iterator it;
  bool allAccountsLoaded = true;
  for (it = accounts.begin(); it != accounts.end(); ++it) {
    if (m_accountsLoaded.find(*it) == m_accountsLoaded.end()) {
      allAccountsLoaded = false;
      break;
    }
  }
  if (allAccountsLoaded) return;
  /* Some filter combinations do not lend themselves to implementation
  * in SQL, or are likely to require such extensive reading of the database
  * as to make it easier to just read everything into memory.  */
  bool canImplementFilter = true;
  MyMoneyMoney m1, m2;
  if (filter.amountFilter( m1, m2 )) {
    alert ("Amount Filter Set");
    canImplementFilter = false;
  }
  QString n1, n2;
  if (filter.numberFilter(n1, n2)) {
    alert("Number filter set");
    canImplementFilter = false;
  }
  int t1;
  if (filter.firstType(t1)) {
    alert("Type filter set");
    canImplementFilter = false;
  }
  int s1;
  if (filter.firstState(s1)) {
    alert("State filter set");
    canImplementFilter = false;
  }
  QRegExp t2;
  if (filter.textFilter(t2)) {
    alert("text filter set");
    canImplementFilter = false;
  }
  MyMoneyTransactionFilter::FilterSet s = filter.filterSet();
  if (s.singleFilter.validityFilter) {
    alert("Validity filter set");
    canImplementFilter = false;
  }
  if (!canImplementFilter) {
    readTransactions();
    return;
  }
  bool accountsOnlyFilter = true;
  bool splitFilterActive = false; // the split filter is active if we are selecting on fields in the split table
  // get start and end dates
  QDate start = filter.fromDate();
  QDate end = filter.toDate();
  // not entirely sure if the following is correct, but at best, saves a lot of reads, at worst
  // it only causes us to read a few more transactions that strictly necessary (I think...)
  if (start == KMyMoneySettings::startDate().date()) start = QDate();
  bool txFilterActive = ((start != QDate()) || (end != QDate())); // and this for fields in the transaction table
  if (txFilterActive) accountsOnlyFilter = false;

  QString whereClause = "";
  QString subClauseconnector = " where ";
  // payees
  QCStringList payees;
  filter.payees(payees);
  if (filter.payees(payees)) {
    accountsOnlyFilter = false;
    QString itemConnector = "";
    QString payeesClause = "(";
    QCStringList::iterator it;
    for (it = payees.begin(); it != payees.end(); ++it) {
      payeesClause.append(QString("%1payeeId = '%2'").arg(itemConnector).arg(*it));
      itemConnector = " or ";
    }
    whereClause += QString(subClauseconnector + payeesClause + ")");
    splitFilterActive = true;
    subClauseconnector = " and ";
  }

  // accounts and categories
  if (!accounts.isEmpty()) {
    splitFilterActive = true;
    QString itemConnector = " txType = 'N' and (";
    QString accountsClause = "";
    QCStringList::iterator it;
    for (it = accounts.begin(); it != accounts.end(); ++it) {
      if (m_accountsLoaded.find(*it) == m_accountsLoaded.end()) {
        accountsClause.append(QString("%1accountId = '%2'").arg(itemConnector).arg(*it));
        itemConnector = " or ";
        if (accountsOnlyFilter) m_accountsLoaded.append(*it); // a bit premature...
      }
    }
    if (!accountsClause.isEmpty()) {
      whereClause += QString(subClauseconnector + accountsClause + ")");
      subClauseconnector = " and (";
    }
  }
  // if the split filter is active, but the where clause is empty
  // it means we already have all the transactions for the specified filter
  // in memory, so just exit
  if ((splitFilterActive) && (whereClause.isEmpty())) {
    qDebug("all transactions already in storage");
    return;
  }

  // if we have neither a split filter, nor a tx (date) filter
  // it's effectively a read all
  if ((!splitFilterActive) && (!txFilterActive)) {
    qDebug("reading all transactions");
    readTransactions();
    return;
  }
  // build a date clause for the transaction table
  QString dateClause;
  QString connector = "";
  if (end != QDate()) {
    dateClause = QString("(postDate < '%1')").arg(end.toString(Qt::ISODate));
    connector = " and ";
  }
  if (start != QDate()) {
    dateClause += QString("%1 (postDate > '%2')").arg(connector).arg(start.toString(Qt::ISODate));
  }
  // now get a list of transaction ids
  // if we have only a date filter, we need to build the list from the tx table
  // otherwise we need to build from the split table
  MyMoneySqlQuery q(this);
  if (splitFilterActive) {
    q.prepare (QString("select distinct transactionId from kmmSplits%1;").arg(whereClause));
  } else {
    q.prepare (QString("select distinct id from kmmTransactions where %1;").arg(dateClause));
    txFilterActive = false; // kill off the date filter now
  }
  if (!q.exec()) throw buildError (q, __func__, QString("%1 reading Tids").arg(__func__));
  qDebug(QString("Id list query = %1 returned %2 rows").arg(q.executedQuery())
      .arg(q.size()));
  QStringList txList;
  while (q.next()) {
    QCString tid = q.value(0).toCString();
    if (!m_storage->isDuplicateTransaction(tid))
      txList.append (tid);
  }
  if (txList.isEmpty()) {
    alert ("No matching transactions found, or all already read");
    return;
  }
  // build a list of txIds
  QString tidList = "(";
  QStringList::iterator it_t;
  connector = "";
  for (it_t = txList.begin(); it_t != txList.end(); ++it_t) {
    tidList.append(QString("%1'%2'").arg(connector).arg(*it_t));
    connector = ", ";
  }
  tidList.append(")");
  readTransactions(tidList, dateClause);
  //FIXME: if we have an accounts-only filter, recalc balances on loaded accounts
}

const unsigned long MyMoneyStorageSql::transactionCount (const QCString& aid) {
  DBG("*** Entering MyMoneyStorageSql::transactionCount");
  if (aid.length() == 0)
    return m_transactions;
  else
    return m_transactionCountMap[aid];
}

void MyMoneyStorageSql::readSplit (MyMoneySplit& s, const MyMoneySqlQuery& q, const MyMoneyDbTable& t) {
  DBG("*** Entering MyMoneyStorageSql::readSplit");
  s.setId(QCString(""));
  MyMoneyDbTable::field_iterator ft = t.begin();
  int i = 0;
  QCString payeeId;
  while (ft != t.end()) {
    CASE(payeeId) payeeId = GETCSTRING;
    CASE(reconcileDate) s.setReconcileDate(GETDATE);
    CASE(action) s.setAction(GETCSTRING);
    CASE(reconcileFlag) s.setReconcileFlag(static_cast<MyMoneySplit::reconcileFlagE>(GETINT));
    CASE(value) s.setValue(MyMoneyMoney(GETSTRING));
    CASE(shares) s.setShares(MyMoneyMoney(GETSTRING));
    CASE(memo) s.setMemo(GETSTRING);
    CASE(accountId) s.setAccountId(GETCSTRING);
    CASE(checkNumber) s.setNumber(GETSTRING);
    CASE(postDate) m_txPostDate = GETDATE; // FIXME - when Tom puts date into split object
    ++ft; ++i;
  }
  s.setPayeeId(payeeId);
  if ((m_mode > 0) && (!m_payeeListRead)) {
    if (m_payeeList.find(payeeId) == m_payeeList.end())
      m_payeeList.append(payeeId);
  }
  return;
}

void MyMoneyStorageSql::readSchedules(void) {
  DBG("*** Entering MyMoneyStorageSql::readSchedules");
  signalProgress(0, m_schedules, QObject::tr("Loading schedules..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmSchedules"];
  MyMoneySqlQuery q(this);
  q.prepare (QString(t.selectAllString(false) + " ORDER BY id;"));
  if (!q.exec()) throw buildError (q, __func__, QString("reading Schedules"));
  while (q.next()) {
    MyMoneyDbTable::field_iterator ft = t.begin();
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
    m_payeeList.clear();
    MyMoneyDbTable t = m_db.m_tables["kmmTransactions"];
    MyMoneySqlQuery q(this);
    q.prepare (QString(t.selectAllString(false) + " WHERE id = :id;"));
    q.bindValue(":id", s.id());
    if (!q.exec()) throw buildError (q, __func__, QString("reading Scheduled Transaction"));
    if (!q.next()) throw buildError (q, __func__, QString("retrieving scheduled transaction"));
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
    MyMoneySqlQuery qs(this);
    qs.prepare (QString(ts.selectAllString(false) + " WHERE transactionId = :id ORDER BY splitId;"));
    qs.bindValue(":id", s.id());
    if (!qs.exec()) throw buildError (qs, __func__, "reading Scheduled Splits");
    while (qs.next()) {
      MyMoneySplit sp;
      readSplit (sp, qs, ts);
      tx.addSplit (sp);
    }
    if (!m_payeeList.isEmpty())
      readPayees(m_payeeList);
    // Process any key value pair
    tx.setPairs(readKeyValuePairs("TRANSACTION", s.id()).pairs());
    s.setTransaction(tx);

    // read in the recorded payments
    MyMoneySqlQuery sq(this);
    sq.prepare (QString("SELECT payDate from kmmSchedulePaymentHistory where schedId = :id"));
    sq.bindValue(":id", s.id());
    if (!sq.exec()) throw buildError (q, __func__, QString("reading schedule payment history"));
    while (sq.next()) s.recordPayment (sq.value(0).toDate());
    TRY
    // FIXME: Adapt to new interface make sure, to take care of the securities as well
    // m_storage->loadSchedule(s);
    PASS
    signalProgress(++progress, 0);
  }
}

void MyMoneyStorageSql::readSecurities(void) {
  DBG("*** Entering MyMoneyStorageSql::readSecurities");
  signalProgress(0, m_securities, QObject::tr("Loading securities..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmSecurities"];
  MyMoneySqlQuery q(this);
  q.prepare (QString(t.selectAllString(false) + " ORDER BY id;"));
  if (!q.exec()) throw buildError (q, __func__, QString("reading Securities"));
  while (q.next()) {
    MyMoneyDbTable::field_iterator ft = t.begin();
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
    TRY
    // FIXME: Adapt to new interface make sure, to take care of the currencies as well
    //   see MyMoneyStorageXML::readSecurites()
    // m_storage->loadSecurity(MyMoneySecurity(eid,e));
    PASS
    signalProgress(++progress, 0);
  }
}

void MyMoneyStorageSql::readPrices(void) {
  DBG("*** Entering MyMoneyStorageSql::readPrices");
  signalProgress(0, m_prices, QObject::tr("Loading prices..."));
  int progress = 0;
  m_readingPrices = true;
  MyMoneyDbTable t = m_db.m_tables["kmmPrices"];
  MyMoneySqlQuery q(this);
  q.prepare (t.selectAllString());
  if (!q.exec()) throw buildError (q, __func__, QString("reading Prices"));
  while (q.next()) {
    MyMoneyDbTable::field_iterator ft = t.begin();
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
    TRY
    m_storage->addPrice(MyMoneyPrice(from, to,  date, rate, source));
    PASS
    signalProgress(++progress, 0);
  }
  m_readingPrices = false;
}

void MyMoneyStorageSql::readCurrencies(void) {
  DBG("*** Entering MyMoneyStorageSql::readCurrencies");
  signalProgress(0, m_currencies, QObject::tr("Loading currencies..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmCurrencies"];
  MyMoneySqlQuery q(this);
  q.prepare (QString(t.selectAllString(false) + " ORDER BY ISOcode;"));
  if (!q.exec()) throw buildError (q, __func__, QString("reading Currencies"));
  while (q.next()) {
    MyMoneyDbTable::field_iterator ft = t.begin();
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
    TRY
    // m_storage->loadCurrency(MyMoneySecurity(id, c));
    PASS
    signalProgress(++progress, 0);
  }
}

void MyMoneyStorageSql::readReports(void) {
  DBG("*** Entering MyMoneyStorageSql::readReports");
  signalProgress(0, m_reports, QObject::tr("Loading reports..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmReportConfig"];
  MyMoneySqlQuery q(this);
  q.prepare (QString(t.selectAllString(true)));
  if (!q.exec()) throw buildError (q, __func__, QString("reading reports"));
  QMap<QCString, MyMoneyReport> rList;
  while (q.next()) {
    MyMoneyDbTable::field_iterator ft = t.begin();
    int i = 0;
    QDomDocument d;
    while (ft != t.end()) {
      CASE(XML) d.setContent(GETSTRING, false);
      ++ft; ++i;
    }
    QDomNode child = d.firstChild();
    child = child.firstChild();
    MyMoneyReport report;
    TRY
    // Not sure, if this change is correct and what this should really be.
    // According to my understanding it should work - ipwizard
    if (report.read(child.toElement()))
      rList[report.id()] = report;
    PASS
    signalProgress(++progress, 0);
  }
  m_storage->loadReports(rList);
}

void MyMoneyStorageSql::readBudgets(void) {
  DBG("*** Entering MyMoneyStorageSql::readBudgets");
  signalProgress(0, m_budgets, QObject::tr("Loading budgets..."));
  int progress = 0;
  MyMoneyDbTable t = m_db.m_tables["kmmBudgetConfig"];
  MyMoneySqlQuery q(this);
  q.prepare (QString(t.selectAllString(true)));
  if (!q.exec()) throw buildError (q, __func__, QString("reading budgets"));
  QMap<QCString, MyMoneyBudget> budgets;
  while (q.next()) {
    MyMoneyDbTable::field_iterator ft = t.begin();
    int i = 0;
    QDomDocument d;
    while (ft != t.end()) {
      CASE(XML) d.setContent(GETSTRING, false);
      ++ft; ++i;
    }
    QDomNode child = d.firstChild();
    child = child.firstChild();
    MyMoneyBudget budget (child.toElement());
    budgets[budget.id()] = budget;
    signalProgress(++progress, 0);
  }
  m_storage->loadBudgets(budgets);
}

MyMoneyKeyValueContainer MyMoneyStorageSql::readKeyValuePairs (const QString kvpType, const QString& kvpId) {
  DBG("*** Entering MyMoneyStorageSql::readKeyValuePairs");
  MyMoneyKeyValueContainer list;
  MyMoneySqlQuery q(this);
  q.prepare ("SELECT kvpKey, kvpData from kmmKeyValuePairs where kvpType = :type and kvpId = :id;");
  q.bindValue(":type", kvpType);
  q.bindValue(":id", kvpId);
  if (!q.exec()) throw buildError (q, __func__, QString("reading Kvp for %1 %2").arg(kvpType).arg(kvpId));
  while (q.next()) list.setValue(QCString(q.value(0).toString()), q.value(1).toString());
  return (list);
}

QMap<QString, MyMoneyKeyValueContainer> MyMoneyStorageSql::readKeyValuePairs (const QString kvpType, const QStringList& kvpIdList) {
  DBG("*** Entering MyMoneyStorageSql::readKeyValuePairs");
  QMap<QString, MyMoneyKeyValueContainer> retval;

  MyMoneySqlQuery q(this);
  QString query ("SELECT kvpId, kvpKey, kvpData from kmmKeyValuePairs where kvpType = :type");

  if (!kvpIdList.empty()) {
    query += " and (kvpId = '" + *(kvpIdList.begin()) + "'";
    for (QStringList::const_iterator i = kvpIdList.begin(); i != kvpIdList.end(); ++i) {
        query += " or kvpId = '" + *i + "'";
    }
    query += ")";
  }

  query += " order by kvpId;";
  q.prepare (query);
  q.bindValue(":type", kvpType);
  if (!q.exec()) throw buildError (q, __func__, QString("reading Kvp List for %1").arg(kvpType));
  while (q.next()) {
    retval [q.value(0).toString()].setValue(QCString(q.value(1).toString()), q.value(2).toString());
  }

  return (retval);
}

//****************************************************
const long long unsigned MyMoneyStorageSql::calcHighId
     (const long long unsigned i, const QString& id) {
  DBG("*** Entering MyMoneyStorageSql::calcHighId");
  QString nid = id;
  long long unsigned high = nid.replace(QRegExp("[A-Z]*"), "").toULongLong();
  return (high > i ? high : i);
}

void MyMoneyStorageSql::setProgressCallback(void(*callback)(int, int, const QString&)) {
  m_progressCallback = callback;
}

void MyMoneyStorageSql::signalProgress(int current, int total, const QString& msg) {
  if (m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}

// **************************** Error display routine *******************************
QString& MyMoneyStorageSql::buildError (const MyMoneySqlQuery& q, const QString& function, const QString& message) {
  QString s = QString("Error in function %1 : %2").arg(function).arg(message);
  QSqlError e = lastError();
  s += QString ("\nDriver = %1, Host = %2, User = %3, Database = %4")
      .arg(driverName()).arg(hostName()).arg(userName()).arg(databaseName());
  s += QString ("\nDriver Error: %1").arg(e.driverText());
  s += QString ("\nDatabase Error No %1: %2").arg(e.number()).arg(e.databaseText());
  e = q.lastError();
  s += QString ("\nExecuted: %1").arg(q.executedQuery());
  s += QString ("\nQuery error No %1: %2").arg(e.number()).arg(e.text());
  m_error = s;
  qDebug(s);
  cancelCommitUnit(function);
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
  Budgets();
}

/* PRIMARYKEY - these fields combine to form a unique key field on which the db will create an index
   NOTNULL - this field should never be null
   UNSIGNED - for numeric types, indicates the field is UNSIGNED
   ?ISKEY - where there is no primary key, these fields can be used to uniquely identify a record
  Default is that a field is not a part of a primary key, nullable, and if numeric, signed */

#define PRIMARYKEY true
#define NOTNULL true
#define UNSIGNED false
//#define ISKEY true

void MyMoneyDbDef::FileInfo(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("version", "varchar(16)"));
  fields.append(new MyMoneyDbColumn("created", "date"));
  fields.append(new MyMoneyDbColumn("lastModified", "date"));
  fields.append(new MyMoneyDbColumn("baseCurrency", "char(3)"));
  fields.append(new MyMoneyDbIntColumn("institutions", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("accounts", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("payees", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("transactions", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("splits", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("securities", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("prices", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("currencies", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("schedules", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("reports", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("kvps", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbColumn("dateRangeStart", "date"));
  fields.append(new MyMoneyDbColumn("dateRangeEnd", "date"));
  fields.append(new MyMoneyDbIntColumn("hiInstitutionId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("hiPayeeId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("hiAccountId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("hiTransactionId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("hiScheduleId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("hiSecurityId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("hiReportId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbColumn("encryptData", "varchar(255)"));
  fields.append(new MyMoneyDbColumn("updateInProgress", "char(1)"));
  fields.append(new MyMoneyDbIntColumn("budgets", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("hiBudgetId", MyMoneyDbIntColumn::BIG, UNSIGNED));
  fields.append(new MyMoneyDbColumn("logonUser", "varchar(255)"));
  fields.append(new MyMoneyDbDatetimeColumn("logonAt"));
  MyMoneyDbTable t("kmmFileInfo", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Institutions(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("name", MyMoneyDbTextColumn::NORMAL, false, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("manager"));
  fields.append(new MyMoneyDbTextColumn("routingCode"));
  fields.append(new MyMoneyDbTextColumn("addressStreet"));
  fields.append(new MyMoneyDbTextColumn("addressCity"));
  fields.append(new MyMoneyDbTextColumn("addressZipcode"));
  fields.append(new MyMoneyDbTextColumn("telephone"));
  MyMoneyDbTable t("kmmInstitutions", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Payees(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("id", "varchar(32)",  PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("name"));
  fields.append(new MyMoneyDbTextColumn("reference"));
  fields.append(new MyMoneyDbTextColumn("email"));
  fields.append(new MyMoneyDbTextColumn("addressStreet"));
  fields.append(new MyMoneyDbTextColumn("addressCity"));
  fields.append(new MyMoneyDbTextColumn("addressZipcode"));
  fields.append(new MyMoneyDbTextColumn("addressState"));
  fields.append(new MyMoneyDbTextColumn("telephone"));
  MyMoneyDbTable t("kmmPayees", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Accounts(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("id", "varchar(32)",  PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbColumn("institutionId", "varchar(32)"));
  fields.append(new MyMoneyDbColumn("parentId", "varchar(32)"));
  fields.append(new MyMoneyDbDatetimeColumn("lastReconciled"));
  fields.append(new MyMoneyDbDatetimeColumn("lastModified"));
  fields.append(new MyMoneyDbColumn("openingDate", "date"));
  fields.append(new MyMoneyDbTextColumn("accountNumber"));
  fields.append(new MyMoneyDbColumn("accountType", "varchar(16)", false, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("accountTypeString"));
  fields.append(new MyMoneyDbColumn("isStockAccount", "char(1)"));
  fields.append(new MyMoneyDbTextColumn("accountName"));
  fields.append(new MyMoneyDbTextColumn("description"));
  fields.append(new MyMoneyDbColumn("currencyId", "varchar(32)"));
  fields.append(new MyMoneyDbTextColumn("balance"));
  fields.append(new MyMoneyDbTextColumn("balanceFormatted"));
  fields.append(new MyMoneyDbIntColumn("transactionCount", MyMoneyDbIntColumn::BIG, UNSIGNED));
  MyMoneyDbTable t("kmmAccounts", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Transactions(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbColumn("txType", "char(1)"));
  fields.append(new MyMoneyDbDatetimeColumn("postDate"));
  fields.append(new MyMoneyDbTextColumn("memo"));
  fields.append(new MyMoneyDbDatetimeColumn("entryDate"));
  fields.append(new MyMoneyDbColumn("currencyId", "char(3)"));
  fields.append(new MyMoneyDbTextColumn("bankId"));
  MyMoneyDbTable t("kmmTransactions", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Splits(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("transactionId", "varchar(32)",  PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbColumn("txType", "char(1)"));
  fields.append(new MyMoneyDbIntColumn("splitId", MyMoneyDbIntColumn::SMALL, UNSIGNED,  PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbColumn("payeeId", "varchar(32)"));
  fields.append(new MyMoneyDbDatetimeColumn("reconcileDate"));
  fields.append(new MyMoneyDbColumn("action", "varchar(16)"));
  fields.append(new MyMoneyDbColumn("reconcileFlag", "char(1)"));
  fields.append(new MyMoneyDbTextColumn("value", MyMoneyDbTextColumn::NORMAL, false, NOTNULL));
  fields.append(new MyMoneyDbColumn("valueFormatted", "text"));
  fields.append(new MyMoneyDbTextColumn("shares", MyMoneyDbTextColumn::NORMAL, false, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("sharesFormatted"));
  fields.append(new MyMoneyDbTextColumn("memo"));
  fields.append(new MyMoneyDbColumn("accountId", "varchar(32)", false, NOTNULL));
  fields.append(new MyMoneyDbColumn("checkNumber", "varchar(32)"));
  fields.append(new MyMoneyDbDatetimeColumn("postDate"));
  MyMoneyDbTable t("kmmSplits", fields);
  QStringList list;
  list << "accountId" << "txType";
  t.addIndex("kmmSplitsaccount_type", list, false);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::KeyValuePairs(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("kvpType", "varchar(16)", false, NOTNULL));
  fields.append(new MyMoneyDbColumn("kvpId", "varchar(32)"));
  fields.append(new MyMoneyDbColumn("kvpKey", "varchar(255)", false, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("kvpData"));
  MyMoneyDbTable t("kmmKeyValuePairs", fields);
  QStringList list;
  list << "kvpType" << "kvpId";
  t.addIndex("type_id", list, false);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Schedules(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("name", MyMoneyDbTextColumn::NORMAL, false, NOTNULL));
  fields.append(new MyMoneyDbIntColumn("type", MyMoneyDbIntColumn::TINY, UNSIGNED, false, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("typeString"));
  fields.append(new MyMoneyDbIntColumn("occurence", MyMoneyDbIntColumn::SMALL, UNSIGNED, false, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("occurenceString"));
  fields.append(new MyMoneyDbIntColumn("paymentType", MyMoneyDbIntColumn::TINY, UNSIGNED));
  fields.append(new MyMoneyDbTextColumn("paymentTypeString", MyMoneyDbTextColumn::LONG));
  fields.append(new MyMoneyDbColumn("startDate", "date", false, NOTNULL));
  fields.append(new MyMoneyDbColumn("endDate", "date"));
  fields.append(new MyMoneyDbColumn("fixed", "char(1)", false, NOTNULL));
  fields.append(new MyMoneyDbColumn("autoEnter", "char(1)", false, NOTNULL));
  fields.append(new MyMoneyDbColumn("lastPayment", "date"));
  fields.append(new MyMoneyDbColumn("nextPaymentDue", "date"));
  fields.append(new MyMoneyDbIntColumn("weekendOption", MyMoneyDbIntColumn::TINY, UNSIGNED, false, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("weekendOptionString"));
  MyMoneyDbTable t("kmmSchedules", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::SchedulePaymentHistory(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("schedId", "varchar(32)", PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbColumn("payDate", "date", PRIMARYKEY,  NOTNULL));
  MyMoneyDbTable t("kmmSchedulePaymentHistory", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Securities(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbColumn("name", "text", false, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("symbol"));
  fields.append(new MyMoneyDbIntColumn("type", MyMoneyDbIntColumn::SMALL, UNSIGNED, false, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("typeString"));
  fields.append(new MyMoneyDbColumn("smallestAccountFraction", "varchar(24)"));
  fields.append(new MyMoneyDbTextColumn("tradingMarket"));
  fields.append(new MyMoneyDbColumn("tradingCurrency", "char(3)"));
  MyMoneyDbTable t("kmmSecurities", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Prices(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("fromId", "varchar(32)", PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbColumn("toId", "varchar(32)",  PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbColumn("priceDate", "date", PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("price", MyMoneyDbTextColumn::NORMAL, false, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("priceFormatted"));
  fields.append(new MyMoneyDbTextColumn("priceSource"));
  MyMoneyDbTable t("kmmPrices", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Currencies(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("ISOcode", "char(3)", PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("name", MyMoneyDbTextColumn::NORMAL, false, NOTNULL));
  fields.append(new MyMoneyDbIntColumn("type", MyMoneyDbIntColumn::SMALL, UNSIGNED));
  fields.append(new MyMoneyDbTextColumn("typeString"));
  fields.append(new MyMoneyDbIntColumn("symbol1", MyMoneyDbIntColumn::SMALL, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("symbol2", MyMoneyDbIntColumn::SMALL, UNSIGNED));
  fields.append(new MyMoneyDbIntColumn("symbol3", MyMoneyDbIntColumn::SMALL, UNSIGNED));
  fields.append(new MyMoneyDbColumn("symbolString", "varchar(255)"));
  fields.append(new MyMoneyDbColumn("partsPerUnit", "varchar(24)"));
  fields.append(new MyMoneyDbColumn("smallestCashFraction", "varchar(24)"));
  fields.append(new MyMoneyDbColumn("smallestAccountFraction", "varchar(24)"));
  MyMoneyDbTable t("kmmCurrencies", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Reports(void) {
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("name", "varchar(255)", PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("XML", MyMoneyDbTextColumn::LONG));
  MyMoneyDbTable t("kmmReportConfig", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

void MyMoneyDbDef::Budgets(void){
  QValueList<KSharedPtr <MyMoneyDbColumn> > fields;
  fields.append(new MyMoneyDbColumn("id", "varchar(32)", PRIMARYKEY, NOTNULL));
  fields.append(new MyMoneyDbColumn("name", "text", false, NOTNULL));
  fields.append(new MyMoneyDbColumn("start", "date", false, NOTNULL));
  fields.append(new MyMoneyDbTextColumn("XML", MyMoneyDbTextColumn::LONG));
  MyMoneyDbTable t("kmmBudgetConfig", fields);
  t.buildSQLStrings();
  m_tables[t.name()] = t;
}

// function to write create SQL to a stream
const QString MyMoneyDbDef::generateSQL (const QString& driver) const {
  QString retval;
  QMapConstIterator<QString, MyMoneyDbTable> tt = m_tables.begin();
  while (tt != m_tables.end()) {
    retval += (*tt).generateCreateSQL(driver) + '\n';    ++tt;
  }
  return retval;
}

//*****************************************************************************

void MyMoneyDbTable::addIndex(const QString& name, const QStringList& columns, bool unique) {
  m_indices.push_back (MyMoneyDbIndex (m_name, name, columns, unique));
}

void MyMoneyDbTable::buildSQLStrings (void) {
  // build fixed SQL strings for this table
  // build the insert string with placeholders for each field
  QString qs = QString("INSERT INTO %1 VALUES (").arg(name());
  field_iterator ft = m_fields.begin();
  while (ft != m_fields.end()) {
    qs += QString(":%1, ").arg((*ft)->name());
    ++ft;
  }
  m_insertString = qs.left(qs.length() - 2) + ");";
  // build a 'select all' string (select * is deprecated)
  // don't terminate with semicolon coz we may want a where or order clause
  qs = QString("SELECT ");
  ft = m_fields.begin();
  while (ft != m_fields.end()) {
    qs += QString("%1, ").arg((*ft)->name());
    ++ft;
  }
  m_selectAllString = qs.left(qs.length() - 2) + " FROM " + name();
  // build an update string; key fields go in the where clause
  qs = QString("UPDATE " + name() + " SET ");
  QString ws = QString();
  ft = m_fields.begin();
  while (ft != m_fields.end()) {
    if ((*ft)->isPrimaryKey()) {
      if (!ws.isEmpty()) ws += " AND ";
      ws += QString("%1 = :%2").arg((*ft)->name()).arg((*ft)->name());
    } else {
      qs += QString("%1 = :%2, ").arg((*ft)->name()).arg((*ft)->name());
    }
    ++ft;
  }
  qs = qs.left(qs.length() - 2);
  if (!ws.isEmpty()) qs += QString(" WHERE " + ws);
  m_updateString = qs + ";";
  // build a delete string; where clause as for update
  qs = QString("DELETE FROM " + name());
  if (!ws.isEmpty()) qs += QString(" WHERE " + ws);
  m_deleteString = qs + ";";
 }


const QString MyMoneyDbTable::generateCreateSQL (const QString& driver) const {
  QString qs = QString("CREATE TABLE %1 (").arg(name());
  QString pkey;
  for (field_iterator it = m_fields.begin(); it != m_fields.end(); ++it) {
    qs += (*it)->generateDDL (driver) + ", ";
    if ((*it)->isPrimaryKey ())
      pkey += (*it)->name () + ", ";
  }

  if (!pkey.isEmpty()) {
    qs += "PRIMARY KEY (" + pkey;
    qs = qs.left(qs.length() -2) + "));\n";
  } else {
    qs = qs.left(qs.length() -2) + ");\n";
  }
  for (index_iterator ii = m_indices.begin(); ii != m_indices.end(); ++ii) {
    qs += (*ii).generateDDL(driver);
  }
  return qs;
}

const QString MyMoneyDbTable::dropPrimaryKeyString(const QString& driver) const {
  if (driver == "QMYSQL3")
    return "ALTER TABLE " + m_name + " DROP PRIMARY KEY;";
  else if (driver == "QPSQL7")
    return "ALTER TABLE " + m_name + " DROP CONSTRAINT " + m_name + "_pkey;";
  else if (driver == "QSQLITE")
    return "";
}

const QString MyMoneyDbTable::modifyColumnString(const QString& driver, const QString& columnName, const MyMoneyDbColumn& newDef) const {
  QString qs = "ALTER TABLE " + m_name + " ";
  if (driver == "QMYSQL3")
    qs += "CHANGE " + columnName + " " + newDef.generateDDL(driver);
  else if (driver == "QPSQL7")
    qs += "ALTER COLUMN " + columnName + " TYPE " + newDef.generateDDL(driver).section(' ', 1);
  else if (driver == "QSQLITE")
    qs = "";

  return qs;
}

//*****************************************************************************
const QString MyMoneyDbIndex::generateDDL (const QString& driver) const {
  QString qs = "CREATE ";

  if (m_unique)
    qs += "UNIQUE ";

  qs += "INDEX " + m_table + "_" + m_name + "_idx ON "
       + m_table + " (";

  // The following should probably be revised.  MySQL supports an index on
  // partial columns, but not on a function.  Postgres supports an index on
  // the result of an SQL function, but not a partial column.  There should be
  // a way to merge these, and support other DBMSs like SQLite at the same time.
  // For now, if we just use plain columns, this will work fine.
  for (QStringList::const_iterator it = m_columns.begin(); it != m_columns.end(); ++it) {
    qs += *it + ",";
  }

  qs = qs.left(qs.length() - 1) + ");\n";

  return qs;
}

//*****************************************************************************
MyMoneyDbColumn*         MyMoneyDbColumn::clone () const
{ return (new MyMoneyDbColumn (*this)); }

MyMoneyDbIntColumn*      MyMoneyDbIntColumn::clone () const
{ return (new MyMoneyDbIntColumn (*this)); }

MyMoneyDbDatetimeColumn* MyMoneyDbDatetimeColumn::clone () const
{ return (new MyMoneyDbDatetimeColumn (*this)); }

MyMoneyDbTextColumn* MyMoneyDbTextColumn::clone () const
{ return (new MyMoneyDbTextColumn (*this)); }

const QString MyMoneyDbColumn::generateDDL (const QString& driver) const {
  QString qs = name() + " " + type();
  //if (isPrimaryKey()) qs += " PRIMARY KEY";
  if (isNotNull()) qs += " NOT NULL";
  return qs;
}

const QString MyMoneyDbIntColumn::generateDDL (const QString& driver) const {
  QString qs = name() + " ";

  switch (m_type) {
    case MyMoneyDbIntColumn::TINY:
      if (driver == "QMYSQL3" || driver == "QSQLITE") {
        qs += "tinyint ";
      } else if (driver == "QPSQL7") {
        qs += "int2 ";
      } else if (driver == "QDB2") {
        qs += "smallint ";
      } else {
        // cross your fingers...
        qs += "smallint ";
      }
      break;
    case MyMoneyDbIntColumn::SMALL:
      if (driver == "QMYSQL3" || driver == "QDB2" || driver == "QSQLITE") {
        qs += "smallint ";
      } else if (driver == "QPSQL7") {
        qs += "int2 ";
      } else {
        // cross your fingers...
        qs += "smallint ";
      }
      break;
    case MyMoneyDbIntColumn::MEDIUM:
      if (driver == "QMYSQL3" || driver == "QDB2") {
        qs += "int ";
      } else if (driver == "QPSQL7") {
        qs += "int4 ";
      } else if (driver == "QSQLITE") {
        qs += "integer ";
      } else {
        // cross your fingers...
        qs += "int ";
      }
      break;
    case MyMoneyDbIntColumn::BIG:
      if (driver == "QMYSQL3" || driver == "QDB2" || driver == "QSQLITE") {
        qs += "bigint ";
      } else if (driver == "QPSQL7") {
        qs += "int8 ";
      } else {
        // cross your fingers...
        qs += "bigint ";
      }
      break;
    default:
      qs += "int ";
      break;
  }

  if ((! m_isSigned) && (driver == "QMYSQL3" || driver == "QSQLITE")) {
    qs += "unsigned ";
  }

  //if (isPrimaryKey()) qs += " PRIMARY KEY";
  if (isNotNull()) qs += " NOT NULL";
  if ((! m_isSigned) && (driver == "QPSQL7")) {
    qs += " check(" + name() + " >= 0)";
  }
  return qs;
}

const QString MyMoneyDbTextColumn::generateDDL (const QString& driver) const {
  QString qs = name() + " ";

  switch (m_type) {
    case MyMoneyDbTextColumn::TINY:
      if (driver == "QMYSQL3" || driver == "QSQLITE") {
        qs += "tinytext ";
      } else if (driver == "QPSQL7") {
        qs += "text ";
      } else if (driver == "QDB2") {
        qs += "varchar(255) ";
      } else {
        // cross your fingers...
        qs += "tinytext ";
      }
      break;
    case MyMoneyDbTextColumn::NORMAL:
      if (driver == "QMYSQL3" || driver == "QSQLITE" || driver == "QPSQL7") {
        qs += "text ";
      } else if (driver == "QDB2") {
        qs += "clob(64K) ";
      } else {
        // cross your fingers...
        qs += "text ";
      }
      break;
    case MyMoneyDbTextColumn::MEDIUM:
      if (driver == "QMYSQL3" || driver == "QSQLITE") {
        qs += "mediumtext ";
      } else if (driver == "QPSQL7") {
        qs += "text ";
      } else if (driver == "QDB2") {
        qs += "clob(16M) ";
      } else {
        // cross your fingers...
        qs += "mediumtext ";
      }
      break;
    case MyMoneyDbTextColumn::LONG:
      if (driver == "QMYSQL3" || driver == "QSQLITE") {
        qs += "longtext ";
      } else if (driver == "QPSQL7") {
        qs += "text ";
      } else if (driver == "QDB2") {
        qs += "clob(2G) ";
      } else {
        // cross your fingers...
        qs += "longtext ";
      }
      break;
    default:
      qs += "text ";
      break;
  }

  //if (isPrimaryKey()) qs += " PRIMARY KEY";
  if (isNotNull()) qs += " NOT NULL";

  return qs;
}

const QString MyMoneyDbDatetimeColumn::generateDDL (const QString& driver) const {
  QString qs = name() + " ";
  if (driver == "QMYSQL3"  || driver == "QODBC3") {
    qs += "datetime ";
  } else if (driver == "QPSQL7" || driver == "QDB2" || driver == "QOCI8"|| driver == "QSQLITE") {
    qs += "timestamp ";
  } else {
    qs += "";
  }
  if (isNotNull()) qs += " NOT NULL";
  //if (isPrimaryKey()) qs += " PRIMARY KEY";
  return qs;
}
