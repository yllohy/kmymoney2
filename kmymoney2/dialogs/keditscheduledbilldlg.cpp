/***************************************************************************
                          keditscheduledbilldlg.cpp  -  description
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
#include <qlabel.h>
#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kconfig.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <knumvalidator.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "keditscheduledbilldlg.h"
#include "../widgets/kmymoneycombo.h"
#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneypayee.h"
#include "../widgets/kmymoneyedit.h"
#include "../dialogs/ksplittransactiondlg.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneycategory.h"

#include "../widgets/kmymoneypayee.h"

KEditScheduledBillDlg::KEditScheduledBillDlg(const QCString& accountId, QWidget *parent, const char *name)
 : kEditScheduledBillDlgDecl(parent, name, true)
{
  m_accountId = accountId;

  KIntValidator *validator = new KIntValidator(1, 32768, this);
  m_qlineeditRemaining->setValidator(validator);
  
  readConfig();
  reloadFromFile();

  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_qbuttonSplit, SIGNAL(clicked()), this, SLOT(slotSplitClicked()));
  connect(m_qcheckboxEnd, SIGNAL(toggled(bool)), this, SLOT(slotWillEndToggled(bool)));
  connect(m_qbuttonOK, SIGNAL(clicked()), this, SLOT(okClicked()));
}

KEditScheduledBillDlg::~KEditScheduledBillDlg()
{
  writeConfig();
}

void KEditScheduledBillDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_lastPayee = config->readEntry("LastPayee");
}

void KEditScheduledBillDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
//  config->writeEntry("LastPayee", m_kcomboPayTo->currentText());
  config->sync();
}

void KEditScheduledBillDlg::reloadFromFile(void)
{
  m_kcomboMethod->insertItem(i18n("Direct Debit"));
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
    if (*it_s == m_accountId)
    {
      MyMoneyAccount a = file->account(*it_s);
      m_qlabelAccountFrom->setText(a.name());
      return;
    }
  }

  acc = file->liability();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
  {
    if (*it_s == m_accountId)
    {
      MyMoneyAccount a = file->account(*it_s);
      m_qlabelAccountFrom->setText(a.name());
      return;
    }
  }
}

/*
 * Cribbed from : KLedgerViewCheckings::slotOpenSplitDialog(void)
 */
void KEditScheduledBillDlg::slotSplitClicked()
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

void KEditScheduledBillDlg::slotWillEndToggled(bool on)
{
  m_endLabel1->setEnabled(on);
  m_qlineeditRemaining->setEnabled(on);
  m_endLabel2->setEnabled(on);
  m_kdateinputFinal->setEnabled(on);
}

void KEditScheduledBillDlg::reloadWidgets(void)
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

MyMoneySchedule KEditScheduledBillDlg::schedule(void)
{
  return m_schedule;
}

void KEditScheduledBillDlg::okClicked()
{
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
    
  QString occurence = m_kcomboFreq->currentText();
  if (occurence == i18n("Once"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_ONCE);
  else if (occurence = i18n("Daily"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_DAILY);
  else if (occurence = i18n("Weekly"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_WEEKLY);
  else if (occurence = i18n("Fortnightly"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_FORTNIGHTLY);
  else if (occurence = i18n("Every other week"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERWEEK);
  else if (occurence = i18n("Every four weeks"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_EVERYFOURWEEKS);
  else if (occurence = i18n("Monthly"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_MONTHLY);
  else if (occurence = i18n("Every other month"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERMONTH);
  else if (occurence = i18n("Every three months"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_EVERYTHREEMONTHS);
  else if (occurence = i18n("Every four months"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_EVERYFOURMONTHS);
  else if (occurence = i18n("Twice a year"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_TWICEYEARLY);
  else if (occurence = i18n("Quarterly"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_QUARTERLY);
  else if (occurence = i18n("Yearly"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_YEARLY);
  else if (occurence = i18n("Every other year"))
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_EVERYOTHERYEAR);

  QString method = m_kcomboMethod->currentText();
  if (method == i18n("Direct Debit"))
    m_schedule.setPaymentType(MyMoneySchedule::STYPE_DIRECTDEBIT);
  else if (method == i18n("Write Cheque"))
    m_schedule.setPaymentType(MyMoneySchedule::STYPE_WRITECHEQUE);
  else if (method == i18n("Other"))
    m_schedule.setPaymentType(MyMoneySchedule::STYPE_OTHER);

  m_schedule.setType(MyMoneySchedule::TYPE_BILL);

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

  if (m_transaction.splitCount() == 0)
  {
    if (m_kmoneyeditAmount->text() == "")
    {
      KMessageBox::information(this, i18n("Please fill in the amount field"));
      m_kmoneyeditAmount->setFocus();
    }

    MyMoneySplit split1;

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

  } catch (MyMoneyException *e)
  {
    QString s(i18n("Exception in okClicked: "));
    s += e->what();
    delete e;
    KMessageBox::information(this, s);
  }

  accept();
}
