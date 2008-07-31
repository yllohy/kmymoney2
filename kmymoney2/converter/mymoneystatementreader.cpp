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

#include <typeinfo>

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
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneystatement.h>
#include <kmymoney/kmymoneyglobalsettings.h>
#include <kmymoney/transactioneditor.h>
#include "../dialogs/kaccountselectdlg.h"
#include "../dialogs/transactionmatcher.h"
#include "../dialogs/kenterscheduledlg.h"
#include "../kmymoney2.h"

MyMoneyStatementReader::MyMoneyStatementReader() :
  m_userAbort(false),
  m_autoCreatePayee(false),
  m_ft(0),
  m_progressCallback(0)
{
}

MyMoneyStatementReader::~MyMoneyStatementReader()
{
}

void MyMoneyStatementReader::setAutoCreatePayee(const bool create)
{
  m_autoCreatePayee = create;
}

bool MyMoneyStatementReader::import(const MyMoneyStatement& s)
{
  //
  // For testing, save the statement to an XML file
  // (uncomment this line)
  //
  //MyMoneyStatement::writeXMLFile(s,"Imported.Xml");

  //
  // Select the account
  //

  m_account = MyMoneyAccount();

  m_ft = new MyMoneyFileTransaction();

  // if the statement source left some information about
  // the account, we use it to get the current data of it
  if(!s.m_accountId.isEmpty()) {
    try {
      m_account = MyMoneyFile::instance()->account(s.m_accountId);
    } catch(MyMoneyException* e) {
      qDebug("Received reference '%s' to unknown account in statement", s.m_accountId.data());
      delete e;
    }
  }

  if(m_account.id().isEmpty()) {
    m_account.setName(s.m_strAccountName);
    m_account.setNumber(s.m_strAccountNumber);

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
  }

  // see if we need to update some values stored with the account
  if(m_account.value("lastStatementBalance") != s.m_closingBalance.toString()
  || m_account.value("lastImportedTransactionDate") != s.m_dateEnd.toString(Qt::ISODate)) {
    m_account.setValue("lastStatementBalance", s.m_closingBalance.toString());
    if ( s.m_dateEnd.isValid() ) {
      m_account.setValue("lastImportedTransactionDate", s.m_dateEnd.toString(Qt::ISODate));
    }

    try {
      MyMoneyFile::instance()->modifyAccount(m_account);
    } catch(MyMoneyException* e) {
      qDebug("Updating account in MyMoneyStatementReader::startImport failed");
      delete e;
    }
  }

  signalProgress(0, s.m_listTransactions.count(), "Importing Statement ...");

  //
  // Process the securities
  //
  if ( s.m_eType == MyMoneyStatement::etInvestment )
  {
    QValueList<MyMoneyStatement::Security>::const_iterator it_s = s.m_listSecurities.begin();
    while ( it_s != s.m_listSecurities.end() )
    {
      processSecurityEntry(*it_s);
      ++it_s;
    }
  }

  //
  // Process the transactions
  //

  if ( !m_userAbort )
  {
    try {
      int progress = 0;
      QValueList<MyMoneyStatement::Transaction>::const_iterator it_t = s.m_listTransactions.begin();
      while ( it_t != s.m_listTransactions.end() )
      {
        processTransactionEntry(*it_t);
        signalProgress(++progress, 0);
        ++it_t;
      }
    } catch(MyMoneyException* e) {
      if(e->what() == "USERABORT")
        m_userAbort = true;
      else
        qDebug("Caught exception from processTransactionEntry() not caused by USERABORT: %s", e->what().data());
      delete e;
    }
  }

  qDebug("Statement balance is '%s'", s.m_closingBalance.formatMoney("",2).data());
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

  // finish the transaction
  if(rc)
    m_ft->commit();
  delete m_ft;
  m_ft = 0;

  return rc;
}

void MyMoneyStatementReader::processSecurityEntry(const MyMoneyStatement::Security& sec_in)
{
  // For a security entry, we will just make sure the security exists in the
  // file. It will not get added to the investment account until it's called
  // for in a transaction.
  MyMoneyFile* file = MyMoneyFile::instance();

  // check if we already have the security
  // In a statement, we do not know what type of security this is, so we will
  // not use type as a matching factor.
  MyMoneySecurity security;
  QValueList<MyMoneySecurity> list = file->securityList();
  QValueList<MyMoneySecurity>::ConstIterator it = list.begin();
  while ( it != list.end() && security.id().isEmpty() )
  {
    if(sec_in.m_strSymbol.isEmpty()) {
      if((*it).name() == sec_in.m_strName)
        security = *it;
    } else if((*it).tradingSymbol() == sec_in.m_strSymbol)
      security = *it;
    ++it;
  }

  // if the security was not found, we have to create it while not forgetting
  // to setup the type
  if(security.id().isEmpty())
  {
    security.setName(sec_in.m_strName);
    security.setTradingSymbol(sec_in.m_strSymbol);
    security.setSmallestAccountFraction(1000);
    security.setTradingCurrency(file->baseCurrency().id());
    security.setValue("kmm-security-id", sec_in.m_strId);
    security.setValue("kmm-online-source", "Yahoo");
    security.setSecurityType(MyMoneySecurity::SECURITY_STOCK);
    MyMoneyFileTransaction ft;
    try {
      file->addSecurity(security);
      ft.commit();
      kdDebug(0) << "Created " << security.name() << " with id " << security.id() << endl;
    } catch(MyMoneyException *e) {
      KMessageBox::error(0, i18n("Error creating security record: %1").arg(e->what()), i18n("Error"));
    }
  } else {
    kdDebug(0) << "Found " << security.name() << " with id " << security.id() << endl;
  }
}

void MyMoneyStatementReader::processTransactionEntry(const MyMoneyStatement::Transaction& t_in)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyTransaction t;

  // mark it imported for the view
  t.setValue("Imported", "true");

  // TODO (Ace) We can get the commodity from the statement!!
  // Although then we would need UI to verify
  t.setCommodity(m_account.currencyId());

  t.setPostDate(t_in.m_datePosted);
  t.setMemo(t_in.m_strMemo);

#if 0
  // (acejones) removing this code.  keeping it around for reference.
  //
  // this is the OLD way of handling bank ID's, which unfortunately was wrong.
  // bank ID's actually need to go on the split which corresponds with the
  // account we're importing into.
  //
  // thus anywhere "this account" is put into a split is also where we need
  // to put the bank ID in.
  //
  if ( ! t_in.m_strBankID.isEmpty() )
    t.setBankID(t_in.m_strBankID);
#endif

  MyMoneySplit s1;

  s1.setMemo(t_in.m_strMemo);
  s1.setValue(t_in.m_amount - t_in.m_fees);
  s1.setShares(s1.value());
  s1.setNumber(t_in.m_strNumber);

  // set these values if a transfer split is needed at the very end.
  MyMoneyMoney transfervalue;

  // If the user has chosen to import into an investment account, determine the correct account to use
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

        if(t_in.m_strSecurity.startsWith(symboltoken,false)
        || (t_in.m_strSecurity == nametoken))
        {
          thisaccount = file->account(*it_account);
          found = true;

          if(t_in.m_eAction != MyMoneyStatement::Transaction::eaCashDividend) // Bug #1581788: Should not update a price for cash dividends
          {
            // update the price, while we're here.  in the future, this should be
            // an option
            QCString basecurrencyid = file->baseCurrency().id();
            MyMoneyPrice price = file->price( currencyid, basecurrencyid, t_in.m_datePosted, true );
            if ( !price.isValid() )
            {
              MyMoneyPrice newprice( currencyid, basecurrencyid, t_in.m_datePosted, t_in.m_amount / t_in.m_shares, i18n("Statement Importer") );
              file->addPrice(newprice);
            }
          }
        }

        ++it_account;
      }

      // If there was no stock account under the m_acccount investment account,
      // add one using the security.
      if (!found)
      {
        // The security should always be available, because the statement file
        // should separately list all the securities referred to in the file,
        // and when we found a security, we added it to the file.

        MyMoneySecurity security;
        QValueList<MyMoneySecurity> list = MyMoneyFile::instance()->securityList();
        QValueList<MyMoneySecurity>::ConstIterator it = list.begin();
        while ( it != list.end() && security.id().isEmpty() )
        {
          QString symboltoken = (*it).tradingSymbol() + " ";
          QString nametoken = " " + (*it).name();
          if (
            t_in.m_strSecurity.startsWith(symboltoken,false)
            ||
            (t_in.m_strSecurity == nametoken)
          )
            security = *it;
          ++it;
        }

        if(!security.id().isEmpty())
        {
          thisaccount = MyMoneyAccount();
          thisaccount.setName(security.name());
          thisaccount.setAccountType(MyMoneyAccount::Stock);
          thisaccount.setCurrencyId(security.id());

          file->addAccount(thisaccount, m_account);
          kdDebug(0) << __func__ << ": created account " << thisaccount.id() << " for security " << t_in.m_strSecurity << " under account " << m_account.id() << endl;
        }
        // this security does not exist in the file.
        else
        {
          if ( t_in.m_strSecurity.isEmpty() )
          {
            KMessageBox::information(0, i18n("This imported statement contains investment transactions with no security.  These transactions will be ignored.").arg(t_in.m_strSecurity),i18n("Security not found"),QString("BlankSecurity"));
          }
          else
          {
            // This should be rare.  A statement should have a security entry for any
            // of the securities referred to in the transactions.  The only way to get
            // here is if that's NOT the case.
            KMessageBox::information(0, i18n("This investment account does not contain the \"%1\" security.  Transactions involving this security will be ignored.").arg(t_in.m_strSecurity),i18n("Security not found"),QString("MissingSecurity%1").arg(t_in.m_strSecurity.stripWhiteSpace()));
          }
          return;
        }
      }
    }

    s1.setAccountId(thisaccount.id());
    if ( ! t_in.m_strBankID.isEmpty() )
      s1.setBankID(t_in.m_strBankID);

    if (t_in.m_eAction==MyMoneyStatement::Transaction::eaReinvestDividend)
    {
      s1.setShares(t_in.m_shares);
      s1.setAction(MyMoneySplit::ActionReinvestDividend);

      MyMoneySplit s2;
      s2.setMemo(t_in.m_strMemo);
      s2.setAccountId(findOrCreateIncomeAccount("_Dividend"));
      s2.setShares(-t_in.m_amount);
      s2.setValue(-t_in.m_amount);
      t.addSplit(s2);
    }
    else if (t_in.m_eAction==MyMoneyStatement::Transaction::eaCashDividend)
    {
      // Cash dividends require setting 2 splits to get all of the information
      // in.  Split #1 will be the income split, and we'll set it to the first
      // income account.  This is a hack, but it's needed in order to get the
      // amount into the transaction.

      // There are some sign issues.  The OFX plugin universally reverses the sign
      // for investment transactions.
      //
      // The way we interpret the sign on 'amount' is the s1 split, which is always
      // the thing that's NOT the cash account.  For dividends, it's the income
      // category, for buy/sell it's the stock account.
      //
      // For cash account transactions, the s1 split IS the cash account split,
      // which explains why they have to be reversed for investment transactions
      //
      // Ergo, the 'amount' is negative at this point and needs to stay negative.
      // The 'fees' is positive.
      //
      // This should probably change.  It would be more consistent to ALWAYS
      // interpret the 'amount' as the cash account part.

      s1.setAccountId(findOrCreateIncomeAccount("_Dividend"));
      s1.setShares(t_in.m_amount);
      s1.setValue(t_in.m_amount);

      // Split 2 will be the zero-amount investment split that serves to
      // mark this transaction as a cash dividend and note which stock account
      // it belongs to.
      MyMoneySplit s2;
      s2.setMemo(t_in.m_strMemo);
      s2.setValue(0);
      s2.setAction(MyMoneySplit::ActionDividend);
      s2.setAccountId(thisaccount.id());
      t.addSplit(s2);

      transfervalue = -t_in.m_amount-t_in.m_fees;
    }
    else if (t_in.m_eAction==MyMoneyStatement::Transaction::eaBuy || t_in.m_eAction==MyMoneyStatement::Transaction::eaSell)
    {
      s1.setShares(t_in.m_shares);
      s1.setAction(MyMoneySplit::ActionBuyShares);

      transfervalue = -t_in.m_amount;
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
        if ( ! t_in.m_strBankID.isEmpty() )
          s1.setBankID(t_in.m_strBankID);

#if 0
        if(!s1.value().isNegative())
          s1.setAction(MyMoneySplit::ActionDeposit);
        else
          s1.setAction(MyMoneySplit::ActionWithdrawal);
#endif

        // Needed to satisfy the bankid check below.
        thisaccount = file->account(brokerageactid);
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
    if ( ! t_in.m_strBankID.isEmpty() )
      s1.setBankID(t_in.m_strBankID);

#if 0
    if(!s1.value().isNegative())
      s1.setAction(MyMoneySplit::ActionDeposit);
    else
      s1.setAction(MyMoneySplit::ActionWithdrawal);
#endif
  }

  QString payeename = t_in.m_strPayee;
  if(!payeename.isEmpty())
  {
    QCString payeeid;
    try {
      QValueList<MyMoneyPayee> pList = file->payeeList();
      QValueList<MyMoneyPayee>::const_iterator it_p;
      for(it_p = pList.begin(); payeeid.isEmpty() && it_p != pList.end(); ++it_p) {
        bool ignoreCase;
        QStringList keys;
        QStringList::const_iterator it_s;
        switch((*it_p).matchData(ignoreCase, keys)) {
          case MyMoneyPayee::matchDisabled:
            break;

          case MyMoneyPayee::matchName:
            keys << (*it_p).name();
            // tricky fall through here

          case MyMoneyPayee::matchKey:
            for(it_s = keys.begin(); it_s != keys.end(); ++it_s) {
              QRegExp exp(*it_s, !ignoreCase);
              if(exp.search(payeename) != -1) {
                payeeid = (*it_p).id();
                break;
              }
            }
            break;
        }
      }
      if(payeeid.isEmpty())
        throw new MYMONEYEXCEPTION("payee not matched");

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
      delete e;

      if(rc == KMessageBox::Yes) {
        // for now, we just add the payee to the pool and turn
        // on simple name matching, so that future transactions
        // with the same name don't get here again.
        //
        // In the future, we could open a dialog and ask for
        // all the other attributes of the payee, but since this
        // is called in the context of an automatic procedure it
        // might distract the user.
        payee.setName(payeename);
        QStringList keys;
        keys << payeename;
        payee.setMatchData(MyMoneyPayee::matchName, true, keys);

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

    MyMoneyTransactionFilter filter(thisaccount.id());
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
          MyMoneySplit s = (*it_trans).splitByAccount(thisaccount.id());
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
        if ( (*it_split).accountId() != thisaccount.id() )
        {
          MyMoneySplit s(*it_split);
          s.setReconcileFlag(MyMoneySplit::NotReconciled);
          s.clearId();
          s.setBankID(QString());

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

  // The fees has to be added AFTER the interest, because
  // KLedgerViewInvestments::preloadInvestmentSplits expects the splits to be
  // ordered this way.
  if ( !t_in.m_fees.isZero() )
  {
    MyMoneySplit s;
    s.setMemo(i18n("(Fees) ") + t_in.m_strMemo);
    s.setValue(t_in.m_fees);
    s.setShares(t_in.m_fees);
    s.setAccountId(findOrCreateExpenseAccount("_Fees"));
    t.addSplit(s);
  }

  // Add the 'account' split if it's needed
  if ( ! transfervalue.isZero() )
  {
    QCString brokerageactid = m_account.value("kmm-brokerage-account").utf8();
    if (brokerageactid.isEmpty() )
    {
      brokerageactid = file->accountByName(m_account.brokerageName()).id();
    }

    if ( !brokerageactid.isEmpty() )
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

    // check for matches already stored in the engine
    MyMoneySplit matchedSplit;
    TransactionMatcher::autoMatchResultE result;
    TransactionMatcher matcher(thisaccount);
    matcher.setMatchWindow(KMyMoneyGlobalSettings::matchInterval());
    const MyMoneyObject *o = matcher.findMatch(t, s1, matchedSplit, result);

    // if we did not already find this one, we need to process it
    if(result != TransactionMatcher::matchedDuplicate) {
      file->addTransaction(t);

      if(o) {
        if(typeid(*o) == typeid(MyMoneyTransaction)) {
          // it matched a simple transaction. that's the easy case
          MyMoneyTransaction tm(*(dynamic_cast<const MyMoneyTransaction*>(o)));
          switch(result) {
            case TransactionMatcher::notMatched:
            case TransactionMatcher::matchedDuplicate:
              // no need to do anything here
              break;
            case TransactionMatcher::matched:
              matcher.match(tm, matchedSplit, t, s1);
              break;
          }

        } else if(typeid(*o) == typeid(MyMoneySchedule)) {
          // a match has been found in a pending schedule. We'll ask the user if she wants
          // to enter the schedule and match it agains the new transaction. Otherwise, we
          // just leave the transaction as imported.
          MyMoneySchedule schedule(*(dynamic_cast<const MyMoneySchedule*>(o)));
          if(KMessageBox::questionYesNo(0, QString("<qt>%1</qt>").arg(i18n("KMyMoney has found a scheduled transaction named <b>%1</b> which matches an imported transaction. Do you want KMyMoney to enter this schedule now so that the transaction can be matched? ").arg(schedule.name())), i18n("Schedule found")) == KMessageBox::Yes) {
            KEnterScheduleDlg dlg(0, schedule);
            TransactionEditor* editor = dlg.startEdit();
            if(editor) {
              MyMoneyTransaction torig;
              editor->createTransaction(torig, dlg.transaction(), dlg.transaction().splits()[0], true);
              QCString newId;
              if(editor->enterTransactions(newId, false)) {
                if(!newId.isEmpty()) {
                  torig = MyMoneyFile::instance()->transaction(newId);
                  schedule.setLastPayment(torig.postDate());
                }
                schedule.setNextDueDate(schedule.nextPayment(schedule.nextDueDate()));
                MyMoneyFile::instance()->modifySchedule(schedule);
              }

              // now match the two transactions
              matcher.match(torig, matchedSplit, t, s1);
            }
            delete editor;
          }
        }
      }
    }
    delete o;
  } catch (MyMoneyException *e) {
    QString message(i18n("Problem adding or matching imported transaction with id '%1': %2").arg(t_in.m_strBankID).arg(e->what()));
    delete e;

    int result = KMessageBox::warningContinueCancel(0, message);
    if ( result == KMessageBox::Cancel )
      throw new MYMONEYEXCEPTION("USERABORT");
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
    QValueList<MyMoneyAccount> accounts;
    file->accountList(accounts);

    // Iterate through them
    QValueList<MyMoneyAccount>::const_iterator it_account = accounts.begin();
    while ( it_account != accounts.end() )
    {
      if (
          ( (*it_account).value("StatementKey") == accountNumber ) ||
          ( (*it_account).number() == accountNumber )
        )
        {
          MyMoneyAccount newAccount((*it_account).id(), account);
          account = newAccount;
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
        account.setValue("StatementKey", accountNumber);
        MyMoneyFileTransaction ft;
        try {
          MyMoneyFile::instance()->modifyAccount(account);
          ft.commit();
        } catch(MyMoneyException* e) {
          qDebug("Updating account in MyMoneyStatementReader::selectOrCreateAccount failed");
          delete e;
        }
      }
    }
    else
    {
      if(accountSelect.aborted())
        //throw new MYMONEYEXCEPTION("USERABORT");
        done = true;
      else
        KMessageBox::error(0, QString("<qt>%1</qt>").arg(i18n("You must select an account, create a new one, or press the <b>Abort</b> button.")));
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

const QCString MyMoneyStatementReader::findOrCreateExpenseAccount(const QString& searchname)
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

#include "mymoneystatementreader.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
