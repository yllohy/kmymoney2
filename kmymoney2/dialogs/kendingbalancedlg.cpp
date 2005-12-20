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
#include <kstandarddirs.h>
#include <kapplication.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kendingbalancedlg.h"
#include "../mymoney/mymoneysplit.h"
#include "../mymoney/mymoneyfile.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneyaccountselector.h"

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

  // We don't need to add the default into the list (see ::help() why)
  // m_helpAnchor[m_startPageCheckings] = QString("");
  m_helpAnchor[m_interestChargeCheckings] = QString("details.reconcile.wizard.interest");
  m_helpAnchor[m_statementInfoPageCheckings] = QString("details.reconcile.wizard.statement");

  if(m_account.accountGroup() == MyMoneyAccount::Liability)
    m_previousBalance->loadText((-MyMoneyMoney(value)).formatMoney());
  else
    m_previousBalance->loadText(MyMoneyMoney(value).formatMoney());

  value = account.value("statementBalance");
  if(m_account.accountGroup() == MyMoneyAccount::Liability)
    m_endingBalance->loadText((-MyMoneyMoney(value)).formatMoney());
  else
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
  removePage(m_startPageLoan);
  removePage(m_checkPaymentsPage);
  removePage(m_adjustmentTransactionPage);

  // connect the signals with the slots
  connect(m_interestEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_interestCategoryEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_interestCategoryEdit, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));
  connect(m_chargesEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_chargesCategoryEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_chargesCategoryEdit, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));
/*
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

const MyMoneyMoney KEndingBalanceDlg::endingBalance(void) const
{
  return adjustedReturnValue(m_endingBalance->value());
}

const MyMoneyMoney KEndingBalanceDlg::previousBalance(void) const
{
  return adjustedReturnValue(m_previousBalance->value());
}

const MyMoneyMoney KEndingBalanceDlg::adjustedReturnValue(const MyMoneyMoney& v) const
{
  return m_account.accountGroup() == MyMoneyAccount::Liability ? -v : v;
}

void KEndingBalanceDlg::slotCheckPageFinished(void)
{
  nextButton()->setEnabled(true);
  finishButton()->setEnabled(true);

  if(currentPage() == m_interestChargeCheckings) {
    int cnt1, cnt2;
    cnt1 = !m_interestEdit->value().isZero() + !m_interestCategoryEdit->text().isEmpty();
    cnt2 = !m_chargesEdit->value().isZero() + !m_chargesCategoryEdit->text().isEmpty();
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
  MyMoneyMoney val = amountEdit->value() * MyMoneyMoney(sign,1);
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

void KEndingBalanceDlg::help(void)
{
  QString anchor = m_helpAnchor[currentPage()];
  if(anchor.isEmpty())
    anchor = QString("details.reconcile.whatis");

  kapp->invokeHelp(anchor);
}

KEndingBalanceLoanDlg::KEndingBalanceLoanDlg(const MyMoneyAccount& account, QWidget *parent, const char *name)
 : KEndingBalanceDlgDecl(parent, name, true),
   m_account(account)
{
  QString value;
  value = account.value("lastStatementDate");
  if(value.isEmpty())
    m_startDateEdit->setDate(m_account.openingDate());
  else
    m_startDateEdit->setDate(QDate::fromString(value, Qt::ISODate).addDays(1));

  // make sure, we show the correct start page
  showPage(m_startPageLoan);

  // enable the finish button on the last page
  setAppropriate(m_checkPaymentsPage, true);

  // remove all unwanted pages
  removePage(m_startPageCheckings);
  removePage(m_statementInfoPageCheckings);
  removePage(m_pagePreviousPostpone);
  removePage(m_interestChargeCheckings);

  // connect the signals with the slots
  connect(m_amortizationTotalEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_interestTotalEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_accountEdit, SIGNAL(stateChanged(void)), this, SLOT(slotCheckPageFinished(void)));
  connect(m_categoryEdit, SIGNAL(stateChanged(void)), this, SLOT(slotCheckPageFinished(void)));
}

KEndingBalanceLoanDlg::~KEndingBalanceLoanDlg()
{
}

void KEndingBalanceLoanDlg::slotCheckPageFinished(void)
{
  nextButton()->setEnabled(true);
  finishButton()->setEnabled(true);

  if(currentPage() == m_checkPaymentsPage) {
    MyMoneyMoney interest = totalInterest(m_startDateEdit->getQDate(), m_endDateEdit->getQDate());
    MyMoneyMoney amortization = totalAmortization(m_startDateEdit->getQDate(), m_endDateEdit->getQDate());

    if(interest == m_interestTotalEdit->value()
    && amortization == m_amortizationTotalEdit->value()) {
      if(indexOf(m_adjustmentTransactionPage) != -1) {
        removePage(m_adjustmentTransactionPage);
        // the following line forces to update the buttons
        showPage(m_checkPaymentsPage);
        nextButton()->setEnabled(true);
        finishButton()->setEnabled(true);
      }
    } else {
      if(indexOf(m_adjustmentTransactionPage) == -1) {
        addPage(m_adjustmentTransactionPage, i18n("Adjustment transaction"));
        // the following line forces to update the buttons
        showPage(m_checkPaymentsPage);
      }
    }
  } else if(currentPage() == m_adjustmentTransactionPage) {
    if(m_accountEdit->selectedAccounts().count() == 0) {
      nextButton()->setEnabled(false);
      finishButton()->setEnabled(false);

    } else if(m_categoryEdit->isEnabled()
    && m_categoryEdit->selectedAccounts().count() == 0) {
      nextButton()->setEnabled(false);
      finishButton()->setEnabled(false);
    }
  }
}

const MyMoneyMoney KEndingBalanceLoanDlg::totalInterest(const QDate& start, const QDate& end) const
{
  MyMoneyMoney  interest;
  MyMoneyTransactionFilter  filter(m_account.id());
  filter.setDateFilter(start, end);

  QValueList<MyMoneyTransaction> list = MyMoneyFile::instance()->transactionList(filter);
  QValueList<MyMoneyTransaction>::ConstIterator it_t;

  for(it_t = list.begin(); it_t != list.end(); ++it_t) {
    QValueList<MyMoneySplit>::ConstIterator it_s;
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      if((*it_s).action() == MyMoneySplit::ActionInterest) {
        interest += (*it_s).value();
      }
    }
  }
  return interest;
}

const MyMoneyMoney KEndingBalanceLoanDlg::totalAmortization(const QDate& start, const QDate& end) const
{
  MyMoneyMoney  amortization;
  MyMoneyMoney  adjust(1,1);
  MyMoneyTransactionFilter  filter(m_account.id());
  filter.setDateFilter(start, end);

  if(m_account.accountType() == MyMoneyAccount::AssetLoan)
    adjust = -adjust;

  QValueList<MyMoneyTransaction> list = MyMoneyFile::instance()->transactionList(filter);
  QValueList<MyMoneyTransaction>::ConstIterator it_t;

  for(it_t = list.begin(); it_t != list.end(); ++it_t) {
    QValueList<MyMoneySplit>::ConstIterator it_s;
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      if((*it_s).accountId() == m_account.id()
      && (*it_s).action() == MyMoneySplit::ActionAmortization
      && ((*it_s).value() * MyMoneyMoney(adjust, 1)).isPositive()) {
        amortization += (*it_s).value();
      }
    }
  }
  // make sure to return a positive number
  return amortization * adjust;
}

void KEndingBalanceLoanDlg::next(void)
{
  bool dontLeavePage = false;

  if(currentPage() == m_startPageLoan) {
    MyMoneyMoney interest = totalInterest(m_startDateEdit->getQDate(), m_endDateEdit->getQDate());
    MyMoneyMoney amortization = totalAmortization(m_startDateEdit->getQDate(), m_endDateEdit->getQDate());

    m_loanOverview->setText(i18n("KMyMoney has calculated the following amounts for "
                                 "interest and amortization according to recorded payments "
                                 "between %1 and %2.")
                                 .arg(KGlobal::locale()->formatDate(m_startDateEdit->getQDate(), true))
                                 .arg(KGlobal::locale()->formatDate(m_endDateEdit->getQDate(), true)));

    // preload widgets with calculated values if they are empty
    if(m_amortizationTotalEdit->text().isEmpty())
      m_amortizationTotalEdit->loadText(amortization.formatMoney());
    if(m_interestTotalEdit->text().isEmpty())
      m_interestTotalEdit->loadText(interest.formatMoney());

  } else if(currentPage() == m_checkPaymentsPage) {
    m_accountEdit->loadList(static_cast<KMyMoneyUtils::categoryTypeE>(KMyMoneyUtils::asset | KMyMoneyUtils::liability));
    m_categoryEdit->loadList(static_cast<KMyMoneyUtils::categoryTypeE>(KMyMoneyUtils::income | KMyMoneyUtils::expense));
    m_categoryEdit->setEnabled(false);

    MyMoneyMoney interest = totalInterest(m_startDateEdit->getQDate(), m_endDateEdit->getQDate());
    if(interest != m_interestTotalEdit->value()) {
      m_categoryEdit->setEnabled(true);
    }
  }

  if(!dontLeavePage)
    KEndingBalanceDlgDecl::next();

  slotCheckPageFinished();
}

const MyMoneyTransaction KEndingBalanceLoanDlg::adjustmentTransaction(void) const
{
  MyMoneyTransaction t;

  MyMoneyMoney interest = totalInterest(m_startDateEdit->getQDate(), m_endDateEdit->getQDate());
  MyMoneyMoney amortization = totalAmortization(m_startDateEdit->getQDate(), m_endDateEdit->getQDate());

  if(interest != m_interestTotalEdit->value()
  || amortization != m_amortizationTotalEdit->value()) {
    MyMoneySplit sAccount, sAmortization, sInterest;
    int          adjust = 1;

    if(m_account.accountType() == MyMoneyAccount::AssetLoan)
      adjust = -1;

    // fix sign if asset
    interest = interest * MyMoneyMoney(adjust,1);
    amortization = amortization * MyMoneyMoney(adjust,1);

    sAmortization.setValue((m_amortizationTotalEdit->value() - amortization) * MyMoneyMoney(adjust,1));
    sInterest.setValue((m_interestTotalEdit->value() - interest) * MyMoneyMoney(adjust,1));
    sAccount.setValue( -(sAmortization.value() + sInterest.value()));

    try {
      sAmortization.setAccountId(m_account.id());
      sAmortization.setPayeeId(m_account.payee());
      sAccount.setAccountId(m_accountEdit->selectedAccounts()[0]);
      sAccount.setPayeeId(m_account.payee());
      if(m_categoryEdit->isEnabled())
        sInterest.setAccountId(m_categoryEdit->selectedAccounts()[0]);

      sAccount.setMemo(i18n("Adjustment transaction"));
      sAmortization.setMemo(sAccount.memo());
      sInterest.setMemo(sAccount.memo());

      sAccount.setAction(MyMoneySplit::ActionAmortization);
      sAmortization.setAction(MyMoneySplit::ActionAmortization);
      sInterest.setAction(MyMoneySplit::ActionInterest);

      t.addSplit(sAccount);
      t.addSplit(sAmortization);
      if(!sInterest.value().isZero())
        t.addSplit(sInterest);

      t.setPostDate(m_endDateEdit->getQDate());

    } catch(MyMoneyException *e) {
      qDebug("Unable to create adjustment transaction for loan reconciliation: %s", e->what().data());
      delete e;
      return MyMoneyTransaction();
    }
  }
  return t;
}

void KEndingBalanceLoanDlg::help(void)
{
  QString anchor = m_helpAnchor[currentPage()];
  if(anchor.isEmpty())
    anchor = QString("details.reconcile.whatis");

  kapp->invokeHelp(anchor);
}

#include "kendingbalancedlg.moc"

