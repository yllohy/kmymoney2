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

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyqifreader.h"
#include "../mymoney/mymoneyfile.h"
#include "../dialogs/kaccountselectdlg.h"
#include "../kmymoney2.h"

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

  connect(&m_filter, SIGNAL(wroteToStdin()), this, SLOT(slotSendDataToFilter()));
  connect(&m_filter, SIGNAL(readyReadStdout()), this, SLOT(slotReceivedDataFromFilter()));
  connect(&m_filter, SIGNAL(processExited()), this, SLOT(slotImportFinished()));
  connect(&m_filter, SIGNAL(readyReadStderr()), this, SLOT(slotReceivedErrorFromFilter()));
}

MyMoneyQifReader::~MyMoneyQifReader()
{
  if(m_file)
    delete m_file;
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
  QByteArray data(0);
    
  if(m_file->atEnd()) {
    m_filter.flushStdin();
    m_filter.closeStdin();
  } else {
    len = m_file->readBlock(m_buffer, sizeof(m_buffer));
    if(len == -1) {
      qWarning("Failed to read block from QIF import file");
      m_filter.kill();
      m_filter.closeStdin();
    } else {
      data.setRawData(m_buffer, len);
      m_filter.writeToStdin(data);
      m_filter.flushStdin();
      data.resetRawData(m_buffer, len);
    }
  }
}

void MyMoneyQifReader::slotReceivedErrorFromFilter(void)
{
  QString errmsg;

  while(m_filter.canReadLineStderr()) {
    errmsg = m_filter.readLineStderr();
    qWarning(errmsg);
  }
}

void MyMoneyQifReader::slotReceivedDataFromFilter(void)
{
  if(!m_processingData) {
    m_processingData = true;
    if(m_filter.isRunning() || m_filter.normalExit()) {
      while(m_filter.canReadLineStdout() && !m_userAbort) {
        // slotReceivedErrorFromFilter();
        m_qifLine = m_filter.readLineStdout();
        m_pos += m_qifLine.length() + 1;
        while(m_qifLine.endsWith(" ") || m_qifLine.endsWith("\t"))
          m_qifLine = m_qifLine.left(m_qifLine.length()-1);

        // skip empty lines
        if(!m_qifLine.isEmpty()) {

          if(m_qifLine == "^") {

            // skip empty entries
            if(m_qifEntry.count() != 0) {
              processQifEntry();
              m_qifEntry.clear();
            }
          } else {
            m_qifEntry += m_qifLine;
          }
        }
        signalProgress(m_pos, 0);
        // qApp->processEvents(20);
      }
    }
    
    if(!m_filter.isRunning() || m_userAbort) {
      emit importFinished();
    }
    
    m_processingData = false;
  }
}

void MyMoneyQifReader::slotImportFinished(void)
{
  slotReceivedDataFromFilter();
  slotReceivedErrorFromFilter();
  if(!m_filter.normalExit()) {
    emit importFinished();
  }
}

const bool MyMoneyQifReader::startImport(void)
{
  bool rc = false;
  
  m_dontAskAgain.clear();
  m_accountTranslation.clear();
  m_userAbort = false;
  m_pos = 0;
  
  m_file = new QFile(m_filename);
  if(m_file->open(IO_ReadOnly)) {
    m_filter.setCommunication(QProcess::Stdin | QProcess::Stdout | QProcess::Stderr);
    
    // start filter process, use 'cat -' as the default filter
    m_filter.clearArguments();
    if(m_qifProfile.filterScriptImport().isEmpty()) {
      m_filter.addArgument("cat");
      m_filter.addArgument("-");
    } else {
      m_filter.setArguments(QStringList::split(" ", m_qifProfile.filterScriptImport(), true));
    }
    m_entryType = EntryUnknown;
    
    if(m_filter.start()) {
      signalProgress(0, m_file->size(), "Importing QIF ...");
      slotSendDataToFilter();
      rc = true;
    } else {
      qDebug("starting filter failed :-(");
    }
  }
  return rc;
}

const bool MyMoneyQifReader::finishImport(void)
{
  bool  rc = false;
  
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
  return rc;
}

void MyMoneyQifReader::processQifEntry(void)
{
  try {
    QString category = extractLine('!');
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

          } else
            qWarning("Unknown '!Type:%s' category", category.latin1());

        } else if(category == "Account") {
          m_account = MyMoneyAccount();
          processAccountEntry();

        } else if(category == "Option:AutoSwitch") {
          m_entryType = EntryAccount;
          category = extractLine('!', 2);    // is there another record?
          continue;

        } else if(category == "Clear:AutoSwitch") {
          m_entryType = EntryTransaction;
          category = extractLine('!', 2);    // is there another record?
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

        case EntryAccount:
          m_account = MyMoneyAccount();
          processAccountEntry();
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
      m_filter.tryTerminate();
      QTimer::singleShot(1000, &m_filter, SLOT(kill()));
      m_userAbort = true;
    }
    delete e;
  }
}

const QString MyMoneyQifReader::extractLine(const QChar id, int cnt) const
{
  QStringList::ConstIterator it;

  for(it = m_qifEntry.begin(); it != m_qifEntry.end(); ++it) {
    if((*it)[0] == id) {
      if(cnt-- == 1) {
        return (*it).mid(1);
      }
    }
  }
  return QString();
}

void MyMoneyQifReader::processMSAccountEntry(const MyMoneyAccount::accountTypeE accountType)
{
  m_account = MyMoneyAccount();
  if(extractLine('P') == m_qifProfile.openingBalanceText()) {
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
    m_account.setAccountType(accountType);
    selectOrCreateAccount(Select, m_account);
    
  } else {
    selectOrCreateAccount(Select, m_account);
    processTransactionEntry();
  }
}

void MyMoneyQifReader::processCategoryEntry(void)
{
  qWarning("MyMoneyQifReader::processCategoryEntry not yet implemented");
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
  
  // Process general transaction data
  t.setPostDate(m_qifProfile.date(extractLine('D')));

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

      // Ask the user if that is what he intended to do?
      QString msg = i18n("Do you want to add '%1' as payee/receiver? ").arg(tmp);
      msg += i18n("If you select the 'Don't ask again' box, you will not be asked for this"
                  " payee again during the current import in case you select 'No'."
                  " 'Cancel' aborts the import and leaves the data unchanged.");

      QString askKey = QString("QIF-Import-Payee-")+tmp;
      if(!m_dontAskAgain.contains(askKey)) {
        m_dontAskAgain += askKey;
      }
      int rc = KMessageBox::questionYesNoCancel(0, msg, i18n("New payee/receiver"),
                  KStdGuiItem::yes(), KStdGuiItem::no(), askKey);
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
        s1.setPayeeId("");
        
      } else {
        throw new MYMONEYEXCEPTION("USERABORT");
        
      }
      delete e;
    }
  }
  
  tmp = extractLine('C');
  if(tmp == "X")
    s1.setReconcileFlag(MyMoneySplit::Reconciled);
  else if(tmp == "*")
    s1.setReconcileFlag(MyMoneySplit::Cleared);

  if(file->accountGroup(m_account.accountType()) == MyMoneyAccount::Asset) {
    if(s1.value() >= 0)
      s1.setAction(MyMoneySplit::ActionDeposit);
    else
      s1.setAction(MyMoneySplit::ActionWithdrawal);
  } else {
    if(s1.value() >= 0)
      s1.setAction(MyMoneySplit::ActionWithdrawal);
    else
      s1.setAction(MyMoneySplit::ActionDeposit);
  }
  s1.setMemo(extractLine('M'));
  
  t.addSplit(s1);

  if(extractLine('$').isEmpty()) {
    MyMoneyAccount account;
    // use the same values for the second split, but clear the ID and reverse the value
    MyMoneySplit s2 = s1;
    s2.setValue(-s1.value());
    s2.setId("");
    
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
      accountId = checkCategory(tmp, s1.value());
    }

    if(!accountId.isEmpty()) {
      try {
        MyMoneyAccount account = file->account(accountId);
        // FIXME: check that the type matches and ask if not
      } catch (MyMoneyException *e) {
        qWarning("Account with id %s not found", accountId.data());
        accountId = "";
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
      s2.setId("");
      s2.setValue(-m_qifProfile.value('$', extractLine('$', count)));
      s2.setMemo(extractLine('E', count));
      tmp = extractLine('S', count);
      pos = tmp.findRev("--");
      if(pos != -1) {
        t.setValue("Dialog", tmp.mid(pos+2));
        tmp = tmp.left(pos);
      }
      
      accountId = checkCategory(tmp, s1.value());
      if(!accountId.isEmpty()) {
        try {
          MyMoneyAccount account = file->account(accountId);
          // FIXME: check that the type matches and ask if not
        } catch (MyMoneyException *e) {
          qWarning("Account with id %s not found", accountId.data());
          accountId = "";
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
    file->addTransaction(t);
  } catch (MyMoneyException *e) {
    QString message(i18n("Problem adding imported transaction: "));
    message += e->what();
    KMessageBox::information(0, message);
    delete e;
  }
  
}

const QCString MyMoneyQifReader::checkCategory(const QString& name, const MyMoneyMoney value)
{
  QCString accountId = "";
  MyMoneyFile *file = MyMoneyFile::instance();
  MyMoneyAccount account;
  
  if(!name.isEmpty()) {
    accountId = file->categoryToAccount(name);
    if(accountId.isEmpty()) {
      // qDebug("Category '%s' not found", name.latin1());
      // qDebug("Parent is '%s'", file->parentName(name).latin1());
      accountId = file->categoryToAccount(file->parentName(name));

      if(!accountId.isEmpty()) {
        account.setParentAccountId(accountId);

        int pos = name.find(':');
        if(pos != -1) {
          account.setName(name.mid(pos+1));
        } else
          account.setName(name);
      } else
        account.setName(name);
      
      if(value >= 0)
        account.setAccountType(MyMoneyAccount::Income);
      else
        account.setAccountType(MyMoneyAccount::Expense);

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
    
  QString type = extractLine('T');
  if(type == m_qifProfile.profileType()) {
    m_account.setAccountType(MyMoneyAccount::Checkings);
  } else if(type == "CCard") {
    m_account.setAccountType(MyMoneyAccount::CreditCard);
  } else if(type == "Cash") {
    m_account.setAccountType(MyMoneyAccount::Cash);
  } else if(type == "Oth A") {
    m_account.setAccountType(MyMoneyAccount::Asset);
  } else if(type == "Oth L") {
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
  KMyMoneyUtils::categoryTypeE type;

  QMap<QString, QCString>::ConstIterator it;
  
  it = m_accountTranslation.find(account.name());
  if(it != m_accountTranslation.end()) {
    try {
      account = file->account(*it);
      return;
      
    } catch (MyMoneyException *e) {
      QString message(i18n("Account '%1' disappeard: ").arg(account.name()));
      message += e->what();
      KMessageBox::error(0, message);
      delete e;
    }
  }

  type = KMyMoneyUtils::none;
  switch(file->accountGroup(account.accountType())) {
    default:
      type = KMyMoneyUtils::asset;
      // tricky fall through here

    case MyMoneyAccount::Liability:
      type = (KMyMoneyUtils::categoryTypeE) (type | KMyMoneyUtils::liability);
      typeStr = i18n("account");
      break;
      
    case MyMoneyAccount::Asset:
      type = KMyMoneyUtils::asset;
      typeStr = i18n("account");
      break;
      
    case MyMoneyAccount::Income:
      type = KMyMoneyUtils::income;
      typeStr = i18n("category");
      break;
      
    case MyMoneyAccount::Expense:
      type = KMyMoneyUtils::expense;
      typeStr = i18n("category");
      break;
  }

  KAccountSelectDlg accountSelect(type,"QifImport", kmymoney2);
  
  if(m_account.name().length() != 0) {
    accountId = file->nameToAccount(account.name());
    if(mode == Create) {
      if(accountId.length() != 0) {
        if(mode == Create) {
          account = file->account(accountId);
          return;
        }

      } else {
        switch(KMessageBox::questionYesNo(0,
                  i18n("The account '%1' does not exist. Do you "
                       "want to create it?").arg(account.name()))) {
          case KMessageBox::Yes:
            break;
          case KMessageBox::No:
            return;
        }
      }
    } else {
      if(accountId.length() != 0) {
        msg = i18n("The %1 <b>%2</b> currently exists. Do you want "
                   "to import transactions to this account?")
                    .arg(typeStr).arg(account.name());

      } else {
        msg = i18n("The %1 <b>%2</b> currently does not exist. You can "
                   "create a new account by pressing the Create button. "
                   "or select another account manually "
                   "from the selection box.").arg(typeStr).arg(account.name());
      }
    }
  } else {
    msg = i18n("No %1 information has been found in the selected QIF file."
               "Please select an account using the selection box in the dialog or "
               "create a new account by pressing the Create button.")
               .arg(typeStr);
  }

  accountSelect.setDescription(msg);
  accountSelect.setHeader(i18n("Account to import transactions to"));
  accountSelect.setAccount(account);
  accountSelect.setMode(mode == Create);
  accountSelect.showAbortButton(true);
  
  if(accountSelect.exec() == QDialog::Accepted) {
    if((type & KMyMoneyUtils::asset)
    || (type & KMyMoneyUtils::liability)) {
      accountId = file->nameToAccount(accountSelect.selectedAccount());
      
    } else if((type & KMyMoneyUtils::income)
           || (type & KMyMoneyUtils::expense)) {
      accountId = file->categoryToAccount(accountSelect.selectedAccount());
    } else {
        qWarning("No account selected!!!!");
    }
    
    if(accountId.length() != 0) {
      m_accountTranslation[account.name()] = accountId;
      account = file->account(accountId);
      
    } else
      account = MyMoneyAccount();
    
  } else {
    account = MyMoneyAccount();
    if(accountSelect.aborted())
      throw new MYMONEYEXCEPTION("USERABORT");
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

