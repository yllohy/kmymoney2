/***************************************************************************
                          mymoneyqifreader.cpp  -  description
                             -------------------
    begin                : Mon Jan 27 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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
  m_pos += m_qifLine.length() + 1;
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

            // Warn the user
            if ( ! m_warnedInvestment )
            {
              m_warnedInvestment = true;
              if ( KMessageBox::warningContinueCancel (qApp->mainWidget(), i18n("This file contains investment entries.  These are not currently supported by the QIF importer."), i18n("Unable to import"), KStdGuiItem::cont(), "QIFCantImportInvestment") == KMessageBox::Cancel )
                throw new MYMONEYEXCEPTION("USERABORT");
            }
          
            m_entryType = EntryInvestmentTransaction;
            processMSAccountEntry(MyMoneyAccount::Investment);

          } else if(category == "Prices") {
            m_entryType = EntryPrice;
            processPriceEntry();
          } else
            qWarning("Unknown '!Type:%s' category", category.latin1());

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
          qWarning("Unknown '!%s' category", category.latin1());

        break;
      }

    } else {
      // Process entry of same type
      switch(m_entryType) {
        case EntryUnknown:
          qWarning("Found an entry without a type being specified. Entry skipped.");
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

        default:
          qWarning("EntryType %d not yet implemented!", m_entryType);
          break;
      }
    }
  } catch(MyMoneyException *e) {
    if(e->what() != "USERABORT") {
      qWarning("This shouldn't happen! : %s", e->what().latin1());
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

  // Collect data for the account's split
  s1.setAccountId(m_account.id());
  tmp = extractLine('S');
  pos = tmp.findRev("--");
  if(pos != -1) {
    tmp = tmp.left(pos);
  }
  s1.setMemo(tmp);
  s1.setValue(m_qifProfile.value('T', extractLine('T')));
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
      } catch (MyMoneyException *e) {
        qWarning("Account with id %s not found", accountId.data());
        accountId = QCString();
        delete e;
      }
    }
    if(!accountId.isEmpty()) {
      s2.setAccountId(accountId);
      try {
        t.addSplit(s2);
      } catch (MyMoneyException *e) {
        QString message(i18n("Unable to add second split: "));
        message += e->what();
        KMessageBox::information(0, message);
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
        } catch (MyMoneyException *e) {
          qWarning("Account with id %s not found", accountId.data());
          accountId = QCString();
          delete e;
        }
      }
      if(!accountId.isEmpty()) {
        s2.setAccountId(accountId);
        try {
          t.addSplit(s2);
        } catch (MyMoneyException *e) {
          QString message(i18n("Unable to add split: "));
          message += e->what();
          KMessageBox::information(0, message);
          delete e;
        }
      }
    }
  }

  // FIXME: here we could check if the transaction already exists
  //        and mark it as a duplicate which will be shown in a
  //        different color after importing

  // Add the transaction
  try {
    bool oktoadd = false;
    
    if ( m_qifProfile.attemptMatchDuplicates() )
    {
      // First, do the duplicate check
      QValueList<MyMoneyTransaction> list;
      MyMoneyTransactionFilter filter;
      filter.setDateFilter(t.postDate().addDays(-4), t.postDate().addDays(4));
      list = file->transactionList(filter);
  
      QValueList<MyMoneyTransaction>::Iterator it;
      for(it = list.begin(); it != list.end(); ++it) {
        if(t.isDuplicate(*it))
          break;
      }
      if(it == list.end()) {
        oktoadd = true;
      }
    }
    else
      oktoadd = true;

    if ( oktoadd )     
      file->addTransaction(t);
    
  } catch (MyMoneyException *e) {
    QString message(i18n("Problem adding imported transaction: "));
    message += e->what();
    KMessageBox::information(0, message);
    delete e;
  }

}

void MyMoneyQifReader::processInvestmentTransactionEntry()
{
  kdDebug(2) << "Investment Transaction:" << m_qifEntry.count() << " lines" << endl;
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
  
  */
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
  } else {
    m_account.setAccountType(MyMoneyAccount::Checkings);
    qWarning("Unknown account type '%s', checkings assumed", type.latin1());
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
  // Warn the user
  if ( ! m_warnedPrice )
  {
    m_warnedPrice = true;
    if ( KMessageBox::warningContinueCancel (qApp->mainWidget(), i18n("This file contains price entries.  These are not currently supported by the QIF importer."), i18n("Unable to import"), KStdGuiItem::cont(), "QIFCantImportPrice") == KMessageBox::Cancel )
      throw new MYMONEYEXCEPTION("USERABORT");
  }

  QStringList::const_iterator it_line = m_qifEntry.begin();

  // Get past the "!Type: Price" line
  if ( it_line != m_qifEntry.end() )
    ++it_line;

  // Make a price for each line
  while ( it_line != m_qifEntry.end() )
  {
    QStringList columns = QStringList::split(",",*it_line,true);
    kdDebug(2) << "Price:" << columns[0] << " / " << columns[1] << " / " << columns[2] << endl;
  
    // TODO Add this price

    ++it_line;
  }  
}

void MyMoneyQifReader::processSecurityEntry(void)
{
  // Warn the user
  if ( ! m_warnedSecurity )
  {
    m_warnedSecurity = true;
    if ( KMessageBox::warningContinueCancel (qApp->mainWidget(), i18n("This file contains security entries.  These are not currently supported by the QIF importer."), i18n("Unable to import"), KStdGuiItem::cont(), "QIFCantImportSecurity") == KMessageBox::Cancel )
      throw new MYMONEYEXCEPTION("USERABORT");
  }
  
  // TODO Implement security
  QString type = extractLine('T');
  QString name = extractLine('N');
  QString symbol = extractLine('S');
  
  kdDebug(2) << "Security (" << type << "): " << name << " (" << symbol << ")" << endl;

  /*
  !Type:Security      throw new MYMONEYEXCEPTION("USERABORT");

  NVANGUARD 500 INDEX
  SVFINX
  TMutual Fund
  ^
  */
}
