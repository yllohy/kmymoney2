/***************************************************************************
                         mymoneyofxconnector.cpp
                             -------------------
    begin                : Sat Nov 13 2004
    copyright            : (C) 2002 by Ace Jones
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef USE_OFX_DIRECTCONNECT

// ----------------------------------------------------------------------------
// System Includes

#include <libofx/libofx.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qdatetime.h>
#include <qregexp.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kdebug.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneyinstitution.h>
#include <kmymoney/mymoneykeyvaluecontainer.h>
#include "mymoneyofxconnector.h"

OfxAppVersion::OfxAppVersion(KComboBox* combo, const QString& appId) :
  m_combo(combo)
{
// http://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-intuit-products/
// http://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-microsoft-money/

  // Quicken
  m_appMap[i18n("Quicken Windows 2003")] = "QWIN:1200";
  m_appMap[i18n("Quicken Windows 2004")] = "QWIN:1300";
  m_appMap[i18n("Quicken Windows 2005")] = "QWIN:1400";
  m_appMap[i18n("Quicken Windows 2006")] = "QWIN:1500";
  m_appMap[i18n("Quicken Windows 2007")] = "QWIN:1600";
  m_appMap[i18n("Quicken Windows 2008")] = "QWIN:1700";

  // MS-Money
  m_appMap[i18n("MS-Money 2003")] = "Money:1100";
  m_appMap[i18n("MS-Money 2004")] = "Money:1200";
  m_appMap[i18n("MS-Money 2005")] = "Money:1400";
  m_appMap[i18n("MS-Money 2006")] = "Money:1500";
  m_appMap[i18n("MS-Money 2007")] = "Money:1600";
  m_appMap[i18n("MS-Money Plus")] = "Money:1700";

  // KMyMoney
  m_appMap["KMyMoney"] = "KMyMoney:1000";

  combo->clear();
  combo->insertStringList(m_appMap.keys());

  QMap<QString, QString>::const_iterator it_a;
  for(it_a = m_appMap.begin(); it_a != m_appMap.end(); ++it_a) {
    if(*it_a == appId)
      break;
  }

  if(it_a != m_appMap.end()) {
    combo->setCurrentItem(it_a.key());
  } else {
    combo->setCurrentItem(i18n("Quicken Windows 2008"));
  }

#if ! LIBOFX_IS_VERSION(0,9,0)
  // This feature does not work with libOFX < 0.9 so
  // we just make disable the button in this case
  combo->setDisabled(true);
#endif
}

const QString& OfxAppVersion::appId(void) const
{
  static QString defaultAppId("QWIN:1700");

  QString app = m_combo->currentText();
  if(m_appMap[app] != defaultAppId)
    return m_appMap[app];
  return QString::null;
}

MyMoneyOfxConnector::MyMoneyOfxConnector(const MyMoneyAccount& _account):
  m_account(_account)
{
  m_fiSettings = m_account.onlineBankingSettings();
}

QString MyMoneyOfxConnector::iban(void) const { return m_fiSettings.value("bankid"); }
QString MyMoneyOfxConnector::fiorg(void) const { return m_fiSettings.value("org"); }
QString MyMoneyOfxConnector::fiid(void) const { return m_fiSettings.value("fid"); }
QString MyMoneyOfxConnector::username(void) const { return m_fiSettings.value("username"); }
QString MyMoneyOfxConnector::password(void) const { return m_fiSettings.value("password"); }
QString MyMoneyOfxConnector::accountnum(void) const { return m_fiSettings.value("accountid"); }
QString MyMoneyOfxConnector::url(void) const { return m_fiSettings.value("url"); }

QDate MyMoneyOfxConnector::statementStartDate(void) const {
  if ((m_fiSettings.value("kmmofx-todayMinus").toInt() != 0) && !m_fiSettings.value("kmmofx-numRequestDays").isEmpty())
  {
    return QDate::currentDate().addDays(-m_fiSettings.value("kmmofx-numRequestDays").toInt());
  }
  else if ((m_fiSettings.value("kmmofx-lastUpdate").toInt() != 0) && !m_account.value("lastImportedTransactionDate").isEmpty())
  {
    return QDate::fromString(m_account.value("lastImportedTransactionDate"), Qt::ISODate);
  }
  else if ((m_fiSettings.value("kmmofx-pickDate").toInt() != 0) && !m_fiSettings.value("kmmofx-specificDate").isEmpty())
  {
    return QDate::fromString(m_fiSettings.value("kmmofx-specificDate"));
  }
  return QDate::currentDate().addMonths(-2);
}

#if LIBOFX_IS_VERSION(0,9,0)
OfxAccountData::AccountType MyMoneyOfxConnector::accounttype(void) const
{
  OfxAccountData::AccountType result = OfxAccountData::OFX_CHECKING;

  QString type = m_account.onlineBankingSettings()["type"];
  if(type == "CHECKING")
    result = OfxAccountData::OFX_CHECKING;
  else if(type == "SAVINGS")
    result = OfxAccountData::OFX_SAVINGS;
  else if(type == "MONEY MARKET")
    result = OfxAccountData::OFX_MONEYMRKT;
  else if(type == "CREDIT LINE")
    result = OfxAccountData::OFX_CREDITLINE;
  else if(type == "CMA")
    result = OfxAccountData::OFX_CMA;
  else if(type == "CREDIT CARD")
    result = OfxAccountData::OFX_CREDITCARD;
  else if(type == "INVESTMENT")
    result = OfxAccountData::OFX_INVESTMENT;
  else {
    switch( m_account.accountType()) {
    case MyMoneyAccount::Investment:
      result = OfxAccountData::OFX_INVESTMENT;
      break;
    case MyMoneyAccount::CreditCard:
      result = OfxAccountData::OFX_CREDITCARD;
      break;
    case MyMoneyAccount::Savings:
      result = OfxAccountData::OFX_SAVINGS;
      break;
    default:
      break;
    }
  }

  // This is a bit of a personalized hack.  Sometimes we may want to override the
  // ofx type for an account.  For now, I will stash it in the notes!

  QRegExp rexp("OFXTYPE:([A-Z]*)");
  if ( rexp.search(m_account.description()) != -1 )
  {
    QString override = rexp.cap(1);
    kdDebug(2) << "MyMoneyOfxConnector::accounttype() overriding to " << result << endl;

    if ( override == "BANK" )
    result = OfxAccountData::OFX_CHECKING;
    else if ( override == "CC" )
      result = OfxAccountData::OFX_CREDITCARD;
    else if ( override == "INV" )
      result = OfxAccountData::OFX_INVESTMENT;
    else if ( override == "MONEYMARKET")
      result = OfxAccountData::OFX_MONEYMRKT;
  }

  return result;
}
#else
AccountType MyMoneyOfxConnector::accounttype(void) const
{
  AccountType result = OFX_BANK_ACCOUNT;

  switch( m_account.accountType() )
  {
  case MyMoneyAccount::Investment:
    result = OFX_INVEST_ACCOUNT;
    break;
  case MyMoneyAccount::CreditCard:
    result = OFX_CREDITCARD_ACCOUNT;
    break;
  default:
    break;
  }

  // This is a bit of a personalized hack.  Sometimes we may want to override the
  // ofx type for an account.  For now, I will stash it in the notes!

  QRegExp rexp("OFXTYPE:([A-Z]*)");
  if ( rexp.search(m_account.description()) != -1 )
  {
    QString override = rexp.cap(1);
    kdDebug(2) << "MyMoneyOfxConnector::accounttype() overriding to " << result << endl;

    if ( override == "BANK" )
    result = OFX_BANK_ACCOUNT;
    else if ( override == "CC" )
      result = OFX_CREDITCARD_ACCOUNT;
    else if ( override == "INV" )
      result = OFX_INVEST_ACCOUNT;
#if 0  // money market is not supported by 0.8.x
    else if ( override == "MONEYMARKET")
      result = OFX_MONEYMRKT;
#endif
  }

  return result;
}
#endif

void MyMoneyOfxConnector::initRequest(OfxFiLogin* fi) const
{
  memset(fi,0,sizeof(OfxFiLogin));
  strncpy(fi->fid, fiid().latin1(), OFX_FID_LENGTH-1);
  strncpy(fi->org, fiorg().latin1(), OFX_ORG_LENGTH-1);
  strncpy(fi->userid, username().latin1(), OFX_USERID_LENGTH-1);
  strncpy(fi->userpass, password().latin1(), OFX_USERPASS_LENGTH-1);

#if LIBOFX_IS_VERSION(0,9,0)
  // If we don't know better, we pretend to be Quicken 2008
  // http://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-intuit-products/
  // http://ofxblog.wordpress.com/2007/06/06/ofx-appid-and-appver-for-microsoft-money/
  QString appId = m_account.onlineBankingSettings().value("appId");
  QRegExp exp("(.*):(.*)");
  if(exp.search(appId) != -1) {
    strncpy(fi->appid, exp.cap(1).latin1(), OFX_APPID_LENGTH-1);
    strncpy(fi->appver, exp.cap(2).latin1(), OFX_APPVER_LENGTH-1);
  } else {
    strncpy(fi->appid, "QWIN", OFX_APPID_LENGTH-1);
    strncpy(fi->appver, "1700", OFX_APPVER_LENGTH-1);
  }
#endif
}

const QByteArray MyMoneyOfxConnector::statementRequest(void) const
{
  OfxFiLogin fi;
  initRequest(&fi);

#if LIBOFX_IS_VERSION(0,9,0)
  OfxAccountData account;
  memset(&account,0,sizeof(OfxAccountData));

  if(iban().latin1() != 0) {
    strncpy(account.bank_id,iban().latin1(),OFX_BANKID_LENGTH-1);
    strncpy(account.broker_id,iban().latin1(),OFX_BROKERID_LENGTH-1);
  }
  strncpy(account.account_number,accountnum().latin1(),OFX_ACCTID_LENGTH-1);
  account.account_type = accounttype();
#else
  OfxAccountInfo account;
  memset(&account,0,sizeof(OfxAccountInfo));

  if(iban().latin1() != 0) {
    strncpy(account.bankid,iban().latin1(),OFX_BANKID_LENGTH-1);
    strncpy(account.brokerid,iban().latin1(),OFX_BROKERID_LENGTH-1);
  }
  strncpy(account.accountid,accountnum().latin1(),OFX_ACCOUNT_ID_LENGTH-1);
  account.type = accounttype();
#endif

  char* szrequest = libofx_request_statement( &fi, &account, QDateTime(statementStartDate()).toTime_t() );
  QString request = szrequest;
  // remove the trailing zero
  QByteArray result = request.utf8();
  result.truncate(result.size()-1);
  free(szrequest);

  QString msg(result);
  return result;
}

#if 0
// this code is not used anymore. The logic is now
// contained in KOnlineBankingSetupWizard::finishLoginPage(void)
const QByteArray MyMoneyOfxConnector::accountInfoRequest(void) const
{
  OfxFiLogin fi;
  initRequest(&fi);

  char* szrequest = libofx_request_accountinfo( &fi );
  QString request = szrequest;
  // remove the trailing zero
  QByteArray result = request.utf8();
  result.truncate(result.size()-1);
  free(szrequest);

  return result;
}
#endif

#if 0

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::message(const QString& _msgType, const QString& _trnType, const Tag& _request)
{
  return Tag(_msgType+"MSGSRQV1")
    .subtag(Tag(_trnType+"TRNRQ")
      .element("TRNUID",uuid())
      .element("CLTCOOKIE","1")
      .subtag(_request));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::investmentRequest(void) const
{
  QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));
  QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

  return message("INVSTMT","INVSTMT",Tag("INVSTMTRQ")
    .subtag(Tag("INVACCTFROM").element("BROKERID", fiorg()).element("ACCTID", accountnum()))
    .subtag(Tag("INCTRAN").element("DTSTART",dtstart_string).element("INCLUDE","Y"))
    .element("INCOO","Y")
    .subtag(Tag("INCPOS").element("DTASOF", dtnow_string).element("INCLUDE","Y"))
    .element("INCBAL","Y"));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::bankStatementRequest(const QDate& _dtstart) const
{
  QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

  return message("BANK","STMT",Tag("STMTRQ")
    .subtag(Tag("BANKACCTFROM").element("BANKID", iban()).element("ACCTID", accountnum()).element("ACCTTYPE", "CHECKING"))
    .subtag(Tag("INCTRAN").element("DTSTART",dtstart_string).element("INCLUDE","Y")));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::creditCardRequest(const QDate& _dtstart) const
{
  QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

  return message("CREDITCARD","CCSTMT",Tag("CCSTMTRQ")
    .subtag(Tag("CCACCTFROM").element("ACCTID",accountnum()))
    .subtag(Tag("INCTRAN").element("DTSTART",dtstart_string).element("INCLUDE","Y")));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::signOn(void) const
{
  QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

  Tag fi("FI");
  fi.element("ORG",fiorg());
  if ( !fiid().isEmpty() )
    fi.element("FID",fiid());

  return Tag("SIGNONMSGSRQV1")
    .subtag(Tag("SONRQ")
      .element("DTCLIENT",dtnow_string)
      .element("USERID",username())
      .element("USERPASS",password())
      .element("LANGUAGE","ENG")
      .subtag(fi)
      .element("APPID","QWIN")
      .element("APPVER","1100"));
}

QString MyMoneyOfxConnector::header(void)
{
  return QString("OFXHEADER:100\r\n"
                 "DATA:OFXSGML\r\n"
                 "VERSION:102\r\n"
                 "SECURITY:NONE\r\n"
                 "ENCODING:USASCII\r\n"
                 "CHARSET:1252\r\n"
                 "COMPRESSION:NONE\r\n"
                 "OLDFILEUID:NONE\r\n"
                 "NEWFILEUID:%1\r\n"
                 "\r\n").arg(uuid());
}

QString MyMoneyOfxConnector::uuid(void)
{
  static int id = 1;
  return QDateTime::currentDateTime().toString("yyyyMMdd-hhmmsszzz-") + QString::number(id++);
}

//
// Methods to provide RESPONSES to OFX requests.  This has no real use in
// KMyMoney, but it's included for the purposes of unit testing.  This way, I
// can create a MyMoneyAccount, write it to an OFX file, import that OFX file,
// and check that everything made it through the importer.
//
// It's also a far-off dream to write an OFX server using KMyMoney as a
// backend.  It really should not be that hard, and it would fill a void in
// the open source software community.
//

const QByteArray MyMoneyOfxConnector::statementResponse(const QDate& _dtstart) const
{
  QString request;

  if ( accounttype()=="CC" )
    request = header() + Tag("OFX").subtag(signOnResponse()).subtag(creditCardStatementResponse(_dtstart));
  else if ( accounttype()=="INV" )
    request = header() + Tag("OFX").subtag(signOnResponse()).data(investmentStatementResponse(_dtstart));
  else
    request = header() + Tag("OFX").subtag(signOnResponse()).subtag(bankStatementResponse(_dtstart));

  // remove the trailing zero
  QByteArray result = request.utf8();
  result.truncate(result.size()-1);

  return result;
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::signOnResponse(void) const
{
  QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

  Tag sonrs("SONRS");
  sonrs
    .subtag(Tag("STATUS")
      .element("CODE","0")
      .element("SEVERITY","INFO")
      .element("MESSAGE","The operation succeeded.")
    )
    .element("DTSERVER",dtnow_string)
    .element("LANGUAGE","ENG");

  Tag fi("FI");
  if ( !fiorg().isEmpty() )
    fi.element("ORG",fiorg());
  if ( !fiid().isEmpty() )
    fi.element("FID",fiid());

  if ( !fi.isEmpty() )
    sonrs.subtag(fi);

  return Tag("SIGNONMSGSRSV1").subtag(sonrs);
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::messageResponse(const QString& _msgType, const QString& _trnType, const Tag& _response)
{
  return Tag(_msgType+"MSGSRSV1")
    .subtag(Tag(_trnType+"TRNRS")
      .element("TRNUID",uuid())
      .subtag(Tag("STATUS").element("CODE","0").element("SEVERITY","INFO"))
      .element("CLTCOOKIE","1")
      .subtag(_response));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::bankStatementResponse(const QDate& _dtstart) const
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));
  QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

  QString transactionlist;

  MyMoneyTransactionFilter filter;
  filter.setDateFilter(_dtstart,QDate::currentDate());
  filter.addAccount(m_account.id());
  QValueList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QValueList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
  while ( it_transaction != transactions.end() )
  {
    transactionlist += transaction( *it_transaction );
    ++it_transaction;
  }

  return messageResponse("BANK","STMT",Tag("STMTRS")
    .element("CURDEF","USD")
    .subtag(Tag("BANKACCTFROM").element("BANKID", iban()).element("ACCTID", accountnum()).element("ACCTTYPE", "CHECKING"))
    .subtag(Tag("BANKTRANLIST").element("DTSTART",dtstart_string).element("DTEND",dtnow_string).data(transactionlist))
    .subtag(Tag("LEDGERBAL").element("BALAMT",file->balance(m_account.id()).formatMoney(QString(),2)).element("DTASOF",dtnow_string )));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::creditCardStatementResponse(const QDate& _dtstart) const
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));
  QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

  QString transactionlist;

  MyMoneyTransactionFilter filter;
  filter.setDateFilter(_dtstart,QDate::currentDate());
  filter.addAccount(m_account.id());
  QValueList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QValueList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
  while ( it_transaction != transactions.end() )
  {
    transactionlist += transaction( *it_transaction );
    ++it_transaction;
  }

  return messageResponse("CREDITCARD","CCSTMT",Tag("CCSTMTRS")
    .element("CURDEF","USD")
    .subtag(Tag("CCACCTFROM").element("ACCTID", accountnum()))
    .subtag(Tag("BANKTRANLIST").element("DTSTART",dtstart_string).element("DTEND",dtnow_string).data(transactionlist))
    .subtag(Tag("LEDGERBAL").element("BALAMT",file->balance(m_account.id()).formatMoney(QString(),2)).element("DTASOF",dtnow_string )));
}

QString MyMoneyOfxConnector::investmentStatementResponse(const QDate& _dtstart) const
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));
  QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

  QString transactionlist;

  MyMoneyTransactionFilter filter;
  filter.setDateFilter(_dtstart,QDate::currentDate());
  filter.addAccount(m_account.id());
  filter.addAccount(m_account.accountList());
  QValueList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QValueList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
  while ( it_transaction != transactions.end() )
  {
    transactionlist += investmentTransaction( *it_transaction );
    ++it_transaction;
  }

  Tag securitylist("SECLIST");
  QCStringList accountids = m_account.accountList();
  QCStringList::const_iterator it_accountid = accountids.begin();
  while ( it_accountid != accountids.end() )
  {
    MyMoneySecurity equity = file->security(file->account(*it_accountid).currencyId());

    securitylist.subtag(Tag("STOCKINFO")
      .subtag(Tag("SECINFO")
        .subtag(Tag("SECID")
          .element("UNIQUEID",equity.id())
          .element("UNIQUEIDTYPE","KMYMONEY"))
        .element("SECNAME",equity.name())
        .element("TICKER",equity.tradingSymbol())
        .element("FIID",equity.id())));

    ++it_accountid;
  }

  return messageResponse("INVSTMT","INVSTMT",Tag("INVSTMTRS")
    .element("DTASOF", dtstart_string)
    .element("CURDEF","USD")
    .subtag(Tag("INVACCTFROM").element("BROKERID", fiorg()).element("ACCTID", accountnum()))
    .subtag(Tag("INVTRANLIST").element("DTSTART",dtstart_string).element("DTEND",dtnow_string).data(transactionlist))
    )
    + Tag("SECLISTMSGSRSV1").subtag(securitylist);
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::transaction(const MyMoneyTransaction& _t) const
{
  // This method creates a transaction tag using ONLY the elements that importer uses

  MyMoneyFile* file = MyMoneyFile::instance();

  //Use this version for bank/cc transactions
  MyMoneySplit s = _t.splitByAccount( m_account.id(), true );

  //TODO (Ace) Write "investmentTransaction()"...
  //Use this version for inv transactions
  //MyMoneySplit s = _t.splitByAccount( m_account.accountList(), true );

  Tag result ("STMTTRN");

  result
    // This is a temporary hack.  I don't use the trntype field in importing at all,
    // but libofx requires it to be there in order to import the file.
    .element("TRNTYPE","DEBIT")
    .element("DTPOSTED",_t.postDate().toString(Qt::ISODate).remove(QRegExp("[^0-9]")))
    .element("TRNAMT",s.value().formatMoney(QString(),2));

  if ( ! _t.bankID().isEmpty() )
    result.element("FITID",_t.bankID());
  else
    result.element("FITID",_t.id());

  if ( ! s.number().isEmpty() )
    result.element("CHECKNUM",s.number());

  if ( ! s.payeeId().isEmpty() )
    result.element("NAME",file->payee(s.payeeId()).name());

  if ( ! _t.memo().isEmpty() )
    result.element("MEMO",_t.memo());

  return result;
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::investmentTransaction(const MyMoneyTransaction& _t) const
{
 MyMoneyFile* file = MyMoneyFile::instance();

  //Use this version for inv transactions
  MyMoneySplit s = _t.splitByAccount( m_account.accountList(), true );

  QCString stockid = file->account(s.accountId()).currencyId();

  Tag invtran("INVTRAN");
  invtran.element("FITID",_t.id()).element("DTTRADE",_t.postDate().toString(Qt::ISODate).remove(QRegExp("[^0-9]")));
  if ( !_t.memo().isEmpty() )
    invtran.element("MEMO",_t.memo());

  if ( s.action() == MyMoneySplit::ActionBuyShares )
  {
    if ( s.shares().isNegative() )
    {
      return Tag("SELLSTOCK")
        .subtag(Tag("INVSELL")
          .subtag(invtran)
          .subtag(Tag("SECID").element("UNIQUEID",stockid).element("UNIQUEIDTYPE","KMYMONEY"))
          .element("UNITS",QString(((s.shares())).formatMoney(QString(),2)).remove(QRegExp("[^0-9.\\-]")))
          .element("UNITPRICE",QString((s.value()/s.shares()).formatMoney(QString(),2)).remove(QRegExp("[^0-9.]")))
          .element("TOTAL",QString((-s.value()).formatMoney(QString(),2)).remove(QRegExp("[^0-9.\\-]")))
          .element("SUBACCTSEC","CASH")
          .element("SUBACCTFUND","CASH"))
        .element("SELLTYPE","SELL");
    }
    else
    {
      return Tag("BUYSTOCK")
        .subtag(Tag("INVBUY")
          .subtag(invtran)
          .subtag(Tag("SECID").element("UNIQUEID",stockid).element("UNIQUEIDTYPE","KMYMONEY"))
          .element("UNITS",QString((s.shares()).formatMoney(QString(),2)).remove(QRegExp("[^0-9.\\-]")))
          .element("UNITPRICE",QString((s.value()/s.shares()).formatMoney(QString(),2)).remove(QRegExp("[^0-9.]")))
          .element("TOTAL",QString((-(s.value())).formatMoney(QString(),2)).remove(QRegExp("[^0-9.\\-]")))
          .element("SUBACCTSEC","CASH")
          .element("SUBACCTFUND","CASH"))
        .element("BUYTYPE","BUY");
    }
  }
  else if ( s.action() == MyMoneySplit::ActionReinvestDividend )
  {
    // Should the TOTAL tag really be negative for a REINVEST?  That's very strange, but
    // it's what they look like coming from my bank, and I can't find any information to refute it.

    return Tag("REINVEST")
      .subtag(invtran)
      .subtag(Tag("SECID").element("UNIQUEID",stockid).element("UNIQUEIDTYPE","KMYMONEY"))
      .element("INCOMETYPE","DIV")
      .element("TOTAL",QString((-s.value()).formatMoney(QString(),2)).remove(QRegExp("[^0-9.\\-]")))
      .element("SUBACCTSEC","CASH")
      .element("UNITS",QString((s.shares()).formatMoney(QString(),2)).remove(QRegExp("[^0-9.\\-]")))
      .element("UNITPRICE",QString((s.value()/s.shares()).formatMoney(QString(),2)).remove(QRegExp("[^0-9.]")));
  }
  else if ( s.action() == MyMoneySplit::ActionDividend )
  {
    // find the split with the category, which has the actual amount of the dividend
    QValueList<MyMoneySplit> splits = _t.splits();
    QValueList<MyMoneySplit>::const_iterator it_split = splits.begin();
    bool found = false;
    while( it_split != splits.end() )
    {
      QCString accid = (*it_split).accountId();
      MyMoneyAccount acc = file->account(accid);
      if ( acc.accountType() == MyMoneyAccount::Income || acc.accountType() == MyMoneyAccount::Expense )
      {
        found = true;
        break;
      }
      ++it_split;
    }

    if ( found )
      return Tag("INCOME")
        .subtag(invtran)
        .subtag(Tag("SECID").element("UNIQUEID",stockid).element("UNIQUEIDTYPE","KMYMONEY"))
        .element("INCOMETYPE","DIV")
        .element("TOTAL",QString((-(*it_split).value()).formatMoney(QString(),2)).remove(QRegExp("[^0-9\\.\\-]")))
        .element("SUBACCTSEC","CASH")
        .element("SUBACCTFUND","CASH");
    else
      return Tag("ERROR").element("DETAILS","Unable to determine the amount of this income transaction.");
  }

  //FIXME: Do something useful with these errors
  return Tag("ERROR").element("DETAILS","This transaction contains an unsupported action type");
}
#endif
#endif // USE_OFX_DIRECTCONNECT
