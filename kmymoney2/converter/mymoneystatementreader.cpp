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
#include <kdebug.h>

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

  signalProgress(0, s.m_listTransactions.count(), "Importing Statement ...");
    
  //
  // Process the transactions
  //

  if ( !m_userAbort )
  {
    int progress = 0;
    QValueList<MyMoneyStatement::Transaction>::const_iterator it_t = s.m_listTransactions.begin();
    while ( it_t != s.m_listTransactions.end() )
    {
      processTransactionEntry(*it_t);
      signalProgress(++progress, 0);
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

  // set these values if a transfer split is needed at the very end.
  MyMoneyMoney transfervalue;
  QCString transferaccountid;  
  
  // If the user has chosent to import into an investment account, determine the correct account to use
  MyMoneyAccount thisaccount = m_account;
  if ( thisaccount.accountType() == MyMoneyAccount::Investment )
  {
    // find the security transacted, UNLESS this transaction didn't
    // involve any security.
    if ( t_in.m_eAction != MyMoneyStatement::Transaction::eaNone )
    {
      // the correct account is the stock account which matches two criteria:
      // (1) it is a sub-account of the selected investment account, and
      // (2a) the symbol of the underlying security matches the security of the
      // transaction, or
      // (2b) the name of the security matches the name of the security of the transaction.
  
      // search through each subordinate account
      bool found = false;
      QCStringList accounts = thisaccount.accountList();
      QCStringList::const_iterator it_account = accounts.begin();
      while( !found && it_account != accounts.end() )
      {
        QCString currencyid = file->account(*it_account).currencyId();
        MyMoneySecurity security = file->security( currencyid );
        QString symboltoken = security.tradingSymbol() + " ";
        QString nametoken = " " + security.name();
  
        if ( 
          t_in.m_strSecurity.startsWith(symboltoken,false)
          ||
          (t_in.m_strSecurity == nametoken) 
        )
        {
          s1.setAccountId(*it_account);
          thisaccount = file->account(*it_account);
          found = true;
  
          // update the price, while we're here.  in the future, this should be
          // an option
          QCString basecurrencyid = file->baseCurrency().id();
          MyMoneyPrice price = file->price( currencyid, basecurrencyid, t_in.m_datePosted, true );
          if ( !price.isValid() )
          {
            MyMoneyPrice newprice( currencyid, basecurrencyid, t_in.m_datePosted, t_in.m_moneyAmount / t_in.m_dShares, i18n("Statement Importer") );
            file->addPrice(newprice);
          }
        }
  
        ++it_account;
      }

      if (!found)
      {
        KMessageBox::information(0, i18n("This investment account does not contain the '%1' security.  "
                                          "Transactions involving this security will be ignored.").arg(t_in.m_strSecurity),
                                    i18n("Security not found"),
                                    QString("MissingSecurity%1").arg(t_in.m_strSecurity.stripWhiteSpace()));
        return;
      }
    }

    if (t_in.m_eAction==MyMoneyStatement::Transaction::eaReinvestDividend)
    {
      s1.setShares(MyMoneyMoney(t_in.m_dShares,1000));
      s1.setAction(MyMoneySplit::ActionReinvestDividend);
      
      MyMoneySplit s2;
      s2.setMemo(t_in.m_strMemo);
      s2.setAccountId(findOrCreateIncomeAccount("_Dividend"));
      s2.setShares(-t_in.m_moneyAmount);
      s2.setValue(-t_in.m_moneyAmount);
      t.addSplit(s2);
    }
    else if (t_in.m_eAction==MyMoneyStatement::Transaction::eaCashDividend)
    {
      // NOTE: With CashDividend, the amount of the dividend should
      // be in data.amount.  Since I've never seen an OFX file with
      // cash dividends, this is an assumption on my part. (acejones)

      // Cash dividends require setting 2 splits to get all of the information
      // in.  Split #1 will be the income split, and we'll set it to the first
      // income account.  This is a hack, but it's needed in order to get the
      // amount into the transaction.

      s1.setAccountId(findOrCreateIncomeAccount("_Dividend"));
      s1.setShares(-t_in.m_moneyAmount);
      s1.setValue(-t_in.m_moneyAmount);

      // Split 2 will be the zero-amount investment split that serves to
      // mark this transaction as a cash dividend and note which stock account
      // it belongs to.
      MyMoneySplit s2;
      s2.setMemo(t_in.m_strMemo);
      s2.setValue(0);
      s2.setAction(MyMoneySplit::ActionDividend);
      s2.setAccountId(thisaccount.id());
      t.addSplit(s2);
      
      transfervalue = t_in.m_moneyAmount;
    }
    else if (t_in.m_eAction==MyMoneyStatement::Transaction::eaBuy || t_in.m_eAction==MyMoneyStatement::Transaction::eaSell)
    {
      s1.setShares(MyMoneyMoney(t_in.m_dShares,1000));
      s1.setAction(MyMoneySplit::ActionBuyShares);
      
      transfervalue = -t_in.m_moneyAmount;
    }
    else if (t_in.m_eAction==MyMoneyStatement::Transaction::eaNone)
    {
      // User is attempting to import a non-investment transaction into this
      // investment account.  This is not supportable the way KMyMoney is 
      // written.  However, if a user has an associated brokerage account,
      // we can stuff the transaction there.
      
      QCString brokerageactid = m_account.value("kmm-brokerage-account").utf8();
      if ( ! brokerageactid.isEmpty() )
      {
        s1.setAccountId(brokerageactid);
    
        if(!s1.value().isNegative())
          s1.setAction(MyMoneySplit::ActionDeposit);
        else
          s1.setAction(MyMoneySplit::ActionWithdrawal);
      }
      else
      {
        // Warning!! Your transaction is being thrown away.
      }
    }
  }
  else
  {
    // For non-investment accounts, just use the selected account
    // Note that it is perfectly reasonable to import an investment statement into a non-investment account
    // if you really want.  The investment-specific information, such as number of shares and action will
    // be discarded in that case.
    s1.setAccountId(m_account.id());

    if(!s1.value().isNegative())
      s1.setAction(MyMoneySplit::ActionDeposit);
    else
      s1.setAction(MyMoneySplit::ActionWithdrawal);
  }

  QString payeename = t_in.m_strPayee;
  if(!payeename.isEmpty())
  {
    QCString payeeid;
    try {
      payeeid = file->payeeByName(payeename).id();
      s1.setPayeeId(payeeid);
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
          payeeid = payee.id();
          s1.setPayeeId(payeeid);

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
    
    //
    // Fill in other side of the transaction (category/etc) based on payee
    //
    // Note, this logic is lifted from KLedgerView::slotPayeeChanged(),
    // however this case is more complicated, because we have an amount and
    // a memo.  We just don't have the other side of the transaction.
    //
    // We'll search for the most recent transaction in this account with
    // this payee.  If this reference transaction is a simple 2-split
    // transaction, it's simple.  If it's a complex split, and the amounts
    // are different, we have a problem.  Somehow we have to balance the
    // transaction.  For now, we'll leave it unbalanced, and let the user
    // handle it.
    //

    MyMoneyTransactionFilter filter(m_account.id());
    filter.addPayee(payeeid);
    QValueList<MyMoneyTransaction> list = file->transactionList(filter);
    if(!list.empty())
    {
      // Default to using the most recent transaction as the reference
      MyMoneyTransaction t_old = list.last();
      
      // if there is more than one matching transaction, try to be a little
      // smart about which one we take.  for now, we'll see if there's one
      // with the same VALUE as our imported transaction, and if so take that one.
      if ( list.count() > 1 )
      {
        QValueList<MyMoneyTransaction>::ConstIterator it_trans = list.fromLast();
        while ( it_trans != list.end() )
        {
          MyMoneySplit s = (*it_trans).splitByAccount(m_account.id());
          if ( s.value() == s1.value() )
          {
            t_old = *it_trans;
            break;
          }        
          --it_trans;        
        }      
      }
    
      QValueList<MyMoneySplit>::ConstIterator it_split;
      for(it_split = t_old.splits().begin(); it_split != t_old.splits().end(); ++it_split)
      {
        // We don't need the split that covers this account,
        // we just need the other ones.
        if ( (*it_split).accountId() != m_account.id() )
        {
          MyMoneySplit s(*it_split);
          s.setReconcileFlag(MyMoneySplit::NotReconciled);
          s.setId(QCString());
    
          if ( t_old.splits().count() == 2 )
          {
            s.setShares(-s1.shares());
            s.setValue(-s1.value());
          }
          t.addSplit(s);
        }
      }
    }
    
  }

  t.addSplit(s1);

  // Add the 'account' split if it's needed
  if ( ! transfervalue.isZero() )
  {
    QCString brokerageactid = m_account.value("kmm-brokerage-account").utf8();
    if ( ! brokerageactid.isEmpty() )
    {
      // FIXME This may not deal with foreign currencies properly
      MyMoneySplit s;
      s.setMemo(t_in.m_strMemo);
      s.setValue(transfervalue);
      s.setShares(transfervalue);
      s.setAccountId(brokerageactid);
      t.addSplit(s);
    }
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
      file->addTransaction(t);
    }
  } catch (MyMoneyException *e) {
    QString message(i18n("Problem adding imported transaction: "));
    message += e->what();
    
    int result = KMessageBox::warningContinueCancel(0, message);
    if ( result == KMessageBox::Cancel )
        throw new MYMONEYEXCEPTION("USERABORT");
    
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

const QCString MyMoneyStatementReader::findOrCreateIncomeAccount(const QString& searchname)
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

#include "mymoneystatementreader.moc"
