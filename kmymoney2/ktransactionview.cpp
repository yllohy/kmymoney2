/***************************************************************************
                          ktransactionview.cpp
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
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>

#include <qpushbutton.h>
#include <kcombobox.h>
#include <qtabwidget.h>
#include <qtable.h>
#include <kpopupmenu.h>
#include <qinputdialog.h>

#include "ktransactionview.h"
#include "knewcategorydlg.h"
#include <kmessagebox.h>

KTransactionView::KTransactionView(QWidget *parent, const char *name)
 : KTransactionViewDecl(parent,name)
{
  transactionsTable->setNumCols(7);
  transactionsTable->horizontalHeader()->setLabel(0, i18n("Date"));
	transactionsTable->horizontalHeader()->setLabel(1, i18n("Type/Num"));
	transactionsTable->horizontalHeader()->setLabel(2, i18n("Payee/Category/Memo"));
	transactionsTable->horizontalHeader()->setLabel(3, i18n("Clr"));
	transactionsTable->horizontalHeader()->setLabel(4, i18n("Payment"));
	transactionsTable->horizontalHeader()->setLabel(5, i18n("Deposit"));
	transactionsTable->horizontalHeader()->setLabel(6, i18n("Balance"));
	transactionsTable->setSelectionMode(QTable::NoSelection);
 	transactionsTable->setLeftMargin(0);
	transactionsTable->verticalHeader()->hide();
  transactionsTable->setColumnStretchable(0, false);
  transactionsTable->setColumnStretchable(1, false);
	transactionsTable->setColumnStretchable(2, false);
  transactionsTable->setColumnStretchable(3, false);
	transactionsTable->setColumnStretchable(4, false);
  transactionsTable->setColumnStretchable(5, false);
	transactionsTable->setColumnStretchable(6, false);
		
	transactionsTable->horizontalHeader()->setResizeEnabled(false);
	transactionsTable->horizontalHeader()->setMovingEnabled(false);
  int w=transactionsTable->width();
  transactionsTable->setColumnWidth(0, 100);
  transactionsTable->setColumnWidth(1, 100);
  transactionsTable->setColumnWidth(2, w-530-20);
  transactionsTable->setColumnWidth(3, 30);
  transactionsTable->setColumnWidth(4, 100);
  transactionsTable->setColumnWidth(5, 100);
  transactionsTable->setColumnWidth(6, 100);

	KConfig *config = KGlobal::config();
  QFont defaultFont = QFont("helvetica", 12);
  transactionsTable->horizontalHeader()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));
	
  m_filePointer=0;

  connect(transactionsTable, SIGNAL(clicked(int, int, int, const QPoint&)),
    this, SLOT(slotFocusChange(int, int, int, const QPoint&)));

  connect(viewTypeCombo, SIGNAL(activated(int)), this, SLOT(viewTypeActivated(int)));

  m_index = -1;

  createInputWidgets();
}

KTransactionView::~KTransactionView()
{
}

void KTransactionView::slotPayeeCompleted()
{
	if(m_payee->currentText() != "")
	{
   QString payee = m_payee->currentText();
   MyMoneyAccount *account;

	 account = getAccount();
	 if(account == 0)
		return;

  	MyMoneyTransaction *transaction;
		MyMoneyTransaction *lasttransaction = 0;

  	for ( transaction = account->transactionFirst(); transaction; transaction=account->transactionNext() )
		{
			if(transaction->payee() == payee)
			{
       	lasttransaction = transaction;
			}			
			
		}
		if(lasttransaction != 0)
		{
  		m_payment->setText(((lasttransaction->type()==MyMoneyTransaction::Debit) ? KGlobal::locale()->formatNumber(lasttransaction->amount().amount()) : QString("")));
  		m_withdrawal->setText(((lasttransaction->type()==MyMoneyTransaction::Credit) ? KGlobal::locale()->formatNumber(lasttransaction->amount().amount()) : QString("")));
  		bool categorySet = false;
  		for(int i = 0; i < m_category->count(); i++)
  		{
				QString theText;
				if(lasttransaction->categoryMinor() == "")
				{
					theText.sprintf("%s", lasttransaction->categoryMajor().latin1());
				}
				else
				{
					theText.sprintf("%s:%s",lasttransaction->categoryMajor().latin1(),lasttransaction->categoryMinor().latin1());
				}
   			if(m_category->text(i) == theText)
				{
					m_category->setCurrentItem(i);
     			categorySet = true;
				}
			}
  		if(!categorySet)
			{
   			m_category->setCurrentItem(0);
			}
		}		
	}

}

void KTransactionView::slotMethodCompleted()
{
	if(m_method->currentText() == "Cheque")
	{

   MyMoneyAccount *account;

	 account = getAccount();
	 if(account == 0)
		 return;

  	MyMoneyTransaction *transaction;
		QStringList payeelist;
		QString oldnumber = "";
		QString number = "";
		lastcheck = 0;
		long lastnumber = 0;

  	for ( transaction = account->transactionFirst(); transaction; transaction=account->transactionNext() )
		{
			
   		number = transaction->number();
			if(number != "")
			{
        lastnumber = number.toLong();
				if(lastnumber > lastcheck)
				{
         	lastcheck = lastnumber;
				}
			}
			
		}
		lastcheck++;
		number.setNum(lastcheck);
		m_number->setText(number);
		
	}

}

void KTransactionView::createInputWidgets()
{

	m_date = new kMyMoneyDateInput(0,QDate::currentDate());
	m_method = new kMyMoneyCombo(0);
  m_payee = new kMyMoneyCombo(0);
  m_payment = new kMyMoneyEdit(0);
  m_withdrawal = new kMyMoneyEdit(0);
	m_number = new kMyMoneyLineEdit(0);
//	m_hlayout = new kMyMoneyHLayout(0);
	m_category = new kMyMoneyCombo(0);
	m_memo = new kMyMoneyLineEdit(0);
//	m_hlayout->addWidget(m_category);
//	m_hlayout->addWidget(m_memo);
  m_enter = new KPushButton("Enter",0);
  m_cancel = new KPushButton("Cancel",0);
  m_delete = new KPushButton("Delete",0);
  m_method->setEditable(false);
  m_method->setAutoCompletion(true);
  KCompletion *methodcomp = m_method->completionObject();
  connect(m_method,SIGNAL(returnPressed(const QString&)),methodcomp,SLOT(addItem(const QString&)));
  m_payee->setEditable(true);
  m_payee->setAutoCompletion(true);
  m_number->setHandleSignals(false);
  m_number->setKeyBinding(KCompletionBase::TextCompletion, Qt::Key_End );
  m_number->setContextMenuEnabled(false);
  m_number->setEnableSignals(false);
  m_number->useGlobalKeyBindings();
  m_number->setAlignment(Qt::AlignLeft);
  m_payment->setHandleSignals(false);
  m_payment->setKeyBinding(KCompletionBase::TextCompletion, Qt::Key_End );
  m_payment->setContextMenuEnabled(false);
  m_payment->setEnableSignals(false);
  m_payment->useGlobalKeyBindings();
  m_payment->setAlignment(Qt::AlignRight);
  m_withdrawal->setHandleSignals(false);
  m_withdrawal->setKeyBinding(KCompletionBase::TextCompletion, Qt::Key_End );
  m_withdrawal->setContextMenuEnabled(false);
  m_withdrawal->setEnableSignals(false);
  m_withdrawal->useGlobalKeyBindings();
  m_withdrawal->setAlignment(Qt::AlignRight);
//	m_method->insertItem("");  // We don't need a blank item.  These will be dynamic in the future
  m_method->insertItem("Cheque");
  m_method->insertItem("Deposit");
  m_method->insertItem("Transfer");
  m_method->insertItem("Withdrawal");
  m_method->insertItem("ATM");
  m_method->setEditable(true);
  m_category->setAutoCompletion(true);
  KCompletion *categorycomp = m_category->completionObject();
  connect(m_category,SIGNAL(returnPressed(const QString&)),categorycomp,SLOT(addItem(const QString&)));
	hideWidgets();

  connect(m_method, SIGNAL(signalFocusOut()),
          this, SLOT(slotMethodCompleted()));
  connect(m_payee, SIGNAL(signalFocusOut()),
          this, SLOT(slotPayeeCompleted()));
	connect(m_payment,SIGNAL(signalEnter()),
				  this, SLOT(enterClicked()));
	connect(m_withdrawal,SIGNAL(signalEnter()),
				  this, SLOT(enterClicked()));
	connect(m_number,SIGNAL(signalEnter()),
				  this, SLOT(enterClicked()));
	connect(m_category,SIGNAL(signalEnter()),
				  this, SLOT(enterClicked()));
	connect(m_payment,SIGNAL(signalNextTransaction()),this,SLOT(slotNextTransaction()));
	connect(m_withdrawal,SIGNAL(signalNextTransaction()),this,SLOT(slotNextTransaction()));
	connect(m_category,SIGNAL(signalNextTransaction()),this,SLOT(slotNextTransaction()));
	connect(m_cancel, SIGNAL(clicked()),this,SLOT(cancelClicked()));
	connect(m_enter, SIGNAL(clicked()),this,SLOT(enterClicked()));
	connect(m_delete, SIGNAL(clicked()),this,SLOT(deleteClicked()));

}

void KTransactionView::loadPayees()
{
  MyMoneyAccount *account;

	account = getAccount();
	if(account == 0)
		return;

  MyMoneyMoney balance;
  MyMoneyTransaction *transaction;
	QStringList payeelist;

  for ( transaction = account->transactionFirst(); transaction; transaction=account->transactionNext() )
	{
   	QString payee = transaction->payee();
		bool inPayee = false;
    for(QStringList::Iterator it = payeelist.begin(); it != payeelist.end(); ++it)
		{
			if((*it).latin1() == payee)
			{
       			inPayee = true;
			}     	
		}
		if(inPayee == false)
		{
    	payeelist.append(payee); 	
		}
	}
	// Load Payees from the Payee List
 	 QListIterator<MyMoneyPayee> payeeit = m_filePointer->payeeIterator();
  	for ( ; payeeit.current(); ++payeeit) {
   		QString payee = payeeit.current()->name();
		bool inPayee = false;
    	for(QStringList::Iterator it = payeelist.begin(); it != payeelist.end(); ++it)
		{
			if((*it).latin1() == payee)
			{
       			inPayee = true;
			}     	
		}
		if(inPayee == false)
		{
    	payeelist.append(payee); 	
		}

  	}

	payeelist.sort();
	m_payee->clear();
  m_payee->insertStringList(payeelist);

}

void KTransactionView::slotFocusChange(int row, int col, int button, const QPoint&  point)
{
   if(m_date->isVisible() || ( m_viewType != NORMAL))
		return;
	
	if (row > (transactionsTable->numRows()-3))  // make sure there is room for the input widgets
	  return;
	
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  const int NO_ROWS = (config->readEntry("RowCount", "2").toInt());
	
	int transrow = row / NO_ROWS;
  int realrow = transrow * NO_ROWS;
	m_currentrow = realrow;
	m_currentcol = col;
	m_currentbutton = button;
	m_currentpos = point;
  if ((transrow != transactionsTable->numRows()-1) && (transactionsTable->numRows()>=1)) {
		if(button == 1) {
      if(m_transactions->count() > transrow)
      {
       	switch (m_transactions->at(transrow)->method()) {
        	case MyMoneyTransaction::Cheque:
						m_method->setCurrentItem(0);
          	break;
        	case MyMoneyTransaction::Deposit:
						m_method->setCurrentItem(1);
          	break;
        	case MyMoneyTransaction::Transfer:
						m_method->setCurrentItem(2);
          	break;
        	case MyMoneyTransaction::Withdrawal:
						m_method->setCurrentItem(3);
          	break;
        	case MyMoneyTransaction::ATM:
						m_method->setCurrentItem(4);
          	break;		
      	}
			}
      transactionsTable->setCellWidget(realrow, 0,m_date);
	  //m_date->setGeometry(transactionsTable->cellGeometry(realrow,0));
      m_date->show();
      transactionsTable->setCellWidget(realrow ,1,m_method);
	  //m_method->setGeometry(transactionsTable->cellGeometry(realrow,1));
      m_method->show();
      transactionsTable->setCellWidget(realrow ,2,m_payee);
	  //m_payee->setGeometry(transactionsTable->cellGeometry(realrow,2));
      m_payee->show();
      transactionsTable->setCellWidget(realrow ,4,m_payment);
	  //m_payment->setGeometry(transactionsTable->cellGeometry(realrow,4));
      m_payment->show();
      transactionsTable->setCellWidget(realrow ,5,m_withdrawal);
	  //m_withdrawal->setGeometry(transactionsTable->cellGeometry(realrow,5));
      m_withdrawal->show();
      transactionsTable->setCellWidget(realrow + 1 ,1,m_number);
	  //m_number->setGeometry(transactionsTable->cellGeometry(realrow + 1,1));
    //  transactionsTable->setCellWidget(realrow + 1 ,2,m_hlayout);
      transactionsTable->setCellWidget(realrow + 1, 2, m_category);
      m_category->show();
      transactionsTable->setCellWidget(realrow + 2, 2, m_memo);
      m_memo->show();
	
	   //m_hlayout->setGeometry(transactionsTable->cellGeometry(realrow + 1,2));
      m_number->show();
//		m_hlayout->show();
      //m_category->show();
	  m_memo->show();
      transactionsTable->setCellWidget(realrow + 1 ,4,m_enter);
	  //m_enter->setGeometry(transactionsTable->cellGeometry(realrow + 1,4));
      m_enter->show();
      transactionsTable->setCellWidget(realrow + 1 ,5,m_cancel);
	  //m_cancel->setGeometry(transactionsTable->cellGeometry(realrow + 1,5));
      m_cancel->show();
      transactionsTable->setCellWidget(realrow + 1 ,6,m_delete);
	  //m_delete->setGeometry(transactionsTable->cellGeometry(realrow + 1,6));
      m_delete->show();
      updateInputLists();
      if(m_transactions->count() > transrow)
      {
	      setInputData(*m_transactions->at(transrow));
			}
			else
			{
       	clearInputData();
			}
//     	viewMode();

    }
    m_index = transrow;

    if (button>=2) {
      m_index=transrow;
      KPopupMenu setAsMenu(i18n("Set As..."), this);
      setAsMenu.insertItem(i18n("Unreconciled (default)"), this, SLOT(slotTransactionUnReconciled()));
      setAsMenu.insertItem(i18n("Cleared"), this, SLOT(slotTransactionCleared()));

      KPopupMenu menu(i18n("Transaction Options"), this);
      menu.insertItem(i18n("Delete..."), this, SLOT(slotTransactionDelete()));
      menu.insertSeparator();
      menu.insertItem(i18n("Set as"), &setAsMenu);
      menu.exec(QCursor::pos());
    }
  } else
    m_index=-1;

}

void KTransactionView::slotTransactionDelete()
{
  MyMoneyAccount *pAccount;

	pAccount = getAccount();
	if(pAccount == 0)
		return;

	QDate transdate;
	MyMoneyMoney transamount;
	QString transcategory;
	
	
  QString prompt;
  MyMoneyTransaction *transaction = m_transactions->at(m_index);
  if (!transaction)
    return;


  prompt.sprintf(i18n("Delete this transaction ? :-\n%s"), transaction->payee().latin1());
  if ((KMessageBox::questionYesNo(this, prompt))==KMessageBox::No)
    return;

	transdate = transaction->date();
	transamount = transaction->amount();
	transcategory = transaction->categoryMajor();

	int lessindex = m_category->currentText().find("<");
  int greatindex = m_category->currentText().find(">");
	QString transferAccount = "";
  if((lessindex != -1) && (greatindex != -1) )
  {
  	transferAccount =  transcategory;
		transferAccount = transferAccount.remove(0,1);
		transferAccount = transferAccount.remove(transferAccount.length() - 1,1);
    MyMoneyAccount *currentAccount;
    for(currentAccount = m_bankIndex.accountFirst(); currentAccount != 0; currentAccount = m_bankIndex.accountNext())
    {
			if(currentAccount->accountName() == transferAccount)
      {
				MyMoneyTransaction *currentTransaction;
				for(currentTransaction = currentAccount->transactionFirst(); currentTransaction != 0; currentTransaction = currentAccount->transactionNext())
				{
					QString matchCategory = "";
					matchCategory.sprintf("<%s>",pAccount->accountName().latin1());
					if(currentTransaction->date().toString() == transdate.toString() &&
             currentTransaction->amount().amount() == transamount.amount() &&
						 currentTransaction->categoryMajor() == matchCategory)
					{
						currentAccount->removeTransaction(*currentTransaction);
					}
				}
			}
		}		
	}

  pAccount->removeTransaction(*transaction);
  m_filePointer->setDirty(true);

  useall = false;
  usedate = true;
  userow = false;
  updateTransactionList(-1, -1);
  emit transactionListChanged();
}

void KTransactionView::slotTransactionUnReconciled()
{
  MyMoneyAccount *pAccount;

  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  transactionsTable->horizontalHeader()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));
  const int NO_ROWS = (config->readEntry("RowCount", "2").toInt());
	
	pAccount = getAccount();
	if(pAccount == 0)
		return;

  MyMoneyTransaction *transaction = m_transactions->at(m_index);
  if (!transaction)
    return;

  transaction->setState(MyMoneyTransaction::Unreconciled);
  updateTransactionList(m_index*NO_ROWS, 3);
  m_filePointer->setDirty(true);
}

void KTransactionView::slotTransactionCleared()
{
  MyMoneyAccount *pAccount;

  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  transactionsTable->horizontalHeader()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));
  const int NO_ROWS = (config->readEntry("RowCount", "2").toInt());
	
	pAccount = getAccount();
	if(pAccount == 0)
		return;

  MyMoneyTransaction *transaction = m_transactions->at(m_index);
  if (!transaction)
    return;

  transaction->setState(MyMoneyTransaction::Cleared);
  updateTransactionList(m_index*NO_ROWS, 3);
  m_filePointer->setDirty(true);
}

void KTransactionView::init(MyMoneyFile *file, MyMoneyBank bank, MyMoneyAccount account,
  QList<MyMoneyTransaction> *transList, viewingType type)
{
  m_filePointer = file;
  m_bankIndex = bank;
  m_accountIndex = account;
  m_transactions = transList;

  switch (type) {
    case NORMAL:
      viewTypeCombo->setCurrentItem(0);
      break;
    case SUBSET:
      viewTypeCombo->setCurrentItem(1);
      break;
    default:
      viewTypeCombo->setCurrentItem(0);
      break;
  }
  m_viewType=type;

 clear();
 useall = true;
 usedate = false;
 userow = false;
 updateTransactionList(-1);
}

void KTransactionView::clear(void)
{
  for (int i=0; i<transactionsTable->numRows(); i++)
    for (int j=0; j<=6; j++)
      transactionsTable->setText(i, j, "");
  transactionsTable->setNumRows(0);
}

void KTransactionView::enterClicked()
{

	hideWidgets();
  if (!m_filePointer)
    return;

  MyMoneyBank *bank;
  MyMoneyAccount *account;

  bank = m_filePointer->bank(m_bankIndex);
  if (!bank) {
    qDebug("unable to find bank in updateData");
    return;
  }

  account = bank->account(m_accountIndex);
  if (!account) {
    qDebug("Unable to find account in updateData");
    return;
  }
  MyMoneyMoney balance;
//  MyMoneyTransaction *transaction;

	MyMoneyTransaction::transactionMethod newmethod;
  double dblnewamount;
  QDate newdate = m_date->getQDate();
	QString newcategory = m_category->currentText();
  int commaindex;

    // Get amount from payment or withdrawal Line Edit
	if(m_payment->text() == "")
	{
    commaindex = m_withdrawal->text().find(",");
		if(commaindex != -1)
			dblnewamount = m_withdrawal->text().remove(commaindex,1).toDouble();
		else
			dblnewamount = m_withdrawal->text().toDouble();
  	dblnewamount = dblnewamount;
	}
  else if(m_withdrawal->text() == "")
	{
		commaindex = m_payment->text().find(",");
		if(commaindex != -1)
			dblnewamount = m_payment->text().remove(commaindex,1).toDouble();
		else
			dblnewamount = m_payment->text().toDouble();

  	dblnewamount = dblnewamount;
	}
	else
	{
   	dblnewamount = 0;
	}

  	MyMoneyMoney newamount(dblnewamount);
	MyMoneyTransaction::stateE newstate;
	
	// Set the transaction type
	if(m_method->currentItem() == 0)
	{
   	 newmethod = MyMoneyTransaction::Cheque;
	}
	else if(m_method->currentItem() == 1)
	{
   	 newmethod = MyMoneyTransaction::Deposit;
	}
	else if(m_method->currentItem() == 2)
	{
   	 newmethod = MyMoneyTransaction::Transfer;
	}
	else if(m_method->currentItem() == 3)
	{
   	 newmethod = MyMoneyTransaction::Withdrawal;
	}
	else if(m_method->currentItem() == 4)
	{
   	 newmethod = MyMoneyTransaction::ATM;
	}
	else
	{
   	 newmethod = MyMoneyTransaction::Cheque;
	}
    // Add payee to Payee List
	m_filePointer->addPayee(m_payee->currentText());
  	int colonindex = m_category->currentText().find(":");
  	QString catmajor;
	QString catminor;
  if(colonindex == -1)
	{
   	catmajor = m_category->currentText();
		catminor = "";
	}
	else
	{
		int len = m_category->currentText().length();
		len--;
   	catmajor = m_category->currentText().left(colonindex);
		catminor = m_category->currentText().right(len - colonindex);
	}


	int lessindex = m_category->currentText().find("<");
  int greatindex = m_category->currentText().find(">");
	QString transferAccount = "";

	if(m_index < m_transactions->count())
	{
		newstate = m_transactions->at(m_index)->state();
    MyMoneyTransaction *transaction = m_transactions->at(m_index);
    if (!transaction)
      return;


	  QDate transdate;
	  MyMoneyMoney transamount;
	  QString transcategory;

	  transdate = transaction->date();
	  transamount = transaction->amount();
	  transcategory = transaction->categoryMajor();

	  QString transferAccount = "";
    if((lessindex != -1) && (greatindex != -1) )
    {
  	  transferAccount =  transcategory;
		  transferAccount = transferAccount.remove(0,1);
		  transferAccount = transferAccount.remove(transferAccount.length() - 1,1);
      MyMoneyAccount *currentAccount;
      for(currentAccount = m_bankIndex.accountFirst(); currentAccount != 0; currentAccount = m_bankIndex.accountNext())
      {
			  if(currentAccount->accountName() == transferAccount)
        {
				  MyMoneyTransaction *currentTransaction;
				  for(currentTransaction = currentAccount->transactionFirst(); currentTransaction != 0; currentTransaction = currentAccount->transactionNext())
				  {
					  QString matchCategory = "";
					  matchCategory.sprintf("<%s>",account->accountName().latin1());
					  if(currentTransaction->date().toString() == transdate.toString() &&
             currentTransaction->amount().amount() == transamount.amount() &&
						 currentTransaction->categoryMajor() == matchCategory)
					  {
						  currentAccount->removeTransaction(*currentTransaction);
					  }
				  }
			  }
		  }		
	  }
   	account->removeTransaction(*m_transactions->at(m_index));
  	account->addTransaction(newmethod, m_number->text(), m_memo->text(),
                                            newamount, newdate, catmajor, catminor, "",
  											    m_payee->currentText(), "", "", newstate);
	}
	else
  {
		newstate = MyMoneyTransaction::Unreconciled;
  	account->addTransaction(newmethod, m_number->text(), m_memo->text(),
                            newamount, newdate, catmajor, catminor, "",
  													m_payee->currentText(), "", "", newstate);
	}
  qDebug("Transaction Added");
  if((lessindex != -1) && (greatindex != -1) && (m_method->currentItem() == 2))
  {
  	transferAccount =  m_category->currentText();
		transferAccount = transferAccount.remove(0,1);
		transferAccount = transferAccount.remove(transferAccount.length() - 1,1);
    MyMoneyAccount *currentAccount;
    for(currentAccount = m_bankIndex.accountFirst(); currentAccount != 0; currentAccount = m_bankIndex.accountNext())
    {
			if(currentAccount->accountName() == transferAccount)
      {
				MyMoneyTransaction::transactionMethod transfermethod = MyMoneyTransaction::Deposit;
				QString theText;
	     	theText.sprintf("<%s>",account->accountName().latin1());
				newstate = MyMoneyTransaction::Unreconciled;
				currentAccount->addTransaction(transfermethod, m_number->text(), m_memo->text(),
																			 newamount, newdate, theText, "", "",
																			 m_payee->currentText(),"" ,"",
																			 newstate);
			}
		}		
	}
  if((lessindex != -1) && (greatindex != -1) && (m_method->currentItem() == 1))
  {
  	transferAccount =  m_category->currentText();
		transferAccount = transferAccount.remove(0,1);
		transferAccount = transferAccount.remove(transferAccount.length() - 1,1);
    MyMoneyAccount *currentAccount;
    for(currentAccount = m_bankIndex.accountFirst(); currentAccount != 0; currentAccount = m_bankIndex.accountNext())
    {
			if(currentAccount->accountName() == transferAccount)
      {
				MyMoneyTransaction::transactionMethod transfermethod = MyMoneyTransaction::Transfer;
				QString theText;
	     	theText.sprintf("<%s>",account->accountName().latin1());
				newstate = MyMoneyTransaction::Unreconciled;
				currentAccount->addTransaction(transfermethod, m_number->text(), m_memo->text(),
																			 newamount, newdate, theText, "", "",
																			 m_payee->currentText(),"" ,"",
																			 newstate);
			}
		}		
	}

	
	useall = false;
  usedate = true;
  userow = false;
  m_filePointer->setDirty(true);
  updateTransactionList(-1, -1);
	emit transactionListChanged();

}

void KTransactionView::clearInputData()
{
	m_date->setDate(QDate::currentDate());
  m_method->setCurrentItem(0);
	m_number->setText(QString(""));
	m_payee->setEditText("");
  m_payment->setText(QString(""));
  m_withdrawal->setText(QString(""));
  m_category->setCurrentItem(0);
	m_memo->setText(QString(""));
	m_delete->hide();
}


void KTransactionView::setInputData(const MyMoneyTransaction transaction)
{
	m_date->setDate(transaction.date());
	m_payee->setEditText(transaction.payee());
	m_memo->setText(transaction.memo());
	m_number->setText(transaction.number());
  m_payment->setText(((transaction.type()==MyMoneyTransaction::Debit) ? KGlobal::locale()->formatNumber(transaction.amount().amount()) : QString("")));
  m_withdrawal->setText(((transaction.type()==MyMoneyTransaction::Credit) ? KGlobal::locale()->formatNumber(transaction.amount().amount()) : QString("")));
  bool categorySet = false;
  for(int i = 0; i < m_category->count(); i++)
  {
		QString theText;
		if(transaction.categoryMinor() == "")
		{
			theText.sprintf("%s", transaction.categoryMajor().latin1());
		}
		else
		{
			theText.sprintf("%s:%s",transaction.categoryMajor().latin1(),transaction.categoryMinor().latin1());
		}
   	if(m_category->text(i) == theText)
		{
			m_category->setCurrentItem(i);
     	categorySet = true;
		}
	}
  if(!categorySet)
	{
   	m_category->setCurrentItem(0);
	}
}

void KTransactionView::updateInputLists(void)
{
  QStringList categoryList;
  QString theText;
  if (m_filePointer) {
    QListIterator<MyMoneyCategory> categoryIterator = m_filePointer->categoryIterator();
    for ( ; categoryIterator.current(); ++categoryIterator) {
      MyMoneyCategory *category = categoryIterator.current();
      theText = category->name().latin1();
      categoryList.append(theText);
      for ( QStringList::Iterator it = category->minorCategories().begin(); it != category->minorCategories().end(); ++it ) {
        theText = category->name().latin1();
				theText += ":";
				theText += (*it).latin1();
        categoryList.append(theText);
      }
    }
    MyMoneyAccount *currentAccount;
    for(currentAccount = m_bankIndex.accountFirst(); currentAccount != 0; currentAccount = m_bankIndex.accountNext())
    {
     	theText = "<";
        theText = theText + currentAccount->accountName().latin1();
        theText = theText + ">";
			categoryList.append(theText);
		}
  }
	m_category->clear();
  m_category->insertStringList(categoryList);
  loadPayees();
}

void KTransactionView::updateTransactionList(int row, int col)
{
  if (!m_filePointer)
    return;

  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  transactionsTable->horizontalHeader()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));
  const int NO_ROWS = (config->readEntry("RowCount", "2").toInt());

  MyMoneyAccount *account;

	account = getAccount();
	if(account == 0)
		return;
  MyMoneyMoney balance;
  MyMoneyTransaction *transaction;
  unsigned long rowCount=0;
  QString currentBalance;

  if (row==-1) { // We are going to refresh the whole list
    m_index=-1;

  int i = 0;
    transactionsTable->setNumRows((m_transactions->count() * NO_ROWS) + 3);
    for (transaction=m_transactions->first(); transaction; transaction=m_transactions->next(), i++) {
//      transactionsTable->setEnabled(false);
      QString colText;
      if (transaction->type()==MyMoneyTransaction::Credit)
        balance += transaction->amount();
      else
        balance -= transaction->amount();

      if((useall == true) ||
         (usedate == true && transaction->date() >= m_date->getQDate()) ||
         (userow == true && m_index >= i))
      {
//        qDebug("useall=%d, usedate=%d, userow=%d, m_date=%s, m_index=%d, i=%d",
//          useall, usedate, userow, m_date->getQDate().toString().latin1(), m_index, i);


      switch (transaction->method()) {
        case MyMoneyTransaction::Cheque:
          colText = "Cheque";
          break;
        case MyMoneyTransaction::Deposit:
          colText = "Deposit";
          break;
        case MyMoneyTransaction::Transfer:
          colText = "Transfer";
          break;
        case MyMoneyTransaction::Withdrawal:
          colText = "Withdrawal";
          break;
        case MyMoneyTransaction::ATM:
          colText = "ATM";
          break;
      }

      transactionsTable->setText(rowCount, 0, KGlobal::locale()->formatDate(transaction->date(), true));

      if (NO_ROWS==2)
        transactionsTable->setText(rowCount + 1, 1, transaction->number());

      transactionsTable->setText(rowCount, 1, colText);

      transactionsTable->setText(rowCount, 2, transaction->payee());


			if(transaction->categoryMinor() == "")
			{
      	colText.sprintf("%s", transaction->categoryMajor().latin1());
			}
			else
			{
				colText.sprintf("%s:%s", transaction->categoryMajor().latin1(),transaction->categoryMinor().latin1());
			}

// Doesn't work
//      if (transaction->memo().length()>1)
//        transactionsTable->setItem2(transaction->memo());
      if (NO_ROWS==2)
        transactionsTable->setText(rowCount + 1, 2, colText);

      if (NO_ROWS==3)
        transactionsTable->setText(rowCount + 2, 2, transaction->memo());

      QString cLet;
      switch (transaction->state()) {
        case MyMoneyTransaction::Cleared:
          colText = "C";
          break;
        case MyMoneyTransaction::Reconciled:
          colText = "R";
          break;
        default:
          colText = " ";
          break;
      }
      transactionsTable->setText(rowCount, 3, colText);

      transactionsTable->setText(rowCount, 5,
        ((transaction->type()==MyMoneyTransaction::Credit) ? KGlobal::locale()->formatNumber(transaction->amount().amount()) : QString("")));

      transactionsTable->setText(rowCount, 4,
        ((transaction->type()==MyMoneyTransaction::Debit) ? KGlobal::locale()->formatNumber(transaction->amount().amount()) : QString("")));

      transactionsTable->setText(rowCount, 6, KGlobal::locale()->formatNumber(balance.amount()));

    }  // useall etc check
      rowCount += NO_ROWS;
			currentBalance = KGlobal::locale()->formatMoney(balance.amount());
    }

/*    // Add the last empty row
    if (NO_ROWS==1) {
      transactionsTable->setText(rowCount+2, 0, "hi");
    }
    else if (NO_ROWS==2) {
      transactionsTable->setText(rowCount+1, 0, "");
    }
*/
    transactionsTable->ensureCellVisible(rowCount+2, 0);

/*
    transactionsTable->setText(rowCount, 1, "");
    transactionsTable->setText(rowCount + 1, 1, "");
    transactionsTable->setText(rowCount, 0, "");
    transactionsTable->setText(rowCount + 1, 0, "");
    transactionsTable->setText(rowCount, 2, "");
    transactionsTable->setText(rowCount + 1, 2, "");
    transactionsTable->setText(rowCount, 3, "");
    transactionsTable->setText(rowCount + 1, 3, "");
    transactionsTable->setText(rowCount, 4, "");
    transactionsTable->setText(rowCount + 1, 4, "");
    transactionsTable->setText(rowCount, 5, "");
    transactionsTable->setText(rowCount + 1, 5, "");
    transactionsTable->setText(rowCount, 6, "");
    transactionsTable->setText(rowCount + 1, 6, "");
*/
		lblBalanceAmt->setText(currentBalance);
    lblBalanceAmt->setFont(config->readFontEntry("listCellFont", &defaultFont));
//    transactionsTable->setEnabled(true);

  } else { // We are just updating a section of it
    QString txt;
    if (row<0 || row>transactionsTable->numRows()-1)
      return;
    if (col<0 || col>transactionsTable->numCols()-1)
      return;
		int transrow = row / NO_ROWS;
		
    switch (col) {
      case 0:
        transactionsTable->setText(row, col, KGlobal::locale()->formatDate(m_transactions->at(transrow)->date()));
        break;

      case 1:
        switch (m_transactions->at(transrow)->method()) {
          case MyMoneyTransaction::Cheque:
            transactionsTable->setText(row, col, i18n("Cheque"));
            break;
          case MyMoneyTransaction::Deposit:
            transactionsTable->setText(row, col, i18n("Deposit"));
            break;
          case MyMoneyTransaction::Transfer:
            transactionsTable->setText(row, col, i18n("Transfer"));
            break;
          case MyMoneyTransaction::Withdrawal:
            transactionsTable->setText(row, col, i18n("Withdrawal"));
            break;
          case MyMoneyTransaction::ATM:
            transactionsTable->setText(row, col, i18n("ATM"));
            break;
        }
        break;

/*      case 2:
        transactionsTable->setText(row, col, m_transactions.at(row)->number());
        break;
*/
      case 2:
/*
  	    if((row % 2) == 0)
        {
          transactionsTable->setText(realrow, col, m_transactions->at(row)->payee());
				}
        else
				{
        	txt.sprintf("%s:%s", m_transactions->at(row)->categoryMajor().latin1(), m_transactions->at(transrow)->categoryMinor().latin1());
        	transactionsTable->setText(realrow + 1, col, txt);
				}
*/
				break;
      case 3:
        switch (m_transactions->at(transrow)->state()) {
          case MyMoneyTransaction::Unreconciled:
            transactionsTable->setText(row, col, "");
            break;
          case MyMoneyTransaction::Cleared:
            transactionsTable->setText(row, col, "C");
            break;
          case MyMoneyTransaction::Reconciled:
            transactionsTable->setText(row, col, "R");
            break;
        }
        break;
      case 4:
        transactionsTable->setText(row, col, KGlobal::locale()->formatNumber(m_transactions->at(transrow)->amount().amount()));
        break;
      case 5:
        transactionsTable->setText(row, col, KGlobal::locale()->formatNumber(m_transactions->at(transrow)->amount().amount()));
        break;
    }
  }

  //transactionsTable->ensureCellVisible(rowCount, 0);
}

void KTransactionView::cancelClicked()
{

	hideWidgets();
	
}

void KTransactionView::deleteClicked()
{
	hideWidgets();	
	slotTransactionDelete();

/*
  if (!m_filePointer)
    return;

  MyMoneyBank *bank;
  MyMoneyAccount *account;

  bank = m_filePointer->bank(m_bankIndex);
  if (!bank) {
    qDebug("unable to find bank in updateData");
    return;
  }

  account = bank->account(m_accountIndex);
  if (!account) {
    qDebug("Unable to find account in updateData");
    return;
  }

  QString prompt;

  prompt.sprintf(i18n("Delete this transaction ? :-\n%s"),m_category->currentText().latin1() );
  if ((KMessageBox::questionYesNo(this, prompt))==KMessageBox::No)
    return;


	if(m_index < m_transactions->count())
	{
   		account->removeTransaction(*m_transactions->at(m_index));
	}
	

	qDebug("enterClicked Before update Transaction List");
  updateTransactionList(-1, -1);
 */

}

void KTransactionView::refresh(void)
{
  useall = true;
  usedate = false;
  userow = false;
  clear();
  updateTransactionList(-1,-1);
}

/** gets a pointer to the current Account */
MyMoneyAccount* KTransactionView::getAccount(){

  MyMoneyBank *bank;
  MyMoneyAccount *account;

  bank = m_filePointer->bank(m_bankIndex);
  if (!bank) {
    qDebug("unable to find bank in updateData");
    return 0;
  }

  account = bank->account(m_accountIndex);
  if (!account) {
    qDebug("Unable to find account in updateData");
    return 0;
  }

	return account;

}
/** No descriptions */
void KTransactionView::slotNextTransaction(){

  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  const int NO_ROWS = (config->readEntry("RowCount", "2").toInt());
	slotFocusChange(m_currentrow + NO_ROWS,m_currentcol,m_currentbutton,m_currentpos);

}

void KTransactionView::viewTypeActivated(int num)
{
  if (num == 1 && m_viewType != SUBSET)
    emit viewTypeSearchActivated();
  else if (num == 0 && m_viewType != NORMAL)
    emit viewTypeNormalActivated();
  switch (num) {
    case 0:
      m_viewType = NORMAL;
      break;
    case 1:
      m_viewType = SUBSET;
      break;
  }
}

void KTransactionView::resizeEvent(QResizeEvent*)
{
	hideWidgets();
//  clear();
  int w=transactionsTable->width();
  transactionsTable->setColumnWidth(0, 100);
  transactionsTable->setColumnWidth(1, 100);
  transactionsTable->setColumnWidth(2, w-530-20);
  transactionsTable->setColumnWidth(3, 30);
  transactionsTable->setColumnWidth(4, 100);
  transactionsTable->setColumnWidth(5, 100);
  transactionsTable->setColumnWidth(6, 100);
}

/** No descriptions */
void KTransactionView::hideWidgets(){

  m_date->hide();
  m_method->hide();
	m_number->hide();
  m_payee->hide();
  m_payment->hide();
  m_withdrawal->hide();
//	m_hlayout->hide();
  m_category->hide();
  m_memo->hide();
	m_enter->hide();
	m_cancel->hide();
	m_delete->hide();


}
