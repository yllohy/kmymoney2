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
#include <kiconloader.h>

#include <qpushbutton.h>
#include <kcombobox.h>
#include <qtabwidget.h>
#include <qtable.h>
#include <qinputdialog.h>

#include "ktransactionview.h"
#include "../dialogs/knewcategorydlg.h"
#include "../dialogs/ksplittransactiondlg.h"
#include <kmessagebox.h>

#if QT_VERSION > 300
#include <qcursor.h>
#endif

KTransactionView::KTransactionView(QWidget *parent, const char *name)
 : KTransactionViewDecl(parent,name)
{
  transactionsTable->setNumCols(7);
  transactionsTable->setCurrentCell(0, 1);
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

  initAmountWidth();

/*
  int w=transactionsTable->width();
  transactionsTable->setColumnWidth(0, 100);
  transactionsTable->setColumnWidth(1, 100);
  transactionsTable->setColumnWidth(2, w-530-25);
  transactionsTable->setColumnWidth(3, 30);
  transactionsTable->setColumnWidth(4, 100);
  transactionsTable->setColumnWidth(5, 100);
  transactionsTable->setColumnWidth(6, 100);
*/
	KConfig *config = KGlobal::config();
  QFont defaultFont = QFont("helvetica", 12);
  transactionsTable->horizontalHeader()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));
	
  m_filePointer=0;

  connect(transactionsTable, SIGNAL(clicked(int, int, int, const QPoint&)),
    this, SLOT(slotFocusChange(int, int, int, const QPoint&)));

  connect(transactionsTable, SIGNAL(pressed(int, int, int ,const QPoint&)),
    this, SLOT(slotContextMenu(int, int, int, const QPoint&)));

  connect(viewTypeCombo, SIGNAL(activated(int)), this, SLOT(viewTypeActivated(int)));

  m_index = -1;

  m_bEditingTransaction=false;

  // create context menu
  KIconLoader *kiconloader = KGlobal::iconLoader();

  KPopupMenu* setAsMenu = new KPopupMenu(this);
  setAsMenu->insertTitle(kiconloader->loadIcon("set_as", KIcon::MainToolbar), i18n("Set As"));
  setAsMenu->insertItem(kiconloader->loadIcon("unreconciled", KIcon::Small), i18n("Unreconciled (default)"), this, SLOT(slotTransactionUnReconciled()));
  setAsMenu->insertItem(kiconloader->loadIcon("cleared", KIcon::Small), i18n("Cleared"), this, SLOT(slotTransactionCleared()));

  m_contextMenu = new KPopupMenu(this);
  m_contextMenu->insertTitle(kiconloader->loadIcon("transaction", KIcon::MainToolbar), i18n("Transaction Options"));
  m_contextMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small), i18n("Delete..."), this, SLOT(slotTransactionDelete()));
  m_contextMenu->insertSeparator();
  m_contextMenu->insertItem(kiconloader->loadIcon("set_as", KIcon::Small), i18n("Set as"), setAsMenu);

  // m_contextMenu->insertItem(kiconloader->loadIcon("split", KIcon::Small), i18n("Split"), this, SLOT(slotEditSplit()));
  createInputWidgets();

  m_bSignals=true;
}

KTransactionView::~KTransactionView()
{
/*
	delete m_date;
	delete m_method;
    delete m_payee;
    delete m_payment;
    delete m_withdrawal;
	delete m_number;
	delete m_category;
	delete m_memo;
    delete m_enter;
    delete m_cancel;
    delete m_delete;
 */
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
  		m_payment->setText(((lasttransaction->type()==MyMoneyTransaction::Debit) ? KGlobal::locale()->formatMoney(lasttransaction->amount().amount(),"") : QString("")));
  		m_withdrawal->setText(((lasttransaction->type()==MyMoneyTransaction::Credit) ? KGlobal::locale()->formatMoney(lasttransaction->amount().amount(),"") : QString("")));
  		bool categorySet = false;
  		for(int i = 0; i < m_category->count(); i++)
  		{
				QString theText;
				if(lasttransaction->categoryMinor() == "")
				{
					theText = lasttransaction->categoryMajor();
				}
				else
				{
				  theText = lasttransaction->categoryMajor();
				  theText += ":";
				  theText += lasttransaction->categoryMinor();
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
	if(m_method->currentText() == i18n("Cheque"))
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
  m_date = new kMyMoneyDateInput(0,QDate::currentDate(), Qt::AlignRight);
  m_method = new kMyMoneyCombo(0);
  m_payee = new kMyMoneyCombo(true,0);
  m_payment = new kMyMoneyEdit(0);
  m_withdrawal = new kMyMoneyEdit(0);
  m_number = new kMyMoneyLineEdit(0);
  m_hlayout = new kMyMoneyHLayout(0);
  m_category = new kMyMoneyCombo(false, 0);
  m_memo = new kMyMoneyLineEdit(0);
  m_hlayout->addWidget(m_category);
  m_hlayout->addWidget(m_memo);
  m_enter = new KPushButton(i18n("Enter"),0);
  m_cancel = new KPushButton(i18n("Cancel"),0);
  m_delete = new KPushButton(i18n("Delete"),0);
  m_split = new KPushButton(i18n("Split"),0);

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
  //  m_method->insertItem("");
  // We don't need a blank item.  These will be dynamic in the future
  KCompletion *methodcomp = m_method->completionObject();
  connect(m_method,SIGNAL(returnPressed(const QString&)),methodcomp,SLOT(addItem(const QString&)));
  m_method->setEditable(true);
  m_method->setAutoCompletion(true);
  m_method->insertItem(i18n("Cheque"));
  m_method->insertItem(i18n("Deposit"));
  m_method->insertItem(i18n("Transfer"));
  m_method->insertItem(i18n("Withdrawal"));
  m_method->insertItem(i18n("ATM"));

  // as the code has hard-coded references to the above list,
  // we do not allow editing of these values here. This somehow
  // makes autocompletion senseless, but I leave it in for later use.
  m_method->setEditable(false);
  m_method->setFocusPolicy(QWidget::StrongFocus);

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
  connect(m_split, SIGNAL(clicked()),this,SLOT(slotEditSplit()));
	
	connect(m_category, SIGNAL(activated(int)), this, SLOT(slotCategoryActivated(int)));
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
			if((*it) == payee)
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
			if((*it) == payee)
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
  KCompletion *payeecomp = m_payee->completionObject();
  payeecomp->setItems(payeelist);
}

void KTransactionView::slotFocusChange(int row, int col, int button, const QPoint&  point)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  const int NO_ROWS = (config->readEntry("RowCount", "2").toInt());
	unsigned transrow = row / NO_ROWS;
  int realrow = transrow * NO_ROWS;

  // Can't add transactions in search mode
  if (m_date->isVisible() || (m_viewType!=NORMAL && (transrow >= m_transactions->count())))
    return;

  // make sure, realrow points to the first line of the transaction
  // it might be off by 1, if there's a single line per transaction
  // and the click went into the bottom line of a new transaction
  if(realrow == transactionsTable->numRows()-1)
    --realrow;

  // make sure the input widgets will be on the screen
  transactionsTable->ensureCellVisible(realrow, col);
  transactionsTable->ensureCellVisible(realrow+1, col);
  transactionsTable->setRowOffset(realrow);
	
	m_currentrow = realrow;
	m_currentcol = col;
	m_currentbutton = button;
	m_currentpos = point;

//  if ((transrow != static_cast<unsigned> (transactionsTable->numRows())-1)

  // figure out, if the click was inside a transaction or not
  int lastY = transactionsTable->cellGeometry(transactionsTable->numRows()-1, 0).bottom();

  // Make sure to use the right coordinate system.
  // Check the online help on QScrollView for details.
  if(lastY >= (transactionsTable->viewportToContents(point).y())
  && (transactionsTable->numRows()>=1)) {
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
      m_date->show();
      transactionsTable->setCellWidget(realrow ,1,m_method);
      m_method->show();
      transactionsTable->setCellWidget(realrow ,2,m_payee);
      m_payee->show();
      transactionsTable->setCellWidget(realrow ,4,m_payment);
      m_payment->show();
      transactionsTable->setCellWidget(realrow ,5,m_withdrawal);
      m_withdrawal->show();
      transactionsTable->setCellWidget(realrow + 1 ,1,m_number);
      transactionsTable->setCellWidget(realrow + 1 ,2,m_hlayout);
	
      m_hlayout->setGeometry(transactionsTable->cellGeometry(realrow + 1,2));
      m_number->show();
      m_hlayout->show();
      transactionsTable->setCellWidget(realrow + 1 ,4,m_enter);
      m_enter->show();
      transactionsTable->setCellWidget(realrow + 1 ,5,m_cancel);
      m_cancel->show();
      transactionsTable->setCellWidget(realrow + 1 ,6,m_delete);
      m_delete->show();
      transactionsTable->setCellWidget(realrow + 1, 3, m_split);
      m_split->show();

      updateInputLists();
      if(m_transactions->count() > transrow)
      {
        m_bEditingTransaction=true;
	      setInputData(*m_transactions->at(transrow));
			}
			else
			{
			  m_bEditingTransaction=false;
       	clearInputData();
			}
    }
    m_index = transrow;

  } else
    m_index=-1;
}

void KTransactionView::slotContextMenu(int row, int col, int button, const QPoint&  point)
{
  if(button == Qt::RightButton) {
    KConfig *config = KGlobal::config();
    config->setGroup("List Options");
    const int NO_ROWS = (config->readEntry("RowCount", "2").toInt());
    unsigned transrow = row / NO_ROWS;

    int realrow = transrow * NO_ROWS;

    // Can't add transactions in search mode
    if (m_date->isVisible() || (m_viewType!=NORMAL && (transrow >= m_transactions->count())))
      return;

    // make sure, realrow points to the first line of the transaction
    // it might be off by 1, if there's a single line per transaction
    // and the click went into the bottom line of a new transaction
    if(realrow == transactionsTable->numRows()-1)
      --realrow;

    // make sure the input widgets will be on the screen
    transactionsTable->ensureCellVisible(realrow, col);
    transactionsTable->ensureCellVisible(realrow+1, col);
    transactionsTable->setRowOffset(realrow);
  	
  	m_currentrow = realrow;
  	m_currentcol = col;
  	m_currentbutton = button;
  	m_currentpos = point;

    // figure out, if the click was inside a transaction or not
    int lastY = transactionsTable->cellGeometry(transactionsTable->numRows()-3, 0).bottom();

    // Make sure to use the right coordinate system.
    // Check the online help on QScrollView for details.
    if(lastY >= point.y()
    && (transactionsTable->numRows()>=1)) {
      m_index = transrow;
      m_contextMenu->exec(QCursor::pos());
    }
  }
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

  prompt = i18n("Delete this transaction ? :- ");
  prompt += transaction->payee();

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
			if(currentAccount->name() == transferAccount)
      {
				MyMoneyTransaction *currentTransaction;
				for(currentTransaction = currentAccount->transactionFirst(); currentTransaction != 0; currentTransaction = currentAccount->transactionNext())
				{
					QString matchCategory = "<";
					matchCategory += pAccount->name();
					matchCategory += ">";
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

  useall = true;
  usedate = false;
  userow = false;
  updateTransactionList(-1, -1);
  emit transactionListChanged();
}

void KTransactionView::slotTransactionUnReconciled()
{
  MyMoneyAccount *pAccount;

  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
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

void KTransactionView::slotEditSplit()
{
  MyMoneyAccount *pAccount;
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  // const int NO_ROWS = (config->readEntry("RowCount", "2").toInt());
	
	pAccount = getAccount();
	if(pAccount == 0)
		return;

  MyMoneyTransaction *transaction = m_transactions->at(m_index);
  if (!transaction)
    return;

  // Get amount from payment or withdrawal Line Edit
  MyMoneyMoney amount;
  bool amountSet = true;
	if(m_payment->text() == "")	{
    if(m_withdrawal->text() == "")	{
     	amount = 0;
      amountSet = false;
    } else {
  	  amount = m_withdrawal->text();
    }
	} else {
  	amount = m_payment->text();
  }

  KSplitTransactionDlg* dlg = new KSplitTransactionDlg(0, 0,
    m_filePointer, getBank(), pAccount,
    &amount, amountSet);

  MyMoneySplitTransaction* split;
  MyMoneySplitTransaction* tmp;

  // copy the split list
  split = transaction->firstSplit();
  while(split) {
    tmp = new MyMoneySplitTransaction(*split);
    dlg->addTransaction(tmp);
    split = transaction->nextSplit();
  }

  if(dlg->exec() == QDialog::Accepted) {
    transaction->clearSplitList();
    // copy the split list
    split = dlg->firstTransaction();
    while(split != NULL) {
      tmp = new MyMoneySplitTransaction(*split);
      tmp->setParent(transaction);
      transaction->addSplit(tmp);
      split = dlg->nextTransaction();
    }
    transaction->setAmount(amount);
    m_payment->setText(((transaction->type()==MyMoneyTransaction::Debit) ? KGlobal::locale()->formatMoney(amount.amount(),"") : QString("")));
    m_withdrawal->setText(((transaction->type()==MyMoneyTransaction::Credit) ? KGlobal::locale()->formatMoney(amount.amount(),"") : QString("")));
    m_category->setCurrentItem("Split");
  }
  delete dlg;
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

  // Set the transaction list to have focus to grab it from the bank view
  // BUG: 490015
  transactionsTable->setFocus();

  updateTransactionList(-1);
}

void KTransactionView::clear(void)
{
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
	MyMoneyTransaction::transactionMethod newmethod;
 	MyMoneyMoney newamount;

  QDate newdate = m_date->getQDate();
	QString newcategory = m_category->currentText();

  if ( newcategory == i18n("--- Income ---") ||
      newcategory == i18n("--- Expense ---") ||
      newcategory == i18n("--- Special ---") ) {
    KMessageBox::error(this, i18n("Please do not choose the type options (--- ??? ---)\nCancelling "
      "transaction update."));
    m_category->setFocus(); // Don't think this will work anyway
    return;
  }

  // Get amount from payment or withdrawal Line Edit
	if(m_payment->text() == "")	{
  	newamount = m_withdrawal->text();

	} else if(m_withdrawal->text() == "")	{
  	newamount = m_payment->text();

  } else {
   	newamount = 0;
	}

	MyMoneyTransaction::stateE newstate;
	
	// Set the transaction type
  switch(m_method->currentItem()) {
    case 0:     // Cheque
      newmethod = MyMoneyTransaction::Cheque;
      break;
    case 1:
      newmethod = MyMoneyTransaction::Deposit;
      break;
    case 2:
      newmethod = MyMoneyTransaction::Transfer;
      break;
    case 3:
      newmethod = MyMoneyTransaction::Withdrawal;
      break;
    case 4:
      newmethod = MyMoneyTransaction::ATM;
      break;
    default:
      KMessageBox::error(this, i18n("Invalid method selected\nCancelling "
                                    "transaction update."));
      m_method->setFocus(); // Don't think this will work anyway
      return;
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

	if(m_index < static_cast<long> (m_transactions->count()))
	{
    MyMoneyTransaction *transaction = m_transactions->at(m_index);
		newstate = transaction->state();

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
			  if(currentAccount->name() == transferAccount)
        {
				  MyMoneyTransaction *currentTransaction;
				  for(currentTransaction = currentAccount->transactionFirst(); currentTransaction != 0; currentTransaction = currentAccount->transactionNext())
				  {
					  QString matchCategory = "<";
					  matchCategory += account->name();
					  matchCategory += ">";
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
//   	  account->removeTransaction(*m_transactions->at(m_index));
//    	account->addTransaction(newmethod, m_number->text(), m_memo->text(),
//            newamount, newdate, catmajor, catminor, "",
//            m_payee->currentText(), "", "", newstate);
	
    // Look for it, in case we are searching, use the new 'parent' methods
    MyMoneyAccount *mymoneyaccount = m_transactions->at(m_index)->account();
    if (!mymoneyaccount) {
      qDebug("Aaaaaaaaaaaaaarrrrrrrrrrrrrggggggggggggghhhhhhhhhhhhhhh!");
      return;
    }
    transaction = mymoneyaccount->transaction(*transaction);
    if (!transaction) {
      qDebug("could not locate transaction");
      return;
    }
	
	  transaction->setDate(newdate);
	  transaction->setMethod(newmethod);
	  transaction->setPayee(m_payee->currentText());
	  transaction->setState(newstate);
	  transaction->setAmount(newamount);
	  transaction->setNumber(m_number->text());
	  transaction->setMemo(m_memo->text());
	  transaction->setCategoryMajor(catmajor);
	  transaction->setCategoryMinor(catminor);
	
	  // update the m_transactions one
	  transaction = m_transactions->at(m_index);
	  transaction->setDate(newdate);
	  transaction->setMethod(newmethod);
	  transaction->setPayee(m_payee->currentText());
	  transaction->setState(newstate);
	  transaction->setAmount(newamount);
	  transaction->setNumber(m_number->text());
	  transaction->setMemo(m_memo->text());
	  transaction->setCategoryMajor(catmajor);
	  transaction->setCategoryMinor(catminor);
	
	}
	else
  {
		newstate = MyMoneyTransaction::Unreconciled;
  	account->addTransaction(newmethod, m_number->text(), m_memo->text(),
                            newamount, newdate, catmajor, catminor, "",
  													m_payee->currentText(), "", "", newstate);
	}

  if((lessindex != -1) && (greatindex != -1) && (m_method->currentItem() == 2))
  {
  	transferAccount =  m_category->currentText();
		transferAccount = transferAccount.remove(0,1);
		transferAccount = transferAccount.remove(transferAccount.length() - 1,1);
    MyMoneyAccount *currentAccount;
    for(currentAccount = m_bankIndex.accountFirst(); currentAccount != 0; currentAccount = m_bankIndex.accountNext())
    {
			if(currentAccount->name() == transferAccount)
      {
				MyMoneyTransaction::transactionMethod transfermethod = MyMoneyTransaction::Deposit;
			  QString theText = "<";
			  theText += account->name();
			  theText += ">";
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
			if(currentAccount->name() == transferAccount)
      {
				MyMoneyTransaction::transactionMethod transfermethod = MyMoneyTransaction::Transfer;
			  QString theText = "<";
			  theText += account->name();
			  theText += ">";
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
  // keep a copy of the transaction for the case the user cancels
  m_originalTransaction = transaction;

	m_date->setDate(transaction.date());
	m_payee->setEditText(transaction.payee());
	m_memo->setText(transaction.memo());
	m_number->setText(transaction.number());
  m_payment->setText(((transaction.type()==MyMoneyTransaction::Debit) ? KGlobal::locale()->formatMoney(transaction.amount().amount(),"") : QString("")));
  m_withdrawal->setText(((transaction.type()==MyMoneyTransaction::Credit) ? KGlobal::locale()->formatMoney(transaction.amount().amount(),"") : QString("")));

  QString theText;
  if(transaction.categoryMinor() == "") {
    theText = transaction.categoryMajor();
  } else {
    theText = transaction.categoryMajor();
    theText += ":";
    theText += transaction.categoryMinor();
  }
  m_category->setCurrentItem(theText);
}

void KTransactionView::updateInputLists(void)
{
  QStringList categoryList;
  QStringList qstringlistIncome;
  QStringList qstringlistExpense;
  QStringList qstringlistAccount;

  QString theText;
  if (m_filePointer) {
    QListIterator<MyMoneyCategory> categoryIterator = m_filePointer->categoryIterator();
    for ( ; categoryIterator.current(); ++categoryIterator) {
      bool bDoneInsert = false;
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

    MyMoneyBank *mymoneybank = getBank();

    if (mymoneybank) {
      MyMoneyAccount *currentAccount;

      for(currentAccount = mymoneybank->accountFirst(); currentAccount != 0; currentAccount = mymoneybank->accountNext())
      {
       	theText = "<";
        theText = theText + currentAccount->name();
        theText = theText + ">";
			  qstringlistAccount.append(theText);
  		}
    }
  }

  qstringlistAccount.append("Split");

	m_category->clear();
	
	qstringlistIncome.prepend(i18n("--- Income ---"));
  qstringlistIncome.prepend("");
	categoryList = qstringlistIncome;
	
	qstringlistExpense.prepend(i18n("--- Expense ---"));
	categoryList += qstringlistExpense;
	
	qstringlistAccount.prepend(i18n("--- Special ---"));
	categoryList += qstringlistAccount;

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
    bool haveCurrentDate = false;
    transactionsTable->setNumRows((m_transactions->count() * NO_ROWS) + 2);

    initAmountWidth();

    for (transaction=m_transactions->first(); transaction; transaction=m_transactions->next(), i++) {
      QString colText;
      if (transaction->type()==MyMoneyTransaction::Credit)
        balance += transaction->amount();
      else
        balance -= transaction->amount();

      if((useall == true) ||
         (usedate == true && transaction->date() >= m_date->getQDate()) ||
         (userow == true && m_index >= i))
      {

      if(!haveCurrentDate && transaction->date() > QDate::currentDate()) {
        haveCurrentDate = true;
        transactionsTable->setCurrentDateRow(rowCount);
      }
      if(!haveCurrentDate && transaction->date() < QDate::currentDate()) {
        transactionsTable->setCurrentDateRow(rowCount);
      }

      switch (transaction->method()) {
        case MyMoneyTransaction::Cheque:
          colText = i18n("Cheque");
          break;
        case MyMoneyTransaction::Deposit:
          colText = i18n("Deposit");
          break;
        case MyMoneyTransaction::Transfer:
          colText = i18n("Transfer");
          break;
        case MyMoneyTransaction::Withdrawal:
          colText = i18n("Withdrawal");
          break;
        case MyMoneyTransaction::ATM:
          colText = i18n("ATM");
          break;
      }

      transactionsTable->setText(rowCount, 0, KGlobal::locale()->formatDate(transaction->date(), true));

      transactionsTable->setText(rowCount, 1, colText);

      transactionsTable->setText(rowCount, 2, transaction->payee());

      switch (transaction->state()) {
        case MyMoneyTransaction::Cleared:
          colText = i18n("C");
          break;
        case MyMoneyTransaction::Reconciled:
          colText = i18n("R");
          break;
        default:
          colText = " ";
          break;
      }
      transactionsTable->setText(rowCount, 3, colText);

      QString amountTxt = KGlobal::locale()->formatMoney(transaction->amount().amount(), "");
      unsigned width = transactionsTable->fontMetrics().width(amountTxt);
      if(transaction->type()==MyMoneyTransaction::Debit) {
        transactionsTable->setText(rowCount, 4, amountTxt);
        transactionsTable->setText(rowCount, 5, "");
        if(width > m_debitWidth)
          m_debitWidth = width;
      } else {
        transactionsTable->setText(rowCount, 4, "");
        transactionsTable->setText(rowCount, 5, amountTxt);
        if(width > m_creditWidth)
          m_creditWidth = width;
      }

      if (m_viewType==NORMAL) {
        amountTxt = KGlobal::locale()->formatMoney(balance.amount(), "");
        width = transactionsTable->fontMetrics().width(amountTxt);
        transactionsTable->setText(rowCount, 6, amountTxt);
        if(width > m_balanceWidth)
          m_balanceWidth = width;
      } else
        transactionsTable->setText(rowCount, 6, i18n("N/A"));

      // initializing the table like I've done below speeds up operations
      // when using 1700 transactions at 2 row display. At 1 row
      // display performance was OK, but with 2 row display it was
      // odd. Measurements on my system:
      //
      // 1758 transactions, 1 row display    ~2 sec
      // 1758 transactions, 2 row display   ~23 sec
      //
      // for the display to show up. Adding the code below helped
      // a lot!! I now get
      //
      // 1758 transactions, 1 row display    ~3 sec
      // 1758 transactions, 2 row display    ~3 sec
      //
      // Please don't ask me why!!

      if (NO_ROWS==2) {
        transactionsTable->setText(rowCount + 1, 0, "");
        transactionsTable->setText(rowCount + 1, 1, transaction->number());

  			if(transaction->categoryMinor() == "") {
          colText = transaction->categoryMajor();
        } else {
          colText = transaction->categoryMajor()
                    + ":"
                    + transaction->categoryMinor();
        }
        colText = colText + "|" + transaction->memo();

        transactionsTable->setText(rowCount + 1, 2, colText);
        transactionsTable->setText(rowCount + 1, 3, "");
        transactionsTable->setText(rowCount + 1, 4, "");
        transactionsTable->setText(rowCount + 1, 5, "");
        transactionsTable->setText(rowCount + 1, 6, "");
      } // NO_ROWS == 2

    }  // useall etc check
      rowCount += NO_ROWS;
			currentBalance = KGlobal::locale()->formatMoney(balance.amount(),"");
    }

    if(!haveCurrentDate && rowCount > 0) {
      transactionsTable->setCurrentDateRow(rowCount);
    } else if(!haveCurrentDate)
      transactionsTable->setCurrentDateRow(-1);

		if (m_viewType==NORMAL) {
		  if (!m_bEditingTransaction)
        transactionsTable->ensureCellVisible(rowCount+1, 0);
      else
        m_bEditingTransaction=false;

      if (config->readBoolEntry("TextPrompt", true)) {
        transactionsTable->setText(rowCount, 0, i18n("Date"));
        transactionsTable->setText(rowCount, 1, i18n("Method"));
        transactionsTable->setText(rowCount, 2, i18n("Click on a field to enter a transaction"));
        transactionsTable->setText(rowCount, 3, i18n("?"));
        transactionsTable->setText(rowCount, 4, i18n("Amount"));
        transactionsTable->setText(rowCount, 5, i18n("Amount"));
        transactionsTable->setText(rowCount, 6, "");
        transactionsTable->setText(rowCount+1, 0, "");
        transactionsTable->setText(rowCount+1, 1, i18n("Number"));
        transactionsTable->setText(rowCount+1, 2, i18n("Category|Description"));
        transactionsTable->setText(rowCount+1, 3, "");
        transactionsTable->setText(rowCount+1, 4, "");
        transactionsTable->setText(rowCount+1, 5, "");
      }

  		lblBalanceAmt->setText(currentBalance);
      lblBalanceAmt->setFont(config->readFontEntry("listCellFont", &defaultFont));
    } else {
      transactionsTable->ensureCellVisible(rowCount, 0);
      lblBalanceAmt->setText(i18n("N/A"));
    }

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

      case 2:
  	    if((row % 2) == 0)
        {
          transactionsTable->setText(row, col, m_transactions->at(row)->payee());
				}
        else
				{
        	txt = m_transactions->at(row)->categoryMajor();
        	txt += ":";
        	txt += m_transactions->at(transrow)->categoryMinor();
          txt = txt + "|" + m_transactions->at(m_index)->memo();
        	transactionsTable->setText(row + 1, col, txt);
				}
				break;
      case 3:
        switch (m_transactions->at(transrow)->state()) {
          case MyMoneyTransaction::Unreconciled:
            transactionsTable->setText(row, col, "");
            break;
          case MyMoneyTransaction::Cleared:
            transactionsTable->setText(row, col, i18n("C"));
            break;
          case MyMoneyTransaction::Reconciled:
            transactionsTable->setText(row, col, i18n("R"));
            break;
        }
        break;
      case 4:
        transactionsTable->setText(row, col, KGlobal::locale()->formatMoney(m_transactions->at(transrow)->amount().amount(),""));
        break;
      case 5:
        transactionsTable->setText(row, col, KGlobal::locale()->formatMoney(m_transactions->at(transrow)->amount().amount(),""));
        break;
    }
  }

  // setup new size values
  resizeEvent(NULL);
}

void KTransactionView::cancelClicked()
{
	hideWidgets();
	if(m_index < static_cast<long> (m_transactions->count())) {
    MyMoneyTransaction *transaction = m_transactions->at(m_index);
    *transaction = m_originalTransaction;
  }
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
  viewingType prevViewType = m_viewType;

  switch (num) {
    case 0:
      m_viewType = NORMAL;
      break;
    case 1:
      m_viewType = SUBSET;
      break;
  }

  if (num == 1 && prevViewType != SUBSET)
    emit viewTypeSearchActivated();
  else if (num == 0 && prevViewType != NORMAL)
    emit viewTypeNormalActivated();
}

void KTransactionView::resizeEvent(QResizeEvent*)
{
	// hideWidgets();

  int w=transactionsTable->visibleWidth() - 200 - 30 -
    m_debitWidth - m_creditWidth - m_balanceWidth;

  transactionsTable->setColumnWidth(0, 100);
  transactionsTable->setColumnWidth(1, 100);
  transactionsTable->setColumnWidth(2, w);
  transactionsTable->setColumnWidth(3, 30);
  transactionsTable->setColumnWidth(4, m_debitWidth);
  transactionsTable->setColumnWidth(5, m_creditWidth);
  transactionsTable->setColumnWidth(6, m_balanceWidth);
}

/** No descriptions */
void KTransactionView::hideWidgets()
{
  m_date->hide();
  m_method->hide();
  m_number->hide();
  m_payee->hide();
  m_payment->hide();
  m_withdrawal->hide();
  m_hlayout->hide();
  m_enter->hide();
  m_cancel->hide();
  m_delete->hide();
  m_split->hide();
}

/**
  * Damned ugly code and i hope to put this sort of validation somewhere
  * else, maybe in a specialised kmymoneycategorycombo type class.
*/
void KTransactionView::slotCategoryActivated(int pos)
{
/* FIXME: not required anymore, can be removed
  QString qstringText = m_category->text(pos);
  if ( qstringText == i18n("--- Income ---") ||
      qstringText == i18n("--- Expense ---") ||
      qstringText == i18n("--- Special ---") ) {
    KMessageBox::error(this, i18n("Please do not choose the type options (--- ??? ---)"));
    m_category->setFocus(); // Don't think this will work anyway
  }
*/
  MyMoneyTransaction* transaction = m_transactions->at(m_index);
  if(transaction == NULL) {
    qDebug("transaction == NULL in slotCategoryActivated");
    return;
  }

  bool haveSplits = transaction->firstSplit() != NULL;
  bool isSplit = m_category->text(pos) == "Split";

  // if we move the category away, we delete all splits
  if(haveSplits && !isSplit) {
    if(KMessageBox::warningContinueCancel(
            0, QString(i18n("Changing the category will erase all "
                            "information about the splits. Do you "
                            "want to continue?")),
            QString(i18n("Delete split information")),
            QString(i18n("Continue")), false  ) == KMessageBox::Continue) {
      transaction->clearSplitList();
      transaction->setDirty(true);
    } else {
      m_category->setCurrentItem("Split");
    }
  }
}

/** gets a pointer to the current Account */
MyMoneyBank* KTransactionView::getBank(void){

  MyMoneyBank *bank;

  bank = m_filePointer->bank(m_bankIndex);
  if (!bank)
    qDebug("unable to find bank in updateData");

  return bank;
}

/** Setup initial width for the amount fields */
void KTransactionView::initAmountWidth(void)
{
  m_debitWidth = m_creditWidth = m_balanceWidth = 80;
}

void KTransactionView::show()
{
  if (m_bSignals)
    emit signalViewActivated();
  QWidget::show();
}

void KTransactionView::setSignals(bool enable)
{
  m_bSignals=enable;
}
