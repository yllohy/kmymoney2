/***************************************************************************
                          keditscheduleddepositdlg.cpp  -  description
                             -------------------
    begin                : Sun Feb 17 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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
// QT Includes
#include <qpushbutton.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kconfig.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "../widgets/kmymoneycombo.h"
#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneypayee.h"
#include "keditscheduleddepositdlg.h"
#include "../dialogs/ksplittransactiondlg.h"

KEditScheduledDepositDlg::KEditScheduledDepositDlg(MyMoneyFile *file, QWidget *parent, const char *name)
 : kEditScheduledDepositDlgDecl(parent,name, true)
{
  m_mymoneyfile = file;

  readConfig();
  reloadFromFile();

  m_transaction = new MyMoneyTransaction();

  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_qbuttonSplit, SIGNAL(clicked()), this, SLOT(slotSplitClicked()));
}

KEditScheduledDepositDlg::~KEditScheduledDepositDlg()
{
  writeConfig();
}

void KEditScheduledDepositDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_lastPayee = config->readEntry("LastPayee");
}

void KEditScheduledDepositDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("LastPayee", m_kcomboPayee->currentText());
  config->sync();
}

void KEditScheduledDepositDlg::reloadFromFile(void)
{
/*
  int pos=0, k=0;
  QListIterator<MyMoneyPayee> it = m_mymoneyfile->payeeIterator();
  for ( ; it.current(); ++it, k++) {
    m_kcomboPayee->insertItem(it.current()->name());
    if (it.current()->name()==m_lastPayee)
      pos = k;
  }
  m_kcomboPayee->setCurrentItem(pos);

  m_kcomboMethod->insertItem(i18n("Cheque"));
  m_kcomboMethod->insertItem(i18n("Deposit"));
  m_kcomboMethod->insertItem(i18n("Transfer"));
  m_kcomboMethod->insertItem(i18n("Withdrawal"));
  m_kcomboMethod->insertItem(i18n("ATM"));

  m_kcomboFreq->insertItem(i18n("Once"));
  m_kcomboFreq->insertItem(i18n("Daily"));
  m_kcomboFreq->insertItem(i18n("Weekly"));
  m_kcomboFreq->insertItem(i18n("Every other week"));
  m_kcomboFreq->insertItem(i18n("Twice a month"));
  m_kcomboFreq->insertItem(i18n("Every four weeks"));
  m_kcomboFreq->insertItem(i18n("Monthly"));
  m_kcomboFreq->insertItem(i18n("Every other month"));
  m_kcomboFreq->insertItem(i18n("Every three months"));
  m_kcomboFreq->insertItem(i18n("Every four months"));
  m_kcomboFreq->insertItem(i18n("Twice a year"));
  m_kcomboFreq->insertItem(i18n("Yearly"));
  m_kcomboFreq->insertItem(i18n("Every other year"));

  QStringList categoryList;
  QStringList qstringlistIncome;
  QStringList qstringlistExpense;
  bool bDoneInsert = false;
  QString theText;

  QListIterator<MyMoneyCategory> categoryIterator = m_mymoneyfile->categoryIterator();
  for ( ; categoryIterator.current(); ++categoryIterator) {
    MyMoneyCategory *category = categoryIterator.current();

    theText = category->name();

    if (category->isIncome()) {
      // Add it alpabetically
      if (qstringlistIncome.count()<=0)
        qstringlistIncome.append(theText);
      else {
        for (QStringList::Iterator it3 = qstringlistIncome.begin(); it3 != qstringlistIncome.end(); ++it3 ) {
          if ((*it3) >= theText && !bDoneInsert) {
            qstringlistIncome.insert(it3, theText);
            bDoneInsert = true;
          }
        }
        if (!bDoneInsert)
          qstringlistIncome.append(theText);
      }
    } else { // is expense
      // Add it alpabetically
      if (qstringlistExpense.count()<=0)
        qstringlistExpense.append(theText);
      else {
        for (QStringList::Iterator it4 = qstringlistExpense.begin(); it4 != qstringlistExpense.end(); ++it4 ) {
          if ((*it4) >= theText && !bDoneInsert) {
            qstringlistExpense.insert(it4, theText);
            bDoneInsert = true;
          }
        }
        if (!bDoneInsert)
          qstringlistExpense.append(theText);
      }
    }

    // Now add all the minor categories
    for ( QStringList::Iterator it = category->minorCategories().begin(); it != category->minorCategories().end(); ++it ) {
      theText = category->name();
			theText += ":";
			theText += (*it);
				
			bDoneInsert = false;
				
      if (category->isIncome()) {
        // Add it alpabetically
        if (qstringlistIncome.count()<=0)
          qstringlistIncome.append(theText);
        else {
          for (QStringList::Iterator it3 = qstringlistIncome.begin(); it3 != qstringlistIncome.end(); ++it3 ) {
            if ((*it3) >= theText && !bDoneInsert) {
              qstringlistIncome.insert(it3, theText);
              bDoneInsert = true;
            }
          }
          if (!bDoneInsert)
            qstringlistIncome.append(theText);
        }
      } else { // is expense
        // Add it alpabetically
        if (qstringlistExpense.count()<=0)
          qstringlistExpense.append(theText);
        else {
          for (QStringList::Iterator it4 = qstringlistExpense.begin(); it4 != qstringlistExpense.end(); ++it4 ) {
            if ((*it4) >= theText && !bDoneInsert) {
              qstringlistExpense.insert(it4, theText);
              bDoneInsert = true;
            }
          }
          if (!bDoneInsert)
            qstringlistExpense.append(theText);
        }
      }
    }  // End minor iterator
  }

  // Load all the accounts.
  QStringList qstringlistAccounts;
  MyMoneyBank *bank;
  MyMoneyAccount *account;
  for (bank=m_mymoneyfile->bankFirst(); bank; bank=m_mymoneyfile->bankNext())
  {
    for (account=bank->accountFirst(); account; account=bank->accountNext())
    {
      qstringlistAccounts.append("<" + bank->name() + ":" + account->name() + ">");
    }
  }
  m_kcomboDepositAccount->insertStringList(qstringlistAccounts);

  qstringlistAccounts.append("Split");

	m_kcomboCategory->clear();
	
	qstringlistIncome.prepend(i18n("--- Income ---"));
  qstringlistIncome.prepend("");
	categoryList = qstringlistIncome;
	
	qstringlistExpense.prepend(i18n("--- Expense ---"));
	categoryList += qstringlistExpense;
	
	qstringlistAccounts.prepend(i18n("--- Special ---"));
	categoryList += qstringlistAccounts;

  m_kcomboCategory->insertStringList(categoryList);
*/
}

/*
  Modified from KTransactionView::slotEditSplit().
*/
void KEditScheduledDepositDlg::slotSplitClicked()
{
/*
  MyMoneyMoney amount;

  bool amountSet = true;
  if(m_kmoneyeditAmount->text() == "")
  {
    amount = 0;
    amountSet = false;
  }
  else
  {
    amount = m_kmoneyeditAmount->text();
  }

  // Build up the global transaction

  QString qstringBankName =
    m_kcomboDepositAccount->currentText().mid(1, m_kcomboDepositAccount->currentText().find(':')-1);

  bool bFoundAll=false;

  MyMoneyBank *bank;
  MyMoneyAccount *account;
  for (bank=m_mymoneyfile->bankFirst(); bank; bank=m_mymoneyfile->bankNext())
  {
    if (bank->name() == qstringBankName)
    {
      QString qstringAccountName =
        m_kcomboDepositAccount->currentText().mid(
          m_kcomboDepositAccount->currentText().find(':')+1,
          m_kcomboDepositAccount->currentText().length() - (qstringBankName.length()+3) );

      for (account=bank->accountFirst(); account; account=bank->accountNext())
      {
        if (account->name() == qstringAccountName)
        {
          bFoundAll=true;
          break;
        }
      }
      if (bFoundAll)
        break;
    }
  }

  if (!bFoundAll)
  {
    KMessageBox::information(this, i18n("Unable to locate account when getting ready for split dialog.\nHave you created one?"));
    return;
  }

  KSplitTransactionDlg* dlg = new KSplitTransactionDlg(0, 0,
    m_mymoneyfile, bank, account,
    &amount, amountSet);

  MyMoneySplitTransaction* split;
  MyMoneySplitTransaction* tmp;

  if(dlg->exec() == QDialog::Accepted)
  {
//    m_transaction.clearSplitList();  // The transaction should be empty anyway
    // copy the split list
    split = dlg->firstTransaction();
    while(split != NULL)
    {
      tmp = new MyMoneySplitTransaction(*split);
      tmp->setParent(m_transaction);
      m_transaction->addSplit(tmp);
      split = dlg->nextTransaction();
    }
    m_transaction->setAmount(amount);
//    m_payment->setText(((transaction->type()==MyMoneyTransaction::Debit) ? KGlobal::locale()->formatMoney(amount.amount(),"") : QString("")));
//    m_withdrawal->setText(((transaction->type()==MyMoneyTransaction::Credit) ? KGlobal::locale()->formatMoney(amount.amount(),"") : QString("")));
    m_kcomboCategory->setCurrentItem("Split");
  }
  delete dlg;
*/
}
