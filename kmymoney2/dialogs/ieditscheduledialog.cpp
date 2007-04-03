/***************************************************************************
                          keditscheduledialog.cpp  -  description
                             -------------------
    begin                : Tue Jul 22 2003
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qgroupbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kstdguiitem.h>
#include <kmessagebox.h>
#include <knumvalidator.h>
#include <klocale.h>
#include <kapplication.h>
#include <ktextedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneyaccountcombo.h>
#include <kmymoney/mymoneyinstitution.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneypayee.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneycategory.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/mymoneyobjectcontainer.h>
#include <kmymoney/kmymoneycombo.h>

#include "../widgets/kmymoneyaccountcompletion.h"
#include "../dialogs/ksplittransactiondlg.h"
#include "ieditscheduledialog.h"

/*
void dumpSplit (int i, MyMoneySplit s) {
  qDebug ("Split %d", i);
  qDebug ("Acct: %s", s.accountId().data());
  qDebug ("Payee: %s", s.payeeId().data());
  qDebug ("Action = %s", s.action().data());
  qDebug ("Value: %s", s.value().toString().latin1());
} */

// list of schedule occurrences in ascending order of period
#define END_OCCURS 99999
MyMoneySchedule::occurenceE KEditScheduleDialog::occurMasks[] = {
  MyMoneySchedule::OCCUR_ONCE,
  MyMoneySchedule::OCCUR_DAILY,
  MyMoneySchedule::OCCUR_WEEKLY,
  MyMoneySchedule::OCCUR_FORTNIGHTLY,
  MyMoneySchedule::OCCUR_EVERYOTHERWEEK,
  MyMoneySchedule::OCCUR_MONTHLY,
  MyMoneySchedule::OCCUR_EVERYFOURWEEKS,
  MyMoneySchedule::OCCUR_EVERYOTHERMONTH,
  MyMoneySchedule::OCCUR_EVERYTHREEMONTHS,
  MyMoneySchedule::OCCUR_QUARTERLY,
  MyMoneySchedule::OCCUR_EVERYFOURMONTHS,
  MyMoneySchedule::OCCUR_TWICEYEARLY,
  MyMoneySchedule::OCCUR_YEARLY,
  MyMoneySchedule::OCCUR_EVERYOTHERYEAR,
  (MyMoneySchedule::occurenceE) END_OCCURS };

KEditScheduleDialog::KEditScheduleDialog(const QCString& action, const MyMoneySchedule& schedule, QWidget *parent, const char *name) :
  kEditScheduledTransferDlgDecl(parent, name, true),
  m_cancelSave(false)
{
  // Since we cannot create a category widget with the split button in designer,
  // we recreate the category widget here and replace it in the layout
  m_paymentGroupBoxLayout->remove(m_category);
  delete m_category;
  m_category = new KMyMoneyCategory(m_paymentGroupBox, "m_category", true);

  // make sure to drop the surrounding frame into the layout
  m_paymentGroupBoxLayout->addMultiCellWidget( m_category->parentWidget(), 4, 4, 1, 3 );

  // Set the icon sets for the buttons
  m_qbuttonOK->setGuiItem(KStdGuiItem::ok());
  m_qbuttonCancel->setGuiItem(KStdGuiItem::cancel());
  m_helpButton->setGuiItem(KStdGuiItem::help());

  m_requiredFields = new kMandatoryFieldGroup (this);
  m_requiredFields->setOkButton(m_qbuttonOK); // button to be enabled when all fields present

  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_category->splitButton(), SIGNAL(clicked()), this, SLOT(slotEditSplits()));
  connect(m_qcheckboxEnd, SIGNAL(toggled(bool)), this, SLOT(slotWillEndToggled(bool)));
  connect(m_qbuttonOK, SIGNAL(clicked()), this, SLOT(okClicked()));
  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));

  connect(m_qlineeditRemaining, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotRemainingChanged(const QString&)));
  connect(m_kdateinputFinal, SIGNAL(dateChanged(const QDate&)),
    this, SLOT(slotEndDateChanged(const QDate&)));
  connect(m_kmoneyeditAmount, SIGNAL(valueChanged(const QString&)),
    this, SLOT(slotAmountChanged(const QString&)));
  connect(m_accountCombo, SIGNAL(accountSelected(const QCString&)),
    this, SLOT(slotAccountChanged(const QCString&)));
  connect(m_scheduleName, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotScheduleNameChanged(const QString&)));
  connect(m_kcomboTo, SIGNAL(accountSelected(const QCString&)),
    this, SLOT(slotToChanged(const QCString&)));
  connect(m_kcomboMethod, SIGNAL(activated(int)),
    this, SLOT(slotMethodChanged(int)));
  connect(m_payee, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotPayeeChanged(const QString&)));
  connect(m_kdateinputDue, SIGNAL(dateChanged(const QDate&)),
    this, SLOT(slotDateChanged(const QDate&)));
  connect(m_kcomboFreq, SIGNAL(activated(int)),
    this, SLOT(slotFrequencyChanged(int)));
  connect(m_qcheckboxEstimate, SIGNAL(clicked()),
    this, SLOT(slotEstimateChanged()));
  // connect(m_category, SIGNAL(textChanged(const QString&)), this, SLOT(slotCategoryChanged(const QString&)));
  connect(m_category, SIGNAL(itemSelected(const QCString&)),
    this, SLOT(slotCategoryChanged(const QCString& )));
  connect(m_qcheckboxAuto, SIGNAL(clicked()),
    this, SLOT(slotAutoEnterChanged()));
  connect(m_qlineeditMemo, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotMemoChanged(const QString&)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotReloadEditWidgets()));

  connect(m_payee, SIGNAL(createItem(const QString&, QCString&)), this, SIGNAL(createPayee(const QString&, QCString&)));
  connect(m_category, SIGNAL(createItem(const QString&, QCString&)), this, SIGNAL(createCategory(const QString&, QCString&)));

  m_actionType = action;
  m_schedule = schedule;
  m_transaction = schedule.transaction();
  createSecondSplit();

  // override the the action type if we have a scheduled transaction
//  if(m_transaction.splitCount() > 0) {
//    m_actionType = m_transaction.splits()[0].action();
//  }

  KIntValidator *validator = new KIntValidator(1, 32768, this);
  m_qlineeditRemaining->setValidator(validator);

  reloadFromFile();
  loadWidgetsFromSchedule();

  if (m_actionType == MyMoneySplit::ActionTransfer)
  {
    m_accountCombo->setEnabled(true);
    m_qlabelPay2->setEnabled(true);
    TextLabel1_3->setEnabled(true);
    m_kcomboTo->setEnabled(true);
    m_category->setEnabled(false);

    m_requiredFields->add(m_accountCombo);
    m_requiredFields->add(m_kcomboTo);
    m_requiredFields->add(m_payee);

    setCaption(i18n("Edit Transfer Schedule"));
  }
  else if (m_actionType == MyMoneySplit::ActionAmortization)
  {
    m_accountCombo->setEnabled(true);
    m_qlabelPay2->setEnabled(true);
    TextLabel1_3->setEnabled(false);
    m_kcomboTo->setEnabled(false);
    TextLabel5->setEnabled(false);
    m_kmoneyeditAmount->setEnabled(false);
    TextLabel6->setEnabled(false);
    m_kdateinputDue->setEnabled(false);
    TextLabel7->setEnabled(false);
    m_kcomboFreq->setEnabled(false);
    m_category->setEnabled(false);
    m_qcheckboxEnd->setEnabled(false);

    m_requiredFields->add(m_accountCombo);
    m_requiredFields->add(m_payee);

    setCaption(i18n("Edit Loan Payment Schedule"));
  }
  else if (m_actionType == MyMoneySplit::ActionDeposit)
  {
    m_accountCombo->setEnabled(false);
    m_qlabelPay2->setEnabled(false);
    TextLabel1_3->setEnabled(true);
    m_kcomboTo->setEnabled(true);

    m_requiredFields->add(m_kcomboTo);
    m_requiredFields->add(m_category);
    m_requiredFields->add(m_payee);

    setCaption(i18n("Edit Deposit Schedule"));
  }
  else
  {
    m_accountCombo->setEnabled(true);
    m_qlabelPay2->setEnabled(true);
    TextLabel1_3->setEnabled(false);
    m_kcomboTo->setEnabled(false);
    setCaption(i18n("Edit Bill Schedule"));

    m_requiredFields->add(m_accountCombo);
    m_requiredFields->add(m_payee);
    m_requiredFields->add(m_category);
  }

  m_requiredFields->add(m_scheduleName);
  m_requiredFields->add(m_kmoneyeditAmount->lineedit());
}

KEditScheduleDialog::~KEditScheduleDialog()
{
  delete m_requiredFields;
}

void KEditScheduleDialog::slotReloadEditWidgets(void)
{
  QCString categoryId = m_category->selectedItem();

  MyMoneyObjectContainer objects;
  AccountSet aSet(&objects);
  aSet.addAccountGroup(MyMoneyAccount::Income);
  aSet.addAccountGroup(MyMoneyAccount::Expense);
  aSet.load(m_category->selector());

  // reload payee widget
  QCString payeeId = m_payee->selectedItem();

  m_payee->loadPayees(MyMoneyFile::instance()->payeeList());

  if(!payeeId.isEmpty()) {
    m_payee->setSelectedItem(payeeId);
  }
}

void KEditScheduleDialog::reloadFromFile(void)
{
  if (m_actionType == MyMoneySplit::ActionTransfer
  ||  m_actionType == MyMoneySplit::ActionAmortization)
  {
    m_kcomboMethod->insertItem(i18n("Direct Debit"));
    m_kcomboMethod->insertItem(i18n("Direct Deposit"));
    m_kcomboMethod->insertItem(i18n("Manual Deposit"));
    m_kcomboMethod->insertItem(i18n("Write Check"));
    m_kcomboMethod->insertItem(i18n("Other"));

    if (m_actionType == MyMoneySplit::ActionAmortization)
    {
      m_paymentMethod->hide();
    }
    else
    {
      m_paymentMethod->insertItem("Transfer");
    }
  }
  else if (m_actionType == MyMoneySplit::ActionDeposit)
  {
    m_kcomboMethod->insertItem(i18n("Direct Deposit"));
    m_kcomboMethod->insertItem(i18n("Manual Deposit"));
    m_kcomboMethod->insertItem(i18n("Other"));

    m_paymentMethod->insertItem(i18n("Check"));
    m_paymentMethod->insertItem(i18n("Deposit"));
    m_paymentMethod->insertItem(i18n("Transfer"));
  }
  else // Withdrawal
  {
    m_kcomboMethod->insertItem(i18n("Direct Debit"));
    m_kcomboMethod->insertItem(i18n("Write Check"));
    m_kcomboMethod->insertItem(i18n("Other"));

    m_paymentMethod->insertItem(i18n("Check"));
    m_paymentMethod->insertItem(i18n("Transfer"));
    m_paymentMethod->insertItem(i18n("Withdrawal"));
    m_paymentMethod->insertItem(i18n("ATM"));
  }
  for (int i = 0; occurMasks[i] != END_OCCURS; i++) m_kcomboFreq->insertItem (KMyMoneyUtils::occurenceToString (occurMasks[i]));
  m_accountCombo->blockSignals(true);
  m_kcomboTo->blockSignals(true);
  m_category->blockSignals(true);
  m_accountCombo->loadList(
    (KMyMoneyUtils::categoryTypeE)(KMyMoneyUtils::asset | KMyMoneyUtils::liability));
  m_kcomboTo->loadList(
    (KMyMoneyUtils::categoryTypeE)(KMyMoneyUtils::asset | KMyMoneyUtils::liability));
  MyMoneyObjectContainer objects;
  AccountSet categories(&objects);
  categories.addAccountGroup(MyMoneyAccount::Income);
  categories.addAccountGroup(MyMoneyAccount::Expense);
  categories.load(m_category->selector());
  m_accountCombo->blockSignals(false);
  m_kcomboTo->blockSignals(false);
  m_category->blockSignals(false);

  // Fire off the activated signals.
  if (m_schedule.name().isEmpty() && m_transaction.splitCount() == 0)
  {
    slotDateChanged(QDate::currentDate());
    slotFrequencyChanged(0);
    slotMethodChanged(0);
    if (m_actionType == MyMoneySplit::ActionTransfer ||
        m_actionType == MyMoneySplit::ActionAmortization ||
        m_actionType == MyMoneySplit::ActionWithdrawal)
      slotAccountChanged(0);

    if (m_actionType == MyMoneySplit::ActionDeposit ||
        m_actionType == MyMoneySplit::ActionAmortization ||
        m_actionType == MyMoneySplit::ActionTransfer)
      slotToChanged(0);
  }
  else if (m_schedule.name().isEmpty() && m_transaction.splitCount() >= 2)
    slotDateChanged(QDate::currentDate());
}

void KEditScheduleDialog::createSecondSplit(void)
{
  if(m_transaction.splitCount() == 1) {
    MyMoneySplit split, osplit;
    split.setAction(m_transaction.splits()[0].action());
    split.setPayeeId(m_transaction.splits()[0].payeeId());
    split.setMemo(m_transaction.splits()[0].memo());
    split.setValue(-m_transaction.splits()[0].value());
    m_transaction.addSplit(split);
  }
}

/*
 * Cribbed from : KGlobalLedgerView::slotEditSplits(void)
 */
void KEditScheduleDialog::slotEditSplits(void)
{
  bool isDeposit = false;
  bool isValidAmount = false;

  // force focus change to update all data
  QWidget* w = m_category->splitButton();
  if(w)
    w->setFocus();

  if(m_kmoneyeditAmount->text().length() != 0)
  {
    if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
      isDeposit = true;
    else
      isDeposit = false;
    isValidAmount = true;
  }

  if(theAccountId().isEmpty()) {
    KMessageBox::information(this, i18n("Please specify the account first before you assign splits."));
    return;
  }

  try
  {
    MyMoneySplit split1;

    checkCategory();
    checkPayee();

    MyMoneyAccount acc  = MyMoneyFile::instance()->account(theAccountId());

    // theAccountId() returned the account that this schedule is attached
    // to. In case of a loan payment, we need to find the account that
    // the amortization goes to. This account contains a member that gives
    // us the fixed amount used for the calculated values of amortization
    // and interest which we have to pass to KSplitTransactionDlg().

    MyMoneyMoney calculatedValue(0);
    QValueList<MyMoneySplit>::ConstIterator it_s;
    for(it_s = m_transaction.splits().begin(); it_s != m_transaction.splits().end(); ++it_s) {
      if((*it_s).accountId() == theAccountId() || (*it_s).accountId().isEmpty())
        continue;
      MyMoneyAccount tmpacc = MyMoneyFile::instance()->account((*it_s).accountId());
      if(tmpacc.accountType() == MyMoneyAccount::Loan
      || tmpacc.accountType() == MyMoneyAccount::AssetLoan) {
        MyMoneyAccountLoan lacc(tmpacc);
        calculatedValue = lacc.periodicPayment();
        if(tmpacc.accountType() == MyMoneyAccount::AssetLoan) {
          calculatedValue = -calculatedValue;
        }
        break;
      }
    }

    m_category->blockSignals(true);

    MyMoneyObjectContainer objects;
    QMap<QCString, MyMoneyMoney> priceInfo;
    KSplitTransactionDlg* dlg = new KSplitTransactionDlg(m_transaction,
                                                         acc,
                                                         isValidAmount,
                                                         isDeposit,
                                                         calculatedValue,
                                                         &objects,
                                                         priceInfo,
                                                         this);
    connect(dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));

    if(dlg->exec() == QDialog::Accepted)
    {
      m_transaction = dlg->transaction();

      MyMoneySplit s;
      QString category;
      MyMoneyMoney amount;

      disconnect(m_category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));
      switch(m_transaction.splitCount())
      {
        case 2:
          s = m_transaction.splits()[1];
          if(!s.accountId().isEmpty()) {
            m_category->setSelectedItem(s.accountId());
            amount = m_transaction.splits()[0].value().abs();
            m_kmoneyeditAmount->setValue(amount);
          }
          break;

        case 1:
          m_category->setSelectedItem(QCString());
          m_transaction.removeSplits();
          break;

        case 0:
          m_category->setSelectedItem(QCString());
          break;

        default:
          m_category->setSplitTransaction();
          connect(m_category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));
          amount = m_transaction.splits()[0].value().abs();
          m_kmoneyeditAmount->setValue(amount);
          break;
      }

    } else {
      m_cancelSave = true;
    }

    delete dlg;
  }
  catch (MyMoneyException *e)
  {
    KMessageBox::detailedError(this, i18n("Exception in slot split clicked"), e->what());
    delete e;
  }

  // focus jumps into the memo field
  if(m_qlineeditMemo) {
    m_qlineeditMemo->setFocus();
  }

  m_category->blockSignals(false);

  // force update of required group
  m_requiredFields->changed();
}

void KEditScheduleDialog::slotWillEndToggled(bool on)
{
  if (m_kcomboFreq->currentItem()==0)
  {
    KMessageBox::error(this, i18n("The frequency of this schedule must be set to something other than Once"));
    m_qcheckboxEnd->blockSignals(true);
    m_qcheckboxEnd->setChecked(false);
    m_qcheckboxEnd->blockSignals(false);
    m_kcomboFreq->setFocus();
    return;
  }

  m_endLabel1->setEnabled(on);
  m_qlineeditRemaining->setEnabled(on);
  m_endLabel2->setEnabled(on);
  m_kdateinputFinal->setEnabled(on);
  if(!on) {
    m_schedule.setEndDate(QDate());
  }
}

void KEditScheduleDialog::okClicked(void)
{
  m_cancelSave = false;

  // force focus change to update all data
  m_qbuttonOK->setFocus();

  // looks like m_cancelSave could never become true, but
  // the above setFocus() could trigger slotEditSplit() which
  // sets this variable to true under certain circumstances.
  if(m_cancelSave)
    return;

  if (m_scheduleName->text().isEmpty())
  {
    KMessageBox::information(this, i18n("Please fill in the name field."));
    m_scheduleName->setFocus();
    return;
  }

  if (m_payee->currentText().isEmpty())
  {
    KMessageBox::information(this, i18n("Please fill in the payee field."));
    m_payee->setFocus();
    return;
  }

  if (m_kmoneyeditAmount->text().isEmpty())
  {
    KMessageBox::information(this, i18n("Please fill in the amount field."));
    m_kmoneyeditAmount->setFocus();
    return;
  }

  if (m_actionType != MyMoneySplit::ActionTransfer && m_transaction.splitCount() < 2)
  {
    KMessageBox::information(this, i18n("Please fill in the category field."));
    m_category->setFocus();
    return;
  }

  if (m_qcheckboxEnd->isChecked() && !m_kdateinputFinal->date().isValid())
  {
    KMessageBox::information(this, i18n("Please fill in ending date"));
    m_kdateinputFinal->setFocus();
    return;
  }

  if (m_actionType == MyMoneySplit::ActionTransfer)
  {
    if (m_fromAccountId == m_toAccountId)
    {
      KMessageBox::detailedError(this, i18n("Unable to edit schedule"), i18n("Account from and account to are the same"));
      m_kcomboTo->setFocus();
      return;
    }
  }

  if (!checkCategory())
    return;

  checkPayee();

  slotEstimateChanged(); // Fix estimate being set when it shouldnt

  switch (m_weekendOption->currentItem())
  {
    case 0:
      m_schedule.setWeekendOption(MyMoneySchedule::MoveNothing);
      break;
    case 1:
      m_schedule.setWeekendOption(MyMoneySchedule::MoveFriday);
      break;
    case 2:
      m_schedule.setWeekendOption(MyMoneySchedule::MoveMonday);
      break;
  }

  // Change the transaction action if one is specified
  if (m_paymentMethod->currentText().length()>0)
  {
    for (unsigned i=0; i<m_transaction.splitCount(); i++)
    {
      MyMoneySplit split = m_transaction.splits()[i];
      QString s = m_paymentMethod->currentText();
      if (s == i18n("Check"))
        split.setAction(MyMoneySplit::ActionCheck);
      else if (s == i18n("Deposit"))
        split.setAction(MyMoneySplit::ActionDeposit);
      else if (s == i18n("Transfer"))
        split.setAction(MyMoneySplit::ActionTransfer);
      else if (s == i18n("Withdrawal"))
        split.setAction(MyMoneySplit::ActionWithdrawal);
      else if (s == i18n("ATM"))
        split.setAction(MyMoneySplit::ActionATM);
      else
        split.setAction(QCString(s));

      m_transaction.modifySplit(split);
    }
  }

  // Just reset the schedules transaction here.
  m_schedule.setTransaction(m_transaction);

/*
  QString message;
  message += "Schedule Name: " + m_schedule.name() + "\n";
  message += "Account1: " + MyMoneyFile::instance()->account(m_schedule.transaction().splits()[0].accountId()).name() + "\n";
  message += "Account2: " + m_schedule.account().name() + "\n";
  if (m_actionType == MyMoneySplit::ActionTransfer)
    message += "To: " + m_schedule.transaction().splits()[1].accountId() + "\n";
  message += "Method: " + KMyMoneyUtils::paymentMethodToString(m_schedule.paymentType()) + "\n";
  message += "Payee: " + m_schedule.transaction().splits()[0].payeeId() + "\n";
  message += "Date: " + m_schedule.startDate().toString() + "\n";
  message += "Frequency: " + KMyMoneyUtils::occurenceToString(m_schedule.occurence()) + "\n";
  message += "Category: " + MyMoneyFile::instance()->account(m_schedule.transaction().splits()[1].accountId()).name() + "\n";
  message += "Memo: " + m_schedule.transaction().splits()[0].memo() + "\n";
  message += "endDate: " + m_schedule.endDate().toString() + "\n";
  KMessageBox::information(this, message);
*/

  accept();
}

void KEditScheduleDialog::loadWidgetsFromSchedule(void)
{
  try
  {
    // preload the widgets with current available data
    m_payee->loadPayees(MyMoneyFile::instance()->payeeList());

    if (m_schedule.account().name().isEmpty())
      return;

    // always disable some fields when editing
    if(!m_schedule.id().isEmpty()) {
      m_kdateinputDue->setEnabled(false);
      m_kdateinputFinal->setEnabled(false);
      m_qlineeditRemaining->setEnabled(false);
    }
    // FIXME we allow to modify the end of a schedule at any time
    // m_qcheckboxEnd->setEnabled(false);

    // for (int i = 0; i < m_transaction.splitCount(); i++) dumpSplit (i, m_transaction.splits()[i]);
    if (m_actionType == MyMoneySplit::ActionTransfer)
    {
      // Jiggle the splits to how we want them
      MyMoneySplit s1 = m_transaction.splits()[0];
      MyMoneySplit s2 = m_transaction.splits()[1];

      if (!s1.value().isNegative())
      {
        try {
        m_transaction.removeSplits();
        s2.clearId();
        s1.clearId();
        m_transaction.addSplit(s2);
        m_transaction.addSplit(s1);
        } catch (MyMoneyException *e)
        {
          KMessageBox::detailedError(this, i18n("Exception in loadWidgetsFromSchedule(1)"), e->what());
          qDebug("e: %s", e->what().latin1());
          delete e;
        }
      }
    }

    if (m_actionType != MyMoneySplit::ActionDeposit) {
      m_fromAccountId = m_transaction.splits()[0].accountId();
      //m_accountCombo->setCurrentText(MyMoneyFile::instance()->account(m_transaction.splits()[0].accountId()).name());
      m_accountCombo->setSelected (m_fromAccountId);
    } else{
      m_toAccountId = m_transaction.splits()[0].accountId();
      //m_kcomboTo->setCurrentText(MyMoneyFile::instance()->account(m_transaction.splits()[0].accountId()).name());
      m_kcomboTo->setSelected (m_toAccountId);
    }
    if (m_actionType == MyMoneySplit::ActionTransfer
    ||  m_actionType == MyMoneySplit::ActionAmortization)
    {
      m_toAccountId = m_transaction.splits()[1].accountId();
     // m_kcomboTo->setCurrentText(
       // MyMoneyFile::instance()->account(m_transaction.splits()[1].accountId()).name());
      m_kcomboTo->setSelected (m_toAccountId);
    }
    m_payee->setSelectedItem(m_transaction.splitByAccount(theAccountId()).payeeId());

#if 0
    m_payee->loadText(MyMoneyFile::instance()->payee(m_transaction.splitByAccount(theAccountId()).payeeId()).name());
#endif

    m_kdateinputDue->setDate(m_schedule.nextPayment(m_schedule.lastPayment())/*m_schedule.startDate()*/);

    int method=0;
    if (m_actionType == MyMoneySplit::ActionTransfer
    ||  m_actionType == MyMoneySplit::ActionAmortization)
    {
      switch (m_schedule.paymentType())
      {
        case MyMoneySchedule::STYPE_DIRECTDEBIT:
          method = 0;
          m_kcomboMethod->setCurrentItem(0);
          break;
        case MyMoneySchedule::STYPE_DIRECTDEPOSIT:
          method = 1;
          m_kcomboMethod->setCurrentItem(1);
          break;
        case MyMoneySchedule::STYPE_MANUALDEPOSIT:
          method = 2;
          m_kcomboMethod->setCurrentItem(2);
          break;
        case MyMoneySchedule::STYPE_WRITECHEQUE:
          method = 3;
          m_kcomboMethod->setCurrentItem(3);
          break;
        case MyMoneySchedule::STYPE_OTHER:
          method = 4;
          m_kcomboMethod->setCurrentItem(4);
          break;
        default:
          break;
      }
    }
    else if (m_actionType == MyMoneySplit::ActionDeposit)
    {
      switch (m_schedule.paymentType())
      {
        case MyMoneySchedule::STYPE_DIRECTDEPOSIT:
          method = 0;
          m_kcomboMethod->setCurrentItem(0);
          break;
        case MyMoneySchedule::STYPE_MANUALDEPOSIT:
          method = 1;
          m_kcomboMethod->setCurrentItem(1);
          break;
        case MyMoneySchedule::STYPE_OTHER:
          method = 2;
          m_kcomboMethod->setCurrentItem(2);
          break;
        default:
          break;
      }
    }
    else
    {
      switch (m_schedule.paymentType())
      {
        case MyMoneySchedule::STYPE_DIRECTDEBIT:
          method = 0;
          m_kcomboMethod->setCurrentItem(0);
          break;
        case MyMoneySchedule::STYPE_WRITECHEQUE:
          method = 1;
          m_kcomboMethod->setCurrentItem(1);
          break;
        case MyMoneySchedule::STYPE_OTHER:
          method = 2;
          m_kcomboMethod->setCurrentItem(2);
          break;
        default:
          break;
      }
    }

    int frequency=0;
    for (frequency = 0; occurMasks[frequency] != END_OCCURS; frequency++) if (occurMasks[frequency] == m_schedule.occurence()) break;
    if (occurMasks[frequency] == END_OCCURS) frequency = 0;
    m_kcomboFreq->setCurrentItem(frequency);

    MyMoneyMoney amount = m_transaction.splitByAccount(theAccountId()).value();
    amount = amount.abs();
    m_kmoneyeditAmount->setValue(amount);
    m_qcheckboxEstimate->setChecked(!m_schedule.isFixed());

    if (m_actionType != MyMoneySplit::ActionTransfer)
    {
      if (m_transaction.splitCount() >= 3)
      {
        m_category->setSplitTransaction();
        connect(m_category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));
      }
      else if(m_actionType == MyMoneySplit::ActionAmortization)
      {
        m_category->setCurrentText(i18n("Loan payment"));
        m_category->setSuppressObjectCreation(true);
        connect(m_category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));
      }
      else
        m_category->setSelectedItem(m_transaction.splitByAccount(theAccountId(), false).accountId());
    }

    m_qlineeditMemo->setText(m_transaction.splitByAccount(theAccountId()).memo());
    m_qcheckboxAuto->setChecked(m_schedule.autoEnter());
    m_qcheckboxEnd->setChecked(m_schedule.willEnd());
    if (m_schedule.willEnd())
    {
      m_endLabel1->setEnabled(true);
      m_qlineeditRemaining->setEnabled(true);
      m_endLabel2->setEnabled(true);
      m_kdateinputFinal->setEnabled(true);
      m_qlineeditRemaining->setText(QString::number(m_schedule.transactionsRemaining()));
      m_kdateinputFinal->setDate(m_schedule.endDate());
    }

    m_scheduleName->setText(m_schedule.name());

    switch (m_schedule.weekendOption())
    {
      case MyMoneySchedule::MoveNothing:
        m_weekendOption->setCurrentItem(0);
        break;
      case MyMoneySchedule::MoveFriday:
        m_weekendOption->setCurrentItem(1);
        break;
      case MyMoneySchedule::MoveMonday:
        m_weekendOption->setCurrentItem(2);
        break;
    }

    for (int i=0; i < m_paymentMethod->count(); i++)
    {
      if (QString(m_paymentMethod->text(i)) == i18n(m_transaction.splits()[0].action()))
      {
        m_paymentMethod->setCurrentItem(i);
        break;
      }
    }

    // Quick hack
    slotFrequencyChanged(frequency);
    slotMethodChanged(method);
  } catch (MyMoneyException *e)
  {
    KMessageBox::detailedError(this, i18n("Exception in loadWidgetsFromSchedule(2)"), e->what());
    delete e;
  }
}

void KEditScheduleDialog::slotRemainingChanged(const QString& text)
{
  // Make sure the required fields are set
  m_schedule.setStartDate(m_kdateinputDue->date());
  m_schedule.setOccurence(comboToOccurence());

  if (m_schedule.transactionsRemaining() != text.toInt())
  {
    m_kdateinputFinal->setDate(m_schedule.dateAfter(text.toInt()));
  }
}

void KEditScheduleDialog::slotEndDateChanged(const QDate& date)
{
  // Make sure the required fields are set
  m_schedule.setStartDate(m_kdateinputDue->date());
  m_schedule.setOccurence(comboToOccurence());

  if (m_schedule.endDate() != date)
  {
    m_schedule.setEndDate(date);
    if (m_schedule.transactionsRemaining() != m_qlineeditRemaining->text().toInt());
      m_qlineeditRemaining->setText(QString::number(m_schedule.transactionsRemaining()));
  }
}

MyMoneySchedule::occurenceE KEditScheduleDialog::comboToOccurence(void)
{
  return (occurMasks [m_kcomboFreq->currentItem()]);
}

void KEditScheduleDialog::slotAmountChanged(const QString&)
{
  try
  {
    int count = m_transaction.splitCount();
    if (count == 0)
    {
      createSplits();
    }

    MyMoneySplit s = m_transaction.splits()[0];
    MyMoneyMoney amount = s.value();
    if (m_kmoneyeditAmount->value() != amount)
    {
      // Cribbed from KLedgerView::slotAmountChanged

      MyMoneyMoney val = MyMoneyMoney(m_kmoneyeditAmount->text());

      // if someone enters a negative number, we have to make sure that
      // the action is corrected. For transfers, we don't have to do anything
      // The accounts will be 'exchanged' in reloadEditWidgets() and fillForm()
      if(val.isNegative())
      {
        if (m_actionType == MyMoneySplit::ActionDeposit)
        {
          s.setAction(MyMoneySplit::ActionWithdrawal);
          val = -val;
        }
        else if (m_actionType == MyMoneySplit::ActionWithdrawal)
        {
          s.setAction(MyMoneySplit::ActionDeposit);
          val = -val;
        }
      }

      if (m_actionType != MyMoneySplit::ActionDeposit)
      {
        val = -val;
      }

      // FIXME settings shares == value does not support multiple currencies
      //       we need to fix this
      s.setValue(val);
      s.setShares(val);
      m_transaction.modifySplit(s);

      if(m_transaction.splitCount() == 2)
      {
        MyMoneySplit split = m_transaction.splits()[1];
        split.setValue(-s.value());
        split.setShares(split.value());
        m_transaction.modifySplit(split);
      }
      else if (count >= 3)
      {
        if(!m_transaction.splitSum().isZero()) {
          KMessageBox::information(this, i18n("Your recent change caused the split transaction to contain some unassigned amount. You will now be directed to the split transaction editor to resolve the issue."), i18n("Unassigned amount"), "ScheduleSplitsumNotZero");
          slotEditSplits();
        }
      }
    }
  }
  catch (MyMoneyException *e)
  {
    KMessageBox::detailedError(this, i18n("Error in slotAmountChanged?"), e->what() + " : " + m_schedule.account().name());
    delete e;
  }
}

void KEditScheduleDialog::slotAccountChanged(const QCString& id)
{
  m_fromAccountId = id;
  if (m_actionType != MyMoneySplit::ActionDeposit)
  {
    int count = m_transaction.splitCount();
    if (count == 0)
    {
      createSplits();
    }

    MyMoneySplit s = m_transaction.splits()[0];
    s.setAccountId(id);
    m_transaction.modifySplit(s);
    if(!id.isEmpty()) {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
      m_transaction.setCommodity(acc.currencyId());
    }
  }
}

void KEditScheduleDialog::slotScheduleNameChanged(const QString& text)
{
  m_schedule.setName(text);
}

void KEditScheduleDialog::slotToChanged(const QCString& id)
{
  m_toAccountId = id;
  if (m_actionType == MyMoneySplit::ActionTransfer
  ||  m_actionType == MyMoneySplit::ActionAmortization)
  {
    int count = m_transaction.splitCount();
    if (count == 0)
    {
      createSplits();
    }
    MyMoneySplit s = m_transaction.splits()[1];
    s.setAccountId(id);
    m_transaction.modifySplit(s);
  }
  else if (m_actionType == MyMoneySplit::ActionDeposit)
  {
    int count = m_transaction.splitCount();
    if (count == 0)
    {
      createSplits();
    }
    MyMoneySplit s = m_transaction.splits()[0];
    s.setAccountId(id);
    m_transaction.modifySplit(s);
  }
}

void KEditScheduleDialog::slotMethodChanged(int item)
{
  if (m_actionType == MyMoneySplit::ActionTransfer)
  {
    switch (item)
    {
      case 0:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_DIRECTDEBIT);
        break;
      case 1:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_DIRECTDEPOSIT);
        break;
      case 2:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_MANUALDEPOSIT);
        break;
      case 3:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_WRITECHEQUE);
        break;
      case 4:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_OTHER);
        break;
      default:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_ANY);
        break;
    }

    m_schedule.setType(MyMoneySchedule::TYPE_TRANSFER);
  }
  else if (m_actionType == MyMoneySplit::ActionDeposit)
  {
    switch (item)
    {
      case 0:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_DIRECTDEPOSIT);
        break;
      case 1:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_MANUALDEPOSIT);
        break;
      case 2:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_OTHER);
        break;
      default:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_ANY);
        break;
    }

    m_schedule.setType(MyMoneySchedule::TYPE_DEPOSIT);
  }
  else
  {
    switch (item)
    {
      case 0:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_DIRECTDEBIT);
        break;
      case 1:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_WRITECHEQUE);
        break;
      case 2:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_OTHER);
        break;
      default:
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_ANY);
        break;
    }

    m_schedule.setType(MyMoneySchedule::TYPE_BILL);
  }
}

void KEditScheduleDialog::slotPayeeChanged(const QString& text)
{
  QCString payeeId;
  try
  {
    payeeId = MyMoneyFile::instance()->payeeByName(text).id();

    int count = m_transaction.splitCount();
    if (count == 0)
    {
      createSplits();
    }

    MyMoneySplit s = m_transaction.splits()[0];
    s.setPayeeId(payeeId);
    m_transaction.modifySplit(s);
  }
  catch (MyMoneyException *e)
  {
       // KMessageBox::detailedError(this, i18n("Exception in slot PayeeChanged"), e->what());
    delete e;
  }
}

void KEditScheduleDialog::slotDateChanged(const QDate& date)
{
  if (m_kdateinputDue->date() <= QDate::currentDate() &&
      m_qcheckboxAuto->isChecked())
  {
    KMessageBox::error(this, i18n("The schedule can not be automatically entered when the start date is on or before todays date."));
    m_qcheckboxAuto->blockSignals(true);
    m_qcheckboxAuto->setChecked(false);
    m_qcheckboxAuto->blockSignals(false);
  }

  if (date != m_schedule.startDate() && m_qcheckboxEnd->isChecked())
  {
    // Re-set the end date
    m_schedule.setStartDate(date);
    m_kdateinputFinal->setDate(m_schedule.dateAfter(m_qlineeditRemaining->text().toInt()));
  }
  else
    m_schedule.setStartDate(date);
}

void KEditScheduleDialog::slotFrequencyChanged(int)
{
  if (m_qcheckboxEnd->isChecked() && m_kcomboFreq->currentItem() == 0)
  {
    KMessageBox::error(this, i18n("The end date can not be set for occurences set to Once"));
    m_qcheckboxEnd->setChecked(false);
  }

  m_schedule.setOccurence(comboToOccurence());
}

void KEditScheduleDialog::slotEstimateChanged(void)
{
  if (m_qcheckboxAuto->isChecked())
  {
/*
    if (m_qcheckboxEstimate->isChecked())
    {
      KMessageBox::error(this, i18n("The amount must not be an estimate to be automatically entered"));
      m_qcheckboxEstimate->blockSignals(true);
      m_qcheckboxEstimate->setChecked(false);
      m_qcheckboxEstimate->blockSignals(false);
      m_qcheckboxAuto->setFocus();
      return;
    }
*/
}

  m_schedule.setFixed(!m_qcheckboxEstimate->isChecked());
}

void KEditScheduleDialog::slotCategoryChanged(const QCString& id)
{
  if(!id.isEmpty()) {
    int count = m_transaction.splitCount();
    if (count == 0)
    {
      createSplits();
    }

    // Dont worry about transfers
    MyMoneySplit s = m_transaction.splits()[1];
    s.setAccountId(id);
    m_transaction.modifySplit(s);
  }
}

void KEditScheduleDialog::slotCategoryChanged(const QString& text)
{
  if (text != i18n("Split transaction (category replacement)", "Split transaction"))
  {
    int count = m_transaction.splitCount();
    if (count == 0)
    {
      createSplits();
    }

    // Dont worry about transfers
    MyMoneySplit s = m_transaction.splits()[1];
    QString category = text;
    QCString id = MyMoneyFile::instance()->categoryToAccount(category);
    if(id.isEmpty() && !category.isEmpty())
    {
      // They are probably still typing
      // The category gets checked in okClicked()
      return;
    }
    s.setAccountId(id);
    m_transaction.modifySplit(s);
  }
}

void KEditScheduleDialog::slotAutoEnterChanged(void)
{
  if (m_kdateinputDue->date() <= QDate::currentDate())
  {
    if (m_qcheckboxAuto->isChecked())
    {
      KMessageBox::error(this, i18n("The start date must be greater than today, to automatically enter this schedule."));
      m_qcheckboxAuto->blockSignals(true);
      m_qcheckboxAuto->setChecked(false);
      m_qcheckboxAuto->blockSignals(false);
      m_kdateinputDue->setFocus();
      return;
    }
  }

/*
  if (m_qcheckboxEstimate->isChecked())
  {
    if (m_qcheckboxAuto->isChecked())
    {
      KMessageBox::error(this, i18n("The amount must not be an estimate to be automatically entered"));
      m_qcheckboxAuto->blockSignals(true);
      m_qcheckboxAuto->setChecked(false);
      m_qcheckboxAuto->blockSignals(false);
      m_qcheckboxEstimate->setFocus();
      return;
    }
  }
*/
  m_schedule.setAutoEnter(m_qcheckboxAuto->isChecked());
}

void KEditScheduleDialog::slotMemoChanged(const QString& text)
{
  int count = m_transaction.splitCount();
  if (count == 0)
  {
    createSplits();
  }

  MyMoneySplit s = m_transaction.splits()[0];
  s.setMemo(text);
  m_transaction.modifySplit(s);

  if (m_actionType == MyMoneySplit::ActionTransfer)
  {
    s = m_transaction.splits()[1];
    s.setMemo(text);
    m_transaction.modifySplit(s);
  }
}

void KEditScheduleDialog::createSplits(void)
{
  if (m_transaction.splitCount() == 0)
  {
    MyMoneySplit split1;
    split1.setAccountId(theAccountId());
    split1.setAction(m_actionType);
    m_transaction.addSplit(split1);

    if (m_actionType == MyMoneySplit::ActionTransfer)
    {
      MyMoneySplit split2;
      split2.setAccountId((QCString)m_kcomboTo->currentText());
      split2.setAction(MyMoneySplit::ActionTransfer);
      m_transaction.addSplit(split2);
    }
    else
    {
      MyMoneySplit split2;
      if (m_actionType == MyMoneySplit::ActionDeposit)
        split2.setAction(MyMoneySplit::ActionWithdrawal);
      else /* MyMoneySplit::ActionWithdrawl */
        split2.setAction(MyMoneySplit::ActionDeposit);
      m_transaction.addSplit(split2);
    }
  }
}

bool KEditScheduleDialog::checkCategory(void)
{
  bool exitDialog = true;

  // Make sure a category has been set
  if (m_category->isSplitTransaction() && !m_category->selectedItem().isEmpty())
  {
    bool invalid=false;
    QCString categoryId = m_category->selectedItem();
    if (m_category->selectedItem().isEmpty())
      invalid = true;

    if (invalid)
    {
      // Create the category
      QString message = QString("The category '%1' does not exist.  Create?").arg(m_category->currentText());
      if (KMessageBox::questionYesNo(this, message) == KMessageBox::Yes)
      {
        MyMoneyAccount base;
        if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
          base = MyMoneyFile::instance()->income();
        else
          base = MyMoneyFile::instance()->expense();

        categoryId = MyMoneyFile::instance()->createCategory(base, m_category->currentText());

        // Modify the split
        MyMoneySplit s = m_transaction.splits()[1];
        s.setAccountId(categoryId);
        m_transaction.modifySplit(s);

        QString type, stype;
        MyMoneyAccount::accountTypeE typeE = MyMoneyFile::instance()->account(categoryId).accountType();
        if (m_actionType == MyMoneySplit::ActionDeposit &&
              typeE != MyMoneyAccount::Income)
        {
          type = i18n("Expense");
          stype = i18n("Deposit");
        }
        else if (m_actionType != MyMoneySplit::ActionDeposit &&
              typeE != MyMoneyAccount::Expense)
        {
          type = i18n("Income");
          stype = i18n("Withdrawal or Transfer");
        }

        if (!type.isEmpty())
        {
          QString message = i18n("You have specified an %1 category for a %2 schedule. Do you want to keep it that way?").arg(type).arg(stype);
          if(KMessageBox::warningYesNo(this, message, i18n("Verify category type")) == KMessageBox::No) {
            m_category->setSelectedItem(QCString());
            m_category->setFocus();
            exitDialog = false;
          }
        }
      }
      else
      {
        m_category->setSelectedItem(QCString());
        m_category->setFocus();
        exitDialog = false;
      }
    }
    else
    {
      QString type, stype;
      MyMoneyAccount::accountTypeE typeE = MyMoneyFile::instance()->account(categoryId).accountType();
      if (m_actionType == MyMoneySplit::ActionDeposit &&
            typeE != MyMoneyAccount::Income)
      {
        type = i18n("Expense");
        stype = i18n("Deposit");
      }
      else if (m_actionType != MyMoneySplit::ActionDeposit &&
            typeE != MyMoneyAccount::Expense)
      {
        type = i18n("Income");
        stype = i18n("Bill or Transfer");
      }

      if (!type.isEmpty())
      {
        QString message = i18n("You have specified an %1 category for a %2 schedule. Do you want to keep it that way?").arg(type).arg(stype);
        if(KMessageBox::warningYesNo(this, message, i18n("Verify category type")) == KMessageBox::No) {
          m_category->setSelectedItem(QCString());
          m_category->setFocus();
          exitDialog = false;
        }
      }
    }
  }

  return exitDialog;
}

void KEditScheduleDialog::checkPayee(void)
{
  QCString payeeId = m_payee->selectedItem();
#if 0
  try
  {
    payeeId = MyMoneyFile::instance()->payeeByName(m_payee->text()).id();
  }
  catch (MyMoneyException *e)
  {
    MyMoneyPayee payee(m_payee->text());
    MyMoneyFile::instance()->addPayee(payee);
    payeeId = payee.id();
    delete e;
  }
#endif

  MyMoneySplit s = m_transaction.splits()[0];
  s.setPayeeId(payeeId);
  m_transaction.modifySplit(s);

  if (m_actionType == MyMoneySplit::ActionTransfer)
  {
    s = m_transaction.splits()[1];
    s.setPayeeId(payeeId);
    m_transaction.modifySplit(s);
  }
}

QCString KEditScheduleDialog::theAccountId(void)
{
  if (m_actionType == MyMoneySplit::ActionTransfer ||
      m_actionType == MyMoneySplit::ActionAmortization ||
      m_actionType == MyMoneySplit::ActionWithdrawal ||
      m_actionType == MyMoneySplit::ActionCheck ||
      m_actionType == MyMoneySplit::ActionATM)
  {
    return m_fromAccountId;
  }
  else if (m_actionType == MyMoneySplit::ActionDeposit)
  {
    return m_toAccountId;
  }

  return QCString();
}

void KEditScheduleDialog::slotHelp(void)
{
  kapp->invokeHelp("details.schedules.new");
}

#include "ieditscheduledialog.moc"
