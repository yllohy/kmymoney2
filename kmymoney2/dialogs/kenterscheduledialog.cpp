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

KEnterScheduleDialog::KEnterScheduleDialog(QWidget *parent, const MyMoneySchedule& schedule,
  const QDate& date)
  : kEnterScheduleDialogDecl(parent, "kenterscheduledialog"), m_schedDate(date)
{
  m_schedule = schedule;
  m_transaction = schedule.transaction();
  initWidgets();

  connect(m_splitButton, SIGNAL(clicked()), this, SLOT(slotSplitClicked()));
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(slotOK()));
  connect(m_from, SIGNAL(activated(int)),
    this, SLOT(slotFromActivated(int)));
  connect(m_to, SIGNAL(activated(int)),
    this, SLOT(slotToActivated(int)));
}

KEnterScheduleDialog::~KEnterScheduleDialog(){
}

void KEnterScheduleDialog::initWidgets()
{
  // Work around backwards transfers
  try
  {
    MyMoneyTransaction transaction = m_schedule.transaction();
    MyMoneySplit s1 = transaction.splits()[0];
    MyMoneySplit s2 = transaction.splits()[1];

    if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
    {
      if (s1.value() < 0)
      {
        transaction.removeSplits();
        s2.setId("");
        s1.setId("");
        transaction.addSplit(s2);
        transaction.addSplit(s1);
        m_schedule.setTransaction(transaction);
        m_transaction = transaction;
      }
    }
    else
    {
      if (s1.value() > 0)
      {
        transaction.removeSplits();
        s2.setId("");
        s1.setId("");
        transaction.addSplit(s2);
        transaction.addSplit(s1);
        m_schedule.setTransaction(transaction);
        m_transaction = transaction;
      }
    }
  }
  catch (MyMoneyException *e)
  {
    delete e;
  }

  m_splitButton->setGuiItem(KMyMoneyUtils::splitGuiItem());

  if (m_schedule.type() != MyMoneySchedule::TYPE_DEPOSIT)
    m_from->loadAccounts(true, false);
    
  if (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER ||
      m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
  {
    if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
      m_from->setEnabled(false);
    else
    {
      m_category->setEnabled(false);
      m_splitButton->setEnabled(false);
    }
      
    m_to->setEnabled(true);
    m_to->loadAccounts(true, false);
  }
    
  try
  {
    if (m_schedule.account().name() == "")
      return;


    if (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER)
    {
      m_from->setCurrentText(m_schedule.account().name());
      m_to->setCurrentText(/*m_schedule.transferAccount().name()*/
        MyMoneyFile::instance()->account(m_schedule.transaction().splits()[1].accountId()).name());
    }
    else if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
      m_to->setCurrentText(m_schedule.account().name());
    else
    {
      m_from->setCurrentText(m_schedule.account().name());
      m_to->setEnabled(false);
    }
      
    m_payee->setText(MyMoneyFile::instance()->payee(m_transaction.splitByAccount(m_schedule.account().id()).payeeId()).name());
    if (m_schedDate.isValid())
      m_date->setDate(m_schedDate);
    else
      m_date->setDate(m_schedule.nextPayment(m_schedule.lastPayment()));

    MyMoneyMoney amount = m_transaction.splitByAccount(m_schedule.account().id()).value();
    if (amount < 0)
      amount = -amount;
    m_amount->setText(amount.formatMoney());

    if (m_schedule.type() != MyMoneySchedule::TYPE_TRANSFER)
    {
      if (m_transaction.splitCount() >= 3)
      {
        m_category->setText(i18n("Splitted Transaction"));
        connect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
      }
      else
        m_category->setText(MyMoneyFile::instance()->instance()->accountToCategory(m_transaction.splitByAccount(m_schedule.account().id(), false).accountId()));
    }

    m_memo->setText(m_transaction.splitByAccount(m_schedule.account().id()).memo());

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
    commitTransaction();
    accept();
  }
}

void KEnterScheduleDialog::slotSplitClicked()
{
  bool isDeposit = false;
  bool isValidAmount = false;

  if(m_amount->text().length() != 0)
  {
    if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
      isDeposit = true;
    else
      isDeposit = false;
    isValidAmount = true;
  }

  try
  {
    MyMoneySplit split1;

    if (m_transaction.splitCount() == 0)
    {
      createSplits();
    }
    else
    {
      // the amount might have changed
      // set it just in case, same for memo etc
      setPayee();
      setMemo();
      setAmount();
    }

    checkCategory();

    m_category->blockSignals(true);

    MyMoneyAccount acc = MyMoneyFile::instance()->account(theAccountId());

    KSplitTransactionDlg* dlg = new KSplitTransactionDlg(m_transaction,
                                                         acc,
                                                         isValidAmount,
                                                         isDeposit,
                                                         this);

    // Avoid focusIn() events.
    m_memo->setFocus();

    if(dlg->exec())
    {
      m_transaction = dlg->transaction();

      MyMoneySplit s;
      QString category;

      switch(m_transaction.splitCount())
      {
        case 2:
          s = m_transaction.splitByAccount(theAccountId(), false);
          category = MyMoneyFile::instance()->accountToCategory(s.accountId());
          disconnect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
          break;
        case 1:
          category = "";
          m_transaction.removeSplits();
          disconnect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
          break;
        case 0:
          disconnect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
          break;
        default:
          category = i18n("Splitted Transaction");
          connect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
          break;
      }

      m_category->setText(category);

      MyMoneySplit split = m_transaction.splitByAccount(theAccountId());
      MyMoneyMoney amount(split.value());
      if (amount < 0)
        amount = -amount;
      m_amount->setText(amount.formatMoney());
    }

    delete dlg;
  } catch (MyMoneyException *e)
  {
//    KMessageBox::detailedError(this, i18n("Exception in slot split clicked"), e->what());
    delete e;
  }

  m_category->blockSignals(false);
}

bool KEnterScheduleDialog::checkData(void)
{
  QString messageDetail;
  int noItemsChanged=0;
  QString payeeName;

  if (m_from->currentText() == m_to->currentText())
  {
    KMessageBox::error(this, i18n("Account and transfer account are the same.  Please change one."));
    m_from->setFocus();
    return false;
  }

  try
  {
    payeeName = MyMoneyFile::instance()->payee(m_schedule.transaction().splitByAccount(
      m_schedule.account().id()).payeeId()).name();

    if (m_payee->text() != payeeName)
    {
      noItemsChanged++;
      messageDetail += QString(i18n("<font size=\"-1\">Payee changed.  Old: %1, New: %2</font><br>"))
        .arg(payeeName).arg(m_payee->text());
    }

    if (  (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER ||
          m_schedule.type() == MyMoneySchedule::TYPE_BILL) &&
          m_from->currentText() != m_schedule.account().name())
    {
      noItemsChanged++;
      messageDetail += QString(i18n("<font size=\"-1\">Account changed.  Old: %1, New: %2</font><br>"))
        .arg(m_schedule.account().name()).arg(m_from->currentText());
    }

    if (  m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT &&
          m_to->currentText() != m_schedule.account().name())
    {
      noItemsChanged++;
      messageDetail += QString(i18n("<font size=\"-1\">Account changed.  Old: %1, New: %2</font><br>"))
        .arg(m_schedule.account().name()).arg(m_to->currentText());
    }

    if (  m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER &&
          m_to->currentText() != MyMoneyFile::instance()->account(m_schedule.transaction().splits()[1].accountId()).name())
    {
      noItemsChanged++;
      messageDetail += QString(i18n("<font size=\"-1\">Transfer account changed.  Old: %1, New: %2</font><br>"))
        .arg(MyMoneyFile::instance()->account(m_schedule.transaction().splits()[1].accountId()).name())
        .arg(m_to->currentText());
    }

    if (m_schedule.type() != MyMoneySchedule::TYPE_TRANSFER)
    {
      QString category;
      if (m_schedule.transaction().splitCount() >= 3)
        category = i18n("Splitted Transaction");
      else
        category = MyMoneyFile::instance()->instance()->accountToCategory(m_schedule.transaction()
          .splitByAccount(m_schedule.account().id(), false).accountId());

      if (category != m_category->text())
      {
        noItemsChanged++;
        messageDetail += QString(i18n("<font size=\"-1\">Category changed.  Old: %1, New: %2</font><br>"))
          .arg(category).arg(m_category->text());
      }
    }

    QString memo = m_schedule.transaction().splitByAccount(m_schedule.account().id()).memo();
    if (memo != m_memo->text())
    {
      noItemsChanged++;
      messageDetail += QString(i18n("<font size=\"-1\">Memo changed.  Old: %1, New: %2</font><br>"))
        .arg(memo).arg(m_memo->text());
    }

    MyMoneyMoney amount = m_schedule.transaction().splitByAccount(m_schedule.account().id()).value();
    if (amount < 0)
      amount = -amount;
    if (amount != m_amount->getMoneyValue())
    {
      noItemsChanged++;
      messageDetail += QString(i18n("<font size=\"-1\">Amount changed.  Old: %1, New: %2</font><br>"))
        .arg(amount.formatMoney()).arg(m_amount->getMoneyValue().formatMoney());
    }

    if (noItemsChanged > 0)
    {
      QString header = QString(
        i18n("<b><font size=\"+1\"><p>%1 items of the details for the transaction have changed.</p></font></b>"))
        .arg(noItemsChanged);
        
      KConfirmManualEnterDialog dlg(this, "kconfirmmanualenterdlg");
      dlg.m_message->setText(header);
      dlg.m_details->setText(messageDetail);
      dlg.m_onceRadio->setChecked(true);
      if (dlg.exec())
      {
        if (dlg.m_onceRadio->isChecked())
        {
          setPayee();
          setTo();
          setFrom();
          setCategory();
          setMemo();
          setAmount();
        }
        else if (dlg.m_discardRadio->isChecked())
        {
          m_transaction = m_schedule.transaction();
        }
        else if (dlg.m_SetRadio->isChecked())
        {
          try
          {
            setPayee();
            setTo();
            setFrom();
            setCategory();
            setMemo();
            setAmount();
            m_schedule.setTransaction(m_transaction);
            MyMoneyFile::instance()->modifySchedule(m_schedule);
          }
          catch (MyMoneyException *e)
          {
            KMessageBox::error(this, i18n("Unable to modify schedule: ") + e->what());
            delete e;
            return false;
          }
        }
        
        return true;
      }
    }
    else
    {
      return true;
    }
  }
  catch (MyMoneyException *e)
  {
    KMessageBox::error(this, i18n("Fatal error in determining data: ") + e->what());
    delete e;
  }

  return false;
}

void KEnterScheduleDialog::checkCategory()
{
  if (m_category->text() != i18n("Splitted Transaction") &&
      m_schedule.type() != MyMoneySchedule::TYPE_TRANSFER)
  {
    QString category = m_category->text();
    QCString id = MyMoneyFile::instance()->categoryToAccount(category);
    if(id == "" && category != "")
    {
      // Create the category
      QString message = QString("The category '%1' does not exist.  Create?").arg(m_category->text());
      if (KMessageBox::questionYesNo(this, message) == KMessageBox::Yes)
      {
        MyMoneyAccount base;
        if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
          base = MyMoneyFile::instance()->income();
        else
          base = MyMoneyFile::instance()->expense();

        id = MyMoneyFile::instance()->createCategory(base, m_category->text());
      }
      else
      {
        m_category->setText("");
        m_category->setFocus();
        return;
      }
    }

    // Modify the split
    MyMoneySplit s = m_transaction.splits()[1];
    s.setAccountId(id);
    m_transaction.modifySplit(s);
  }
}

void KEnterScheduleDialog::commitTransaction()
{
  try
  {
    if (!m_schedDate.isValid())
      m_schedDate = m_schedule.nextPayment(m_schedule.lastPayment());

    if (m_schedDate > m_schedule.nextPayment(m_schedule.lastPayment()))
    {
      QString message = QString(i18n("Some occurences that are older than '%1' have not been entered yet.\n\nDelete all occurences that have not been entered before this date?")).arg(m_schedDate.toString());
      if (KMessageBox::warningYesNo(this, message) == KMessageBox::No)
        m_schedule.recordPayment(m_schedDate);
      else
        m_schedule.setLastPayment(m_schedDate);   
    }        
    else if (m_schedDate > QDate::currentDate())
    {
      QString message = QString(i18n("Are you sure you want to enter this occurence which is %1 days after today?")).arg(QDate::currentDate().daysTo(m_schedDate));
      if (KMessageBox::warningYesNo(this, message) == KMessageBox::No)
        return;

      if (m_schedDate > m_schedule.nextPayment(m_schedule.lastPayment()))
        m_schedule.recordPayment(m_schedDate);
      else
        m_schedule.setLastPayment(m_schedDate);
    }
    else
    {
      m_schedule.setLastPayment(m_schedDate);
    }

    m_transaction.setEntryDate(QDate::currentDate());
    m_transaction.setPostDate(m_schedDate);
    MyMoneyFile::instance()->addTransaction(m_transaction);

    try
    {
      MyMoneyFile::instance()->modifySchedule(m_schedule);
    }
    catch (MyMoneyException *e)
    {
      KMessageBox::error(this, i18n("Unable to modify schedule: ") + e->what());
      delete e;
    }
  }
  catch (MyMoneyException *e)
  {
    KMessageBox::error(this, i18n("Unable to add transaction: ") + e->what());
    delete e;
  }
}

void KEnterScheduleDialog::setPayee()
{
  QCString payeeId;
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
  MyMoneySplit s = m_transaction.splits()[0];
  s.setPayeeId(payeeId);
  m_transaction.modifySplit(s);
}

void KEnterScheduleDialog::setTo()
{
  if (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER ||
      m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
  {
    int count = m_transaction.splitCount();
    if (count == 0)
    {
      createSplits();
    }
    int id;
    if (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER)
      id = 1;
    else
      id = 0;
    MyMoneySplit s = m_transaction.splits()[id];
    s.setAccountId(m_to->currentAccountId());
    m_transaction.modifySplit(s);
  }
}

void KEnterScheduleDialog::setFrom()
{
  if (m_schedule.type() != MyMoneySchedule::TYPE_DEPOSIT)
  {
    int count = m_transaction.splitCount();
    if (count == 0)
    {
      createSplits();
    }

    MyMoneySplit s = m_transaction.splits()[0];
    s.setAccountId(m_from->currentAccountId());
    m_transaction.modifySplit(s);
  }
}

void KEnterScheduleDialog::setCategory()
{
  checkCategory();
}

void KEnterScheduleDialog::setMemo()
{
  int count = m_transaction.splitCount();
  if (count == 0)
  {
    createSplits();
  }

  MyMoneySplit s = m_transaction.splits()[0];
  s.setMemo(m_memo->text());
  m_transaction.modifySplit(s);

  if (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER)
  {
    s = m_transaction.splits()[1];
    s.setMemo(m_memo->text());
    m_transaction.modifySplit(s);
  }
}

void KEnterScheduleDialog::setAmount()
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
    if (m_amount->getMoneyValue() != amount)
    {
      if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
        s.setValue(m_amount->getMoneyValue());
      else
        s.setValue(-m_amount->getMoneyValue());
      m_transaction.modifySplit(s);

      if (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER)
      {
        MyMoneySplit s2 = m_transaction.splits()[1];
        s2.setValue(-s.value());
        m_transaction.modifySplit(s2);
      }
      else
      {
        if (count >= 3)
        {
          KMessageBox::information(this, i18n("All split data lost.  Please re-enter splits"));
          disconnect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
          m_transaction.removeSplits();
          m_category->setText("");
          m_category->setFocus();
        }
        else  // Must be two
        {
          MyMoneySplit s2 = m_transaction.splits()[1];
          s2.setValue(-s.value());
          m_transaction.modifySplit(s2);
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

void KEnterScheduleDialog::createSplits()
{
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

    MyMoneySplit split1;
    split1.setAccountId(theAccountId());
    
    if (m_schedule.type() == MyMoneySchedule::TYPE_BILL)
      split1.setAction(MyMoneySplit::ActionWithdrawal);
    else if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
      split1.setAction(MyMoneySplit::ActionDeposit);
    else
      split1.setAction(MyMoneySplit::ActionTransfer);

    split1.setPayeeId(payeeId);
    split1.setMemo(m_memo->text());
    
    if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
      split1.setValue(m_amount->getMoneyValue());
    else
      split1.setValue(-m_amount->getMoneyValue());
      
    m_transaction.addSplit(split1);

    
    MyMoneySplit split2;
    if (m_schedule.type() != MyMoneySchedule::TYPE_TRANSFER)
    {
      checkCategory();
    }
    else
      split2.setAccountId(m_to->currentAccountId());

    split2.setPayeeId(split1.payeeId());
    split2.setMemo(split1.memo());
    split2.setValue(-split1.value());
    
    if (split1.action() == MyMoneySplit::ActionDeposit)
      split2.setAction(MyMoneySplit::ActionWithdrawal);
    else if (split1.action() == MyMoneySplit::ActionWithdrawal)
      split2.setAction(MyMoneySplit::ActionDeposit);
    else
      split2.setAction(MyMoneySplit::ActionTransfer);

    m_transaction.addSplit(split2);
  }
}

QCString KEnterScheduleDialog::theAccountId()
{
  if (m_schedule.type() != MyMoneySchedule::TYPE_DEPOSIT)
    return m_from->currentAccountId();
  else
    return m_to->currentAccountId();
}

void KEnterScheduleDialog::slotFromActivated(int)
{
  if (m_schedule.type() != MyMoneySchedule::TYPE_DEPOSIT)
  {
    // Change the splits because otherwise they wont be found
    // if the user clicks on the split button
    MyMoneySplit s = m_transaction.splits()[0];
    s.setAccountId(m_from->currentAccountId());
    m_transaction.modifySplit(s);
  }
}

void KEnterScheduleDialog::slotToActivated(int)
{
  if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
  {
    // Change the splits because otherwise they wont be found
    // if the user clicks on the split button
    MyMoneySplit s = m_transaction.splits()[0];
    s.setAccountId(m_to->currentAccountId());
    m_transaction.modifySplit(s);
  }
}
