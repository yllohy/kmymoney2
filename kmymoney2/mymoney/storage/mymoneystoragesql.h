/***************************************************************************
                          mymoneystoragesql.h
                          -------------------
    begin                : 11 November 2005
    copyright            : (C) 2005 by Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
 ***************************************************************************/

#ifndef MYMONEYSTORAGESQL_H
#define MYMONEYSTORAGESQL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlerror.h>

class QIODevice;

// ----------------------------------------------------------------------------
// Project Includes

#include "imymoneyserialize.h"
#include "imymoneystorageformat.h"
#include "../mymoneyinstitution.h"
#include "../mymoneypayee.h"
#include "../mymoneyaccount.h"
#include "../mymoneytransaction.h"
#include "../mymoneysplit.h"
#include "../mymoneyscheduled.h"
#include "../mymoneysecurity.h"
#include "../mymoneyprice.h"
#include "../mymoneyreport.h"
#include "../mymoneyfile.h"
#include "../mymoneykeyvaluecontainer.h"
#include "../mymoneymoney.h"

/**
@author Tony Bloomfield
 */

class IMyMoneyStorageFormat;

class dbField {
  public:
    dbField (const QString& iname, 
             const QString& itype = QString::null, 
             const bool iprimary = false, 
             const bool inotnull = false,
             const bool ikey = false,
             const QString &initVersion = "1.0"):
    m_name(iname),
    m_type(itype),
    m_isPrimary(iprimary),
    m_isNotNull(inotnull),
    m_isKey(ikey),
    m_initVersion(initVersion) 
    {};
    dbField (void) {};
    
    const QString& name(void) const {return (m_name);}
    const QString& type(void) const {return (m_type);}
    const bool isPrimaryKey(void) const {return (m_isPrimary);}
    const bool isNotNull(void) const {return (m_isNotNull);}
    const bool isKey(void) const {return (m_isKey);}
  private:
    QString m_name;
    QString m_type;
    bool m_isPrimary;
    bool m_isNotNull;
    bool m_isKey;
    QString m_initVersion;
};

class dbTable {
  public:
    dbTable (const QString& iname, 
             const QValueList<dbField>& ifields,
             const QString& initVersion = "1.0"):
      m_name(iname),
    m_fields(ifields),
    m_initVersion(initVersion)
    {};
    dbTable (void) {};
    const QString& name(void) const {return (m_name);}
    const QString& insertString(void) const {return (m_insertString);};
    const QString selectAllString(bool terminate = true) const
      {return (terminate ? QString(m_selectAllString + ";") : m_selectAllString);};
    const QString updateString(void) const {return (m_updateString);};
    void buildSQLStrings(void);
    
    typedef QValueList<dbField>::const_iterator field_iterator;
    field_iterator begin(void) const {return m_fields.constBegin();}
    field_iterator end(void) const {return m_fields.constEnd(); }
  private:
    QString m_name;
    QValueList<dbField> m_fields;
    QString m_initVersion;
    QString m_insertString; // string to insert a record
    QString m_selectAllString; // to select all fields
    QString m_updateString;  // normal string for record update
};

class dbDef  {
  friend class MyMoneyStorageSql;
  public:
    dbDef();
    ~dbDef() {};
  
    void generateSQL (QTextStream& fileName);
  
    typedef QMap<QString, dbTable>::const_iterator table_iterator;
    table_iterator begin(void) const {return m_tables.constBegin();}
    table_iterator end(void) const {return m_tables.constEnd();}
  
  private:
#define TABLE(name) void name();
  TABLE(FileInfo);
//  TABLE(User);
  TABLE(Institutions);
  TABLE(Payees);
  TABLE(Accounts);
  TABLE(Transactions);
  TABLE(Splits);
  TABLE(KeyValuePairs);
  TABLE(Schedules);
  TABLE(SchedulePaymentHistory);
  TABLE(Securities);
  TABLE(Prices);
  TABLE(Currencies);
  TABLE(Reports);
  protected:
  QMap<QString, dbTable> m_tables;
};

class MyMoneyStorageSql : public QSqlDatabase, IMyMoneyStorageFormat {
  public:
  /**
   * MyMoneyStorageSql constructor
   *
   * @param driver : Qt driver name (see QSqlDatabase class)
   *
   * @return void
   *
   */
    MyMoneyStorageSql (const QString& driver);

    ~MyMoneyStorageSql() {};
  /**
     * MyMoneyStorageSql - open database file
     *
     * @param dbName : name of database (file name for SQLite)
     * @param hostName : name of connected host where server resides
     * @param userName : name of user for database permission checking
     * @param password : user's password where required
     *
     * @return 0 - database successfully opened
     * @return 1 - database not opened, but may be able to create it
     * @return -1 - database not opened, cannot create it
     *
   */
    int open(const QString& dbName, const QString& hostName,
             const QString& userName, const QString& password);
  /**
     * MyMoneyStorageSql create database 
     *
     * @param dbName : name of database (file name for SQLite)
     * @param hostName : name of connected host where server resides
     * @param userName : name of user for database permission checking
     * @param password : user's password where required
     *
     * @return void
     *
   */
    void createDatabase(const QString& dbName, const QString& hostName,
                        const QString& userName, const QString& password);
  /**
     * MyMoneyStorageSql set storage
     *
     * @param storage : pointer to MyMoneySerialize storage 
     *
     * @return void
     *
   */
    void setStorage(IMyMoneySerialize *storage) {m_storage = storage;};
  /**
     * MyMoneyStorageSql read all the database into storage
     *
     * @return void
     *
   */
    void readFile(void);
  /**
     * MyMoneyStorageSql write/update the database from storage
     *
     * @return void
     *
   */
    void writeFile(void);
  /**
     * MyMoneyStorageSql generalized error display routine
     *
     * @param message : error message to be displayed
     *
     * @return void
     *
   */
    void displayError(const QString& message);
  protected:
    void readFile(QIODevice* qf, IMyMoneySerialize* storage) {};
    void writeFile(QIODevice* qf, IMyMoneySerialize* storage) {};
    void setProgressCallback(void(*callback)(int, int, const QString&));
  private:
    void clean (void);
    void writeUserInformation(void);
    void writeInstitutions(void);
    void writePayees(void);
    void writeAccounts(void);
    void writeTransactions(void);
    void writeSchedules(void);
    void writeSecurities(void);
    void writePrices(void);
    void writeCurrencies(void);
    void writeFileInfo(void);
    void writeReports(void);
  
    void readFileInfo(void);
    void readUserInformation(void);
    void readInstitutions(void);
    void readPayees(void);
    void readAccounts(void);
    void readTransactions(void);
    void readTransaction(MyMoneyTransaction &tx, const QString& tid);
    void readSchedules(void); 
    void readSecurities(void);
    void readPrices(void);
    void readCurrencies(void);
    void readReports(void);

    void createTable(const dbTable& t);
  
    void writeInstitution(const MyMoneyInstitution& i, QSqlQuery& q);
    void writePayee(const MyMoneyPayee& p, QSqlQuery& q);
    void writeAccount (const MyMoneyAccount& a, QSqlQuery& q);
    void writeTransaction(const QString& txId, const MyMoneyTransaction& tx, QSqlQuery& q, const QString& type);
    void writeSplits(const QString& txId, const QValueList<MyMoneySplit>& splitList);
    void writeSplit(const QString& txId, const MyMoneySplit& split, const int splitId, QSqlQuery& q);
    void writeSchedule(const MyMoneySchedule& sch, QSqlQuery& q);
    void writeSecurity(const MyMoneySecurity& security, QSqlQuery& q);
    void writePricePair (const QString& from, const QString& to, const MyMoneyPriceEntries& p);
    void writePrice (const QString& from, const QString& to, const MyMoneyPrice& p);
    void writeCurrency(const MyMoneySecurity& currency, QSqlQuery& q);
    void writeKeyValuePairs(const QString& kvpType, const QString& kvpId, const QMap<QCString, QString>& pairs);
    void writeKeyValuePair(const QString& kvpType, const QString& kvpId,
                           const QString& kvpKey, const QString& kvpData);
    void deleteTransaction(const QString& id);
    void deleteKeyValuePairs(const QString& kvpType, const QString& kvpId);
    const long long unsigned calcHighId (const long long unsigned, const QString&);
    //
    void readTransaction(const QString id);
    void readSplits(MyMoneyTransaction& tx, const QString& txId);
    MyMoneyKeyValueContainer readKeyValuePairs (const QString kvpType, const QString& kvpId);
    
    void signalProgress(int current, int total, const QString& = "");
    QString buildError (const QSqlQuery&, const QString&);
    // data 
    dbDef m_db;
    unsigned int m_majorVersion;
    unsigned int m_minorVersion;
    IMyMoneySerialize *m_storage;
    // record counts
    long long unsigned m_institutions;
    long long unsigned m_accounts;
    long long unsigned m_payees;
    long long unsigned m_transactions;
    long long unsigned m_splits;
    long long unsigned m_securities;
    long long unsigned m_prices;
    long long unsigned m_currencies;
    long long unsigned m_schedules;
    long long unsigned m_reports;
    long long unsigned m_kvps;
  // next id to use (for future archive)
    long long unsigned m_hiIdInstitutions;
    long long unsigned m_hiIdPayees;
    long long unsigned m_hiIdAccounts;
    long long unsigned m_hiIdTransactions;
    long long unsigned m_hiIdSchedules;
    long long unsigned m_hiIdSecurities;
    long long unsigned m_hiIdReports;
    // encrypt option - usage TBD
    QString m_encryptData;
    void (*m_progressCallback)(int, int, const QString&);
};

#endif // MYMONEYSTORAGESQL_H