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
  enum ERowType { eNoRows = 0x0, eAsset = 0x1, eLiability = 0x2, eAssetLiability = 0x3, eExpense = 0x4, eIncome = 0x8, eExpenseIncome = 0xc };
  enum EColumnType { eNoColumns = 0, eMonths = 1, eBiMonths = 2, eQuarters = 3, eYears = 12 };
  
  static const QStringList kRowTypeText;
  static const QStringList kColumnTypeText;
  
public:
  MyMoneyReport(ERowType _rt = eExpenseIncome, EColumnType _ct = eMonths, const QDate& _db = QDate(), const QDate& _de = QDate()):
    m_name("Unconfigured Report"),
    m_showSubAccounts(false),
    m_convertCurrency(true),
    m_rowType(_rt),
    m_columnType(_ct)
  {
    setDateFilter(_db,_de);
  }
  void setName(const QString& _s) { m_name = _s; }
  void setShowSubAccounts(bool _f) { m_showSubAccounts = _f; }
  void setConvertCurrency(bool _f) { m_convertCurrency = _f; }
  void setRowType(ERowType _rt) { m_rowType = _rt; }
  void setColumnType(EColumnType _ct) { m_columnType = _ct; }
  void assignFilter(const MyMoneyTransactionFilter& _filter) { MyMoneyTransactionFilter::operator=(_filter); }
  const QString& name(void) const { return m_name; }
  bool isShowingSubAccounts(void) const { return m_showSubAccounts; }
  bool isShowingRowTotals(void) const { return ((m_rowType & eExpense) || (m_rowType & eIncome)); }
  ERowType rowType(void) const { return m_rowType; }
  EColumnType columnType(void) const { return m_columnType; }
  bool isRunningSum(void) const { return ((m_rowType & eAsset) || (m_rowType & eLiability)); }
  bool isConvertCurrency(void) const { return m_convertCurrency; }
  unsigned columnPitch(void) const { return static_cast<unsigned>(m_columnType); }
  bool isShowingColumnTotals(void) const { return m_convertCurrency; }
  void setId( const QCString& _id ) { m_id = _id; }
  QCString id(void) const { return m_id; }
  void write(QDomElement& e, QDomDocument *doc) const;
  bool read(const QDomElement& e);
  void setComment( const QString& _comment ) { m_comment = _comment; }
  const QString& comment( void ) const { return m_comment; }
  
private:
  QCString m_id;
  QString m_name;
  QString m_comment;
  bool m_showSubAccounts;
  bool m_convertCurrency;
  enum ERowType m_rowType;
  enum EColumnType m_columnType;

};

#endif // MYMONEYREPORT_H  
