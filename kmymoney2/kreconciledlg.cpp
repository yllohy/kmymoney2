/***************************************************************************
                          kreconciledlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
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

KReconcileDlg::KReconcileDlg(const MyMoneyMoney previousBal, const MyMoneyMoney endingBal, const QDate endingDate, const MyMoneyBank bankIndex, const MyMoneyAccount accountIndex, const MyMoneyFile file, QWidget *parent, const char *name)
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
	debitListView->addColumn(i18n("Description"));
	debitListView->addColumn(i18n("Amount"));
	debitListView->addColumn(i18n("C"));
	debitListView->setMultiSelection(true);
  debitListView->setAllColumnsShowFocus(true);
	
	creditListView->setRootIsDecorated(false);
	creditListView->addColumn(i18n("Date"));
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

  doDifference();
}

KReconcileDlg::~KReconcileDlg()
{
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
  for (i=0, transaction=m_accountIndex.transactionFirst(); transaction; transaction=m_accountIndex.transactionNext(), i++) {
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
//  m_debitsList->setCellFont(p_settings->lists_listCellFont());
//  m_debitsList->setHeaderFont(p_settings->lists_listHeaderFont());
//  m_creditsList->setCellFont(p_settings->lists_listCellFont());
//  m_creditsList->setHeaderFont(p_settings->lists_listHeaderFont());
//  m_defaultBGColor = p_settings->lists_listBGColor();
//  m_defaultListColor = p_settings->lists_listColor();

  // Loop through the two lists and insert the items
//  m_lastCount=0;
//  QString cellText;
//  QdbtTableCell tableCell;
  creditListView->clear();
  debitListView->clear();

  QListIterator<MyMoneyTransaction> it(m_debitsQList);
  for ( ; it.current(); ++it) {
//    m_debitsList->setAutoUpdate(false);
//    QColor color;
//    if (m_lastCount>=2)
//      m_lastCount=0;

//    if (m_lastCount==0)
//      color = m_defaultListColor;
//    else
//      color = m_defaultBGColor;
//    m_lastCount++;

//    MyMoneyTransactionE *transaction = it.current();
    (void) new KReconcileListItem(debitListView, it.current());
  }

//  m_lastCount=0;

  // Now do the credits list
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
		}
		else
		{
			dblDebit -= dblItem;
			dblCleared += dblItem;
			reconcileItem->setReconciled(false);
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

//  static bool bState=false;

//  if (bState) {
/*
//    m_debitsList->setRowSelected(row, false);
    KReconcileListItem *reconcileItem = (KReconcileListItem*)item;
    if (reconcileItem->transaction().state() == MyMoneyTransaction::Reconciled) {
      MyMoneyTransaction *rt;
      for ( rt=m_reconciledTransactions.first(); rt!=0; rt=m_reconciledTransactions.next()) {
        if (rt->index() == (findTransaction(m_debitsQList, *reconcileItem->transaction())->index())) {
          m_reconciledTransactions.remove(m_reconciledTransactions.at());
          break;
        }
      }
      m_debitsQList.at(row)->setState(MyMoneyTransaction::Unreconciled); // Removes old state ????
      reconcileItem->changeState(MyMoneyTransaction::Unreconciled);
      bState=true;
    }
    else {
      MyMoneyTransaction *rt = new MyMoneyTransaction;
      rt = m_debitsQList.at(row);
      rt->setIndex(m_debitsQList.at(row)->index());
      m_reconciledTransactions.append(rt);
      QdbtTableCell c(*m_debitsList->cell(row, 3));
      c.setText("R");
      m_debitsList->changeCell(&c, row, 3);
      m_debitsQList.at(row)->setState(MyMoneyTransaction::Reconciled);
      bState=false;
    }
    doDifference();
  }
*/
 /*
    MyMoneyMoney l_totalDebits;
    MyMoneyTransaction *rt;
    for ( rt=m_reconciledTransactions.first(); rt!=0; rt=m_reconciledTransactions.next()) {
      if (rt->type()==MyMoneyTransaction::Debit)
        l_totalDebits += rt->amount();
    }

    MyMoneyMoney money;
    QString text;

    text = i18n("Total: ");
    text += KGlobal::locale()->formatMoney(l_totalDebits.amount());
    totalDebitsLabel->setText(text);
  */
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
		}
		else
    {
			dblCredit -= dblItem;
			dblCleared -= dblItem;
			reconcileItem->setReconciled(false);
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

/*
  if (state) {
    m_creditsList->setRowSelected(row, false);
    if (m_creditsQList.at(row)->state() == MyMoneyTransaction::Reconciled) {
      MyMoneyTransactionE *rt;
      for ( rt=m_reconciledTransactions.first(); rt!=0; rt=m_reconciledTransactions.next()) {
        if (rt->index() == m_creditsQList.at(row)->index()) {
          m_reconciledTransactions.remove(m_reconciledTransactions.at());
          break;
        }
      }
      m_creditsQList.at(row)->setState(MyMoneyTransaction::Unreconciled); // Removes old state ????
      QdbtTableCell c(*m_creditsList->cell(row, 3));
      c.setText("");
      m_creditsList->changeCell(&c, row, 3);
//      m_creditsList->changeCell(" ", row, 3, black, AlignCenter, false);
    }
    else {
      MyMoneyTransactionE *rt = new MyMoneyTransactionE;
      rt = m_creditsQList.at(row);
      rt->setIndex(m_creditsQList.at(row)->index());
      m_reconciledTransactions.append(rt);
      QdbtTableCell c(*m_creditsList->cell(row, 3));
      c.setText("R");
      m_creditsList->changeCell(&c, row, 3);
//      m_creditsList->changeCell("R", row, 3, black, AlignCenter, false);
      m_creditsQList.at(row)->setState(MyMoneyTransaction::Reconciled);
    }
    doDifference();
  }

  MyMoneyMoney l_totalCredits;
  MyMoneyTransactionE *rt;
  for ( rt=m_reconciledTransactions.first(); rt!=0; rt=m_reconciledTransactions.next()) {
    if (rt->type()==MyMoneyTransaction::Credit)
      l_totalCredits += rt->amount();
  }

  QString text;

  text = i18n("Total: ");
  text += KGlobal::locale()->formatMoney(l_totalCredits.amount());
  totalCreditsLabel->setText(text);
*/
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
      return;
    }
  }
	else
  {
   	
  }
/*
  // Update the real list to reflect the reconciliation
  MyMoneyTransactionE *rt;
  for ( rt=m_reconciledTransactions.first(); rt!=0; rt=m_reconciledTransactions.next()) {
    if (rt->index()<0) {
      emit reconcileFinished(false);
      return;
    }
    MyMoneyTransaction *transaction = m_file->transaction(rt->index(), m_bankIndex, m_accountIndex);
    if ((transaction->amount() != rt->amount()) || (transaction->type() != rt->type()) || (transaction->date() != rt->date()) || (transaction->memo()!=rt->memo())) {
      emit reconcileFinished(false);
      return;
    }
     // If weve got here then it's alright to set the new reconciled status
     transaction->setState(MyMoneyTransaction::Reconciled);
  }

  m_file->account(m_accountIndex, m_bankIndex)->setLastReconcile(m_endingDate);
*/
//  delete m_creditsList;
//	m_creditsList=0;
//	delete m_debitsList;
//	m_debitsList=0;
  emit reconcileFinished(true);
}

void KReconcileDlg::updateData(void)
{
  // Simply reload the list clearing the status.
  m_reconciledTransactions.clear();
  m_debitsQList.clear();
  m_creditsQList.clear();

  loadLists();
  insertTransactions();
  doDifference();
}

void KReconcileDlg::cancelClicked()
{
  emit reconcileFinished(true);
}

void KReconcileDlg::resetData(const MyMoneyMoney previousBal, const MyMoneyMoney endingBal, const QDate endingDate, const MyMoneyBank bankIndex, const MyMoneyAccount accountIndex, const MyMoneyFile file)
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

/*	m_file = file;
	m_bankIndex = bankIndex;
	m_accountIndex = accountIndex;
  m_endingBalance = endingBal;
  m_previousBalance = previousBal;
  m_endingDate = endingDate;
	
	MyMoneyMoney money(m_endingBalance);
	QString text(i18n("Ending Balance: "));
	text += KGlobal::locale()->formatMoney(money.amount());
	endingLabel->setText(text);
	
  money = m_previousBalance;
	text = i18n("Previous Balance: ");
	text += KGlobal::locale()->formatMoney(money.amount());
	previousLabel->setText(text);
/*	
  // WHY WHY WHY do I need to recreate the QdbtTabulars ???
  delete m_creditsList;
  delete m_debitsList;
	m_creditsList = new QdbtTabular(creditsGroup, "creditsList");
	m_creditsList->setGeometry(10, 20, 290, 170);
	m_debitsList = new QdbtTabular(debitsGroup, "debitsList");
	m_debitsList->setGeometry(10, 20, 290, 170);
	KMyMoneySettings *p_settings = KMyMoneySettings::singleton();
  m_debitsList->setCellFont(p_settings->lists_listCellFont());
  m_debitsList->setHeaderFont(p_settings->lists_listHeaderFont());
  m_creditsList->setCellFont(p_settings->lists_listCellFont());
  m_creditsList->setHeaderFont(p_settings->lists_listHeaderFont());
  m_defaultBGColor = p_settings->lists_listBGColor();
  m_defaultListColor = p_settings->lists_listColor();

  m_creditsList->selectByRow(true); // default
  m_debitsList->selectByRow(true);
  m_creditsList->setMultiSelect(false);
  m_debitsList->setMultiSelect(false);

  // Connections are automatically removed when objects are deleted so we can just
  // reconnect again
  connect(m_debitsList, SIGNAL(selected(int, bool)), this, SLOT(slotDebitSelected(int, bool)));
  connect(m_creditsList, SIGNAL(selected(int, bool)), this, SLOT(slotCreditSelected(int, bool)));

  // WHY can't I just do this
  debitListView->clear();
  creditListView->clear();

  m_reconciledTransactions.clear();
  m_debitsQList.clear();
  m_creditsQList.clear();
	
	loadLists();
	insertTransactions();
  doDifference();
*/
}

#include "kreconciledlg.moc"
