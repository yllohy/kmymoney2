/***************************************************************************
                      mymoneygncreader  -  description
                         -------------------
begin                : Wed Mar 3 2004
copyright            : (C) 2000-2004 by Michael Edwardes
email                : mte@users.sourceforge.net
                       Javier Campos Morales <javi_c@users.sourceforge.net>
                       Felix Rodriguez <frodriguez@users.sourceforge.net>
                       John C <thetacoturtle@users.sourceforge.net>
                       Thomas Baumgart <ipwizard@users.sourceforge.net>
                       Kevin Tambascio <ktambascio@users.sourceforge.net>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdarg.h>

// ----------------------------------------------------------------------------
// QT Includes
#include <qfile.h>
#include <qmap.h>
#include <qobject.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qdatetime.h>

// ----------------------------------------------------------------------------
// Third party Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "config.h"
#include "mymoneygncreader.h"
#ifndef _GNCFILEANON
  #include "../mymoney/storage/imymoneystorage.h"
  #include "../kmymoneyutils.h"
  #include "../mymoney/mymoneyfile.h"
  #include "../mymoney/mymoneyprice.h"
  #include "../dialogs/kgncimportoptionsdlg.h"

  #define TRY try {
  #define PASS } catch (MyMoneyException *e) { throw e; }
#else
  #include "../mymoneymoney/mymoneymoney.h"
  #define TRY
  #define PASS
  #define MYMONEYEXCEPTION QString
  #define MyMoneyException QString
  #define PACKAGE "KMyMoney"
#endif // _GNCFILEANON

// init static variables
// to hold gnucash count data (only used for progress bar)
int GncObject::m_gncCommodityCount = 0;
int GncObject::m_gncAccountCount = 0;
int GncObject::m_gncTransactionCount = 0;
int GncObject::m_gncScheduleCount = 0;
double MyMoneyGncReader::m_fileHideFactor = 0.0;
double GncObject::m_moneyHideFactor;

// user options
void MyMoneyGncReader::setOptions () {
#ifndef _GNCFILEANON
  KGncImportOptionsDlg dlg; // display the dialog to allow the user to set own options
  if (dlg.exec()) {
    // set users input options
    m_dropSuspectSchedules = dlg.scheduleOption();
    m_investmentOption = dlg.investmentOption();
    gncdebug = dlg.generalDebugOption();
    xmldebug = dlg.xmlDebugOption();
    bAnonymize = dlg.anonymizeOption();
  } else {
    // user declined, so set some sensible defaults
    m_dropSuspectSchedules = false;
    // investment option - 0, create investment a/c per stock a/c, 1 = single new investment account, 2 = prompt for each stock
    // option 2 doesn't really work too well at present
    m_investmentOption = 0;
    gncdebug = false; // general debug messages
    xmldebug = false; // xml trace
    bAnonymize = false; // anonymize input
  }
  // no dialog option for the following; it will set base currency, and print actual XML data
  developerDebug = false;
  // set your fave currency here to save getting that enormous dialog each time you run a test
  // especially if you have to scroll down to USD...
  if (developerDebug) m_storage->setValue ("kmm-baseCurrency", "GBP");
#endif // _GNCFILEANON
}

GncObject::GncObject () {
  m_v.setAutoDelete (true);
  m_gncCommodityCount = 0;
  m_gncAccountCount = 0;
  m_gncTransactionCount = 0;
  m_gncScheduleCount = 0;
}

// Check that the current element is of a version we are coded for
void GncObject::checkVersion (const QString& elName, const QXmlAttributes& elAttrs) {
#ifdef _GNCFILEANON // suppress all checks
    static bool validHeaderFound = true;
    return;
#else
  // a list of elements to check, and the required version numbers
  static const QString versionList[] = {"gnc:book 2.0.0", "gnc:commodity 2.0.0", "gnc:pricedb 1",
                                        "gnc:account 2.0.0", "gnc:transaction 2.0.0", "gnc:schedxaction 1.0.0",
                                        "gnc:freqspec 1.0.0", "zzz" // zzz = stopper
                                       };
  static bool validHeaderFound = false;
  TRY
  if (!validHeaderFound) {  // check the header first
    if (elName != "gnc-v2") throw new MYMONEYEXCEPTION (QObject::tr("Invalid header for file. Should be gnc-v2"));
  }
  validHeaderFound = true;

  for (uint i = 0; versionList[i] != "zzz"; i++) {
    if (versionList[i].section (' ', 0, 0) == elName) {
      if (elAttrs.value("version") != versionList[i].section(' ', 1, 1)) {
        QString em = (QObject::tr(QString().sprintf("chkVersion: Element %s must have version %s",
                                  elName.latin1(), versionList[i].section(' ', 1, 1).latin1())));
        throw new MYMONEYEXCEPTION (em);
      }
    }
  }
  return ;
  PASS
#endif // _GNCFILEANON
}

// Check if this element is in the current object's sub element list
GncObject *GncObject::isSubElement (const QString& elName, const QXmlAttributes& elAttrs) {
  TRY
  uint i;
  GncObject *next = 0;
  for (i = 0; i < m_subElementListCount; i++) {
    if (elName == m_subElementList[i]) {
      m_state = i;
      next = startSubEl(); // go create the sub object
      if (next != 0) {
        next->initiate(elName, elAttrs); // initialize it
        next->m_elementName = elName;    // save it's name so we can identify the end
      }
      break;
    }
  }
  return (next);
  PASS
}

// Check if this element is in the current object's data element list
bool GncObject::isDataElement (const QString &elName, const QXmlAttributes& elAttrs) {
  TRY
  uint i;
  for (i = 0; i < m_dataElementListCount; i++) {
    if (elName == m_dataElementList[i]) {
      m_state = i;
      dataEl(elAttrs); // go set the pointer so the data can be stored
      return (true);
    }
  }
  m_dataPtr = 0; // we don't need this, so make sure we don't store extraneous data
  return (false);
  PASS
}
void GncObject::adjustHideFactor () {
  m_moneyHideFactor = pMain->m_fileHideFactor * (1.0 + (int)(200.0 * rand()/(RAND_MAX+1.0))) / 100.0;
}

// data anonymizer
QString GncObject::hide (QString data, unsigned int anonClass) {
  TRY
  if (!pMain->bAnonymize) return (data); // no anonymizing required
  // counters used to generate names for anonymizer
  static int nextAccount;
  static int nextEquity;
  static int nextPayee;
  static int nextSched;
  static QMap<QString, QString> anonPayees; // to check for duplicate payee names
  static QMap<QString, QString> anonStocks; // for reference to equities

  QString result (data);
  QMap<QString, QString>::Iterator it;
  MyMoneyMoney in, mresult;
  switch (anonClass) {
  case ASIS: break;                  // this is not personal data
  case SUPPRESS: result = ""; break; // this is personal and is not essential
  case NXTACC: result.sprintf ("%s %.6d", QObject::tr("Account").latin1(), ++nextAccount); break; // generate account name
  case NXTEQU:   // generate/return an equity name
    it = anonStocks.find (data);
    if (it == anonStocks.end()) {
      result.sprintf ("%s %.6d", QObject::tr("Stock").latin1(), ++nextEquity);
      anonStocks.insert (data, result);
    } else {
      result = (*it).data();
    }
    break;
  case NXTPAY:   // genearet/return a payee name
    it = anonPayees.find (data);
    if (it == anonPayees.end()) {
      result.sprintf ("%s %.6d", QObject::tr("Payee").latin1(), ++nextPayee);
      anonPayees.insert (data, result);
    } else {
      result = (*it).data();
    }
    break;
  case NXTSCHD: result.sprintf ("%s %.6d", QObject::tr("Schedule").latin1(), ++nextSched); break; // generate a schedule name
  case MONEY1:
    in = MyMoneyMoney(data);
    if (data == "-1/0") in = MyMoneyMoney (0); // spurious gnucash data - causes a crash sometimes
    mresult = MyMoneyMoney(m_moneyHideFactor) * in;
    mresult.convert(10000);
    result = mresult.toString();
    break;
  case MONEY2:
    in = MyMoneyMoney(data);
    if (data == "-1/0") in = MyMoneyMoney (0);
    mresult  = MyMoneyMoney(m_moneyHideFactor) * in;
    mresult.convert(10000);
    mresult.setThousandSeparator (' ');
    result = mresult.formatMoney();
    break;
  }
  return (result);
  PASS
}
 
// dump current object data values // only called if gncdebug set
void GncObject::debugDump () {
  uint i;
  qDebug ("Object %s", m_elementName.latin1());
  for (i = 0; i < m_dataElementListCount; i++) {
    qDebug ("%s = %s", m_dataElementList[i].latin1(), m_v.at(i)->latin1());
  }
}
//*****************************************************************
GncFile::GncFile () {
  static const QString subEls[] = {"gnc:book", "gnc:count-data", "gnc:commodity", "price",
                                   "gnc:account", "gnc:transaction", "gnc:template-transactions",
                                   "gnc:schedxaction"
                                  };
  m_subElementList = subEls;
  m_subElementListCount = END_FILE_SELS;
  m_dataElementListCount = 0;
  m_processingTemplates = false;
  m_bookFound = false;
}

GncFile::~GncFile () {}

GncObject *GncFile::startSubEl() {
  TRY
  if (pMain->xmldebug) qDebug ("File start subel m_state %d", m_state);
  GncObject *next = 0;
  switch (m_state) {
  case BOOK:
    if (m_bookFound) throw new MYMONEYEXCEPTION (QObject::tr("This version of the importer cannot handle multi-book files."));
    m_bookFound = true;
    break;
  case COUNT: next = new GncCountData; break;
  case CMDTY: next = new GncCommodity; break;
  case PRICE: next = new GncPrice; break;
  case ACCT:
    // accounts within the template section are ignored
    if (!m_processingTemplates) next = new GncAccount;
    break;
  case TX: next = new GncTransaction (m_processingTemplates); break;
  case TEMPLATES: m_processingTemplates = true; break;
  case SCHEDULES: m_processingTemplates = false; next = new GncSchedule; break;
  default: throw new MYMONEYEXCEPTION ("GncFile rcvd invalid state");
  }
  return (next);
  PASS
}

void GncFile::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("File end subel");
  if (!m_processingTemplates) delete subObj; // template txs must be saved awaiting schedules
  m_dataPtr = 0;
  return ;
}
//****************************************** GncDate *********************************************
GncDate::GncDate () {
  m_subElementListCount = 0;
  static const QString dEls[] = {"ts:date", "gdate"};
  m_dataElementList = dEls;
  m_dataElementListCount = END_Date_DELS;
  static const unsigned int anonClasses[] = {ASIS, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (new QString (""));
}

GncDate::~GncDate() {}
//*************************************GncCmdtySpec***************************************
GncCmdtySpec::GncCmdtySpec () {
  m_subElementListCount = 0;
  static const QString dEls[] = {"cmdty:space", "cmdty:id"};
  m_dataElementList = dEls;
  m_dataElementListCount = END_CmdtySpec_DELS;
  static const unsigned int anonClasses[] = {ASIS, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (new QString (""));
}

GncCmdtySpec::~GncCmdtySpec () {}

QString GncCmdtySpec::hide(QString data, unsigned int) {
  // hide equity names, but not currency names
  unsigned int newClass = ASIS;
  switch (m_state) {
  case CMDTYID:
    if (!isCurrency()) newClass = NXTEQU;
  }
  return (GncObject::hide (data, newClass));
}
//************* GncKvp********************************************
GncKvp::GncKvp () {
  m_subElementListCount = END_Kvp_SELS;
  static const QString subEls[] = {"slot"}; // kvp's may be nested
  m_subElementList = subEls;
  m_dataElementListCount = END_Kvp_DELS;
  static const QString dataEls[] = {"slot:key", "slot:value"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (new QString (""));
  m_kvpList.setAutoDelete (true);
}

GncKvp::~GncKvp () {}

void GncKvp::dataEl (const QXmlAttributes& elAttrs) {
  switch (m_state) {
  case VALUE:
    m_kvpType = elAttrs.value("type");
  }
  m_dataPtr = m_v.at(m_state);
  if (key().contains ("formula")) {
    m_anonClass = MONEY2;
  } else {
    m_anonClass = ASIS;
  }
  return ;
}

GncObject *GncKvp::startSubEl() {
  if (pMain->xmldebug) qDebug ("Kvp start subel m_state %d", m_state);
  TRY
  GncObject *next = 0;
  switch (m_state) {
  case KVP: next = new GncKvp; break;
  default: throw new MYMONEYEXCEPTION ("GncKvp rcvd invalid m_state ");
  }
  return (next);
  PASS
}

void GncKvp::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("Kvp end subel");
  m_kvpList.append (subObj);
  m_dataPtr = 0;
  return ;
}
//*********************************GncCountData***************************************
GncCountData::GncCountData() {
  m_subElementListCount = 0;
  m_dataElementListCount = 0;
  m_v.append (new QString ("")); // only 1 data item
}

GncCountData::~GncCountData () {}

void GncCountData::initiate (const QString&, const QXmlAttributes& elAttrs) {
  m_countType = elAttrs.value ("cd:type");
  m_dataPtr = m_v.at(0);
  return ;
}

void GncCountData::terminate () {
  if (m_countType == "commodity") {
    m_gncCommodityCount = m_v.at(0)->toInt(); return ;
  }
  if (m_countType == "account") {
    m_gncAccountCount = m_v.at(0)->toInt(); return ;
  }
  if (m_countType == "transaction") {
    m_gncTransactionCount = m_v.at(0)->toInt(); return ;
  }
  if (m_countType == "schedxaction") {
    m_gncScheduleCount = m_v.at(0)->toInt(); return ;
  }
  return ;
}
//*********************************GncCommodity***************************************
GncCommodity::GncCommodity () {
  m_subElementListCount = 0;
  static const QString dEls[] = {"cmdty:space", "cmdty:id", "cmdty:name", "cmdty:fraction"};
  m_dataElementList = dEls;
  m_dataElementListCount = END_Commodity_DELS;
  static const unsigned int anonClasses[] = {ASIS, NXTEQU, SUPPRESS, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (new QString (""));
}

GncCommodity::~GncCommodity () {}

void GncCommodity::terminate() {
  TRY
  pMain->convertCommodity (this);
  return ;
  PASS
}
//************* GncPrice********************************************
GncPrice::GncPrice () {
  static const QString subEls[] = {"price:commodity", "price:currency", "price:time"};
  m_subElementList = subEls;
  m_subElementListCount = END_Price_SELS;
  m_dataElementListCount = END_Price_DELS;
  static const QString dataEls[] = {"price:value"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (new QString (""));
}

GncPrice::~GncPrice () {
  delete m_vpCommodity; delete m_vpCurrency; delete m_vpPriceDate;
}

GncObject *GncPrice::startSubEl() {
  TRY
  GncObject *next = 0;
  switch (m_state) {
  case CMDTY: next = new GncCmdtySpec; break;
  case CURR: next = new GncCmdtySpec; break;
  case PRICEDATE: next = new GncDate; break;
  default: throw new MYMONEYEXCEPTION ("GncPrice rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncPrice::endSubEl(GncObject *subObj) {
  TRY
  switch (m_state) {
  case CMDTY: m_vpCommodity = static_cast<GncCmdtySpec *>(subObj); break;
  case CURR: m_vpCurrency = static_cast<GncCmdtySpec *>(subObj); break;
  case PRICEDATE: m_vpPriceDate = static_cast<GncDate *>(subObj); break;
  default: throw new MYMONEYEXCEPTION ("GncPrice rcvd invalid m_state");
  }
  PASS
}

void GncPrice::terminate() {
  TRY
  pMain->convertPrice (this);
  return ;
  PASS
}
//************* GncAccount********************************************
GncAccount::GncAccount () {
  m_subElementListCount = END_Account_SELS;
  static const QString subEls[] = {"act:commodity", "slot"};
  m_subElementList = subEls;
  m_dataElementListCount = END_Account_DELS;
  static const QString dataEls[] = {"act:id", "act:name", "act:description",
                                    "act:type", "act:parent"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, NXTACC, SUPPRESS, ASIS, ASIS};
  m_anonClassList = anonClasses;
  kvpList.setAutoDelete (true);
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (new QString (""));
}

GncAccount::~GncAccount () {
  delete m_vpCommodity;
}

GncObject *GncAccount::startSubEl() {
  TRY
  if (pMain->xmldebug) qDebug ("Account start subel m_state %d", m_state);
  GncObject *next = 0;
  switch (m_state) {
  case CMDTY: next = new GncCmdtySpec; break;
  case KVP: next = new GncKvp; break;
  default: throw new MYMONEYEXCEPTION ("GncAccount rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncAccount::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("Account end subel");
  switch (m_state) {
  case CMDTY: m_vpCommodity = static_cast<GncCmdtySpec *>(subObj); break;
  case KVP: kvpList.append (subObj);
  }
  return ;
}

void GncAccount::terminate() {
  TRY
  pMain->convertAccount (this);
  return ;
  PASS
}
//************* GncTransaction********************************************
GncTransaction::GncTransaction (bool processingTemplates) {
  m_subElementListCount = END_Transaction_SELS;
  static const QString subEls[] = {"trn:currency", "trn:date-posted", "trn:date-entered", "trn:split"};
  m_subElementList = subEls;
  m_dataElementListCount = END_Transaction_DELS;
  static const QString dataEls[] = {"trn:id", "trn:num", "trn:description"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, SUPPRESS, NXTPAY};
  m_anonClassList = anonClasses;
  adjustHideFactor();
  m_template = processingTemplates;
  m_splitList.setAutoDelete (true);
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (new QString (""));
}

GncTransaction::~GncTransaction () {
  delete m_vpCurrency; delete m_vpDatePosted; delete m_vpDateEntered;
}

GncObject *GncTransaction::startSubEl() {
  TRY
  if (pMain->xmldebug) qDebug ("Transaction start subel m_state %d", m_state);
  GncObject *next = 0;
  switch (m_state) {
  case CURRCY: next = new GncCmdtySpec; break;
  case POSTED:
  case ENTERED:
    next = new GncDate; break;
  case SPLIT:
    if (isTemplate()) {
      next = new GncTemplateSplit;
    } else {
      next = new GncSplit;
    }
    break;
  default: throw new MYMONEYEXCEPTION ("GncTransaction rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncTransaction::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("Transaction end subel");
  switch (m_state) {
  case CURRCY: m_vpCurrency = static_cast<GncCmdtySpec *>(subObj); break;
  case POSTED: m_vpDatePosted = static_cast<GncDate *>(subObj); break;
  case ENTERED: m_vpDateEntered = static_cast<GncDate *>(subObj); break;
  case SPLIT: m_splitList.append (subObj); break;
  }
  return ;
}

void GncTransaction::terminate() {
  TRY
  if (isTemplate()) {
    pMain->saveTemplateTransaction(this);
  } else {
    pMain->convertTransaction (this);
  }
  return ;
  PASS
}
//************* GncSplit********************************************
GncSplit::GncSplit () {
  m_subElementListCount = END_Split_SELS;
  static const QString subEls[] = {"split:reconcile-date"};
  m_subElementList = subEls;
  m_dataElementListCount = END_Split_DELS;
  static const QString dataEls[] = {"split:id", "split:memo", "split:reconciled-state", "split:value",
                                    "split:quantity", "split:account"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, SUPPRESS, ASIS, MONEY1, MONEY1, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (new QString (""));
  m_vpDateReconciled = NULL;
}

GncSplit::~GncSplit () {
  delete m_vpDateReconciled;
}

GncObject *GncSplit::startSubEl () {
  TRY
  GncObject *next = 0;
  switch (m_state) {
  case RECDATE: next = new GncDate; break;
  default: throw new MYMONEYEXCEPTION ("GncTemplateSplit rcvd invalid m_state ");
  }
  return (next);
  PASS
}

void GncSplit::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("Split end subel");
  switch (m_state) {
  case RECDATE: m_vpDateReconciled = static_cast<GncDate *>(subObj); break;
  }
  return ;
}
//************* GncTemplateSplit********************************************
GncTemplateSplit::GncTemplateSplit () {
  m_subElementListCount = END_TemplateSplit_SELS;
  static const QString subEls[] = {"slot"};
  m_subElementList = subEls;
  m_dataElementListCount = END_TemplateSplit_DELS;
  static const QString dataEls[] = {"split:id", "split:memo", "split:reconciled-state", "split:value",
                                    "split:quantity", "split:account"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, SUPPRESS, ASIS, MONEY1, MONEY1, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (new QString (""));
  m_kvpList.setAutoDelete (true);
}

GncTemplateSplit::~GncTemplateSplit () {}

GncObject *GncTemplateSplit::startSubEl() {
  if (pMain->xmldebug) qDebug ("TemplateSplit start subel m_state %d", m_state);
  TRY
  GncObject *next = 0;
  switch (m_state) {
  case KVP: next = new GncKvp; break;
  default: throw new MYMONEYEXCEPTION ("GncTemplateSplit rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncTemplateSplit::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("TemplateSplit end subel");
  m_kvpList.append (subObj);
  m_dataPtr = 0;
  return ;
}
//************* GncSchedule********************************************
GncSchedule::GncSchedule () {
  m_subElementListCount = END_Schedule_SELS;
  static const QString subEls[] = {"sx:start", "sx:last", "sx:end", "gnc:freqspec"};
  m_subElementList = subEls;
  m_dataElementListCount = END_Schedule_DELS;
  static const QString dataEls[] = {"sx:name", "sx:autoCreate", "sx:autoCreateNotify",
                                    "sx:autoCreateDays", "sx:advanceCreateDays", "sx:advanceRemindDays",
                                    "sx:instanceCount", "sx:num-occur",
                                    "sx:rem-occur", "sx:templ-acct"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {NXTSCHD, ASIS, ASIS, ASIS, ASIS, ASIS, ASIS, ASIS, ASIS, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (new QString (""));
  m_vpStartDate = m_vpLastDate = m_vpEndDate = NULL;
}

GncSchedule::~GncSchedule () {
  delete m_vpStartDate; delete m_vpLastDate; delete m_vpEndDate; delete m_vpFreqSpec;
}

GncObject *GncSchedule::startSubEl() {
  if (pMain->xmldebug) qDebug ("Schedule start subel m_state %d", m_state);
  TRY
  GncObject *next = 0;
  switch (m_state) {
  case STARTDATE:
  case LASTDATE:
  case ENDDATE: next = new GncDate; break;
  case FREQ: next = new GncFreqSpec; break;
  default: throw new MYMONEYEXCEPTION ("GncSchedule rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncSchedule::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("Schedule end subel");
  switch (m_state) {
  case STARTDATE: m_vpStartDate = static_cast<GncDate *>(subObj); break;
  case LASTDATE: m_vpLastDate = static_cast<GncDate *>(subObj); break;
  case ENDDATE: m_vpEndDate = static_cast<GncDate *>(subObj); break;
  case FREQ: m_vpFreqSpec = static_cast<GncFreqSpec *>(subObj); break;
  }
  return ;
}

void GncSchedule::terminate() {
  TRY
  pMain->convertSchedule (this);
  return ;
  PASS
}
//************* GncFreqSpec********************************************
GncFreqSpec::GncFreqSpec () {
  m_subElementListCount = END_FreqSpec_SELS;
  static const QString subEls[] = {"gnc:freqspec"};
  m_subElementList = subEls;
  m_dataElementListCount = END_FreqSpec_DELS;
  static const QString dataEls[] = {"fs:ui_type", "fs:monthly", "fs:daily", "fs:weekly", "fs:interval",
                                    "fs:offset", "fs:day"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, ASIS, ASIS, ASIS, ASIS, ASIS, ASIS      };
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (new QString (""));
  m_fsList.setAutoDelete (true);
}

GncFreqSpec::~GncFreqSpec () {}

GncObject *GncFreqSpec::startSubEl() {
  TRY
  if (pMain->xmldebug) qDebug ("FreqSpec start subel m_state %d", m_state);

  GncObject *next = 0;
  switch (m_state) {
  case COMPO: next = new GncFreqSpec; break;
  default: throw new MYMONEYEXCEPTION ("GncFreqSpec rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncFreqSpec::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("FreqSpec end subel");
  switch (m_state) {
  case COMPO: m_fsList.append (subObj); break;
  }
  m_dataPtr = 0;
  return ;
}

void GncFreqSpec::terminate() {
  pMain->convertFreqSpec (this);
  return ;
}
/************************************************************************************************
                         XML Reader
************************************************************************************************/
void XmlReader::processFile (QIODevice* pDevice) {
  m_source = new QXmlInputSource (pDevice); // set up the Qt XML reader
  m_reader = new QXmlSimpleReader;
  m_reader->setContentHandler (this);
  // go read the file
  if (!m_reader->parse (m_source)) {
    throw new MYMONEYEXCEPTION (QObject::tr("Input file cannot be parsed; may be corrupt\n%s", errorString().latin1()));
  }
  delete m_reader;
  delete m_source;
  return ;
}

// XML handling routines
bool XmlReader::startDocument() {
  m_os.setAutoDelete (true);
  m_co = new GncFile; // create initial object, push to stack , pass it the 'main' pointer
  m_os.push (m_co);
  m_co->setPm (pMain);
#ifdef _GNCFILEANON
  pMain->oStream << "<?xml version=\"1.0\"?>";
  lastType = -1;
  indentCount = 0;
#endif // _GNCFILEANON
   return (true);
}

bool XmlReader::startElement (const QString&, const QString&, const QString& elName ,
                              const QXmlAttributes& elAttrs) {
  try {
    if (pMain->gncdebug) qDebug ("XML start - %s", elName.latin1());
#ifdef _GNCFILEANON
    int i;
    QString spaces;
    // anonymizer - write data
    if (elName == "gnc:book" || elName == "gnc:count-data" || elName == "book:id") lastType = -1;
    pMain->oStream << endl;
    switch (lastType) {
    case 0: indentCount += 2;
    case 2: spaces.fill (' ', indentCount); pMain->oStream << spaces.latin1(); break;
    }
    pMain->oStream << '<' << elName;
    for (i = 0; i < elAttrs.count(); i++) {
          pMain->oStream << ' ' << elAttrs.qName(i) << '='  << '"' << elAttrs.value(i) << '"';
    }
    pMain->oStream << '>';
    lastType = 0;
#endif // _GNCFILEANON
    GncObject::checkVersion (elName, elAttrs);
    // check if this is a sub object element; if so, push stack and initialize
    GncObject *temp = m_co->isSubElement (elName, elAttrs);
    if (temp != 0) {
      m_os.push (temp);
      m_co = m_os.top();
      m_co->setPm (pMain); // pass the 'main' pointer to the sub object
      return (true);
    }
    // check for a data element
    if (m_co->isDataElement (elName, elAttrs)) return (true);
    return (true);
  } catch (MyMoneyException *e) {
#ifndef _GNCFILEANON
    // we can't pass on exceptions here coz the XML reader won't catch them and we just abort
    QMessageBox::critical (0, PACKAGE, QObject::tr("Import failed\n\n") + e->what(),
                           QMessageBox::Abort, QMessageBox::NoButton , QMessageBox::NoButton);
    qFatal ("%s", e->what().latin1());
#else
    qFatal ("%s", e->latin1());
#endif // _GNCFILEANON
  }
  return (true); // to keep compiler happy
}

bool XmlReader::endElement( const QString&, const QString&, const QString&elName ) {
  try {
    if (pMain->xmldebug) qDebug ("XML end - %s", elName.latin1());
#ifdef _GNCFILEANON
    QString spaces;
    switch (lastType) {
    case 2:
      indentCount -= 2; spaces.fill (' ', indentCount); pMain->oStream << endl << spaces.latin1(); break;
    }
    pMain->oStream << "</" << elName << '>' ;
    lastType = 2;
#endif // _GNCFILEANON
    m_co->resetDataPtr(); // so we don't get extraneous data loaded into the variables
    if (elName == m_co->getElName()) { // check if this is the end of the current object
      if (pMain->gncdebug) m_co->debugDump(); // dump the object data (temp)
      // call the terminate routine, pop the stack, and advise the parent that it's done
      m_co->terminate();
      GncObject *temp = m_co;
      m_os.pop();
      m_co = m_os.top();
      m_co->endSubEl (temp);
    }
    return (true);
  } catch (MyMoneyException *e) {
#ifndef _GNCFILEANON
    // we can't pass on exceptions here coz the XML reader won't catch them and we just abort
    QMessageBox::critical (0, PACKAGE, QObject::tr("Import failed\n\n") + e->what(),
                           QMessageBox::Abort, QMessageBox::NoButton , QMessageBox::NoButton);
    qFatal ("%s", e->what().latin1());
#else
    qFatal ("%s", e->latin1());
#endif // _GNCFILEANON
  }
  return (true); // to keep compiler happy
}

bool XmlReader::characters (const QString &data) {
  if (pMain->xmldebug) qDebug ("XML Data received - %d bytes", data.length());
  QString pData = data.stripWhiteSpace(); // data may contain line feeds and indentation spaces
  if (!pData.isEmpty()) {
    if (pMain->developerDebug) qDebug ("XML Data - %s", pData.latin1());
    m_co->storeData (pData); //go store it
#ifdef _GNCFILEANON
    QString anonData = m_co->getData ();
    if (!anonData.isEmpty()) {
      pMain->oStream << anonData; // write anonymized data
    } else {
      pMain->oStream << pData; // write original data
    }
    lastType = 1;
#endif // _GNCFILEANON
  }
  return (true);
}

bool XmlReader::endDocument() {
#ifdef _GNCFILEANON
  pMain->oStream << endl << endl;
  pMain->oStream << "<!-- Local variables: -->" << endl;
  pMain->oStream << "<!-- mode: xml        -->" << endl;
  pMain->oStream << "<!-- End:             -->" << endl;
#endif // _GNCFILEANON
  return (true);
}

/*******************************************************************************************
                                 Main class for this module
  Controls overall operation of the importer
********************************************************************************************/ 
//***************** Constructor ***********************
MyMoneyGncReader::MyMoneyGncReader() {
#ifndef _GNCFILEANON
  m_storage = NULL;
  m_messageList.setAutoDelete (true);
  m_templateList.setAutoDelete (true);
#endif // _GNCFILEANON
  m_commodityCount = m_priceCount = m_accountCount = m_transactionCount = m_templateCount = m_scheduleCount = 0;
}

//***************** Destructor *************************
MyMoneyGncReader::~MyMoneyGncReader() {}

//**************************** Main Entry Point ************************************
#ifndef _GNCFILEANON
void MyMoneyGncReader::readFile(QIODevice* pDevice, IMyMoneySerialize* storage) {

  Q_CHECK_PTR (pDevice);
  Q_CHECK_PTR (storage);

  m_storage = dynamic_cast<IMyMoneyStorage *>(storage);
  qDebug ("Entering gnucash importer");
  setOptions ();
  // get a file anonymization factor from the user
  if (bAnonymize) setFileHideFactor ();
  m_defaultPayee = createPayee (QObject::tr("Unknown payee"));

  xr = new XmlReader (this);
  try {
    xr->processFile (pDevice);
    terminate (); // do all the wind-up things
  } catch (MyMoneyException *e) {
    QMessageBox::critical (0, PACKAGE, QObject::tr("Import failed\n\n") + e->what(),
                           QMessageBox::Abort, QMessageBox::NoButton , QMessageBox::NoButton);
    qFatal ("%s", e->what().latin1());
  } // end catch
  signalProgress ( -1, -1); // switch off progress bar
  delete xr;
  qDebug ("Exiting gnucash importer");
  return ;
}
#else
// Control code for the file anonymizer
void MyMoneyGncReader::readFile(QString in, QString out) {
  QFile pDevice (in);
  if (!pDevice.open (IO_ReadOnly)) qFatal ("Can't open input file");
  QFile outFile (out);
  if (!outFile.open (IO_WriteOnly)) qFatal ("Can't open output file");
  oStream.setDevice (&outFile);
  bAnonymize = true;
  // get a file anonymization factor from the user
  setFileHideFactor ();
  xr = new XmlReader (this);
  try {
    xr->processFile (&pDevice);
  } catch (MyMoneyException *e) {
    qFatal ("%s", e->latin1());
  } // end catch
  delete xr;
  pDevice.close();
  outFile.close();
  return ;
}

#include <qapplication.h>
int main (int argc, char ** argv) {
    QApplication a (argc, argv);
    MyMoneyGncReader m;
    QString inFile, outFile;
    
    if (argc > 0) inFile = a.argv()[1];
    if (argc > 1) outFile = a.argv()[2];
    if (inFile.isEmpty()) {
        inFile = QFileDialog::getOpenFileName("",
                    "Gnucash files(*.nc *)",
                    0);
    }
    if (inFile.isEmpty()) qFatal ("Input file required");
    if (outFile.isEmpty()) outFile = inFile + ".anon";
    m.readFile (inFile, outFile);
    qFatal ("finished");
}
#endif // _GNCFILEANON

void MyMoneyGncReader::setFileHideFactor () {
#define MINFILEHIDEF 0.01
#define MAXFILEHIDEF 99.99
    srand (QTime::currentTime().second()); // seed randomizer for anonymize
    m_fileHideFactor = 0.0;
    while (m_fileHideFactor == 0.0) {
      m_fileHideFactor = QInputDialog::getDouble (
        QObject::tr ("Disguise your wealth"),
        QObject::tr (QString ("Each monetary value on your file will be multiplied by a random number between 0.01 and 1.99\n"
	             "with a different value used for each transaction. In addition, to further disguise the true\n"
		             "values, you may enter a number between %1 and %2 which will be applied to all values.\n"
		             "These numbers will not be stored in the file.").arg(MINFILEHIDEF).arg(MAXFILEHIDEF)),
        	(1.0 + (int)(1000.0 * rand() / (RAND_MAX + 1.0))) / 100.0,
        MINFILEHIDEF, MAXFILEHIDEF, 2);
    }
}
#ifndef _GNCFILEANON
//********************************* convertCommodity *******************************************
void MyMoneyGncReader::convertCommodity (const GncCommodity *gcm) {
  Q_CHECK_PTR (gcm);
  MyMoneySecurity equ;
  if (m_commodityCount == 0) signalProgress (0, gcm->gncCommodityCount(), QObject::tr("Loading commodities..."));
  if (!gcm->isCurrency()) { // currencies should not be present here but...
    equ.setName (gcm->name());
    equ.setTradingSymbol (gcm->id());
    equ.setTradingMarket (gcm->space()); // the 'space' may be market or quote source, dep on what the user did
    equ.setValue ("kmm-online-source", gcm->space()); // we don't know, so use it as both
    equ.setTradingCurrency (""); // not available here, will set from pricedb or transaction
    equ.setSecurityType (MyMoneySecurity::SECURITY_STOCK); // default to it being a stock
    //tell the storage objects we have a new equity object.
    m_storage->addSecurity(equ);

    //assign the gnucash id as the key into the map to find our id
    if (gncdebug) qDebug ("mapping, key = %s, id = %s", gcm->id().latin1(), equ.id().data());
    m_mapEquities[gcm->id().utf8()] = equ.id();
  }
  signalProgress (++m_commodityCount, 0);
  return ;
}

//******************************* convertPrice ************************************************
void MyMoneyGncReader::convertPrice (const GncPrice *gpr) {
  Q_CHECK_PTR (gpr);
  // add this to our price history
  if (m_priceCount == 0) signalProgress (0, 1, QObject::tr("Loading prices..."));
  if (gpr->commodity()->isCurrency()) {
    MyMoneyPrice exchangeRate (gpr->commodity()->id().utf8(), gpr->currency()->id().utf8(),
                               gpr->priceDate(), MyMoneyMoney(gpr->value()), QObject::tr("Imported History"));
    m_storage->addPrice (exchangeRate);
  } else {
    MyMoneySecurity e = m_storage->security(m_mapEquities[gpr->commodity()->id().utf8()]);
    if (gncdebug) qDebug ("Searching map, key = %s, found id = %s",
                            gpr->commodity()->id().latin1(), e.id().data());
    e.setTradingCurrency (gpr->currency()->id().utf8());
    MyMoneyMoney priceValue(gpr->value());
    MyMoneyPrice stockPrice(e.id(), gpr->currency()->id().utf8(), gpr->priceDate(), priceValue, QObject::tr("Imported History"));
    m_storage->addPrice (stockPrice);
    m_storage->modifySecurity(e);
  }
  signalProgress (++m_priceCount, 0);
  return ;
}

//*********************************convertAccount ****************************************
void MyMoneyGncReader::convertAccount (const GncAccount* gac) {
  Q_CHECK_PTR (gac);
  TRY

  MyMoneyAccount acc;
  if (m_accountCount == 0) signalProgress (0, gac->gncAccountCount(), QObject::tr("Loading accounts..."));
  acc.setName(gac->name());

  acc.setDescription(gac->desc());

  QDate currentDate = QDate::currentDate();
  acc.setOpeningDate(currentDate);
  acc.setLastModified(currentDate);
  acc.setLastReconciliationDate(currentDate);
  if (gac->commodity()->isCurrency()) {
    acc.setCurrencyId (gac->commodity()->id().utf8());
    m_currencyCount[gac->commodity()->id()]++;
  }

  acc.setParentAccountId (gac->parent().utf8());
  // now determine the account type and its parent id
  if (QString("BANK") == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Checkings);
  } else if (QString("ASSET") == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Asset);
  } else if (QString("CASH") == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Cash);
  } else if (QString("STOCK") == gac->type() || QString("MUTUAL") == gac->type() ) {
    // gnucash allows a 'broker' account to be denominated as type STOCK, but with
    // a currency balance. We do not need to create a stock account for this
    // actually, the latest version of gnc (1.8.8) doesn't seem to allow you to do
    // this any more, though I do have one in my own account...
    if (gac->commodity()->isCurrency()) {
      acc.setAccountType(MyMoneyAccount::Investment);
    } else {
      acc.setAccountType(MyMoneyAccount::Stock);
    }
  } else if (QString("EQUITY") == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Equity);
  } else if (QString("LIABILITY") == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Liability);
  } else if (QString("CREDIT") == gac->type()) {
    acc.setAccountType(MyMoneyAccount::CreditCard);
  } else if (QString("INCOME") == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Income);
  } else if (QString("EXPENSE") == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Expense);
  } else if (QString("RECEIVABLE") == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Asset);
  } else if (QString("PAYABLE") == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Liability);
  } else { // we have here an account type we can't currently handle
    QString em =
      (QObject::tr(QString().sprintf("Current importer does not recognize GnuCash account type %s", gac->type().latin1())));
    throw new MYMONEYEXCEPTION (em);
  }
  // if no parent account is present, assign to one of our standard accounts
  if (acc.parentAccountId().isEmpty()) {
    switch (acc.accountGroup()) {
    case MyMoneyAccount::Asset: acc.setParentAccountId (m_storage->asset().id()); break;
    case MyMoneyAccount::Liability: acc.setParentAccountId (m_storage->liability().id()); break;
    case MyMoneyAccount::Income: acc.setParentAccountId (m_storage->income().id()); break;
    case MyMoneyAccount::Expense: acc.setParentAccountId (m_storage->expense().id()); break;
    case MyMoneyAccount::Equity: acc.setParentAccountId (m_storage->equity().id()); break;
    default: break; // not necessary but avoids compiler warnings
    }
  }

  // extra processing for a stock account
  if (acc.accountType() == MyMoneyAccount::Stock) {
    // save the id for later linking to investment account
    m_stockList.append (gac->id());
    // set the equity type
    MyMoneySecurity e = m_storage->security (m_mapEquities[gac->commodity()->id().utf8()]);
    if (gncdebug) qDebug ("Acct equity search, key = %s, found id = %s",
                            gac->commodity()->id().latin1(), e.id().data());
    acc.setCurrencyId (e.id()); // actually, the security id
    if (QString("MUTUAL") == gac->type()) {
      e.setSecurityType (MyMoneySecurity::SECURITY_MUTUALFUND);
      if (gncdebug) qDebug ("Setting %s to mutual", e.name().latin1());
      m_storage->modifySecurity (e);
    }
  }
  // all the details from the file about the account should be known by now.
  // calling addAccount will automatically fill in the account ID.
  m_storage->addAccount(acc);
  m_mapIds[gac->id().utf8()] = acc.id(); // to link gnucash id to ours for tx posting

  if (gncdebug) qDebug("Gnucash account %s has id of %s, type of %s, parent is %s",
                         gac->id().latin1(), acc.id().data(),
                         KMyMoneyUtils::accountTypeToString(acc.accountType()).latin1(), acc.parentAccountId().data());
  signalProgress (++m_accountCount, 0);
  return ;
  PASS
}

//********************************************** convertTransaction *****************************
void MyMoneyGncReader::convertTransaction (const GncTransaction *gtx) {
  Q_CHECK_PTR (gtx);
  MyMoneyTransaction tx;
  MyMoneySplit split;
  unsigned int i;

  if (m_transactionCount == 0) signalProgress (0, gtx->gncTransactionCount(), QObject::tr("Loading transactions..."));
  // initialize class variables related to transactions
  m_txCommodity = "";
  m_txPayeeId = m_defaultPayee;
  m_potentialTransfer = true;
  m_splitList.clear(); m_liabilitySplitList.clear(); m_otherSplitList.clear();
  // payee, dates, commodity
  if (!gtx->desc().isEmpty()) m_txPayeeId = createPayee (gtx->desc());
  tx.setEntryDate (gtx->dateEntered());
  tx.setPostDate (gtx->datePosted());
  m_txDatePosted = tx.postDate(); // save for use in splits
  tx.setCommodity (gtx->currency().utf8());
  m_txCommodity = tx.commodity(); // save in storage, maybe needed for Orphan accounts
  // process splits
  for (i = 0; i < gtx->splitCount(); i++) {
    convertSplit (static_cast<const GncSplit *>(gtx->getSplit (i)));
  }
  m_splitList += m_liabilitySplitList += m_otherSplitList;
  // the splits are in order in splitList. Link them to the tx. also, determine the
  // action type, and fill in some fields which gnc holds at transaction level
  // first off, is it a transfer (can only have 2 splits?)
  if (m_splitList.count() != 2) m_potentialTransfer = false;
  // at this point, if m_potentialTransfer is still true, it is actually one!
  QString txMemo = "";
  QValueList<MyMoneySplit>::iterator it = m_splitList.begin();
  while (!m_splitList.isEmpty()) {
    split = *it;
    if (m_potentialTransfer) split.setAction(MyMoneySplit::ActionTransfer);
    split.setNumber(gtx->no());
    // Arbitrarily, save the first non-null split memo as the memo for the whole tx
    // I think this is necessary because txs with just 2 splits (the majority)
    // are not viewable as split transactions in kmm so the split memo is not seen
    if ((txMemo.isEmpty()) && (!split.memo().isEmpty())) txMemo = split.memo();
    tx.addSplit(split);
    it = m_splitList.remove(it);
  }
  // memo - set from split
  tx.setMemo(txMemo);
  m_storage->addTransaction(tx, true); // all done, add the transaction to storage
  signalProgress (++m_transactionCount, 0);
  return ;
}
//******************************************convertSplit********************************
void MyMoneyGncReader::convertSplit (const GncSplit *gsp) {
  Q_CHECK_PTR (gsp);
  MyMoneySplit split;
  MyMoneyAccount splitAccount;
  // find the kmm account id coresponding to the gnc id
  QCString kmmAccountId;
  map_accountIds::Iterator id = m_mapIds.find(gsp->acct().utf8());
  if (id != m_mapIds.end()) {
    kmmAccountId = id.data();
  } else { // for the case where the acs not found (which shouldn't happen?), create an account with gnc name
    kmmAccountId = createOrphanAccount (gsp->acct());
  }
  // find the account pointer and save for later
  splitAccount = m_storage->account (kmmAccountId);
  // print some data so we can maybe identify this split later
  // TODO : prints personal data
  //if (gncdebug) qDebug ("Split data - gncid %s, kmmid %s, memo %s, value %s, recon state %s",
  //                        gsp->acct().latin1(), kmmAccountId.data(), gsp->memo().latin1(), gsp->value().latin1(),
  //                        gsp->recon().latin1());
  // payee id
  split.setPayeeId (m_txPayeeId.utf8());
  // reconciled state and date
  switch (gsp->recon().at(0).latin1()) {
  case 'n':
    split.setReconcileFlag(MyMoneySplit::NotReconciled); break;
  case 'c':
    split.setReconcileFlag(MyMoneySplit::Cleared); break;
  case 'y':
    split.setReconcileFlag(MyMoneySplit::Reconciled); break;
  }
  split.setReconcileDate(gsp->reconDate());
  // memo
  split.setMemo(gsp->memo());
  // accountId
  split.setAccountId (kmmAccountId);
  // value and quantity
  MyMoneyMoney splitValue (0);
  if (gsp->value() != QString("-1/0")) { // treat gnc invalid value as zero
   splitValue = gsp->value();
 } else {
   // it's not quite a consistency check, but easier to treat it as such
   postMessage ("CC", 4, splitAccount.name().latin1(), m_txDatePosted.toString(Qt::ISODate).latin1());
 }
  MyMoneyMoney splitQuantity(gsp->qty());
  split.setValue (splitValue);
  split.setShares (splitQuantity);

  // in kmm, the first split is important. in this routine we will
  // save the splits in our split list with the priority:
  // 1. assets
  // 2. liabilities
  // 3. others (categories)
  // but keeping each in same order as gnucash
  MyMoneySecurity e;
  MyMoneyMoney price, newPrice;

  switch (splitAccount.accountGroup()) {
  case MyMoneyAccount::Asset:
    if (splitAccount.accountType() == MyMoneyAccount::Stock) {
      split.value() == MyMoneyMoney(0) ?
      split.setAction (MyMoneySplit::ActionAddShares) :     // free shares?
      split.setAction (MyMoneySplit::ActionBuyShares);
      m_potentialTransfer = false; // ?
      // add a price history entry
      e = m_storage->security(splitAccount.currencyId());
      // newPrice fix supplied by Phil Longstaff
      price = split.value() / split.shares();
#define NEW_DENOM 10000
      newPrice = MyMoneyMoney ( price.toDouble(), (signed64)NEW_DENOM );
      if (!newPrice.isZero()) {
        e.setTradingCurrency (m_txCommodity);
        if (gncdebug) qDebug ("added price for %s, %s date %s",
                                e.name().latin1(), price.toString().latin1(), m_txDatePosted.toString(Qt::ISODate).latin1());
        m_storage->modifySecurity(e);
        MyMoneyPrice dealPrice (e.id(), m_txCommodity, m_txDatePosted, newPrice, QObject::tr("Imported Transaction"));
        m_storage->addPrice (dealPrice);
      }
    } else { // not stock
      if (split.value().isNegative()) {
        split.setAction (MyMoneySplit::ActionDeposit);
      } else {
        bool isNumeric = false;
        if (!split.number().isEmpty()) {
          split.number().toLong(&isNumeric);    // No QString.isNumeric()??
        }
        if (isNumeric) {
          split.setAction (MyMoneySplit::ActionCheck);
        } else {
          split.setAction (MyMoneySplit::ActionWithdrawal);
        }
      }
    }
    m_splitList.append(split);
    break;
  case MyMoneyAccount::Liability:
    split.value().isNegative() ?
    split.setAction (MyMoneySplit::ActionWithdrawal) :
    split.setAction (MyMoneySplit::ActionDeposit);
    m_liabilitySplitList.append(split);
    break;
  default:
    m_potentialTransfer = false;
    m_otherSplitList.append (split);
  }
  // backdate the account opening date if necessary
  if (m_txDatePosted < splitAccount.openingDate()) {
    splitAccount.setOpeningDate(m_txDatePosted);
    m_storage->modifyAccount(splitAccount);
  }
  return ;
}
//********************************* convertTemplateTransaction **********************************************
MyMoneyTransaction MyMoneyGncReader::convertTemplateTransaction (const QString schedName, const GncTransaction *gtx) {

  Q_CHECK_PTR (gtx);
  MyMoneyTransaction tx;
  MyMoneySplit split;
  unsigned int i;
  if (m_templateCount == 0) signalProgress (0, 1, QObject::tr("Loading templates..."));

  // initialize class variables related to transactions
  m_txCommodity = "";
  m_txPayeeId = m_defaultPayee;
  m_potentialTransfer = true;
  m_splitList.clear(); m_liabilitySplitList.clear(); m_otherSplitList.clear();

  // payee, dates, commodity
  if (!gtx->desc().isEmpty()) m_txPayeeId = createPayee (gtx->desc());
  tx.setEntryDate(gtx->dateEntered());
  tx.setPostDate(gtx->datePosted());
  m_txDatePosted = tx.postDate();
  tx.setCommodity (gtx->currency().utf8());
  m_txCommodity = tx.commodity(); // save for possible use in orphan account
  // process splits
  for (i = 0; i < gtx->splitCount(); i++) {
    convertTemplateSplit (schedName, static_cast<const GncTemplateSplit *>(gtx->getSplit (i)));
  }
  // determine the action type for the splits and link them to the template tx
  QCString negativeActionType, positiveActionType;
  if (!m_splitList.isEmpty()) { // if there are asset splits
    positiveActionType = MyMoneySplit::ActionDeposit;
    negativeActionType = MyMoneySplit::ActionWithdrawal;
  } else { // if there are liability splits
    positiveActionType = MyMoneySplit::ActionWithdrawal;
    negativeActionType = MyMoneySplit::ActionDeposit;
  }
  if (!m_otherSplitList.isEmpty()) m_potentialTransfer = false; // tfrs can occur only between assets and asset/liabilities
  m_splitList += m_liabilitySplitList += m_otherSplitList;
  // the splits are in order in splitList. Transfer them to the tx
  // also, determine the action type. first off, is it a transfer (can only have 2 splits?)
  if (m_splitList.count() != 2) m_potentialTransfer = false;
  // at this point, if m_potentialTransfer is still true, it is actually one!
  QString txMemo = "";
  QValueList<MyMoneySplit>::iterator it = m_splitList.begin();
  while (!m_splitList.isEmpty()) {
    split = *it;
    if (m_potentialTransfer) {
      split.setAction(MyMoneySplit::ActionTransfer);
    } else {
      if (split.value() <= MyMoneyMoney (0)) {
        split.setAction (negativeActionType);
      } else {
        split.setAction (positiveActionType);
      }
    }
    split.setNumber(gtx->no()); // set cheque no (or equivalent description)
    // Arbitrarily, save the first non-null split memo as the memo for the whole tx
    // I think this is necessary because txs with just 2 splits (the majority)
    // are not viewable as split transactions in kmm so the split memo is not seen
    if ((txMemo.isEmpty()) && (!split.memo().isEmpty())) txMemo = split.memo();
    tx.addSplit(split);
    it = m_splitList.remove(it);
  }
  // memo - set from split
  tx.setMemo (txMemo);
  signalProgress (++m_templateCount, 0);
  return (tx);
}
//********************************* convertTemplateSplit ****************************************************
void MyMoneyGncReader::convertTemplateSplit (const QString schedName, const GncTemplateSplit *gsp) {
  Q_CHECK_PTR (gsp);
  // convertTemplateSplit
  MyMoneySplit split;
  MyMoneyAccount splitAccount;
  unsigned int i, j;
  bool nonNumericFormula = false;

  // action, value and account will be set from slots
  // reconcile state, always Not since it hasn't even been posted yet (?)
  split.setReconcileFlag(MyMoneySplit::NotReconciled);
  // memo
  split.setMemo(gsp->memo());
  // payee id
  split.setPayeeId (m_txPayeeId.utf8());
  // read split slots (KVPs)
  int xactionCount = 0;
  int validSlotCount = 0;
  QString gncAccountId;
  for (i = 0; i < gsp->kvpCount(); i++ ) {
    const GncKvp *slot = gsp->getKvp(i);
    if ((slot->key() == "sched-xaction") && (slot->type() == "frame")) {
      bool bFoundStringCreditFormula = false;
      bool bFoundStringDebitFormula = false;
      bool bFoundGuidAccountId = false;
      QString gncCreditFormula, gncDebitFormula;
      for (j = 0; j < slot->kvpCount(); j++) {
        const GncKvp *subSlot = slot->getKvp (j);
        // again, see comments above. when we have a full specification
        // of all the options available to us, we can no doubt improve on this
        if ((subSlot->key() == "credit-formula") && (subSlot->type() == "string")) {
          gncCreditFormula = subSlot->value();
          bFoundStringCreditFormula = true;
        }
        if ((subSlot->key() == "debit-formula") && (subSlot->type() == "string")) {
          gncDebitFormula = subSlot->value();
          bFoundStringDebitFormula = true;
        }
        if ((subSlot->key() == "account") && (subSlot->type() == "guid")) {
          gncAccountId = subSlot->value();
          bFoundGuidAccountId = true;
        }
      }
      // all data read, now check we have everything
      if ((bFoundStringCreditFormula) && (bFoundStringDebitFormula) && (bFoundGuidAccountId)) {
        if (gncdebug) qDebug ("Found valid slot; credit %s, debit %s, acct %s",
                                gncCreditFormula.latin1(), gncDebitFormula.latin1(), gncAccountId.latin1());
        validSlotCount++;
      }
      // validate numeric, work out sign
      MyMoneyMoney exFormula;
      exFormula.setThousandSeparator (','); // gnucash always uses these in internal file format?
      exFormula.setDecimalSeparator ('.');
      QString numericTest;
      if (!gncCreditFormula.isEmpty()) {
        exFormula = "-" + gncCreditFormula;
        numericTest = gncCreditFormula;
      } else if (!gncDebitFormula.isEmpty()) {
        exFormula = gncDebitFormula;
        numericTest = gncDebitFormula;
      }
      if (exFormula.isZero()) { // if there was a real formula there it would be converted to zero
        bool isNumeric;         // but just the fact that it's zero doesn't make it bad...
        double temp;
        temp = numericTest.toDouble (&isNumeric); // this seems to be the only way to test for valid numeric
        if (!isNumeric) {
          qDebug ("%s is not numeric", numericTest.latin1());
          nonNumericFormula = true;
        }
      }
      split.setValue (exFormula);
      xactionCount++;
    } else {
      postMessage ("SC", 3, schedName.latin1(), slot->key().latin1(), slot->type().latin1());
      m_suspectSchedule = true;
    }
  }
  // report this as untranslatable tx
  if (xactionCount > 1) {
    postMessage ("SC", 4, schedName.latin1());
    m_suspectSchedule = true;
  }
  if (validSlotCount == 0) {
    postMessage ("SC", 5, schedName.latin1());
    m_suspectSchedule = true;
  }
  if (nonNumericFormula) {
    postMessage ("SC", 6, schedName.latin1());
    m_suspectSchedule = true;
  }
  // find the kmm account id coresponding to the gnc id
  QCString kmmAccountId;
  map_accountIds::Iterator id = m_mapIds.find(gncAccountId.utf8());
  if (id != m_mapIds.end()) {
    kmmAccountId = id.data();
  } else { // for the case where the acs not found (which shouldn't happen?), create an account with gnc name
    kmmAccountId = createOrphanAccount (gncAccountId);
  }
  splitAccount = m_storage->account (kmmAccountId);
  split.setAccountId (kmmAccountId);
  // add the split to one of the lists
  switch (splitAccount.accountGroup()) {
  case MyMoneyAccount::Asset:
    m_splitList.append (split); break;
  case MyMoneyAccount::Liability:
    m_liabilitySplitList.append (split); break;
  default:
    m_otherSplitList.append (split);
  }
  // backdate the account opening date if necessary
  if (m_txDatePosted < splitAccount.openingDate()) {
    splitAccount.setOpeningDate(m_txDatePosted);
    m_storage->modifyAccount(splitAccount);
  }
  return ;
}
//********************************* convertSchedule  ********************************************************
void MyMoneyGncReader::convertSchedule (const GncSchedule *gsc) {
  TRY
  Q_CHECK_PTR (gsc);
  MyMoneySchedule sc;
  MyMoneyTransaction tx;
  m_suspectSchedule = false;
  QDate startDate, nextDate, lastDate, endDate;  // for date calculations
  QDate today = QDate::currentDate();
  int numOccurs, remOccurs;

  if (m_scheduleCount == 0) signalProgress (0, gsc->gncScheduleCount(), QObject::tr("Loading schedules..."));
  // schedule name
  sc.setName(gsc->name());
  // find the transaction template as stored earlier
  QPtrListIterator<GncTransaction> itt (m_templateList);
  GncTransaction *ttx;
  while ((ttx = itt.current()) != 0) {
    // the id to match against is the split:account value in the splits
    if (static_cast<const GncTemplateSplit *>(ttx->getSplit(0))->acct() == gsc->templId()) break;
    ++itt;
  }
  if (itt == 0) {
    throw new MYMONEYEXCEPTION (QObject::tr(QString("Can't find template transaction for schedule " + sc.name())));
  } else {
    tx = convertTemplateTransaction (sc.name(), *itt);
  }
  tx.setId("");
  sc.setTransaction(tx);
  // define the conversion table for intervals
  struct convIntvl {
    QString gncType; // the gnucash name
    unsigned char interval; // for date calculation
    unsigned int intervalCount;
    MyMoneySchedule::occurenceE occ; // equivalent occurence code
    MyMoneySchedule::weekendOptionE wo;
  };
  static convIntvl vi [] = {
                             {"daily" , 'd', 1, MyMoneySchedule::OCCUR_DAILY, MyMoneySchedule::MoveNothing },
                             //{"daily_mf", 'd', 1, MyMoneySchedule::OCCUR_DAILY, MyMoneySchedule::MoveMonday }, doesn't work, need new freq in kmm
                             {"weekly", 'w', 1, MyMoneySchedule::OCCUR_WEEKLY, MyMoneySchedule::MoveNothing },
                             {"bi_weekly", 'w', 2, MyMoneySchedule::OCCUR_EVERYOTHERWEEK, MyMoneySchedule::MoveNothing },
                             {"monthly", 'm', 1, MyMoneySchedule::OCCUR_MONTHLY, MyMoneySchedule::MoveNothing },
                             {"quarterly", 'm', 3, MyMoneySchedule::OCCUR_QUARTERLY, MyMoneySchedule::MoveNothing },
                             {"tri_annually", 'm', 4, MyMoneySchedule::OCCUR_EVERYFOURMONTHS, MyMoneySchedule::MoveNothing },
                             {"semi_yearly", 'm', 6, MyMoneySchedule::OCCUR_TWICEYEARLY, MyMoneySchedule::MoveNothing },
                             {"yearly", 'y', 1, MyMoneySchedule::OCCUR_YEARLY, MyMoneySchedule::MoveNothing },
                             {"zzz", 'y', 1, MyMoneySchedule::OCCUR_YEARLY, MyMoneySchedule::MoveNothing}
                              // zzz = stopper, may cause problems. what else can we do?
                           };
  // find this interval
  const GncFreqSpec *fs = gsc->getFreqSpec();
  int i;
  for (i = 0; vi[i].gncType != "zzz"; i++) {
    if (fs->intervalType() == vi[i].gncType) break;
  }
  if (vi[i].gncType == "zzz") {
    postMessage ("SC", 1, sc.name().latin1(), fs->intervalType().latin1());
    m_suspectSchedule = true;
  }
  if (!fs->m_fsList.isEmpty()) {
    postMessage ("SC", 7, sc.name().latin1());
    m_suspectSchedule = true;
  }
  // set the occurrence interval, weekend option, start date
  sc.setOccurence (vi[i].occ);
  sc.setWeekendOption (vi[i].wo);
  sc.setStartDate (gsc->startDate());
  // if a last date was specified, use it, otherwise try to work out the last date
  sc.setLastPayment(gsc->lastDate());
  numOccurs = gsc->numOccurs().toInt();
  if (sc.lastPayment() == QDate()) {
    nextDate = gsc->startDate();
    while (nextDate < today) {
      lastDate = nextDate;
      nextDate = incrDate (lastDate, vi[i].interval, vi[i].intervalCount);
    }
    sc.setLastPayment(lastDate);
  }
  // if an end date was specified, use it, otherwise if the input file had a number
  // of occurs remaining, work out the end date
  sc.setEndDate(gsc->endDate());
  remOccurs = gsc->remOccurs().toInt();
  if ((sc.endDate() == QDate()) && (remOccurs > 0)) {
    endDate = sc.lastPayment();
    while (remOccurs-- > 0) {
      endDate = incrDate (endDate, vi[i].interval, vi[i].intervalCount);
    }
    sc.setEndDate(endDate);
  }
  // payment type, options
  sc.setPaymentType((MyMoneySchedule::paymentTypeE)MyMoneySchedule::STYPE_OTHER);
  sc.setFixed (!m_suspectSchedule); // if any probs were found, set it as variable so user will always be prompted
  sc.setAutoEnter (gsc->autoCreate() == QString ("y"));
  // type
  QCString actionType = tx.splits().first().action();
  if (actionType == MyMoneySplit::ActionDeposit) {
    sc.setType((MyMoneySchedule::typeE)MyMoneySchedule::TYPE_DEPOSIT);
  } else if (actionType == MyMoneySplit::ActionTransfer) {
    sc.setType((MyMoneySchedule::typeE)MyMoneySchedule::TYPE_TRANSFER);
  } else {
    sc.setType((MyMoneySchedule::typeE)MyMoneySchedule::TYPE_BILL);
  }
  //tell the storage objects we have a new schedule object.
  if (m_suspectSchedule && m_dropSuspectSchedules) {
    postMessage ("SC", 2, sc.name().latin1());
  } else {
    m_storage->addSchedule(sc);
  }
  signalProgress (++m_scheduleCount, 0);
  return ;
  PASS
}
//********************************* convertFreqSpec  ********************************************************
void MyMoneyGncReader::convertFreqSpec (const GncFreqSpec *) {
  // Nowt to do here at the moment, convertSched only retrieves the interval type
  // but we will probably need to look into the nested freqspec when we properly implement semi-monthly and stuff
  return ;
}
//**********************************************************************************************************
//************************************* terminate **********************************************************
void MyMoneyGncReader::terminate () {
  TRY
  // All data has been converted and added to storage
  // this code is just temporary to show us what is in the file.
  if (gncdebug) qDebug("%d accounts found in the GNU Cash file", m_mapIds.count());
  for (map_accountIds::Iterator it = m_mapIds.begin(); it != m_mapIds.end(); ++it) {
    if (gncdebug) qDebug("key = %s, value = %s", it.key().data(), it.data().data());
  }
  // first step is to implement the users investment option, now we
  // have all the accounts available
  QValueList<QString>::iterator stocks;
  for (stocks = m_stockList.begin(); stocks != m_stockList.end(); ++stocks) {
    checkInvestmentOption (*stocks);
  }
  // Next step is to walk the list and assign the parent/child relationship between the objects.
  unsigned int i = 0;
  signalProgress (0, m_accountCount, QObject::tr ("Reorganizing accounts..."));
  QValueList<MyMoneyAccount> list;
  QValueList<MyMoneyAccount>::Iterator acc;
  list = m_storage->accountList();
  for (acc = list.begin(); acc != list.end(); ++acc) {
    if ((*acc).parentAccountId() == m_storage->asset().id()) {
      MyMoneyAccount assets = m_storage->asset();
      m_storage->addAccount(assets, (*acc));
      if (gncdebug) qDebug("Account id %s is a child of the main asset account", (*acc).id().data());
    } else if ((*acc).parentAccountId() == m_storage->liability().id()) {
      MyMoneyAccount liabilities = m_storage->liability();
      m_storage->addAccount(liabilities, (*acc));
      if (gncdebug) qDebug("Account id %s is a child of the main liability account", (*acc).id().data());
    } else if ((*acc).parentAccountId() == m_storage->income().id()) {
      MyMoneyAccount incomes = m_storage->income();
      m_storage->addAccount(incomes, (*acc));
      if (gncdebug) qDebug("Account id %s is a child of the main income account", (*acc).id().data());
    } else if ((*acc).parentAccountId() == m_storage->expense().id()) {
      MyMoneyAccount expenses = m_storage->expense();
      m_storage->addAccount(expenses, (*acc));
      if (gncdebug) qDebug("Account id %s is a child of the main expense account", (*acc).id().data());
    } else if ((*acc).parentAccountId() == m_storage->equity().id()) {
      MyMoneyAccount equity = m_storage->equity();
      m_storage->addAccount(equity, (*acc));
      if (gncdebug) qDebug("Account id %s is a child of the main equity account", (*acc).id().data());
    } else {
      // it is not under one of the main accounts, so find gnucash parent
      QCString parentKey = (*acc).parentAccountId();
      if (gncdebug) qDebug ("acc %s, parent %s", (*acc).id().data(),
                              (*acc).parentAccountId().data());
      map_accountIds::Iterator id = m_mapIds.find(parentKey);
      if (id != m_mapIds.end()) {
        if (gncdebug) qDebug("Setting account id %s's parent account id to %s",
                               (*acc).id().data(), id.data().data());
        MyMoneyAccount parent = m_storage->account(id.data());
        parent = checkConsistency (parent, (*acc));
        m_storage->addAccount (parent, (*acc));
      } else {
        throw new MYMONEYEXCEPTION ("terminate() could not find account id");
      }
    }
    signalProgress (++i, 0);
  } // end for account
  // offer the most common account currency as a default
  QString mainCurrency = "";
  unsigned int maxCount = 0;
  QMap<QString, unsigned int>::ConstIterator it;
  for (it = m_currencyCount.begin(); it != m_currencyCount.end(); ++it) {
    if (it.data() > maxCount) {
      maxCount = it.data();
      mainCurrency = it.key();
      }
  }
  if (mainCurrency != "") {
    switch (QMessageBox::question (0, PACKAGE,
        QObject::tr("Your main currency seems to be %1 (%2); do you want to set this as your base currency?")
            .arg(mainCurrency).arg(m_storage->currency(mainCurrency.utf8()).name()),
                    QMessageBox::Yes | QMessageBox::Default, QMessageBox::No)) {
        case QMessageBox::Yes:
          m_storage->setValue ("kmm-baseCurrency", mainCurrency);
    }
  }
  // now produce the end of job reports - first, work out which ones are required
  m_ccCount = 0, m_orCount = 0, m_scCount = 0;
  for (i = 0; i < m_messageList.count(); i++) {
    if ((*m_messageList.at(i)).source == "CC") m_ccCount++;
    if ((*m_messageList.at(i)).source == "OR") m_orCount++;
    if ((*m_messageList.at(i)).source == "SC") m_scCount++;
  }
  QValueList<QString> sectionsToReport; // list of sections needing report
  sectionsToReport.append ("MN"); // always build the main section
  if (m_ccCount > 0) sectionsToReport.append ("CC");
  if (m_orCount > 0) sectionsToReport.append ("OR");
  if (m_scCount > 0) sectionsToReport.append ("SC");
  // produce the sections in message boxes
  bool exit = false;
  for (i = 0; (i < sectionsToReport.count()) && !exit; i++) {
    QString button0Text = "More";
    if (i + 1 == sectionsToReport.count()) button0Text = "Done"; // last section
    switch (QMessageBox::information (0, PACKAGE,
                                      buildReportSection (*sectionsToReport.at(i)),
                                      button0Text, "Save Report", "Cancel",
                                      0, 2))    // Enter == button 0, Escape == button 2
    {
    case 0:
      break; // more
    case 1:
      exit = writeReportToFile (sectionsToReport);
      break;
    case 2:   // Cancel clicked or Escape pressed
      exit = true;
      break;
    }
  }
  PASS
}
//************************************ buildReportSection************************************
const QString MyMoneyGncReader::buildReportSection (const QString source) {
  TRY
  QString s = "";
  bool more = false;
  if (source == "MN") {
    s.append (QObject::tr("Found:\n\n"));
    s.append (QString::number(m_commodityCount) + QObject::tr(" commodities (equities)\n"));
    s.append (QString::number(m_priceCount) + QObject::tr(" prices\n"));
    s.append (QString::number(m_accountCount) + QObject::tr(" accounts\n"));
    s.append (QString::number(m_transactionCount) + QObject::tr(" transactions\n"));
    s.append (QString::number(m_scheduleCount) + QObject::tr(" schedules\n"));
    s.append ("\n\n");
    if (m_ccCount == 0) {
      s.append (QObject::tr("No inconsistencies were detected"));
    } else {
      s.append (QString::number(m_ccCount) + QObject::tr(" inconsistencies were detected and corrected\n"));
      more = true;
    }
    if (m_orCount > 0) {
      s.append ("\n\n");
      s.append (QString::number(m_orCount) + QObject::tr(" orphan accounts were created\n"));
      more = true;
    }
    if (m_scCount > 0) {
      s.append ("\n\n");
      s.append (QString::number(m_scCount) + QObject::tr(" possible schedule problems were noted\n"));
      more = true;
    }
    if (more) s.append (QObject::tr("\n\nPress More for further information"));
  } else { // we need to retrieve the posted messages for this source
    unsigned int i, j;
    for (i = 0; i < m_messageList.count(); i++) {
      GncMessageArgs *m = m_messageList.at(i);
      if (m->source == source) {
        QString ss = GncMessages::text (m->source, m->code);
        // add variable args. the .arg function seems always to replace the
        // lowest numbered placeholder it finds, so translating messages
        // with variables in a different order should still work okay (I think...)
        for (j = 0; j < m->args.count(); j++) ss = ss.arg (*m->args.at(j));
        s.append (ss + "\n");
      }
    }
  }
  if (gncdebug) qDebug ("%s", s.latin1());
  return (static_cast<const QString>(s));
  PASS
}
//************************ writeReportToFile*********************************
bool MyMoneyGncReader::writeReportToFile (const QValueList<QString> sectionsToReport) {
  TRY
  unsigned int i;
  QFileDialog* fd = new QFileDialog (0, "Save report as", TRUE);
  fd->setMode (QFileDialog::AnyFile);
  if (fd->exec() != QDialog::Accepted) return (false);
  QFile reportFile(fd->selectedFile());
  QFileInfo fi (reportFile);
  if (!reportFile.open (IO_WriteOnly)) return (false);
  QTextStream stream (&reportFile);
  for (i = 0; i < sectionsToReport.count(); i++) {
    stream << buildReportSection (*sectionsToReport.at(i)).latin1() << endl;
  }
  reportFile.close();
  return (true);
  PASS
}
/****************************************************************************
                    Utility routines
*****************************************************************************/ 
//************************ createPayee ***************************

const QString MyMoneyGncReader::createPayee (const QString gncDescription) {
  MyMoneyPayee payee;
  try {
    payee = m_storage->payeeByName (gncDescription);
  } catch (MyMoneyException *e) { // payee not found, create one
    delete e;
    payee.setName (gncDescription);
    m_storage->addPayee (payee);
  }
  return (payee.id());
}
//************************************** createOrphanAccount *******************************
const QCString MyMoneyGncReader::createOrphanAccount (const QString gncName) {
  MyMoneyAccount acc;

  acc.setName ("orphan_" + gncName);
  acc.setDescription (QObject::tr("Orphan created from unknown gnucash account"));

  QDate today = QDate::currentDate();

  acc.setOpeningDate (today);
  acc.setLastModified (today);
  acc.setLastReconciliationDate (today);
  acc.setCurrencyId (m_txCommodity);
  acc.setAccountType (MyMoneyAccount::Asset);
  acc.setParentAccountId (m_storage->asset().id());
  m_storage->addAccount (acc);
  // assign the gnucash id as the key into the map to find our id
  m_mapIds[gncName.utf8()] = acc.id();
  postMessage ("OR", 1, acc.name().data());
  return (acc.id());
}
//****************************** incrDate *********************************************
QDate MyMoneyGncReader::incrDate (QDate lastDate, unsigned char interval, unsigned int intervalCount) {
  TRY
  switch (interval) {
  case 'd':
    return (lastDate.addDays(intervalCount));
  case 'w':
    return (lastDate.addDays(intervalCount * 7));
  case 'm':
    return (lastDate.addMonths(intervalCount));
  case 'y':
    return (lastDate.addYears(intervalCount));
  }
  throw new MYMONEYEXCEPTION (QObject::tr("Internal error - invalid interval char in incrDate"));
  QDate r = QDate(); return (r); // to keep compiler happy
  PASS
}
//********************************* checkConsistency **********************************
MyMoneyAccount MyMoneyGncReader::checkConsistency (MyMoneyAccount& parent, MyMoneyAccount& child) {
  TRY
  // gnucash is flexible/weird enough to allow various inconsistencies
  // these are a couple I found in my file, no doubt more will be discovered
  if ((child.accountType() == MyMoneyAccount::Investment) &&
      (parent.accountType() != MyMoneyAccount::Asset)) {
    postMessage ("CC", 1, child.name().latin1());
    return m_storage->asset();
  }
  if ((child.accountType() == MyMoneyAccount::Income) &&
      (parent.accountType() != MyMoneyAccount::Income)) {
    postMessage ("CC", 2, child.name().latin1());
    return m_storage->income();
  }
  if ((child.accountType() == MyMoneyAccount::Expense) &&
      (parent.accountType() != MyMoneyAccount::Expense)) {
    postMessage ("CC", 3, child.name().latin1());
    return m_storage->expense();
  }
  return (parent);
  PASS
}
//*********************************** checkInvestmentOption *************************
void MyMoneyGncReader::checkInvestmentOption (QString stockId) {
  // implement the investment option for stock accounts
  // first check whether the parent account (gnucash id) is actually an
  // investment account. if it is, no further action is needed
  MyMoneyAccount stockAcc = m_storage->account (m_mapIds[stockId.utf8()]);
  MyMoneyAccount parent;
  QCString parentKey = stockAcc.parentAccountId();
  map_accountIds::Iterator id = m_mapIds.find (parentKey);
  if (id != m_mapIds.end()) {
    parent = m_storage->account (id.data());
    if (parent.accountType() == MyMoneyAccount::Investment) return ;
  }
  // so now, check the investment option requested by the user
  // option 0 creates a separate investment account for each stock account
  if (m_investmentOption == 0) {
    MyMoneyAccount invAcc (stockAcc);
    invAcc.setAccountType (MyMoneyAccount::Investment);
    invAcc.setCurrencyId (QCString("")); // we don't know what currency it is!!
    invAcc.setParentAccountId (parentKey); // intersperse it between old parent and child stock acct
    m_storage->addAccount (invAcc);
    m_mapIds [invAcc.id()] = invAcc.id(); // so stock account gets parented (again) to investment account later
    if (gncdebug) qDebug ("Created investment account %s as id %s, parent %s", invAcc.name().data(), invAcc.id().data(),
                            invAcc.parentAccountId().data());
    if (gncdebug) qDebug ("Setting stock %s, id %s, as child of %s", stockAcc.name().data(), stockAcc.id().data(), invAcc.id().data());
    stockAcc.setParentAccountId (invAcc.id());
    m_storage->addAccount(invAcc, stockAcc);
    // investment option 1 creates a single investment account for all stocks
  } else if (m_investmentOption == 1) {
    static QCString singleInvAccId = "";
    MyMoneyAccount singleInvAcc;
    bool ok = false;
    if (singleInvAccId.isEmpty()) { // if the account has not yet been created
      QString invAccName;
      while (!ok) {
        invAccName = QInputDialog::getText (PACKAGE,
                                            QObject::tr("Enter the investment account name "), QLineEdit::Normal,
                                            QObject::tr("My Investments"), &ok);
      }
      singleInvAcc.setName (invAccName);
      singleInvAcc.setAccountType (MyMoneyAccount::Investment);
      singleInvAcc.setCurrencyId (QCString(""));
      singleInvAcc.setParentAccountId (m_storage->asset().id());
      m_storage->addAccount (singleInvAcc);
      m_mapIds [singleInvAcc.id()] = singleInvAcc.id(); // so stock account gets parented (again) to investment account later
      if (gncdebug) qDebug ("Created investment account %s as id %s, parent %s, reparenting stock",
                              singleInvAcc.name().data(), singleInvAcc.id().data(), singleInvAcc.parentAccountId().data());
      singleInvAccId = singleInvAcc.id();
    } else { // the account has already been created
      singleInvAcc = m_storage->account (singleInvAccId);
    }
    m_storage->addAccount(singleInvAcc, stockAcc); // add stock as child
    // the original intention of option 2 was to allow any asset account to be converted to an investment (broker) account
    // however, since we have already stored the accounts as asset, we have no way at present of changing their type
    // the only alternative would be to hold all the gnucash data in memory, then implement this option, then convert all the data
    // that would mean a major overhaul of the code. Perhaps I'll think of another way...
  } else if (m_investmentOption == 2) {
    static int lastSelected = 0;
    MyMoneyAccount invAcc (stockAcc);
    QStringList accList;
    QValueList<MyMoneyAccount> list;
    QValueList<MyMoneyAccount>::Iterator acc;
    list = m_storage->accountList();
    // build a list of candidates for the input box
    for (acc = list.begin(); acc != list.end(); ++acc) {
      //      if (((*acc).accountGroup() == MyMoneyAccount::Asset) && ((*acc).accountType() != MyMoneyAccount::Stock)) accList.append ((*acc).name());
      if ((*acc).accountType() == MyMoneyAccount::Investment) accList.append ((*acc).name());
    }
    //if (accList.isEmpty()) qFatal ("No available accounts");
    bool ok = false;
    while (!ok) { // keep going till we have a valid investment parent
      QString invAccName = QInputDialog::getItem (
                             PACKAGE, QObject::tr("Select parent investment account or enter new name. Stock ") + stockAcc.name (),
                             accList, lastSelected, true, &ok);
      if (ok) {
        lastSelected = accList.findIndex (invAccName); // preserve selection for next time
        for (acc = list.begin(); acc != list.end(); ++acc) {
          if ((*acc).name() == invAccName) break;
        }
        if (acc != list.end()) { // an account was selected
          invAcc = *acc;
        } else {                 // a new account name was entered
          invAcc.setAccountType (MyMoneyAccount::Investment);
          invAcc.setName (invAccName);
          invAcc.setCurrencyId (QCString(""));
          invAcc.setParentAccountId (m_storage->asset().id());
          m_storage->addAccount (invAcc);
          ok = true;
        }
        if (invAcc.accountType() == MyMoneyAccount::Investment) {
          ok = true;
        } else {
          // this code is probably not going to be implemented coz we can't change account types (??)
          QMessageBox mb (PACKAGE,
                          invAcc.name() + QObject::tr (" is not an Investment Account. Do you wish to make it one?"),
                          QMessageBox::Question,
                          QMessageBox::Yes | QMessageBox::Default,
                          QMessageBox::No | QMessageBox::Escape,
                          QMessageBox::NoButton);
          switch (mb.exec()) {
          case QMessageBox::No :
            ok = false; break;
          default:
            // convert it - but what if it has splits???
            qFatal ("Not yet implemented");
            ok = true;
            break;
          }
        }
      } // end if ok - user pressed Cancel
    } // end while !ok
    m_mapIds [invAcc.id()] = invAcc.id(); // so stock account gets parented (again) to investment account later
    m_storage->addAccount(invAcc, stockAcc);
  } else { // investment option != 0, 1, 2
    qFatal ("Invalid investment option %d", m_investmentOption);
  }
}
// functions to control the progress bar
//*********************** setProgressCallback *****************************
void MyMoneyGncReader::setProgressCallback(void(*callback)(int, int, const QString&)) {
  m_progressCallback = callback; return ;
}
//************************** signalProgress *******************************
void MyMoneyGncReader::signalProgress(int current, int total, const QString& msg) {
  if (m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
  return ;
}
// error and information reporting
//***************************** Information and error messages *********************
void MyMoneyGncReader::postMessage (const QString source, const unsigned int code, ...) {
  TRY
  unsigned int i;
  GncMessageArgs *m = new GncMessageArgs;

  m->source = source;
  m->code = code;
  // get the number of args this message requires
  const unsigned int argCount = GncMessages::argCount (source, code);
  // incredibly, va cannot for some reason provide a count of the args it contains
  // so we may get a seg fault if not enough are provided
  // the following debug may help trace the cause
  if (gncdebug) qDebug
    ("MyMoneyGncReader::postMessage: Message %s, code %d, requires %d arguments of type char *, else may segfault",
     source.latin1(), code, argCount);
  // store the arguments
  va_list arguments;
  va_start(arguments, code); //Initializing arguments to store all values passed in after code
  for (i = 0; i < argCount; i++)
    m->args.append (QString(va_arg(arguments, char *))); //Adds the next argument to the list
  va_end(arguments);      //Cleans up the list
  m_messageList.append (m);
  return ;
  PASS
}
//********************************** Message texts **********************************************
GncMessages::messText GncMessages::texts [] = {
      {"CC", 1, QObject::tr("An Investment account must be a child of an Asset account\n"
                            "Account %1 will be stored under the main Asset account")},
      {"CC", 2, QObject::tr("An Income account must be a child of an Income account\n"
                            "Account %1 will be stored under the main Income account")},
      {"CC", 3, QObject::tr("An Expense account must be a child of an Expense account\n"
                            "Account %1 will be stored under the main Expense account")},
      {"OR", 1, QObject::tr("One or more transactions contain a reference to an otherwise unknown account\n"
                            "An asset account with the name %1 has been created to hold the data")},
      {"SC", 1, QObject::tr("Schedule %1 has interval of %2 which is not currently available")},
      {"SC", 2, QObject::tr("Schedule %1 dropped at user request")},
      {"SC", 3, QObject::tr("Schedule %1 contains unknown action (key = %2, type = %3)")},
      {"SC", 4, QObject::tr("Schedule %1 contains multiple actions; only one has been imported")},
      {"SC", 5, QObject::tr("Schedule %1 contains no valid splits")},
      {"SC", 6, QObject::tr("Schedule %1 appears to contain a formula. GnuCash formulae are not convertible")},
      {"SC", 7, QObject::tr("Schedule %1 contains a composite interval specification; please check for correct operation")},
      {"CC", 4, QObject::tr("Account or Category %1, transaction date %2; split contains invalid value; please check")},
      {"ZZ", 0, ""} // stopper
    };
//
QString GncMessages::text (const QString source, const unsigned int code) {
  TRY
  unsigned int i;
  for (i = 0; texts[i].source != "ZZ"; i++) {
    if ((source == texts[i].source) && (code == texts[i].code)) break;
  }
  if (texts[i].source == "ZZ") {
    QString mess = QString().sprintf("Internal error - unknown message - source %s, code %d", source.latin1(), code);
    throw new MYMONEYEXCEPTION (mess);
  }
  return (texts[i].text);
  PASS
}
//
const unsigned int GncMessages::argCount (const QString source, const unsigned int code) {
  TRY
  unsigned int i;
  for (i = 0; texts[i].source != "ZZ"; i++) {
    if ((source == texts[i].source) && (code == texts[i].code)) break;
  }
  if (texts[i].source == "ZZ") {
    QString mess = QString().sprintf("Internal error - unknown message - source %s, code %d", source.latin1(), code);
    throw new MYMONEYEXCEPTION (mess);
  }
  QRegExp argConst ("%\\d");
  int offset = 0;
  unsigned int argCount = 0;
  while ((offset = argConst.search (texts[i].text, offset)) != -1) {
    argCount++;
    offset += 2;
  }
  return (argCount);
  PASS
}
#endif // _GNCFILEANON