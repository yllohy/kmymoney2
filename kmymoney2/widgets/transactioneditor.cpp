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
#include <kmymoney/kmymoneypayee.h>
#include <kmymoney/kmymoneycategory.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneylineedit.h>
#include <kmymoney/kmymoneyaccountcompletion.h>
#include <kmymoney/kmymoneyaccountselector.h>
#include <kmymoney/mymoneyfile.h>

#include "../dialogs/ksplittransactiondlg.h"
#include "../dialogs/kcurrencycalculator.h"

#include "../kmymoneysettings.h"

TransactionEditor::TransactionEditor(TransactionEditorContainer* regForm, MyMoneyObjectContainer* objects, KMyMoneyRegister::Transaction* item, const QValueList<KMyMoneyRegister::SelectedTransaction>& list, const QDate& lastPostDate) :
  m_transactions(list),
  m_regForm(regForm),
  m_item(item),
  m_objects(objects),
  m_transaction(item->transaction()),
  m_split(item->split()),
  m_lastPostDate(lastPostDate)
{
}

TransactionEditor::~TransactionEditor()
{
  m_regForm->removeEditWidgets();

  // FIXME remove tabbar
  // m_regForm->setProtectedAction(m_editWidgets, ProtectNone);
  emit finishEdit(m_transactions);
}

void TransactionEditor::deleteUnusedEditWidgets(void)
{
  QMap<QString, QWidget*>::iterator it_w;
  for(it_w = m_editWidgets.begin(); it_w != m_editWidgets.end(); ) {
    if((*it_w) && (*it_w)->parent())
      ++it_w;
    else {
      delete (*it_w);
      m_editWidgets.remove(it_w);
      it_w = m_editWidgets.begin();
    }
  }
}

void TransactionEditor::setup(const MyMoneyAccount& account)
{
  m_account = account;
  createEditWidgets();
  m_regForm->arrangeEditWidgets(m_editWidgets, m_item);
  loadEditWidgets();
  deleteUnusedEditWidgets();
  slotUpdateButtonState();
}

void TransactionEditor::tabOrder(QWidgetList& tabOrderWidgets) const
{
  m_regForm->tabOrder(tabOrderWidgets, m_item);
}


void TransactionEditor::slotReloadEditWidgets(void)
{
}

void TransactionEditor::slotUpdateButtonState(void)
{
  emit transactionDataSufficient(isComplete());
}

QWidget* TransactionEditor::haveWidget(const QString& name) const
{
  QMap<QString, QWidget*>::const_iterator it_w;
  it_w = m_editWidgets.find(name);
  if(it_w != m_editWidgets.end())
    return *it_w;
  return 0;
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

bool TransactionEditor::fixTransactionCommodity(const MyMoneyAccount& account)
{
  bool rc = true;
  bool firstTimeMultiCurrency = true;
  m_account = account;

  // determine the max fraction for this account
  MyMoneySecurity sec = m_objects->security(m_account.currencyId());
  int fract = m_account.fraction(sec);

  // scan the list of selected transactions
  QValueList<KMyMoneyRegister::SelectedTransaction>::iterator it_t;
  for(it_t = m_transactions.begin(); (rc == true) && (it_t != m_transactions.end()); ++it_t) {
    // we need to check things only if a different commodity is used
    if(m_account.currencyId() != (*it_t).transaction().commodity()) {
      MyMoneySecurity osec = m_objects->security((*it_t).transaction().commodity());
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
              if(m_transactions.count() == 1) {
                msg = i18n("This transaction has more than two splits and is originally based on a different currency (%1). Using this account to modify the transaction may result in rounding errors. Do you want to continue?").arg(osec.name());
              } else {
                msg = i18n("At least one of the selected transactions has more than two splits and is originally based on a different currency (%1). Using this account to modify the transactions may result in rounding errors. Do you want to continue?").arg(osec.name());
              }

              if(KMessageBox::warningContinueCancel(0, QString("<qt>%1</qt>").arg(msg)) == KMessageBox::Cancel) {
                rc = false;
              }
            }

            if(rc == true) {
              MyMoneyMoney price = (*it_t).split().shares() / (*it_t).split().value();
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
    Q_ULLONG num = m_account.value("lastNumberUsed").toULongLong();
    number->loadText(QString::number(num + 1));
  }
}

bool TransactionEditor::canAssignNumber(void) const
{
  kMyMoneyLineEdit* number = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));
  return (number != 0) && (number->text().isEmpty());
}

StdTransactionEditor::StdTransactionEditor()
{
}

StdTransactionEditor::StdTransactionEditor(TransactionEditorContainer* regForm, MyMoneyObjectContainer* objects, KMyMoneyRegister::Transaction* item, const QValueList<KMyMoneyRegister::SelectedTransaction>& list, const QDate& lastPostDate) :
  TransactionEditor(regForm, objects, item, list, lastPostDate),
  m_inUpdateVat(false)
{
}

void StdTransactionEditor::createEditWidgets(void)
{
  KMyMoneyPayee* payee = new KMyMoneyPayee;
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

// FIXME remove tabbar
#if 0
  KMyMoneyComboAction* combo = new KMyMoneyComboAction;
  m_editWidgets["action"] = combo;
  connect(combo, SIGNAL(actionSelected(int)), this, SLOT(slotUpdateAction(int)));
#endif

  KMyMoneyReconcileCombo* reconcile = new KMyMoneyReconcileCombo;
  m_editWidgets["status"] = reconcile;
  connect(reconcile, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotUpdateButtonState()));

  // if we don't have more than 1 selected transaction, we don't need
  // the "don't change" item in some of the combo widgets
  if(m_transactions.count() < 2) {
    reconcile->removeDontCare();
    cashflow->removeDontCare();
  }

  m_editWidgets["categoryLabel"] = new QLabel(i18n("Category"), 0);
}

void StdTransactionEditor::setupCategoryWidget(QCString& categoryId)
{
  KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
  category->setSuppressObjectCreation(false);
  disconnect(category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));

  switch(m_splits.count()) {
    case 0:
      categoryId = QCString();
      category->completion()->setSelected(QCString());
      break;

    case 1:
      categoryId = m_splits[0].accountId();
      category->completion()->setSelected(categoryId);
      category->slotItemSelected(categoryId);
      m_shares = m_splits[0].shares();
      break;

    default:
      categoryId = QCString();
      category->setCurrentText(i18n("Split transaction (category replacement)", "Split transaction"));
      connect(category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));
      category->setSuppressObjectCreation(true);
      break;
  }
}

bool StdTransactionEditor::isTransfer(const QCString& accId1, const QCString& accId2) const
{
  if(accId1.isEmpty() || accId2.isEmpty())
    return false;

  MyMoneyAccount acc1 = m_objects->account(accId1);
  MyMoneyAccount acc2 = m_objects->account(accId2);

  bool isCat1 = (acc1.accountGroup() == MyMoneyAccount::Income) || (acc1.accountGroup() == MyMoneyAccount::Expense);
  bool isCat2 = (acc2.accountGroup() == MyMoneyAccount::Income) || (acc2.accountGroup() == MyMoneyAccount::Expense);

  return isCat1 == isCat2;
}

void StdTransactionEditor::loadEditWidgets()
{
  // don't kick off VAT processing from here
  m_inUpdateVat = true;

  QMap<QString, QWidget*>::const_iterator it_w;
  QWidget* w;

  // load the payee widget
  KMyMoneyPayee* payee = dynamic_cast<KMyMoneyPayee*>(m_editWidgets["payee"]);
  payee->loadPayees(MyMoneyFile::instance()->payeeList());

  // load the category widget
  KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
  disconnect(category, SIGNAL(focusIn()), this, SLOT(slotEditSplits()));
  category->setSuppressObjectCreation(false);

  AccountSet aSet(m_objects);
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

  if(m_transactions.count() < 2) {
    dynamic_cast<KTextEdit*>(m_editWidgets["memo"])->setText(m_split.memo());
    if(m_transaction.postDate().isValid())
      dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->setDate(m_transaction.postDate());
    else if(m_lastPostDate.isValid())
      dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->setDate(m_lastPostDate);
    else
      dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->setDate(QDate::currentDate());

    if((w = haveWidget("number")) != 0)
      dynamic_cast<kMyMoneyLineEdit*>(w)->loadText(m_split.number());
    dynamic_cast<KMyMoneyReconcileCombo*>(m_editWidgets["status"])->setState(m_split.reconcileFlag());

    QCString payeeId = m_split.payeeId();
    if(!payeeId.isEmpty()) {
      payee->completion()->setSelected(payeeId);
      payee->slotItemSelected(payeeId);
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
        if(m_split.value().isNegative())
          categoryLabel->setText(i18n("Transfer to"));
        else
          categoryLabel->setText(i18n("Transfer from"));
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

  } else {
    dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"])->loadDate(QDate());
    dynamic_cast<KMyMoneyReconcileCombo*>(m_editWidgets["status"])->setState(MyMoneySplit::Unknown);
    if(haveWidget("deposit")) {
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["deposit"])->loadText("");
      dynamic_cast<kMyMoneyEdit*>(m_editWidgets["payment"])->loadText("");
    }
    if((w = haveWidget("amount")) != 0)
      dynamic_cast<kMyMoneyEdit*>(w)->loadText("");

    if((w = haveWidget("cashflow")) != 0) {
      KMyMoneyCashFlowCombo* cashflow = dynamic_cast<KMyMoneyCashFlowCombo*>(w);
      cashflow->setDirection(KMyMoneyRegister::Unknown);
    }
    category->completion()->setSelected(QCString());
  }

  // allow kick off VAT processing again
  m_inUpdateVat = false;
}

void StdTransactionEditor::slotReloadEditWidgets(void)
{
  // reload category widget
  KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
  QCStringList list;
  category->selectedItems(list);
  QCString categoryId;
  if(!list.isEmpty())
    categoryId = list[0];

  AccountSet aSet(m_objects);
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
  KMyMoneyPayee* payee = dynamic_cast<KMyMoneyPayee*>(m_editWidgets["payee"]);
  payee->selectedItems(list);
  QCString payeeId;
  if(!list.isEmpty())
    payeeId = list[0];

  payee->loadPayees(MyMoneyFile::instance()->payeeList());

  if(!payeeId.isEmpty()) {
    payee->completion()->setSelected(payeeId);
    payee->slotItemSelected(payeeId);
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
    // TODO add logic to allow selection

    MyMoneyTransaction t = list.last().first;

    m_transaction.removeSplits();
    m_split = MyMoneySplit();
    QValueList<MyMoneySplit>::ConstIterator it;
    for(it = t.splits().begin(); it != t.splits().end(); ++it) {
      MyMoneySplit s(*it);
      s.setReconcileFlag(MyMoneySplit::NotReconciled);
      s.setReconcileDate(QDate());
      s.clearId();
      s.setBankID(QString());
#if 0
      // FIXME update check number. The old comment contained
      //
      // <quote>
      // If a check number is already specified by the user it is
      // used. If the input field is empty and the previous transaction
      // contains a checknumber, the next usuable check no will be assigned
      // to the transaction.
      // </quote>

      if(m_editNr && !m_editNr->text().isEmpty()) {
        s.setNumber(m_editNr->text());
      } else if(!s.number().isEmpty()) {
        unsigned64 no = MyMoneyFile::instance()->highestCheckNo(s.accountId()).toULongLong();
        s.setNumber(QString::number(no+1));
      }
#endif
      m_transaction.addSplit(s);
      if(s.accountId() == m_account.id() && m_split == MyMoneySplit()) {
        m_split = s;
      }
    }
    // now setup the widgets with the new data
    loadEditWidgets();
  }

  // focus jumps into the category field
  QWidget* w;
  if((w = haveWidget("payee")) != 0) {
    w->setFocus();
  }
}

void StdTransactionEditor::slotUpdateCashFlow(KMyMoneyRegister::CashFlowDirection dir)
{
  QLabel *categoryLabel = dynamic_cast<QLabel*>(haveWidget("categoryLabel"));
  // qDebug("Update cashflow to %d", dir);
  if(categoryLabel) {
    if(categoryLabel->text() != i18n("Category")) {
      if(dir == KMyMoneyRegister::Deposit) {
        categoryLabel->setText(i18n("Transfer from"));
      } else {
        categoryLabel->setText(i18n("Transfer to"));
      }
    }
  }
}

void StdTransactionEditor::slotUpdateCategory(const QCString& id)
{
  QLabel *categoryLabel = dynamic_cast<QLabel*>(haveWidget("categoryLabel"));
  // qDebug("Update category to %s", id.data());

  if(categoryLabel) {
    kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(m_editWidgets["amount"]);
    MyMoneyMoney val = amount->value();

    if(categoryLabel->text() == i18n("Transfer from")) {
      val = -val;
    } else {
      val = val.abs();
    }

    if(!id.isEmpty()) {
      MyMoneyAccount acc = m_objects->account(id);
      if(acc.accountGroup() == MyMoneyAccount::Asset
      || acc.accountGroup() == MyMoneyAccount::Liability
      || acc.accountGroup() == MyMoneyAccount::Equity) {
        if(val.isNegative())
          categoryLabel->setText(i18n("Transfer from"));
        else
          categoryLabel->setText(i18n("Transfer to"));
      } else {
        categoryLabel->setText(i18n("Category"));
      }
      updateAmount(val.abs());
    } else {
      categoryLabel->setText(i18n("Category"));
    }
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
  if(m_transactions.count() != 1)
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
    QCStringList list;
    category->selectedItems(list);
    if(list.isEmpty())
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

  try {
    MyMoneySplit cat;  // category
    MyMoneySplit tax;  // tax

    // extract the category split from the transaction
    MyMoneyAccount category = m_objects->account(tr.splitByAccount(m_account.id(), false).accountId());
    MyMoneyAccount vatAcc = m_objects->account(category.value("VatAccount").latin1());
    MyMoneySecurity asec = m_objects->security(m_account.currencyId());
    MyMoneySecurity csec = m_objects->security(category.currencyId());
    MyMoneySecurity vsec = m_objects->security(vatAcc.currencyId());
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

      qDebug("vat amount is '%s'", category.value("VatAmount").latin1());
      if(category.value("VatAmount").lower() != QString("net")) {
        // split value is the gross value
        gv = amount;
        nv = gv / (MyMoneyMoney(1,1) + vatRate);
        cat.setShares(-nv.convert(fract));
        cat.setValue(cat.shares());

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
#if 0
#endif
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
    MyMoneyAccount acc = m_objects->account((*it_s).accountId());
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
  QCString id;
  setupCategoryWidget(id);

  // ... and return the updated amount
  return amount;
}

bool StdTransactionEditor::isComplete(void) const
{
  QMap<QString, QWidget*>::const_iterator it_w;
  for(it_w = m_editWidgets.begin(); it_w != m_editWidgets.end(); ++it_w) {
    KMyMoneyPayee* payee = dynamic_cast<KMyMoneyPayee*>(*it_w);
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

    if(reconcile && reconcile->state() != MyMoneySplit::Unknown)
      break;

    if(cashflow && cashflow->direction() != KMyMoneyRegister::Unknown)
      break;
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
                                                        m_objects,
                                                        m_priceInfo,
                                                        m_regForm);
    // connect(dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));

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

  return rc;
}

void StdTransactionEditor::checkPayeeInSplit(MyMoneySplit& s, const QCString& payeeId)
{
  if(s.accountId().isEmpty() || payeeId.isEmpty())
    return;

  MyMoneyAccount acc = m_objects->account(s.accountId());
  if(acc.accountGroup() == MyMoneyAccount::Income
  || acc.accountGroup() == MyMoneyAccount::Expense) {
    s.setPayeeId(QCString());
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
    if(!(deposit->text().isEmpty() && payment->text().isEmpty()))
      updateValue = true;

    if(deposit->value().isPositive())
      value = deposit->value();
    else
      value = -(payment->value());
  }

  if(update)
    *update = updateValue;

  return value;
}

bool StdTransactionEditor::createTransaction(MyMoneyTransaction& t, const MyMoneyTransaction& torig, const MyMoneySplit& sorig)
{
  bool multiSelection = m_transactions.count() > 1;

  // extract price info from original transaction
  m_priceInfo.clear();
  QValueList<MyMoneySplit>::const_iterator it_s;
  if(!torig.id().isEmpty()) {
    for(it_s = torig.splits().begin(); it_s != torig.splits().end(); ++it_s) {
      if((*it_s).id() != sorig.id()) {
        MyMoneyAccount cat = m_objects->account((*it_s).accountId());
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
    if(!multiSelection || (multiSelection && !memo->text().isEmpty() ) )
      s0.setMemo(memo->text());
  }

  kMyMoneyLineEdit* number = dynamic_cast<kMyMoneyLineEdit*>(haveWidget("number"));
  if(number) {
    if(!multiSelection || (multiSelection && !number->text().isEmpty() ) )
      s0.setNumber(number->text());
  }

  KMyMoneyPayee* payee = dynamic_cast<KMyMoneyPayee*>(m_editWidgets["payee"]);
  QCString payeeId;
  if(!multiSelection || (multiSelection && !payee->currentText().isEmpty())) {
    QCStringList list;
    payee->selectedItems(list);
    if(list.count() > 0) {
      payeeId = list[0];
    }
    s0.setPayeeId(payeeId);
  }

  bool updateValue;
  MyMoneyMoney value = amountFromWidget(&updateValue);

  if(updateValue) {
    // for this account, the shares and value is the same
    s0.setValue(value);
    s0.setShares(value);
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
    splits.append(s1);
  }

  // FIXME in multiSelection we currently only support transactions with one
  // or two splits. So we check the original transaction and extract the other
  // split or create it
  if(multiSelection) {
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

  if(multiSelection || splits.count() == 1) {
    KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
    if(!multiSelection || (multiSelection && !category->currentText().isEmpty())) {
      QCString categoryId;
      QCStringList list;
      category->selectedItems(list);
      if(!list.isEmpty()) {
        categoryId = list[0];
      }
      s1.setAccountId(categoryId);
    }

    // if the first split has a memo but the second split is empty,
    // we just copy the memo text over
    if(memo) {
      if(!multiSelection || (multiSelection && !memo->text().isEmpty())) {
        if(s1.memo().isEmpty())
          s1.setMemo(memo->text());
      }
    }

    if(updateValue && !s1.accountId().isEmpty()) {
      s1.setValue(-value);
      if(!value.isZero()) {
        MyMoneyAccount cat = m_objects->account(s1.accountId());
        if(cat.currencyId() != m_transaction.commodity()) {

          MyMoneySecurity fromCurrency, toCurrency;
          MyMoneyMoney fromValue, toValue;
          fromCurrency = m_objects->security(m_transaction.commodity());
          toCurrency = m_objects->security(cat.currencyId());

          // determine the fraction required for this category
          int fract = cat.fraction(toCurrency);

          // display only positive values to the user
          fromValue = s1.value().abs();

          // if we had a price info in the beginning, we use it here
          if(m_priceInfo.find(cat.currencyId()) != m_priceInfo.end()) {
            toValue = (fromValue * m_priceInfo[cat.currencyId()]).convert(fract);
          }

          // if the shares are still 0, we need to change that
          if(toValue.isZero()) {
            MyMoneyPrice price = MyMoneyFile::instance()->price(fromCurrency.id(), toCurrency.id());
            // if the price is valid calculate the shares. If it is invalid
            // assume a conversion rate of 1.0
            if(price.isValid()) {
              toValue = (price.rate(toCurrency.id()) * fromValue).convert(fract);
            } else {
              toValue = fromValue;
            }
          }

          // now present all that to the user
          KCurrencyCalculator calc(fromCurrency,
                                  toCurrency,
                                  fromValue,
                                  toValue,
                                  m_transaction.postDate(),
                                  fract,
                                  m_regForm, "currencyCalculator");

          if(calc.exec() == QDialog::Rejected) {
            return false;
          }
          s1.setShares((s1.value() * calc.price()).convert(fract));

        } else {
          s1.setShares(-value);
        }
      } else
        s1.setShares(-value);
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

bool StdTransactionEditor::enterTransactions(void)
{
  // make sure to run through all stuff that is tied to 'focusout events'.
  m_regForm->parentWidget()->setFocus();
  QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 10);

  // we don't need to update our widgets anymore, so we just disconnect the signal
  disconnect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotReloadEditWidgets()));

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
    QValueList<MyMoneyTransaction>::iterator it_ts;
    for(it_ts = list.begin(); it_ts != list.end(); ++it_ts) {
      try {
        if((*it_ts).id().isEmpty()) {
          // add new transaction
          MyMoneyFile::instance()->addTransaction(*it_ts);
        } else {
          // modify existing transaction
          MyMoneyFile::instance()->modifyTransaction(*it_ts);
        }
      } catch(MyMoneyException * e) {
        qDebug("Unable to store transaction within engine: %s", e->what().latin1());
        delete e;
      }
    }
    // update m_transactions to contain the newly created transaction so that
    // it is selected as the current one
    if(newTransactionCreated) {
      m_transactions.clear();
      KMyMoneyRegister::SelectedTransaction s((*it_ts), (*it_ts).splits()[0]);
      m_transactions.append(s);
    }
  }
  return storeTransactions;
}


#include "transactioneditor.moc"

