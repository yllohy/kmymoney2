/***************************************************************************
                          kenterscheduledialog.cpp  -  description
                             -------------------
    begin                : Mon Sep 1 2003
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
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kenterscheduledialog.h"
#include "../widgets/kmymoneyedit.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneyutils.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneycategory.h"
#include "../dialogs/ksplittransactiondlg.h"
#include "../dialogs/kconfirmmanualenterdialog.h"

KEnterScheduleDialog::KEnterScheduleDialog(QWidget *parent, const MyMoneySchedule& schedule)
  : kEnterScheduleDialogDecl(parent, "kenterscheduledialog")
{
  m_schedule = schedule;
  initWidgets();

  connect(m_splitButton, SIGNAL(clicked()), this, SLOT(slotSplitClicked()));
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(slotOK()));
}

KEnterScheduleDialog::~KEnterScheduleDialog(){
}

void KEnterScheduleDialog::initWidgets()
{
  m_splitButton->setGuiItem(KMyMoneyUtils::splitGuiItem());
  m_from->loadAccounts(true, false);
  m_to->loadAccounts(true, false);
    
  try
  {
    if (m_schedule.account().name() == "")
      return;

    m_from->setCurrentText(m_schedule.account().name());
    if (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER)
      m_to->setCurrentText(m_schedule.transferAccount().name());
    else
      m_to->setEnabled(false);
      
    m_payee->setText(MyMoneyFile::instance()->payee(m_schedule.transaction().split(m_schedule.account().id()).payeeId()).name());
    m_date->setDate(m_schedule.nextPayment(m_schedule.lastPayment()));

    MyMoneyMoney amount = m_schedule.transaction().split(m_schedule.account().id()).value();
    m_amount->setText(amount.formatMoney());

    if (m_schedule.transaction().splitCount() >= 3)
      m_category->setText(i18n("Splitted Transaction"));
    else
      m_category->setText(MyMoneyFile::instance()->instance()->accountToCategory(m_schedule.transaction().split(m_schedule.account().id(), false).accountId()));

    m_memo->setText(m_schedule.transaction().split(m_schedule.account().id()).memo());

    m_scheduleName->setText(m_schedule.name());

    m_type->setText(KMyMoneyUtils::scheduleTypeToString(m_schedule.type()));
  } catch (MyMoneyException *e)
  {
    KMessageBox::detailedError(this, i18n("Unable to load schedule details"), e->what());
    delete e;
  }
}

void KEnterScheduleDialog::slotOK()
{
  if (checkData())
  {
    // Commit this transaction to file.
    accept();
  }
}

void KEnterScheduleDialog::slotSplitClicked()
{
  bool isDeposit = false;
  bool isValidAmount = false;

  if(m_amount->text().length() != 0)
  {
    isDeposit = false;
    isValidAmount = true;
  }

  try
  {
    MyMoneySplit split1;
    m_transaction = m_schedule.transaction();

    if (m_transaction.splitCount() == 0)
    {
      MyMoneyFile *file = MyMoneyFile::instance();

      QCString payeeId;

      try
      {
        payeeId = file->payeeByName(m_payee->text()).id();
      }
      catch (MyMoneyException *e)
      {
        delete e;
      }

      split1.setAccountId(m_from->currentAccountId());
      if (m_schedule.type() == MyMoneySchedule::TYPE_BILL)
        split1.setAction(MyMoneySplit::ActionWithdrawal);
      else if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
        split1.setAction(MyMoneySplit::ActionDeposit);
      else
        split1.setAction(MyMoneySplit::ActionTransfer);
        
      split1.setPayeeId(payeeId);
      split1.setMemo(m_memo->text());
      split1.setValue(m_amount->getMoneyValue());
      m_transaction.addSplit(split1);

      if (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER)
      {
        MyMoneySplit split2;
        split2.setAccountId(m_to->currentAccountId());
        if (m_schedule.type() == MyMoneySchedule::TYPE_BILL)
          split2.setAction(MyMoneySplit::ActionWithdrawal);
        else if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
          split2.setAction(MyMoneySplit::ActionDeposit);
        else
          split2.setAction(MyMoneySplit::ActionTransfer);
          
        split2.setPayeeId(split1.payeeId());
        split2.setMemo(split1.memo());
        split2.setValue(-split1.value());
        m_transaction.addSplit(split2);
      }
    }

    MyMoneyAccount acc = MyMoneyFile::instance()->account(m_from->currentAccountId());

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

      m_amount->setFocus();
    }

    reloadWidgets();

    delete dlg;
  } catch (MyMoneyException *e)
  {
//    KMessageBox::detailedError(this, i18n("Exception in slot split clicked"), e->what());
    delete e;
  }
}

void KEnterScheduleDialog::reloadWidgets(void)
{
  QString category;
  try {
    MyMoneySplit s;
    switch(m_transaction.splitCount()) {
      case 2:
        s = m_transaction.split(m_from->currentAccountId(), false);
        category = MyMoneyFile::instance()->accountToCategory(s.accountId());
        break;
      case 1:
        category = "";
        m_transaction.removeSplits();
        break;
      default:
        category = i18n("Splitted transaction");
        break;
    }
  } catch (MyMoneyException *e) {
    delete e;
    category = "";
  }

  m_category->setText(category);

  MyMoneySplit split = m_transaction.split(m_from->currentAccountId());
  MyMoneyMoney amount(split.value());
  if (amount < 0)
    amount = -amount;
  m_amount->setText(amount.formatMoney());
}

bool KEnterScheduleDialog::checkData(void)
{
  QString messageDetail;
  int noItemsChanged=0;
  QString payeeName;

  try
  {
    payeeName = MyMoneyFile::instance()->payee(m_schedule.transaction().split(
      m_schedule.account().id()).payeeId()).name();
  }
  catch (MyMoneyException *e)
  {
    delete e;
  }

  if (m_payee->text() != payeeName)
  {
    noItemsChanged++;
    messageDetail += QString(i18n("<font size=\"-1\">Payee changed.  Old: %1, New: %2</font><br>")).arg(payeeName).arg(m_payee->text());
  }

  if (  (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER ||
        m_schedule.type() == MyMoneySchedule::TYPE_BILL) &&
        m_from->currentText() != m_schedule.account().name())
  {
    noItemsChanged++;
    messageDetail += QString(i18n("<font size=\"-1\">Account changed.  Old: %1, New: %2</font><br>")).arg(m_schedule.account().name()).arg(m_from->currentText());
  }

  if (  m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT ||
        m_to->currentText() != m_schedule.account().name())
  {
    noItemsChanged++;
    messageDetail += QString(i18n("<font size=\"-1\">Deposit account changed.  Old: %1, New: %2</font><br>")).arg(m_schedule.account().name()).arg(m_to->currentText());
  }

  if (  m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER &&
        m_to->currentText() != m_schedule.transferAccount().name())
  {
    noItemsChanged++;
    messageDetail += QString(i18n("<font size=\"-1\">Transfer to account changed.  Old: %1, New: %2</font><br>")).arg(m_schedule.transferAccount().name()).arg(m_to->currentText());
  }

  QString category;
  if (m_schedule.transaction().splitCount() >= 3)
    category = i18n("Splitted Transaction");
  else
    category = MyMoneyFile::instance()->instance()->accountToCategory(m_schedule.transaction().split(m_schedule.account().id(), false).accountId());
  if (category != m_category->text())
  {
    noItemsChanged++;
    messageDetail += QString(i18n("<font size=\"-1\">Category changed.  Old: %1, New: %2</font><br>")).arg(category).arg(m_category->text());
  }
  
  QString memo = m_schedule.transaction().split(m_schedule.account().id()).memo();
  if (memo != m_memo->text())
  {
    noItemsChanged++;
    messageDetail += QString(i18n("<font size=\"-1\">Memo changed.  Old: %1, New: %2</font><br>")).arg(memo).arg(m_memo->text());
  }

  MyMoneyMoney amount = m_schedule.transaction().split(m_schedule.account().id()).value();
  if (amount != m_amount->getMoneyValue())
  {
    noItemsChanged++;
    messageDetail += QString(i18n("<font size=\"-1\">Amount changed.  Old: %1, New: %2</font><br>")).arg(amount.formatMoney()).arg(m_amount->getMoneyValue().formatMoney());
  }
  
  if (noItemsChanged > 0)
  {
    QString header = QString(i18n("<b><font size=\"+1\"><p>%1 items of the details for the transaction have changed.</p></font></b>")).arg(noItemsChanged);
    KConfirmManualEnterDialog dlg(this, "kconfirmmanualenterdlg");
    dlg.m_message->setText(header);
    dlg.m_details->setText(messageDetail);
    dlg.m_onceRadio->setChecked(true);
    if (dlg.exec())
    {
      return true;
    }
  }

  return false;
}
