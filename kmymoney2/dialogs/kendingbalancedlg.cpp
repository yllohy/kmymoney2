/***************************************************************************
                          kendingbalancedlg.cpp
                             -------------------
    copyright            : (C) 2000,2003 by Michael Edwardes, Thomas Baumgart
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
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

#include <qpixmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kendingbalancedlg.h"
#include "../mymoney/mymoneysplit.h"
#include "../mymoney/mymoneyfile.h"
#include "../widgets/kmymoneycategory.h"

KEndingBalanceDlg::KEndingBalanceDlg(const MyMoneyAccount& account, QWidget *parent, const char *name)
 : KEndingBalanceDlgDecl(parent, name, true)
{
  QString value;

  m_account = account;
    
  // If the previous reconciliation was postponed,
  // we show a different first page
  value = account.value("lastReconciledBalance");
  if(value.isEmpty()) {
    value = account.value("lastStatementBalance");
    setAppropriate(m_startPageCheckings, true);
    setAppropriate(m_pagePreviousPostpone, false);
    setAppropriate(m_interestChargeCheckings, true);
    setFinishEnabled(m_interestChargeCheckings, true);
  } else {
    setAppropriate(m_startPageCheckings, false);
    setAppropriate(m_pagePreviousPostpone, true);
    removePage(m_interestChargeCheckings);
    setFinishEnabled(m_statementInfoPageCheckings, true);
    // make sure, we show the correct start page
    showPage(m_pagePreviousPostpone);
  }
  
  m_previousBalance->loadText(MyMoneyMoney(value).formatMoney());

  value = account.value("statementBalance");
  m_endingBalance->loadText(MyMoneyMoney(value).formatMoney());
  
  m_statementDate->setDate(QDate::currentDate());
  value = account.value("statementDate");
  if(!value.isEmpty())
    m_statementDate->setDate(QDate::fromString(value, Qt::ISODate));

  value = account.value("lastStatementDate");
  m_lastStatementDate->setText(QString());
  if(!value.isEmpty()) {
    m_lastStatementDate->setText(i18n("Last reconciled statement: %1")
      .arg(KGlobal::locale()->formatDate(QDate::fromString(value, Qt::ISODate), true)));
  }

  // remove all unwanted pages
  setAppropriate(m_startPageLoan, false);

  
  // FIXME: we need the online help first
  helpButton()->hide();

  // connect the signals with the slots
  connect(m_interestEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_interestCategoryEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_chargesEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_chargesCategoryEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
/*
   //QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_ending_balance.png");
  m_qpixmaplabel->setPixmap(QPixmap(KGlobal::dirs()->findResource("appdata", "pics/dlg_ending_balance.png")));

	previousbalEdit->setText(prevBal.formatMoney());
	previousbalEdit->setFocus();
	previousbalEdit->setSelection(0, previousbalEdit->text().length());	
	
	endingEdit->setText(endingGuess.formatMoney());

  endingDateEdit->setDate(statementDate);
	
	connect(cancelBtn, SIGNAL(clicked()), SLOT(reject()));
	connect(okBtn, SIGNAL(clicked()), SLOT(okClicked()));
*/
}

KEndingBalanceDlg::~KEndingBalanceDlg()
{
}

void KEndingBalanceDlg::slotCheckPageFinished(void)
{
  nextButton()->setEnabled(true);
  finishButton()->setEnabled(true);
  
  if(currentPage() == m_interestChargeCheckings) {
    int cnt1, cnt2;
    cnt1 = !m_interestEdit->text().isEmpty() + !m_interestCategoryEdit->text().isEmpty();
    cnt2 = !m_chargesEdit->text().isEmpty() + !m_chargesCategoryEdit->text().isEmpty();
    if(cnt1 == 1 || cnt2 == 1) {
      finishButton()->setEnabled(false);
      nextButton()->setEnabled(false);
    }
  }
}

const MyMoneyTransaction KEndingBalanceDlg::interestTransaction(void) const
{
  return createTransaction(-1, m_interestEdit, m_interestCategoryEdit);
}

const MyMoneyTransaction KEndingBalanceDlg::chargeTransaction(void) const
{
  return createTransaction(1, m_chargesEdit, m_chargesCategoryEdit);
}

const MyMoneyTransaction KEndingBalanceDlg::createTransaction(const int sign, kMyMoneyEdit *amountEdit, kMyMoneyCategory *categoryEdit) const
{
  MyMoneyTransaction t;
  MyMoneyFile* file = MyMoneyFile::instance();
  
  if(!amountEdit->isValid() || categoryEdit->text().isEmpty())
    return t;

  MyMoneySplit s1, s2;
  MyMoneyMoney val = amountEdit->getMoneyValue() * sign;
  try {
    QCString accId = file->categoryToAccount(categoryEdit->text());
    if(accId.isEmpty())
      throw new MYMONEYEXCEPTION("category not found");

    try {
      // if we find a payee with the same name as the institution,
      // than this is what we use as payee.
      if(!m_account.institutionId().isEmpty()) {
        MyMoneyInstitution inst = file->institution(m_account.institutionId());
        MyMoneyPayee payee = file->payeeByName(inst.name());
        s1.setPayeeId(payee.id());
      }
    } catch(MyMoneyException *e) {
      delete e;
    }

    if(sign == 1)
      s1.setAction(MyMoneySplit::ActionWithdrawal);
    else
      s1.setAction(MyMoneySplit::ActionDeposit);
          
    s1.setReconcileFlag(MyMoneySplit::Cleared);
    s1.setReconcileDate(QDate::currentDate());
    s2 = s1;
    s1.setAccountId(m_account.id());
    s1.setValue(-val);

    s2.setAccountId(accId);
    s2.setValue(val);

    t.addSplit(s1);
    t.addSplit(s2);
    t.setPostDate(m_statementDate->getQDate());
    
  } catch(MyMoneyException *e) {
    qDebug("%s", e->what().data());
    delete e;
    return MyMoneyTransaction();
  }
  
  return t;
}

void KEndingBalanceDlg::okClicked()
{
/*  
  m_endingBalance = endingEdit->getMoneyValue();
	m_previousBalance = previousbalEdit->getMoneyValue();
  m_endingDate = endingDateEdit->getQDate();

  // removed the date check because it can't be invalid !
  accept();
*/
}


KEndingBalanceLoanDlg::KEndingBalanceLoanDlg(const MyMoneyAccount& account, QWidget *parent, const char *name)
 : KEndingBalanceDlgDecl(parent, name, true)
{
  // enable the finish button on the last page
  setAppropriate(m_startPageLoan, true);

  // remove all unwanted pages
  setAppropriate(m_startPageCheckings, false);
  setAppropriate(m_statementInfoPageCheckings, false);
  setAppropriate(m_interestChargeCheckings, false);
  
  // FIXME: we need the online help first
  helpButton()->hide();
}

KEndingBalanceLoanDlg::~KEndingBalanceLoanDlg()
{
}

void KEndingBalanceLoanDlg::okClicked()
{
/*
  m_endingBalance = endingEdit->getMoneyValue();
	m_previousBalance = previousbalEdit->getMoneyValue();
  m_endingDate = endingDateEdit->getQDate();

  // removed the date check because it can't be invalid !
  accept();
*/
}
