/***************************************************************************
                          mymoneyreport.h
                             -------------------
    begin                : Sun July 4 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : acejones@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYREPORT_H
#define MYMONEYREPORT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes
#include <qmap.h>
#include <qvaluelist.h>
#include <qstring.h>
class QDomElement;
class QDomDocument;

// ----------------------------------------------------------------------------
// Project Includes
#include <kmymoney/mymoneyobject.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneytransactionfilter.h>
#include <kmymoney/export.h>

/**
  * This class defines a report within the MyMoneyEngine.  The report class
  * contains all the configuration parameters needed to run a report, plus
  * XML serialization.
  *
  * A report is a transactionfilter, so any report can specify which
  * transactions it's interested down to the most minute level of detail.
  * It extends the transactionfilter by providing identification (name,
  * comments, group type, etc) as well as layout information (what kind
  * of layout should be used, how the rows & columns should be presented,
  * currency converted, etc.)
  *
  * As noted above, this class only provides a report DEFINITION.  The
  * generation and presentation of the report itself are left to higher
  * level classes.
  *
  * @author Ace Jones <acejones@users.sourceforge.net>
  */

class KMYMONEY_EXPORT MyMoneyReport: public MyMoneyObject, public MyMoneyTransactionFilter
{
public:
  // When adding a new row type, be sure to add a corresponding entry in kTypeArray
  enum ERowType { eNoRows = 0, eAssetLiability, eExpenseIncome, eCategory, eTopCategory, eAccount, ePayee, eMonth, eWeek, eTopAccount, eAccountByTopAccount, eEquityType, eAccountType, eInstitution, eBudget, eBudgetActual, eSchedule, eAccountInfo, eAccountLoanInfo, eAccountReconcile, eCashFlow};
  enum EReportType { eNoReport = 0, ePivotTable, eQueryTable, eInfoTable };
  enum EColumnType { eNoColumns = 0, eDays = 1, eMonths = 1, eBiMonths = 2, eQuarters = 3, eWeeks = 7, eYears = 12 };

  // if you add bits to this bitmask, start with the value currently assigned to eQCend and update its value afterwards
  // also don't forget to add column names to kQueryColumnsText in mymoneyreport.cpp
  enum EQueryColumns { eQCnone = 0x0, eQCbegin = 0x1, eQCnumber = 0x1, eQCpayee = 0x2, eQCcategory = 0x4, eQCmemo = 0x8, eQCaccount = 0x10, eQCreconciled = 0x20, eQCaction = 0x40, eQCshares = 0x80, eQCprice = 0x100, eQCperformance = 0x200, eQCloan = 0x400, eQCbalance = 0x800, eQCend = 0x1000 };

  enum EDetailLevel { eDetailNone = 0, eDetailAll, eDetailTop, eDetailGroup, eDetailTotal, eDetailEnd };
  enum EChartType { eChartNone = 0, eChartLine, eChartBar, eChartPie, eChartRing, eChartStackedBar, eChartEnd };

  static const QStringList kRowTypeText;
  static const QStringList kColumnTypeText;
  static const QStringList kQueryColumnsText;
  static const QStringList kDetailLevelText;
  static const QStringList kChartTypeText;
  static const EReportType kTypeArray[];

public:
  MyMoneyReport(void);
  MyMoneyReport(ERowType _rt, unsigned _ct, dateOptionE _dl, EDetailLevel _ss, const QString& _name, const QString& _comment );
  MyMoneyReport(const QString& id, const MyMoneyReport& right);

  /**
    * This constructor creates an object based on the data found in the
    * QDomElement referenced by @p node. If problems arise, the @p id of
    * the object is cleared (see MyMoneyObject::clearId()).
    */
  MyMoneyReport(const QDomElement& node);

  // Simple get operations
  const QString& name(void) const { return m_name; }
  bool isShowingRowTotals(void) const { return (m_showRowTotals); }
  EReportType reportType(void) const { return m_reportType; }
  ERowType rowType(void) const { return m_rowType; }
  EColumnType columnType(void) const { return m_columnType; }
  bool isRunningSum(void) const { return (m_rowType==eAssetLiability); }
  bool isConvertCurrency(void) const { return m_convertCurrency; }
  unsigned columnPitch(void) const { return static_cast<unsigned>(m_columnType); }
  bool isShowingColumnTotals(void) const { return m_convertCurrency; }
  const QString& comment( void ) const { return m_comment; }
  EQueryColumns queryColumns(void) const { return m_queryColumns; }
  const QString& group( void ) const { return m_group; }
  bool isFavorite(void) const { return m_favorite; }
  bool isTax(void) const { return m_tax; }
  bool isInvestmentsOnly(void) const { return m_investments; }
  bool isLoansOnly(void) const { return m_loans; }
  EDetailLevel detailLevel(void) const { return m_detailLevel; }
  EChartType chartType(void) const { return m_chartType; }
  bool isChartDataLabels(void) const { return m_chartDataLabels; }
  bool isChartGridLines(void) const { return m_chartGridLines; }
  bool isChartByDefault(void) const { return m_chartByDefault; }
  uint chartLineWidth(void) const { return m_chartLineWidth; }
  bool isIncludingSchedules(void) const { return m_includeSchedules; }
  bool isColumnsAreDays(void) const { return m_columnsAreDays; }
  bool isIncludingTransfers(void) const { return m_includeTransfers; }
  bool isIncludingUnusedAccounts(void) const { return m_includeUnusedAccounts; }
  bool hasBudget(void) const { return !m_budgetId.isEmpty(); }
  const QString& budget(void) const { return m_budgetId; }
  bool isIncludingBudgetActuals(void) const { return m_includeBudgetActuals; }
  bool isIncludingForecast(void) const { return m_includeForecast; }
  bool isIncludingMovingAverage(void) const { return m_includeMovingAverage; }
  int movingAverageDays(void) const { return m_movingAverageDays; }
  bool isIncludingPrice(void) const { return m_includePrice; }
  bool isIncludingAveragePrice(void) const { return m_includeAveragePrice; }
  bool isUserDefined(void) const { return m_dateLock == userDefined; }

  // Simple set operations
  void setName(const QString& _s) { m_name = _s; }
  void setConvertCurrency(bool _f) { m_convertCurrency = _f; }
  void setRowType(ERowType _rt);
  void setColumnType(EColumnType _ct) { m_columnType = _ct; }
  void setComment( const QString& _comment ) { m_comment = _comment; }
  void setGroup( const QString& _group ) { m_group = _group; }
  void setFavorite(bool _f) { m_favorite = _f; }
  void setQueryColumns( EQueryColumns _qc ) { m_queryColumns = _qc; }
  void setTax(bool _f) { m_tax = _f; }
  void setInvestmentsOnly(bool _f) { m_investments = _f; if (_f) m_loans = false; }
  void setLoansOnly(bool _f) { m_loans = _f; if (_f) m_investments = false; }
  void setDetailLevel( EDetailLevel _detail ) { m_detailLevel = _detail; }
  void setChartType ( EChartType _type ) { m_chartType = _type; }
  void setChartDataLabels ( bool _f ) { m_chartDataLabels = _f; }
  void setChartGridLines ( bool _f ) { m_chartGridLines = _f; }
  void setChartByDefault ( bool _f ) { m_chartByDefault = _f; }
  void setChartLineWidth ( uint _f ) { m_chartLineWidth = _f; }
  void setIncludingSchedules( bool _f ) { m_includeSchedules = _f; }
  void setColumnsAreDays( bool _f ) { m_columnsAreDays = _f; }
  void setIncludingTransfers( bool _f ) { m_includeTransfers = _f; }
  void setIncludingUnusedAccounts( bool _f ) { m_includeUnusedAccounts = _f; }
  void setShowingRowTotals( bool _f ) { m_showRowTotals = _f; }
  void setIncludingBudgetActuals( bool _f ) { m_includeBudgetActuals = _f; }
  void setIncludingForecast( bool _f ) { m_includeForecast = _f; }
  void setIncludingMovingAverage( bool _f ) { m_includeMovingAverage = _f; }
  void setMovingAverageDays( int _days ) { m_movingAverageDays = _days; }
  void setIncludingPrice( bool _f ) { m_includePrice = _f; }
  void setIncludingAveragePrice( bool _f ) { m_includeAveragePrice = _f; }

  /**
    * Sets the budget used for this report
    *
    * @param _budget The ID of the budget to use, or an empty string
    * to indicate a budget is NOT included
    * @param _fa Whether to display actual data alongside the budget.
    * Setting to false means the report displays ONLY the budget itself.
    * @warning For now, the budget ID is ignored.  The budget id is
    * simply checked for any non-empty string, and if so, hasBudget()
    * will return true.
    */
  void setBudget( const QString& _budget, bool _fa = true ) { m_budgetId = _budget; m_includeBudgetActuals=_fa; }

  /**
    * This method allows you to clear the underlying transaction filter
    */
  void clear(void);

  /**
    * This method allows you to set the underlying transaction filter
    *
    * @param _filter The filter which should replace the existing transaction
    * filter.
    */
  void assignFilter(const MyMoneyTransactionFilter& _filter) { MyMoneyTransactionFilter::operator=(_filter); }

  /**
    * Set the underlying date filter and LOCK that filter to the specified
    * range.  For example, if @p _u is "CurrentMonth", this report should always
    * be updated to the current month no matter when the report is run.
    *
    * This updating is not entirely automatic, you should update it yourself by
    * calling updateDateFilter.
    *
    * @param _u The date range constant (MyMoneyTransactionFilter::dateRangeE)
    *          which this report should be locked to.
    */

  void setDateFilter(dateOptionE _u)
    {
      m_dateLock = _u;
      if (_u != userDefined)
        MyMoneyTransactionFilter::setDateFilter( _u );
    }

  /**
    * Set the underlying date filter using the start and end dates provided.
    * Note that this does not LOCK to any range like setDateFilter(unsigned)
    * above.  It is just a reimplementation of the MyMoneyTransactionFilter
    * version.
    *
    * @param _db The inclusive begin date of the date range
    * @param _de The inclusive end date of the date range
    */

  void setDateFilter(const QDate& _db,const QDate& _de) { MyMoneyTransactionFilter::setDateFilter( _db,_de ); }

  /**
    * Set the underlying date filter using the 'date lock' property.
    *
    * Always call this function before executing the report to be sure that
    * the date filters properly match the plain-language 'date lock'.
    *
    * For example, if the report is date-locked to "Current Month", and the
    * last time you loaded or ran the report was in August, but it's now
    * September, this function will update the date range to be September,
    * as is proper.
    */
  void updateDateFilter(void) { if (m_dateLock != userDefined) MyMoneyTransactionFilter::setDateFilter(m_dateLock); }

  /**
    * Retrieves a VALID beginning & ending date for this report.
    *
    * The underlying date filter can return en empty QDate() for either the
    * begin or end date or both.  This is typically unacceptable for reports,
    * which need the REAL begin and end date.
    *
    * This function gets the underlying date filter range, and if either is
    * an empty QDate(), it determines the missing date from looking at all
    * the transactions which match the underlying filter, and returning the
    * date of the first or last transaction (as appropriate).
    *
    * @param _db The inclusive begin date of the date range
    * @param _de The inclusive end date of the date range
    */
  void validDateRange(QDate& _db, QDate& _de);

  /**
    * This method turns on the account group filter and adds the
    * @p type to the list of allowed groups.
    *
    * Note that account group filtering is handled differently
    * than all the filters of the underlying class.  This filter
    * is meant to be applied to individual splits of matched
    * transactions AFTER the underlying filter is used to find
    * the matching transactions.
    *
    * @param type the account group to add to the allowed groups list
    */
  void addAccountGroup(MyMoneyAccount::accountTypeE type);

  /**
    * This method returns whether an account group filter has been set,
    * and if so, it returns all the account groups set in the filter.
    *
    * @param list list to append account groups into
    * @return return true if an account group filter has been set
    */
  bool accountGroups(QValueList<MyMoneyAccount::accountTypeE>& list) const;

  /**
    * This method returns whether the specified account group
    * is allowed by the account groups filter.
    *
    * @param type group to append account groups into
    * @return return true if an account group filter has been set
    */
  bool includesAccountGroup( MyMoneyAccount::accountTypeE type ) const;

  /**
    * This method is used to test whether a specific account
    * passes the accountGroup test and either the Account or
    * Category test, depending on which sort of Account it is.
    *
    * The m_tax and m_investments properties are also considered.
    *
    * @param acc the account in question
    * @return true if account is in filter set, false otherwise
    */
  bool includes( const MyMoneyAccount& acc ) const;

  /**
    * This method writes this report to the DOM element @p e,
    * within the DOM document @p doc.
    *
    * @param e The element which should be populated with info from this report
    * @param doc The document which we can use to create new sub-elements
    *              if needed
    * @param anonymous Whether the sensitive parts of the report should be
    *              masked
    */
  void write(QDomElement& e, QDomDocument *doc, bool anonymous=false) const;

  /**
    * This method reads a report from the DOM element @p e, and
    * populates this report with the results.
    *
    * @param e The element from which the report should be read
    *
    * @return bool True if a report was successfully loaded from the
    *    element @p e.  If false is returned, the contents of this report
    *    object are undefined.
    */
  bool read(const QDomElement& e);

  /**
    * This method creates a QDomElement for the @p document
    * under the parent node @p parent.  (This version overwrites the
    * MMObject base class.)
    *
    * @param document reference to QDomDocument
    * @param parent reference to QDomElement parent node
    */
  virtual void writeXML(QDomDocument& document, QDomElement& parent) const;

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  virtual bool hasReferenceTo(const QString& id) const;

private:
  /**
    * The user-assigned name of the report
    */
  QString m_name;
  /**
    * The user-assigned comment for the report, in case they want to make
    * additional notes for themselves about the report.
    */
  QString m_comment;
  /**
    * Where to group this report amongst the others in the UI view.  This
    * should be assigned by the UI system.
    */
  QString m_group;
  /**
    * How much detail to show in the accounts
    */
  enum EDetailLevel m_detailLevel;
  /**
    * Whether to convert all currencies to the base currency of the file (true).
    * If this is false, it's up to the report generator to decide how to handle
    * the currency.
    */
  bool m_convertCurrency;
  /**
    * Whether this is one of the users' favorite reports
    */
  bool m_favorite;
  /**
    * Whether this report should only include categories marked as "Tax"="Yes"
    */
  bool m_tax;
  /**
    * Whether this report should only include investment accounts
    */
  bool m_investments;
  /**
    * Whether this report should only include loan accounts
    * Applies only to querytable reports.  Mutually exclusive with
    * m_investments.
    */
  bool m_loans;
  /**
    * What sort of algorithm should be used to run the report
    */
  enum EReportType m_reportType;
  /**
    * What sort of values should show up on the ROWS of this report
    */
  enum ERowType m_rowType;
  /**
    * What sort of values should show up on the COLUMNS of this report,
    * in the case of a 'PivotTable' report.  Really this is used more as a
    * QUANTITY of months or days.  Whether it's months or days is determiend
    * by m_columnsAreDays.
    */
  enum EColumnType m_columnType;
   /**
    * Whether the base unit of columns of this report is days.  Only applies to
    * 'PivotTable' reports.  If false, then columns are months or multiples thereof.
    */
  bool m_columnsAreDays;
 /**
    * What sort of values should show up on the COLUMNS of this report,
    * in the case of a 'QueryTable' report
    */
  enum EQueryColumns m_queryColumns;

  /**
    * The plain-language description of what the date range should be locked
    * to.  'userDefined' means NO locking, in any other case, the report
    * will be adjusted to match the date lock.  So if the date lock is
    * 'currentMonth', the start and end dates of the underlying filter will
    * be updated to whatever the current month is.  This updating happens
    * automatically when the report is loaded, and should also be done
    * manually by calling updateDateFilter() before generating the report
    */
  dateOptionE m_dateLock;
  /**
    * Which account groups should be included in the report.  This filter
    * is applied to the individual splits AFTER a transaction has been
    * matched using the underlying filter.
    */
  QValueList<MyMoneyAccount::accountTypeE> m_accountGroups;
  /**
    * Whether an account group filter has been set (see m_accountGroups)
    */
  bool m_accountGroupFilter;
  /**
    * What format should be used to draw this report as a chart
    */
  enum EChartType m_chartType;
  /**
    * Whether the value of individual data points should be drawn on the chart
    */
  bool m_chartDataLabels;
  /**
    * Whether grid lines should be drawn on the chart
    */
  bool m_chartGridLines;
  /**
    * Whether this report should be shown as a chart by default (otherwise it
    * should be shown as a textual report)
    */
  bool m_chartByDefault;
  /**
   * Width of the chart lines
   */
  uint m_chartLineWidth;
  /**
    * Whether to include scheduled transactions
    */
  bool m_includeSchedules;
  /**
    * Whether to include transfers.  Only applies to Income/Expense reports
    */
  bool m_includeTransfers;
  /**
    * The id of the budget associated with this report.
    */
  QString m_budgetId;
  /**
    * Whether this report should print the actual data to go along with
    * the budget.  This is only valid if the report has a budget.
    */
  bool m_includeBudgetActuals;
  /**
    * Whether this report should include all accounts and not only
    * accounts with transactions.
    */
  bool m_includeUnusedAccounts;
  /**
   * Whether this report should include columns for row totals
   */
  bool m_showRowTotals;
  /**
   * Whether this report should include forecast balance
   */
  bool m_includeForecast;
  /**
   * Whether this report should include moving average
   */
  bool m_includeMovingAverage;
  /**
   * The amount of days that spans each moving average
   */
  int m_movingAverageDays;
  /**
   * Whether this report should include prices
   */
  bool m_includePrice;
  /**
   * Whether this report should include moving average prices
   */
  bool m_includeAveragePrice;



};

#endif // MYMONEYREPORT_H
