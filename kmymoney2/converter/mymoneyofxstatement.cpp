/***************************************************************************
                          mymoneyofxstatement.cpp
                          -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <acejones@users.sourceforge.net>
 ***************************************************************************/

 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#ifdef HAVE_LIBOFX

#include "../mymoney/mymoneyexception.h"
#include "mymoneyofxstatement.h"

#include <libofx/libofx.h>

MyMoneyOfxStatement* pgCurrentStatement = NULL;

//
// MyMoneyOfxStatement Implementation
//

MyMoneyOfxStatement::MyMoneyOfxStatement(const QString& filename):
  m_valid( false )
{
  if ( pgCurrentStatement )
    throw new MYMONEYEXCEPTION("Ofx import already in progress. Only ONE ofx import can be processed at once!");

  pgCurrentStatement = this;
  
  QCString filename_deep( filename.utf8() );
  const char* argv[2];
  argv[0] = "KMyMoney2";
  argv[1] = filename_deep;
  ofx_proc_file(2, const_cast<char**>(argv));
    
  pgCurrentStatement = NULL;
}

MyMoneyOfxStatement::~MyMoneyOfxStatement()
{
}

//
// These global _cb functions are callbacks from libofx.  They are required to
// be in the global scope, and named as such.
//

int ofx_proc_status_cb(struct OfxStatusData /*data*/)
{
  return 0;
}

int ofx_proc_security_cb(struct OfxSecurityData /*data*/)
{
  return 0;
}

int ofx_proc_transaction_cb(struct OfxTransactionData data)
{

  MyMoneyStatement::Transaction t;
  
  if(data.date_posted_valid==true)
  {
    QDateTime dt;
    dt.setTime_t(data.date_posted);
    t.m_datePosted = dt.date();
  }
  
  if(data.amount_valid==true)
  {
    t.m_moneyAmount = data.amount;
  }
  
  if(data.check_number_valid==true)
  {
    t.m_strNumber = data.check_number;
  }
  else if(data.fi_id_valid==true)
  {
    t.m_strNumber = QString("ID ") + data.fi_id;
  }
  else if(data.reference_number_valid==true)
  {
    t.m_strNumber = QString("REF ") + data.reference_number;
  }
  
  if(data.payee_id_valid==true)
  {
    t.m_strPayee = data.payee_id;
  }
  else if(data.name_valid==true)
  {
    t.m_strPayee = data.name;
  }
  
  if(data.memo_valid==true){
    t.m_strMemo = data.memo;
  }

  // If the payee or memo fields are blank, set them to
  // the other one which is NOT blank.
  if ( t.m_strPayee.isEmpty() )
  {
    if ( ! t.m_strMemo.isEmpty() )
      t.m_strPayee = t.m_strMemo;
  }
  else
  {
    if ( t.m_strMemo.isEmpty() )
      t.m_strMemo = t.m_strPayee;
  }
  
  pgCurrentStatement->m_listTransactions += t;
  
  return 0;
}

int ofx_proc_statement_cb(struct OfxStatementData data)
{
  pgCurrentStatement->setValid();
  
  if(data.currency_valid==true)
  {
    pgCurrentStatement->m_strCurrency = data.currency;
  }
  if(data.account_id_valid==true)
  {
    pgCurrentStatement->m_strAccountNumber = data.account_id;
  }
  if(data.date_start_valid==true)
  {
    QDateTime dt;
    dt.setTime_t(data.date_start);
    pgCurrentStatement->m_dateBegin = dt.date();
  }
  
  if(data.date_end_valid==true)
  {
    QDateTime dt;
    dt.setTime_t(data.date_end);
    pgCurrentStatement->m_dateEnd = dt.date();
  }
  
  if(data.ledger_balance_valid==true)
  {
    pgCurrentStatement->m_moneyClosingBalance = static_cast<double>(data.ledger_balance);
  }
  
  return 0;
}

int ofx_proc_account_cb(struct OfxAccountData data)
{
  if(data.account_id_valid==true)
  {
    pgCurrentStatement->m_strAccountName = data.account_name;
    pgCurrentStatement->m_strAccountNumber = data.account_id;
  }
  if(data.currency_valid==true)
  {
    pgCurrentStatement->m_strCurrency = data.currency;
  }
  
  return 0;
}

#endif
