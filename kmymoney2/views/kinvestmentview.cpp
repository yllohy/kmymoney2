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

#include <qpushbutton.h>
#include <kcombobox.h>
#include <qtabwidget.h>
#include <qtable.h>
#include <qinputdialog.h>

#include "../mymoney/mymoneytransaction.h"
#include "../mymoney/mymoneyinvesttransaction.h"
#include "../widgets/kmymoneytable.h"
#include "../mymoney/mymoneyaccount.h"
#include "kinvestmentview.h"

KInvestmentView::KInvestmentView(QWidget *parent, const char *name)
 :  kInvestmentViewDecl(parent,name)
{
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
	
  KConfig *config = KGlobal::config();
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
