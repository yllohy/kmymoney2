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
#include "../mymoney/mymoneyfinancialcalculator.h"
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

  // initWidgets must be called before calculateInterst, because
  // the internals of initWidgets require the unmodified splits
  // with the setting of automatically calculated values
  initWidgets();
  calculateInterest();

  connect(m_splitButton, SIGNAL(clicked()), this, SLOT(slotSplitClicked()));
  connect(m_buttonOk, SIGNAL(clicked()), this, SLOT(slotOK()));
  connect(m_from, SIGNAL(activated(int)),
    this, SLOT(slotFromActivated(int)));
  connect(m_to, SIGNAL(activated(int)),
    this, SLOT(slotToActivated(int)));
  connect(m_date, SIGNAL(dateChanged(const QDate&)), this, SLOT(checkDateInPeriod(const QDate&)));
}

KEnterScheduleDialog::~KEnterScheduleDialog()
{
}

void KEnterScheduleDialog::initWidgets()
{
  kMyMoneyCombo* loanAccount = 0;

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
        s2.setId(QCString());
        s1.setId(QCString());
        transaction.addSplit(s2);
        transaction.addSplit(s1);
        m_schedule.setTransaction(transaction);
        m_transaction = transaction;
      }
    }
    else if(m_schedule.type() != MyMoneySchedule::TYPE_LOANPAYMENT)
    {
      if (s1.value() > 0)
      {
        transaction.removeSplits();
        s2.setId(QCString());
        s1.setId(QCString());
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
  m_buttonOk->setGuiItem(KStdGuiItem::ok());
  m_buttonCancel->setGuiItem(KStdGuiItem::cancel());
  m_buttonHelp->setGuiItem(KStdGuiItem::help());

  if (m_schedule.type() != MyMoneySchedule::TYPE_DEPOSIT
  &&  m_schedule.type() != MyMoneySchedule::TYPE_LOANPAYMENT)
    m_from->loadAccounts(true, false);

  if (m_schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT)
  {
    MyMoneySplit amortizationSplit;
    QValueList<MyMoneySplit>::ConstIterator it_s;
    for(it_s = m_transaction.splits().begin(); it_s != m_transaction.splits().end(); ++it_s) {
      if((*it_s).value() == MyMoneyMoney::minValue+1
      && (*it_s).action() == MyMoneySplit::ActionAmortization) {
        MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
        switch(acc.accountType()) {
          case MyMoneyAccount::Loan:
            m_from->setEnabled(true);
            m_to->setEnabled(false);
            m_from->loadAccounts(true, false);
            loanAccount = m_from;
            break;

          case MyMoneyAccount::AssetLoan:
            m_from->setEnabled(false);
            m_to->setEnabled(true);
            m_to->loadAccounts(true, false);
            loanAccount = m_to;
            break;

          default:
            qFatal("invalid account type in %s:%d", __FILE__, __LINE__);
            break;
        }
        break;
      }
    }
  }
  else if (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER
       ||  m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
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
    if (m_schedule.account().name().isEmpty())
      return;

    if (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER)
    {
      m_from->setCurrentText(m_schedule.account().name());
      m_to->setCurrentText(/*m_schedule.transferAccount().name()*/
        MyMoneyFile::instance()->account(m_schedule.transaction().splits()[1].accountId()).name());
    }
    else if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
      m_to->setCurrentText(m_schedule.account().name());

    else if (m_schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT)
    {
      loanAccount->setCurrentText(m_schedule.account().name());
    }
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
        m_category->setText(i18n("Split Transaction"));
        connect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
      }
      else
        m_category->setText(MyMoneyFile::instance()->accountToCategory(m_transaction.splitByAccount(m_schedule.account().id(), false).accountId()));
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
    else if(m_schedule.type() != MyMoneySchedule::TYPE_LOANPAYMENT)
      isDeposit = false;
    else {
      isDeposit = m_to->isEnabled();
    }
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
                                                         0,
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
          category = QString();
          m_transaction.removeSplits();
          disconnect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
          break;
        case 0:
          disconnect(m_category, SIGNAL(signalFocusIn()), this, SLOT(slotSplitClicked()));
          break;
        default:
          category = i18n("Split Transaction");
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

  if (!checkDateInPeriod(m_date->getQDate()))
    return false;

  try
  {
    payeeName = MyMoneyFile::instance()->payee(m_schedule.transaction().splitByAccount(
      m_schedule.account().id()).payeeId()).name();

    if (m_payee->text() != payeeName)
    {
      noItemsChanged++;
      messageDetail += QString(i18n("Payee changed.  Old: \"%1\", New: \"%2\""))
        .arg(payeeName).arg(m_payee->text()) + QString("\n");
    }

    if (  (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER ||
          m_schedule.type() == MyMoneySchedule::TYPE_BILL) &&
          m_from->currentText() != m_schedule.account().name())
    {
      noItemsChanged++;
      messageDetail += QString(i18n("Account changed.  Old: \"%1\", New: \"%2\""))
        .arg(m_schedule.account().name()).arg(m_from->currentText()) + QString("\n");
    }

    if (  m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT &&
          m_to->currentText() != m_schedule.account().name())
    {
      noItemsChanged++;
      messageDetail += QString(i18n("Account changed.  Old: \"%1\", New: \"%2\""))
        .arg(m_schedule.account().name()).arg(m_to->currentText()) + QString("\n");
    }

    if (  m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER &&
          m_to->currentText() != MyMoneyFile::instance()->account(m_schedule.transaction().splits()[1].accountId()).name())
    {
      noItemsChanged++;
      messageDetail += QString(i18n("Transfer account changed.  Old: \"%1\", New: \"%2\""))
        .arg(MyMoneyFile::instance()->account(m_schedule.transaction().splits()[1].accountId()).name())
        .arg(m_to->currentText()) + QString("\n");
    }

    if (m_schedule.type() != MyMoneySchedule::TYPE_TRANSFER)
    {
      QString category;
      if (m_schedule.transaction().splitCount() >= 3)
        category = i18n("Split Transaction");
      else
        category = MyMoneyFile::instance()->accountToCategory(m_schedule.transaction()
          .splitByAccount(m_schedule.account().id(), false).accountId());

      if (category != m_category->text())
      {
        noItemsChanged++;
        messageDetail += QString(i18n("Category changed.  Old: \"%1\", New: \"%2\""))
          .arg(category).arg(m_category->text()) + QString("\n");
      }
    }

    QString memo = m_schedule.transaction().splitByAccount(m_schedule.account().id()).memo();
    if (!memo.isEmpty() || !m_memo->text().isEmpty())
    {
      if(memo != m_memo->text())
      {
        noItemsChanged++;
        messageDetail += QString(i18n("Memo changed.  Old: \"%1\", New: \"%2\""))
          .arg(memo).arg(m_memo->text()) + QString("\n");
      }
    }

    MyMoneyMoney amount = m_schedule.transaction().splitByAccount(m_schedule.account().id()).value();
    if (amount < 0)
      amount = -amount;
    if (amount != m_amount->getMoneyValue())
    {
      noItemsChanged++;
      messageDetail += QString(i18n("Amount changed.  Old: \"%1\", New: \"%2\""))
        .arg(amount.formatMoney()).arg(m_amount->getMoneyValue().formatMoney()) + QString("\n");
    }

    if (noItemsChanged > 0)
    {
      QString header = QString("\n") +
        i18n("%1 items of the details for the transaction have changed.")
        .arg(noItemsChanged);

      KConfirmManualEnterDialog dlg(this, "kconfirmmanualenterdlg");
      dlg.m_message->setText(header);
      dlg.m_details->setText(messageDetail);
      dlg.m_onceRadio->setChecked(true);

      // for loan payments, we do not allow to override the
      // stored transaction as this would destroy the automatic
      // amortization/interest calculation. Also, we cannot use
      // the stored transaction directly. So for now, we don't
      // allow these two options.
      if(m_schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT)
      {
        dlg.m_SetRadio->setEnabled(false);
        dlg.m_discardRadio->setEnabled(false);
      }

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
  if (m_category->text() != i18n("Split Transaction") &&
      m_schedule.type() != MyMoneySchedule::TYPE_TRANSFER)
  {
    QString category = m_category->text();
    QCString id = MyMoneyFile::instance()->categoryToAccount(category);
    if(id.isEmpty() && !category.isEmpty())
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
        m_category->setText(QString());
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
  QDate realLastPayment;

  try
  {
    m_schedDate = m_date->getQDate();
    if (!m_schedDate.isValid())
      m_schedDate = m_schedule.nextPayment(m_schedule.lastPayment());

    realLastPayment = m_schedDate;

    if (m_schedDate != m_schedule.nextPayment(m_schedule.lastPayment()))
    {
      if (m_schedDate > m_schedule.lastPayment() && m_schedDate < m_schedule.nextPayment(m_schedule.lastPayment()))
      {
        // just replace the date
        realLastPayment = m_schedule.nextPayment(m_schedule.lastPayment());
      }
      //else
      //out of order dates will be handled below
    }

    if (m_schedDate > m_schedule.nextPayment(m_schedule.lastPayment()))
    {
      realLastPayment = m_schedDate;

      if (m_schedDate < m_schedule.nextPayment(m_schedule.nextPayment(m_schedule.lastPayment())))
        realLastPayment = m_schedule.nextPayment(m_schedule.lastPayment());

      QString message = QString(i18n("Some occurences that are older than '%1' have not been entered yet.\n\nDelete all occurences that have not been entered before this date?")).arg(m_schedDate.toString());

      if (realLastPayment != m_schedule.nextPayment(m_schedule.lastPayment()) &&
          KMessageBox::warningYesNo(this, message) == KMessageBox::No)
      {
        m_schedule.recordPayment(realLastPayment);
      }
      else
      {
        m_schedule.setLastPayment(realLastPayment);
      }
    }
    else if (m_schedDate > QDate::currentDate())
    {
      // FIXME: we should probably respect the configurable pre-enter period
      QString message = QString(i18n("Are you sure you want to enter this occurence which is %1 days after today?")).arg(QDate::currentDate().daysTo(m_schedDate));
      if (KMessageBox::warningYesNo(this, message) == KMessageBox::No)
        return;

      if (m_schedDate > m_schedule.nextPayment(m_schedule.lastPayment()))
        m_schedule.recordPayment(m_schedDate);
      else
        m_schedule.setLastPayment(realLastPayment);
    }
    else
    {
      m_schedule.setLastPayment(realLastPayment);
    }

    if (m_schedule.weekendOption() != MyMoneySchedule::MoveNothing)
    {
      int dayOfWeek = m_schedDate.dayOfWeek();
      if (dayOfWeek >= 6)
      {
        if (m_schedule.weekendOption() == MyMoneySchedule::MoveFriday)
        {
          if (dayOfWeek == 7)
            m_schedDate = m_schedDate.addDays(-2);
          else
            m_schedDate = m_schedDate.addDays(-1);
        }
        else
        {
          if (dayOfWeek == 6)
            m_schedDate = m_schedDate.addDays(2);
          else
            m_schedDate = m_schedDate.addDays(1);
        }
      }
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
      else if(m_schedule.type() != MyMoneySchedule::TYPE_LOANPAYMENT)
        s.setValue(-m_amount->getMoneyValue());

      m_transaction.modifySplit(s);

      // if we have exactly two splits, we can update the 'other' side
      // of the transaction. If we have more than two splits, then
      // we need to check before we enter if the sum of the splits
      // equals 0 and if not warn the user to modify the splits.
      if(m_transaction.splitCount() == 2)
      {
        MyMoneySplit s2 = m_transaction.splits()[1];
        s2.setValue(-s.value());
        m_transaction.modifySplit(s2);
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
  if( m_schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
    if(m_from->isEnabled())
      return m_from->currentAccountId();
    if(m_to->isEnabled())
      return m_to->currentAccountId();
    qFatal("This should never happen %s:%d", __FILE__, __LINE__);
  }

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

void KEnterScheduleDialog::calculateInterest(void)
{
  if (m_schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
    MyMoneySplit interestSplit, amortizationSplit;
    QValueList<MyMoneySplit>::ConstIterator it_s;
    for(it_s = m_transaction.splits().begin(); it_s != m_transaction.splits().end(); ++it_s) {
      if((*it_s).value() == MyMoneyMoney::minValue+1) {
        if((*it_s).action() == MyMoneySplit::ActionAmortization) {
          amortizationSplit = (*it_s);
        } else if((*it_s).action() == MyMoneySplit::ActionInterest) {
          interestSplit = (*it_s);
        }
      }
    }

    if(!amortizationSplit.id().isEmpty() && !interestSplit.id().isEmpty()) {
      MyMoneyAccountLoan acc(MyMoneyFile::instance()->account(amortizationSplit.accountId()));
      MyMoneyFinancialCalculator calc;
      QDate dueDate;

      // FIXME: setup dueDate according to when the interest should be calculated
      // current implementation: take the date of the next payment according to
      // the schedule. If the calculation is based on the payment reception, and
      // the payment is overdue then take the current date
      dueDate = m_schedule.nextPayment(m_schedule.lastPayment());
      if(acc.interestCalculation() == MyMoneyAccountLoan::paymentReceived) {
        if(dueDate < QDate::currentDate())
          dueDate = QDate::currentDate();
      }

      // we need to calculate the balance at the time the payment is due
      MyMoneyMoney balance = MyMoneyFile::instance()->balance(acc.id(), dueDate.addDays(-1));

/*
      QValueList<MyMoneyTransaction> list;
      QValueList<MyMoneyTransaction>::ConstIterator it;
      MyMoneySplit split;
      MyMoneyTransactionFilter filter(acc.id());

      filter.setDateFilter(QDate(), dueDate.addDays(-1));
      list = MyMoneyFile::instance()->transactionList(filter);

      for(it = list.begin(); it != list.end(); ++it) {
        try {
          split = (*it).splitByAccount(acc.id());
          balance += split.value();

        } catch(MyMoneyException *e) {
          // account is not referenced within this transaction
          delete e;
        }
      }
*/

      // FIXME: for now, we only support interest calculation at the end of the period
      calc.setBep();
      // FIXME: for now, we only support periodic compounding
      calc.setDisc();

      calc.setPF(KMyMoneyUtils::occurenceToFrequency(m_schedule.occurence()));
      // FIXME: for now we only support compounding frequency == payment frequency
      calc.setCF(KMyMoneyUtils::occurenceToFrequency(m_schedule.occurence()));

      calc.setPv(balance.toDouble());
      calc.setIr(static_cast<FCALC_DOUBLE> (acc.interestRate(dueDate).abs().toDouble()) / 100.0);
      calc.setPmt(acc.periodicPayment().toDouble());

      MyMoneyMoney interest(calc.interestDue()/100.0), amortization;
      interest = interest.abs();    // make sure it's positive for now
      amortization = acc.periodicPayment() - interest;

      if(acc.accountType() == MyMoneyAccount::AssetLoan) {
        interest = -interest;
        amortization = -amortization;
      }
      amortizationSplit.setValue(amortization);
      interestSplit.setValue(interest);

      m_transaction.modifySplit(amortizationSplit);
      m_transaction.modifySplit(interestSplit);
    }
  }
}

bool KEnterScheduleDialog::checkDateInPeriod(const QDate& date)
{
  QDate firstDate;
  QDate nextDate = m_schedule.nextPayment(m_schedule.lastPayment());
  switch (m_schedule.occurence())
  {
    case MyMoneySchedule::OCCUR_ONCE:
      firstDate = nextDate;
      break;
    case MyMoneySchedule::OCCUR_DAILY:
      firstDate = nextDate.addDays(-1);
      break;
    case MyMoneySchedule::OCCUR_WEEKLY:
      firstDate = nextDate.addDays(-7);
      break;
    case MyMoneySchedule::OCCUR_FORTNIGHTLY:
      firstDate = nextDate.addDays(-14);
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURWEEKS:
      firstDate = nextDate.addDays(-28);
      break;
    case MyMoneySchedule::OCCUR_MONTHLY:
      firstDate = nextDate.addMonths(-1);
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERMONTH:
      firstDate = nextDate.addMonths(-2);
      break;
    case MyMoneySchedule::OCCUR_EVERYTHREEMONTHS:
      firstDate = nextDate.addMonths(-3);
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURMONTHS:
      firstDate = nextDate.addMonths(-4);
      break;
    case MyMoneySchedule::OCCUR_TWICEYEARLY:
      firstDate = nextDate.addMonths(-6);
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
      firstDate = nextDate.addYears(-1);
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERYEAR:
      firstDate = nextDate.addYears(-2);
      break;
    default:
      break;
  }

  if (date < firstDate || date > nextDate)
  {
    KMessageBox::error(this, i18n("The date must lie in the range %1 to %2").arg(firstDate.addDays(1).toString()).arg(nextDate.toString()));
    return false;
  }

  return true;
}
