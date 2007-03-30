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
#include <qlayout.h>
#include <qgroupbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <ktextedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kenterscheduledialog.h"
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneyutils.h>
#include <kmymoney/kmymoneyaccountcombo.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneycategory.h>
#include <kmymoney/mymoneyfinancialcalculator.h>
#include "../dialogs/ksplittransactiondlg.h"
#include "../dialogs/kconfirmmanualenterdialog.h"
#include "../dialogs/kcurrencycalculator.h"

KEnterScheduleDialog::KEnterScheduleDialog(QWidget *parent, const MyMoneySchedule& schedule,
  const QDate& date)
  : kEnterScheduleDialogDecl(parent, "kenterscheduledialog"), m_schedDate(date)
{
  // Since we cannot create a category widget with the split button in designer,
  // we recreate the category widget here and replace it in the layout
  m_detailsGroupLayout->remove(m_category);
  delete m_category;
  m_category = new KMyMoneyCategory(m_detailsGroup, "m_category", true);

  // make sure to drop the surrounding frame into the layout
  m_detailsGroupLayout->addWidget(m_category->parentWidget(), 3, 1);


  m_schedule = schedule;
  m_transaction = schedule.transaction();

  connect(m_category->splitButton(), SIGNAL(clicked()), this, SLOT(slotEditSplits()));

  connect(m_buttonOk, SIGNAL(clicked()), this, SLOT(slotOK()));

  connect(m_from, SIGNAL(accountSelected(const QCString&)),
    this, SLOT(slotFromActivated(const QCString&)));
  connect(m_to, SIGNAL(accountSelected(const QCString&)),
    this, SLOT(slotToActivated(const QCString&)));
  connect(m_date, SIGNAL(dateChanged(const QDate&)), this, SLOT(checkDateInPeriod(const QDate&)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotReloadEditWidgets()));

  connect(m_payee, SIGNAL(createItem(const QString&, QCString&)), this, SIGNAL(createPayee(const QString&, QCString&)));
  connect(m_category, SIGNAL(createItem(const QString&, QCString&)), this, SIGNAL(createCategory(const QString&, QCString&)));

  // initWidgets must be called before calculateInterst, because
  // the internals of initWidgets require the unmodified splits
  // with the setting of automatically calculated values
  initWidgets();
  calculateInterest();

  if(m_schedule.id().isEmpty())
    m_buttonOk->setEnabled(false);
}

KEnterScheduleDialog::~KEnterScheduleDialog()
{
}

void KEnterScheduleDialog::initWidgets()
{
  KMyMoneyAccountCombo* loanAccount = 0;

  m_number->setText(QString());
  m_number->hide();
  m_numberLabel->hide();

  // Work around backwards transfers
  try
  {
    MyMoneyTransaction transaction = m_schedule.transaction();
    MyMoneySplit s1 = transaction.splits()[0];
    MyMoneySplit s2 = transaction.splits()[1];

    if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
    {
      if (s1.value().isNegative())
      {
        transaction.removeSplits();
        s2.clearId();
        s1.clearId();
        transaction.addSplit(s2);
        transaction.addSplit(s1);
        m_schedule.setTransaction(transaction);
        m_transaction = transaction;
      }
    }
    else if(m_schedule.type() != MyMoneySchedule::TYPE_LOANPAYMENT)
    {
      if (s1.value().isPositive())
      {
        transaction.removeSplits();
        s2.clearId();
        s1.clearId();
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

  // FIXME
  // m_splitButton->setGuiItem(KMyMoneyUtils::splitGuiItem());
  m_buttonOk->setGuiItem(KStdGuiItem::ok());
  m_buttonCancel->setGuiItem(KStdGuiItem::cancel());
  m_buttonHelp->setGuiItem(KStdGuiItem::help());

  if (m_schedule.type() != MyMoneySchedule::TYPE_DEPOSIT
  &&  m_schedule.type() != MyMoneySchedule::TYPE_LOANPAYMENT)
    m_from->loadList((KMyMoneyUtils::categoryTypeE)(KMyMoneyUtils::asset | KMyMoneyUtils::liability));

  if (m_schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT)
  {
    try
    {
      MyMoneySplit amortizationSplit;
      QValueList<MyMoneySplit>::ConstIterator it_s;
      for(it_s = m_transaction.splits().begin(); it_s != m_transaction.splits().end(); ++it_s) {
        if((*it_s).value() == MyMoneyMoney::autoCalc
        && (*it_s).action() == MyMoneySplit::ActionAmortization) {
          MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
          switch(acc.accountType()) {
            case MyMoneyAccount::Loan:
              m_from->setEnabled(true);
              m_to->setEnabled(false);
              m_from->loadList((KMyMoneyUtils::categoryTypeE)(KMyMoneyUtils::asset | KMyMoneyUtils::liability));
              loanAccount = m_from;
              break;

            case MyMoneyAccount::AssetLoan:
              m_from->setEnabled(false);
              m_to->setEnabled(true);
              m_to->loadList((KMyMoneyUtils::categoryTypeE)(KMyMoneyUtils::asset | KMyMoneyUtils::liability));
              loanAccount = m_to;
              break;

            default:
              qFatal("invalid account type in %s:%d", __FILE__, __LINE__);
              break;
          }
          break;
        }
      }
      if(loanAccount == 0) {
        KMessageBox::detailedError(this, i18n("Unable to load schedule details"), i18n("The schedule %1 caused an internal problem. Please contact the developers via e-mail on kmymoney2-developer@lists.sourceforge.net for further instructions mentioning this problem.").arg(m_schedule.id()));
        m_schedule = MyMoneySchedule();
        return;
      }
    }
    catch (MyMoneyException* e)
    {
      KMessageBox::detailedError(this, i18n("Unable to load schedule details"), e->what());
      delete e;
      return;
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
      // FIXME splits now allows in transfers
      // m_splitButton->setEnabled(false);
    }

    m_to->setEnabled(true);
    m_to->loadList((KMyMoneyUtils::categoryTypeE)(KMyMoneyUtils::asset | KMyMoneyUtils::liability));
  }

  try
  {
    if (m_schedule.account().name().isEmpty())
      return;

    if(m_schedule.paymentType() == MyMoneySchedule::STYPE_WRITECHEQUE) {
      m_number->show();
      m_numberLabel->show();

      m_number->setText(KMyMoneyUtils::nextCheckNumber(m_schedule.account()));
    }

    if (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER)
    {
      m_from->setCurrentText(m_schedule.account().name());
      m_to->setCurrentText(/*m_schedule.transferAccount().name()*/
        MyMoneyFile::instance()->account(m_schedule.transaction().splits()[1].accountId()).name());
      m_fromAccountId = m_schedule.transaction().splits()[0].accountId();
      m_toAccountId = m_schedule.transaction().splits()[1].accountId();
    }
    else if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT) {
      m_to->setCurrentText(m_schedule.account().name());
      m_toAccountId = m_schedule.account().id();
    }
    else if (m_schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT)
    {
      loanAccount->setCurrentText(m_schedule.account().name());
      loanAccount == m_to ?
          m_toAccountId = m_schedule.account().id() : m_fromAccountId = m_schedule.account().id();
    }
    else
    {
      m_from->setCurrentText(m_schedule.account().name());
      m_fromAccountId = m_schedule.account().id();
      m_to->setEnabled(false);
    }

    m_payee->loadPayees(MyMoneyFile::instance()->payeeList());
    m_payee->setSelectedItem(m_transaction.splitByAccount(m_schedule.account().id()).payeeId());

    //  m_payee->setText(MyMoneyFile::instance()->payee(m_transaction.splitByAccount(m_schedule.account().id()).payeeId()).name());

    if (m_schedDate.isValid())
      m_date->setDate(m_schedDate);
    else
      m_date->setDate(m_schedule.nextPayment(m_schedule.lastPayment()));

    MyMoneyMoney amount = m_transaction.splitByAccount(m_schedule.account().id()).value();
    if (amount.isNegative())
      amount = -amount;
    m_amount->setValue(amount);

    if (m_transaction.splitCount() >= 3)
    {
      m_category->setSplitTransaction();
      // m_category->setText(i18n("Split transaction (category replacement)", "Split transaction"));
      connect(m_category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));
    }
    else if (m_schedule.type() != MyMoneySchedule::TYPE_TRANSFER)
    {
        m_category->setSelectedItem(m_transaction.splitByAccount(m_schedule.account().id(), false).accountId());
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

void KEnterScheduleDialog::slotOK(void)
{
  if (checkData())
  {
    // Commit this transaction to file.
    commitTransaction();
    accept();
  }
}

void KEnterScheduleDialog::slotReloadEditWidgets(void)
{
#if 0
  // TODO: reload the account and category widgets
  // reload category widget
  KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
  QCString categoryId;
  category->selectedItem(categoryId);

  AccountSet aSet(m_objects);
  aSet.addAccountGroup(MyMoneyAccount::Asset);
  aSet.addAccountGroup(MyMoneyAccount::Liability);
  aSet.addAccountGroup(MyMoneyAccount::Income);
  aSet.addAccountGroup(MyMoneyAccount::Expense);
  if(KMyMoneySettings::expertMode())
    aSet.addAccountGroup(MyMoneyAccount::Equity);
  aSet.load(category->selector());
#endif

  // reload payee widget
  QCString payeeId = m_payee->selectedItem();

  m_payee->loadPayees(MyMoneyFile::instance()->payeeList());

  if(!payeeId.isEmpty()) {
    m_payee->setSelectedItem(payeeId);
  }
}

void KEnterScheduleDialog::slotEditSplits(void)
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

    MyMoneyObjectContainer objects;
    QMap<QCString, MyMoneyMoney> priceInfo;
    KSplitTransactionDlg* dlg = new KSplitTransactionDlg(m_transaction,
                                                         acc,
                                                         isValidAmount,
                                                         isDeposit,
                                                         0,
                                                         &objects,
                                                         priceInfo,
                                                         this);
    connect(dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));

    // Avoid focusIn() events.
    m_memo->setFocus();

    if(dlg->exec())
    {
      m_transaction = dlg->transaction();

      MyMoneySplit s;

      switch(m_transaction.splitCount())
      {
        case 2:
          s = m_transaction.splitByAccount(theAccountId(), false);
          m_category->setSelectedItem(s.accountId());
          disconnect(m_category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));
          break;
        case 1:
          m_category->setSelectedItem(QCString());
          m_transaction.removeSplits();
          disconnect(m_category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));
          break;
        case 0:
          m_category->setCurrentText(QString());
          disconnect(m_category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));
          break;
        default:
          m_category->setSplitTransaction();
          connect(m_category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));
          break;
      }

      MyMoneySplit split = m_transaction.splitByAccount(theAccountId());
      MyMoneyMoney amount(split.value());
      amount = amount.abs();
      m_amount->setValue(amount);
    }

    delete dlg;
  } catch (MyMoneyException *e)
  {
    KMessageBox::detailedError(this, i18n("Exception in slot split clicked"), e->what());
    delete e;
  }

  m_category->blockSignals(false);
}

bool KEnterScheduleDialog::checkData(void)
{
  QString messageDetail;
  int noItemsChanged=0;
  QCString payeeOld, payeeNew;

  // if no schedule is present, we cannot enter it
  if(m_schedule.id().isEmpty())
    return false;

  if (m_fromAccountId == m_toAccountId)
  {
    KMessageBox::error(this, i18n("Account and transfer account are the same.  Please change one."));
    m_from->setFocus();
    return false;
  }

  if (!checkDateInPeriod(m_date->date()))
    return false;

  try
  {
    payeeOld = m_schedule.transaction().splitByAccount(m_schedule.account().id()).payeeId();
    payeeNew = m_payee->selectedItem();

    if (payeeOld != payeeNew)
    {
      noItemsChanged++;
      messageDetail += i18n("Payee changed.  Old: \"%1\", New: \"%2\"")
        .arg(MyMoneyFile::instance()->payee(payeeOld).name()).arg(m_payee->currentText()) + QString("\n");
    }

    if (  (m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER ||
          m_schedule.type() == MyMoneySchedule::TYPE_BILL) &&
          m_from->currentText() != m_schedule.account().name())
    {
      noItemsChanged++;
      messageDetail += i18n("Account changed.  Old: \"%1\", New: \"%2\"")
        .arg(m_schedule.account().name()).arg(m_from->currentText()) + QString("\n");
    }

    if (  m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT &&
          m_to->currentText() != m_schedule.account().name())
    {
      noItemsChanged++;
      messageDetail += i18n("Account changed.  Old: \"%1\", New: \"%2\"")
        .arg(m_schedule.account().name()).arg(m_to->currentText()) + QString("\n");
    }

    if (  m_schedule.type() == MyMoneySchedule::TYPE_TRANSFER &&
          m_to->currentText() != MyMoneyFile::instance()->account(m_schedule.transaction().splits()[1].accountId()).name())
    {
      noItemsChanged++;
      messageDetail += i18n("Transfer account changed.  Old: \"%1\", New: \"%2\"")
        .arg(MyMoneyFile::instance()->account(m_schedule.transaction().splits()[1].accountId()).name())
        .arg(m_to->currentText()) + QString("\n");
    }

    if (m_schedule.type() != MyMoneySchedule::TYPE_TRANSFER)
    {
      QString category;
      if (m_schedule.transaction().splitCount() >= 3)
        category = i18n("Split transaction (category replacement)", "Split transaction");
      else
        category = MyMoneyFile::instance()->accountToCategory(m_schedule.transaction()
          .splitByAccount(m_schedule.account().id(), false).accountId());

      if (category != m_category->currentText())
      {
        noItemsChanged++;
        messageDetail += i18n("Category changed.  Old: \"%1\", New: \"%2\"")
          .arg(category).arg(m_category->currentText()) + QString("\n");
      }
    }

    QString memo = m_schedule.transaction().splitByAccount(m_schedule.account().id()).memo();
    if (!memo.isEmpty() || !m_memo->text().isEmpty())
    {
      if(memo != m_memo->text())
      {
        noItemsChanged++;
        messageDetail += i18n("Memo changed.  Old: \"%1\", New: \"%2\"")
          .arg(memo).arg(m_memo->text()) + QString("\n");
      }
    }

    MyMoneyMoney amount = m_schedule.transaction().splitByAccount(m_schedule.account().id()).value();
    amount = amount.abs();
    if (amount != m_amount->value())
    {
      noItemsChanged++;
      messageDetail += i18n("Amount changed.  Old: \"%1\", New: \"%2\"")
        .arg(amount.formatMoney()).arg(m_amount->value().formatMoney()) + QString("\n");
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
  if (m_category->isSplitTransaction() &&
      m_schedule.type() != MyMoneySchedule::TYPE_TRANSFER)
  {
    QCString id = m_category->selectedItem();
#if 0
    // the following part is not used anymore, as the category field
    // should always contain a valid category or identification of a
    // split transaction.
    if(id.isEmpty() && !m_category->currentText().isEmpty())
    {
      // Create the category
      QString message = i18n("The category '%1' does not exist.  Create?").arg(m_category->currentText());
      if (KMessageBox::questionYesNo(this, message) == KMessageBox::Yes)
      {
        MyMoneyAccount base;
        if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
          base = MyMoneyFile::instance()->income();
        else
          base = MyMoneyFile::instance()->expense();

        id = MyMoneyFile::instance()->createCategory(base, m_category->currentText());
      }
      else
      {
        m_category->setSelectedItem(QCString());
        m_category->setFocus();
        return;
      }
    }
#endif

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
    m_schedDate = m_date->date();
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

      QString message = i18n("Some occurences that are older than '%1' have not been entered yet.\n\nDelete all occurences that have not been entered before this date?").arg(m_schedDate.toString());

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
      QString message = i18n("Are you sure you want to enter this occurence which is %1 days after today?").arg(QDate::currentDate().daysTo(m_schedDate));
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
    m_transaction.setCommodity(m_schedule.account().currencyId());

    try {
      MyMoneyFile* file = MyMoneyFile::instance();
      QValueList<MyMoneySplit> list = m_transaction.splits();
      QValueList<MyMoneySplit>::Iterator it;

      // Fix the payeeId. For non-asset and non-liability accounts,
      // the payeeId will be cleared. If a transfer has one split
      // with an empty payeeId the other one will be copied.
      //
      // Splits not referencing any account will be removed.
      // Price information for other currencies will be collected

      QMap<QCString, MyMoneyMoney> priceInfo;
      for(it = list.begin(); it != list.end(); ++it) {
        if((*it).accountId().isEmpty()) {
          m_transaction.removeSplit(*it);
          continue;
        }
        MyMoneyAccount acc = file->account((*it).accountId());
        MyMoneySecurity currency(file->currency(m_schedule.account().currencyId()));
        int fract = currency.smallestAccountFraction();
        if(acc.accountType() == MyMoneyAccount::Cash)
          fract = currency.smallestCashFraction();

#if 0
// Commented so that the payee also gets recorded with categories
        MyMoneyAccount::accountTypeE accType = file->accountGroup(acc.accountType());
        switch(accType) {
          case MyMoneyAccount::Asset:
          case MyMoneyAccount::Liability:
            break;

          default:
            (*it).setPayeeId(QCString());
            m_transaction.modifySplit(*it);
            break;
        }
#endif

        if(m_transaction.commodity() != acc.currencyId()) {
          // different currencies, try to find recent price info
          QMap<QCString, MyMoneyMoney>::Iterator it_p;
          QCString key = m_transaction.commodity() + "-" + acc.currencyId();
          it_p = priceInfo.find(key);

          // if it's not found, then collect it from the user first
          MyMoneyMoney price;
          if(it_p == priceInfo.end()) {
            MyMoneySecurity fromCurrency, toCurrency;
            MyMoneyMoney fromValue, toValue;
            if(m_transaction.commodity() != m_schedule.account().currencyId()) {
              toCurrency = file->currency(m_transaction.commodity());
              fromCurrency = file->currency(acc.currencyId());
              toValue = (*it).value();
              fromValue = (*it).shares();
            } else {
              fromCurrency = file->currency(m_transaction.commodity());
              toCurrency = file->currency(acc.currencyId());
              fromValue = (*it).value();
              toValue = (*it).shares();
            }

            KCurrencyCalculator calc(fromCurrency,
                                    toCurrency,
                                    fromValue,
                                    toValue,
                                    m_transaction.postDate(),
                                    fract,
                                    this, "currencyCalculator");
            if(calc.exec() == QDialog::Rejected) {
              return;
            }
            price = calc.price();
            priceInfo[key] = price;
          } else {
            price = (*it_p);
          }

          // update shares if the transaction commodity is the currency
          // of the current selected account
          if(m_transaction.commodity() == m_schedule.account().currencyId()) {
            (*it).setShares(((*it).value() * price).convert(fract));
          }

          // now update the split
          m_transaction.modifySplit(*it);
        } else {
          if((*it).shares() != (*it).value()) {
            (*it).setShares((*it).value());
            m_transaction.modifySplit(*it);
          }
        }
      }

#if 0
      // FIXME: This should be run through, but currently we don't have
      // access to the transactionType() routine. So we keep the source here
      // and let things go.
      if(transactionType(m_transaction) == Transfer && !m_split.payeeId().isEmpty()) {
        for(it = list.begin(); it != list.end(); ++it) {
          if((*it).id() == m_split.id())
            continue;

          if((*it).payeeId().isEmpty()) {
            (*it).setPayeeId(m_split.payeeId());
            m_transaction.modifySplit(*it);
          }
        }
      }

      // check if this is a transfer to/from loan account and
      // mark it as amortization in this case
      if(m_transaction.splitCount() == 2) {
        bool isAmortization = false;
        for(it = list.begin(); !isAmortization && it != list.end(); ++it) {
          if((*it).action() == MyMoneySplit::ActionTransfer) {
            MyMoneyAccount acc = file->account((*it).accountId());
            if(acc.accountType() == MyMoneyAccount::Loan
            || acc.accountType() == MyMoneyAccount::AssetLoan)
              isAmortization = true;
          }
        }

        if(isAmortization) {
          for(it = list.begin(); it != list.end(); ++it) {
            (*it).setAction(MyMoneySplit::ActionAmortization);
            m_transaction.modifySplit(*it);
          }
        }
      }
#endif

      // If we are writing a check, we adjust the split's
      // number to the highest number used in this account plus one
      if(m_schedule.paymentType() == MyMoneySchedule::STYPE_WRITECHEQUE
      && !m_number->text().isEmpty()) {
        MyMoneyAccount acc = m_schedule.account();
        MyMoneySplit s = m_transaction.splitByAccount(acc.id(), true);
        s.setAction(MyMoneySplit::ActionCheck);
        s.setNumber(m_number->text());
        m_transaction.modifySplit(s);

        acc.setValue("lastNumberUsed", m_number->text());
        MyMoneyFile::instance()->modifyAccount(acc);
      }

      MyMoneyFile::instance()->addTransaction(m_transaction);
      MyMoneyFile::instance()->modifySchedule(m_schedule);
    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to add transaction/modify schedule"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
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
    s.setAccountId(m_toAccountId);
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
    s.setAccountId(m_fromAccountId);
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
    if (m_amount->value() != amount)
    {
      if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
        s.setValue(m_amount->value());
      else if(m_schedule.type() != MyMoneySchedule::TYPE_LOANPAYMENT)
        s.setValue(-m_amount->value());

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
    QCString payeeId = m_payee->selectedItem();

    MyMoneySplit split1;
    split1.setAccountId(theAccountId());

#if 0
    // These actions are not required anymore
    if (m_schedule.type() == MyMoneySchedule::TYPE_BILL)
      split1.setAction(MyMoneySplit::ActionWithdrawal);
    else if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
      split1.setAction(MyMoneySplit::ActionDeposit);
    else
      split1.setAction(MyMoneySplit::ActionTransfer);
#endif
    split1.setPayeeId(payeeId);
    split1.setMemo(m_memo->text());

    if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
      split1.setValue(m_amount->value());
    else
      split1.setValue(-m_amount->value());

    m_transaction.addSplit(split1);


    MyMoneySplit split2;
    if (m_schedule.type() != MyMoneySchedule::TYPE_TRANSFER)
    {
      checkCategory();
    }
    else
      split2.setAccountId(m_toAccountId);

    split2.setPayeeId(split1.payeeId());
    split2.setMemo(split1.memo());
    split2.setValue(-split1.value());

#if 0
    // These actions are not required anymore
    if (split1.action() == MyMoneySplit::ActionDeposit)
      split2.setAction(MyMoneySplit::ActionWithdrawal);
    else if (split1.action() == MyMoneySplit::ActionWithdrawal)
      split2.setAction(MyMoneySplit::ActionDeposit);
    else
      split2.setAction(MyMoneySplit::ActionTransfer);
#endif

    m_transaction.addSplit(split2);
  }
}

QCString KEnterScheduleDialog::theAccountId()
{
  if( m_schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
    if(m_from->isEnabled())
      return m_fromAccountId;
    if(m_to->isEnabled())
      return m_toAccountId;
    qFatal("This should never happen %s:%d", __FILE__, __LINE__);
  }

  if (m_schedule.type() != MyMoneySchedule::TYPE_DEPOSIT)
    return m_fromAccountId;
  else
    return m_toAccountId;
}

void KEnterScheduleDialog::slotFromActivated(const QCString& id)
{
  if (m_schedule.type() != MyMoneySchedule::TYPE_DEPOSIT)
  {
    m_fromAccountId = id;
    // Change the splits because otherwise they won't be found
    // if the user clicks on the split button
    MyMoneySplit s = m_transaction.splits()[0];
    s.setAccountId(id);
    m_transaction.modifySplit(s);
  }
}

void KEnterScheduleDialog::slotToActivated(const QCString& id)
{
  if (m_schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
  {
    m_toAccountId = id;
    // Change the splits because otherwise they won't be found
    // if the user clicks on the split button
    MyMoneySplit s = m_transaction.splits()[0];
    s.setAccountId(id);
    m_transaction.modifySplit(s);
  }
}

void KEnterScheduleDialog::calculateInterest(void)
{
  try
  {
  if (m_schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
    MyMoneySplit interestSplit, amortizationSplit;
    QValueList<MyMoneySplit>::ConstIterator it_s;
    for(it_s = m_transaction.splits().begin(); it_s != m_transaction.splits().end(); ++it_s) {
      if((*it_s).value() == MyMoneyMoney::autoCalc) {
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
      calc.setIr(static_cast<FCALC_DOUBLE> (acc.interestRate(dueDate).abs().toDouble()));
      calc.setPmt(acc.periodicPayment().toDouble());

      MyMoneyMoney interest(calc.interestDue()), amortization;
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
  catch (MyMoneyException* e)
  {
    KMessageBox::detailedError(this, i18n("Unable to load schedule details"), e->what());
    delete e;
    reject();
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
    // let the user continue anyway as the calculation is not always correct
    // proper fix will be available in 0.7/0.9.
    if (KMessageBox::warningContinueCancel(this, i18n("The date must lie in the range %1 to %2").arg(firstDate.addDays(1).toString()).arg(nextDate.toString())) != KMessageBox::Continue)
      return false;
  }

  return true;
}
// vim:cin:si:ai:et:ts=2:sw=2:

#include "kenterscheduledialog.moc"
