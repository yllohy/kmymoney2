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

#include <qfile.h>
#include <qtextstream.h>

#include "../mymoney/mymoneyexception.h"
#include "mymoneyofxstatement.h"


#if defined(HAVE_LIBOFX) || defined(HAVE_NEW_OFX)

/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *
 * The following part is compiled for ANY version of LibOFX.
 *
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

#include <libofx/libofx.h>

// In libofx 0.6.6 these defines are inside OfxTransactionData, while in the
// new version they are in the global scope.
#if !defined(HAVE_NEW_OFX)
#define OFX_BUYDEBT OfxTransactionData::OFX_BUYDEBT 
#define OFX_BUYMF OfxTransactionData::OFX_BUYMF 
#define OFX_BUYOPT OfxTransactionData::OFX_BUYOPT 
#define OFX_BUYOTHER OfxTransactionData::OFX_BUYOTHER 
#define OFX_BUYSTOCK OfxTransactionData::OFX_BUYSTOCK 
#define OFX_REINVEST OfxTransactionData::OFX_REINVEST 
#define OFX_SELLDEBT OfxTransactionData::OFX_SELLDEBT 
#define OFX_SELLMF OfxTransactionData::OFX_SELLMF 
#define OFX_SELLOPT OfxTransactionData::OFX_SELLOPT 
#define OFX_SELLOTHER OfxTransactionData::OFX_SELLOTHER 
#define OFX_SELLSTOCK OfxTransactionData::OFX_SELLSTOCK 
#endif

int ofxTransactionCallback(struct OfxTransactionData data, void * pv)
{
  MyMoneyOfxStatement* pofx = reinterpret_cast<MyMoneyOfxStatement*>(pv);
  MyMoneyStatement& s = pofx->back();

  MyMoneyStatement::Transaction t;

  if(data.date_posted_valid==true)
  {
    QDateTime dt;
    dt.setTime_t(data.date_posted);
    t.m_datePosted = dt.date();
  }

  if(data.amount_valid==true)
  {
    // if this is an investment statement, reverse the sign.  not sure
    // why this is needed, so I suppose it's a bit of a hack for the moment.
    double sign = 1.0;
    if (data.invtransactiontype_valid==true)
      sign=-1.0;
  
    t.m_moneyAmount = sign * data.amount;
  }

  if(data.check_number_valid==true)
  {
    t.m_strNumber = data.check_number;
  }
  
  if(data.fi_id_valid==true)
  {
    t.m_strBankID = QString("ID ") + data.fi_id;
  }
  else if(data.reference_number_valid==true)
  {
    t.m_strBankID = QString("REF ") + data.reference_number;
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
  
  if(data.security_data_valid==true)
  {
    struct OfxSecurityData* secdata = data.security_data_ptr;
  
    if(secdata->ticker_valid==true){
      t.m_strSecurity += secdata->ticker;
    }
    
    if(secdata->secname_valid==true){
      t.m_strSecurity += QString(" ") + secdata->secname;
    }
  }

  if(data.units_valid==true)
  {
    t.m_dShares = data.units;
  }
 
  if(data.invtransactiontype_valid==true)
  {
    switch (data.invtransactiontype)
    {
    case OFX_BUYDEBT:
    case OFX_BUYMF:
    case OFX_BUYOPT:
    case OFX_BUYOTHER:
    case OFX_BUYSTOCK:
      t.m_eAction = MyMoneyStatement::Transaction::eaBuy;
      break;
    case OFX_REINVEST:
      t.m_eAction = MyMoneyStatement::Transaction::eaReinvestDividend;
      break;
    case OFX_SELLDEBT:
    case OFX_SELLMF:
    case OFX_SELLOPT:
    case OFX_SELLOTHER:
    case OFX_SELLSTOCK:
      t.m_eAction = MyMoneyStatement::Transaction::eaSell;
      break;
    default:
      // the importer does not support this kind of action
      break;
    }
  }

  s.m_listTransactions += t;

  return 0;
}

int ofxStatementCallback(struct OfxStatementData data, void* pv)
{
  MyMoneyOfxStatement* pofx = reinterpret_cast<MyMoneyOfxStatement*>(pv);
  MyMoneyStatement& s = pofx->back();

  pofx->setValid();

  if(data.currency_valid==true)
  {
    s.m_strCurrency = data.currency;
  }
  if(data.account_id_valid==true)
  {
    s.m_strAccountNumber = data.account_id;
  }
  if(data.date_start_valid==true)
  {
    QDateTime dt;
    dt.setTime_t(data.date_start);
    s.m_dateBegin = dt.date();
  }

  if(data.date_end_valid==true)
  {
    QDateTime dt;
    dt.setTime_t(data.date_end);
    s.m_dateEnd = dt.date();
  }

  if(data.ledger_balance_valid==true)
  {
    s.m_moneyClosingBalance = static_cast<double>(data.ledger_balance);
  }

  return 0;
}

int ofxAccountCallback(struct OfxAccountData data, void * pv)
{
  MyMoneyOfxStatement* pofx = reinterpret_cast<MyMoneyOfxStatement*>(pv);
  pofx->addnew();
  MyMoneyStatement& s = pofx->back();
  
  if(data.account_id_valid==true)
  {
    s.m_strAccountName = data.account_name;
    s.m_strAccountNumber = data.account_id;
  }
  if(data.currency_valid==true)
  {
    s.m_strCurrency = data.currency;
  }

  if(data.account_type_valid==true)
  {
    switch(data.account_type)
    {
    case OfxAccountData::OFX_CHECKING : s.m_eType = MyMoneyStatement::etCheckings;
      break;
    case OfxAccountData::OFX_SAVINGS : s.m_eType = MyMoneyStatement::etSavings;
      break;
    case OfxAccountData::OFX_MONEYMRKT : s.m_eType = MyMoneyStatement::etInvestment;
      break;
    case OfxAccountData::OFX_CREDITLINE : s.m_eType = MyMoneyStatement::etCreditCard;
      break;
    case OfxAccountData::OFX_CMA : s.m_eType = MyMoneyStatement::etCreditCard;
      break;
    case OfxAccountData::OFX_CREDITCARD : s.m_eType = MyMoneyStatement::etCreditCard;
      break;
    case OfxAccountData::OFX_INVESTMENT : s.m_eType = MyMoneyStatement::etInvestment;
      break;
    }
  }
  
  return 0;
}

int ofxStatusCallback(struct OfxStatusData data, void * pv)
{
  MyMoneyOfxStatement* pofx = reinterpret_cast<MyMoneyOfxStatement*>(pv);
  QString message;  

  // Having any status at all makes an ofx statement valid
  pofx->setValid();
    
  if(data.ofx_element_name_valid==true)
    message.prepend(QString("%1: ").arg(data.ofx_element_name));
  
  if(data.code_valid==true)
    message += QString("%1 (Code %2): %3").arg(data.name).arg(data.code).arg(data.description);
  
  if(data.server_message_valid==true)
    message += QString(" (%1)").arg(data.server_message);
  
  if(data.severity_valid==true){
    switch(data.severity){
    case OfxStatusData::INFO:
      pofx->addInfo( message );
      break;
    case OfxStatusData::ERROR: 
      pofx->addError( message );
      break;
    case OfxStatusData::WARN: 
      pofx->addWarning( message );
      break;
    default:
      pofx->addWarning( message );
      pofx->addWarning( "Previous message was an unknown type.  'WARNING' was assumed.");
      break;
    }
  }
  return 0;
}

#endif 




 

#ifdef HAVE_NEW_OFX

/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *
 * The following part is only compiled if a newer version of LibOFX is used
 * (0.7 and higher). The second half of this file contains the old code.
 *
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

//
// MyMoneyOfxStatement Implementation
//

MyMoneyOfxStatement::MyMoneyOfxStatement(const QString& filename):
  m_valid( false )
{
  QCString filename_deep( filename.utf8() );

  LibofxContextPtr ctx = libofx_get_new_context();
  Q_CHECK_PTR(ctx);
  
  ofx_set_transaction_cb(ctx, ofxTransactionCallback, this);
  ofx_set_statement_cb(ctx, ofxStatementCallback, this);
  ofx_set_account_cb(ctx, ofxAccountCallback, this);
  ofx_set_status_cb(ctx, ofxStatusCallback, this);
  libofx_proc_file(ctx, filename_deep, AUTODETECT);
  libofx_free_context(ctx);
}

#endif // HAVE_NEW_OFX





// FIXME: Remove this workaround, and solve this problem correctly.  The
// program should not construct MMOS objects without one of these
// defined.
#if ! defined(HAVE_LIBOFX) && ! defined(HAVE_NEW_OFX)
MyMoneyOfxStatement::MyMoneyOfxStatement(const QString&): m_valid(false) {}
#endif


#ifdef HAVE_LIBOFX
/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *
 * The following part is only compiled if an older version of LibOFX is used
 * (up to 0.6.6)
 *
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

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

//
// These global _cb functions are callbacks from libofx.  They are required to
// be in the global scope, and named as such.
//

int ofx_proc_status_cb(struct OfxStatusData data)
{
  return ofxStatusCallback(data,pgCurrentStatement);
}

int ofx_proc_security_cb(struct OfxSecurityData /*data*/)
{
  return 0;
}

int ofx_proc_transaction_cb(struct OfxTransactionData data)
{
  return ofxTransactionCallback(data,pgCurrentStatement);
}

int ofx_proc_statement_cb(struct OfxStatementData data)
{
  return ofxStatementCallback(data,pgCurrentStatement);
}

int ofx_proc_account_cb(struct OfxAccountData data)
{
  return ofxAccountCallback(data, pgCurrentStatement);
}

#endif // #ifdef HAVE_LIBOFX

/* __________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *
 * The following part is compiled whether or not there is a version of LibOFX 
 * available.
 *
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

bool MyMoneyOfxStatement::isOfxFile(const QString& filename)
{
  // filename is an Ofx file if it contains the tag "<OFX>" somewhere.
  bool result = false;

  QFile f( filename );
  if ( f.open( IO_ReadOnly ) )
  {
    QTextStream ts( &f );

    while ( !ts.atEnd() && !result )
      if ( ts.readLine().contains("<OFX>",false) )
        result = true;

    f.close();
  }

  return result;
}

MyMoneyOfxStatement::~MyMoneyOfxStatement()
{
}



