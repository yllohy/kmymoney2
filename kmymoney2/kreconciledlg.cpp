/***************************************************************************
                          kreconciledlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>

#include "kreconciledlg.h"
#include "kmymoneysettings.h"
#include "kreconcilelistitem.h"

KReconcileDlg::KReconcileDlg(const MyMoneyMoney previousBal, const MyMoneyMoney endingBal, const QDate endingDate, const MyMoneyBank bankIndex, MyMoneyAccount *accountIndex, const MyMoneyFile file, QWidget *parent, const char *name)
 : KReconcileDlgDecl(parent,name,false)
{
  descriptionLabel->setText(i18n("Click on a transaction to mark it as Reconciled.\nYou can add/delete transactions from the main register window."));

  m_balanced = false;
  m_debitsQList.setAutoDelete(false);
  m_creditsQList.setAutoDelete(false);
  m_reconciledTransactions.setAutoDelete(false);

	m_file = file;
  m_bankIndex = bankIndex;
	m_accountIndex = accountIndex;
  m_endingBalance = endingBal;
  m_previousBalance = previousBal;
  m_clearedBalance.setAmount(0.0);
  m_debitBalance.setAmount(0.0);
  m_creditBalance.setAmount(0.0);
  m_endingDate = endingDate;
	
	totalCreditsLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	totalDebitsLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	previousLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	endingLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	differenceLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);

	
	debitListView->setRootIsDecorated(false);
	debitListView->addColumn(i18n("Date"));
	debitListView->addColumn(i18n("Number"));
	debitListView->addColumn(i18n("Description"));
	debitListView->addColumn(i18n("Amount"));
	debitListView->addColumn(i18n("C"));
	debitListView->setMultiSelection(true);
  debitListView->setAllColumnsShowFocus(true);
	
	creditListView->setRootIsDecorated(false);
	creditListView->addColumn(i18n("Date"));
	creditListView->addColumn(i18n("Number"));
	creditListView->addColumn(i18n("Description"));
	creditListView->addColumn(i18n("Amount"));
	creditListView->addColumn(i18n("C"));
	creditListView->setMultiSelection(true);
  creditListView->setAllColumnsShowFocus(true);
	
	MyMoneyMoney money(m_clearedBalance);
	QString text(i18n("Cleared Balance: "));
	text += KGlobal::locale()->formatMoney(money.amount());
	endingLabel->setText(text);
	
  money = m_endingBalance;
	text = i18n("Ending Balance: ");
	text += KGlobal::locale()->formatMoney(money.amount());
	previousLabel->setText(text);

  money = m_creditBalance;
	text = i18n("Deposits: ");
	text += KGlobal::locale()->formatMoney(money.amount());
	totalCreditsLabel->setText(text);
	
  money = m_debitBalance;
	text = i18n("Withdrawals: ");
	text += KGlobal::locale()->formatMoney(money.amount());
	totalDebitsLabel->setText(text);


	loadLists();
	insertTransactions();
	
  connect(debitListView, SIGNAL(clicked(QListViewItem*, const QPoint&, int)), this, SLOT(slotDebitSelected(QListViewItem*, const QPoint&, int)));
  connect(creditListView, SIGNAL(clicked(QListViewItem*, const QPoint&, int)), this, SLOT(slotCreditSelected(QListViewItem*, const QPoint&, int)));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(finishClicked()));
  connect(buttonEdit, SIGNAL(clicked()), this, SLOT(editClicked()));

  doDifference();
}

KReconcileDlg::~KReconcileDlg()
{
}

void KReconcileDlg::clearReconcile()
{
	
  MyMoneyTransaction *temp_transaction;
 	for(temp_transaction = m_creditsQList.first();temp_transaction;temp_transaction = m_creditsQList.next())
  {
    if(temp_transaction->state() == MyMoneyTransaction::Reconciled)
		{
     	temp_transaction->setState(MyMoneyTransaction::Unreconciled);
		}
  }
 	for(temp_transaction = m_debitsQList.first();temp_transaction;temp_transaction = m_debitsQList.next())
  {
    if(temp_transaction->state() == MyMoneyTransaction::Reconciled)
		{
     	temp_transaction->setState(MyMoneyTransaction::Unreconciled);
		}
  }

}

void KReconcileDlg::loadLists(void)
{
  // Load the internal transaaction lists
  if (m_file.isInitialised())
    return;

  m_creditsQList.clear();
  m_debitsQList.clear();

//  QListIterator<MyMoneyTransaction> it = m_file.transactionIterator(m_bankIndex, m_accountIndex);
  unsigned int i=0;
  MyMoneyTransaction *transaction;
  for (i=0, transaction=m_accountIndex->transactionFirst(); transaction; transaction=m_accountIndex->transactionNext(), i++) {
//    MyMoneyTransaction *transaction = it.current();
    if (transaction->date()>m_endingDate)
      break;

    if (transaction->state()!=MyMoneyTransaction::Reconciled) {
      if (transaction->type() == MyMoneyTransaction::Debit) {
        transaction->setIndex(i);
        m_debitsQList.append(transaction);
      }
      else {
        transaction->setIndex(i);
        m_creditsQList.append(transaction);
      }
    }
  }
}

void KReconcileDlg::insertTransactions(void)
{
	KMyMoneySettings *p_settings = KMyMoneySettings::singleton();
  creditListView->clear();
  debitListView->clear();

  QListIterator<MyMoneyTransaction> it(m_debitsQList);
  for ( ; it.current(); ++it) {
    (void) new KReconcileListItem(debitListView, it.current());
  }

  QListIterator<MyMoneyTransaction> it2(m_creditsQList);
  for ( ; it2.current(); ++it2) {
    (void) new KReconcileListItem(creditListView, it2.current());
  }
}

void KReconcileDlg::slotDebitSelected(QListViewItem* item, const QPoint& p, int col)
{
    KReconcileListItem *reconcileItem = (KReconcileListItem*)item;

		double dblDebit = m_debitBalance.amount();
		double dblCleared = m_clearedBalance.amount();
    double dblItem = reconcileItem->transaction()->amount().amount();
    if(reconcileItem->isSelected())
		{
  			dblDebit += dblItem;
			dblCleared -= dblItem;
			reconcileItem->setReconciled(true);
			m_reconciledTransactions.append(reconcileItem->transaction());
		}
		else
		{
			dblDebit -= dblItem;
			dblCleared += dblItem;
			reconcileItem->setReconciled(false);
			m_reconciledTransactions.remove(reconcileItem->transaction());
    }
		m_debitBalance.setAmount(dblDebit);
		m_clearedBalance.setAmount(dblCleared);
  	MyMoneyMoney money;
    money.setAmount(dblDebit);
		QString text = i18n("Withdrawals: ");
		text += KGlobal::locale()->formatMoney(money.amount());
		totalDebitsLabel->setText(text);

    money.setAmount(dblCleared);
		text = i18n("Cleared Balance: ");
		text += KGlobal::locale()->formatMoney(money.amount());
		endingLabel->setText(text);

		doDifference();


}

void KReconcileDlg::slotCreditSelected(QListViewItem* item, const QPoint&, int)
{
    KReconcileListItem *reconcileItem = (KReconcileListItem*)item;

		double dblCredit = m_creditBalance.amount();
		double dblCleared = m_clearedBalance.amount();
    double dblItem = reconcileItem->transaction()->amount().amount();
    if(reconcileItem->isSelected())
		{
  		dblCredit += dblItem;
			dblCleared += dblItem;
			reconcileItem->setReconciled(true);
			m_reconciledTransactions.append(reconcileItem->transaction());
		}
		else
    {
			dblCredit -= dblItem;
			dblCleared -= dblItem;
			reconcileItem->setReconciled(false);
			m_reconciledTransactions.remove(reconcileItem->transaction());
    }
		m_creditBalance.setAmount(dblCredit);
    m_clearedBalance.setAmount(dblCleared);
  	MyMoneyMoney money;
    money.setAmount(dblCredit);
		QString text = i18n("Deposits: ");
		text += KGlobal::locale()->formatMoney(money.amount());
		totalCreditsLabel->setText(text);

    money.setAmount(dblCleared);
		text = i18n("Cleared Balance: ");
		text += KGlobal::locale()->formatMoney(money.amount());
		endingLabel->setText(text);

		doDifference();


}

void KReconcileDlg::doDifference(void)
{
  MyMoneyMoney previousMoney(m_previousBalance);
  MyMoneyMoney endingMoney(m_endingBalance);

  MyMoneyMoney l_enteredMoney;

  QListIterator<MyMoneyTransaction> it(m_reconciledTransactions);
  for ( ; it.current(); ++it) {
    MyMoneyTransaction *rt = it.current();
    if (rt->type() == MyMoneyTransaction::Credit)
      l_enteredMoney += rt->amount();
    else
      l_enteredMoney -= rt->amount();
  }
  MyMoneyMoney difference((m_previousBalance + m_clearedBalance)- m_endingBalance);

  QString text;
  text = i18n("Difference: ");
  text += KGlobal::locale()->formatMoney(difference.amount());
  differenceLabel->setText(text);
  if (difference.isZero())
    m_balanced = true;
  else
    m_balanced = false;
}

void KReconcileDlg::finishClicked(void)
{
  if (!m_balanced) {
    if ((KMessageBox::questionYesNo(this, i18n("Account did not balance, are you sure ?")))==KMessageBox::No) {
			clearReconcile();
      return;
    }
  }
	else
  {
   	
  }
  emit reconcileFinished(true);
}

void KReconcileDlg::updateData(void)
{
  // Simply reload the list clearing the status.
  qDebug("In updateData");
  m_reconciledTransactions.clear();
  m_debitsQList.clear();
  m_creditsQList.clear();

  loadLists();
  insertTransactions();
  doDifference();
}

void KReconcileDlg::cancelClicked()
{
	clearReconcile();
  emit reconcileFinished(true);
}

void KReconcileDlg::resetData(const MyMoneyMoney previousBal, const MyMoneyMoney endingBal, const QDate endingDate, const MyMoneyBank bankIndex, MyMoneyAccount *accountIndex, const MyMoneyFile file)
{

  m_reconciledTransactions.clear();
  m_debitsQList.clear();
  m_creditsQList.clear();

  m_balanced = false;

	m_file = file;
  m_bankIndex = bankIndex;
	m_accountIndex = accountIndex;
  m_endingBalance = endingBal;
  m_previousBalance = previousBal;
  m_clearedBalance.setAmount(0.0);
  m_debitBalance.setAmount(0.0);
  m_creditBalance.setAmount(0.0);
  m_endingDate = endingDate;
	
	totalCreditsLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	totalDebitsLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	previousLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	endingLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	differenceLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);


	
	MyMoneyMoney money(m_clearedBalance);
	QString text(i18n("Cleared Balance: "));
	text += KGlobal::locale()->formatMoney(money.amount());
	endingLabel->setText(text);
	
  money = m_endingBalance;
	text = i18n("Ending Balance: ");
	text += KGlobal::locale()->formatMoney(money.amount());
	previousLabel->setText(text);

  money = m_creditBalance;
	text = i18n("Deposits: ");
	text += KGlobal::locale()->formatMoney(money.amount());
	totalCreditsLabel->setText(text);
	
  money = m_debitBalance;
	text = i18n("Withdrawals: ");
	text += KGlobal::locale()->formatMoney(money.amount());
	totalDebitsLabel->setText(text);


	loadLists();
	insertTransactions();

}
/** No descriptions */
/** No descriptions */
void KReconcileDlg::slotTransactionChanged(){


	reloadLists();
	insertTransactions();
	show();


}
/** No descriptions */
void KReconcileDlg::reloadLists(){
  // Load the internal transaaction lists
  if (m_file.isInitialised())
    return;


//  QListIterator<MyMoneyTransaction> it = m_file.transactionIterator(m_bankIndex, m_accountIndex);
  unsigned int i=0;
  MyMoneyTransaction *transaction;
  for (i=0, transaction=m_accountIndex->transactionFirst(); transaction; transaction=m_accountIndex->transactionNext(), i++) {
//    MyMoneyTransaction *transaction = it.current();

    if (transaction->state()!=MyMoneyTransaction::Reconciled) {
      if (transaction->type() == MyMoneyTransaction::Debit) {
        transaction->setIndex(i);
		if(m_debitsQList.find(transaction) <  0)
    	{
        	m_debitsQList.append(transaction);
		}
      }
      else {
        transaction->setIndex(i);
		if(m_creditsQList.find(transaction) <  0)
		{
        	m_creditsQList.append(transaction);
		}
      }
    }
  }


  QListIterator<MyMoneyTransaction> it(m_debitsQList);
  for ( ; it.current(); ++it) {
	bool transactionFound = inTransactions(it.current());
	if(transactionFound == false)
		m_debitsQList.remove(it.current());
  }

  QListIterator<MyMoneyTransaction> it2(m_creditsQList);
  for ( ; it2.current(); ++it2) {
	bool transactionFound = inTransactions(it2.current());
	if(transactionFound == false)
		m_debitsQList.remove(it2.current());
  }


}


/** No descriptions */
bool KReconcileDlg::inTransactions(MyMoneyTransaction *credittrans){

  MyMoneyTransaction *transaction;
     int i = 0;
 	 for ( i=0, transaction=m_accountIndex->transactionFirst(); transaction; transaction=m_accountIndex->transactionNext(), i++) {
    	if( credittrans == transaction)
			return true;  	
	}
	return false;

}

/** No descriptions */
bool KReconcileDlg::inCredits(MyMoneyTransaction *transaction){

   QListIterator<MyMoneyTransaction> it(m_creditsQList);
  for ( ; it.current(); ++it) {
	if(transaction == it.current())
		return true;
  }

	return false;

}
/** No descriptions */
bool KReconcileDlg::inDebits(MyMoneyTransaction *transaction){

  QListIterator<MyMoneyTransaction> it(m_debitsQList);
  for ( ; it.current(); ++it) {
	if(transaction == it.current())
		return true;
  }

	return false;

}
/** No descriptions */
void KReconcileDlg::editClicked(){

	hide();
}
