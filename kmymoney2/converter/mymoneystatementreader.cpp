/***************************************************************************
                          mymoneystatementreader.cpp
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

// ----------------------------------------------------------------------------
// QT Headers

#include <qfile.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qtextedit.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneystatementreader.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneystatement.h"
#include "../dialogs/kaccountselectdlg.h"
#include "../kmymoney2.h"

MyMoneyStatementReader::MyMoneyStatementReader()
{
  m_progressCallback = 0;
  m_userAbort = false;
  m_autoCreatePayee = false;
}

MyMoneyStatementReader::~MyMoneyStatementReader()
{
}

void MyMoneyStatementReader::setAutoCreatePayee(const bool create)
{
  m_autoCreatePayee = create;
}

const bool MyMoneyStatementReader::startImport(const MyMoneyStatement& s)
{
  bool result = false;
  
  //
  // Select the account
  //
  
  m_account = MyMoneyAccount();

  m_account.setName(s.m_strAccountName);
  m_account.setNumber(s.m_strAccountNumber);
  m_account.setValue("lastStatementBalance", QString::number(s.m_moneyClosingBalance));

  if ( s.m_dateEnd.isValid() )
    m_account.setValue("lastStatementDate", s.m_dateEnd.toString("yyyy-MM-dd"));

  // TODO: Make this a statement property
  m_account.setAccountType(MyMoneyAccount::Checkings);
    
  selectOrCreateAccount(Select, m_account);
  
  //  
  // Process the transactions
  //
  
  QValueList<MyMoneyStatement::Transaction>::const_iterator it_t = s.m_listTransactions.begin();
  while ( it_t != s.m_listTransactions.end() )
  {
    processTransactionEntry(*it_t);
    ++it_t;
  }  
  
  emit importFinished();
  return result;
}

const bool MyMoneyStatementReader::finishImport(void)
{
  bool  rc = false;

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
  
  return rc;
}

void MyMoneyStatementReader::processTransactionEntry(const MyMoneyStatement::Transaction& t_in)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyTransaction t;
  
  // mark it imported for the view
  t.setValue("Imported", "true");
  
  // TODO: We can get the commodity from the statement!!
  // Although then we would need UI to 
  t.setCommodity(m_account.currencyId());

  t.setPostDate(t_in.m_datePosted);
  t.setMemo(t_in.m_strMemo);

  MyMoneySplit s1;

  s1.setMemo(t_in.m_strMemo);
  s1.setValue(t_in.m_moneyAmount);
  s1.setNumber(t_in.m_strNumber);
  s1.setAccountId(m_account.id());
  
  if(s1.value() >= 0)
    s1.setAction(MyMoneySplit::ActionDeposit);
  else
    s1.setAction(MyMoneySplit::ActionWithdrawal);

  QString payeename = t_in.m_strPayee;
  if(!payeename.isEmpty()) 
  {
    try {
      s1.setPayeeId(file->payeeByName(payeename).id());
    } 
    catch (MyMoneyException *e) 
    {
      MyMoneyPayee payee;
      int rc = KMessageBox::Yes;

      if(m_autoCreatePayee == false) {
        // Ask the user if that is what he intended to do?
        QString msg = i18n("Do you want to add \"%1\" as payee/receiver?\n\n").arg(payeename);
        msg += i18n("Selecting \"Yes\" will create the payee, \"No\" will skip "
                    "creation of a payee record and remove the payee information "
                    "from this transaction. Selecting \"Cancel\" aborts the import "
                    "operation.\n\nIf you select \"No\" here and mark the \"Don't ask "
                    "again\" checkbox, the payee information for all following transactions "
                    "referencing \"%1\" will be removed.").arg(payeename);

        QString askKey = QString("Statement-Import-Payee-")+payeename;
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
        payee.setName(payeename);

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
  
  t.addSplit(s1);
  
  // Add the transaction
  try {
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
      file->addTransaction(t);
    }
  } catch (MyMoneyException *e) {
    QString message(i18n("Problem adding imported transaction: "));
    message += e->what();
    KMessageBox::information(0, message);
    delete e;
  }

}

void MyMoneyStatementReader::selectOrCreateAccount(const SelectCreateMode mode, MyMoneyAccount& account)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QCString accountId;
  QString msg;
  QString typeStr;
  QString leadIn;
  KMyMoneyUtils::categoryTypeE type;

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

  KAccountSelectDlg accountSelect(type, "StatementImport", kmymoney2);
  if(!msg.isEmpty())
    accountSelect.setCaption(msg);
    
  // TODO: Prompt the user to set the account number or comment such that we'll
  // be able to find it again next time.

  QMap<QString, QCString>::ConstIterator it = m_accountTranslation.find((leadIn + ":" + account.name()).lower());
  if(it != m_accountTranslation.end()) {
    try {
      account = file->account(*it);
      m_account = account;
      return;

    } catch (MyMoneyException *e) {
      QString message(i18n("Account \"%1\" disappeard: ").arg(account.name()));
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
      
        QString number;
        if ( ! account.number().isEmpty() )
          number = QString(" [%1]").arg(account.number());
        msg = i18n("The %1 <b>%2</b> currently does not exist. You can "
                   "create a new %3 by pressing the <b>Create</b> button "
                   "or select another %4 manually from the selection box.")
                  .arg(typeStr).arg(account.name()+number).arg(typeStr).arg(typeStr);
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

  for(;;) {
    if(accountSelect.exec() == QDialog::Accepted) {
      if(!accountSelect.selectedAccount().isEmpty()) {
        accountId = accountSelect.selectedAccount();

        m_accountTranslation[(leadIn + ":" + account.name()).lower()] = accountId;
        MyMoneyAccount importedAccountData(account);
        account = file->account(accountId);
        if(typeStr == i18n("account")
        && importedAccountData.openingBalance() != account.openingBalance()) {
          if(KMessageBox::questionYesNo(0, i18n("Do you want to override the opening balance of this account currently set to %1 with %2?")
              .arg(account.openingBalance().formatMoney())
              .arg(importedAccountData.openingBalance().formatMoney())
            ,QString(),KStdGuiItem::yes(),KStdGuiItem::no(),"replaceopeningbalance") == KMessageBox::Yes) {
            account.setOpeningBalance(importedAccountData.openingBalance());
            MyMoneyFile::instance()->modifyAccount(account);
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

void MyMoneyStatementReader::setProgressCallback(void(*callback)(int, int, const QString&))
{
  m_progressCallback = callback;
}

void MyMoneyStatementReader::signalProgress(int current, int total, const QString& msg)
{
  if(m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}

