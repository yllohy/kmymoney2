/***************************************************************************
                          mymoneystoragexml.h  -  description
                             -------------------
    begin                : Thu Oct 24 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <ace.j@hotpop.com>
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

// ----------------------------------------------------------------------------
// QT Includes
#include <qmap.h>
#include <qvaluelist.h>
#include <qstring.h>
class QDomElement;
class QDomDocument;

// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneytransactionfilter.h"

/**
  *@author Ace Jones <ace.j@hotpop.com>
  */

class MyMoneyReport: public MyMoneyTransactionFilter
{
public:
  enum EReportType { eNoReport = 0, ePivotTable, eQueryTable };
  enum ERowType { eNoRows = 0, eAssetLiability, eExpenseIncome, eCategory, eTopCategory, eAccount, ePayee, eMonth, eWeek };
  enum EColumnType { eNoColumns = 0, eMonths = 1, eBiMonths = 2, eQuarters = 3, eYears = 12 };
  enum EQueryColumns { eQCnone = 0x0, eQCbegin = 0x1, eQCnumber = 0x1, eQCpayee = 0x2, eQCcategory = 0x4, eQCmemo = 0x8, eQCaccount = 0x10, eQCreconciled=0x20, eQCend=0x40 };
  
  static const QStringList kRowTypeText;
  static const QStringList kColumnTypeText;
  static const QStringList kQueryColumnsText;
  static const EReportType kTypeArray[];
  
public:
  MyMoneyReport(ERowType _rt = eExpenseIncome, EColumnType _ct = eMonths, const QDate& _db = QDate(), const QDate& _de = QDate()):
    m_name("Unconfigured Pivot Table Report"),
    m_showSubAccounts(false),
    m_convertCurrency(true),
    m_reportType(kTypeArray[_rt]),
    m_rowType(_rt),
    m_columnType(_ct),
    m_queryColumns(eQCnone),
    m_dateLock(userDefined)
  {
    setDateFilter(_db,_de);
  }
  
  MyMoneyReport(ERowType _rt, unsigned _ct, unsigned _dl, bool _ss, const QString& _name, const QString& _comment ):
    m_name(_name),
    m_comment(_comment),
    m_showSubAccounts(_ss),
    m_convertCurrency(true),
    m_reportType(kTypeArray[_rt]),
    m_rowType(_rt),
    m_dateLock(_dl)
  {
    if ( m_reportType == ePivotTable )
      m_columnType = static_cast<EColumnType>(_ct);
    if ( m_reportType == eQueryTable )
      m_queryColumns = static_cast<EQueryColumns>(_ct);
    setDateFilter(_dl);
  }
  
  void setName(const QString& _s) { m_name = _s; }
  void setShowSubAccounts(bool _f) { m_showSubAccounts = _f; }
  void setConvertCurrency(bool _f) { m_convertCurrency = _f; }
  void setRowType(ERowType _rt) { m_rowType = _rt; m_reportType = kTypeArray[_rt]; }
  void setColumnType(EColumnType _ct) { m_columnType = _ct; }
  void assignFilter(const MyMoneyTransactionFilter& _filter) { MyMoneyTransactionFilter::operator=(_filter); }
  const QString& name(void) const { return m_name; }
  bool isShowingSubAccounts(void) const { return m_showSubAccounts; }
  bool isShowingRowTotals(void) const { return (m_rowType==eExpenseIncome); }
  EReportType reportType(void) const { return m_reportType; }
  ERowType rowType(void) const { return m_rowType; }
  EColumnType columnType(void) const { return m_columnType; }
  bool isRunningSum(void) const { return (m_rowType==eAssetLiability); }
  bool isConvertCurrency(void) const { return m_convertCurrency; }
  unsigned columnPitch(void) const { return static_cast<unsigned>(m_columnType); }
  bool isShowingColumnTotals(void) const { return m_convertCurrency; }
  void setId( const QCString& _id ) { m_id = _id; }
  QCString id(void) const { return m_id; }
  void write(QDomElement& e, QDomDocument *doc) const;
  bool read(const QDomElement& e);
  void setComment( const QString& _comment ) { m_comment = _comment; }
  const QString& comment( void ) const { return m_comment; }
  void setQueryColumns( EQueryColumns _qc ) { m_queryColumns = _qc; }
  EQueryColumns queryColumns(void) const { return m_queryColumns; }
  void setDateFilter(unsigned _u) { m_dateLock = _u; if (_u != userDefined) MyMoneyTransactionFilter::setDateFilter( _u ); }
  void setDateFilter(const QDate& _db,const QDate& _de) { MyMoneyTransactionFilter::setDateFilter( _db,_de ); }
  void updateDateFilter(void) { if (m_dateLock != userDefined) MyMoneyTransactionFilter::setDateFilter(m_dateLock); }
  
private:
  QCString m_id;
  QString m_name;
  QString m_comment;
  bool m_showSubAccounts;
  bool m_convertCurrency;
  enum EReportType m_reportType;
  enum ERowType m_rowType;
  enum EColumnType m_columnType;
  enum EQueryColumns m_queryColumns;
  unsigned m_dateLock;
};

#endif // MYMONEYREPORT_H  
