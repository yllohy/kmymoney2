/***************************************************************************
                          querytable.cpp  -  description
                             -------------------
    begin                : Fri Jul 23 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

// ----------------------------------------------------------------------------
// QT Includes
#include <qvaluelist.h>
#include <qfile.h>
#include <qtextstream.h>

// ----------------------------------------------------------------------------
// KDE Includes
// This is just needed for i18n().  Once I figure out how to handle i18n
// without using this macro directly, I'll be freed of KDE dependency.

#include <klocale.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneytransaction.h"
#include "../mymoney/mymoneyreport.h"
#include "querytable.h"

namespace reports {

// this should be in mymoneysplit.h
static const QStringList kReconcileText = QStringList::split(",","notreconciled,cleared,reconciled,frozen,none");
static const QStringList kReconcileTextChar = QStringList::split(",","N,C,R,F,none");

QStringList QueryTable::TableRow::m_sortCriteria;
  
bool QueryTable::TableRow::operator<( const TableRow& _compare ) const
{
  bool result = false;
  
  QStringList::const_iterator it_criterion = m_sortCriteria.begin();
  while ( it_criterion != m_sortCriteria.end() )
  {
    if ( this->operator[]( *it_criterion ) < _compare[ *it_criterion ] )
    {
      result = true;
      break;
    }
    else if ( this->operator[]( *it_criterion ) > _compare[ *it_criterion ] )
      break;
    
    ++it_criterion;  
  }
  return result;
}

/**
  * TODO
  *
  * - Handle sub-accounts
  * - Subtotals
  * - Convert currency
  * - Link into UI
  * - Customization
  *
  */

// stealing this function from pivottable.cpp.  As noted in the comments of that file,
// we do need a better solution, but it'll do 'for now'.
const QString accountTypeToString(const MyMoneyAccount::accountTypeE accountType);

QueryTable::QueryTable(const MyMoneyReport& _report)
{
  //
  // The main job of this constructor is to translate the transaction & split list
  // into a database-style table, which we can then query later SQL-style.
  //

  MyMoneyFile* file = MyMoneyFile::instance();
  
  MyMoneyReport report(_report);
  QValueList<MyMoneyTransaction> transactions = file->transactionList( report );
  QValueList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
  while ( it_transaction != transactions.end() )
  {
    TableRow qtransactionrow;
    
    qtransactionrow["id"] = (*it_transaction).id();
    qtransactionrow["postdate"] = (*it_transaction).postDate().toString(Qt::ISODate);
    qtransactionrow["memo"] = (*it_transaction).memo();
    qtransactionrow["entrydate"] = (*it_transaction).entryDate().toString(Qt::ISODate);
    qtransactionrow["commodity"] = (*it_transaction).commodity();
    
    // A table row ALWAYS has one asset/liability account.  A transaction 
    // will generate one table row for each A/L account.
    //
    // Likewise, a table row ALWAYS has one E/I category.  In the case of complex 
    // multi-split transactions, another table, the 'splits table' will be used to 
    // hold the extra info in case the user wants to see it.
    //
    // Splits table handling differs depending on what the user has asked for.
    // If the user wants transactions BY CATEGORY, then multiple table row
    // are generated, one for each category.
    //
    // For now, implementing the simple case...which is one row per-A/L account
    // per E/I category.
    
    QValueList<MyMoneySplit> splits = (*it_transaction).splits();
    QValueList<MyMoneySplit>::const_iterator it_split = splits.begin();
    while ( it_split != splits.end() )
    {
      // Loop through the splits once for every asset/liability account.  
      // Create one table row for each such account
      if ( file->account((*it_split).accountId()).accountGroup() == MyMoneyAccount::Asset || file->account((*it_split).accountId()).accountGroup() == MyMoneyAccount::Liability )
      {
        TableRow qaccountrow = qtransactionrow;
      
        // These items are relevant for each A/L split
        qaccountrow["payee"] = file->payee(splits.front().payeeId()).name();
        qaccountrow["reconciledate"] = splits.front().reconcileDate().toString(Qt::ISODate);
        qaccountrow["reconcileflag"] = kReconcileTextChar[splits.front().reconcileFlag()];
        qaccountrow["number"] = splits.front().number();
      
        QValueList<MyMoneySplit>::const_iterator it_split2 = splits.begin();
        while ( it_split2 != splits.end() )
        {
          // Only process this split if it is not the A/L account we're working with anyway
          if ( (*it_split2).accountId() != (*it_split).accountId() )
          {
            // Create one query line for each target account/category
            // (This is the 'expand categories' behaviour.  
            // 'collapse categories' would entail cramming them all into one 
            // line and calling it "Split Categories").
            
            TableRow qsplitrow = qaccountrow;
            
            qsplitrow["account"] = file->account((*it_split).accountId()).name();
            qsplitrow["category"] = file->account((*it_split2).accountId()).name();
            qsplitrow["categorytype"] = accountTypeToString(file->account((*it_split2).accountId()).accountGroup());
            qsplitrow["action"] = (*it_split2).action();
            qsplitrow["value"] = (*it_split2).value().formatMoney();
            qsplitrow["memo"] = (*it_split2).memo();
            qsplitrow["id"] = (*it_split2).id();
      
            m_transactions += qsplitrow;
                        
          }
          ++it_split2;
        }
      }
      ++it_split;
    }
    ++it_transaction;  
  }

  // Sort the transactions to match the report definition
  // ...or just category,date for now!
  
  // SELECT number,date,payee,account,amount FROM m_transactions ORDER BY category type, category, subcategory, date
  // subtotal by category type, category, subcategory
  
  TableRow::setSortCriteria("categorytype,category,postdate");
  qHeapSort(m_transactions);
}

QString QueryTable::renderHTML( void ) const
{
  QString result;
  QStringList columns = QStringList::split(",","categorytype,category,number,postdate,payee,account,value");

  //
  // Table header
  //
  
  result += "<table><tr>";
  
  QStringList::const_iterator it_column = columns.begin();
  while ( it_column != columns.end() )
  {
    result += "<th>" + (*it_column) + "</th>";
    ++it_column;
  }
  
  result += "</tr>\n";

  //
  // Rows
  //
    
  QValueList<TableRow>::const_iterator it_row = m_transactions.begin();
  while ( it_row != m_transactions.end() )
  {
    result += "<tr>";
    
    //
    // Columns
    //
    
    QStringList::const_iterator it_column = columns.begin();
    while ( it_column != columns.end() )
    {
      result += "<td>" + (*it_row)[(*it_column)] + "</td>";
      ++it_column;
    }
    
    result += "</tr>\n";
    ++it_row;
  }
  result += "</table>";
  
  return result;
}

QString QueryTable::renderCSV( void ) const
{
  return "CSV output not yet implemented for query reports";
}

void QueryTable::dump( const QString& file ) const
{
  QFile g( file );
  g.open( IO_WriteOnly );
  QTextStream(&g) << renderHTML();
  g.close();
}

}
