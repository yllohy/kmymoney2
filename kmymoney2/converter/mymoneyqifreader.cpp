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

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyqifreader.h"
#include "../mymoney/mymoneyfile.h"
#include "../dialogs/kaccountselectdlg.h"

MyMoneyQifReader::MyMoneyQifReader() :
  m_tempFile(QString::null, "qif")
{
  m_tempFile.close();
  m_tempFile.setAutoDelete(true);
  m_skipAccount = false;
}

MyMoneyQifReader::~MyMoneyQifReader()
{
}

void MyMoneyQifReader::setFilename(const QString& name)
{
  m_originalFilename = name;
  runFilter();
}

void MyMoneyQifReader::setProfile(const QString& profile)
{
  m_qifProfile.loadProfile("Profile-" + profile);
  runFilter();
}

void MyMoneyQifReader::runFilter(void)
{
/*
  if(m_qifProfile.filterScript() != "") {
    m_filename = m_tempFile.name();
    
  } else
*/
    m_filename = m_originalFilename;
}

void MyMoneyQifReader::import(void)
{
  enum type {
     EntryUnknown = 0,
     EntryAccount,
     EntryTransaction,
     EntryCategory,
     EntryMemorizedTransaction
  };
  int entryType = EntryUnknown;

  QFile qifFile(m_filename);
  QStringList entry;
  if(qifFile.open(IO_ReadOnly)) {
    QTextStream s(&qifFile);
    while(!s.atEnd()) {
      // get an entry
      entry = readEntry(s);
      QString category = extractLine('!', entry);
      if(!category.isEmpty()) {
        
        while(!category.isEmpty()) {
          if(category.left(5) == "Type:") {

            category = category.mid(5);
            entryType = EntryTransaction;
            if(category == m_qifProfile.profileType()) {
              processMSAccountEntry(entry, MyMoneyAccount::Checkings);

            } else if(category == "CCard") {
              processMSAccountEntry(entry, MyMoneyAccount::CreditCard);

            } else if(category == "Cash") {
              processMSAccountEntry(entry, MyMoneyAccount::Cash);

            } else if(category == "Oth A") {
              processMSAccountEntry(entry, MyMoneyAccount::Asset);

            } else if(category == "Oth L") {
              processMSAccountEntry(entry, MyMoneyAccount::Liability);

            } else if(category == "Cat") {
              entryType = EntryCategory;
              processCategoryEntry(entry);

            } else if(category == "Memorized") {
              entryType = EntryMemorizedTransaction;

            } else
              qWarning("Unknown '!Type:%s' category", category.latin1());

          } else if(category == "Account") {
            m_account = MyMoneyAccount();
            processAccountEntry(entry);

          } else if(category == "Option:AutoSwitch") {
            entryType = EntryAccount;
            category = extractLine('!', entry, 2);    // is there another record?
            continue;

          } else if(category == "Clear:AutoSwitch") {
            entryType = EntryTransaction;
            category = extractLine('!', entry, 2);    // is there another record?
            continue;

          } else
            qWarning("Unknown '!%s' category", category.latin1());

          break;
        }
                  
      } else {
        // Process entry of same type
        switch(entryType) {
          case EntryUnknown:
            qWarning("Found an entry without a type being specified. Entry skipped.");
            break;
            
          case EntryCategory:
            processCategoryEntry(entry);
            break;
            
          case EntryTransaction:
            processTransactionEntry(entry);
            break;

          case EntryAccount:
            m_account = MyMoneyAccount();
            processAccountEntry(entry);
            break;
                        
          default:
            qDebug("EntryType %d not yet implemented!", entryType);
            break;
        }
      }
    }
  }  
}

const QString MyMoneyQifReader::scanFileForAccount(void)
{
  QString rc;
/*
  QFile qifFile(m_filename);
  QStringList entry;
  if(qifFile.open(IO_ReadOnly)) {
    QTextStream s(&qifFile);
    while(!s.atEnd()) {
      // get an entry
      entry = readEntry(s);
      QString category = extractLine('!', entry);
      if(!category.isEmpty()) {
        if(category.left(5) == "Type:") {
          if(category.mid(5) == m_qifProfile.profileType()) {
            processMSAccountEntry(entry);
            if(!m_account.name().isEmpty()) {
              rc = m_account.name();
              break;
            }
          }    
        } else if(category == "Cat") {
          
        } else if(category == "Memorized") {
          
        } else if(category == "Account") {
          processAccountEntry(entry);
          if(!m_account.name().isEmpty()) {
            rc = m_account.name();
            break;
          }
        } else
          qDebug("Unknown '!%s' category", category.latin1());
          
      }
    }
    qifFile.close();
  }
*/  
  return rc;
}

const QStringList MyMoneyQifReader::readEntry(QTextStream& s) const
{
  QStringList entry;
  while(!s.atEnd()) {
    QString line = s.readLine();
    if(line[0] == '^')
      break;
    // truncate trailing blanks/tabs
    while(line.endsWith(" ") || line.endsWith("\t"))
      line = line.left(line.length()-1);
    entry += line;
  }
  return entry;
}

const QString MyMoneyQifReader::extractLine(const QChar id, const QStringList& lines, int cnt) const
{
  QStringList::ConstIterator it;

  for(it = lines.begin(); it != lines.end(); ++it) {
    if((*it)[0] == id) {
      if(cnt-- == 1) {
        return (*it).mid(1);
      }
    }
  }
  return QString();
}

void MyMoneyQifReader::processMSAccountEntry(const QStringList& lines, const MyMoneyAccount::accountTypeE accountType)
{
  m_account = MyMoneyAccount();
  if(extractLine('P', lines) == m_qifProfile.openingBalanceText()) {
    QString txt = extractLine('T', lines);
    MyMoneyMoney balance = m_qifProfile.value('T', txt);
    m_account.setOpeningBalance(balance);

    QDate date = m_qifProfile.date(extractLine('D', lines));
    m_account.setOpeningDate(date);

    QString name = extractLine('L', lines);
    if(name.left(1) == m_qifProfile.accountDelimiter().left(1)) {
      name = name.mid(1, name.length()-2);
    }
    m_account.setName(name);
    m_account.setAccountType(accountType);
    selectOrCreateAccount(Create);
    
  } else {
    selectOrCreateAccount(Select);
    // process the data as transaction but first we have to select an account
  }
}

void MyMoneyQifReader::processCategoryEntry(const QStringList& lines)
{
}

void MyMoneyQifReader::processTransactionEntry(const QStringList& lines)
{
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

  // Process general transaction data
  t.setPostDate(m_qifProfile.date(extractLine('D', lines)));
  t.setMemo(extractLine('L', lines));

  // Collect data for the account's split
  s1.setAccountId(m_account.id());
  s1.setMemo(extractLine('S', lines));
  s1.setValue(m_qifProfile.value('T', extractLine('T', lines)));
  s1.setNumber(extractLine('N', lines));

  tmp = extractLine('P', lines);
  try {
    s1.setPayeeId(file->payeeByName(tmp).id());
  } catch (MyMoneyException *e) {
    // FIXME: Payee does not exist, ask if it should be created or left blank
    delete e;
  }

  tmp = extractLine('C', lines);
  if(tmp == "X")
    s1.setReconcileFlag(MyMoneySplit::Reconciled);
  else if(tmp == "*")
    s1.setReconcileFlag(MyMoneySplit::Cleared);

  t.addSplit(s1);

  if(extractLine('$', lines).isEmpty()) {
    MyMoneySplit s2 = s1;
    s2.setValue(-s1.value());

    // standard transaction
    tmp = extractLine('L', lines);
    if(tmp.left(1) == m_qifProfile.accountDelimiter().left(1)) {
      // it's a transfer, extract the account name
      tmp = tmp.mid(1, tmp.length()-2);

    } else {
      // it's an expense / income
      accountId = file->categoryToAccount(tmp);
      if(accountId.isEmpty()) {
        qDebug("Category '%s' not found", tmp.latin1());
        // FIXME: selectOrCreateCategory
        //        set accountId to id if selected
      }

      if(!accountId.isEmpty()) {
        try {
          MyMoneyAccount account = file->account(accountId);
          // FIXME: check that the type matches and ask if not
          s2.setAccountId(accountId);
          t.addSplit(s2);
        } catch (MyMoneyException *e) {
          qWarning("Account with id %s not found", accountId.data());
          delete e;
        }
      }
    }
  } else {
    // splitted transaction
  }

  // FIXME: now check if the transaction already exists and mark it appropriately

  // Add the transaction
  try {
    file->addTransaction(t);
  } catch (MyMoneyException *e) {
    qWarning("Problem while adding imported transaction");
    delete e;
  }
  
}

void MyMoneyQifReader::processAccountEntry(const QStringList& lines)
{
  m_account = MyMoneyAccount();
  QString tmp;
  
  m_account.setName(extractLine('N', lines));
  m_account.setDescription(extractLine('D', lines));
  
  tmp = extractLine('$', lines);
  if(tmp.length() > 0)
    m_account.setValue("lastStatementBalance", tmp);
    
  tmp = extractLine('/', lines);
  if(tmp.length() > 0)
    m_account.setValue("lastStatementDate", m_qifProfile.date(tmp).toString("yyyy-MM-dd"));
    
  QString type = extractLine('T', lines);
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
    qDebug("Unknown account type '%s', checkings assumed", type.latin1());
  }
  selectOrCreateAccount(Create);
}

void MyMoneyQifReader::selectOrCreateAccount(SelectCreateMode mode)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  
  QCString accountId;
  QString msg;
  KMyMoneyUtils::categoryTypeE type;

  switch(file->accountGroup(m_account.accountType())) {
    case MyMoneyAccount::Liability:
      type = KMyMoneyUtils::liability;
      break;
      
    default:
    case MyMoneyAccount::Asset:
      type = KMyMoneyUtils::asset;
      break;
      
    case MyMoneyAccount::Income:
      type = KMyMoneyUtils::income;
      break;
      
    case MyMoneyAccount::Expense:
      type = KMyMoneyUtils::expense;
      break;
  }
  
  KAccountSelectDlg accountSelect(type,"QifImport");
  
  if(m_account.name().length() != 0) {
    accountId = file->nameToAccount(m_account.name());
    if(mode == Create) {
      if(accountId.length() != 0) {
        if(mode == Create) {
          m_account = file->account(accountId);
          return;
        }

      } else {
        switch(KMessageBox::questionYesNo(0,
                  i18n("The account '%1' does not exist. Do you "
                       "want to create it?").arg(m_account.name()))) {
          case KMessageBox::Yes:
            break;
          case KMessageBox::No:
            return;
        }
      }
    } else {
      if(accountId.length() != 0) {
        msg = i18n("The account '%1' currently exists. Do you want "
                   "to import transactions to this account?").arg(m_account.name());

      } else {
        msg = i18n("The account '%1' currently does not exist. You can "
                   "create a new account by pressing the Create button. "
                   "or select another account manually "
                   "from the selection box.").arg(m_account.name());
      }
    }
  } else {
    msg = i18n("No account information has been found in the selected QIF file."
               "Please select an account using the selection box in the dialog or "
               "create a new account by pressing the Create button.");
  }

  accountSelect.setDescription(msg);
  accountSelect.setHeader(i18n("Account to import transactions to"));
  accountSelect.setAccount(m_account);
  accountSelect.setMode(mode == Create);
 
  if(accountSelect.exec() == QDialog::Accepted) {
    accountId = file->nameToAccount(accountSelect.selectedAccount());
    if(accountId.length() != 0) {
      m_account = file->account(accountId);
      
    } else
      m_account = MyMoneyAccount();
    
  } else
    m_account = MyMoneyAccount();
}

