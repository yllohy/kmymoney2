/***************************************************************************
                             transactioneditor.cpp
                             ----------
    begin                : Wed Jun 07 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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
#include <qapplication.h>
#include <qeventloop.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <ktextedit.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kstdguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/transactioneditor.h>
#include <kmymoney/kmymoneycategory.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneylineedit.h>
#include <kmymoney/kmymoneyaccountcompletion.h>
#include <kmymoney/kmymoneyaccountselector.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneyutils.h>
#include <kmymoney/transactionform.h>
#include <kmymoney/kmymoneyglobalsettings.h>

#include "../dialogs/ksplittransactiondlg.h"
#include "../dialogs/kcurrencycalculator.h"
#include "../dialogs/kselecttransactionsdlg.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

TransactionEditor::TransactionEditor(TransactionEditorContainer* regForm, KMyMoneyRegister::Transaction* item, const QValueList<KMyMoneyRegister::SelectedTransaction>& list, const QDate& lastPostDate) :
  m_transactions(list),
  m_regForm(regForm),
  m_item(item),
  m_transaction(item->transaction()),
  m_split(item->split()),
  m_lastPostDate(lastPostDate),
  m_openEditSplits(false)
{
  m_item->startEditMode();
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotUpdateAccount()));
}

TransactionEditor::~TransactionEditor()
{
  // Make sure the widgets do not send out signals to the editor anymore
  // After all, the editor is about to die
  QMap<QString, QWidget*>::iterator it_w;
  for(it_w = m_editWidgets.begin(); it_w != m_editWidgets.end(); ++it_w) {
    (*it_w)->disconnect(this);
  }

  m_regForm->removeEditWidgets(m_editWidgets);
  m_item->leaveEditMode();
  emit finishEdit(m_transactions);
}

void TransactionEditor::slotUpdateAccount(void)
{
  // reload m_account as it might have been changed
  m_account = MyMoneyFile::instance()->account(m_account.id());
}

void TransactionEditor::setup(QWidgetList& tabOrderWidgets, const MyMoneyAccount& account, KMyMoneyRegister::Action action)
{
  m_account = account;
  m_initialAction = action;
  createEditWidgets();
  m_regForm->arrangeEditWidgets(m_editWidgets, m_item);
  m_regForm->tabOrder(tabOrderWidgets, m_item);
  QWidget* w = haveWidget("tabbar");
  if(w)
    tabOrderWidgets.append(w);
  loadEditWidgets(action);
  m_editWidgets.removeOrphans();
  slotUpdateButtonState();
}

void TransactionEditor::slotReloadEditWidgets(void)
{
}

void TransactionEditor::slotNumberChanged(const QString& txt)
{
  kMyMoneyLineEdit* number = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));

  if(number) {
    if(MyMoneyFile::instance()->checkNoUsed(m_account.id(), txt)) {
      if(KMessageBox::questionYesNo(m_regForm, QString("<qt>")+i18n("The number <b>%1</b> has already been used in account <b>%2</b>. Do you want to replace it with the next available number?").arg(txt).arg(m_account.name())+QString("</qt>"), i18n("Duplicate number")) == KMessageBox::Yes) {
        number->loadText(KMyMoneyUtils::nextCheckNumber(m_account));
      }
    }
  }
}

void TransactionEditor::slotUpdateButtonState(void)
{
  emit transactionDataSufficient(isComplete());
}

QWidget* TransactionEditor::haveWidget(const QString& name) const
{
  return m_editWidgets.haveWidget(name);
}

int TransactionEditor::slotEditSplits(void)
{
  return QDialog::Rejected;
}

#if 0
  // If we deal with multiple currencies we make sure, that for
  // transactions with two splits, the transaction's commodity is the
  // currency of the currently selected account. This saves us from a
  // lot of grieve later on.
  // Editing a transaction which has more than two splits and a commodity
  // that differs from the currency of the current selected account is
  // not a good idea. We will warn the user and give him a hint if there
  // is an account where he can perfom the edit operation much better.
  if(m_transaction.commodity() != m_account.currencyId()) {
    if(m_transaction.splitCount() == 2) {
      // in case of two splits, it's easy. We just have to switch the
      // transactions commodity. Let's assume the following scenario:
      // - transactions commodity is CA
      // - account's currencyId is CB
      // - second split is of course in CA (otherwise we have a real problem)
      // - Value is V in both splits
      // - Shares in this account's split is SB
      // - Shares in the other account's split is SA (and equal to V)
      //
      // We do the following:
      // - change transactions commodity to CB
      // - set V in both splits to SB
      // - modify the splits in the transaction
      try {
        MyMoneySplit split = m_transaction.splitByAccount(m_account.id(), false);
        m_transaction.setCommodity(m_account.currencyId());
        m_split.setValue(m_split.shares());
        split.setValue(-m_split.shares());
        m_transaction.modifySplit(m_split);
        m_transaction.modifySplit(split);

        if(m_transactionPtr) {
          KMyMoneyTransaction k(m_transaction);
          k.setSplitId(m_split.id());
          *m_transactionPtr = k;
        }
      } catch(MyMoneyException *e) {
        qDebug("Unable to update commodity to second splits currency in %s: '%s'", m_transaction.id().data(), e->what().data());
        delete e;
      }

    } else {
      // Find a suitable account
      MyMoneySecurity sec = MyMoneyFile::instance()->currency(m_transaction.commodity());
      MyMoneyAccount acc;
      for(it = m_transaction.splits().begin(); it != m_transaction.splits().end(); ++it) {
        if((*it).id() == m_split.id())
          continue;
        acc = MyMoneyFile::instance()->account((*it).accountId());
        if((acc.accountGroup() == MyMoneyAccount::Asset
        || acc.accountGroup() == MyMoneyAccount::Liability)
        && acc.accountType() != MyMoneyAccount::Stock) {
          if(m_transaction.commodity() == acc.currencyId())
            break;
        }
        acc = MyMoneyAccount();
      }
      QString msg;
      msg = QString("<p>")+i18n("This transaction has more than two splits and is based on a different currency (%1). Using this account to modify the transaction is currently not very well supported by KMyMoney and may result in false results.").arg(sec.name())+QString(" ");
      if(acc.id().isEmpty()) {
        msg += i18n("KMyMoney was not able to find a more appropriate account to edit this transaction. Nevertheless, you are allowed to modify the transaction. If you don't want to edit this transaction, please cancel from editing next.");
      } else {
         msg += i18n("Using e.g. <b>%1</b> to edit this transaction is a better choice. Nevertheless, you are allowed to modify the transaction. If you want to use the suggested account instead, please cancel from editing next and change the view to the suggested account.").arg(acc.name());
      }
      KMessageBox::information(0, msg);
    }
  }
#endif

void TransactionEditor::setTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s)
{
  m_transaction = t;
  m_split = s;
  loadEditWidgets();
}

bool TransactionEditor::fixTransactionCommodity(const MyMoneyAccount& account)
{
  bool rc = true;
  bool firstTimeMultiCurrency = true;
  m_account = account;

  MyMoneyFile* file = MyMoneyFile::instance();

  // determine the max fraction for this account
  MyMoneySecurity sec = file->security(m_account.currencyId());
  int fract = m_account.fraction(sec);

  // scan the list of selected transactions
  QValueList<KMyMoneyRegister::SelectedTransaction>::iterator it_t;
  for(it_t = m_transactions.begin(); (rc == true) && (it_t != m_transactions.end()); ++it_t) {
    // there was a time when the schedule editor did not setup the transaction commodity
    // let's give a helping hand here for those old schedules
    if((*it_t).transaction().commodity().isEmpty())
      (*it_t).transaction().setCommodity(m_account.currencyId());
    // we need to check things only if a different commodity is used
    if(m_account.currencyId() != (*it_t).transaction().commodity()) {
      MyMoneySecurity osec = file->security((*it_t).transaction().commodity());
      switch((*it_t).transaction().splitCount()) {
        case 0:
          // new transaction, guess nothing's here yet ;)
          break;

        case 1:
          try {
            // make sure, that the value is equal to the shares, don't forget our own copy
            MyMoneySplit& splitB = (*it_t).split();  // reference usage wanted here
            if(m_split == splitB)
              m_split.setValue(splitB.shares());
            splitB.setValue(splitB.shares());
            (*it_t).transaction().modifySplit(splitB);

          } catch(MyMoneyException *e) {
            qDebug("Unable to update commodity to second splits currency in %s: '%s'", (*it_t).transaction().id().data(), e->what().data());
            delete e;
          }
          break;

        case 2:
          // If we deal with multiple currencies we make sure, that for
          // transactions with two splits, the transaction's commodity is the
          // currency of the currently selected account. This saves us from a
          // lot of grieve later on.  We just have to switch the
          // transactions commodity. Let's assume the following scenario:
          // - transactions commodity is CA
          // - splitB and account's currencyId is CB
          // - splitA is of course in CA (otherwise we have a real problem)
          // - Value is V in both splits
          // - Shares in splitB is SB
          // - Shares in splitA is SA (and equal to V)
          //
          // We do the following:
          // - change transactions commodity to CB
          // - set V in both splits to SB
          // - modify the splits in the transaction
          try {
            // retrieve the splits
            MyMoneySplit& splitB = (*it_t).split();  // reference usage wanted here
            MyMoneySplit splitA = (*it_t).transaction().splitByAccount(m_account.id(), false);

            // - set V in both splits to SB. Don't forget our own copy
            if(m_split == splitB) {
              m_split.setValue(splitB.shares());
            }
            splitB.setValue(splitB.shares());
            splitA.setValue(-splitB.shares());
            (*it_t).transaction().modifySplit(splitA);
            (*it_t).transaction().modifySplit(splitB);

          } catch(MyMoneyException *e) {
            qDebug("Unable to update commodity to second splits currency in %s: '%s'", (*it_t).transaction().id().data(), e->what().data());
            delete e;
          }
          break;

        default:
          // TODO: use new logic by adjusting all splits by the price
          // extracted from the selected split. Inform the user that
          // this will happen and allow him to stop the processing (rc = false)

          try {
            QString msg;
            if(firstTimeMultiCurrency) {
              firstTimeMultiCurrency = false;
              if(!isMultiSelection()) {
                msg = i18n("This transaction has more than two splits and is originally based on a different currency (%1). Using this account to modify the transaction may result in rounding errors. Do you want to continue?").arg(osec.name());
              } else {
                msg = i18n("At least one of the selected transactions has more than two splits and is originally based on a different currency (%1). Using this account to modify the transactions may result in rounding errors. Do you want to continue?").arg(osec.name());
              }

              if(KMessageBox::warningContinueCancel(0, QString("<qt>%1</qt>").arg(msg)) == KMessageBox::Cancel) {
                rc = false;
              }
            }

            if(rc == true) {
              MyMoneyMoney price;
              if(!(*it_t).split().shares().isZero() && !(*it_t).split().value().isZero())
                price = (*it_t).split().shares() / (*it_t).split().value();
              QValueList<MyMoneySplit>::iterator it_s;
              MyMoneySplit& mySplit = (*it_t).split();
              for(it_s = (*it_t).transaction().splits().begin(); it_s != (*it_t).transaction().splits().end(); ++it_s) {
                MyMoneySplit s = (*it_s);
                if(s == mySplit) {
                  s.setValue(s.shares());
                  if(mySplit == m_split) {
                    m_split = s;
                  }
                  mySplit = s;
                } else {
                  s.setValue((s.value() * price).convert(fract));
                }
                (*it_t).transaction().modifySplit(s);
              }
            }
          } catch(MyMoneyException *e) {
            qDebug("Unable to update commodity of split currency in %s: '%s'", (*it_t).transaction().id().data(), e->what().data());
            delete e;
          }
          break;
      }

      // set the transaction's ommodity to this account's currency
      (*it_t).transaction().setCommodity(m_account.currencyId());

      // update our copy of the transaction that has the focus
      if((*it_t).transaction().id() == m_transaction.id()) {
        m_transaction = (*it_t).transaction();
      }
    }
  }
  return rc;
}

void TransactionEditor::assignNumber(void)
{
  if(canAssignNumber()) {
    kMyMoneyLineEdit* number = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));
    number->loadText(KMyMoneyUtils::nextCheckNumber(m_account));
  }
}

bool TransactionEditor::canAssignNumber(void) const
{
  kMyMoneyLineEdit* number = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));
  return (number != 0) && (number->text().isEmpty());
}

void TransactionEditor::setupCategoryWidget(KMyMoneyCategory* category, const QValueList<MyMoneySplit>& splits, QCString& categoryId, const char* splitEditSlot, bool /* allowObjectCreation */)
{
  disconnect(category, SIGNAL(focusIn()), this, splitEditSlot);
#if 0
  // FIXME must deal with the logic that suppressObjectCreation is
  // automatically turned off when the createItem() signal is connected
  if(allowObjectCreation)
    category->setSuppressObjectCreation(false);
#endif

  switch(splits.count()) {
    case 0:
      categoryId = QCString();
      if(!category->currentText().isEmpty()) {
        category->setCurrentText();
        // make sure, we don't see the selector
        category->completion()->hide();
      }
      category->completion()->setSelected(QCString());
      break;

    case 1:
      categoryId = splits[0].accountId();
      category->completion()->setSelected(categoryId);
      category->slotItemSelected(categoryId);
      break;

    default:
      categoryId = QCString();
      category->setSplitTransaction();
      connect(category, SIGNAL(focusIn()), this, splitEditSlot);
#if 0
  // FIXME must deal with the logic that suppressObjectCreation is
  // automatically turned off when the createItem() signal is connected
      if(allowObjectCreation)
        category->setSuppressObjectCreation(true);
#endif
      break;
  }
}

bool TransactionEditor::enterTransactions(QCString& newId)
{
  newId = QCString();
  MyMoneyFile* file = MyMoneyFile::instance();

  // make sure to run through all stuff that is tied to 'focusout events'.
  m_regForm->parentWidget()->setFocus();
  QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 10);

  // we don't need to update our widgets anymore, so we just disconnect the signal
  disconnect(file, SIGNAL(dataChanged()), this, SLOT(slotReloadEditWidgets()));

  QValueList<KMyMoneyRegister::SelectedTransaction>::iterator it_t;
  MyMoneyTransaction t;
  bool newTransactionCreated = false;

  // make sure, that only a single new transaction can be created.
  // we need to update m_transactions to contain the new transaction
  // which is then stored in the variable t when we leave the loop.
  // m_transactions will be sent out in finishEdit() and forces
  // the new transaction to be selected in the ledger view

  // collect the transactions to be stored in the engine in a local
  // list first, so that the user has a chance to interrupt the storage
  // process
  QValueList<MyMoneyTransaction> list;
  bool storeTransactions = true;

  // collect transactions
  for(it_t = m_transactions.begin(); storeTransactions && !newTransactionCreated && it_t != m_transactions.end(); ++it_t) {
    storeTransactions = createTransaction(t, (*it_t).transaction(), (*it_t).split());
    // if the transaction was created successfully, append it to the list
    if(storeTransactions)
      list.append(t);

    // if we created a new transaction keep that in mind
    if(t.id().isEmpty())
      newTransactionCreated = true;
  }

  // if not interrupted by user, continue to store them in the engine
  if(storeTransactions) {
    int i = 0;
    emit statusMsg(i18n("Storeing transactions"));
    emit statusProgress(0, list.count());

    MyMoneyFileTransaction ft;

    try {
      QValueList<MyMoneyTransaction>::iterator it_ts;
      for(it_ts = list.begin(); it_ts != list.end(); ++it_ts) {
        // if we have a categorization, make sure we remove
        // the 'imported' flag automagically
        if((*it_ts).splitCount() > 1)
          (*it_ts).deletePair("Imported");

        if((*it_ts).id().isEmpty()) {
          // add new transaction
          file->addTransaction(*it_ts);
          // pass the newly assigned id on to the caller
          newId = (*it_ts).id();
          // refresh account object for transactional changes
          m_account = file->account(m_account.id());

          // if a new transaction has a valid number, keep it with the account
          QString number = (*it_ts).splits()[0].number();
          if(!number.isEmpty()) {
            m_account.setValue("lastNumberUsed", number);
            file->modifyAccount(m_account);
          }

          // send out the post date of this transaction
          emit lastPostDateUsed((*it_ts).postDate());

        } else {
          // modify existing transaction
          file->modifyTransaction(*it_ts);
        }
      }
      emit statusProgress(i++, 0);

      // update m_transactions to contain the newly created transaction so that
      // it is selected as the current one
      if(newTransactionCreated) {
        m_transactions.clear();
        KMyMoneyRegister::SelectedTransaction s((*it_ts), (*it_ts).splits()[0]);
        m_transactions.append(s);
      }

      ft.commit();

    } catch(MyMoneyException * e) {
      qDebug("Unable to store transaction within engine: %s", e->what().latin1());
      delete e;
      newTransactionCreated = false;
    }

    emit statusProgress(-1, -1);
    emit statusMsg(QString());

  }
  return storeTransactions;
}


StdTransactionEditor::StdTransactionEditor()
{
}

StdTransactionEditor::StdTransactionEditor(TransactionEditorContainer* regForm, KMyMoneyRegister::Transaction* item, const QValueList<KMyMoneyRegister::SelectedTransaction>& list, const QDate& lastPostDate) :
  TransactionEditor(regForm, item, list, lastPostDate),
  m_inUpdateVat(false)
{
}

void StdTransactionEditor::createEditWidgets(void)
{
  KMyMoneyPayeeCombo* payee = new KMyMoneyPayeeCombo;
  payee->setHint(i18n("Payer/Receiver"));
  m_editWidgets["payee"] = payee;
  connect(payee, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(payee, SIGNAL(createItem(const QString&, QCString&)), this, SIGNAL(createPayee(const QString&, QCString&)));
  connect(payee, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));
  connect(payee, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotUpdatePayee(const QCString&)));

  KMyMoneyCategory* category = new KMyMoneyCategory(0, 0, true);
  category->setHint(i18n("Category/Account"));
  m_editWidgets["category"] = category;
  connect(category, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotUpdateCategory(const QCString&)));
  connect(category, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(category, SIGNAL(createItem(const QString&, QCString&)), this, SLOT(slotCreateCategory(const QString&, QCString&)));
  connect(category, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));
  connect(category->splitButton(), SIGNAL(clicked()), this, SLOT(slotEditSplits()));

  KTextEdit* memo = new KTextEdit;
  memo->setTabChangesFocus(true);
  m_editWidgets["memo"] = memo;

  bool showNumberField = true;
  switch(m_account.accountType()) {
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Equity:
      showNumberField = KMyMoneySettings::alwaysShowNrField();
      break;

    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
      showNumberField = false;
      break;

    default:
      break;
  }
  if(showNumberField) {
    kMyMoneyLineEdit* number = new kMyMoneyLineEdit;
    number->setHint(i18n("Number"));
    m_editWidgets["number"] = number;
    connect(number, SIGNAL(lineChanged(const QString&)), this, SLOT(slotNumberChanged(const QString&)));
  }

  m_editWidgets["postdate"] = new kMyMoneyDateInput;

  kMyMoneyEdit* value = new kMyMoneyEdit;
  m_editWidgets["amount"] = value;
  value->setResetButtonVisible(false);
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateAmount(const QString&)));
  connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));

  value = new kMyMoneyEdit;
  m_editWidgets["payment"] = value;
  value->setResetButtonVisible(false);
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdatePayment(const QString&)));
  connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));

  value = new kMyMoneyEdit;
  m_editWidgets["deposit"] = value;
  value->setResetButtonVisible(false);
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateDeposit(const QString&)));
  connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));

  KMyMoneyCashFlowCombo* cashflow = new KMyMoneyCashFlowCombo(0, 0, m_account.accountGroup());
  m_editWidgets["cashflow"] = cashflow;
  connect(cashflow, SIGNAL(directionSelected(KMyMoneyRegister::CashFlowDirection)), this, SLOT(slotUpdateCashFlow(KMyMoneyRegister::CashFlowDirection)));
  connect(cashflow, SIGNAL(directionSelected(KMyMoneyRegister::CashFlowDirection)), this, SLOT(slotUpdateButtonState()));

  KMyMoneyReconcileCombo* reconcile = new KMyMoneyReconcileCombo;
  m_editWidgets["status"] = reconcile;
  connect(reconcile, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotUpdateButtonState()));

  // if we don't have more than 1 selected transaction, we don't need
  // the "don't change" item in some of the combo widgets
  if(!isMultiSelection()) {
    reconcile->removeDontCare();
    cashflow->removeDontCare();
  }

  m_editWidgets["categoryLabel"] = new QLabel(i18n("Category"), 0);

  // create a copy of tabbar above the form (if we are created for a form)
  KMyMoneyTransactionForm::TransactionForm* form = dynamic_cast<KMyMoneyTransactionForm::TransactionForm*>(m_regForm);
  if(form) {
    KMyMoneyTransactionForm::TabBar* tabbar = new KMyMoneyTransactionForm::TabBar;
    m_editWidgets["tabbar"] = tabbar;
    tabbar->copyTabs(form->tabBar());
    connect(tabbar, SIGNAL(tabSelected(int)), this, SLOT(slotUpdateAction(int)));
  }
}

void StdTransactionEditor::setupCategoryWidget(QCString& categoryId)
{
  TransactionEditor::setupCategoryWidget(dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]), m_splits, categoryId, SLOT(slotEditSplits()));

  if(m_splits.count() == 1)
    m_shares = m_splits[0].shares();
}

bool StdTransactionEditor::isTransfer(const QCString& accId1, const QCString& accId2) const
{
  if(accId1.isEmpty() || accId2.isEmpty())
    return false;

  return MyMoneyFile::instance()->account(accId1).isIncomeExpense() == MyMoneyFile::instance()->account(accId2).isIncomeExpense();
}

void StdTransactionEditor::loadEditWidgets(KMyMoneyRegister::Action action)
{
  // don't kick off VAT processing from here
  m_inUpdateVat = true;

  QMap<QString, QWidget*>::const_iterator it_w;
  QWidget* w;

  // load the payee widget
  KMyMoneyPayeeCombo* payee = dynamic_cast<KMyMoneyPayeeCombo*>(m_editWidgets["payee"]);
  payee->loadPayees(MyMoneyFile::instance()->payeeList());

  // load the category widget
  KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
  disconnect(category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));

  // check if the current transaction has a reference to an equity account
  bool haveEquityAccount = false;
  QValueList<MyMoneySplit>::const_iterator it_s;
  for(it_s = m_transaction.splits().begin(); !haveEquityAccount && it_s != m_transaction.splits().end(); ++it_s) {
    MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
    if(acc.accountType() == MyMoneyAccount::Equity)
      haveEquityAccount = true;
  }

  AccountSet aSet;
  aSet.addAccountGroup(MyMoneyAccount::Asset);
  aSet.addAccountGroup(MyMoneyAccount::Liability);
  aSet.addAccountGroup(MyMoneyAccount::Income);
  aSet.addAccountGroup(MyMoneyAccount::Expense);
  if(KMyMoneySettings::expertMode() || haveEquityAccount)
    aSet.addAccountGroup(MyMoneyAccount::Equity);
  aSet.load(category->selector());

  // if an account is specified then remove it from the widget so that the user
  // cannot create a transfer with from and to account being the same account
  if(!m_account.id().isEmpty())
    category->selector()->removeItem(m_account.id());

  if(!isMultiSelection()) {
    dynamic_cast<KTextEdit*>(m_editWidgets["memo"])->setText(m_split.memo());
    if(m_transaction.postDate().isValid())
      dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->setDate(m_transaction.postDate());
    else if(m_lastPostDate.isValid())
      dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->setDate(m_lastPostDate);
    else
      dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->setDate(QDate::currentDate());

    if((w = haveWidget("number")) != 0) {
      dynamic_cast<kMyMoneyLineEdit*>(w)->loadText(m_split.number());
      if(m_transaction.id().isEmpty()                            // new transaction
      && dynamic_cast<kMyMoneyLineEdit*>(w)->text().isEmpty()    // no number filled in
      && m_account.accountType() == MyMoneyAccount::Checkings    // checkings account
      && KMyMoneyGlobalSettings::autoIncCheckNumber()) {         // and auto inc number turned on?
        assignNumber();
      }
    }
    dynamic_cast<KMyMoneyReconcileCombo*>(m_editWidgets["status"])->setState(m_split.reconcileFlag());

    QCString payeeId = m_split.payeeId();
    if(!payeeId.isEmpty()) {
      payee->setSelectedItem(payeeId);
    }

    m_splits.clear();
    if(m_transaction.splitCount() < 2) {
      category->completion()->setSelected(QCString());
    } else {
      QValueList<MyMoneySplit>::const_iterator it_s;
      for(it_s = m_transaction.splits().begin(); it_s != m_transaction.splits().end(); ++it_s) {
        if((*it_s) == m_split)
          continue;
        m_splits.append(*it_s);
      }
    }
    QCString categoryId;
    setupCategoryWidget(categoryId);

    if((w = haveWidget("cashflow")) != 0) {
      KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(w);
      cashflow->setDirection(m_split.value().isNegative() ? KMyMoneyRegister::Payment : KMyMoneyRegister::Deposit);
    }

    if((w = haveWidget("categoryLabel")) != 0) {
      QLabel *categoryLabel = dynamic_cast<QLabel*>(w);
      if(isTransfer(m_split.accountId(), categoryId)) {
        if(m_split.value().isPositive())
          categoryLabel->setText(i18n("Transfer from"));
        else
          categoryLabel->setText(i18n("Transfer to"));
      }
    }

    MyMoneyMoney value = m_split.shares();

    if(haveWidget("deposit")) {
      if(m_split.shares().isNegative()) {
        dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->loadText("");
        dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->setValue(value.abs());
      } else {
        dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->setValue(value.abs());
        dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->loadText("");
      }
    }
    if((w = haveWidget("amount")) != 0) {
      dynamic_cast<kMyMoneyEdit*>(w)->setValue(value.abs());
    }

    slotUpdateCategory(categoryId);

    // try to preset for specific action if a new transaction is being started
    if(m_transaction.id().isEmpty()) {
      if((w = haveWidget("categoryLabel")) != 0) {
        TabBar* tabbar = dynamic_cast<TabBar*>(haveWidget("tabbar"));
        if(action == KMyMoneyRegister::ActionNone) {
          if(tabbar) {
            action = static_cast<KMyMoneyRegister::Action>(tabbar->currentTab());
          }
        }
        if(action != KMyMoneyRegister::ActionNone) {
          QLabel *categoryLabel = dynamic_cast<QLabel*>(w);
          if(action == KMyMoneyRegister::ActionTransfer)
            categoryLabel->setText(i18n("Transfer to"));
          if((w = haveWidget("cashflow")) != 0) {
            KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(w);
            if(action == KMyMoneyRegister::ActionDeposit)
              cashflow->setDirection(KMyMoneyRegister::Deposit);
            else
              cashflow->setDirection(KMyMoneyRegister::Payment);
          }
          if(tabbar) {
            tabbar->setCurrentTab(action);
          }
        }
      }
    } else {
      TabBar* tabbar = dynamic_cast<TabBar*>(haveWidget("tabbar"));
      if(tabbar) {
        if(!isTransfer(m_split.accountId(), categoryId)) {
          tabbar->setCurrentTab(m_split.value().isNegative() ? KMyMoneyRegister::ActionWithdrawal : KMyMoneyRegister::ActionDeposit);
        } else {
          tabbar->setCurrentTab(KMyMoneyRegister::ActionTransfer);
        }
      }
    }

  } else {
    dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->loadDate(QDate());
    dynamic_cast<KMyMoneyReconcileCombo*>(m_editWidgets["status"])->setState(MyMoneySplit::Unknown);
    if(haveWidget("deposit")) {
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->loadText("");
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->setEmptyAllowed();
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->loadText("");
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->setEmptyAllowed();
    }
    if((w = haveWidget("amount")) != 0) {
      dynamic_cast<kMyMoneyEdit*>(w)->loadText("");
      dynamic_cast<kMyMoneyEdit*>(w)->setEmptyAllowed();
    }

    if((w = haveWidget("cashflow")) != 0) {
      KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(w);
      cashflow->setDirection(KMyMoneyRegister::Unknown);
    }
    if((w = haveWidget("tabbar")) != 0) {
      w->setEnabled(false);
    }

    category->completion()->setSelected(QCString());
  }

  // allow kick off VAT processing again
  m_inUpdateVat = false;
}

QWidget* StdTransactionEditor::firstWidget(void) const
{
  QWidget* w = 0;
  if(m_initialAction != KMyMoneyRegister::ActionNone)
    w = haveWidget("payee");
  return w;
}

void StdTransactionEditor::slotReloadEditWidgets(void)
{
  // reload category widget
  KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
  QCString categoryId = category->selectedItem();

  AccountSet aSet;
  aSet.addAccountGroup(MyMoneyAccount::Asset);
  aSet.addAccountGroup(MyMoneyAccount::Liability);
  aSet.addAccountGroup(MyMoneyAccount::Income);
  aSet.addAccountGroup(MyMoneyAccount::Expense);
  if(KMyMoneySettings::expertMode())
    aSet.addAccountGroup(MyMoneyAccount::Equity);
  aSet.load(category->selector());

  // if an account is specified then remove it from the widget so that the user
  // cannot create a transfer with from and to account being the same account
  if(!m_account.id().isEmpty())
    category->selector()->removeItem(m_account.id());

  if(!categoryId.isEmpty())
    category->setSelectedItem(categoryId);


  // reload payee widget
  KMyMoneyPayeeCombo* payee = dynamic_cast<KMyMoneyPayeeCombo*>(m_editWidgets["payee"]);
  QCString payeeId = payee->selectedItem();

  payee->loadPayees(MyMoneyFile::instance()->payeeList());

  if(!payeeId.isEmpty()) {
    payee->setSelectedItem(payeeId);
  }
}

void StdTransactionEditor::slotUpdatePayee(const QCString& payeeId)
{
  // we have a new payee assigned to this transaction.
  // in case there is no category assigned, no value entered and no
  // memo available, we search for the last transaction of this payee
  // in the account.
  if(m_transaction.id().isEmpty()
  && m_splits.count() == 0
  && KMyMoneySettings::autoFillTransaction()) {
    // check if category is empty
    KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
    QCStringList list;
    category->selectedItems(list);
    if(!list.isEmpty())
      return;

    // check if memo is empty
    KTextEdit* memo = dynamic_cast<KTextEdit*>(m_editWidgets["memo"]);
    if(memo && !memo->text().isEmpty())
      return;

    // check if all value fields are empty
    kMyMoneyEdit* amount;
    QStringList fields;
    fields << "amount" << "payment" << "deposit";
    QStringList::const_iterator it_f;
    for(it_f = fields.begin(); it_f != fields.end(); ++it_f) {
      amount = dynamic_cast<kMyMoneyEdit*>(haveWidget(*it_f));
      if(amount && !amount->value().isZero())
        return;
    }

    // check if date has been altered by user
    kMyMoneyDateInput* postDate = dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"]);
    if((m_lastPostDate.isValid() && (postDate->date() != m_lastPostDate))
    || (!m_lastPostDate.isValid() && (postDate->date() != QDate::currentDate())))
      return;

    // if we got here, we have to autofill
    autoFill(payeeId);
  }
}

void StdTransactionEditor::autoFill(const QCString& payeeId)
{
  QValueList<QPair<MyMoneyTransaction, MyMoneySplit> >  list;
  MyMoneyTransactionFilter filter(m_account.id());
  filter.addPayee(payeeId);
  MyMoneyFile::instance()->transactionList(list, filter);
  if(!list.empty()) {
    // ok, we found at least one previous transaction. now we clear out
    // what we have collected so far and add those splits from
    // the previous transaction.
    QValueList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator  it_t;
    QMap<QString, const MyMoneyTransaction* > uniqList;

    for(it_t = list.begin(); it_t != list.end(); ++it_t) {
      uniqList[(*it_t).first.accountSignature()] = &((*it_t).first);
    }

    MyMoneyTransaction t;
    if(uniqList.count() == 1) {
      t = list.last().first;
    } else {
      KSelectTransactionsDlg dlg(m_account, m_regForm);
      dlg.setCaption(i18n("Select autofill transaction"));

      QMap<QString, const MyMoneyTransaction*>::const_iterator it_u;
      for(it_u = uniqList.begin(); it_u != uniqList.end(); ++it_u) {
        dlg.addTransaction(*(*it_u));
      }

      // setup sort order
      dlg.m_register->setSortOrder("1,-9,-4");
      // sort the transactions according to the sort setting
      dlg.m_register->sortItems();

      // and select the last item
      if(dlg.m_register->lastItem())
        dlg.m_register->selectItem(dlg.m_register->lastItem());

      if(dlg.exec() == QDialog::Accepted) {
        t = dlg.transaction();
      }
    }

    if(t != MyMoneyTransaction()) {
      m_transaction.removeSplits();
      m_split = MyMoneySplit();
      MyMoneySplit otherSplit;
      QValueList<MyMoneySplit>::ConstIterator it;
      for(it = t.splits().begin(); it != t.splits().end(); ++it) {
        MyMoneySplit s(*it);
        s.setReconcileFlag(MyMoneySplit::NotReconciled);
        s.setReconcileDate(QDate());
        s.clearId();
        s.setBankID(QString());
        // older versions of KMyMoney used to set the action
        // we don't need this anymore
        if(s.action() != MyMoneySplit::ActionAmortization
        && s.action() != MyMoneySplit::ActionInterest)  {
          s.setAction(QCString());
        }

        // FIXME update check number. The old comment contained
        //
        // <quote>
        // If a check number is already specified by the user it is
        // used. If the input field is empty and the previous transaction
        // contains a checknumber, the next usuable check no will be assigned
        // to the transaction.
        // </quote>

        kMyMoneyLineEdit* editNr = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));
        if(editNr && !editNr->text().isEmpty()) {
          s.setNumber(editNr->text());
        } else if(!s.number().isEmpty()) {
          s.setNumber(KMyMoneyUtils::nextCheckNumber(m_account));
        }

        // if the transaction has exactly two splits, remove
        // the memo text of the split that does not reference
        // the current account. This allows the user to change
        // the autofilled memo text which will then also be used
        // in this split. See createTransaction() for this logic.
        if(s.accountId() != m_account.id() && t.splitCount() == 2)
          s.setMemo(QString());

        m_transaction.addSplit(s);
        if(s.accountId() == m_account.id() && m_split == MyMoneySplit()) {
          m_split = s;
        } else {
          otherSplit = s;
        }
      }

      // make sure to extract the right action
      KMyMoneyRegister::Action action;
      action = m_split.shares().isNegative() ? KMyMoneyRegister::ActionWithdrawal : KMyMoneyRegister::ActionDeposit;

      if(m_transaction.splitCount() == 2) {
        MyMoneyAccount acc = MyMoneyFile::instance()->account(otherSplit.accountId());
        if(acc.isAssetLiability())
          action = KMyMoneyRegister::ActionTransfer;
      }

      // now setup the widgets with the new data
      loadEditWidgets(action);
    }
  }

  // focus jumps into the category field
  QWidget* w;
  if((w = haveWidget("payee")) != 0) {
    w->setFocus();
  }
}

void StdTransactionEditor::slotUpdateAction(int action)
{
  TabBar* tabbar = dynamic_cast<TabBar*>(haveWidget("tabbar"));
  if(tabbar) {
    QLabel* categoryLabel = dynamic_cast<QLabel*>(haveWidget("categoryLabel"));
    KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(m_editWidgets["cashflow"]);
    switch(action) {
      case KMyMoneyRegister::ActionDeposit:
        categoryLabel->setText(i18n("Category"));
        cashflow->setDirection(KMyMoneyRegister::Deposit);
        break;
      case KMyMoneyRegister::ActionTransfer:
        categoryLabel->setText(i18n("Transfer from"));
        slotUpdateCashFlow(cashflow->direction());
        break;
      case KMyMoneyRegister::ActionWithdrawal:
        categoryLabel->setText(i18n("Category"));
        cashflow->setDirection(KMyMoneyRegister::Payment);
        break;
    }
  }
}

void StdTransactionEditor::slotUpdateCashFlow(KMyMoneyRegister::CashFlowDirection dir)
{
  QLabel* categoryLabel = dynamic_cast<QLabel*>(haveWidget("categoryLabel"));

  // qDebug("Update cashflow to %d", dir);
  if(categoryLabel) {
    TabBar* tabbar = dynamic_cast<TabBar*>(haveWidget("tabbar"));
    if(categoryLabel->text() != i18n("Category")) {
      tabbar->setCurrentTab(KMyMoneyRegister::ActionTransfer);
      if(dir == KMyMoneyRegister::Deposit) {
        categoryLabel->setText(i18n("Transfer from"));
      } else {
        categoryLabel->setText(i18n("Transfer to"));
      }
    } else {
      if(dir == KMyMoneyRegister::Deposit)
        tabbar->setCurrentTab(KMyMoneyRegister::ActionDeposit);
      else
        tabbar->setCurrentTab(KMyMoneyRegister::ActionWithdrawal);
    }
  }
}

void StdTransactionEditor::slotUpdateCategory(const QCString& id)
{
  QLabel *categoryLabel = dynamic_cast<QLabel*>(haveWidget("categoryLabel"));
  // qDebug("Update category to %s", id.data());

  if(categoryLabel) {
    TabBar* tabbar = dynamic_cast<TabBar*>(haveWidget("tabbar"));
    kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(m_editWidgets["amount"]);
    MyMoneyMoney val = amount->value();

    if(categoryLabel->text() == i18n("Transfer from")) {
      val = -val;
    } else {
      val = val.abs();
    }

    if(tabbar) {
      tabbar->tab(KMyMoneyRegister::ActionTransfer)->setEnabled(true);
      tabbar->tab(KMyMoneyRegister::ActionDeposit)->setEnabled(true);
      tabbar->tab(KMyMoneyRegister::ActionWithdrawal)->setEnabled(true);
    }

    if(!id.isEmpty()) {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
      if(acc.isAssetLiability()
      || acc.accountGroup() == MyMoneyAccount::Equity) {
        if(tabbar) {
          tabbar->tab(KMyMoneyRegister::ActionDeposit)->setEnabled(false);
          tabbar->tab(KMyMoneyRegister::ActionWithdrawal)->setEnabled(false);
        }
        if(val.isNegative())
          categoryLabel->setText(i18n("Transfer from"));
        else
          categoryLabel->setText(i18n("Transfer to"));
      } else {
        if(tabbar)
          tabbar->tab(KMyMoneyRegister::ActionTransfer)->setEnabled(false);
        categoryLabel->setText(i18n("Category"));
      }
      updateAmount(val.abs());
    } else {
      KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
      if(!category->currentText().isEmpty() && tabbar)
        tabbar->tab(KMyMoneyRegister::ActionTransfer)->setEnabled(false);
      categoryLabel->setText(i18n("Category"));
    }
    if(tabbar)
      tabbar->update();
  }
  updateVAT(false);
}

void StdTransactionEditor::slotUpdatePayment(const QString& txt)
{
  MyMoneyMoney val(txt);

  if(val.isNegative()) {
    dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->setValue(val.abs());
    dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->clearText();
  } else {
    dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->clearText();
  }
  updateVAT();
}

void StdTransactionEditor::slotUpdateDeposit(const QString& txt)
{
  MyMoneyMoney val(txt);
  if(val.isNegative()) {
    dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->setValue(val.abs());
    dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->clearText();
  } else {
    dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->clearText();
  }
  updateVAT();
}

void StdTransactionEditor::slotUpdateAmount(const QString& txt)
{
  // qDebug("Update amount to %s", txt.data());
  MyMoneyMoney val(txt);
  updateAmount(val);
  updateVAT(true);
}

void StdTransactionEditor::updateAmount(const MyMoneyMoney& val)
{
  QLabel *categoryLabel = dynamic_cast<QLabel*>(haveWidget("categoryLabel"));
  if(categoryLabel) {
    KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(m_editWidgets["cashflow"]);

    if(val.isNegative()) {
      if(categoryLabel->text() != i18n("Category")) {
        if(categoryLabel->text() == i18n("Transfer from")) {
          categoryLabel->setText(i18n("Transfer to"));
          cashflow->setDirection(KMyMoneyRegister::Payment);
        } else {
          categoryLabel->setText(i18n("Transfer from"));
          cashflow->setDirection(KMyMoneyRegister::Deposit);
        }
      } else {
        if(cashflow->direction() == KMyMoneyRegister::Deposit)
          cashflow->setDirection(KMyMoneyRegister::Payment);
        else
          cashflow->setDirection(KMyMoneyRegister::Deposit);
      }
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["amount"])->setValue(val.abs());
    }
  }
}

void StdTransactionEditor::updateVAT(bool amountChanged)
{
  // make sure that we don't do this recursively
  if(m_inUpdateVat)
    return;

  // we don't do anything if we have multiple transactions selected
  if(isMultiSelection())
    return;

  // if auto vat assignment for this account is turned off
  // we don't care about taxes
  if(m_account.value("NoVat") == "Yes")
    return;

  // more splits than category and tax are not supported
  if(m_splits.count() > 2)
    return;

  // in order to do anything, we need an amount
  MyMoneyMoney amount, newAmount;
  bool amountOk;
  amount = amountFromWidget(&amountOk);
  if(!amountOk)
    return;

  // If the transaction has a tax and a category split, remove the tax split
  if(m_splits.count() == 2) {
    newAmount = removeVatSplit();
    if(m_splits.count() == 2)  // not removed?
      return;

  } else {
    // otherwise, we need a category
    KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
    if(category->selectedItem().isEmpty())
      return;

    // if no VAT account is associated with this category/account, then we bail out
    MyMoneyAccount cat = MyMoneyFile::instance()->account(category->selectedItem());
    if(cat.value("VatAccount").isEmpty())
      return;

    newAmount = amount;
  }

  // seems we have everything we need
  if(amountChanged)
    newAmount = amount;

  MyMoneyTransaction transaction;
  if(createTransaction(transaction, m_transaction, m_split)) {
    if(addVatSplit(transaction, newAmount)) {
      m_transaction = transaction;
      m_split = m_transaction.splits()[0];

      loadEditWidgets();

      // if we made this a split transaction, then move the
      // focus to the memo field
      if(qApp->focusWidget() == haveWidget("category")) {
        QWidget* w = haveWidget("memo");
        if(w)
          w->setFocus();
      }
    }
  }
}

bool StdTransactionEditor::addVatSplit(MyMoneyTransaction& tr, const MyMoneyMoney& amount)
{
  if(tr.splitCount() != 2)
    return false;

  bool rc = false;
  MyMoneyFile* file = MyMoneyFile::instance();

  try {
    MyMoneySplit cat;  // category
    MyMoneySplit tax;  // tax

    // extract the category split from the transaction
    MyMoneyAccount category = file->account(tr.splitByAccount(m_account.id(), false).accountId());
    if(category.value("VatAccount").isEmpty())
      return false;
    MyMoneyAccount vatAcc = file->account(category.value("VatAccount").latin1());
    MyMoneySecurity asec = file->security(m_account.currencyId());
    MyMoneySecurity csec = file->security(category.currencyId());
    MyMoneySecurity vsec = file->security(vatAcc.currencyId());
    if(asec.id() != csec.id() || asec.id() != vsec.id()) {
      qDebug("Auto VAT assignment only works if all three accounts use the same currency.");
      return false;
    }

    MyMoneyMoney vatRate;
    MyMoneyMoney gv, nv;    // gross value, net value
    int fract = m_account.fraction(asec);

    vatRate.fromString(vatAcc.value("VatRate"));
    if(!vatRate.isZero()) {

      tax.setAccountId(vatAcc.id());

      // qDebug("vat amount is '%s'", category.value("VatAmount").latin1());
      if(category.value("VatAmount").lower() != QString("net")) {
        // split value is the gross value
        gv = amount;
        nv = gv / (MyMoneyMoney(1,1) + vatRate);
        MyMoneySplit catSplit = tr.splitByAccount(m_account.id(), false);
        catSplit.setShares(-nv.convert(fract));
        catSplit.setValue(catSplit.shares());
        tr.modifySplit(catSplit);

      } else {
        // split value is the net value
        nv = amount;
        gv = nv * (MyMoneyMoney(1,1) + vatRate);
        MyMoneySplit accSplit = tr.splitByAccount(m_account.id());
        accSplit.setValue(gv.convert(fract));
        accSplit.setShares(accSplit.value());
        tr.modifySplit(accSplit);
      }

      tax.setValue(-(gv - nv).convert(fract));
      tax.setShares(tax.value());
      tr.addSplit(tax);
      rc = true;
    }
  } catch(MyMoneyException *e) {
    delete e;
  }
  return rc;
}

MyMoneyMoney StdTransactionEditor::removeVatSplit(void)
{
  // we only deal with splits that have three splits
  if(m_splits.count() != 2)
    return amountFromWidget();

  MyMoneySplit c; // category split
  MyMoneySplit t; // tax split

  bool netValue = false;
  QValueList<MyMoneySplit>::const_iterator it_s;
  for(it_s = m_splits.begin(); it_s != m_splits.end(); ++it_s) {
    MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
    if(!acc.value("VatAccount").isEmpty()) {
      netValue = (acc.value("VatAmount").lower() == "net");
      c = (*it_s);
    } else if(!acc.value("VatRate").isEmpty()) {
      t = (*it_s);
    }
  }

  // bail out if not all splits are setup
  if(c.id().isEmpty() || t.id().isEmpty())
    return amountFromWidget();

  MyMoneyMoney amount;
  // reduce the splits
  if(netValue) {
    amount = -c.shares();
  } else {
    amount = -(c.shares() + t.shares());
  }

  // remove tax split from the list, ...
  m_splits.clear();
  m_splits.append(c);

  // ... make sure that the widget is updated ...
  // block the signals to avoid popping up the split editor dialog
  // for nothing
  m_editWidgets["category"]->blockSignals(true);
  QCString id;
  setupCategoryWidget(id);
  m_editWidgets["category"]->blockSignals(false);

  // ... and return the updated amount
  return amount;
}

bool StdTransactionEditor::isComplete(void) const
{
  QMap<QString, QWidget*>::const_iterator it_w;
  for(it_w = m_editWidgets.begin(); it_w != m_editWidgets.end(); ++it_w) {
    KMyMoneyPayeeCombo* payee = dynamic_cast<KMyMoneyPayeeCombo*>(*it_w);
    KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(*it_w);
    kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(*it_w);
    KMyMoneyReconcileCombo* reconcile = dynamic_cast<KMyMoneyReconcileCombo*>(*it_w);
    KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(*it_w);

    if(payee && !(payee->currentText().isEmpty()))
      break;

    if(category && !category->lineEdit()->text().isEmpty())
      break;

    if(amount && !(amount->value().isZero()))
      break;

    // the following two widgets are only checked if we are editing multiple transactions
    if(isMultiSelection()) {
      if(reconcile && reconcile->state() != MyMoneySplit::Unknown)
        break;

      if(cashflow && cashflow->direction() != KMyMoneyRegister::Unknown)
        break;
    }
  }
  return it_w != m_editWidgets.end();
}

void StdTransactionEditor::slotCreateCategory(const QString& name, QCString& id)
{
  MyMoneyAccount acc, parent;
  acc.setName(name);

  KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(haveWidget("cashflow"));
  if(cashflow) {
    // form based input
    if(cashflow->direction() == KMyMoneyRegister::Deposit)
      parent = MyMoneyFile::instance()->income();
    else
      parent = MyMoneyFile::instance()->expense();

  } else if(haveWidget("deposit")) {
    // register based input
    kMyMoneyEdit* deposit = dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"]);
    if(deposit->value().isPositive())
      parent = MyMoneyFile::instance()->income();
    else
      parent = MyMoneyFile::instance()->expense();

  } else
    parent = MyMoneyFile::instance()->expense();

  // TODO extract possible first part of a hierarchy and check if it is one
  // of our top categories. If so, remove it and select the parent
  // according to this information.

  emit createCategory(acc, parent);

  // return id
  id = acc.id();
}

int StdTransactionEditor::slotEditSplits(void)
{
  int rc = QDialog::Rejected;

  if(!m_openEditSplits) {
    // only get in here in a single instance
    m_openEditSplits = true;

    // force focus change to update all data
    QWidget* w = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"])->splitButton();
    if(w)
      w->setFocus();

    kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(haveWidget("amount"));
    kMyMoneyEdit* deposit = dynamic_cast<kMyMoneyEdit*>(haveWidget("deposit"));
    kMyMoneyEdit* payment = dynamic_cast<kMyMoneyEdit*>(haveWidget("payment"));
    KMyMoneyCashFlowCombo* cashflow = 0;
    KMyMoneyRegister::CashFlowDirection dir = KMyMoneyRegister::Unknown;
    bool isValidAmount = false;

    if(amount) {
      isValidAmount = amount->text().length() != 0;
      cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(haveWidget("cashflow"));
      if(cashflow)
        dir = cashflow->direction();

    } else {
      if(deposit) {
        if (deposit->text().length() != 0) {
          isValidAmount = true;
          dir = KMyMoneyRegister::Deposit;
        }
      }
      if(payment) {
        if (payment->text().length() != 0) {
          isValidAmount = true;
          dir = KMyMoneyRegister::Payment;
        }
      }
      if(!deposit || !payment) {
        qDebug("Internal error: deposit(%p) & payment(%p) widgets not found but required", deposit, payment);
        return rc;
      }
    }

    if(dir == KMyMoneyRegister::Unknown)
      dir = KMyMoneyRegister::Payment;

    MyMoneyTransaction transaction;
    if(createTransaction(transaction, m_transaction, m_split)) {
      MyMoneyMoney value;

      KSplitTransactionDlg* dlg = new KSplitTransactionDlg(transaction,
                                                          m_account,
                                                          isValidAmount,
                                                          dir == KMyMoneyRegister::Deposit,
                                                          0,
                                                          m_priceInfo,
                                                          m_regForm);
      connect(dlg, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));
      connect(dlg, SIGNAL(createCategory(MyMoneyAccount&, const MyMoneyAccount&)), this, SIGNAL(createCategory(MyMoneyAccount&, const MyMoneyAccount&)));

      if((rc = dlg->exec()) == QDialog::Accepted) {
        m_transaction = dlg->transaction();
        m_split = m_transaction.splits()[0];
        loadEditWidgets();
      }

      delete dlg;
    }

    // focus jumps into the memo field
    if((w = haveWidget("memo")) != 0) {
      w->setFocus();
    }

    m_openEditSplits = false;
  }

  return rc;
}

void StdTransactionEditor::checkPayeeInSplit(MyMoneySplit& s, const QCString& payeeId)
{
  if(s.accountId().isEmpty() || payeeId.isEmpty())
    return;

  MyMoneyAccount acc = MyMoneyFile::instance()->account(s.accountId());
  if(acc.isIncomeExpense()) {
    s.setPayeeId(payeeId);
  } else {
    if(s.payeeId().isEmpty())
      s.setPayeeId(payeeId);
  }
}

MyMoneyMoney StdTransactionEditor::amountFromWidget(bool* update) const
{
  bool updateValue = false;
  MyMoneyMoney value;

  KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(haveWidget("cashflow"));
  if(cashflow) {
    // form based input
    kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(m_editWidgets["amount"]);
    // if both fields do not contain changes -> no need to update
    if(cashflow->direction() != KMyMoneyRegister::Unknown
    && !amount->text().isEmpty())
      updateValue = true;
    value = amount->value();
    if(cashflow->direction() == KMyMoneyRegister::Payment)
      value = -value;

  } else if(haveWidget("deposit")) {
    // register based input
    kMyMoneyEdit* deposit = dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"]);
    kMyMoneyEdit* payment = dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"]);
    // if both fields do not contain text -> no need to update
    if(!(deposit->lineedit()->text().isEmpty() && payment->lineedit()->text().isEmpty()))
      updateValue = true;

    if(deposit->value().isPositive())
      value = deposit->value();
    else
      value = -(payment->value());
  }

  if(update)
    *update = updateValue;

  // determine the max fraction for this account and
  // adjust the value accordingly
  const MyMoneySecurity& sec = MyMoneyFile::instance()->security(m_account.currencyId());
  return value.convert(m_account.fraction(sec));
}

bool StdTransactionEditor::createTransaction(MyMoneyTransaction& t, const MyMoneyTransaction& torig, const MyMoneySplit& sorig, bool skipPriceDialog)
{
  // extract price info from original transaction
  m_priceInfo.clear();
  QValueList<MyMoneySplit>::const_iterator it_s;
  if(!torig.id().isEmpty()) {
    for(it_s = torig.splits().begin(); it_s != torig.splits().end(); ++it_s) {
      if((*it_s).id() != sorig.id()) {
        MyMoneyAccount cat = MyMoneyFile::instance()->account((*it_s).accountId());
        if(cat.currencyId() != m_account.currencyId()) {
          if(!(*it_s).shares().isZero() && !(*it_s).value().isZero()) {
            m_priceInfo[cat.currencyId()] = ((*it_s).shares() / (*it_s).value()).reduce();
          }
        }
      }
    }
  }

  t = torig;

  t.removeSplits();
  t.setCommodity(m_account.currencyId());

  kMyMoneyDateInput* postDate = dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"]);
  if(postDate->date().isValid()) {
    t.setPostDate(postDate->date());
  }

  // we start with the previous values, make sure we can add them later on
  MyMoneySplit s0 = sorig;
  s0.clearId();

  // make sure we reference this account here
  s0.setAccountId(m_account.id());

  // memo and number field are special: if we have multiple transactions selected
  // and the edit field is empty, we treat it as "not modified".
  // FIXME a better approach would be to have a 'dirty' flag with the widgets
  //       which identifies if the originally loaded value has been modified
  //       by the user
  KTextEdit* memo = dynamic_cast<KTextEdit*>(m_editWidgets["memo"]);
  if(memo) {
    if(!isMultiSelection() || (isMultiSelection() && !memo->text().isEmpty() ) )
      s0.setMemo(memo->text());
  }

  kMyMoneyLineEdit* number = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));
  if(number) {
    if(!isMultiSelection() || (isMultiSelection() && !number->text().isEmpty() ) )
      s0.setNumber(number->text());
  }

  KMyMoneyPayeeCombo* payee = dynamic_cast<KMyMoneyPayeeCombo*>(m_editWidgets["payee"]);
  QCString payeeId;
  if(!isMultiSelection() || (isMultiSelection() && !payee->currentText().isEmpty())) {
    payeeId = payee->selectedItem();
    s0.setPayeeId(payeeId);
  }

  bool updateValue;
  MyMoneyMoney value = amountFromWidget(&updateValue);

  if(updateValue) {
    // for this account, the shares and value is the same
    s0.setValue(value);
    s0.setShares(value);
  } else {
    value = s0.value();
  }

  // if we mark the split reconciled here, we'll use today's date if no reconciliation date is given
  KMyMoneyReconcileCombo* status = dynamic_cast<KMyMoneyReconcileCombo*>(m_editWidgets["status"]);
  if(status->state() != MyMoneySplit::Unknown)
    s0.setReconcileFlag(status->state());

  if(s0.reconcileFlag() == MyMoneySplit::Reconciled && !s0.reconcileDate().isValid())
    s0.setReconcileDate(QDate::currentDate());

  checkPayeeInSplit(s0, payeeId);

  // add the split to the transaction
  t.addSplit(s0);

  // if we have no other split we create it
  // if we have none or only one other split, we reconstruct it here
  // if we have more than one other split, we take them as they are
  // make sure to perform all those changes on a local copy
  QValueList<MyMoneySplit> splits = m_splits;

  MyMoneySplit s1;
  if(torig.splitCount() < 2 && splits.count() == 0) {
    s1.setMemo(s0.memo());
    splits.append(s1);

    // make sure we will fill the value and share fields later on
    updateValue = true;
  }

  // FIXME in multiSelection we currently only support transactions with one
  // or two splits. So we check the original transaction and extract the other
  // split or create it
  if(isMultiSelection()) {
    if(torig.splitCount() == 2) {
      QValueList<MyMoneySplit>::const_iterator it_s;
      for(it_s = torig.splits().begin(); it_s != torig.splits().end(); ++it_s) {
        if((*it_s).id() == sorig.id())
          continue;
        s1 = *it_s;
        s1.clearId();
        break;
      }
    }
  } else {
    if(splits.count() == 1) {
      s1 = splits[0];
      s1.clearId();
    }
  }

  if(isMultiSelection() || splits.count() == 1) {
    KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
    if(!isMultiSelection() || (isMultiSelection() && !category->currentText().isEmpty())) {
      s1.setAccountId(category->selectedItem());
    }

    // if the first split has a memo but the second split is empty,
    // we just copy the memo text over
    if(memo) {
      if(!isMultiSelection() || (isMultiSelection() && !memo->text().isEmpty())) {
        if(s1.memo().isEmpty())
          s1.setMemo(memo->text());
      }
    }

    if(updateValue && !s1.accountId().isEmpty()) {
      s1.setValue(-value);
      MyMoneyMoney shares;
      if(!skipPriceDialog) {
        if(!KCurrencyCalculator::setupSplitPrice(shares, t, s1, m_priceInfo, m_regForm))
          return false;
      } else {
        MyMoneyAccount cat = MyMoneyFile::instance()->account(s1.accountId());
        shares = (s1.value() * m_priceInfo[cat.currencyId()]).reduce();
      }
      s1.setShares(shares);
    }

    checkPayeeInSplit(s1, payeeId);

    if(!s1.accountId().isEmpty())
      t.addSplit(s1);

  } else {
    QValueList<MyMoneySplit>::iterator it_s;
    for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
      s1 = *it_s;
      s1.clearId();
      checkPayeeInSplit(s1, payeeId);
      t.addSplit(s1);
    }
  }
  return true;
}


#include "transactioneditor.moc"
