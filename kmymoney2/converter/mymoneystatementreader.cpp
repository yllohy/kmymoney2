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
  // right now, it's always true that the import started successfully.
  // if there are problems, we'll report them in the result code to
  // finishImport, just so the caller can handle the result in the
  // same place.
  bool result = true;

  //
  // Select the account
  //

  m_account = MyMoneyAccount();

  m_account.setName(s.m_strAccountName);
  m_account.setNumber(s.m_strAccountNumber);
  m_account.setValue("lastStatementBalance", QString::number(s.m_moneyClosingBalance));

  if ( s.m_dateEnd.isValid() )
    m_account.setValue("lastStatementDate", s.m_dateEnd.toString("yyyy-MM-dd"));

  switch ( s.m_eType )
  {
    case MyMoneyStatement::etCheckings:
      m_account.setAccountType(MyMoneyAccount::Checkings);
      break;
    case MyMoneyStatement::etSavings:
      m_account.setAccountType(MyMoneyAccount::Savings);
      break;
    case MyMoneyStatement::etInvestment:
      //testing support for investment statements!
      //m_userAbort = true;
      //KMessageBox::error(kmymoney2, i18n("This is an investment statement.  These are not supported currently."), i18n("Critical Error"));
      m_account.setAccountType(MyMoneyAccount::Investment);
      break;
    case MyMoneyStatement::etCreditCard:
      m_account.setAccountType(MyMoneyAccount::CreditCard);
      break;
    default:
      m_account.setAccountType(MyMoneyAccount::Checkings);
      break;
  }

  if ( !m_userAbort )
    m_userAbort = ! selectOrCreateAccount(Select, m_account);

  //
  // Process the transactions
  //

  if ( !m_userAbort )
  {
    QValueList<MyMoneyStatement::Transaction>::const_iterator it_t = s.m_listTransactions.begin();
    while ( it_t != s.m_listTransactions.end() )
    {
      processTransactionEntry(*it_t);
      ++it_t;
    }
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
  // Although then we would need UI to verify
  t.setCommodity(m_account.currencyId());

  t.setPostDate(t_in.m_datePosted);
  t.setMemo(t_in.m_strMemo);

  if ( ! t_in.m_strBankID.isEmpty() )
    t.setBankID(t_in.m_strBankID);

  MyMoneySplit s1;

  s1.setMemo(t_in.m_strMemo);
  s1.setValue(t_in.m_moneyAmount);
  s1.setNumber(t_in.m_strNumber);

  // If the user has chosent to import into an investment account, determine the correct account to use
  MyMoneyAccount thisaccount = m_account; //file->account( m_account.id() );
  if ( thisaccount.accountType() == MyMoneyAccount::Investment )
  {
    // the correct account is the stock account which matches two criteria:
    // (1) it is a sub-account of the selected investment account, and
    // (2) the symbol of the underlying equity matches the security of the
    // transaction

    // search through each subordinate account
    bool found = false;
    QCStringList accounts = thisaccount.accountList();
    QCStringList::const_iterator it_account = accounts.begin();
    while( !found && it_account != accounts.end() )
    {
      QCString currencyid = file->account(*it_account).currencyId();
      MyMoneyEquity equity = file->equity( currencyid );
      QString symbol = equity.tradingSymbol();

      // startsWith(QString, bool) is not available in Qt 3.0
      if ( t_in.m_strSecurity.lower().startsWith(QString(symbol+" ").lower()) )
      {
        s1.setAccountId(*it_account);
        thisaccount = file->account(*it_account);
        found = true;

        // update the price, while we're here.  in the future, this should be
        // an option
        if ( ! equity.hasPrice( t_in.m_datePosted,true ) )
        {
          equity.addPriceHistory( t_in.m_datePosted, t_in.m_moneyAmount / t_in.m_dShares );
          file->modifyEquity(equity);
        }

      }

      ++it_account;
    }

    if (!found)
    {
      KMessageBox::information(0, i18n("This investment account does not contain the '%1' security.  "
                                        "Transactions involving this security will be ignored.").arg(t_in.m_strSecurity),
                                  i18n("Security not found"),
                                  QString("MissingEquity%1").arg(t_in.m_strSecurity.stripWhiteSpace()));
      return;
    }

    if (t_in.m_eAction==MyMoneyStatement::Transaction::eaReinvestDividend)
    {
      s1.setShares(MyMoneyMoney(t_in.m_dShares,1000));
      s1.setAction(MyMoneySplit::ActionReinvestDividend);
    }
    else if (t_in.m_eAction==MyMoneyStatement::Transaction::eaBuy || t_in.m_eAction==MyMoneyStatement::Transaction::eaSell)
    {
      s1.setShares(MyMoneyMoney(t_in.m_dShares,1000));
      s1.setAction(MyMoneySplit::ActionBuyShares);
    }

  }
  else
  {
    // For non-investment accounts, just use the selected account
    // Note that it is perfectly reasonable to import an investment statement into a non-investment account
    // if you really want.  The investment-specific information, such as number of shares and action will
    // be disacarded in that case.
    s1.setAccountId(m_account.id());

    if(s1.value() >= 0)
      s1.setAction(MyMoneySplit::ActionDeposit);
    else
      s1.setAction(MyMoneySplit::ActionWithdrawal);
  }

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

bool MyMoneyStatementReader::selectOrCreateAccount(const SelectCreateMode /*mode*/, MyMoneyAccount& account)
{
  bool result = false;

  MyMoneyFile* file = MyMoneyFile::instance();

  QCString accountId;

  // Try to find an existing account in the engine which matches this one.
  // There are two ways to be a "matching account".  The account number can
  // match the statement account OR the "StatementKey" property can match.
  // Either way, we'll update the "StatementKey" property for next time.

  QString accountNumber = account.number();
  if ( ! accountNumber.isEmpty() )
  {
    // Get a list of all accounts
    QValueList<MyMoneyAccount> accounts = file->accountList(QCStringList());

    // Iterate through them
    QValueList<MyMoneyAccount>::const_iterator it_account = accounts.begin();
    while ( it_account != accounts.end() )
    {
      if (
          ( (*it_account).value("StatementKey") == accountNumber ) ||
          ( (*it_account).number() == accountNumber )
        )
        {
          account.setAccountId( (*it_account).id() );
          accountId = (*it_account).id();
          break;
        }

      ++it_account;
    }
  }

  QString msg = i18n("<b>You have downloaded a statement for the following account:</b><br><br>");
  msg += i18n(" - Account Name: %1<br>").arg(account.name());
  msg += i18n(" - Account Type: %1<br>").arg(KMyMoneyUtils::accountTypeToString(account.accountType()));
  msg += i18n(" - Account Number: %1<br>").arg(account.number());
  msg += "<br>";

  QString header;

  if(!account.name().isEmpty())
  {
    if(!accountId.isEmpty())
      msg += i18n("Do you want to import transactions to this account?");
    else
      msg += i18n("KMyMoney cannot determine which of your accounts to use.  You can "
                  "create a new account by pressing the <b>Create</b> button "
                  "or select another one manually from the selection box below.");
  }
  else
  {
    msg += i18n("No account information has been found in the selected statement file. "
               "Please select an account using the selection box in the dialog or "
               "create a new account by pressing the <b>Create</b> button.");
  }

  KMyMoneyUtils::categoryTypeE type = static_cast<KMyMoneyUtils::categoryTypeE>(KMyMoneyUtils::asset|KMyMoneyUtils::liability);
  KAccountSelectDlg accountSelect(type, "StatementImport", kmymoney2);
  accountSelect.setHeader(i18n("Import transactions"));
  accountSelect.setDescription(msg);
  accountSelect.setAccount(account, accountId);
  accountSelect.setMode(false);
  accountSelect.showAbortButton(true);
  accountSelect.m_qifEntry->hide();

  bool done = false;
  while ( !done )
  {
    if ( accountSelect.exec() == QDialog::Accepted && !accountSelect.selectedAccount().isEmpty() )
    {
      result = true;
      done = true;
      accountId = accountSelect.selectedAccount();
      account = file->account(accountId);
      if ( ! accountNumber.isEmpty() && account.value("StatementKey") != accountNumber )
      {
        account.setValue("StatementKey",accountNumber);
        MyMoneyFile::instance()->modifyAccount(account);
      }
    }
    else
    {
      if(accountSelect.aborted())
        //throw new MYMONEYEXCEPTION("USERABORT");
        done = true;
      else
        KMessageBox::error(0, i18n("You must select an account, create a new one, or press the Abort button."));
    }
  }
  return result;
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

