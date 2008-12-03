/***************************************************************************
                          mymoneyqifreader.cpp
                             -------------------
    begin                : Mon Jan 27 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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

#include <iostream>

// ----------------------------------------------------------------------------
// QT Headers

#include <qfile.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qtextedit.h>
#include <qregexp.h>
#include <qbuffer.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kprogress.h>
#include <kinputdialog.h>
#include <kio/netaccess.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyqifreader.h"
#include "../mymoney/mymoneyfile.h"
#include "../dialogs/kaccountselectdlg.h"
#include "../kmymoney2.h"
#include "kmymoneyglobalsettings.h"

#include "mymoneystatementreader.h"
#include <kmymoney/mymoneystatement.h>

// define this to debug the code. Using external filters
// while debugging did not work too good for me, so I added
// this code.
// #define DEBUG_IMPORT

#ifdef DEBUG_IMPORT
#warning "DEBUG_IMPORT defined --> external filter not available!!!!!!!"
#endif

class MyMoneyQifReaderPrivate {
  public:
    MyMoneyQifReaderPrivate() :
      accountType(MyMoneyAccount::Checkings)
    {}

    QString accountTypeToQif(MyMoneyAccount::accountTypeE type) const;

    /**
     * finalize the current statement and add it to the statement list
     */
    void finishStatement(void);

    bool isTransfer(QString& name, const QString& leftDelim, const QString& rightDelim);

    /**
     * Converts the QIF specific N-record of investment transactions into
     * a category name
     */
    QString typeToAccountName(const QString& type) const;

    /**
     * the statement that is currently collected/processed
     */
    MyMoneyStatement st;
    /**
     * the list of all statements to be sent to MyMoneyStatementReader
     */
    QValueList<MyMoneyStatement> statements;

    /**
     * a list of already used hashes in this file
     */
    QMap<QString, bool> m_hashMap;

    QString st_AccountName;
    QString st_AccountId;
    MyMoneyAccount::accountTypeE accountType;
    bool     firstTransaction;
    MyMoneyQifReader::QifEntryTypeE  transactionType;
};

void MyMoneyQifReaderPrivate::finishStatement(void)
{
  // in case we have collected any data in the statement, we keep it
  if((st.m_listTransactions.count() + st.m_listPrices.count() + st.m_listSecurities.count()) > 0) {
    statements += st;
    qDebug("Statement with %d transactions, %d prices and %d securities added to the statement list",
           st.m_listTransactions.count(), st.m_listPrices.count(), st.m_listSecurities.count());
  }
  // start with a fresh statement
  st = MyMoneyStatement();
  st.m_eType = (transactionType == MyMoneyQifReader::EntryTransaction) ? MyMoneyStatement::etCheckings : MyMoneyStatement::etInvestment;
}

QString MyMoneyQifReaderPrivate::accountTypeToQif(MyMoneyAccount::accountTypeE type) const
{
  QString rc = "Bank";

  switch(type) {
    default:
      break;
    case MyMoneyAccount::Cash:
      rc = "Cash";
      break;
    case MyMoneyAccount::CreditCard:
      rc = "CCard";
      break;
    case MyMoneyAccount::Asset:
      rc = "Oth A";
      break;
    case MyMoneyAccount::Liability:
      rc = "Oth L";
      break;
    case MyMoneyAccount::Investment:
      rc = "Port";
      break;
  }
  return rc;
}

QString MyMoneyQifReaderPrivate::typeToAccountName(const QString& type) const
{
  if(type == "reinvdiv")
    return i18n("Category name", "Reinvested dividend");

  if(type == "reinvlg")
    return i18n("Category name", "Reinvested dividend (long term)");

  if(type == "reinvsh")
    return i18n("Category name", "Reinvested dividend (short term)");

  if (type == "div")
    return i18n("Category name", "Dividend");

  if(type == "intinc")
    return i18n("Category name", "Interest");

  if(type == "cgshort")
    return i18n("Category name", "Capital Gain (short term)");

  if( type == "cgmid")
    return i18n("Category name", "Capital Gain (mid term)");

  if(type == "cglong")
    return i18n("Category name", "Capital Gain (long term)");

  if(type == "rtrncap")
    return i18n("Category name", "Returned capital");

  if(type == "miscinc")
    return i18n("Category name", "Miscellaneous income");

  if(type == "miscexp")
    return i18n("Category name", "Miscellaneous expense");

  if(type == "sell" || type == "buy")
    return i18n("Category name", "Investment fees");

  return i18n("Unknown QIF type %1").arg(type);
}

bool MyMoneyQifReaderPrivate::isTransfer(QString& tmp, const QString& leftDelim, const QString& rightDelim)
{
  // it's a transfer, extract the account name
  // I've seen entries like this
  //
  // S[Mehrwertsteuer]/_VATCode_N_I
  //
  // so extracting is a bit more complex and we use a regexp for it
  QRegExp exp(QString("\\%1(.*)\\%2(.*)").arg(leftDelim, rightDelim));

  bool rc;
  if((rc = (exp.search(tmp) != -1)) == true) {
    tmp = exp.cap(1)+exp.cap(2);
    tmp = tmp.stripWhiteSpace();
  }
  return rc;
}

MyMoneyQifReader::MyMoneyQifReader() :
  d(new MyMoneyQifReaderPrivate)
{
  m_skipAccount = false;
  m_transactionsProcessed =
  m_transactionsSkipped = 0;
  m_progressCallback = 0;
  m_file = 0;
  m_entryType = EntryUnknown;
  m_processingData = false;
  m_userAbort = false;
  m_autoCreatePayee = false;
  m_warnedInvestment = false;
  m_warnedSecurity = false;
  m_warnedPrice = false;

  connect(&m_filter, SIGNAL(wroteStdin(KProcess*)), this, SLOT(slotSendDataToFilter()));
  connect(&m_filter, SIGNAL(receivedStdout(KProcess*, char*, int)), this, SLOT(slotReceivedDataFromFilter(KProcess*, char*, int)));
  connect(&m_filter, SIGNAL(processExited(KProcess*)), this, SLOT(slotImportFinished()));
  connect(&m_filter, SIGNAL(receivedStderr(KProcess*, char*, int)), this, SLOT(slotReceivedErrorFromFilter(KProcess*, char*, int)));
}

MyMoneyQifReader::~MyMoneyQifReader()
{
  if(m_file)
    delete m_file;
  delete d;
}

void MyMoneyQifReader::setAutoCreatePayee(const bool create)
{
  m_autoCreatePayee = create;
}

void MyMoneyQifReader::setURL(const KURL& url)
{
  m_url = url;
}

void MyMoneyQifReader::setProfile(const QString& profile)
{
  m_qifProfile.loadProfile("Profile-" + profile);
}

void MyMoneyQifReader::slotSendDataToFilter(void)
{
  Q_LONG len;

  if(m_file->atEnd()) {
    // m_filter.flushStdin();
    m_filter.closeStdin();
  } else {
    len = m_file->readBlock(m_buffer, sizeof(m_buffer));
    if(len == -1) {
      qWarning("Failed to read block from QIF import file");
      m_filter.closeStdin();
      m_filter.kill();
    } else {
      m_filter.writeStdin(m_buffer, len);
    }
  }
}

void MyMoneyQifReader::slotReceivedErrorFromFilter(KProcess* /* proc */, char *buff, int len)
{
  QByteArray data;
  data.duplicate(buff, len);
  qWarning("%s",static_cast<const char*>(data));
}

void MyMoneyQifReader::slotReceivedDataFromFilter(KProcess* /* proc */, char *buff, int len)
{
  m_pos += len;
  // signalProgress(m_pos, 0);

  while(len) {
    // process char
    if(*buff == '\n' || *buff == '\r') {
      // found EOL
      if(!m_lineBuffer.isEmpty()) {
        m_qifLines << QString::fromUtf8(m_lineBuffer.stripWhiteSpace());
      }
      m_lineBuffer = QCString();
    } else {
      // collect all others
      m_lineBuffer += (*buff);
    }
    ++buff;
    --len;
  }
}

void MyMoneyQifReader::slotImportFinished(void)
{
  qDebug("Read %d bytes", m_pos);
  QTimer::singleShot(0, this, SLOT(slotProcessData()));
}

void MyMoneyQifReader::slotProcessData(void)
{
  signalProgress(-1, -1);

  // scan the file and try to determine numeric and date formats
  m_qifProfile.autoDetect(m_qifLines);

  // the detection is accurate for numeric values, but it could be
  // that the dates were too ambiguous so that we have to let the user
  // decide which one to pick.
  QStringList dateFormats;
  m_qifProfile.possibleDateFormats(dateFormats);
  QStringList list;
  if(dateFormats.count() > 1) {
    list << dateFormats.first();
    bool ok;
    list = KInputDialog::getItemList(i18n("Date format selection"), i18n("Pick the date format that suits your input file"), dateFormats, list, false, &ok);
    if(!ok) {
      m_userAbort = true;
    }
  } else
    list = dateFormats;

  m_qifProfile.setInputDateFormat(list.first());

  signalProgress(0, m_qifLines.count(), i18n("Importing QIF ..."));
  QStringList::iterator it;
  for(it = m_qifLines.begin(); m_userAbort == false && it != m_qifLines.end(); ++it) {
    ++m_linenumber;
    // qDebug("Proc: '%s'", (*it).data());
    if((*it).startsWith("!")) {
      processQifSpecial(*it);
      m_qifEntry.clear();
    } else if(*it == "^") {
      if(m_qifEntry.count() > 0) {
        signalProgress(m_linenumber, 0);
        processQifEntry();
        m_qifEntry.clear();
      }
    } else {
      m_qifEntry += *it;
    }
  }
  d->finishStatement();

  qDebug("%d lines processed", m_linenumber);
  signalProgress(-1, -1);

  emit importFinished();
}

bool MyMoneyQifReader::startImport(void)
{
  bool rc = false;
  d->st = MyMoneyStatement();
  m_dontAskAgain.clear();
  m_accountTranslation.clear();
  m_userAbort = false;
  m_pos = 0;
  m_linenumber = 0;
  m_filename = QString::null;
  m_data.clear();

  if(!KIO::NetAccess::download(m_url, m_filename, NULL)) {
    KMessageBox::detailedError(0,
                               i18n("Error while loading file '%1'!").arg(m_url.prettyURL()),
                               KIO::NetAccess::lastErrorString(),
                               i18n("File access error"));
    return false;
  }

  m_file = new QFile(m_filename);
  if(m_file->open(IO_ReadOnly)) {

#ifdef DEBUG_IMPORT
    Q_LONG len;

    while(!m_file->atEnd()) {
      len = m_file->readBlock(m_buffer, sizeof(m_buffer));
      if(len == -1) {
        qWarning("Failed to read block from QIF import file");
      } else {
        slotReceivedDataFromFilter(0, m_buffer, len);
      }
    }
    slotImportFinished();

#else
    // start filter process, use 'cat -' as the default filter
    m_filter.clearArguments();
    if(m_qifProfile.filterScriptImport().isEmpty()) {
      m_filter << "cat";
      m_filter << "-";
    } else {
      m_filter << QStringList::split(" ", m_qifProfile.filterScriptImport(), true);
    }
    m_entryType = EntryUnknown;

    if(m_filter.start(KProcess::NotifyOnExit, KProcess::All)) {
      m_filter.resume();
      signalProgress(0, m_file->size(), i18n("Reading QIF ..."));
      slotSendDataToFilter();
      rc = true;
    } else {
      qDebug("starting filter failed :-(");
    }
#endif
  }
  return rc;
}

bool MyMoneyQifReader::finishImport(void)
{
  bool  rc = false;

#ifdef DEBUG_IMPORT
  delete m_file;
  m_file = 0;

  // remove the Don't ask again entries
  KConfig* config = KGlobal::config();
  config->setGroup(QString::fromLatin1("Notification Messages"));
  QStringList::ConstIterator it;

  for(it = m_dontAskAgain.begin(); it != m_dontAskAgain.end(); ++it) {
    config->deleteEntry(*it);
  }
  config->sync();
  m_dontAskAgain.clear();
  m_accountTranslation.clear();

  signalProgress(-1, -1);
  rc = !m_userAbort;

#else
  if(!m_filter.isRunning()) {
    delete m_file;
    m_file = 0;

    // remove the Don't ask again entries
    KConfig* config = KGlobal::config();
    config->setGroup(QString::fromLatin1("Notification Messages"));
    QStringList::ConstIterator it;

    for(it = m_dontAskAgain.begin(); it != m_dontAskAgain.end(); ++it) {
      config->deleteEntry(*it);
    }
    config->sync();
    m_dontAskAgain.clear();
    m_accountTranslation.clear();

    signalProgress(-1, -1);
    rc = !m_userAbort && m_filter.normalExit();
  } else {
    qWarning("MyMoneyQifReader::finishImport() must not be called while the filter\n\tprocess is still running.");
  }
#endif

  // if a temporary file was constructed by NetAccess::download,
  // then it will be removed with the next call. Otherwise, it
  // stays untouched on the local filesystem
  KIO::NetAccess::removeTempFile(m_filename);

#if 0
  // Add the transaction entries
  KProgressDialog dlg(0,"transactionaddprogress",i18n("Adding transactions"),i18n("Now adding the transactions to your ledger..."));
  dlg.progressBar()->setTotalSteps(m_transactionCache.count());
  dlg.progressBar()->setTextEnabled(true);
  dlg.setAllowCancel(true);
  dlg.show();
  kapp->processEvents();
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyTransaction>::iterator it = m_transactionCache.begin();
  MyMoneyFileTransaction ft;
  try
  {
    while( it != m_transactionCache.end() )
    {
      if ( dlg.wasCancelled() )
      {
        m_userAbort = true;
        rc = false;
        break;
      }
      file->addTransaction(*it);
      dlg.progressBar()->advance(1);
      ++it;
    }
    if(rc)
      ft.commit();
  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to add transactions"),
    (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    rc = false;
  }
#endif
  // Now to import the statements
  QValueList<MyMoneyStatement>::const_iterator it_st;
  for(it_st = d->statements.begin(); it_st != d->statements.end(); ++it_st)
    kmymoney2->slotStatementImport(*it_st);
  return rc;
}

void MyMoneyQifReader::processQifSpecial(const QString& _line)
{
  QString line = _line.mid(1);   // get rid of exclamation mark
  // QString test = line.left(5).lower();
  if(line.left(5).lower() == QString("type:")) {
    line = line.mid(5);
    if(line.lower() == "ccard" || KMyMoneyGlobalSettings::qifCreditCard().lower().contains(line.lower())) {
      d->accountType = MyMoneyAccount::CreditCard;
      d->firstTransaction = true;
      d->transactionType = m_entryType = EntryTransaction;

    } else if(line.lower() == "cash" || KMyMoneyGlobalSettings::qifCash().lower().contains(line.lower())) {
      d->accountType = MyMoneyAccount::Cash;
      d->firstTransaction = true;
      d->transactionType = m_entryType = EntryTransaction;

    } else if(line.lower() == "oth a" || KMyMoneyGlobalSettings::qifAsset().lower().contains(line.lower())) {
      d->accountType = MyMoneyAccount::Asset;
      d->firstTransaction = true;
      d->transactionType = m_entryType = EntryTransaction;

    } else if(line.lower() == "oth l" || line.lower() == i18n("QIF tag for liability account", "Oth L").lower()) {
      d->accountType = MyMoneyAccount::Liability;
      d->firstTransaction = true;
      d->transactionType = m_entryType = EntryTransaction;

    } else if(line.lower() == "cat" || line.lower() == i18n("QIF tag for category", "Cat").lower()) {
      m_entryType = EntryCategory;

    } else if(line.lower() == "security" || line.lower() == i18n("QIF tag for security", "Security").lower()) {
      m_entryType = EntrySecurity;

    } else if(line.lower() == "invst" || line.lower() == i18n("QIF tag for investment account", "Invst").lower()) {
      d->transactionType = m_entryType = EntryInvestmentTransaction;

    } else if(line.lower() == "prices" || line.lower() == i18n("QIF tag for prices", "Prices").lower()) {
      m_entryType = EntryPrice;

    } else if(line.lower() == "bank" || KMyMoneyGlobalSettings::qifBank().lower().contains(line.lower())) {
      d->accountType = MyMoneyAccount::Checkings;
      d->firstTransaction = true;
      d->transactionType = m_entryType = EntryTransaction;

    } else if(line.lower() == "payee") {
      m_entryType = EntryPayee;

    } else if(line.lower() == "invoice" || KMyMoneyGlobalSettings::qifInvoice().lower().contains(line.lower())) {
      m_entryType = EntrySkip;
#if 0
    } else if(line.lower() == "memorized") {
      m_entryType = EntryMemorizedTransaction;

#endif
    } else if(line.lower() == "class" || line.lower() == i18n("QIF tag for a class", "Class").lower()) {
      m_entryType = EntryClass;

    } else {
      qWarning("Unknown type code '%s' in QIF file on line %d", line.data(), m_linenumber);
    }
  } else if(line.lower() == "account") {
    m_entryType = EntryAccount;

  } else if(line.lower() == "option:autoswitch") {
    m_entryType = EntryAccount;

  } else if(line.lower() == "clear:autoswitch") {
    m_entryType = d->transactionType;
  }
}

void MyMoneyQifReader::processQifEntry(void)
{
  // This method processes a 'QIF Entry' which is everything between two caret
  // signs
  //
  try {
    switch(m_entryType) {
      case EntryCategory:
        processCategoryEntry();
        break;

      case EntryUnknown:
        kdDebug(2) << "Line " << m_linenumber << ": Warning: Found an entry without a type being specified. Checking assumed." << endl;
        processTransactionEntry();
        break;

      case EntryTransaction:
        processTransactionEntry();
        break;

      case EntryInvestmentTransaction:
        processInvestmentTransactionEntry();
        break;

      case EntryAccount:
        processAccountEntry();
        break;

      case EntrySecurity:
        processSecurityEntry();
        break;

      case EntryPrice:
        processPriceEntry();
        break;

      case EntryPayee:
        processPayeeEntry();
        break;

      case EntryClass:
        kdDebug(2) << "Line " << m_linenumber << ": Classes are not yet supported!" << endl;
        break;

      case EntryMemorizedTransaction:
        kdDebug(2) << "Line " << m_linenumber << ": Memorized transactions are not yet implemented!" << endl;
        break;

      default:
        kdDebug(2) << "Line " << m_linenumber<< ": EntryType " << m_entryType <<" not yet implemented!" << endl;
        break;
    }
  } catch(MyMoneyException *e) {
    if(e->what() != "USERABORT") {
      kdDebug(2) << "Line " << m_linenumber << ": Unhandled error: " << e->what() << endl;
    } else {
      m_userAbort = true;
    }
    delete e;
  }
}

const QString MyMoneyQifReader::extractLine(const QChar id, int cnt)
{
  QStringList::ConstIterator it;

  m_extractedLine = -1;
  for(it = m_qifEntry.begin(); it != m_qifEntry.end(); ++it) {
    m_extractedLine++;
    if((*it)[0] == id) {
      if(cnt-- == 1) {
        if((*it).mid(1).isEmpty())
          return QString(" ");
        return (*it).mid(1);
      }
    }
  }
  m_extractedLine = -1;
  return QString();
}

void MyMoneyQifReader::extractSplits(QValueList<qSplit>& listqSplits) const
{
//     *** With apologies to QString MyMoneyQifReader::extractLine ***

  QStringList::ConstIterator it;

  for(it = m_qifEntry.begin(); it != m_qifEntry.end(); ++it) {
    if((*it)[0] == "S") {
      qSplit q;
      q.m_strCategoryName = (*it++).mid(1);       //   'S'
      if((*it)[0] == "E") {
        q.m_strMemo =  (*it++).mid(1);//           'E'
      }
      if((*it)[0] == "$") {
        q.m_amount =  (*it).mid(1);//              '$'
      }
      listqSplits += q;
    }
  }
}
#if 0
void MyMoneyQifReader::processMSAccountEntry(const MyMoneyAccount::accountTypeE accountType)
{
  if(extractLine('P').lower() == m_qifProfile.openingBalanceText().lower()) {
    m_account = MyMoneyAccount();
    m_account.setAccountType(accountType);
    QString txt = extractLine('T');
    MyMoneyMoney balance = m_qifProfile.value('T', txt);

    QDate date = m_qifProfile.date(extractLine('D'));
    m_account.setOpeningDate(date);

    QString name = extractLine('L');
    if(name.left(1) == m_qifProfile.accountDelimiter().left(1)) {
      name = name.mid(1, name.length()-2);
    }
    d->st_AccountName = name;
    m_account.setName(name);
    selectOrCreateAccount(Select, m_account, balance);
    d->st.m_accountId = m_account.id();
    if ( ! balance.isZero() )
    {
      MyMoneyFile* file = MyMoneyFile::instance();
      QCString openingtxid = file->openingBalanceTransaction(m_account);
      MyMoneyFileTransaction ft;
      if ( ! openingtxid.isEmpty() )
      {
        MyMoneyTransaction openingtx = file->transaction(openingtxid);
        MyMoneySplit split = openingtx.splitByAccount(m_account.id());

        if ( split.shares() != balance )
        {
          const MyMoneySecurity& sec = file->security(m_account.currencyId());
          if ( KMessageBox::questionYesNo(
            qApp->mainWidget(),
            i18n("The %1 account currently has an opening balance of %2. This QIF file reports an opening balance of %3. Would you like to overwrite the current balance with the one from the QIF file?").arg(m_account.name(), split.shares().formatMoney(m_account, sec),balance.formatMoney(m_account, sec)),
            i18n("Overwrite opening balance"),
            KStdGuiItem::yes(),
            KStdGuiItem::no(),
            "OverwriteOpeningBalance" )
            == KMessageBox::Yes )
          {
            file->removeTransaction( openingtx );
            m_account.setOpeningDate( date );
            file->createOpeningBalanceTransaction( m_account, balance );
          }
        }

      }
      else
      {
        // Add an opening balance
        m_account.setOpeningDate( date );
        file->createOpeningBalanceTransaction( m_account, balance );
      }
      ft.commit();
    }

  } else {
    // for some unknown reason, Quicken 2001 generates the following (somewhat
    // misleading) sequence of lines:
    //
    //  1: !Account
    //  2: NAT&T Universal
    //  3: DAT&T Univers(...xxxx) [CLOSED]
    //  4: TCCard
    //  5: ^
    //  6: !Type:CCard
    //  7: !Account
    //  8: NCFCU Visa
    //  9: DRick's CFCU Visa card (...xxxx)
    // 10: TCCard
    // 11: ^
    // 12: !Type:CCard
    // 13: D1/ 4' 1
    //
    // Lines 1-5 are processed via processQifEntry() and processAccountEntry()
    // Then Quicken issues line 6 but since the account does not carry any
    // transaction does not write an end delimiter. Arrrgh! So we end up with
    // a QIF entry comprising of lines 6-11 and end up in this routine. Actually,
    // lines 7-11 are the leadin for the next account. So we check here if
    // the !Type:xxx record also contains an !Account line and process the
    // entry as required.
    //
    // (Ace) I think a better solution here is to handle exclamation point
    // lines separately from entries.  In the above case:
    // Line 1 would set the mode to "account entries".
    // Lines 2-5 would be interpreted as an account entry.  This would set m_account.
    // Line 6 would set the mode to "cc transaction entries".
    // Line 7 would immediately set the mode to "account entries" again
    // Lines 8-11 would be interpreted as an account entry.  This would set m_account.
    // Line 12 would set the mode to "cc transaction entries"
    // Lines 13+ would be interpreted as cc transaction entries, and life is good
    int exclamationCnt = 1;
    QString category;
    do {
      category = extractLine('!', exclamationCnt++);
    } while(!category.isEmpty() && category != "Account");

    // we have such a weird empty account
    if(category == "Account") {
      processAccountEntry();
    } else
    {
      selectOrCreateAccount(Select, m_account);

      d->st_AccountName = m_account.name();
      d->st.m_strAccountName = m_account.name();
      d->st.m_accountId = m_account.id();
      d->st.m_strAccountNumber = m_account.id();
      m_account.setNumber(m_account.id());
      if ( m_entryType == EntryInvestmentTransaction )
        processInvestmentTransactionEntry();
      else
        processTransactionEntry();
    }
  }
}
#endif

void MyMoneyQifReader::processPayeeEntry(void)
{
  // TODO
}

void MyMoneyQifReader::processCategoryEntry(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  m_account = MyMoneyAccount();
  m_account.setName(extractLine('N'));
  m_account.setDescription(extractLine('D'));

  MyMoneyAccount parentAccount;
  if(!extractLine('I').isEmpty()) {
    m_account.setAccountType(MyMoneyAccount::Income);
    parentAccount = file->income();
  } else if(!extractLine('E').isEmpty()) {
    m_account.setAccountType(MyMoneyAccount::Expense);
    parentAccount = file->expense();
  }

  // check if we can find the account already in the file
  MyMoneyAccount acc = kmymoney2->findAccount(m_account, MyMoneyAccount());

  // if not, we just create it
  if(acc.id().isEmpty()) {
    MyMoneyAccount brokerage;
    MyMoneyMoney balance;
    kmymoney2->createAccount(m_account, parentAccount, brokerage, balance);
  } else
    m_account = acc;
}

QCString MyMoneyQifReader::transferAccount(QString name, bool useBrokerage)
{
  QCString accountId;
  QStringList tmpEntry = m_qifEntry;   // keep temp copies
  MyMoneyAccount tmpAccount = m_account;

  m_qifEntry.clear();               // and construct a temp entry to create/search the account
  m_qifEntry << QString("N%1").arg(name);
  m_qifEntry << QString("Tunknown");
  m_qifEntry << QString("D%1").arg(i18n("Autogenerated by QIF importer"));
  accountId = processAccountEntry(false);

  // in case we found a reference to an investment account, we need
  // to switch to the brokerage account instead.
  MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
  if(useBrokerage && (acc.accountType() == MyMoneyAccount::Investment)) {
    name = acc.brokerageName();
    m_qifEntry.clear();               // and construct a temp entry to create/search the account
    m_qifEntry << QString("N%1").arg(name);
    m_qifEntry << QString("Tunknown");
    m_qifEntry << QString("D%1").arg(i18n("Autogenerated by QIF importer"));
    accountId = processAccountEntry(false);
  }
  m_qifEntry = tmpEntry;               // restore local copies
  m_account = tmpAccount;

  return accountId;
}

void MyMoneyQifReader::processTransactionEntry(void)
{
  ++m_transactionsProcessed;
  // in case the user selected to skip the account or the account
  // was not found we skip this transaction
/*
  if(m_account.id().isEmpty()) {
    m_transactionsSkipped++;
    return;
  }
*/
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyStatement::Split s1;
  MyMoneyStatement::Transaction tr;
  QString tmp;
  QCString accountId;
  int pos;
  QString payee = extractLine('P');
  unsigned long h;

  h = MyMoneyTransaction::hash(m_qifEntry.join(";"));

  QString hashBase;
  hashBase.sprintf("%s-%07lx", m_qifProfile.date(extractLine('D')).toString(Qt::ISODate).data(), h);
  int idx = 1;
  QString hash;
  for(;;) {
    hash = QString("%1-%2").arg(hashBase).arg(idx);
    QMap<QString, bool>::const_iterator it;
    it = d->m_hashMap.find(hash);
    if(it == d->m_hashMap.end()) {
      d->m_hashMap[hash] = true;
      break;
    }
    ++idx;
  }
  tr.m_strBankID = hash;

  if(d->firstTransaction) {
    // check if this is an opening balance transaction and process it out of the statement
    if(!payee.isEmpty() && ((payee.lower() == "opening balance") || KMyMoneyGlobalSettings::qifOpeningBalance().lower().contains(payee.lower()))) {
      // if we don't have a name for the current account we need to extract the name from the L-record
      if(m_account.name().isEmpty()) {
        QString name = extractLine('L');
        if(name.isEmpty()) {
          name = i18n("QIF imported, no account name supplied");
        }
        d->isTransfer(name, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1,1));
        QStringList entry = m_qifEntry;   // keep a temp copy
        m_qifEntry.clear();               // and construct a temp entry to create/search the account
        m_qifEntry << QString("N%1").arg(name);
        m_qifEntry << QString("T%1").arg(d->accountTypeToQif(m_account.accountType()));
        m_qifEntry << QString("D%1").arg(i18n("Autogenerate by QIF importer"));
        processAccountEntry();
        m_qifEntry = entry;               // restore local copy
      }
      qDebug("Found opening balance transaction in account '%s'", m_account.name().data());

      MyMoneyFileTransaction ft;
      try {
        m_account.setOpeningDate(m_qifProfile.date(extractLine('D')));
        file->modifyAccount(m_account);
        file->createOpeningBalanceTransaction(m_account, m_qifProfile.value('T', extractLine('T')));
        ft.commit();
        // remember which account we created
        d->st.m_accountId = m_account.id();
      } catch(MyMoneyException* e) {
        KMessageBox::detailedError(0,
                    i18n("Error while creating opening balance transaction"),
                    QString("%1(%2):%3").arg(e->file()).arg(e->line()).arg(e->what()),
                    i18n("File access error"));
        delete e;
      }
      d->firstTransaction = false;
      return;
    }
  }

  // Process general transaction data

  if(d->st.m_accountId.isEmpty())
    d->st.m_accountId = m_account.id();

  s1.m_accountId = d->st.m_accountId;

  d->st.m_eType = MyMoneyStatement::etCheckings;
  tr.m_datePosted = (m_qifProfile.date(extractLine('D')));
  if(!tr.m_datePosted.isValid())
  {
    int rc = KMessageBox::warningContinueCancel(0,
         i18n("The date entry \"%1\" read from the file cannot be interpreted through the current "
              "date profile setting of \"%2\".\n\nPressing \"Continue\" will "
              "assign todays date to the transaction. Pressing \"Cancel\" will abort "
              "the import operation. You can then restart the import and select a different "
              "QIF profile or create a new one.")
           .arg(extractLine('D')).arg(m_qifProfile.inputDateFormat()),
         i18n("Invalid date format"));
    switch(rc) {
      case KMessageBox::Continue:
        tr.m_datePosted = (QDate::currentDate());
        break;

      case KMessageBox::Cancel:
        throw new MYMONEYEXCEPTION("USERABORT");
        break;
    }
  }

  tmp = extractLine('L');
  pos = tmp.findRev("--");
  if(tmp.left(1) == m_qifProfile.accountDelimiter().left(1)) {
    // it's a transfer, so we wipe the memo
//   tmp = "";         why??
//    st.m_strAccountName = tmp;
  } else if(pos != -1) {
//    what's this?
//    t.setValue("Dialog", tmp.mid(pos+2));
    tmp = tmp.left(pos);
  }
//  t.setMemo(tmp);

  // Assign the "#" field to the transaction's bank id
  // This is the custom KMM extension to QIF for a unique ID
  tmp = extractLine('#');
  if(!tmp.isEmpty())
  {
     tr.m_strBankID = ("ID " + tmp);
  }

#if 0
  // Collect data for the account's split
  s1.m_accountId = m_account.id();
  tmp = extractLine('S');
  pos = tmp.findRev("--");
  if(pos != -1) {
    tmp = tmp.left(pos);
  }
  if(tmp.left(1) == m_qifProfile.accountDelimiter().left(1))
      // it's a transfer, extract the account name
      tmp = tmp.mid(1, tmp.length()-2);
  s1.m_strCategoryName = tmp;
#endif
  // TODO (Ace) Deal with currencies more gracefully.  QIF cannot deal with multiple
  // currencies, so we should assume that transactions imported into a given
  // account are in THAT ACCOUNT's currency.  If one of those involves a transfer
  // to an account with a different currency, value and shares should be
  // different.  (Shares is in the target account's currency, value is in the
  // transaction's)


  s1.m_amount = m_qifProfile.value('T', extractLine('T'));
  tr.m_amount = m_qifProfile.value('T', extractLine('T'));
  tr.m_shares = m_qifProfile.value('T', extractLine('T'));
  tmp = extractLine('N');
  if (!tmp.isEmpty())
    tr.m_strNumber = tmp;
  tmp = extractLine('P');
  if(!tmp.isEmpty()) {
      tr.m_strPayee = tmp;
  }

  tmp = extractLine('C');
  if(tmp == "X") {
    tr.m_reconcile = MyMoneySplit::Reconciled;
  } else if(tmp == "*")               // Cleared
    tr.m_reconcile = MyMoneySplit::Cleared;

  tr.m_strMemo = extractLine('M');
  s1.m_strMemo = tr.m_strMemo;
  // tr.m_listSplits.append(s1);

  if(extractLine('$').isEmpty()) {
    MyMoneyAccount account;
    // use the same values for the second split, but clear the ID and reverse the value
    MyMoneyStatement::Split s2 = s1;
    s2.m_reconcile = tr.m_reconcile;
    s2.m_amount = (-s1.m_amount);
//    s2.clearId();

    // standard transaction
    tmp = extractLine('L');
    if(d->isTransfer(tmp, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1, 1))) {
      accountId = transferAccount(tmp, false);

    } else {
/*      pos = tmp.findRev("--");
      if(pos != -1) {
        t.setValue("Dialog", tmp.mid(pos+2));
        tmp = tmp.left(pos);
      }*/

      // it's an expense / income
      tmp = tmp.stripWhiteSpace();
      accountId = checkCategory(tmp, s1.m_amount, s2.m_amount);
    }

    if(!accountId.isEmpty()) {
      try {
        MyMoneyAccount account = file->account(accountId);
        // FIXME: check that the type matches and ask if not

        if ( account.accountType() == MyMoneyAccount::Investment )
        {
          kdDebug(0) << "Line " << m_linenumber << ": Cannot transfer to/from an investment account. Transaction ignored." << endl;
          return;
        }
        if ( account.id() == m_account.id() )
        {
          kdDebug(0) << "Line " << m_linenumber << ": Cannot transfer to the same account. Transfer ignored." << endl;
          accountId = QCString();
        }

      } catch (MyMoneyException *e) {
        kdDebug(0) << "Line " << m_linenumber << ": Account with id " << accountId.data() << " not found" << endl;
        accountId = QCString();
        delete e;
      }
    }

    if(!accountId.isEmpty()) {
      s2.m_accountId = accountId;
      s2.m_strCategoryName = tmp;
      tr.m_listSplits.append(s2);
    }

  } else {
    // split transaction
    QValueList<qSplit> listqSplits;

    extractSplits(listqSplits);   //      ****** ensure each field is ******
                                  //      *   attached to correct split    *
    int   count;

    for(count = 1; !extractLine('$', count).isEmpty(); ++count)
    {
      MyMoneyStatement::Split s2 = s1;
      s2.m_amount = (-m_qifProfile.value('$', listqSplits[count-1].m_amount));   // Amount of split
      s2.m_strMemo = listqSplits[count-1].m_strMemo;                             // Memo in split
      tmp = listqSplits[count-1].m_strCategoryName;                              // Category in split

      if(d->isTransfer(tmp, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1, 1)))
      {
        accountId = transferAccount(tmp, false);

      } else {
        pos = tmp.findRev("--");
        if(pos != -1) {
///          t.setValue("Dialog", tmp.mid(pos+2));
          tmp = tmp.left(pos);
        }
        tmp = tmp.stripWhiteSpace();
        accountId = checkCategory(tmp, s1.m_amount, s2.m_amount);
      }

      if(!accountId.isEmpty()) {
        try {
          MyMoneyAccount account = file->account(accountId);
          // FIXME: check that the type matches and ask if not

          if ( account.accountType() == MyMoneyAccount::Investment )
          {
            kdDebug(0) << "Line " << m_linenumber << ": Cannot convert a split transfer to/from an investment account. Split removed. Total amount adjusted from " << tr.m_amount.formatMoney("", 2) << " to " << (tr.m_amount + s2.m_amount).formatMoney("", 2) << "\n";
            tr.m_amount += s2.m_amount;
            continue;
          }
          if ( account.id() == m_account.id() )
          {
            kdDebug(0) << "Line " << m_linenumber << ": Cannot transfer to the same account. Transfer ignored." << endl;
            accountId = QCString();
          }

        } catch (MyMoneyException *e) {
          kdDebug(0) << "Line " << m_linenumber << ": Account with id " << accountId.data() << " not found" << endl;
          accountId = QCString();
          delete e;
        }
      }
      if(!accountId.isEmpty())
      {
        s2.m_accountId = accountId;
        s2.m_strCategoryName = tmp;
        tr.m_listSplits += s2;
        if(tr.m_listSplits.count() == 1)
          tr.m_strMemo = s2.m_strMemo;
      }
      else
      {
        // TODO add an option to create a "Unassigned" category
        // for now, we just drop the split which will show up as unbalanced
        // transaction in the KMyMoney ledger view
      }
    }
  }

  // Add the transaction to the statement
  d->st.m_listTransactions +=tr;
}

void MyMoneyQifReader::processInvestmentTransactionEntry(void)
{
//   kdDebug(2) << "Investment Transaction:" << m_qifEntry.count() << " lines" << endl;
  /*
  Items for Investment Accounts
  Field   Indicator Explanation
  D   Date
  N   Action
  Y   Security (NAME, not symbol)
  I   Price
  Q   Quantity (number of shares or split ratio)
  T   Transaction amount
  C   Cleared status
  P   Text in the first line for transfers and reminders (Payee)
  M   Memo
  O   Commission
  L   Account for the transfer
  $   Amount transferred
  ^   End of the entry

  It will be presumed all transactions are to the associated cash account, if
  one exists, unless otherwise noted by the 'L' field.

  Expense/Income categories will be automatically generated, "_Dividend",
  "_InterestIncome", etc.

  */

  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyStatement::Transaction tr;
  d->st.m_eType = MyMoneyStatement::etInvestment;
  // mark it imported for the view

//  t.setCommodity(m_account.currencyId());
  // 'D' field: Date
  QDate date = m_qifProfile.date(extractLine('D'));
  if(date.isValid())
    tr.m_datePosted = date;
  else
  {
    int rc = KMessageBox::warningContinueCancel(0,
         i18n("The date entry \"%1\" read from the file cannot be interpreted through the current "
              "date profile setting of \"%2\".\n\nPressing \"Continue\" will "
              "assign todays date to the transaction. Pressing \"Cancel\" will abort "
              "the import operation. You can then restart the import and select a different "
              "QIF profile or create a new one.")
           .arg(extractLine('D')).arg(m_qifProfile.inputDateFormat()),
         i18n("Invalid date format"));
    switch(rc) {
      case KMessageBox::Continue:
        tr.m_datePosted = QDate::currentDate();
        break;

      case KMessageBox::Cancel:
        throw new MYMONEYEXCEPTION("USERABORT");
        break;
    }
  }

  // 'M' field: Memo
  QString memo = extractLine('M');
  tr.m_strMemo = extractLine('M');
  // '#' field: BankID
  QString bankid = extractLine('#');
  if ( ! bankid.isEmpty() )
    tr.m_strBankID = bankid;

  // 'O' field: Fees
  tr.m_fees = m_qifProfile.value('T', extractLine('O'));
  // 'T' field: Amount
  MyMoneyMoney amount = m_qifProfile.value('T', extractLine('T'));
  tr.m_amount = amount;

  MyMoneyStatement::Price price;

  price.m_date = date;
  price.m_strSecurity = extractLine('Y');
  price.m_amount = m_qifProfile.value('T', extractLine('I'));

#if 0 // we must check for that later, because certain activities don't need a security
  // 'Y' field: Security name

  QString securityname = extractLine('Y').lower();
  if ( securityname.isEmpty() )
  {
    kdDebug(2) << "Line " << m_linenumber << ": Investment transaction without a security is not supported." << endl;
    return;
  }
  tr.m_strSecurity = securityname;
#endif

#if 0

  // For now, we let the statement reader take care of that.

  // The big problem here is that the Y field is not the SYMBOL, it's the NAME.
  // The name is not very unique, because people could have used slightly different
  // abbreviations or ordered words differently, etc.
  //
  // If there is a perfect name match with a subordinate stock account, great.
  // More likely, we have to rely on the QIF file containing !Type:Security
  // records, which tell us the mapping from name to symbol.
  //
  // Therefore, generally it is not recommended to import a QIF file containing
  // investment transactions but NOT containing security records.

  QString securitysymbol = m_investmentMap[securityname];

  // the correct account is the stock account which matches two criteria:
  // (1) it is a sub-account of the selected investment account, and either
  // (2a) the security name of the transaction matches the name of the security, OR
  // (2b) the security name of the transaction maps to a symbol which matches the symbol of the security

  // search through each subordinate account
  bool found = false;
  MyMoneyAccount thisaccount = m_account;
  QCStringList accounts = thisaccount.accountList();
  QCStringList::const_iterator it_account = accounts.begin();
  while( !found && it_account != accounts.end() )
  {
    QCString currencyid = file->account(*it_account).currencyId();
    MyMoneySecurity security = file->security( currencyid );
    QString symbol = security.tradingSymbol().lower();
    QString name = security.name().lower();

    if ( securityname == name || securitysymbol == symbol )
    {
      d->st_AccountId = *it_account;
      s1.m_accountId = *it_account;
      thisaccount = file->account(*it_account);
      found = true;

#if 0
      // update the price, while we're here.  in the future, this should be
      // an option
      QCString basecurrencyid = file->baseCurrency().id();
      MyMoneyPrice price = file->price( currencyid, basecurrencyid, t_in.m_datePosted, true );
      if ( !price.isValid() )
      {
        MyMoneyPrice newprice( currencyid, basecurrencyid, t_in.m_datePosted, t_in.m_moneyAmount / t_in.m_dShares, i18n("Statement Importer") );
        file->addPrice(newprice);
      }
#endif
    }

    ++it_account;
  }

  if (!found)
  {
    kdDebug(2) << "Line " << m_linenumber << ": Security " << securityname << " not found in this account.  Transaction ignored." << endl;

    // If the security is not known, notify the user
    // TODO (Ace) A "SelectOrCreateAccount" interface for investments
    KMessageBox::information(0, i18n("This investment account does not contain the \"%1\" security.  "
                                      "Transactions involving this security will be ignored.").arg(securityname),
                                i18n("Security not found"),
                                QString("MissingSecurity%1").arg(securityname.stripWhiteSpace()));
    return;
  }
#endif

  // 'Y' field: Security
  tr.m_strSecurity = extractLine('Y');

  // 'Q' field: Quantity
  MyMoneyMoney quantity = m_qifProfile.value('T', extractLine('Q'));

  // 'N' field: Action
  QString action = extractLine('N').lower();

  // remove trailing X, which seems to have no purpose (?!)
  bool xAction = false;
  if ( action.endsWith("x") ) {
    action = action.left( action.length() - 1 );
    xAction = true;
  }

  // Whether to create a cash split for the other side of the value
  QString accountname ;//= extractLine('L');
  if ( action == "reinvdiv" || action == "reinvlg" || action == "reinvsh" )
  {
    d->st.m_listPrices += price;
    tr.m_shares = quantity;
    tr.m_eAction = (MyMoneyStatement::Transaction::eaReinvestDividend);
    tr.m_price = m_qifProfile.value('I', extractLine('I'));

    tr.m_strInterestCategory = extractLine('L');
    if(tr.m_strInterestCategory.isEmpty()) {
      tr.m_strInterestCategory = d->typeToAccountName(action);
    }
  }
  else if ( action == "div" || action == "intinc" || action == "cgshort"
         || action == "cgmid" || action == "cglong" || action == "rtrncap"
         || action == "miscinc")
  {
    tr.m_eAction = (MyMoneyStatement::Transaction::eaCashDividend);

    QString tmp = extractLine('L');
    // if the action ends in an X, the L-Record contains the asset account
    // to which the dividend should be transferred. In the other cases, it
    // may contain a category that identifies the income category for the
    // dividend payment
    if((xAction == true)
    && (d->isTransfer(tmp, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1, 1)) == true)) {
      tr.m_strBrokerageAccount = tmp;
      transferAccount(tmp);           // make sure the account exists
    } else {
      tr.m_strInterestCategory = tmp;
    }

    // make sure, we have valid category. Either taken from the L-Record above,
    // or derived from the action code
    if(tr.m_strInterestCategory.isEmpty()) {
      tr.m_strInterestCategory = d->typeToAccountName(action);
    }

    // For historic reasons (coming from the OFX importer) the statement
    // reader expects the dividend with a reverse sign. So we just do that.
    tr.m_amount = -(amount - tr.m_fees);

    // We need an extra split which will be the zero-amount investment split
    // that serves to mark this transaction as a cash dividend and note which
    // stock account it belongs to.
    MyMoneyStatement::Split s2;
    s2.m_amount = MyMoneyMoney();
    s2.m_strCategoryName = extractLine('Y');
    tr.m_listSplits.append(s2);
  }
  else if (action == "miscexp")
  {
    tr.m_eAction = (MyMoneyStatement::Transaction::eaFees);
    QString tmp = extractLine('L');
    if(d->isTransfer(tmp, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1, 1))) {

      // FIXME Use transferAccount() instead of the logic below
      // FIXME Also, I noticed that the accountid is unused in this context
      QStringList entry = m_qifEntry;   // keep temp copies
      MyMoneyAccount acc = m_account;

      m_qifEntry.clear();               // and construct a temp entry to create/search the account
      m_qifEntry << QString("N%1").arg(tmp);
      m_qifEntry << QString("Tunknown");
      m_qifEntry << QString("D%1").arg(i18n("Autogenerated by QIF importer"));
      QCString accountId = processAccountEntry(false);

      tr.m_strInterestCategory = tmp;

      m_qifEntry = entry;               // restore local copies
      m_account = acc;

    } else {
      tr.m_strInterestCategory = extractLine('L');
      if(tr.m_strInterestCategory.isEmpty()) {
        tr.m_strInterestCategory = d->typeToAccountName(action);
      }
    }
  }
  else if (action == "xin" || action == "xout")
  {
    tr.m_eAction = (MyMoneyStatement::Transaction::eaNone);
    MyMoneyStatement::Split s2;
    QString tmp = extractLine('L');
    if(d->isTransfer(tmp, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1, 1))) {
      s2.m_accountId = transferAccount(tmp);
      s2.m_strCategoryName = tmp;
    } else {
      s2.m_strCategoryName = extractLine('L');
      if(tr.m_strInterestCategory.isEmpty()) {
        s2.m_strCategoryName = d->typeToAccountName(action);
      }
    }
    if(action == "xout")
      tr.m_amount = -tr.m_amount;

    s2.m_amount = -tr.m_amount;
    tr.m_listSplits.append(s2);
  }
  else if (action == "buy")
  {
    QString tmp = extractLine('L');
    if((xAction == true)
    && (d->isTransfer(tmp, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1, 1)) == true)) {
      tr.m_strBrokerageAccount = tmp;
      transferAccount(tmp);           // make sure the account exists
    }

    d->st.m_listPrices += price;
    tr.m_shares = quantity;
    tr.m_eAction = (MyMoneyStatement::Transaction::eaBuy);
  }
  else if (action == "sell")
  {
    QString tmp = extractLine('L');
    if((xAction == true)
    && (d->isTransfer(tmp, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1, 1)) == true)) {
      tr.m_strBrokerageAccount = tmp;
      transferAccount(tmp);           // make sure the account exists
    }
    d->st.m_listPrices += price;
    tr.m_shares = -quantity;
    tr.m_eAction = (MyMoneyStatement::Transaction::eaSell);
  }
  else if ( action == "shrsin" )
  {
    tr.m_shares = quantity;
    tr.m_eAction = (MyMoneyStatement::Transaction::eaShrsin);
  }
  else if ( action == "shrsout" )
  {
    tr.m_shares = -quantity;
    tr.m_eAction = (MyMoneyStatement::Transaction::eaShrsout);
  }
  else if ( action == "stksplit" )
  {
    MyMoneyMoney splitfactor = (quantity / MyMoneyMoney(10,1)).reduce();

    // Stock splits not supported
//     kdDebug(2) << "Line " << m_linenumber << ": Stock split not supported (date=" << date << " security=" << securityname << " factor=" << splitfactor.toString() << ")" << endl;

//    s1.setShares(splitfactor);
//    s1.setValue(0);
//   s1.setAction(MyMoneySplit::ActionSplitShares);

//     return;
  }
  else
  {
    // Unsupported action type
    kdDebug(0) << "Line " << m_linenumber << ": Unsupported transaction action (" << action << ")" << endl;
    return;
  }
  d->st.m_strAccountName = accountname;
  d->st.m_listTransactions +=tr;

  /*************************************************************************
   *
   * These transactions are natively supported by KMyMoney
   *
   *************************************************************************/
  /*
  D1/ 3' 5
  NShrsIn
  YGENERAL MOTORS CORP 52BR1
  I20
  Q200
  U4,000.00
  T4,000.00
  M200 shares added to account @ $20/share
  ^
  */
  /*
  ^
  D1/14' 5
  NShrsOut
  YTEMPLETON GROWTH 97GJ0
  Q50
90  ^
  */
  /*
  D1/28' 5
  NBuy
  YGENERAL MOTORS CORP 52BR1
  I24.35
  Q100
  U2,435.00
  T2,435.00
  ^
  */
  /*
  D1/ 5' 5
  NSell
  YUnited Vanguard
  I8.41
  Q50
  U420.50
  T420.50
  ^
  */
  /*
  D1/ 7' 5
  NReinvDiv
  YFRANKLIN INCOME 97GM2
  I38
  Q1
  U38.00
  T38.00
  ^
  */
  /*************************************************************************
   *
   * These transactions are all different kinds of income.  (Anything that
   * follows the DNYUT pattern).  They are all handled the same, the only
   * difference is which income account the income is placed into.  By
   * default, it's placed into _xxx where xxx is the right side of the
   * N field.  e.g. NDiv transaction goes into the _Div account
   *
   *************************************************************************/
  /*
  D1/10' 5
  NDiv
  YTEMPLETON GROWTH 97GJ0
  U10.00
  T10.00
  ^
  */
  /*
  D1/10' 5
  NIntInc
  YTEMPLETON GROWTH 97GJ0
  U20.00
  T20.00
  ^
  */
  /*
  D1/10' 5
  NCGShort
  YTEMPLETON GROWTH 97GJ0
  U111.00
  T111.00
  ^
  */
  /*
  D1/10' 5
  NCGLong
  YTEMPLETON GROWTH 97GJ0
  U333.00
  T333.00
  ^
  */
  /*
  D1/10' 5
  NCGMid
  YTEMPLETON GROWTH 97GJ0
  U222.00
  T222.00
  ^
  */
  /*
  D2/ 2' 5
  NRtrnCap
  YFRANKLIN INCOME 97GM2
  U1,234.00
  T1,234.00
  ^
  */
  /*************************************************************************
   *
   * These transactions deal with miscellaneous activity that KMyMoney
   * does not support, but may support in the future.
   *
   *************************************************************************/
  /*   Note the Q field is the split ratio per 10 shares, so Q12.5 is a
        12.5:10 split, otherwise known as 5:4.
  D1/14' 5
  NStkSplit
  YIBM
  Q12.5
  ^
  */
  /*************************************************************************
   *
   * These transactions deal with short positions and options, which are
   * not supported at all by KMyMoney.  They will be ignored for now.
   * There may be a way to hack around this, by creating a new security
   * "IBM_Short".
   *
   *************************************************************************/
  /*
  D1/21' 5
  NShtSell
  YIBM
  I92.38
  Q100
  U9,238.00
  T9,238.00
  ^
  */
  /*
  D1/28' 5
  NCvrShrt
  YIBM
  I92.89
  Q100
  U9,339.00
  T9,339.00
  O50.00
  ^
  */
  /*
  D6/ 1' 5
  NVest
  YIBM Option
  Q20
  ^
  */
  /*
  D6/ 8' 5
  NExercise
  YIBM Option
  I60.952381
  Q20
  MFrom IBM Option Grant 6/1/2004
  ^
  */
  /*
  D6/ 1'14
  NExpire
  YIBM Option
  Q5
  ^
  */
  /*************************************************************************
   *
   * These transactions do not have an associated investment ("Y" field)
   * so presumably they are only valid for the cash account.  Once I
   * understand how these are really implemented, they can probably be
   * handled without much trouble.
   *
   *************************************************************************/
  /*
  D1/14' 5
  NCash
  U-100.00
  T-100.00
  LBank Chrg
  ^
  */
  /*
  D1/15' 5
  NXOut
  U500.00
  T500.00
  L[CU Savings]
  $500.00
  ^
  */
  /*
  D1/28' 5
  NXIn
  U1,000.00
  T1,000.00
  L[CU Checking]
  $1,000.00
  ^
  */
  /*
  D1/25' 5
  NMargInt
  U25.00
  T25.00
  ^
  */
}

const QCString MyMoneyQifReader::findOrCreateIncomeAccount(const QString& searchname)
{
  QCString result;

  MyMoneyFile *file = MyMoneyFile::instance();

  // First, try to find this account as an income account
  MyMoneyAccount acc = file->income();
  QCStringList list = acc.accountList();
  QCStringList::ConstIterator it_accid = list.begin();
  while ( it_accid != list.end() )
  {
    acc = file->account(*it_accid);
    if ( acc.name() == searchname )
    {
      result = *it_accid;
      break;
    }
    ++it_accid;
  }

  // If we did not find the account, now we must create one.
  if ( result.isEmpty() )
  {
    MyMoneyAccount acc;
    acc.setName( searchname );
    acc.setAccountType( MyMoneyAccount::Income );
    MyMoneyAccount income = file->income();
    MyMoneyFileTransaction ft;
    file->addAccount( acc, income );
    ft.commit();
    result = acc.id();
  }

  return result;
}

// TODO (Ace) Combine this and the previous function

const QCString MyMoneyQifReader::findOrCreateExpenseAccount(const QString& searchname)
{
  QCString result;

  MyMoneyFile *file = MyMoneyFile::instance();

  // First, try to find this account as an income account
  MyMoneyAccount acc = file->expense();
  QCStringList list = acc.accountList();
  QCStringList::ConstIterator it_accid = list.begin();
  while ( it_accid != list.end() )
  {
    acc = file->account(*it_accid);
    if ( acc.name() == searchname )
    {
      result = *it_accid;
      break;
    }
    ++it_accid;
  }

  // If we did not find the account, now we must create one.
  if ( result.isEmpty() )
  {
    MyMoneyAccount acc;
    acc.setName( searchname );
    acc.setAccountType( MyMoneyAccount::Expense );
    MyMoneyFileTransaction ft;
    MyMoneyAccount expense = file->expense();
    file->addAccount( acc, expense );
    ft.commit();
    result = acc.id();
  }

  return result;
}

const QCString MyMoneyQifReader::checkCategory(const QString& name, const MyMoneyMoney value, const MyMoneyMoney value2)
{
  QCString accountId;
  MyMoneyFile *file = MyMoneyFile::instance();
  MyMoneyAccount account;
  bool found = true;

  if(!name.isEmpty()) {
    // The category might be constructed with an arbitraty depth (number of
    // colon delimited fields). We try to find a parent account within this
    // hierarchy by searching the following sequence:
    //
    //    aaaa:bbbb:cccc:ddddd
    //
    // 1. search aaaa:bbbb:cccc:dddd, create nothing
    // 2. search aaaa:bbbb:cccc     , create dddd
    // 3. search aaaa:bbbb          , create cccc:dddd
    // 4. search aaaa               , create bbbb:cccc:dddd
    // 5. don't search              , create aaaa:bbbb:cccc:dddd

    account.setName(name);
    QString accName;      // part to be created (right side in above list)
    QString parent(name); // a possible parent part (left side in above list)
    do {
      accountId = file->categoryToAccount(parent);
      if(accountId.isEmpty()) {
        found = false;
        // prepare next step
        if(!accName.isEmpty())
          accName.prepend(':');
        accName.prepend(parent.section(':', -1));
        account.setName(accName);
        parent = parent.section(':', 0, -2);
      } else if(!accName.isEmpty()) {
        account.setParentAccountId(accountId);
      }
    }
    while(!parent.isEmpty() && accountId.isEmpty());

    // if we did not find the category, we create it
    if(!found) {
      MyMoneyAccount parent;
      if(account.parentAccountId().isEmpty()) {
        if(!value.isNegative() && value2.isNegative())
          parent = file->income();
        else
          parent = file->expense();
      } else {
        parent = file->account(account.parentAccountId());
      }
      account.setAccountType((!value.isNegative() && value2.isNegative()) ? MyMoneyAccount::Income : MyMoneyAccount::Expense);
      MyMoneyAccount brokerage;
      // clear out the parent id, because createAccount() does not like that
      account.setParentAccountId(QCString());
      kmymoney2->createAccount(account, parent, brokerage, MyMoneyMoney());
      accountId = account.id();
    }
  }

  return accountId;
}

QCString MyMoneyQifReader::processAccountEntry(bool resetAccountId)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyAccount account;
  QString tmp;

  account.setName(extractLine('N'));
  // qDebug("Process account '%s'", account.name().data());

  account.setDescription(extractLine('D'));

  tmp = extractLine('$');
  if(tmp.length() > 0)
    account.setValue("lastStatementBalance", tmp);

  tmp = extractLine('/');
  if(tmp.length() > 0)
    account.setValue("lastStatementDate", m_qifProfile.date(tmp).toString("yyyy-MM-dd"));

  QifEntryTypeE transactionType = EntryTransaction;
  QString type = extractLine('T').lower().remove(QRegExp("\\s+"));
  if(type == m_qifProfile.profileType().lower().remove(QRegExp("\\s+"))) {
    account.setAccountType(MyMoneyAccount::Checkings);
  } else if(type == "ccard" || type == "creditcard") {
    account.setAccountType(MyMoneyAccount::CreditCard);
  } else if(type == "cash") {
    account.setAccountType(MyMoneyAccount::Cash);
  } else if(type == "otha") {
    account.setAccountType(MyMoneyAccount::Asset);
  } else if(type == "othl") {
    account.setAccountType(MyMoneyAccount::Liability);
  } else if(type == "invst" || type == "port") {
    account.setAccountType(MyMoneyAccount::Investment);
    transactionType = EntryInvestmentTransaction;
  } else if(type == "mutual") { // stock account w/o umbrella investment account
    account.setAccountType(MyMoneyAccount::Stock);
    transactionType = EntryInvestmentTransaction;
  } else if(type == "unknown") {
    // don't do anything with the type, leave it unknown
  } else {
    account.setAccountType(MyMoneyAccount::Checkings);
    kdDebug(2) << "Line " << m_linenumber << ": Unknown account type '" << type << "', checkings assumed" << endl;
  }

  // check if we can find the account already in the file
  MyMoneyAccount acc = kmymoney2->findAccount(account, MyMoneyAccount());
  if(acc.id().isEmpty()) {
    // in case the account is not found by name and the type is
    // unknown, we have to assume something and create a checking account.
    // this might be wrong, but we have no choice at this point.
    if(account.accountType() == MyMoneyAccount::UnknownAccountType)
      account.setAccountType(MyMoneyAccount::Checkings);

    MyMoneyAccount parentAccount;
    MyMoneyAccount brokerage;
    MyMoneyMoney balance;
    // in case it's a stock account, we need to setup a fix investment account
    if(account.isInvest()) {
      acc.setName(i18n("Standard name for QIF investment portfolio account", "QIF investment portfolio"));
      acc.setAccountType(MyMoneyAccount::Investment);
      parentAccount = file->asset();
      kmymoney2->createAccount(acc, parentAccount, brokerage, MyMoneyMoney());
      parentAccount = acc;
      qDebug("We still need to create the stock account in MyMoneyQifReader::processAccountEntry()");
    } else {
      // setup parent according the type of the account
      switch(account.accountGroup()) {
        case MyMoneyAccount::Asset:
        default:
          parentAccount = file->asset();
          break;
        case MyMoneyAccount::Liability:
          parentAccount = file->liability();
          break;
        case MyMoneyAccount::Equity:
          parentAccount = file->equity();
          break;
      }
    }

    // investment accounts will receive a brokerage account, as KMyMoney
    // currently does not allow to store funds in the investment account directly
    if(account.accountType() == MyMoneyAccount::Investment) {
      brokerage.setName(account.brokerageName());
      brokerage.setAccountType(MyMoneyAccount::Checkings);
      brokerage.setCurrencyId(MyMoneyFile::instance()->baseCurrency().id());
    }
    kmymoney2->createAccount(account, parentAccount, brokerage, balance);
    acc = account;
    // qDebug("Account created");
  } else {
    // qDebug("Existing account found");
  }

  if(resetAccountId) {
    // possibly start a new statement
    d->finishStatement();
    m_account = acc;
    d->st.m_accountId = m_account.id();
    d->transactionType = transactionType;
  }
  return acc.id();
}

void MyMoneyQifReader::selectOrCreateAccount(const SelectCreateMode mode, MyMoneyAccount& account, const MyMoneyMoney& balance)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QCString accountId;
  QString msg;
  QString typeStr;
  QString leadIn;
  KMyMoneyUtils::categoryTypeE type;

  QMap<QString, QCString>::ConstIterator it;

  type = KMyMoneyUtils::none;
  switch(account.accountGroup()) {
    default:
      type = KMyMoneyUtils::asset;
      type = (KMyMoneyUtils::categoryTypeE) (type | KMyMoneyUtils::liability);
      typeStr = i18n("account");
      leadIn = i18n("al");
      break;

    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
      type = KMyMoneyUtils::income;
      type = (KMyMoneyUtils::categoryTypeE) (type | KMyMoneyUtils::expense);
      typeStr = i18n("category");
      leadIn = i18n("ei");
      msg = i18n("Category selection");
      break;
  }

  KAccountSelectDlg accountSelect(type, "QifImport", kmymoney2);
  if(!msg.isEmpty())
    accountSelect.setCaption(msg);

  it = m_accountTranslation.find((leadIn + MyMoneyFile::AccountSeperator + account.name()).lower());
  if(it != m_accountTranslation.end()) {
    try {
      account = file->account(*it);
      return;

    } catch (MyMoneyException *e) {
      QString message(i18n("Account \"%1\" disappeared: ").arg(account.name()));
      message += e->what();
      KMessageBox::error(0, message);
      delete e;
    }
  }

  if(!account.name().isEmpty()) {
    if(type & (KMyMoneyUtils::income | KMyMoneyUtils::expense)) {
      accountId = file->categoryToAccount(account.name());
    } else {
      accountId = file->nameToAccount(account.name());
    }

    if(mode == Create) {
      if(!accountId.isEmpty()) {
        account = file->account(accountId);
        return;

      } else {
        switch(KMessageBox::questionYesNo(0,
                  i18n("The %1 '%2' does not exist. Do you "
                       "want to create it?").arg(typeStr).arg(account.name()))) {
          case KMessageBox::Yes:
            break;
          case KMessageBox::No:
            return;
        }
      }
    } else {
      accountSelect.setHeader(i18n("Select %1").arg(typeStr));
      if(!accountId.isEmpty()) {
        msg = i18n("The %1 <b>%2</b> currently exists. Do you want "
                   "to import transactions to this account?")
                    .arg(typeStr).arg(account.name());

      } else {
        msg = i18n("The %1 <b>%2</b> currently does not exist. You can "
                   "create a new %3 by pressing the <b>Create</b> button "
                   "or select another %4 manually from the selection box.")
                  .arg(typeStr).arg(account.name()).arg(typeStr).arg(typeStr);
      }
    }
  } else {
    accountSelect.setHeader(i18n("Import transactions to %1").arg(typeStr));
    msg = i18n("No %1 information has been found in the selected QIF file. "
               "Please select an account using the selection box in the dialog or "
               "create a new %2 by pressing the <b>Create</b> button.")
               .arg(typeStr).arg(typeStr);
  }

  accountSelect.setDescription(msg);
  accountSelect.setAccount(account, accountId);
  accountSelect.setMode(mode == Create);
  accountSelect.showAbortButton(true);

  // display current entry in widget, the offending line (if any) will be shown in red
  QStringList::Iterator it_e;
  int i = 0;
  for(it_e = m_qifEntry.begin(); it_e != m_qifEntry.end(); ++it_e) {
    if(m_extractedLine == i)
      accountSelect.m_qifEntry->setColor(QColor("red"));
    accountSelect.m_qifEntry->append(*it_e);
    accountSelect.m_qifEntry->setColor(QColor("black"));
    ++i;
  }

  for(;;) {
    if(accountSelect.exec() == QDialog::Accepted) {
      if(!accountSelect.selectedAccount().isEmpty()) {
        accountId = accountSelect.selectedAccount();

        m_accountTranslation[(leadIn + MyMoneyFile::AccountSeperator + account.name()).lower()] = accountId;

        // MMAccount::openingBalance() is where the accountSelect dialog has
        // stashed the opening balance that the user chose.
        MyMoneyAccount importedAccountData(account);
        // MyMoneyMoney balance = importedAccountData.openingBalance();
        account = file->account(accountId);
        if ( ! balance.isZero() )
        {
          QCString openingtxid = file->openingBalanceTransaction(account);
          MyMoneyFileTransaction ft;
          if ( ! openingtxid.isEmpty() )
          {
            MyMoneyTransaction openingtx = file->transaction(openingtxid);
            MyMoneySplit split = openingtx.splitByAccount(account.id());

            if ( split.shares() != balance )
            {
              const MyMoneySecurity&  sec = file->security(account.currencyId());
              if ( KMessageBox::questionYesNo(
                qApp->mainWidget(),
                i18n("The %1 account currently has an opening balance of %2. This QIF file reports an opening balance of %3. Would you like to overwrite the current balance with the one from the QIF file?").arg(account.name(), split.shares().formatMoney(account, sec), balance.formatMoney(account, sec)),
                i18n("Overwrite opening balance"),
                KStdGuiItem::yes(),
                KStdGuiItem::no(),
                "OverwriteOpeningBalance" )
                == KMessageBox::Yes )
              {
                file->removeTransaction( openingtx );
                file->createOpeningBalanceTransaction( account, balance );
              }
            }
          }
          else
          {
            // Add an opening balance
            file->createOpeningBalanceTransaction( account, balance );
          }
          ft.commit();
        }
        break;
      }

    } else if(accountSelect.aborted())
      throw new MYMONEYEXCEPTION("USERABORT");

    if(typeStr == i18n("account")) {
      KMessageBox::error(0, i18n("You must select or create an account."));
    } else {
      KMessageBox::error(0, i18n("You must select or create a category."));
    }
  }
}

void MyMoneyQifReader::setProgressCallback(void(*callback)(int, int, const QString&))
{
  m_progressCallback = callback;
}

void MyMoneyQifReader::signalProgress(int current, int total, const QString& msg)
{
  if(m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}

void MyMoneyQifReader::processPriceEntry(void)
{
/*
  !Type:Prices
  "IBM",141 9/16,"10/23/98"
  ^
  !Type:Prices
  "GMW",21.28," 3/17' 5"
  ^
  !Type:Prices
  "GMW",71652181.001,"67/128/ 0"
  ^

  Note that Quicken will often put in a price with a bogus date and number.  We will ignore
  prices with bogus dates.  Hopefully that will catch all of these.

  Also note that prices can be in fractional units, e.g. 141 9/16.

*/

  QStringList::const_iterator it_line = m_qifEntry.begin();

  // Make a price for each line
  QRegExp priceExp("\"(.*)\",(.*),\"(.*)\"");
  while ( it_line != m_qifEntry.end() )
  {
    if(priceExp.search(*it_line) != -1) {
      MyMoneyStatement::Price price;
      price.m_strSecurity = priceExp.cap(1);
      QString pricestr = priceExp.cap(2);
      QString datestr = priceExp.cap(3);
      kdDebug(2) << "Price:" << price.m_strSecurity << " / " << pricestr << " / " << datestr << endl;

      // Only add the price if the date is valid.  If invalid, fail silently.  See note above.
      // Also require the price value to not have any slashes.  Old prices will be something like
      // "25 9/16", which we do not support.  So we'll skip the price for now.
      QDate date = m_qifProfile.date(datestr);
      MyMoneyMoney rate(m_qifProfile.value('P', pricestr));
      if(date.isValid() && !rate.isZero())
      {
        price.m_amount = rate;
        price.m_date = date;
        d->st.m_listPrices += price;
      }
    }
    ++it_line;
  }
}

void MyMoneyQifReader::processSecurityEntry(void)
{
  /*
  !Type:Security
  NVANGUARD 500 INDEX
  SVFINX
  TMutual Fund
  ^
  */

  MyMoneyStatement::Security security;
  security.m_strName = extractLine('N');
  security.m_strSymbol = extractLine('S');

  d->st.m_listSecurities += security;
}

#include "mymoneyqifreader.moc"
