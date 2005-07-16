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

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyqifreader.h"
#include "../mymoney/mymoneyfile.h"
#include "../dialogs/kaccountselectdlg.h"
#include "../kmymoney2.h"

// define this to debug the code. Using external filters
// while debugging did not work to good for me, so I added
// this code.
// #define DEBUG_IMPORT

#ifdef DEBUG_IMPORT
#warning "DEBUG_IMPORT defined --> external filter not available!!!!!!!"
#endif

MyMoneyQifReader::MyMoneyQifReader()
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
}

void MyMoneyQifReader::setAutoCreatePayee(const bool create)
{
  m_autoCreatePayee = create;
}

void MyMoneyQifReader::setFilename(const QString& name)
{
  m_filename = name;
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
  QByteArray data;
  data.duplicate(buff, len);

  // if the list of received data blocks is empty, start the processor
  if(m_data.count() == 0)
    QTimer::singleShot(10, this, SLOT(slotProcessBuffers()));

  m_data.append(data);
}

void MyMoneyQifReader::slotProcessBuffers(void)
{
  QValueList<QByteArray>::Iterator it = m_data.begin();
  while(!m_userAbort && it != m_data.end()) 
  {
    QBuffer databuffer(*it);
    databuffer.open( IO_ReadOnly );
    
    while ( ! databuffer.atEnd() )
    {
      QCString line(80);
      databuffer.readLine( line.data(), line.size() );
      m_qifLine += QString::fromUtf8(line);  // or fromLocal8Bit?
      
      if ( m_qifLine.right(1) == "\n" )
      {
        m_qifLine.remove(QRegExp("[\r\n]"));
        processQifLine();
        m_qifLine = QCString();
      }
    }
    
    databuffer.close();

    // remove this chunk from the list.  In case ANOTHER chunk came in while
    // we were doing this operation, we can't just ++it and clear() at the end.    
    it = m_data.remove(it);
  }

#ifndef DEBUG_IMPORT
  emit importFinished();
#endif
}

void MyMoneyQifReader::processQifLine(void)
{
  ++m_linenumber;
  m_pos += m_qifLine.length() + 2;
  while(m_qifLine.endsWith(" ") || m_qifLine.endsWith("\t"))
    m_qifLine = m_qifLine.left(m_qifLine.length()-1);

  // skip empty lines
  if(!m_qifLine.isEmpty()) {

    if(m_qifLine == "^") {

      // skip empty entries
      if(m_qifEntry.count() != 0) {
        signalProgress(m_pos, 0);
        processQifEntry();
        m_qifEntry.clear();
      }
    } else {
      m_qifEntry += m_qifLine;
    }
  }
}

void MyMoneyQifReader::slotImportFinished(void)
{
  // slotReceivedDataFromFilter();
  // slotReceivedErrorFromFilter();
}

const bool MyMoneyQifReader::startImport(void)
{
  bool rc = false;

  m_dontAskAgain.clear();
  m_accountTranslation.clear();
  m_userAbort = false;
  m_pos = 0;
  m_linenumber = 0;

  m_data.clear();

  m_file = new QFile(m_filename);
  if(m_file->open(IO_ReadOnly)) {

#ifdef DEBUG_IMPORT
    Q_LONG len;

    while(!m_file->atEnd()) {
      len = m_file->readBlock(m_buffer, sizeof(m_buffer));
      if(len == -1) {
        qWarning("Failed to read block from QIF import file");
      } else {
        QByteArray data;
        data.duplicate(m_buffer, len);
        m_data.append(data);
        slotProcessBuffers();
      }
    }
    emit importFinished();

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
      signalProgress(0, m_file->size(), "Importing QIF ...");
      slotSendDataToFilter();
      rc = true;
    } else {
      qDebug("starting filter failed :-(");
    }
#endif
  }
  return rc;
}

const bool MyMoneyQifReader::finishImport(void)
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

  // Add the transaction entries
  KProgressDialog dlg(0,"transactionaddprogress",i18n("Adding transactions"),i18n("Now adding the transactions to your ledger..."));
  dlg.progressBar()->setTotalSteps(m_transactionCache.count());
  dlg.progressBar()->setTextEnabled(true);
  dlg.setAllowCancel(true);
  dlg.show();
  kapp->processEvents();
  MyMoneyFile* file = MyMoneyFile::instance();
  file->suspendNotify(true);
  QValueList<MyMoneyTransaction>::iterator it = m_transactionCache.begin();
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
  file->suspendNotify(false);

  return rc;
}

void MyMoneyQifReader::processQifEntry(void)
{
  // This method processes a 'QIF Entry' which is everything between two caret
  // signs
  //
  // FIXME I think that's not strictly true to say that an entry is everything
  // between carets.  An exclamation-point-started line should not be considered
  // part of an entry.

  int exclamationCnt = 1;

  try {
    QString category = extractLine('!', exclamationCnt++);
    if(!category.isEmpty()) {

      while(!category.isEmpty()) {
        if(category.left(5) == "Type:") {

          category = category.mid(5);
          m_entryType = EntryTransaction;
          if(category == m_qifProfile.profileType()) {
            processMSAccountEntry(MyMoneyAccount::Checkings);

          } else if(category == "CCard") {
            processMSAccountEntry(MyMoneyAccount::CreditCard);

          } else if(category == "Cash") {
            processMSAccountEntry(MyMoneyAccount::Cash);

          } else if(category == "Oth A") {
            processMSAccountEntry(MyMoneyAccount::Asset);

          } else if(category == "Oth L") {
            processMSAccountEntry(MyMoneyAccount::Liability);

          } else if(category == "Cat") {
            m_entryType = EntryCategory;
            processCategoryEntry();

          } else if(category == "Memorized") {
            m_entryType = EntryMemorizedTransaction;

          } else if(category == "Security") {
            m_entryType = EntrySecurity;
            processSecurityEntry();
          } else if(category == "Invst") {
            m_entryType = EntryInvestmentTransaction;
            processMSAccountEntry(MyMoneyAccount::Investment);
          } else if(category == "Prices") {
            m_entryType = EntryPrice;
            processPriceEntry();
          } else
            kdDebug(2) << "Line " << m_linenumber<< ": Unknown '!Type:" << category << "' category" << endl;

        } else if(category == "Account") {
          processAccountEntry();

        } else if(category == "Option:AutoSwitch") {
          m_entryType = EntryAccount;
          category = extractLine('!', exclamationCnt++);    // is there another record?
          continue;

        } else if(category == "Clear:AutoSwitch") {
          m_entryType = EntryTransaction;
          category = extractLine('!', exclamationCnt++);    // is there another record?
          continue;

        } else
          kdDebug(2) << "Line " << m_linenumber<< ": Unknown '!" << category << "' category" << endl;

        break;
      }

    } else {
      // Process entry of same type
      switch(m_entryType) {
        case EntryUnknown:
          kdDebug(2) << "Line " << m_linenumber << ":Found an entry without a type being specified. Entry skipped." << endl;
          break;

        case EntryCategory:
          processCategoryEntry();
          break;

        case EntryTransaction:
          processTransactionEntry();
          break;

        case EntryInvestmentTransaction:
          processInvestmentTransactionEntry();
          break;

        case EntryAccount:
          m_account = MyMoneyAccount();
          processAccountEntry();
          break;
          
        case EntrySecurity:
          processSecurityEntry();
          break;

        case EntryPrice:
          processPriceEntry();
          break;

        case EntryMemorizedTransaction:
          kdDebug(2) << "Line " << m_linenumber << ": Memorized transactions are not yet implemented!" << endl;
          break;
        
        default:
          kdDebug(2) << "Line " << m_linenumber<< ": EntryType " << m_entryType <<" not yet implemented!" << endl;
          break;
      }
    }
  } catch(MyMoneyException *e) {
    if(e->what() != "USERABORT") {
      kdDebug(2) << "Line " << m_linenumber << ": Unhandled error: " << e->what() << endl;
    } else {
      m_filter.closeStdin();
      m_filter.kill();
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

void MyMoneyQifReader::processMSAccountEntry(const MyMoneyAccount::accountTypeE accountType)
{
  if(extractLine('P').lower() == m_qifProfile.openingBalanceText().lower()) {
    m_account = MyMoneyAccount();
    m_account.setAccountType(accountType);
    QString txt = extractLine('T');
    MyMoneyMoney balance = m_qifProfile.value('T', txt);
    m_account.setOpeningBalance(balance);

    QDate date = m_qifProfile.date(extractLine('D'));
    m_account.setOpeningDate(date);

    QString name = extractLine('L');
    if(name.left(1) == m_qifProfile.accountDelimiter().left(1)) {
      name = name.mid(1, name.length()-2);
    }
    m_account.setName(name);
    selectOrCreateAccount(Select, m_account);
       
    if ( ! balance.isZero() )
    {          
      MyMoneyFile* file = MyMoneyFile::instance();
      QCString openingtxid = file->openingBalanceTransaction(m_account);
      if ( ! openingtxid.isEmpty() )
      {
        MyMoneyTransaction openingtx = file->transaction(openingtxid);
        MyMoneySplit split = openingtx.splitByAccount(m_account.id());
        
        if ( split.shares() != balance )
        {
          if ( KMessageBox::questionYesNo(
            qApp->mainWidget(),
            i18n("The %1 account currently has an opening balance of %2. This QIF file reports an opening balance of %3. Would you like to overwrite the current balance with the one from the QIF file?").arg(m_account.name(),split.shares().formatMoney(),balance.formatMoney()),
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
    } else {
      selectOrCreateAccount(Select, m_account);
      if ( m_entryType == EntryInvestmentTransaction )
        processInvestmentTransactionEntry();
      else
        processTransactionEntry();
    }
  }
}

void MyMoneyQifReader::processCategoryEntry(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QString name = extractLine('N');

  if(!extractLine('I').isEmpty()) {
    file->createCategory(file->income(), name);

  } else if(!extractLine('E').isEmpty()) {
    file->createCategory(file->expense(), name);

  } else {
    MyMoneyAccount acc;
    acc.setName(name);
    selectOrCreateAccount(Select, acc);
  }
}

void MyMoneyQifReader::processTransactionEntry(void)
{
  ++m_transactionsProcessed;
  // in case the user selected to skip the account or the account
  // was not found we skip this transaction
  if(m_account.id().isEmpty()) {
    m_transactionsSkipped++;
    return;
  }

  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyTransaction t;
  MyMoneySplit s1;
  QString tmp;
  QCString accountId;
  int pos;

  // mark it imported for the view
  t.setValue("Imported", "true");
  t.setCommodity(m_account.currencyId());

  // Process general transaction data
  t.setPostDate(m_qifProfile.date(extractLine('D')));
  if(!t.postDate().isValid()) {
    int rc = KMessageBox::warningContinueCancel(0,
         i18n("The date entry \"%1\" read from the file cannot be interpreted through the current "
              "date profile setting of \"%2\".\n\nPressing \"Continue\" will "
              "assign todays date to the transaction. Pressing \"Cancel\" will abort "
              "the import operation. You can then restart the import and select a different "
              "QIF profile or create a new one.")
           .arg(extractLine('D')).arg(m_qifProfile.dateFormat()),
         i18n("Invalid date format"));
    switch(rc) {
      case KMessageBox::Continue:
        t.setPostDate(QDate::currentDate());
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
    tmp = "";
  } else if(pos != -1) {
    t.setValue("Dialog", tmp.mid(pos+2));
    tmp = tmp.left(pos);
  }
  t.setMemo(tmp);

  // Assign the "#" field to the transaction's bank id
  // This is the custom KMM extension to QIF for a unique ID
  tmp = extractLine('#');
  if(!tmp.isEmpty()) 
  {
    t.setBankID(tmp);
  }
  
  // Collect data for the account's split
  s1.setAccountId(m_account.id());
  tmp = extractLine('S');
  pos = tmp.findRev("--");
  if(pos != -1) {
    tmp = tmp.left(pos);
  }
  s1.setMemo(tmp);
  
  // TODO Deal with currencies more gracefully.  QIF cannot deal with multiple
  // currencies, so we should assume that transactions imported into a given
  // account are in THAT ACCOUNT's currency.  If one of those involves a transfer
  // to an account with a different currency, value and shares should be
  // different.  (Shares is in the target account's currency, value is in the
  // transaction's)
  
  s1.setValue(m_qifProfile.value('T', extractLine('T')));
  s1.setShares(m_qifProfile.value('T', extractLine('T')));
  s1.setNumber(extractLine('N'));

  tmp = extractLine('P');
  if(!tmp.isEmpty()) {
    try {
      s1.setPayeeId(file->payeeByName(tmp).id());
    } catch (MyMoneyException *e) {
      MyMoneyPayee payee;
      int rc = KMessageBox::Yes;

      if(m_autoCreatePayee == false) {
        // Ask the user if that is what he intended to do?
        QString msg = i18n("Do you want to add \"%1\" as payee/receiver?\n\n").arg(tmp);
        msg += i18n("Selecting \"Yes\" will create the payee, \"No\" will skip "
                    "creation of a payee record and remove the payee information "
                    "from this transaction. Selecting \"Cancel\" aborts the import "
                    "operation.\n\nIf you select \"No\" here and mark the \"Don't ask "
                    "again\" checkbox, the payee information for all following transactions "
                    "referencing \"%1\" will be removed.").arg(tmp);

        QString askKey = QString("QIF-Import-Payee-")+tmp;
        if(!m_dontAskAgain.contains(askKey)) {
          m_dontAskAgain += askKey;
        }
        rc = KMessageBox::questionYesNoCancel(0, msg, i18n("New payee/receiver"),
                  KStdGuiItem::yes(), KStdGuiItem::no(), askKey);
      }

      if(rc == KMessageBox::Yes) {
        // for now, we just add the payee to the pool. In the future,
        // we could open a dialog and ask for all the other attributes
        // of the payee.
        payee.setName(tmp);

        try {
          file->addPayee(payee);
          s1.setPayeeId(payee.id());

        } catch(MyMoneyException *e) {
          KMessageBox::detailedSorry(0, i18n("Unable to add payee/receiver"),
            (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
          delete e;

        }

      } else if(rc == KMessageBox::No) {
        s1.setPayeeId(QCString());

      } else {
        throw new MYMONEYEXCEPTION("USERABORT");

      }
      delete e;
    }
  }

  tmp = extractLine('C');
  if(tmp == "X") {
    s1.setReconcileFlag(MyMoneySplit::Reconciled);
    s1.setReconcileDate(t.postDate());
  } else if(tmp == "*")
    s1.setReconcileFlag(MyMoneySplit::Cleared);

  if(!s1.value().isNegative())
    s1.setAction(MyMoneySplit::ActionDeposit);
  else
    s1.setAction(MyMoneySplit::ActionWithdrawal);
  s1.setMemo(extractLine('M'));

  t.addSplit(s1);

  if(extractLine('$').isEmpty()) {
    MyMoneyAccount account;
    // use the same values for the second split, but clear the ID and reverse the value
    MyMoneySplit s2 = s1;
    s2.setValue(-s1.value());
    s2.setShares(-s1.value());
    s2.setId(QCString());

    // standard transaction
    tmp = extractLine('L');
    if(tmp.left(1) == m_qifProfile.accountDelimiter().left(1)) {
      // it's a transfer, extract the account name
      tmp = tmp.mid(1, tmp.length()-2);
      accountId = file->nameToAccount(tmp);
      if(accountId.isEmpty()) {
        account.setName(tmp);
        selectOrCreateAccount(Select, account);
        accountId = account.id();
      }
      s1.setAction(MyMoneySplit::ActionTransfer);
      s2.setAction(MyMoneySplit::ActionTransfer);
      t.modifySplit(s1);

    } else {
      pos = tmp.findRev("--");
      if(pos != -1) {
        t.setValue("Dialog", tmp.mid(pos+2));
        tmp = tmp.left(pos);
      }

      // it's an expense / income
      accountId = checkCategory(tmp, s1.value(), s2.value());
    }

    if(!accountId.isEmpty()) {
      try {
        MyMoneyAccount account = file->account(accountId);
        // FIXME: check that the type matches and ask if not
        
        if ( account.accountType() == MyMoneyAccount::Investment )
        {
          kdDebug(2) << "Line " << m_linenumber << ": Cannot transfer to an investment account. Transaction ignored." << endl;
          return;
        }
        
      } catch (MyMoneyException *e) {
        kdDebug(2) << "Line " << m_linenumber << ": Account with id " << accountId.data() << " not found" << endl;
        accountId = QCString();
        delete e;
      }
    }
    
    if(!accountId.isEmpty()) {
      s2.setAccountId(accountId);
      try {
        t.addSplit(s2);
      } catch (MyMoneyException *e) {
        kdDebug(2) << "Line " << m_linenumber << ": Unable to add second split: " << e->what() << endl;
        delete e;
      }
    }
  } else {
    // splitted transaction
    int   count;

    for(count = 1; !extractLine('$', count).isEmpty(); ++count) {
      MyMoneySplit s2 = s1;
      s2.setId(QCString());
      s2.setValue(-m_qifProfile.value('$', extractLine('$', count)));
      s2.setShares(-m_qifProfile.value('$', extractLine('$', count)));
      s2.setMemo(extractLine('E', count));
      tmp = extractLine('S', count);

      if(tmp.left(1) == m_qifProfile.accountDelimiter().left(1)) {
        // it's a transfer, extract the account name
        tmp = tmp.mid(1, tmp.length()-2);
        accountId = file->nameToAccount(tmp);
        if(accountId.isEmpty()) {
          MyMoneyAccount account;
          account.setName(tmp);
          selectOrCreateAccount(Select, account);
          accountId = account.id();
        }
        s1.setAction(MyMoneySplit::ActionTransfer);
        s2.setAction(MyMoneySplit::ActionTransfer);
        t.modifySplit(s1);
      } else {
        pos = tmp.findRev("--");
        if(pos != -1) {
          t.setValue("Dialog", tmp.mid(pos+2));
          tmp = tmp.left(pos);
        }

        accountId = checkCategory(tmp, s1.value(), s2.value());
      }

      if(!accountId.isEmpty()) {
        try {
          MyMoneyAccount account = file->account(accountId);
          // FIXME: check that the type matches and ask if not
        
          if ( account.accountType() == MyMoneyAccount::Investment )
          {
            kdDebug(2) << "Line " << m_linenumber << ": Cannot transfer to an investment account. Transaction ignored." << endl;
            return;
          }
        
        } catch (MyMoneyException *e) {
          kdDebug(2) << "Line " << m_linenumber << ": Account with id " << accountId.data() << " not found" << endl;
          accountId = QCString();
          delete e;
        }
      }
      if(!accountId.isEmpty()) {
        s2.setAccountId(accountId);
        try {
          t.addSplit(s2);
        } catch (MyMoneyException *e) {
          kdDebug(2) << "Line " << m_linenumber << ": Unable to add split: " << e->what() << endl;
          delete e;
        }
      }
    }
  }

  // Add the transaction
  try {
    bool oktoadd = true;

    // first, check for duplicates by Bank ID in this account.
    MyMoneyTransactionFilter filter;
    filter.setDateFilter(t.postDate().addDays(-4), t.postDate().addDays(4));
    filter.addAccount(m_account.id());
    QValueList<MyMoneyTransaction> list = file->transactionList(filter);
    QValueList<MyMoneyTransaction>::ConstIterator it;
    for(it = list.begin(); it != list.end(); ++it) {
      if(t.bankID() == (*it).bankID() && !t.bankID().isNull() && !(*it).bankID().isNull())
      {
        oktoadd = false;
        break;
      }
    }

    // Second, if no bank-id diplicates, try using the more error-prone
    // MMTransaction::isDuplicate() method, as long as the user asked for
    // this behaviour        
    if ( oktoadd & m_qifProfile.attemptMatchDuplicates() )
    {
      for(it = list.begin(); it != list.end(); ++it) {
        if(t.isDuplicate(*it))
        {
          oktoadd = false;
          break;
        }
      }
    }

    if ( oktoadd )
    {
      m_transactionCache.push_back(t);      
    }
  } 
  catch (MyMoneyException *e) 
  {
    kdDebug(2) << "Line " << m_linenumber << ": Problem adding imported transaction: " << e->what() << endl;  
    delete e;
  }

}

void MyMoneyQifReader::processInvestmentTransactionEntry()
{
//   kdDebug(2) << "Investment Transaction:" << m_qifEntry.count() << " lines" << endl;
  /*
  Items for Investment Accounts
  Field 	Indicator Explanation
  D 	Date
  N 	Action
  Y 	Security (NAME, not symbol)
  I 	Price
  Q 	Quantity (number of shares or split ratio)
  T 	Transaction amount
  C 	Cleared status
  P 	Text in the first line for transfers and reminders (Payee)
  M 	Memo
  O 	Commission
  L 	Account for the transfer
  $ 	Amount transferred
  ^ 	End of the entry
  
  It will be presumed all transactions are to the associated cash account, if 
  one exists, unless otherwise noted by the 'L' field.

  Expense/Income categories will be automatically generated, "_Dividend",
  "_InterestIncome", etc.
  
  */

  MyMoneyFile* file = MyMoneyFile::instance();

  // ensure that the user is really importing this into an investment account
  if ( m_account.accountType() != MyMoneyAccount::Investment )
  {
    kdDebug(2) << "Line " << m_linenumber << ": Can't import investment transactions into this account." << endl;
    
    int rc = KMessageBox::warningContinueCancel(0,
         i18n("This QIF file contains investment transactions.  You are trying to import this file into "
              "an account which is not an investment account.  These transactions will be ignored."),
         i18n("Invalid account for investments"),
         KStdGuiItem::cont(),
         "InvalidAccountForInvestments");
    switch(rc) {
      case KMessageBox::Continue:
        return;

      case KMessageBox::Cancel:
        throw new MYMONEYEXCEPTION("USERABORT");
        break;
    }
  }
   
  MyMoneyTransaction t;

  // mark it imported for the view
  t.setValue("Imported", "true");

  t.setCommodity(m_account.currencyId());

  // 'D' field: Date
  QDate date = m_qifProfile.date(extractLine('D'));
  if(date.isValid())
    t.setPostDate(date);
  else
  {
    int rc = KMessageBox::warningContinueCancel(0,
         i18n("The date entry \"%1\" read from the file cannot be interpreted through the current "
              "date profile setting of \"%2\".\n\nPressing \"Continue\" will "
              "assign todays date to the transaction. Pressing \"Cancel\" will abort "
              "the import operation. You can then restart the import and select a different "
              "QIF profile or create a new one.")
           .arg(extractLine('D')).arg(m_qifProfile.dateFormat()),
         i18n("Invalid date format"));
    switch(rc) {
      case KMessageBox::Continue:
        t.setPostDate(QDate::currentDate());
        break;

      case KMessageBox::Cancel:
        throw new MYMONEYEXCEPTION("USERABORT");
        break;
    }
  }

  // 'M' field: Memo
  QString memo = extractLine('M');
  t.setMemo(memo);

  // '#' field: BankID
  QString bankid = extractLine('#');
  if ( ! bankid.isEmpty() )
    t.setBankID(bankid);

  MyMoneySplit s1;
  s1.setMemo(extractLine('M'));
  
  // 'O' field: Fees
  MyMoneyMoney fees = m_qifProfile.value('T', extractLine('O'));
  if ( ! fees.isZero() )
  {
    MyMoneySplit s;
    s.setMemo(i18n("(Fees) ") + memo);
    s.setValue(fees);
    s.setShares(fees);
    s.setAccountId(findOrCreateExpenseAccount("_Fees"));
    t.addSplit(s);
  }
  
  // 'T' field: Amount
  MyMoneyMoney amount = m_qifProfile.value('T', extractLine('T'));
  s1.setValue(amount-fees);

  // 'Y' field: Security name

  QString securityname = extractLine('Y').lower();
  if ( securityname.isEmpty() )
  {
    kdDebug(2) << "Line " << m_linenumber << ": Investment transaction without a security is not supported." << endl;
    return;
  }

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
      s1.setAccountId(*it_account);
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
    // TODO: A "SelectOrCreateAccount" interface for investments
    KMessageBox::information(0, i18n("This investment account does not contain the '%1' security.  "
                                      "Transactions involving this security will be ignored.").arg(securityname),
                                i18n("Security not found"),
                                QString("MissingSecurity%1").arg(securityname.stripWhiteSpace()));
    return;
  }
  
  // 'Q' field: Quantity
  MyMoneyMoney quantity = m_qifProfile.value('T', extractLine('Q'));

  // 'N' field: Action
  QString action = extractLine('N').lower();
  
  // remove trailing X, which seems to have no purpose (?!)
  if ( action.endsWith("x") )
    action = action.left( action.length() - 1 );

  // Whether to create a cash split for the other side of the value
  MyMoneyMoney cashsplit;
  
  if ( action == "reinvdiv" || action == "reinvlg" || action == "reinvsh" )
  {
    s1.setShares(quantity);
    s1.setAction(MyMoneySplit::ActionReinvestDividend);
    
    MyMoneySplit s2;
    s2.setMemo(memo);
    
    QString incomeaccount = QString("_") + extractLine('N');
    s2.setAccountId(findOrCreateIncomeAccount(incomeaccount));
        
    s2.setValue(-amount-fees);
    s2.setShares(-amount-fees);
    t.addSplit(s2);
  }
  else if ( action == "div" || action == "intinc" || action == "cgshort" || action == "cgmid" || action == "cglong" || action == "rtrncap" )
  {
    // Cash dividends require setting 2 splits to get all of the information
    // in.  Split #1 will be the income split, and we'll set it to the first
    // income account.  This is a hack, but it's needed in order to get the
    // amount into the transaction.

    QString incomeaccount = QString("_") + extractLine('N');
    
    s1.setAccountId(findOrCreateIncomeAccount(incomeaccount));
    s1.setValue(-amount-fees);
    s1.setShares(-amount-fees);

    // Split 2 will be the zero-amount investment split that serves to
    // mark this transaction as a cash dividend and note which stock account
    // it belongs to.
    MyMoneySplit s2;
    s2.setMemo(memo);
    s2.setValue(0);
    s2.setAction(MyMoneySplit::ActionDividend);
    s2.setAccountId(thisaccount.id());
    t.addSplit(s2);
    
    cashsplit = amount;
  }
  else if (action == "buy")
  {
    s1.setShares(quantity);
    s1.setAction(MyMoneySplit::ActionBuyShares);

    cashsplit = -amount;      
  }
  else if (action == "sell")
  {
    s1.setShares(-quantity);
    s1.setValue(-amount-fees);
    s1.setAction(MyMoneySplit::ActionBuyShares);
    
    cashsplit = amount;
  }
  else if ( action == "shrsin" )
  {
    s1.setShares(quantity);
    s1.setValue(0);
    s1.setAction(MyMoneySplit::ActionAddShares);
  }
  else if ( action == "shrsout" )
  {
    s1.setShares(-quantity);
    s1.setValue(0);
    s1.setAction(MyMoneySplit::ActionAddShares);
  }
  else if ( action == "stksplit" )
  {
    MyMoneyMoney splitfactor = (quantity / MyMoneyMoney(10.0)).reduce();
    
    // Stock splits not supported
    kdDebug(2) << "Line " << m_linenumber << ": Stock split not supported (date=" << date << " security=" << securityname << " factor=" << splitfactor.toString() << ")" << endl;
    return;
  }
  else
  {
    // Unsupported action type
    kdDebug(2) << "Line " << m_linenumber << ": Unsupported transaction action (" << action << ")" << endl;
    return;
  }

  t.addSplit(s1);

  // If there is a brokerage account, add a final split to stash the -value of the
  // transaction there, if needed based on the type of transaction

  QCString accountid;
  QString accountname = extractLine('L');
  if ( accountname.isEmpty() )
  {
    accountid = m_account.value("kmm-brokerage-account").utf8();
  }
  else
  {
    if(accountname.left(1) == m_qifProfile.accountDelimiter().left(1)) 
    {
      accountname = accountname.mid(1, accountname.length()-2);
      accountid = file->nameToAccount(accountname);
      if(accountid.isEmpty()) {
        MyMoneyAccount account;
        account.setName(accountname);
        selectOrCreateAccount(Select, account);
        accountid = account.id();
      }
    } 
    else 
    {
      // L field with income/expense categories not supported in investments
      kdDebug(2) << "Line " << m_linenumber << ": L field with income/expense categories not supported in investments" << endl;
    }
  }  
    
  if ( ! accountid.isEmpty() && ! cashsplit.isZero() )
  {
    // FIXME This may not deal with foreign currencies properly
    MyMoneySplit s;
    s.setMemo(memo);
    s.setValue(cashsplit);
    s.setShares(cashsplit);
    s.setAccountId(accountid);
    t.addSplit(s);
  }
  
  // Add the transaction
  try {
    // check for duplicates ONLY by Bank ID in this account.
    // We know Bank ID will definitely
    // find duplicates.  For "softer" duplicates, we'll wait for the full
    // matching interface.
    MyMoneyTransactionFilter filter;
    filter.setDateFilter(t.postDate().addDays(-4), t.postDate().addDays(4));
    filter.addAccount(thisaccount.id());
    QValueList<MyMoneyTransaction> list = file->transactionList(filter);
    QValueList<MyMoneyTransaction>::Iterator it;
    for(it = list.begin(); it != list.end(); ++it) {
      if(t.bankID() == (*it).bankID() && !t.bankID().isNull() && !(*it).bankID().isNull())
        break;
    }
    if(it == list.end())
    {
      m_transactionCache.push_back(t);      
    }
  } catch (MyMoneyException *e) {
  
    kdDebug(2) << "Line " << m_linenumber << ": Problem adding imported transaction: " << e->what() << endl;
    
    QString message(i18n("Problem adding imported transaction: "));
    message += e->what();
    KMessageBox::information(0, message);
    delete e;
  }
  
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
    file->addAccount( acc, income );
    result = acc.id();
  }

  return result;
}

// TODO: Combine this and the previous function

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
    MyMoneyAccount expense = file->expense();
    file->addAccount( acc, expense );
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
      account.setAccountType((!value.isNegative() && value2.isNegative()) ? MyMoneyAccount::Income : MyMoneyAccount::Expense);
      selectOrCreateAccount(Select, account);
      accountId = account.id();
    }
  }

  return accountId;
}

void MyMoneyQifReader::processAccountEntry(void)
{
  m_account = MyMoneyAccount();
  QString tmp;

  m_account.setName(extractLine('N'));
  m_account.setDescription(extractLine('D'));

  tmp = extractLine('$');
  if(tmp.length() > 0)
    m_account.setValue("lastStatementBalance", tmp);

  tmp = extractLine('/');
  if(tmp.length() > 0)
    m_account.setValue("lastStatementDate", m_qifProfile.date(tmp).toString("yyyy-MM-dd"));

  QString type = extractLine('T').lower().remove(QRegExp("\\s+"));
  if(type == m_qifProfile.profileType().lower().remove(QRegExp("\\s+"))) {
    m_account.setAccountType(MyMoneyAccount::Checkings);
  } else if(type == "ccard" || type == "creditcard") {
    m_account.setAccountType(MyMoneyAccount::CreditCard);
  } else if(type == "cash") {
    m_account.setAccountType(MyMoneyAccount::Cash);
  } else if(type == "otha") {
    m_account.setAccountType(MyMoneyAccount::Asset);
  } else if(type == "othl") {
    m_account.setAccountType(MyMoneyAccount::Liability);
  } else if(type == "invst") {
    m_account.setAccountType(MyMoneyAccount::Checkings);
    //kdDebug(2) << "Line " << m_linenumber << ": This file contains an investment account with 'bank' transactions.  KMM does not natively support this.  However, it usually works fine to treat it as a checking account." << endl;
    // I can't fathom why I originally wrote this warning, so I am taking it out for now.
    // Perhaps a !Type:Invst is used to refer to BOTH the brokerage account (which is a checkings account in KMM)
    // and the investment account.
  
  } else {
    m_account.setAccountType(MyMoneyAccount::Checkings);
    kdDebug(2) << "Line " << m_linenumber << ": Unknown account type '" << type << "', checkings assumed" << endl;
  }
  selectOrCreateAccount(Select, m_account);
}

void MyMoneyQifReader::selectOrCreateAccount(const SelectCreateMode mode, MyMoneyAccount& account)
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

  it = m_accountTranslation.find((leadIn + ":" + account.name()).lower());
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

        m_accountTranslation[(leadIn + ":" + account.name()).lower()] = accountId;

        // MMAccount::openingBalance() is where the accountSelect dialog has
        // stashed the opening balance that the user chose.
        MyMoneyAccount importedAccountData(account);
        MyMoneyMoney balance = importedAccountData.openingBalance();
        account = file->account(accountId);
        if ( ! balance.isZero() )
        {          
          QCString openingtxid = file->openingBalanceTransaction(account);
          if ( ! openingtxid.isEmpty() )
          {
            MyMoneyTransaction openingtx = file->transaction(openingtxid);
            MyMoneySplit split = openingtx.splitByAccount(account.id());
            
            if ( split.shares() != balance )
            {
              if ( KMessageBox::questionYesNo(
                qApp->mainWidget(),
                i18n("The %1 account currently has an opening balance of %2. This QIF file reports an opening balance of %3. Would you like to overwrite the current balance with the one from the QIF file?").arg(account.name(),split.shares().formatMoney(),balance.formatMoney()),
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

  MyMoneyFile* file = MyMoneyFile::instance();
  QStringList::const_iterator it_line = m_qifEntry.begin();

  // Get past the "!Type: Price" line
  if ( it_line != m_qifEntry.end() )
    ++it_line;

  QValueList<MyMoneySecurity> securities = file->securityList();
    
  // Make a price for each line
  while ( it_line != m_qifEntry.end() )
  {
    QStringList column = QStringList::split(",",*it_line,true);
    QString symbol = column[0].remove("\"").lower();
    QString pricestr = column[1];
    QString datestr = column[2].remove("\"");
    kdDebug(2) << "Price:" << column[0] << " / " << column[1] << " / " << column[2] << endl;
  
    // Only add the price if the date is valid.  If invalid, fail silently.  See note above.
    // Also require the price value to not have any slashes.  Old prices will be something like
    // "25 9/16", which we do not support.  So we'll skip the price for now.
    QDate date = m_qifProfile.date(datestr);
    if(date.isValid() && !pricestr.contains("/"))
    {
      // NOTE: Price entries are not associated with an account.  They are
      // universal in the file, so all securities in the file must be
      // searched.

      QValueList<MyMoneySecurity>::const_iterator it_security = securities.begin();
      while ( it_security != securities.end() )
      {
        QString thissymbol = (*it_security).tradingSymbol();
        thissymbol = thissymbol.lower();
        if ( thissymbol == symbol )
        {
          MyMoneyMoney pricerate(pricestr);
          MyMoneyPrice price((*it_security).id(),(*it_security).tradingCurrency(),date,pricerate,i18n("QIF Import"));
          file->addPrice(price);
          break;
        }
        ++ it_security;      
      }
      
      // if the security mentioned in this price was not found in the file
      if ( it_security == securities.end() )
      {
        // warn the user
        kdDebug(2) << "Line " << m_linenumber << ": Security " << symbol << " not found." << endl;
      }
    }
    
    ++it_line;
  }  
}

void MyMoneyQifReader::processSecurityEntry(void)
{
#if 0
  // Warn the user
  if ( ! m_warnedSecurity )
  {
    m_warnedSecurity = true;
    if ( KMessageBox::warningContinueCancel (qApp->mainWidget(), i18n("This file contains security entries.  These are not currently supported by the QIF importer."), i18n("Unable to import"), KStdGuiItem::cont(), "QIFCantImportSecurity") == KMessageBox::Cancel )
      throw new MYMONEYEXCEPTION("USERABORT");
  }
#endif
  
  /*
  !Type:Security
  NVANGUARD 500 INDEX
  SVFINX
  TMutual Fund
  ^
  */

  QString type = extractLine('T').lower();
  QString name = extractLine('N').lower();
  QString symbol = extractLine('S').lower();
  
  kdDebug(2) << "Line " << m_linenumber << ": Security (" << type << "): " << name << " (" << symbol << ")" << endl;

  // TODO: If this investment isn't already in the account, add it

  // Add it to our name-to-symbol mapping.  This will allow the investment transaction
  // importer to translate from the QIF file's security name to the symbol, which we
  // can then use to look up the security in KMM.

  m_investmentMap[name] = symbol;  
}

#include "mymoneyqifreader.moc"
