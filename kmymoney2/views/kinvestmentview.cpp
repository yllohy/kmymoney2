/***************************************************************************
                          kinvestmentview.cpp  -  description
                             -------------------
    begin                : Tue Jan 29 2002
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

#include <qlayout.h>
#include <qtabwidget.h>
#include <qheader.h>
#include <qlistbox.h>

/*
#if QT_VERSION > 300
// #include <qcursor.h>
#endif

#include <qpushbutton.h>
#include <qtable.h>
#include <qinputdialog.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
*/

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klistview.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyutils.h"
#include "../mymoney/mymoneyequity.h"
#include "../mymoney/mymoneytransaction.h"
#include "../mymoney/mymoneyinvesttransaction.h"
#include "../mymoney/mymoneyaccount.h"
#include "kinvestmentlistitem.h"
#include "../dialogs/knewequityentrydlg.h"
#include "../dialogs/kupdatestockpricedlg.h"
#include "../dialogs/keditequityentrydlg.h"
#include "kinvestmentview.h"
#include "kledgerviewinvestments.h"

KInvestmentView::KInvestmentView(QWidget *parent, const char *name)
 :  kInvestmentViewDecl(parent,name)
{
  m_account = MyMoneyAccount();
  m_popMenu	= NULL;
  // FIXME: check if we really want to remove the margin
  // kInvestmentViewDeclLayout->setMargin(0);

  qDebug("KInvestmentView::KInvestmentView: Investment View starting up");

  initSummaryTab();
  initTransactionTab();

  // fill in some demo data
  KListViewItem* item1 = new KListViewItem(investmentTable, QString("Redhat"), QString("RHAT"), QString("24"), QString("$20.00"), QString("$13.43"), QString("$212"), QString("5.43%"), QString("9.43%"));
  investmentTable->insertItem(item1);

  KListViewItem* item2 = new KListViewItem(investmentTable, QString("Yahoo"), QString("YHOO"), QString("100"), QString("$14.21"), QString("$25.43"), QString("$900"), QString("10.43%"), QString("19.12%"));
  investmentTable->insertItem(item2);        

  // never show a horizontal scroll bar
 // investmentTable->setHScrollBarMode(QScrollView::AlwaysOff);

  //no sorting yet...
 // investmentTable->setSorting(-1);

//  connect(investmentTable, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
//    this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));

  //connects the signal when a radio button is checked.
 // connect(m_btnGroupView, SIGNAL(clicked(int)), this, SLOT(slotViewChanged(int)));

  //hide transaction view, since we show the summary view by default.
//  transactionTable->hide();

  //set the summary button to be true.
 // btnSummary->setChecked(TRUE);

  connect(investmentTable, SIGNAL(doubleClicked(QListViewItem*,const QPoint&, int)), this, SLOT(slotItemDoubleClicked(QListViewItem*,const QPoint&, int)));
}

KInvestmentView::~KInvestmentView()
{
}

void KInvestmentView::initSummaryTab(void)
{
  investmentTable->setRootIsDecorated(true);
  investmentTable->setColumnText(0, QString(i18n("Symbol")));
  investmentTable->addColumn(i18n("Name"));
  investmentTable->addColumn(i18n("Symbol"));
  investmentTable->addColumn(i18n("Quantity"));
  investmentTable->addColumn(i18n("Current Price"));
  investmentTable->addColumn(i18n("Original Price"));
  investmentTable->addColumn(i18n("$ Gain"));
  investmentTable->addColumn(i18n("1 week %"));
  investmentTable->addColumn(i18n("4 weeks %"));
  investmentTable->addColumn(i18n("3 Months %"));
  investmentTable->addColumn(i18n("YTD %"));

  investmentTable->setMultiSelection(false);
  investmentTable->setColumnWidthMode(0, QListView::Maximum);
  investmentTable->header()->setResizeEnabled(true);
  investmentTable->setAllColumnsShowFocus(true);
}

void KInvestmentView::initTransactionTab(void)
{
  QHBoxLayout* m_TransactionTabLayout = new QHBoxLayout( m_transactionTab, 11, 6, "m_summaryTabLayout");

  m_ledgerView = new KLedgerViewInvestments(m_transactionTab, "ledgerview");
  m_TransactionTabLayout->addWidget(m_ledgerView);
}

/** No descriptions */
bool KInvestmentView::init(const MyMoneyAccount& account)
{
  m_account = account;

//  KConfig *config = KGlobal::config();
  QDateTime defaultDate = QDate::currentDate();
  QDate qdateStart = QDate::currentDate();//config->readDateTimeEntry("StartDate", &defaultDate).date();

  if(qdateStart != defaultDate.date())
  {
  	MyMoneyInvestTransaction *pInvestTransaction = NULL;
    MyMoneyTransaction *transaction = NULL;
//    m_transactionList.clear();

//    for(transaction=pAccount->transactionFirst(); transaction; transaction=pAccount->transactionNext())
    {
 //     if(transaction->date() >= qdateStart)
      {
//      	if(transaction)
      	{
        	/*pInvestTransaction = static_cast<MyMoneyInvestTransaction*>(transaction);
        	m_transactionList.append(new MyMoneyInvestTransaction(
            pAccount,
            transaction->id(),
            transaction->method(),
            transaction->number(),
            transaction->memo(),
            transaction->amount(),
            transaction->date(),
            transaction->categoryMajor(),
            transaction->categoryMinor(),
            transaction->atmBankName(),
            transaction->payee(),
            transaction->accountFrom(),
            transaction->accountTo(),
            transaction->state()));  */
      	}
      }
    }
	}
	return true;
}
/** No descriptions */
void KInvestmentView::updateDisplay()
{
	//for(emp=list.first(); emp != 0; emp=list.next())
	//{
		//printf( "%s earns %d\n", emp->name().latin1(), emp->salary() );
	//}
}

void KInvestmentView::slotItemDoubleClicked(QListViewItem* pItem, const QPoint& pos, int c)
{
  if(COLUMN_NAME_INDEX == c || COLUMN_SYMBOL_INDEX == c)
  {
    MyMoneyFile* currentFile = MyMoneyFile::instance();
    //currentFile->
    //QString clickedEquity = pItem->text(COLUMN_SYMBOL_INDEX);
    MyMoneyEquity equity;
    KEditEquityEntryDlg *pDlg = new KEditEquityEntryDlg(&equity, this);
	  pDlg->exec();
  }
}



/*void KInvestmentView::slotNewInvestment()
{
	MyMoneyEquity equity;
	KNewEquityEntryDlg *pDlg = new KNewEquityEntryDlg(this);
	pDlg->exec();
	int nResult = pDlg->result();
	if(nResult)
	{
		//populate this equity entry with information from the dialog.
		QString strTemp;
		strTemp = pDlg->edtEquityName->text();
		kdDebug(1) << "Equity name is: " << strTemp << endl;
//		pEquity->setEquityName(strTemp);
			
		strTemp = pDlg->edtMarketSymbol->text();
		kdDebug(1) << "Equity Symbol is: " << strTemp << endl;
//	pEquity->setEquitySymbol(strTemp);
			
	  strTemp = pDlg->cmbInvestmentType->currentText();
		kdDebug(1) << "Equity Type is: " << strTemp << endl;
//	pEquity->setEquityType(strTemp);
			
		const double price = pDlg->getStockPrice();
		kdDebug(1) << "Current Equity Price is: " << price << endl;
    MyMoneyMoney money(price);
//  pEquity->setCurrentPrice(QDate::currentDate(), &money);
    	
    //add to equity database
    addEquityEntry(equity);
    	
    //display new equity in the list view.
    //displayNewEquity(equity);
	}
}   */

void KInvestmentView::addEquityEntry(MyMoneyEquity* /*pEntry*/)
{/*
	if(m_pAccount)
	{
		MyMoneyBank *pBank = m_pAccount->bank();
		if(pBank)
		{
			MyMoneyFile *pFile = pBank->file();
			if(pFile)
			{
				pFile->addEquityEntry(pEntry);
			}
		}
	}
*/
}

void KInvestmentView::displayNewEquity(MyMoneyEquity* /*pEntry*/)
{
/*
	KInvestmentListItem *pItem = new KInvestmentListItem(investmentTable, pEntry);
	investmentTable->insertItem(pItem);
*/
}

void KInvestmentView::slotEditInvestment()
{
	
}

void KInvestmentView::slotUpdatePrice()
{
/*
	KUpdateStockPriceDlg *pDlg = new KUpdateStockPriceDlg;
	if(pDlg)
	{
		pDlg->exec();
		int nResult = pDlg->result();
	}
*/
}

void KInvestmentView::slotListRightMouse(QListViewItem* /*item*/, const QPoint& /*point*/, int)
{
/*
  // setup the context menu
  KIconLoader *kiconloader = KGlobal::iconLoader();
  m_popMenu = new KPopupMenu(this);
  m_popMenu->insertTitle(kiconloader->loadIcon("transaction", KIcon::MainToolbar), i18n("Transaction Options"));
  m_popMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("New Investment"), this, SLOT(slotNewInvestment()));
  m_popMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("Edit Investment Properties"), this, SLOT(slotEditInvestment()));
  m_popMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("Update Investment Price"), this, SLOT(slotUpdatePrice()));
	//m_popMenu = m_contextMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small),
  //                      i18n("Delete ..."),
  //                      this, SLOT(slotDeleteSplitTransaction()));
  if(m_popMenu)
  {
  	m_popMenu->exec(QCursor::pos());
  }
*/
}

void KInvestmentView::slotViewChanged(int ID)
{
	switch(ID)
	{
		case VIEW_SUMMARY:
		{
//			investmentTable->show();
//			transactionTable->hide();
			break;
		}
		case VIEW_INVESTMENT:
		{
//			investmentTable->hide();
//			transactionTable->show();
			break;
		}
		default:
		{
			break;
		}
	}
}

void KInvestmentView::slotReloadView(void)
{
  // qDebug("KInvestmentView::slotReloadView()");

  // make sure to determine the current account from scratch
  m_account = MyMoneyAccount();

  slotRefreshView();
}

void KInvestmentView::slotRefreshView(void)
{
  QCString id = m_account.id();

  // qDebug("KGlobalLedgerView::slotRefreshView()");

  // load the combobox from scratch and determine the current account
  loadAccounts();

  // if the current account differs from the previous selection
  // then select the correct ledgerview first and force loading
  // the newly selected account
  if(m_account.id() != id) {
    slotSelectAccount(m_account.id());
  } else if(m_account.id().isEmpty()) {
    slotSelectAccount(QCString());
  } else {
    m_ledgerView->refreshView();
  }

  m_accountComboBox->setEnabled(m_accountComboBox->count() > 0);
}

void KInvestmentView::loadAccounts(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QString currentName;

  // qDebug("KGlobalLedgerView::loadAccounts()");
  m_accountComboBox->clear();

  MyMoneyAccount acc, subAcc;

  // check if the current account still exists and make it the
  // current account
  if(!m_account.id().isEmpty()) {
    try {
      acc = file->account(m_account.id());
      currentName = acc.name();
    } catch(MyMoneyException *e) {
      delete e;
      m_account = MyMoneyAccount();
    }
  }

  // load all asset and liability accounts into the combobox
  QCStringList::ConstIterator it_s;
  acc = file->asset();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s) {
    subAcc = file->account(*it_s);
    if(subAcc.accountType() == MyMoneyAccount::Investment) {
      m_accountComboBox->insertItem(subAcc.name());
      if(m_account == MyMoneyAccount()) {
        currentName = subAcc.name();
      }
    }
  }

  // sort list by name of accounts
  m_accountComboBox->listBox()->sort();
  if(!currentName.isEmpty())
    m_accountComboBox->setCurrentItem(currentName);
}

const bool KInvestmentView::slotSelectAccount(const QCString& id, const bool reconciliation)
{
/*
  bool    rc = false;

  // qDebug("KInvestmentView::slotSelectAccount(const QCString& id, const bool reconciliation)");

  // cancel any pending edit operation in the ledger views
  emit cancelEdit();

  if(!id.isEmpty()) {
    // if the account id differs, then we have to do something
    MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
    if(m_accountId != id) {
      if(m_specificView[acc.accountType()] != 0) {
        m_accountStack->raiseWidget(acc.accountType());
        m_currentView = m_specificView[acc.accountType()];
        m_currentView->slotSelectAccount(id);
        m_accountComboBox->setCurrentItem(acc.name());
        rc = true;

      } else {
        QString msg = "Specific ledger view for account type '" +
          KMyMoneyUtils::accountTypeToString(acc.accountType()) + "' not yet implemented";
        KMessageBox::sorry(0, msg, "Implementation problem");
      }
    } else {
#if KDE_VERSION < 310
      // in KDE 3.1 and above, QWidgetStack::show() takes care of this
      m_accountStack->raiseWidget(acc.accountType());
#endif
      rc = true;
    }

  } else {
    if(m_specificView[MyMoneyAccount::Checkings] != 0) {
      m_accountStack->raiseWidget(MyMoneyAccount::Checkings);
      m_currentView = m_specificView[MyMoneyAccount::Checkings];
      m_currentView->slotSelectAccount(id);

    } else {
      qFatal("Houston: we have a serious problem in KInvestmentView");
    }
  }

  // keep this as the current account
  m_accountId = id;

  if(reconciliation == true && m_currentView)
    m_currentView->slotReconciliation();

  return rc;
*/
}

const bool KInvestmentView::slotSelectAccount(const QString& accountName)
{
  // qDebug("KGlobalLedgerView::slotSelectAccount(const QString& accountName)");

  QCString id = MyMoneyFile::instance()->nameToAccount(accountName);
  bool     rc = false;
  if(!id.isEmpty()) {
    rc = slotSelectAccount(id);
  }
  return rc;
}


