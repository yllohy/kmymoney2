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
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kconfig.h>
#include <kmessagebox.h>
#include <knumvalidator.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "../widgets/kmymoneycombo.h"
#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneypayee.h"
#include "../widgets/kmymoneyedit.h"
#include "../dialogs/ksplittransactiondlg.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneydateinput.h"
#include "ieditscheduledialog.h"

KEditScheduleDialog::KEditScheduleDialog(const QCString& action, const MyMoneySchedule& schedule, QWidget *parent, const char *name)
 : kEditScheduledTransferDlgDecl(parent,name, true)
{
  m_actionType = action;
  m_schedule = schedule;
  m_transaction = schedule.transaction();

  KIntValidator *validator = new KIntValidator(1, 32768, this);
  m_qlineeditRemaining->setValidator(validator);

  readConfig();
  reloadFromFile();
  loadWidgetsFromSchedule();

  if (m_actionType != MyMoneySplit::ActionTransfer)
  {
    TextLabel1_3->setEnabled(false);
    m_kcomboTo->setEnabled(false);
    if (m_actionType == MyMoneySplit::ActionDeposit)
      setCaption(i18n("Edit Deposit Schedule"));
    else
      setCaption(i18n("Edit Bill Schedule"));
  }
  else
  {
    setCaption(i18n("Edit Transfer Schedule"));
  }

  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_qbuttonSplit, SIGNAL(clicked()), this, SLOT(slotSplitClicked()));
  connect(m_qcheckboxEnd, SIGNAL(toggled(bool)), this, SLOT(slotWillEndToggled(bool)));
  connect(m_qbuttonOK, SIGNAL(clicked()), this, SLOT(okClicked()));
  connect(m_qlineeditRemaining, SIGNAL(textChanged(const QString&)),
    this, SLOT(slotRemainingChanged(const QString&)));
  connect(m_kdateinputFinal, SIGNAL(dateChanged(const QDate&)),
    this, SLOT(slotEndDateChanged(const QDate&)));
}

KEditScheduleDialog::~KEditScheduleDialog()
{
  writeConfig();
}

void KEditScheduleDialog::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
//  m_lastPayee = config->readEntry("LastPayee");
}

void KEditScheduleDialog::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
//  config->writeEntry("LastPayee", m_kcomboPayTo->currentText());
  config->sync();
}

void KEditScheduleDialog::reloadFromFile(void)
{
  if (m_actionType == MyMoneySplit::ActionTransfer)
  {
    m_kcomboMethod->insertItem(i18n("Direct Debit"));
    m_kcomboMethod->insertItem(i18n("Direct Deposit"));
    m_kcomboMethod->insertItem(i18n("Manual Deposit"));
    m_kcomboMethod->insertItem(i18n("Write Cheque"));
    m_kcomboMethod->insertItem(i18n("Other"));
  }
  else if (m_actionType == MyMoneySplit::ActionDeposit)
  {
    m_kcomboMethod->insertItem(i18n("Direct Deposit"));
    m_kcomboMethod->insertItem(i18n("Manual Deposit"));
    m_kcomboMethod->insertItem(i18n("Other"));
  }
  else // Withdrawal
  {
    m_kcomboMethod->insertItem(i18n("Direct Debit"));
    m_kcomboMethod->insertItem(i18n("Write Cheque"));
    m_kcomboMethod->insertItem(i18n("Other"));
  }

  m_kcomboFreq->insertItem(i18n("Once"));
  m_kcomboFreq->insertItem(i18n("Daily"));
  m_kcomboFreq->insertItem(i18n("Weekly"));
  m_kcomboFreq->insertItem(i18n("Every two weeks"));
  m_kcomboFreq->insertItem(i18n("Every four weeks"));
  m_kcomboFreq->insertItem(i18n("Monthly"));
  m_kcomboFreq->insertItem(i18n("Every other month"));
  m_kcomboFreq->insertItem(i18n("Every three months"));
  m_kcomboFreq->insertItem(i18n("Every four months"));
  m_kcomboFreq->insertItem(i18n("Twice a year"));
  m_kcomboFreq->insertItem(i18n("Yearly"));
  m_kcomboFreq->insertItem(i18n("Every other year"));

  m_accountCombo->loadAccounts(true, false);
  m_kcomboTo->loadAccounts(true, false);
}

/*
 * Cribbed from : KLedgerViewCheckings::slotOpenSplitDialog(void)
 */
void KEditScheduleDialog::slotSplitClicked()
{
  bool isDeposit = false;
  bool isValidAmount = false;

  if(m_kmoneyeditAmount->text().length() != 0)
  {
    isDeposit = false;
    isValidAmount = true;
  }

  try
  {
    MyMoneySplit split1;

    if (m_transaction.splitCount() == 0)
    {
      MyMoneyFile *file = MyMoneyFile::instance();

      QCString payeeId;
      
      try
      {
        payeeId = file->payeeByName(m_kcomboPayTo->text()).id();
      }
      catch (MyMoneyException *e)
      {
        delete e;
      }

      split1.setAccountId(m_accountCombo->currentAccountId());
      split1.setAction(m_actionType);
      split1.setPayeeId(payeeId);
      split1.setMemo(m_qlineeditMemo->text());
      split1.setValue(m_kmoneyeditAmount->getMoneyValue());
      m_transaction.addSplit(split1);

      MyMoneySplit split2;
      split2.setAccountId(m_kcomboTo->currentAccountId());
      split2.setAction(m_actionType);
      split2.setPayeeId(split1.payeeId());
      split2.setMemo(split1.memo());
      split2.setValue(-split1.value());
      m_transaction.addSplit(split2);
    }

    MyMoneyAccount acc = MyMoneyFile::instance()->account(m_accountCombo->currentAccountId());

    KSplitTransactionDlg* dlg = new KSplitTransactionDlg(m_transaction,
                                                         acc,
                                                         isValidAmount,
                                                         isDeposit,
                                                         this);

    if(dlg->exec())
    {
      m_transaction = dlg->transaction();

      try {
        switch(m_transaction.splitCount()) {
          case 2:
            disconnect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
            break;
          case 1:
            disconnect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
            break;
          case 0:
            disconnect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
            break;
          default:
            connect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
            break;
        }
      } catch (MyMoneyException *e) {
        delete e;
      }

      reloadWidgets();

      m_kmoneyeditAmount->setFocus();
    }

    delete dlg;
  } catch (MyMoneyException *e)
  {
    KMessageBox::detailedError(this, i18n("Exception in slot split clicked"), e->what());
    delete e;
  }

}

void KEditScheduleDialog::slotWillEndToggled(bool on)
{
  m_endLabel1->setEnabled(on);
  m_qlineeditRemaining->setEnabled(on);
  m_endLabel2->setEnabled(on);
  m_kdateinputFinal->setEnabled(on);
}

void KEditScheduleDialog::reloadWidgets(void)
{
  QString category;
  try {
    MyMoneySplit s;
    switch(m_transaction.splitCount()) {
      case 2:
        s = m_transaction.split(m_accountCombo->currentAccountId(), false);
        category = MyMoneyFile::instance()->accountToCategory(s.accountId());
        break;
      case 1:
        category = " ";
        break;
      default:
        category = i18n("Splitted transaction");
        break;
    }
  } catch (MyMoneyException *e) {
    delete e;
    category = " ";
  }

  m_category->setText(category);

  MyMoneySplit split = m_transaction.split(m_accountCombo->currentAccountId());
  MyMoneyMoney amount(split.value());
  if (amount < 0)
    amount = -amount;
  m_kmoneyeditAmount->setText(amount.formatMoney());
}

MyMoneySchedule KEditScheduleDialog::schedule(void)
{
  return m_schedule;
}

void KEditScheduleDialog::okClicked()
{
  if (m_scheduleName->text() == "")
  {
    KMessageBox::information(this, i18n("Please fill in the name field."));
    m_scheduleName->setFocus();
    return;
  }

  if (m_kcomboPayTo->text() == "")
  {
    KMessageBox::information(this, i18n("Please fill in the payee field."));
    m_kcomboPayTo->setFocus();
    return;
  }

  if (m_kmoneyeditAmount->text() == "")
  {
    KMessageBox::information(this, i18n("Please fill in the amount field."));
    m_kmoneyeditAmount->setFocus();
    return;
  }

  if (m_category->text() == "")
  {
    KMessageBox::information(this, i18n("Please fill in the category field."));
    m_category->setFocus();
    return;
  }

  if (m_actionType == MyMoneySplit::ActionTransfer)
  {
    if (m_accountCombo->currentText() == m_kcomboTo->currentText())
    {
      KMessageBox::detailedError(this, i18n("Unable to edit schedule"), i18n("Account from and account to are the same"));
      m_kcomboTo->setFocus();
      return;
    }
  }

  try
  {
    m_schedule.setOccurence(comboToOccurence());

    if (m_actionType == MyMoneySplit::ActionTransfer)
    {
      switch (m_kcomboMethod->currentItem())
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
      switch (m_kcomboMethod->currentItem())
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
      switch (m_kcomboMethod->currentItem())
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

    m_schedule.setStartDate(m_kdateinputDue->getQDate());
    m_schedule.setLastPayment(m_kdateinputDue->getQDate());

    m_schedule.setAutoEnter(m_qcheckboxAuto->isChecked());

    m_schedule.setFixed(!m_qcheckboxEstimate->isChecked());

    MyMoneyFile *file = MyMoneyFile::instance();

    QCString payeeId;
    try
    {
      payeeId = file->payeeByName(m_kcomboPayTo->text()).id();
    }
    catch (MyMoneyException *e)
    {
      MyMoneyPayee payee(m_kcomboPayTo->text());
      file->addPayee(payee);
      payeeId = file->payeeByName(m_kcomboPayTo->text()).id();
      delete e;
    }

    if (m_transaction.splitCount() == 0)
    {
      if (m_kmoneyeditAmount->text() == "")
      {
        KMessageBox::information(this, i18n("Please fill in the amount field"));
        m_kmoneyeditAmount->setFocus();
      }

      MyMoneySplit split1;
      split1.setAccountId(m_accountCombo->currentAccountId());
      split1.setAction(m_actionType);
      split1.setPayeeId(payeeId);
      split1.setMemo(m_qlineeditMemo->text());
      split1.setValue(m_kmoneyeditAmount->getMoneyValue());
      m_transaction.addSplit(split1);

      if (m_actionType == MyMoneySplit::ActionTransfer)
      {
        MyMoneySplit split2;
        split2.setAccountId(m_kcomboTo->currentAccountId());
        split2.setAction(m_actionType);
        split2.setPayeeId(split1.payeeId());
        split2.setMemo(split1.memo());
        split2.setValue(-split1.value());
        m_transaction.addSplit(split2);
      }
    }

    MyMoneySplit s = m_transaction.split(m_accountCombo->currentAccountId());
    s.setPayeeId(payeeId);
    m_transaction.modifySplit(s);

    m_schedule.setTransaction(m_transaction);

    m_schedule.setWillEnd(m_qcheckboxEnd->isChecked());

    if (m_schedule.willEnd())
    {
      m_schedule.setEndDate(m_kdateinputFinal->getQDate());
    }
    m_schedule.setName(m_scheduleName->text());
  } catch (MyMoneyException *e)
  {
    KMessageBox::detailedError(this, i18n("Unable to add transfer"), e->what());
    delete e;
  }

  accept();
}

void KEditScheduleDialog::loadWidgetsFromSchedule(void)
{
  try
  {
    m_accountCombo->setCurrentText(MyMoneyFile::instance()->account(m_schedule.accountId()).name());
    if (m_actionType == MyMoneySplit::ActionTransfer)
      m_kcomboTo->setCurrentText(MyMoneyFile::instance()->account(m_schedule.transferAccountId()).name());
    m_kcomboPayTo->loadText(MyMoneyFile::instance()->payee(m_schedule.transaction().split(m_accountCombo->currentAccountId()).payeeId()).name());
    m_kdateinputDue->setDate(m_schedule.startDate());

    if (m_actionType == MyMoneySplit::ActionTransfer)
    {
      switch (m_schedule.paymentType())
      {
        case MyMoneySchedule::STYPE_DIRECTDEPOSIT:
          m_kcomboMethod->setCurrentItem(0);
          break;
        case MyMoneySchedule::STYPE_MANUALDEPOSIT:
          m_kcomboMethod->setCurrentItem(1);
          break;
        case MyMoneySchedule::STYPE_OTHER:
          m_kcomboMethod->setCurrentItem(2);
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
          m_kcomboMethod->setCurrentItem(0);
          break;
        case MyMoneySchedule::STYPE_MANUALDEPOSIT:
          m_kcomboMethod->setCurrentItem(1);
          break;
        case MyMoneySchedule::STYPE_OTHER:
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
          m_kcomboMethod->setCurrentItem(0);
          break;
        case MyMoneySchedule::STYPE_WRITECHEQUE:
          m_kcomboMethod->setCurrentItem(1);
          break;
        case MyMoneySchedule::STYPE_OTHER:
          m_kcomboMethod->setCurrentItem(2);
          break;
        default:
          break;
      }
    }

    switch (m_schedule.occurence())
    {
      case MyMoneySchedule::OCCUR_ONCE:
        m_kcomboFreq->setCurrentItem(0);
        break;
      case MyMoneySchedule::OCCUR_DAILY:
        m_kcomboFreq->setCurrentItem(1);
        break;
      case MyMoneySchedule::OCCUR_WEEKLY:
        m_kcomboFreq->setCurrentItem(2);
        break;
      case MyMoneySchedule::OCCUR_FORTNIGHTLY:
        m_kcomboFreq->setCurrentItem(3);
        break;
      case MyMoneySchedule::OCCUR_EVERYFOURWEEKS:
        m_kcomboFreq->setCurrentItem(4);
        break;
      case MyMoneySchedule::OCCUR_MONTHLY:
        m_kcomboFreq->setCurrentItem(5);
        break;
      case MyMoneySchedule::OCCUR_EVERYOTHERMONTH:
        m_kcomboFreq->setCurrentItem(6);
        break;
      case MyMoneySchedule::OCCUR_EVERYTHREEMONTHS:
        m_kcomboFreq->setCurrentItem(7);
        break;
      case MyMoneySchedule::OCCUR_EVERYFOURMONTHS:
        m_kcomboFreq->setCurrentItem(8);
        break;
      case MyMoneySchedule::OCCUR_TWICEYEARLY:
        m_kcomboFreq->setCurrentItem(9);
        break;
      case MyMoneySchedule::OCCUR_YEARLY:
        m_kcomboFreq->setCurrentItem(10);
        break;
      case MyMoneySchedule::OCCUR_EVERYOTHERYEAR:
        m_kcomboFreq->setCurrentItem(11);
        break;
      default:
        break;
    }

    MyMoneyMoney amount = m_schedule.transaction().split(m_accountCombo->currentAccountId()).value();
    m_kmoneyeditAmount->setText(amount.formatMoney());
    m_qcheckboxEstimate->setChecked(!m_schedule.isFixed());
    if (m_schedule.transaction().splitCount() >= 3)
      m_category->loadText(i18n("Splitted Transaction"));
    else
      m_category->loadText(MyMoneyFile::instance()->instance()->accountToCategory(m_schedule.transaction().split(m_accountCombo->currentAccountId(), false).accountId()));
    m_qlineeditMemo->setText(m_schedule.transaction().split(m_accountCombo->currentAccountId()).memo());
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
  } catch (MyMoneyException *e)
  {
    delete e;
  }
}

void KEditScheduleDialog::slotRemainingChanged(const QString& text)
{
  // Make sure the required fields are set
  m_schedule.setWillEnd(true);
  m_schedule.setStartDate(m_kdateinputDue->getQDate());
  m_schedule.setLastPayment(m_kdateinputDue->getQDate());
  m_schedule.setOccurence(comboToOccurence());
  
  if (m_schedule.transactionsRemaining() != text.toInt())
  {
    m_kdateinputFinal->setDate(m_schedule.dateAfter(text.toInt()));
  }
}

void KEditScheduleDialog::slotEndDateChanged(const QDate& date)
{
  // Make sure the required fields are set
  m_schedule.setWillEnd(true);
  m_schedule.setStartDate(m_kdateinputDue->getQDate());
  m_schedule.setLastPayment(m_kdateinputDue->getQDate());
  m_schedule.setOccurence(comboToOccurence());

  if (m_schedule.endDate() != date)
  {
    m_schedule.setEndDate(date);
    m_qlineeditRemaining->setText(QString::number(m_schedule.transactionsRemaining()));
  }
}

MyMoneySchedule::occurenceE KEditScheduleDialog::comboToOccurence()
{
  MyMoneySchedule::occurenceE occur;
  
  switch (m_kcomboFreq->currentItem())
  {
    case 0:
      occur = MyMoneySchedule::OCCUR_ONCE;
      break;
    case 1:
      occur = MyMoneySchedule::OCCUR_DAILY;
      break;
    case 2:
      occur = MyMoneySchedule::OCCUR_WEEKLY;
      break;
    case 3:
      occur = MyMoneySchedule::OCCUR_FORTNIGHTLY;
      break;
    case 4:
      occur = MyMoneySchedule::OCCUR_EVERYFOURWEEKS;
      break;
    case 5:
      occur = MyMoneySchedule::OCCUR_MONTHLY;
      break;
    case 6:
      occur = MyMoneySchedule::OCCUR_EVERYOTHERMONTH;
      break;
    case 7:
      occur = MyMoneySchedule::OCCUR_EVERYTHREEMONTHS;
      break;
    case 8:
      occur = MyMoneySchedule::OCCUR_EVERYFOURMONTHS;
      break;
    case 9:
      occur = MyMoneySchedule::OCCUR_TWICEYEARLY;
      break;
    case 10:
      occur = MyMoneySchedule::OCCUR_YEARLY;
      break;
    case 11:
      occur = MyMoneySchedule::OCCUR_EVERYOTHERYEAR;
      break;
    default:
      occur = MyMoneySchedule::OCCUR_ANY;
      break;
  }

  return occur;
}
