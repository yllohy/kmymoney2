/***************************************************************************
                          mymoneystoragesql.h
                          -------------------
    begin                : 11 November 2005
    copyright            : (C) 2005 by Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
                         : Fernando Vilas <fvilas@iname.com>
 ***************************************************************************/

#ifndef MYMONEYSTORAGESQL_H
#define MYMONEYSTORAGESQL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qvaluestack.h>

class QIODevice;
// ----------------------------------------------------------------------------
// KDE Includes

#include <kurl.h>
#include <ksharedptr.h>

// ----------------------------------------------------------------------------
// Project Includes

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
#include "../mymoneybudget.h"
#include "../mymoneyfile.h"
#include "../mymoneykeyvaluecontainer.h"
#include "../mymoneymoney.h"
#include "../mymoneytransactionfilter.h"

/**
@author Tony Bloomfield
 */

class MyMoneySqlQuery : public QSqlQuery {
  public:
    MyMoneySqlQuery (QSqlDatabase* db) : QSqlQuery (db) {};
    MyMoneySqlQuery () : QSqlQuery() {};
    bool exec ();
};

class MyMoneyDbColumn : public KShared {
  public:
    MyMoneyDbColumn (const QString& iname,
             const QString& itype = QString::null,
             const bool iprimary = false,
             const bool inotnull = false,
             const QString &initVersion = "0.1"):
    m_name(iname),
    m_type(itype),
    m_isPrimary(iprimary),
    m_isNotNull(inotnull),
    m_initVersion(initVersion)
    {};
    MyMoneyDbColumn (void) {};
    virtual ~MyMoneyDbColumn () {};

    virtual MyMoneyDbColumn* clone () const;

    virtual const QString generateDDL (const QString& driver) const;

    const QString& name(void) const {return (m_name);}
    const QString& type(void) const {return (m_type);}
    const bool isPrimaryKey(void) const {return (m_isPrimary);}
    const bool isNotNull(void) const {return (m_isNotNull);}
  private:
    QString m_name;
    QString m_type;
    bool m_isPrimary;
    bool m_isNotNull;
    QString m_initVersion;
};

class MyMoneyDbDatetimeColumn : public MyMoneyDbColumn {
  public:
    MyMoneyDbDatetimeColumn (const QString& iname,
                             const bool iprimary = false,
                             const bool inotnull = false,
             const QString &initVersion = "0.1"):
      MyMoneyDbColumn (iname, "", iprimary, inotnull, initVersion)
      {};
    virtual const QString generateDDL (const QString& driver) const;
    virtual MyMoneyDbDatetimeColumn* clone () const;
  private:
    static const QString calcType(void);
};

class MyMoneyDbIntColumn : public MyMoneyDbColumn {
  public:
    enum size {TINY, SMALL, MEDIUM, BIG};
    MyMoneyDbIntColumn (const QString& iname,
                        const size type = MEDIUM,
                        const bool isigned = true,
                        const bool iprimary = false,
                        const bool inotnull = false,
             const QString &initVersion = "0.1"):
        MyMoneyDbColumn (iname, "", iprimary, inotnull, initVersion),
    m_type  (type),
    m_isSigned (isigned)
    {};
    virtual const QString generateDDL (const QString& driver) const;
    virtual MyMoneyDbIntColumn* clone () const;
  private:
    size m_type;
    bool m_isSigned;
};

class MyMoneyDbTextColumn : public MyMoneyDbColumn {
  public:
    enum size {TINY, NORMAL, MEDIUM, LONG};
    MyMoneyDbTextColumn (const QString& iname,
                         const size type = MEDIUM,
                         const bool iprimary = false,
                         const bool inotnull = false,
             const QString &initVersion = "0.1"):
        MyMoneyDbColumn (iname, "", iprimary, inotnull, initVersion),
    m_type  (type)
    {};

    virtual const QString generateDDL (const QString& driver) const;
    virtual MyMoneyDbTextColumn* clone () const;
  private:
    size m_type;
};

class MyMoneyDbIndex {
  public:
    MyMoneyDbIndex (const QString& table,
                    const QString& name,
                    const QStringList& columns,
                    bool unique = false):
      m_table(table),
      m_unique(unique),
      m_name(name),
      m_columns(columns)
      {}
    MyMoneyDbIndex () {}
    inline const QString table () const {return m_table;}
    inline bool isUnique () const {return m_unique;}
    inline const QString name () const {return m_name;}
    inline const QStringList columns () const {return m_columns;}
    const QString generateDDL (const QString& driver) const;
  private:
    QString m_table;
    bool m_unique;
    QString m_name;
    QStringList m_columns;
};

class MyMoneyDbTable {
  public:
    MyMoneyDbTable (const QString& iname,
             const QValueList<KSharedPtr <MyMoneyDbColumn> >& ifields,
             const QString& initVersion = "1.0"):
    m_name(iname),
    m_fields(ifields),
    m_initVersion(initVersion) {};
    MyMoneyDbTable (void) {};

    const QString& name(void) const {return (m_name);}
    const QString& insertString(void) const {return (m_insertString);};
    const QString selectAllString(bool terminate = true) const
      {return (terminate ? QString(m_selectAllString + ";") : m_selectAllString);};
    const QString updateString(void) const {return (m_updateString);};
    const QString deleteString(void) const {return (m_deleteString);};
    const QString dropPrimaryKeyString(const QString& driver) const;
    const QString modifyColumnString(const QString& driver, const QString& columnName, const MyMoneyDbColumn& newDef) const;
    void buildSQLStrings(void);
    const QString generateCreateSQL (const QString& driver) const;
    void addIndex(const QString& name, const QStringList& columns, bool unique = false);

    typedef QValueList<KSharedPtr <MyMoneyDbColumn> >::const_iterator field_iterator;
    field_iterator begin(void) const {return m_fields.constBegin();}
    field_iterator end(void) const {return m_fields.constEnd(); }
  private:
    QString m_name;
    QValueList<KSharedPtr <MyMoneyDbColumn> > m_fields;

    typedef QValueList<MyMoneyDbIndex>::const_iterator index_iterator;
    QValueList<MyMoneyDbIndex> m_indices;
    QString m_initVersion;
    QString m_insertString; // string to insert a record
    QString m_selectAllString; // to select all fields
    QString m_updateString;  // normal string for record update
    QString m_deleteString; // string to delete 1 record
};

class MyMoneyDbDef  {
  friend class MyMoneyStorageSql;
public:
    MyMoneyDbDef();
    ~MyMoneyDbDef() {};

    const QString generateSQL (const QString& driver) const;

    typedef QMap<QString, MyMoneyDbTable>::const_iterator table_iterator;
    table_iterator begin(void) const {return m_tables.constBegin();}
    table_iterator end(void) const {return m_tables.constEnd();}
    const unsigned int currentVersion() const {return (m_currentVersion);};

private:
  static unsigned int m_currentVersion; // The current version of the database layout
#define TABLE(name) void name();
  TABLE(FileInfo);
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
  TABLE(Budgets);
protected:
  QMap<QString, MyMoneyDbTable> m_tables;
};

class IMyMoneySerialize;

class MyMoneyStorageSql : public IMyMoneyStorageFormat, public QSqlDatabase {
public:
  MyMoneyStorageSql (IMyMoneySerialize *storage, const KURL& = KURL());
  ~MyMoneyStorageSql() {};

    /**
   * MyMoneyStorageSql - open database file
   *
   * @param url pseudo-URL of database to be opened
   * @param openMode open mode, same as for QFile::open
   * @param clear whether existing data can be deleted

   * @return 0 - database successfully opened
   * @return 1 - database not opened, use lastError function for reason
   * @return -1 - output database not opened, contains data, clean not specified
   *
     */
  int open(const KURL& url, int openMode, bool clear = false);
  /**
   * MyMoneyStorageSql close the database
   *
   * @return void
   *
   */
  void close(bool logoff = true);
  /**
   * MyMoneyStorageSql read all the database into storage
   *
   * @return void
   *
   */
  bool readFile(void);
  /**
   * MyMoneyStorageSql write/update the database from storage
   *
   * @return void
   *
   */
  bool writeFile(void);

    /**
   * MyMoneyStorageSql generalized error routine
   *
   * @return : error message to be displayed
   *
     */
  const QString& lastError() {return (m_error);};
  /**
   * This method is used when a database file is open, and the data is to
   * be saved in a different file or format. It will ensure that all data
   * from the database is available in memory to enable it to be written.
   */
  virtual void fillStorage();
  /**
    * The following functions correspond to the identically named (usually) functions
    * within the Storage Manager, and are called to update the database
    */
  void modifyUserInfo(const MyMoneyPayee& payee);
  void addInstitution(const MyMoneyInstitution& inst);
  void modifyInstitution(const MyMoneyInstitution& inst);
  void removeInstitution(const MyMoneyInstitution& inst);
  void addPayee(const MyMoneyPayee& payee);
  void modifyPayee(const MyMoneyPayee& payee);
  void removePayee(const MyMoneyPayee& payee);
  void addAccount(const MyMoneyAccount& acc);
  void modifyAccount(const MyMoneyAccount& acc);
  void removeAccount(const MyMoneyAccount& acc);
  void addTransaction(const MyMoneyTransaction& tx);
  void modifyTransaction(const MyMoneyTransaction& tx);
  void removeTransaction(const MyMoneyTransaction& tx);
  void addSchedule(const MyMoneySchedule& sch);
  void modifySchedule(const MyMoneySchedule& sch);
  void removeSchedule(const MyMoneySchedule& sch);
  void addSecurity(const MyMoneySecurity& sec);
  void modifySecurity(const MyMoneySecurity& sec);
  void removeSecurity(const MyMoneySecurity& sec);
  void addPrice(const MyMoneyPrice& p);
  void removePrice(const MyMoneyPrice& p);
  void addCurrency(const MyMoneySecurity& sec);
  void modifyCurrency(const MyMoneySecurity& sec);
  void removeCurrency(const MyMoneySecurity& sec);
  void addReport(const MyMoneyReport& rep);
  void modifyReport(const MyMoneyReport& rep);
  void removeReport(const MyMoneyReport& rep);
  void addBudget(const MyMoneyBudget& bud);
  void modifyBudget(const MyMoneyBudget& bud);
  void removeBudget(const MyMoneyBudget& bud);

  const unsigned long transactionCount  (const QCString& aid = QCString());
  const QMap<QCString, unsigned long> transactionCountMap ()
      {return (m_transactionCountMap);};
  /**
    * the storage manager also needs the following read entry points
    */
  void readPayees(const QCString&);
  void readPayees(const QValueList<QCString> payeeList = QValueList<QCString>());
  void readTransactions(const MyMoneyTransactionFilter& filter);
  void setProgressCallback(void(*callback)(int, int, const QString&));

  virtual void readFile(QIODevice* s, IMyMoneySerialize* storage) { Q_UNUSED(s); Q_UNUSED(storage) };
  virtual void writeFile(QIODevice* s, IMyMoneySerialize* storage){ Q_UNUSED(s); Q_UNUSED(storage) };

private:
  // a function to build a comprehensive error message
  QString& buildError (const MyMoneySqlQuery& q, const QString& function, const QString& message);
  // write routines
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
  void writeBudgets(void);

  void writeInstitution(const MyMoneyInstitution& i, MyMoneySqlQuery& q);
  void writePayee(const MyMoneyPayee& p, MyMoneySqlQuery& q, bool isUserInfo = false);
  void writeAccount (const MyMoneyAccount& a, MyMoneySqlQuery& q);
  void writeTransaction(const QString& txId, const MyMoneyTransaction& tx, MyMoneySqlQuery& q, const QString& type);
  void writeSplits(const QString& txId, const QString& type, const QValueList<MyMoneySplit>& splitList);
  void writeSplit(const QString& txId, const MyMoneySplit& split, const QString& type, const int splitId, MyMoneySqlQuery& q);
  void writeSchedule(const MyMoneySchedule& sch, MyMoneySqlQuery& q, bool insert);
  void writeSecurity(const MyMoneySecurity& security, MyMoneySqlQuery& q);
  void writePricePair ( const MyMoneyPriceEntries& p);
  void writePrice (const MyMoneyPrice& p);
  void writeCurrency(const MyMoneySecurity& currency, MyMoneySqlQuery& q);
  void writeReport (const MyMoneyReport& rep, MyMoneySqlQuery& q);
  void writeBudget (const MyMoneyBudget& bud, MyMoneySqlQuery& q);
  void writeKeyValuePairs(const QString& kvpType, const QString& kvpId, const QMap<QCString, QString>& pairs);
  void writeKeyValuePair(const QString& kvpType, const QString& kvpId,
                         const QString& kvpKey, const QString& kvpData);
  // read routines
  void readFileInfo(void);
  void readLogonData(void);
  void readUserInformation(void);
  void readInstitutions(void);
  void readAccounts(void);
  void readTransaction(const QString id);
  void readTransactions(const QString& tidList = QString(), const QString& dateClause = QString());
  void readTransaction(MyMoneyTransaction &tx, const QString& tid);
  void readSplit (MyMoneySplit& s, const MyMoneySqlQuery& q, const MyMoneyDbTable& t);
  MyMoneyKeyValueContainer readKeyValuePairs (const QString kvpType, const QString& kvpId);
  QMap<QString, MyMoneyKeyValueContainer> readKeyValuePairs (const QString kvpType, const QStringList& kvpIdList);
  void readSchedules(void);
  void readSecurities(void);
  void readPrices(void);
  void readCurrencies(void);
  void readReports(void);
  void readBudgets(void);

  void deleteTransaction(const QString& id);
  void deleteSchedule(const QString& id);
  void deleteKeyValuePairs(const QString& kvpType, const QString& kvpId);
  const long long unsigned calcHighId (const long long unsigned, const QString&);

  void setVersion (const QString& version);

  void signalProgress(int current, int total, const QString& = "");
  void (*m_progressCallback)(int, int, const QString&);

  void startCommitUnit (const QString& callingFunction);
  void endCommitUnit (const QString& callingFunction);
  void cancelCommitUnit (const QString& callingFunction);

  // open routines
  /**
   * MyMoneyStorageSql create database
   *
   * @param url pseudo-URL of database to be opened
   *
   * @return true - creation successful
   * @return false - could not create
   *
   */
  int createDatabase(const KURL& url);
  int upgradeDb();
  int upgradeToV1();
  long long unsigned getRecCount(const QString& table);
  int createTables();
  void createTable(const MyMoneyDbTable& t);
  void clean ();
  int isEmpty();
  // data

  typedef enum databaseTypeE {
    Db2 = 0, //
    Interbase, //
    Mysql, //
    Oracle8, //
    ODBC3, //
    Postgresql, //
    Sqlite, //
    Sybase //
  } _databaseType;

  QMap<QString, QString> m_driverMap;
  databaseTypeE m_dbType;

  MyMoneyDbDef m_db;
  unsigned int m_majorVersion;
  unsigned int m_minorVersion;
  IMyMoneySerialize *m_storage;
  IMyMoneyStorage *m_storagePtr;
  // input options
  unsigned int m_mode; // 0 = old method (file mode) 1 = single-user, 2 = multi-user (NYI)

  bool m_loadAll; // preload all data
  bool m_override; // override open if already in use

  bool m_isDbaseMode;
  // error message
  QString m_error;
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
  long long unsigned m_budgets;
  // next id to use (for future archive)
  long long unsigned m_hiIdInstitutions;
  long long unsigned m_hiIdPayees;
  long long unsigned m_hiIdAccounts;
  long long unsigned m_hiIdTransactions;
  long long unsigned m_hiIdSchedules;
  long long unsigned m_hiIdSecurities;
  long long unsigned m_hiIdReports;
  long long unsigned m_hiIdBudgets;
  // encrypt option - usage TBD
  QString m_encryptData;

  /**
    * This variable is used to suppress status messages except during
   * initial data load and final write

  */
  bool m_displayStatus;
  /**
   * On occasions, e.g. after a complex transaction search, or for populating a
   * payee popup list, it becomes necessary to load all data into memory. The
   * following flags will be set after such a load, to indicate that further
   * retrievals are not needed.
   */
  bool m_transactionListRead;
  bool m_payeeListRead;
  /**
   * This member variable holds a list of those accounts for which all
   * transactions are in memory, thus saving reading them again
   */
  QValueList<QCString> m_accountsLoaded;
  /**
    * This member variable is used when loading transactions to list all
    * referenced payees, which can then be read into memory (if not already there)
    */
  QValueList<QCString> m_payeeList;

  void alert(QString s) {qDebug("%s", s.data());} // FIXME: remove...
  /** The following keeps track of commitment units (known as transactions in SQL
    * though it would be confusing to use that term within KMM). It is implemented
    * as a stack for debug purposes. Long term, probably a count would suffice
    */
  QValueStack<QString> m_commitUnitStack;
  /**
   * These member variables hold the SQL statements necessary to start and end commit units.
   * Currently, they seem to be the same for each supported DBMS, but that may not always
   * be the case. The values will be set in the constructor if the database is open
   */
  QString m_startCommitUnitStatement;
  QString m_endCommitUnitStatement;
  QString m_cancelCommitUnitStatement;
  /**
    * This member variable is used to preload transactions for preferred accounts
    */
  MyMoneyTransactionFilter m_preferred;
  /**
    * This member variable is used because reading prices from a file uses the 'add...' function rather than a
    * 'load...' function which other objects use. Having this variable allows us to avoid needing to check the
    * database to see if this really is a new or modified price
    */
  bool m_readingPrices;
  /**
    * This member variable holds a map of transaction counts per account, indexed by
    * the account id. It is used
    * to avoid having to scan all transactions whenever a count is needed. It should
    * probably be moved into the MyMoneyAccount object; maybe we will do that once
    * the database code has been properly checked out
    */
  QMap<QCString, unsigned long> m_transactionCountMap;
  /**
    * These member variables hold the user name and date/time of logon
    */
  QString m_logonUser;
  QDateTime m_logonAt;
  QDateTime m_txPostDate; // FIXME: remove when Tom puts date into split object
};
#endif // MYMONEYSTORAGESQL_H
