/***************************************************************************
                          keditscheduledtransferdlg.cpp  -  description
                             -------------------
    begin                : Sun Feb 17 2002
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
#include "keditscheduledtransferdlg.h"
#include "../widgets/kmymoneyedit.h"
#include "../dialogs/ksplittransactiondlg.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneydateinput.h"

KEditScheduledTransferDlg::KEditScheduledTransferDlg(const QCString& accountId, const MyMoneySchedule& schedule, QWidget *parent, const char *name)
 : kEditScheduledTransferDlgDecl(parent,name, true)
{
  m_accountId = accountId;
  m_schedule = schedule;
  m_transaction = schedule.transaction();

  KIntValidator *validator = new KIntValidator(1, 32768, this);
  m_qlineeditRemaining->setValidator(validator);

  readConfig();
  reloadFromFile();
  loadWidgetsFromSchedule();

  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_qbuttonSplit, SIGNAL(clicked()), this, SLOT(slotSplitClicked()));
  connect(m_qcheckboxEnd, SIGNAL(toggled(bool)), this, SLOT(slotWillEndToggled(bool)));
  connect(m_qbuttonOK, SIGNAL(clicked()), this, SLOT(okClicked()));
}

KEditScheduledTransferDlg::~KEditScheduledTransferDlg()
{
  writeConfig();
}

void KEditScheduledTransferDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
//  m_lastPayee = config->readEntry("LastPayee");
}

void KEditScheduledTransferDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
//  config->writeEntry("LastPayee", m_kcomboPayTo->currentText());
  config->sync();
}

void KEditScheduledTransferDlg::reloadFromFile(void)
{
  m_kcomboMethod->insertItem(i18n("Direct Debit"));
  m_kcomboMethod->insertItem(i18n("Direct Deposit"));
  m_kcomboMethod->insertItem(i18n("Manual Deposit"));
  m_kcomboMethod->insertItem(i18n("Write Cheque"));
  m_kcomboMethod->insertItem(i18n("Other"));

  m_kcomboFreq->insertItem(i18n("Once"));
  m_kcomboFreq->insertItem(i18n("Daily"));
  m_kcomboFreq->insertItem(i18n("Weekly"));
  m_kcomboFreq->insertItem(i18n("Fortnightly"));
  m_kcomboFreq->insertItem(i18n("Every other week"));
  m_kcomboFreq->insertItem(i18n("Every four weeks"));
  m_kcomboFreq->insertItem(i18n("Monthly"));
  m_kcomboFreq->insertItem(i18n("Twice a month"));
  m_kcomboFreq->insertItem(i18n("Every other month"));
  m_kcomboFreq->insertItem(i18n("Every three months"));
  m_kcomboFreq->insertItem(i18n("Every four months"));
  m_kcomboFreq->insertItem(i18n("Twice a year"));
  m_kcomboFreq->insertItem(i18n("Yearly"));
  m_kcomboFreq->insertItem(i18n("Every other year"));

  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyAccount acc;
  QCStringList::ConstIterator it_s;

  acc = file->asset();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
  {
    MyMoneyAccount a = file->account(*it_s);
    if (*it_s == m_accountId)
    {
      m_qlabelAccountFrom->setText(a.name());
      continue;  // Dont add the selected account
    }
    m_kcomboTo->insertItem(a.name());
  }

  acc = file->liability();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
  {
    MyMoneyAccount a = file->account(*it_s);
    if (*it_s == m_accountId)
    {
      m_qlabelAccountFrom->setText(a.name());
      continue;  // Dont add the selected account
    }
    m_kcomboTo->insertItem(a.name());
  }
}

/*
 * Cribbed from : KLedgerViewCheckings::slotOpenSplitDialog(void)
 */
void KEditScheduledTransferDlg::slotSplitClicked()
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

      QValueList<MyMoneyPayee> list = file->payeeList();
      QValueList<MyMoneyPayee>::ConstIterator it_p;

      QCString payeeId;

      for(it_p = list.begin(); it_p != list.end(); ++it_p)
      {
        if ((*it_p).name() == m_kcomboPayTo->text())
          payeeId = (*it_p).id();
      }

      split1.setAccountId(m_accountId);
      split1.setAction(split1.ActionWithdrawal);
      split1.setPayeeId(payeeId);
      split1.setMemo(m_qlineeditMemo->text());
      split1.setValue(m_kmoneyeditAmount->getMoneyValue());
      m_transaction.addSplit(split1);
    }

    MyMoneyAccount acc = MyMoneyFile::instance()->account(m_accountId);

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
    QString s;
    s.sprintf(i18n("Exception in slot split clicked: %s"), e->what().latin1());
    KMessageBox::information(this, s);
    delete e;
  }

}

void KEditScheduledTransferDlg::slotWillEndToggled(bool on)
{
  m_endLabel1->setEnabled(on);
  m_qlineeditRemaining->setEnabled(on);
  m_endLabel2->setEnabled(on);
  m_kdateinputFinal->setEnabled(on);
}

void KEditScheduledTransferDlg::reloadWidgets(void)
{
  QString category;
  try {
    MyMoneySplit s;
    switch(m_transaction.splitCount()) {
      case 2:
        s = m_transaction.split(m_accountId, false);
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

  MyMoneySplit split = m_transaction.split(m_accountId);
  MyMoneyMoney amount(split.value());
  if (amount < 0)
    amount = -amount;
  m_kmoneyeditAmount->setText(amount.formatMoney());
}

MyMoneySchedule KEditScheduledTransferDlg::schedule(void)
{
  return m_schedule;
}

void KEditScheduledTransferDlg::okClicked()
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

  try
  {
    switch (m_kcomboFreq->currentItem())
    {
      case 0:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_ONCE);
        break;
      case 1:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_DAILY);
        break;
      case 2:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_WEEKLY);
        break;
      case 3:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_FORTNIGHTLY);
        break;
      case 4:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
        break;
      case 5:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
        break;
      case 6:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_MONTHLY);
        break;
      case 7:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
        break;
      case 8:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
        break;
      case 9:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
        break;
      case 10:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_TWICEYEARLY);
        break;
      case 11:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_QUARTERLY);
        break;
      case 12:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_YEARLY);
        break;
      case 13:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
        break;
      default:
        m_schedule.setOccurence(MyMoneySchedule::OCCUR_ANY);
        break;
    }

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

    m_schedule.setStartDate(m_kdateinputDue->getQDate());
    m_schedule.setLastPayment(m_kdateinputDue->getQDate());

    m_schedule.setAutoEnter(m_qcheckboxAuto->isChecked());

    m_schedule.setFixed(!m_qcheckboxEstimate->isChecked());

    MyMoneyFile *file = MyMoneyFile::instance();

    QValueList<MyMoneyPayee> list = file->payeeList();
    QValueList<MyMoneyPayee>::ConstIterator it_p;

    QCString payeeId;

    for(it_p = list.begin(); it_p != list.end(); ++it_p)
    {
      if ((*it_p).name() == m_kcomboPayTo->text())
        payeeId = (*it_p).id();
    }

    if (payeeId.length() == 0)
    {
      KMessageBox::information(this, "Adding " + m_kcomboPayTo->text() + " as a payee");
      MyMoneyPayee payee(m_kcomboPayTo->text());
      file->addPayee(payee);
      payeeId = file->payeeByName(m_kcomboPayTo->text()).id();
    }

    if (m_transaction.splitCount() == 0)
    {
      if (m_kmoneyeditAmount->text() == "")
      {
        KMessageBox::information(this, i18n("Please fill in the amount field"));
        m_kmoneyeditAmount->setFocus();
      }

      MyMoneySplit split1;
      split1.setAccountId(m_accountId);
      split1.setAction(split1.ActionWithdrawal);
      split1.setPayeeId(payeeId);
      split1.setMemo(m_qlineeditMemo->text());
      split1.setValue(m_kmoneyeditAmount->getMoneyValue());
      m_transaction.addSplit(split1);

      MyMoneySplit split2;
      split2.setPayeeId(split1.payeeId());
      split2.setMemo(split2.memo());
      split2.setValue(-split2.value());
      m_transaction.addSplit(split2);
    }

    MyMoneySplit s = m_transaction.split(m_accountId);
    s.setPayeeId(payeeId);
    m_transaction.modifySplit(s);

    m_schedule.setTransaction(m_transaction);

    m_schedule.setWillEnd(m_qcheckboxEnd->isChecked());

    if (m_schedule.willEnd())
    {
      m_schedule.setEndDate(m_kdateinputFinal->getQDate());
      m_schedule.setTransactionsRemaining(m_qlineeditRemaining->text().toInt());
    }
    m_schedule.setName(m_scheduleName->text());
  } catch (MyMoneyException *e)
  {
    QString s(i18n("Exception in okClicked: "));
    s += e->what();
    delete e;
    KMessageBox::information(this, s);
  }

  accept();
}

void KEditScheduledTransferDlg::loadWidgetsFromSchedule(void)
{
  try
  {
    m_kcomboPayTo->loadText(MyMoneyFile::instance()->payee(m_schedule.transaction().split(m_accountId).payeeId()).name());
    m_kdateinputDue->setDate(m_schedule.startDate());

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
      case MyMoneySchedule::OCCUR_EVERYOTHERWEEK:
        m_kcomboFreq->setCurrentItem(4);
        break;
      case MyMoneySchedule::OCCUR_EVERYFOURWEEKS:
        m_kcomboFreq->setCurrentItem(5);
        break;
      case MyMoneySchedule::OCCUR_MONTHLY:
        m_kcomboFreq->setCurrentItem(6);
        break;
      case MyMoneySchedule::OCCUR_EVERYOTHERMONTH:
        m_kcomboFreq->setCurrentItem(7);
        break;
      case MyMoneySchedule::OCCUR_EVERYTHREEMONTHS:
        m_kcomboFreq->setCurrentItem(8);
        break;
      case MyMoneySchedule::OCCUR_QUARTERLY:
        m_kcomboFreq->setCurrentItem(9);
        break;
      case MyMoneySchedule::OCCUR_EVERYFOURMONTHS:
        m_kcomboFreq->setCurrentItem(10);
        break;
      case MyMoneySchedule::OCCUR_TWICEYEARLY:
        m_kcomboFreq->setCurrentItem(11);
        break;
      case MyMoneySchedule::OCCUR_YEARLY:
        m_kcomboFreq->setCurrentItem(12);
        break;
      case MyMoneySchedule::OCCUR_EVERYOTHERYEAR:
        m_kcomboFreq->setCurrentItem(13);
        break;
    }

    MyMoneyMoney amount = m_schedule.transaction().split(m_accountId).value();
    m_kmoneyeditAmount->setText(amount.formatMoney());
    m_qcheckboxEstimate->setChecked(!m_schedule.isFixed());
    if (m_schedule.transaction().splitCount() >= 3)
      m_category->loadText(i18n("Splitted Transaction"));
    else
      m_category->loadText(MyMoneyFile::instance()->instance()->accountToCategory(m_schedule.transaction().split(m_accountId, false).accountId()));
    m_qlineeditMemo->setText(m_schedule.transaction().split(m_accountId).memo());
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
