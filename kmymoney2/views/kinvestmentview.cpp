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
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klistview.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include <qpushbutton.h>
#include <kcombobox.h>
#include <qtabwidget.h>
#include <qtable.h>
#include <qinputdialog.h>
#include <qlineedit.h>

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyutils.h"
#include "../mymoney/mymoneyequity.h"
#include "../mymoney/mymoneytransaction.h"
#include "../mymoney/mymoneyinvesttransaction.h"
#include "../widgets/kmymoneytable.h"
#include "../mymoney/mymoneyaccount.h"
#include "kinvestmentview.h"
#include "../dialogs/knewequityentrydlg.h"

KInvestmentView::KInvestmentView(QWidget *parent, const char *name)
 :  kInvestmentViewDecl(parent,name)
{
	m_pAccount 	= NULL;
	m_popMenu		= NULL;

	qDebug("KInvestmentView::KInvestmentView: Investment View starting up");

	investmentTable->setRootIsDecorated(true);
	investmentTable->setColumnText(0, QString(i18n("Symbol")));
  investmentTable->addColumn(i18n("Name"));
  investmentTable->addColumn(i18n("Number of Shares"));
  investmentTable->addColumn(i18n("Current Price"));
  investmentTable->addColumn(i18n("Original Price"));
  investmentTable->addColumn(i18n("$ Gain"));
  investmentTable->addColumn(i18n("1 week %"));
  investmentTable->addColumn(i18n("4 weeks %"));
  investmentTable->addColumn(i18n("3 Months %"));
  investmentTable->addColumn(i18n("YTD %"));

  investmentTable->setMultiSelection(false);
//  m_qlistviewScheduled->setColumnWidthMode(0, QListView::Manual);
  investmentTable->header()->setResizeEnabled(false);
  investmentTable->setAllColumnsShowFocus(true);

  // never show a horizontal scroll bar
  investmentTable->setHScrollBarMode(QScrollView::AlwaysOff);

  investmentTable->setSorting(-1);

  connect(investmentTable, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));
}

KInvestmentView::~KInvestmentView()
{
}

/** No descriptions */
bool KInvestmentView::init(MyMoneyAccount *pAccount)
{
	if(!pAccount)
	{
		return false;
	}

	m_pAccount = pAccount;
//  KConfig *config = KGlobal::config();
  QDateTime defaultDate = QDate::currentDate();
  QDate qdateStart = QDate::currentDate();//config->readDateTimeEntry("StartDate", &defaultDate).date();

  if(qdateStart != defaultDate.date())
  {
  	MyMoneyInvestTransaction *pInvestTransaction = NULL;
    MyMoneyTransaction *transaction = NULL;
    m_transactionList.clear();

    for(transaction=pAccount->transactionFirst(); transaction; transaction=pAccount->transactionNext())
    {
      if(transaction->date() >= qdateStart)
      {
      	if(transaction)
      	{
        	pInvestTransaction = static_cast<MyMoneyInvestTransaction*>(transaction);
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
            transaction->state()));
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

void KInvestmentView::slotNewInvestment()
{
	MyMoneyEquity *pEquity = NULL;
	KNewEquityEntryDlg *pDlg = new KNewEquityEntryDlg(this);
	pDlg->exec();
	int nResult = pDlg->result();
	if(nResult)
	{
		pEquity = new MyMoneyEquity;
    if(pEquity)
    {
			//populate this equity entry with information from the dialog.
			pEquity->setEquityName(String(pDlg->edtEquityName->text()));
			pEquity->setEquitySymbol(String(pDlg->edtMarketSymbol->text()));
			pEquity->setEquityType(String(pDlg->cmbInvestmentType->currentText()));
    	MyMoneyMoney money(pDlg->dblCurrentPrice->value());
    	pEquity->setCurrentPrice(&money);
    	addEquityEntry(pEquity);
    }
	}
}

void KInvestmentView::addEquityEntry(MyMoneyEquity *pEntry)
{
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
}

void KInvestmentView::displayNewEquity(const MyMoneyEquity *pEntry)
{
}

void KInvestmentView::slotEditInvestment()
{
	
}

void KInvestmentView::slotListDoubleClicked(QListViewItem* pItem, const QPoint& pos, int c)
{
}

void KInvestmentView::slotListRightMouse(QListViewItem* item, const QPoint& point, int)
{
  // setup the context menu
  KIconLoader *kiconloader = KGlobal::iconLoader();
  m_popMenu = new KPopupMenu(this);
  m_popMenu->insertTitle(kiconloader->loadIcon("transaction", KIcon::MainToolbar), i18n("Transaction Options"));
  m_popMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("New Investment"), this, SLOT(slotNewInvestment()));
  m_popMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("Edit Investment Properties"), this, SLOT(slotEditInvestment()));

	//m_popMenu = m_contextMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small),
  //                      i18n("Delete ..."),
  //                      this, SLOT(slotDeleteSplitTransaction()));
  if(m_popMenu)
  {
  	m_popMenu->exec(QCursor::pos());
  }
}
