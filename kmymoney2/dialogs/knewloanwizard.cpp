/***************************************************************************
                          knewloanwizard.cpp  -  description
                             -------------------
    begin                : Wed Oct 8 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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

#include <math.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kpushbutton.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewloanwizard.h"

#include "../kmymoneyutils.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneyaccountselector.h"

#include "../dialogs/knewaccountdlg.h"
#include "../dialogs/ksplittransactiondlg.h"

#include "../mymoney/mymoneyfinancialcalculator.h"
#include "../mymoney/mymoneyfile.h"

#include "../kmymoney2.h"

KNewLoanWizard::KNewLoanWizard(QWidget *parent, const char *name ) :
  KNewLoanWizardDecl(parent, name, true)
{
  connect(m_borrowButton, SIGNAL(clicked()), this, SLOT(slotLiabilityLoan()));
  connect(m_lendButton, SIGNAL(clicked()), this, SLOT(slotAssetLoan()));

  connect(m_nameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished()));
  connect(m_payeeEdit, SIGNAL(newPayee(const QString&)), this, SLOT(slotNewPayee(const QString&)));

  connect(m_previousPaymentButton, SIGNAL(clicked()), this, SLOT(slotPaymentsMade()));
  connect(m_noPreviousPaymentButton, SIGNAL(clicked()), this, SLOT(slotNoPaymentsMade()));

  connect(m_allPaymentsButton, SIGNAL(clicked()), this, SLOT(slotRecordAllPayments()));
  connect(m_thisYearPaymentButton, SIGNAL(clicked()), this, SLOT(slotRecordThisYearsPayments()));

  connect(m_firstDueDateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotCheckPageFinished()));

  connect(m_interestOnPaymentButton, SIGNAL(clicked()), this, SLOT(slotInterestOnPayment()));
  connect(m_interestOnReceptionButton, SIGNAL(clicked()), this, SLOT(slotInterestOnReception()));

  connect(m_loanAmountEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished()));

  connect(m_interestAccountEdit, SIGNAL(stateChanged()), this, SLOT(slotCheckPageFinished()));

  connect(m_nextDueDateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotCheckPageFinished()));
  connect(m_paymentAccountEdit,  SIGNAL(stateChanged()), this, SLOT(slotCheckPageFinished()));

  connect(m_assetAccountEdit,  SIGNAL(stateChanged()), this, SLOT(slotCheckPageFinished()));
  connect(m_dontCreatePayoutCheckBox,  SIGNAL(clicked()), this, SLOT(slotCheckPageFinished()));

  loadComboBoxes();

  resetCalculator();

  // As default we assume a liability loan, with fixed interest rate,
  // with a first payment due on the 30th of this month. All payments
  // should be recorded and none have been made so far.
  m_dontCreatePayoutCheckBox->setChecked(false);
  m_borrowButton->animateClick();
  m_fixedInterestButton->animateClick();
  m_noPreviousPaymentButton->animateClick();
  m_allPaymentsButton->animateClick();
  m_interestOnReceptionButton->animateClick();

  m_interestFrequencyAmountEdit->setValue(1);
  m_interestFrequencyUnitEdit->setCurrentItem(i18n("Years"));
  m_paymentFrequencyUnitEdit->setCurrentItem(KMyMoneyUtils::occurenceToString(MyMoneySchedule::OCCUR_MONTHLY));
  m_firstDueDateEdit->loadDate(QDate(QDate::currentDate().year(),QDate::currentDate().month(),30));

  // load button icons
  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem createCategoryButtenItem( i18n( "&Create..." ),
                      QIconSet(il->loadIcon("filenew", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Create a new category"),
                      i18n("Use this to open the new account editor"));
  m_createCategoryButton->setGuiItem(createCategoryButtenItem);
  connect(m_createCategoryButton, SIGNAL(clicked()), this, SLOT(slotCreateCategory()));

  KGuiItem additionalFeeButtenItem( i18n( "&Additional fees..." ),
                      0, //QIconSet(il->loadIcon("filenew", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Enter additional fees"),
                      i18n("Use this to add any additional fees other than principal and interest contained in your periodical payments."));
  m_additionalFeeButton->setGuiItem(additionalFeeButtenItem);
  connect(m_additionalFeeButton, SIGNAL(clicked()), this, SLOT(slotAdditionalFees()));

  KGuiItem createAssetButtenItem( i18n( "&Create..." ),
                      QIconSet(il->loadIcon("filenew", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Create a new asset account"),
                      i18n("Use this to create a new account to which the initial payment should be made"));
  m_createNewAssetButton->setGuiItem(createAssetButtenItem);
  connect(m_createNewAssetButton, SIGNAL(clicked()), kmymoney2, SLOT(slotAccountNew()));

  // enable the finish button on the last page
  setFinishEnabled(m_summaryPage, true);

  // FIXME: we currently only support interest calculation on reception
  setAppropriate(m_interestCalculationPage, false);

  // turn off all pages that are contained here for derived classes
  setAppropriate(m_editIntroPage, false);
  setAppropriate(m_editSelectionPage, false);
  setAppropriate(m_effectiveDatePage, false);
  setAppropriate(m_paymentEditPage, false);
  setAppropriate(m_interestEditPage, false);
  setAppropriate(m_summaryEditPage, false);

  // for now, we don't have online help :-(
  helpButton()->hide();

  // setup a phony transaction for additional fee processing
  m_account = MyMoneyAccount(QCString("Phony-ID"), MyMoneyAccount());
  MyMoneySplit split;
  split.setAccountId(m_account.id());
  split.setValue(0);
  m_transaction.addSplit(split);
}

KNewLoanWizard::~KNewLoanWizard()
{
}

void KNewLoanWizard::resetCalculator(void)
{
  m_loanAmount1->setText(QString());
  m_interestRate1->setText(QString());
  m_duration1->setText(QString());
  m_payment1->setText(QString());
  m_balloon1->setText(QString());

  m_loanAmount2->setText(QString());
  m_interestRate2->setText(QString());
  m_duration2->setText(QString());
  m_payment2->setText(QString());
  m_balloon2->setText(QString());

  m_loanAmount3->setText(QString());
  m_interestRate3->setText(QString());
  m_duration3->setText(QString());
  m_payment3->setText(QString());
  m_balloon3->setText(QString());

  m_loanAmount4->setText(QString());
  m_interestRate4->setText(QString());
  m_duration4->setText(QString());
  m_payment4->setText(QString());
  m_balloon4->setText(QString());

  m_loanAmount5->setText(QString());
  m_interestRate5->setText(QString());
  m_duration5->setText(QString());
  m_payment5->setText(QString());
  m_balloon5->setText(QString());

  m_additionalCost->setText(MyMoneyMoney(0).formatMoney());
}

void KNewLoanWizard::slotNewPayee(const QString& payeeName)
{
  KMyMoneyUtils::newPayee(this, m_payeeEdit, payeeName);
}

void KNewLoanWizard::slotLiabilityLoan(void)
{
  m_generalReceiverText->setText(i18n("To whom do you make payments?"));
  m_receiverLabel->setText(i18n("Payments to"));
}

void KNewLoanWizard::slotAssetLoan(void)
{
  m_generalReceiverText->setText(i18n("From whom do you expect payments?"));
  m_receiverLabel->setText(i18n("Payments from"));
}

void KNewLoanWizard::slotPaymentsMade(void)
{
  setAppropriate(m_recordPaymentPage, true);
}

void KNewLoanWizard::slotNoPaymentsMade(void)
{
  m_allPaymentsButton->animateClick();
  setAppropriate(m_recordPaymentPage, false);
}

void KNewLoanWizard::slotRecordAllPayments(void)
{
  m_firstPaymentLabel->setText(
      QString("\n") +
      i18n("Please enter the date, the first payment for this loan was/is due."));
  m_firstPaymentNote->setText(
      i18n("Note: Consult the loan contract for details of the first due date. "
           "Keep in mind, that the first due date usually differs from the date "
           "the contract was signed"));
  m_balanceLabel->setText(
      QString("\n") +
      i18n("Please enter the original loan amount in the field below or leave it "
           "empty to be calculated."));
}

void KNewLoanWizard::slotRecordThisYearsPayments(void)
{
  m_firstPaymentLabel->setText(
      QString("\n") +
      i18n("Please enter the date, the first payment for this loan was/is due this year."));
  m_firstPaymentNote->setText(
      i18n("Note: You can easily figure out the date of the first payment "
           "if you consult the last statement of last year."));
  m_balanceLabel->setText(
      QString("\n") +
      i18n("Please enter the remaining loan amount of last years final "
           "statement in the field below. You should not leave this field empty."));
}

void KNewLoanWizard::slotCheckPageFinished(void)
{
  nextButton()->setEnabled(false);

  if(currentPage() == m_namePage) {
    if(!m_nameEdit->text().isEmpty()) {
      nextButton()->setEnabled(true);
    }

  } else if(currentPage() == m_loanAmountPage) {
    nextButton()->setEnabled(true);
    if(m_thisYearPaymentButton->isChecked()
    && !m_loanAmountEdit->isValid()) {
      nextButton()->setEnabled(false);
    }

  } else if(currentPage() == m_interestCategoryPage) {
    if(m_interestAccountEdit->selectedAccounts().count() > 0) {
      nextButton()->setEnabled(true);
    }

  } else if(currentPage() == m_firstPaymentPage) {
    if(m_firstDueDateEdit->getQDate().isValid())
      nextButton()->setEnabled(true);

  } else if(currentPage() == m_schedulePage) {
    if(m_nextDueDateEdit->getQDate().isValid()
    && m_nextDueDateEdit->getQDate() >= m_firstDueDateEdit->getQDate()
    && m_paymentAccountEdit->selectedAccounts().count() > 0)
      nextButton()->setEnabled(true);

  } else if(currentPage() == m_assetAccountPage) {
    if(m_dontCreatePayoutCheckBox->isChecked()) {
      m_assetAccountEdit->setEnabled(false);
      m_paymentDate->setEnabled(false);
      m_createNewAssetButton->setEnabled(false);
      nextButton()->setEnabled(true);
    } else {
      m_assetAccountEdit->setEnabled(true);
      m_paymentDate->setEnabled(true);
      m_createNewAssetButton->setEnabled(true);
      if(!m_assetAccountEdit->selectedAccounts().isEmpty()
      && m_paymentDate->getQDate().isValid())
        nextButton()->setEnabled(true);
    }
  } else
    nextButton()->setEnabled(true);
}

void KNewLoanWizard::updateLoanAmount(void)
{
  QString txt;
  if(m_loanAmountEdit->text().isEmpty()) {
    txt = QString("<") + i18n("calculate") + QString(">");
  } else {
    txt = m_loanAmountEdit->value().formatMoney();
  }
  m_loanAmount1->setText(txt);
  m_loanAmount2->setText(txt);
  m_loanAmount3->setText(txt);
  m_loanAmount4->setText(txt);
  m_loanAmount5->setText(txt);
}

void KNewLoanWizard::updateInterestRate(void)
{
  QString txt;
  if(m_interestRateEdit->text().isEmpty()) {
    txt = QString("<") + i18n("calculate") + QString(">");
  } else {
    txt = m_interestRateEdit->value().formatMoney("", 3) + QString("%");
  }
  m_interestRate1->setText(txt);
  m_interestRate2->setText(txt);
  m_interestRate3->setText(txt);
  m_interestRate4->setText(txt);
  m_interestRate5->setText(txt);
}

void KNewLoanWizard::updateDuration(void)
{
  QString txt;
  if(m_durationValueEdit->value() == 0) {
    txt = QString("<") + i18n("calculate") + QString(">");
  } else {
    txt = QString().sprintf("%d ", m_durationValueEdit->value())
        + m_durationUnitEdit->currentText();
  }
  m_duration1->setText(txt);
  m_duration2->setText(txt);
  m_duration3->setText(txt);
  m_duration4->setText(txt);
  m_duration5->setText(txt);
}

void KNewLoanWizard::updatePayment(void)
{
  QString txt;
  if(m_paymentEdit->text().isEmpty()) {
    txt = QString("<") + i18n("calculate") + QString(">");
  } else {
    txt = m_paymentEdit->value().formatMoney();
  }
  m_payment1->setText(txt);
  m_payment2->setText(txt);
  m_payment3->setText(txt);
  m_payment4->setText(txt);
  m_payment5->setText(txt);
  m_basePayment->setText(txt);
}

void KNewLoanWizard::updateFinalPayment(void)
{
  QString txt;
  if(m_finalPaymentEdit->text().isEmpty()) {
    txt = QString("<") + i18n("calculate") + QString(">");
  } else {
    txt = m_finalPaymentEdit->value().formatMoney();
  }
  m_balloon1->setText(txt);
  m_balloon2->setText(txt);
  m_balloon3->setText(txt);
  m_balloon4->setText(txt);
  m_balloon5->setText(txt);
}

void KNewLoanWizard::updateLoanInfo(void)
{
  updateLoanAmount();
  updateInterestRate();
  updateDuration();
  updatePayment();
  updateFinalPayment();
  updatePeriodicPayment();

  QString txt;

  m_loanAmount6->setText(m_loanAmountEdit->value().formatMoney());
  m_interestRate6->setText(m_interestRateEdit->value().formatMoney("", 3) + QString("%"));
  txt = QString().sprintf("%d ", m_durationValueEdit->value())
        + m_durationUnitEdit->currentText();
  m_duration6->setText(txt);
  m_payment6->setText(m_paymentEdit->value().formatMoney());
  m_balloon6->setText(m_finalPaymentEdit->value().formatMoney());
}

void KNewLoanWizard::updatePeriodicPayment(void)
{
  MyMoneyMoney base(m_basePayment->text());
  MyMoneyMoney add(m_additionalCost->text());

  m_periodicPayment->setText((base + add).formatMoney());
}

void KNewLoanWizard::updateSummary(void)
{
  // General
  if(m_borrowButton->isChecked())
    m_summaryLoanType->setText(i18n("borrowed"));
  else
    m_summaryLoanType->setText(i18n("lend"));

  m_summaryFirstPayment->setText(KGlobal::locale()->formatDate(m_firstDueDateEdit->getQDate(), true));
  if(m_payeeEdit->text().isEmpty()) {
    m_summaryPayee->setText(i18n("not assigned"));
  } else {
    m_summaryPayee->setText(m_payeeEdit->text());
  }

  // Calculation
  if(m_interestOnReceptionButton->isChecked())
    m_summaryInterestDue->setText(i18n("on reception"));
  else
    m_summaryInterestDue->setText(i18n("on due date"));
  m_summaryPaymentFrequency->setText(m_paymentFrequencyUnitEdit->currentText());
  m_summaryAmount->setText(m_loanAmount6->text());
  m_summaryInterestRate->setText(m_interestRate6->text());
  m_summaryTerm->setText(m_duration6->text());
  m_summaryPeriodicPayment->setText(m_payment6->text());
  m_summaryBalloonPayment->setText(m_balloon6->text());

  // Payment
  try {
    QCStringList sel = m_interestAccountEdit->selectedAccounts();
    if(sel.count() != 1)
      throw new MYMONEYEXCEPTION("Need a single selected interest category");
    MyMoneyAccount acc = MyMoneyFile::instance()->account(sel.first());
    m_summaryInterestCategory->setText(acc.name());
  } catch(MyMoneyException *e) {
    qWarning("Unable to determine interest category for loan account creation");
    delete e;
  }
  m_summaryAdditionalFees->setText(m_additionalCost->text());
  m_summaryTotalPeriodicPayment->setText(m_periodicPayment->text());
  m_summaryNextPayment->setText(KGlobal::locale()->formatDate(m_nextDueDateEdit->getQDate(), true));

  try {
    QCStringList sel = m_paymentAccountEdit->selectedAccounts();
    if(sel.count() != 1)
      throw new MYMONEYEXCEPTION("Need a single selected payment account");
    MyMoneyAccount acc = MyMoneyFile::instance()->account(sel.first());
    m_summaryPaymentAccount->setText(acc.name());
  } catch(MyMoneyException *e) {
    qWarning("Unable to determine payment account for loan account creation");
    delete e;
  }
}

void KNewLoanWizard::next()
{
  bool dontLeavePage = false;
  QString errMsg = i18n(
              "The loan wizard is unable to calculate two different values for your loan "
              "at the same time. "
              "Please enter a value for the %1 on this page or backup to the page where the "
              " current value to be calculated is defined and fill in a value.");

  if(currentPage() == m_lendBorrowPage) {
    // load the appropriate categories into the list
    loadAccountList();
    m_nameEdit->setFocus();

  } else if(currentPage() == m_interestTypePage) {
    if(m_fixedInterestButton->isChecked()) {
      setAppropriate(m_previousPaymentsPage, true);
      if(m_previousPaymentButton->isChecked())
        setAppropriate(m_recordPaymentPage, true);
      else
        setAppropriate(m_recordPaymentPage, false);
      setAppropriate(m_variableInterestDatePage, false);

    } else {
      setAppropriate(m_previousPaymentsPage, false);
      setAppropriate(m_recordPaymentPage, false);
      setAppropriate(m_variableInterestDatePage, true);
    }

  } else if(currentPage() == m_loanAmountPage) {
    m_interestRateEdit->setFocus();
    if(m_thisYearPaymentButton->isChecked()
    && m_loanAmountEdit->text().isEmpty()) {
      errMsg = i18n("You selected, that payments have already been made towards this loan. "
                    "This requires you to enter the loan amount exactly as found on your "
                    "last statement.");
      dontLeavePage = true;
      KMessageBox::error(0, errMsg, i18n("Calculation error"));
    } else
      updateLoanAmount();

  } else if(currentPage() == m_interestPage) {

    if(m_loanAmountEdit->text().isEmpty()
    && m_interestRateEdit->text().isEmpty()) {
      dontLeavePage = true;
      KMessageBox::error(0, errMsg.arg(i18n("interest rate")), i18n("Calculation error"));
    } else
      updateInterestRate();

  } else if(currentPage() == m_durationPage) {
    if((m_loanAmountEdit->text().isEmpty()
    || m_interestRateEdit->text().isEmpty())
    && m_durationValueEdit->value() == 0) {
      dontLeavePage = true;
      KMessageBox::error(0, errMsg.arg(i18n("term")), i18n("Calculation error"));
    } else
      updateDuration();

  } else if(currentPage() == m_paymentPage) {
    if((m_loanAmountEdit->text().isEmpty()
    || m_interestRateEdit->text().isEmpty()
    || m_durationValueEdit->value() == 0)
    && m_paymentEdit->text().isEmpty()) {
      dontLeavePage = true;
      KMessageBox::error(0, errMsg.arg(i18n("principal and interest")), i18n("Calculation error"));
    } else
      updatePayment();

  } else if(currentPage() == m_finalPaymentPage) {
    if((m_loanAmountEdit->text().isEmpty()
    || m_interestRateEdit->text().isEmpty()
    || m_durationValueEdit->value() == 0
    || m_paymentEdit->text().isEmpty())
    && m_finalPaymentEdit->text().isEmpty()) {
      // if two fields are empty and one of them is the final payment
      // we assume the final payment to be 0 instead of presenting a
      m_finalPaymentEdit->setValue(MyMoneyMoney(0, 1));
    }
    updateFinalPayment();
    if(!calculateLoan()) {
      dontLeavePage = true;
    } else
      updateLoanInfo();

  } else if(currentPage() == m_additionalFeesPage) {
    m_nextDueDateEdit->setEnabled(true);
    if(m_allPaymentsButton->isChecked() || m_noPreviousPaymentButton->isChecked()) {
      m_nextDueDateEdit->setDate(m_firstDueDateEdit->getQDate());
      m_nextDueDateEdit->setEnabled(false);
      setAppropriate(m_assetAccountPage, true);
    } else {
      QDate nextPayment(QDate::currentDate().year(), 1, m_firstDueDateEdit->getQDate().day());
      m_nextDueDateEdit->setDate(nextPayment);
      setAppropriate(m_assetAccountPage, false);
      m_assetAccountEdit->slotDeselectAllAccounts();
    }
    if(m_nextDueDateEdit->getQDate() < m_firstDueDateEdit->getQDate()) {
      m_nextDueDateEdit->setDate(m_firstDueDateEdit->getQDate());
    }

  } else if(currentPage() == m_schedulePage) {
    updateSummary();
  }

/*
  switch(m_accountType) {
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Asset:
      if(indexOf(accountPaymentPage) != -1) {
        removePage(accountPaymentPage);
      }
      setAppropriate(accountNumberPage, false);
      setFinishEnabled(accountDetailsPage, true);
      break;

    case MyMoneyAccount::CreditCard:
      if(indexOf(accountPaymentPage) == -1) {
        loadPaymentMethods();
        addPage(accountPaymentPage, m_accountPaymentPageTitle);
      }
      setAppropriate(accountPaymentPage, true);
      setFinishEnabled(accountPaymentPage, true);
      setFinishEnabled(accountDetailsPage, false);
      break;

    default:
      setAppropriate(accountNumberPage, institutionComboBox->currentText() != "");
      if(indexOf(accountPaymentPage) != -1) {
        removePage(accountPaymentPage);
      }
      setFinishEnabled(accountDetailsPage, true);
      break;
  }
*/
  if(!dontLeavePage)
    KNewLoanWizardDecl::next();

  // setup the availability of widgets on the selected page
  slotCheckPageFinished();
}

void KNewLoanWizard::loadComboBoxes(void)
{
  m_interestFrequencyUnitEdit->insertItem(i18n("Days"));
  m_interestFrequencyUnitEdit->insertItem(i18n("Weeks"));
  m_interestFrequencyUnitEdit->insertItem(i18n("Months"));
  m_interestFrequencyUnitEdit->insertItem(i18n("Years"));

  m_paymentFrequencyUnitEdit->insertItem(KMyMoneyUtils::occurenceToString(MyMoneySchedule::OCCUR_DAILY));
  m_paymentFrequencyUnitEdit->insertItem(KMyMoneyUtils::occurenceToString(MyMoneySchedule::OCCUR_WEEKLY));
  m_paymentFrequencyUnitEdit->insertItem(KMyMoneyUtils::occurenceToString(MyMoneySchedule::OCCUR_FORTNIGHTLY));
  m_paymentFrequencyUnitEdit->insertItem(KMyMoneyUtils::occurenceToString(MyMoneySchedule::OCCUR_EVERYOTHERWEEK));
  m_paymentFrequencyUnitEdit->insertItem(KMyMoneyUtils::occurenceToString(MyMoneySchedule::OCCUR_EVERYFOURWEEKS));
  m_paymentFrequencyUnitEdit->insertItem(KMyMoneyUtils::occurenceToString(MyMoneySchedule::OCCUR_MONTHLY));
  m_paymentFrequencyUnitEdit->insertItem(KMyMoneyUtils::occurenceToString(MyMoneySchedule::OCCUR_EVERYOTHERMONTH));
  m_paymentFrequencyUnitEdit->insertItem(KMyMoneyUtils::occurenceToString(MyMoneySchedule::OCCUR_QUARTERLY));
  m_paymentFrequencyUnitEdit->insertItem(KMyMoneyUtils::occurenceToString(MyMoneySchedule::OCCUR_EVERYFOURMONTHS));
  m_paymentFrequencyUnitEdit->insertItem(KMyMoneyUtils::occurenceToString(MyMoneySchedule::OCCUR_TWICEYEARLY));
  m_paymentFrequencyUnitEdit->insertItem(KMyMoneyUtils::occurenceToString(MyMoneySchedule::OCCUR_YEARLY));

  m_durationUnitEdit->insertItem(i18n("Months"));
  m_durationUnitEdit->insertItem(i18n("Years"));
  m_durationUnitEdit->insertItem(i18n("Payments"));

}

void KNewLoanWizard::slotInterestOnPayment(void)
{
  m_interestOnPaymentButton->setChecked(true);
  m_interestOnReceptionButton->setChecked(false);
}

void KNewLoanWizard::slotInterestOnReception(void)
{
  m_interestOnPaymentButton->setChecked(false);
  m_interestOnReceptionButton->setChecked(true);
}

int KNewLoanWizard::occurenceToPeriod(const MyMoneySchedule::occurenceE occurence) const
{
  int rc = 0;

  switch(occurence) {
    case MyMoneySchedule::OCCUR_DAILY:
      rc = 1;
      break;
    case MyMoneySchedule::OCCUR_WEEKLY:
      rc = 7;
      break;
    case MyMoneySchedule::OCCUR_FORTNIGHTLY:
      rc = 14;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERWEEK:
      rc = 15;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURWEEKS:
      rc = 28;
      break;
    case MyMoneySchedule::OCCUR_MONTHLY:
      rc = 30;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERMONTH:
      rc = 60;
      break;
    case MyMoneySchedule::OCCUR_QUARTERLY:
      rc = 90;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURMONTHS:
      rc = 120;
      break;
    case MyMoneySchedule::OCCUR_TWICEYEARLY:
      rc = 180;
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
      rc = 360;
      break;
    default:
      qWarning("Occurence not supported by financial calculator");
  }

  return rc;
}
/*
int KNewLoanWizard::occurenceToFrequency(const MyMoneySchedule::occurenceE occurence) const
{
  int rc = 0;

  switch(occurence) {
    case MyMoneySchedule::OCCUR_DAILY:
      rc = 365;
      break;
    case MyMoneySchedule::OCCUR_WEEKLY:
      rc = 52;
      break;
    case MyMoneySchedule::OCCUR_FORTNIGHTLY:
      rc = 24;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERWEEK:
      rc = 26;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURWEEKS:
      rc = 13;
      break;
    case MyMoneySchedule::OCCUR_MONTHLY:
      rc = 12;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERMONTH:
      rc = 6;
      break;
    case MyMoneySchedule::OCCUR_QUARTERLY:
      rc = 4;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURMONTHS:
      rc = 3;
      break;
    case MyMoneySchedule::OCCUR_TWICEYEARLY:
      rc = 2;
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
      rc = 1;
      break;
    default:
      qWarning("Occurence not supported by financial calculator");
  }

  return rc;
}
*/
int KNewLoanWizard::calculateLoan(void)
{
  MyMoneyFinancialCalculator calc;
  long double val;
  int PF;
  QString result;

  // FIXME: for now, we only support interest calculation at the end of the period
  calc.setBep();
  // FIXME: for now, we only support periodic compounding
  calc.setDisc();

  PF = KMyMoneyUtils::occurenceToFrequency(KMyMoneyUtils::stringToOccurence(
                            m_paymentFrequencyUnitEdit->currentText()));
  if(PF == 0)
    return 0;
  calc.setPF(PF);

  // FIXME: for now we only support compounding frequency == payment frequency
  calc.setCF(PF);


  if(!m_loanAmountEdit->text().isEmpty()) {
    val = static_cast<long double> (m_loanAmountEdit->value().abs().toDouble());
    if(m_borrowButton->isChecked())
      val = -val;
    calc.setPv(val);
  }

  if(!m_interestRateEdit->text().isEmpty()) {
    val = static_cast<long double> (m_interestRateEdit->value().abs().toDouble());
    calc.setIr(val);
  }

  if(!m_paymentEdit->text().isEmpty()) {
    val = static_cast<long double> (m_paymentEdit->value().abs().toDouble());
    if(m_lendButton->isChecked())
      val = -val;
    calc.setPmt(val);
  }

  if(!m_finalPaymentEdit->text().isEmpty()) {
    val = static_cast<long double> (m_finalPaymentEdit->value().abs().toDouble());
    if(m_lendButton->isChecked())
      val = -val;
    calc.setFv(val);
  }

  if(m_durationValueEdit->value() != 0) {
    calc.setNpp(static_cast<long double>(term()));
  }

  // setup of parameters is done, now do the calculation
  try {
    if(m_loanAmountEdit->text().isEmpty()) {
      // calculate the amount of the loan out of the other information
      val = calc.presentValue();
      m_loanAmountEdit->loadText(MyMoneyMoney(static_cast<double>(val)).abs().formatMoney());
      result = i18n("KMyMoney has calculated the amount of the loan as %1.")
                        .arg(m_loanAmountEdit->text());

    } else if(m_interestRateEdit->text().isEmpty()) {
      // calculate the interest rate out of the other information
      val = calc.interestRate();
      m_interestRateEdit->loadText(MyMoneyMoney(static_cast<double>(val)).abs().formatMoney("", 3));
      result = i18n("KMyMoney has calculated the interest rate to %1%.")
                        .arg(m_interestRateEdit->text());

    } else if(m_paymentEdit->text().isEmpty()) {
      // calculate the periodical amount of the payment out of the other information
      val = calc.payment();
      m_paymentEdit->setValue(MyMoneyMoney(static_cast<double>(val)).abs());
      // reset payment as it might have changed due to rounding
      val = static_cast<long double> (m_paymentEdit->value().abs().toDouble());
      if(m_lendButton->isChecked())
        val = -val;
      calc.setPmt(val);

      result = i18n("KMyMoney has calculated a periodic payment of %1 to cover principal and interest.")
                      .arg(m_paymentEdit->text());

      val = calc.futureValue();
      if((m_borrowButton->isChecked() && val < 0 && fabsl(val) >= fabsl(calc.payment()))
      || (m_lendButton->isChecked() && val > 0 && fabs(val) >= fabs(calc.payment()))) {
        calc.setNpp(calc.npp()-1);
        updateTermWidgets(calc.npp());
        val = calc.futureValue();
        MyMoneyMoney refVal(static_cast<double>(val));
        m_finalPaymentEdit->loadText(refVal.abs().formatMoney());
        result += QString(" ");
        result += i18n("The number of payments has been decremented and the final payment has been modified to %1.")
                      .arg(m_finalPaymentEdit->text());
      } else if((m_borrowButton->isChecked() && val < 0 && fabsl(val) < fabsl(calc.payment()))
             || (m_lendButton->isChecked() && val > 0 && fabs(val) < fabs(calc.payment()))) {
        m_finalPaymentEdit->loadText(MyMoneyMoney(0,1).formatMoney());
      } else {
        MyMoneyMoney refVal(static_cast<double>(val));
        m_finalPaymentEdit->loadText(refVal.abs().formatMoney());
        result += i18n("The final payment has been modified to %1.")
                      .arg(m_finalPaymentEdit->text());
      }

    } else if(m_durationValueEdit->value() == 0) {
      // calculate the number of payments out of the other information
      val = calc.numPayments();
      if(val == 0)
        throw new MYMONEYEXCEPTION("incorrect fincancial calculation");

      // if the number of payments has a fractional part, then we
      // round it to the smallest integer and calculate the balloon payment
      result = i18n("KMyMoney has calculated the term of your loan as %1. ")
                        .arg(updateTermWidgets(floorl(val)));

      if(val != floorl(val)) {
        calc.setNpp(floorl(val));
        val = calc.futureValue();
        MyMoneyMoney refVal(static_cast<double>(val));
        m_finalPaymentEdit->loadText(refVal.abs().formatMoney());
        result += i18n("The final payment has been modified to %1.")
                      .arg(m_finalPaymentEdit->text());
      }

    } else {
      // calculate the future value of the loan out of the other information
      val = calc.futureValue();

      // we differentiate between the following cases:
      // a) the future value is greater than a payment
      // b) the future value is less than a payment or the loan is overpaid
      // c) all other cases
      //
      // a) means, we have paid more than we owed. This can't be
      // b) means, we paid more than we owed but the last payment is
      //    less in value than regular payments. That means, that the
      //    future value is to be treated as  (fully payed back)
      // c) the loan is not payed back yet
      if((m_borrowButton->isChecked() && val < 0 && fabsl(val) > fabsl(calc.payment()))
      || (m_lendButton->isChecked() && val > 0 && fabs(val) > fabs(calc.payment()))) {
        // case a)
        qDebug("Future Value is %Lf", val);
        throw new MYMONEYEXCEPTION("incorrect fincancial calculation");

      } else if((m_borrowButton->isChecked() && val < 0 && fabsl(val) <= fabsl(calc.payment()))
             || (m_lendButton->isChecked() && val > 0 && fabs(val) <= fabs(calc.payment()))) {
        // case b)
        val = 0;
      }

      MyMoneyMoney refVal(static_cast<double>(val));
      result = i18n("KMyMoney has calculated a final payment of %1 for this loan.")
                        .arg(refVal.abs().formatMoney());

      if(!m_finalPaymentEdit->text().isEmpty()) {
        if((m_finalPaymentEdit->value().abs() - refVal.abs()).abs().toDouble() > 1) {
          throw new MYMONEYEXCEPTION("incorrect fincancial calculation");
        }
        result = i18n("KMyMoney has successfully verified your loan information.");
      }
      m_finalPaymentEdit->loadText(refVal.abs().formatMoney());
    }

  } catch (MyMoneyException *e) {
    delete e;
    KMessageBox::error(0,
      i18n("You have entered mis-matching information. Please backup to the "
           "appropriate page and update your figures or leave one value empty "
           "to let KMyMoney calculate it for you"),
      i18n("Calculation error"));
    return 0;
  }

  result += i18n("\n\nAccept this or modify the loan information and recalculate.");

  KMessageBox::information(0, result, i18n("Calculation successful"));
  return 1;
}

const QString KNewLoanWizard::updateTermWidgets(const long double val)
{
  long long vl = static_cast<long long>(floorl(val));

  QString valString;
  MyMoneySchedule::occurenceE unit;
  unit = KMyMoneyUtils::stringToOccurence(m_paymentFrequencyUnitEdit->currentText());

  if((unit == MyMoneySchedule::OCCUR_MONTHLY)
  && ((vl % 12) == 0)) {
    vl /= 12;
    unit = MyMoneySchedule::OCCUR_YEARLY;
  }

  switch(unit) {
    case MyMoneySchedule::OCCUR_MONTHLY:
      valString = i18n("one month", "%n months", vl);
      m_durationUnitEdit->setCurrentItem(i18n("Months"));
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
      valString = i18n("one year", "%n years", vl);
      m_durationUnitEdit->setCurrentItem(i18n("Years"));
      break;
    default:
      valString = i18n("one payment", "%n payments", vl);
      m_durationUnitEdit->setCurrentItem(i18n("Payments"));
      break;
  }
  m_durationValueEdit->setValue(vl);
  return valString;
}

void KNewLoanWizard::slotCreateCategory(void)
{
  MyMoneyAccount acc, base;
  MyMoneyFile* file = MyMoneyFile::instance();

  if(m_borrowButton->isChecked()) {
    base = file->expense();
    acc.setAccountType(MyMoneyAccount::Expense);
  } else {
    base = file->income();
    acc.setAccountType(MyMoneyAccount::Income);
  }
  acc.setParentAccountId(base.id());

  KNewAccountDlg* dlg = new KNewAccountDlg(acc, true, true);
  if(dlg->exec() == QDialog::Accepted) {
    acc = dlg->account();

    try {
      QCString id;
      id = file->createCategory(base, acc.name());
      if(id.isEmpty())
        throw new MYMONEYEXCEPTION("failure while creating the account hierarchy");

      loadAccountList();
      m_interestAccountEdit->setSelected(id);

    } catch (MyMoneyException *e) {
      QString message("Unable to add account: ");
      message += e->what();
      KMessageBox::information(this, message);
      delete e;
    }
  }
  delete dlg;
}

void KNewLoanWizard::loadAccountList(void)
{
  if(m_borrowButton->isChecked()) {
    m_interestAccountEdit->loadList(KMyMoneyUtils::expense);
  } else {
    m_interestAccountEdit->loadList(KMyMoneyUtils::income);
  }

  QValueList<int> typeList;
  typeList << MyMoneyAccount::Checkings;
  typeList << MyMoneyAccount::Savings;
  typeList << MyMoneyAccount::Cash;
  // typeList << MyMoneyAccount::AssetLoan;
  // typeList << MyMoneyAccount::CertificateDep;
  // typeList << MyMoneyAccount::Investment;
  // typeList << MyMoneyAccount::MoneyMarket;
  typeList << MyMoneyAccount::Asset;
  typeList << MyMoneyAccount::Currency;

  m_assetAccountEdit->loadList(typeList);

  typeList << MyMoneyAccount::CreditCard;
  // typeList << MyMoneyAccount::Loan;
  typeList << MyMoneyAccount::Liability;
  // typeList << MyMoneyAccount::Income;
  // typeList << MyMoneyAccount::Expense;
  m_paymentAccountEdit->loadList(typeList);
}

void KNewLoanWizard::slotAdditionalFees(void)
{
  // KMessageBox::information(0, QString("Not yet implemented ... if you want to help, contact kmymoney2-developer@lists.sourceforge.net"), QString("Development notice"));
  MyMoneyAccount account(QCString("Phony-ID"), MyMoneyAccount());

  KSplitTransactionDlg* dlg = new KSplitTransactionDlg(m_transaction, account, false, !m_borrowButton->isChecked(), MyMoneyMoney(0));
  connect(dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));

  if(dlg->exec() == QDialog::Accepted) {
    m_transaction = dlg->transaction();
    // sum up the additional fees
    QValueList<MyMoneySplit>::ConstIterator it;

    MyMoneyMoney fees;
    for(it = m_transaction.splits().begin(); it != m_transaction.splits().end(); ++it) {
      if((*it).accountId() != account.id()) {
        fees += (*it).value();
      }
    }
    m_additionalCost->setText(fees.abs().formatMoney());
  }

  delete dlg;
  updatePeriodicPayment();
}

const MyMoneyTransaction KNewLoanWizard::transaction() const
{
  MyMoneyTransaction t;

  MyMoneySplit sPayment, sInterest, sAmortization;
  // setup accounts. at this point, we cannot fill in the id of the
  // account that the amortization will be performed on, because we
  // create the is account. So the id is yet unknown.
  sPayment.setAccountId(m_paymentAccountEdit->selectedAccounts().first());
  sInterest.setAccountId(m_interestAccountEdit->selectedAccounts().first());

  // values
  if(m_borrowButton->isChecked()) {
    sPayment.setValue(-m_paymentEdit->value());
  } else {
    sPayment.setValue(m_paymentEdit->value());
  }
  sInterest.setValue(MyMoneyMoney::autoCalc);
  sAmortization.setValue(MyMoneyMoney::autoCalc);

  // actions
  sPayment.setAction(MyMoneySplit::ActionAmortization);
  sAmortization.setAction(MyMoneySplit::ActionAmortization);
  sInterest.setAction(MyMoneySplit::ActionInterest);

  // payee
  if(!m_payeeEdit->text().isEmpty()) {
    try {
      QCString payeeId = MyMoneyFile::instance()->payeeByName(m_payeeEdit->text()).id();
      sPayment.setPayeeId(payeeId);
      sAmortization.setPayeeId(payeeId);

    } catch(MyMoneyException *e) {
      delete e;
    }
  }

  // IMPORTANT: Payment split must be the first one, because
  //            the schedule view expects it this way during display
  t.addSplit(sPayment);
  t.addSplit(sAmortization);
  t.addSplit(sInterest);

  // copy the splits from the other costs and update the payment split
  MyMoneyAccount account(QCString("Phony-ID"), MyMoneyAccount());
  QValueList<MyMoneySplit>::ConstIterator it;
  for(it = m_transaction.splits().begin(); it != m_transaction.splits().end(); ++it) {
    if((*it).accountId() != account.id()) {
      MyMoneySplit sp = (*it);
      sp.setId(QCString());
      t.addSplit(sp);
      sPayment.setValue(sPayment.value()-sp.value());
      t.modifySplit(sPayment);
    }
  }
  return t;
}

const MyMoneySchedule KNewLoanWizard::schedule() const
{
  MyMoneySchedule sched(m_nameEdit->text(),
                        MyMoneySchedule::TYPE_LOANPAYMENT,
                        KMyMoneyUtils::stringToOccurence(m_paymentFrequencyUnitEdit->currentText()),
                        MyMoneySchedule::STYPE_OTHER,
                        m_nextDueDateEdit->getQDate(),
                        QDate(),
                        false,
                        false);

  sched.setTransaction(transaction());

  return sched;
}

const MyMoneyAccount KNewLoanWizard::account(void) const
{
  MyMoneyAccountLoan acc;

  acc.setName(m_nameEdit->text());
  acc.setOpeningDate(m_firstDueDateEdit->getQDate());
  if(m_allPaymentsButton->isChecked()) {
    acc.setOpeningBalance(0);
  } else {
    acc.setOpeningBalance(m_loanAmountEdit->value());
  }
  if(m_borrowButton->isChecked()) {
    acc.setAccountType(MyMoneyAccount::Loan);
    acc.setOpeningBalance(-acc.openingBalance());
  } else {
    acc.setAccountType(MyMoneyAccount::AssetLoan);
  }
  acc.setLoanAmount(m_loanAmountEdit->value());
  acc.setInterestRate(m_firstDueDateEdit->getQDate(), m_interestRateEdit->value());
  if(m_interestOnReceptionButton->isChecked())
    acc.setInterestCalculation(MyMoneyAccountLoan::paymentReceived);
  else
    acc.setInterestCalculation(MyMoneyAccountLoan::paymentDue);

  acc.setFixedInterestRate(m_fixedInterestButton->isChecked());
  acc.setFinalPayment(MyMoneyMoney(m_finalPaymentEdit->text()));
  acc.setTerm(term());
  acc.setPeriodicPayment(m_paymentEdit->value());

  if(!m_payeeEdit->text().isEmpty()) {
    try {
      acc.setPayee(MyMoneyFile::instance()->payeeByName(m_payeeEdit->text()).id());
    } catch(MyMoneyException *e) {
      delete e;
      qWarning("%s(%d): Someone has removed the selected payee", __FILE__, __LINE__);
      acc.setPayee(QCString());
    }
  } else
    acc.setPayee(QCString());

  if(m_variableInterestButton->isChecked()) {
    acc.setNextInterestChange(m_interestChangeDateEdit->getQDate());
    acc.setInterestChangeFrequency(m_interestFrequencyAmountEdit->value(),
                                   m_interestFrequencyUnitEdit->currentItem());
  }
  return acc;
}

void KNewLoanWizard::loadWidgets(const MyMoneyAccount& account)
{
  bool borrowing = false;
  MyMoneyAccountLoan acc(account);

  m_nameEdit->loadText(acc.name());
  if(acc.accountType() == MyMoneyAccount::Loan) {
    m_borrowButton->animateClick();
    borrowing = true;
  } else
    m_lendButton->animateClick();

  if(!acc.openingBalance().isZero()) {
    if(borrowing)
      m_loanAmountEdit->loadText((-account.openingBalance()).formatMoney());
    else
      m_loanAmountEdit->loadText(account.openingBalance().formatMoney());
  }


  if(account.openingDate().isValid())
    m_firstDueDateEdit->setDate(account.openingDate());
}

int KNewLoanWizard::term(void) const
{
  int factor = 0;

  if(m_durationValueEdit->value() != 0) {
    factor = 1;
    switch(m_durationUnitEdit->currentItem()) {
      case 1: // years
        factor = 12;
        // tricky fall through here

      case 0: // months
        factor *= 30;
        factor *= m_durationValueEdit->value();
        // factor now is the duration in days. we divide this by the
        // payment frequency and get the number of payments
        factor /= occurenceToPeriod(KMyMoneyUtils::stringToOccurence(
                            m_paymentFrequencyUnitEdit->currentText()));;
        break;

      case 2: // payments
        factor = m_durationValueEdit->value();
        break;
    }
  }
  return factor;
}

const QCString KNewLoanWizard::initialPaymentAccount(void) const
{
  if(m_dontCreatePayoutCheckBox->isChecked()) {
    return QCString();
  }
  return m_assetAccountEdit->selectedAccounts().first();
}

const QDate KNewLoanWizard::initialPaymentDate(void) const
{
  if(m_dontCreatePayoutCheckBox->isChecked()) {
    return QDate();
  }
  return m_paymentDate->getQDate();
}

#include "knewloanwizard.moc"
