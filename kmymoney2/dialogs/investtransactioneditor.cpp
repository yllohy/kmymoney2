/***************************************************************************
                             investtransactioneditor.cpp
                             ----------
    begin                : Fri Dec 15 2006
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

#include <typeinfo>

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

#include <kmymoney/investtransactioneditor.h>
#include <kmymoney/kmymoneycategory.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneylineedit.h>
#include <kmymoney/kmymoneyaccountcompletion.h>
#include <kmymoney/kmymoneyaccountselector.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/transactionform.h>

#include "../dialogs/ksplittransactiondlg.h"
#include "../dialogs/kcurrencycalculator.h"

#include "../kmymoneyglobalsettings.h"

#include "investactivities.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;
using namespace Invest;

class InvestTransactionEditor::Private {
  friend class Invest::Activity;

public:
  Private(InvestTransactionEditor* parent) :
    m_parent(parent),
    m_activity(0)
  {
    m_phonyAccount = MyMoneyAccount("Phony-ID", MyMoneyAccount());
  }

  ~Private() {
    delete m_activity;
  }

  QWidget* haveWidget(const QString& name) { return m_parent->haveWidget(name); }

  InvestTransactionEditor* m_parent;
  Activity*                m_activity;
  MyMoneyAccount           m_phonyAccount;
  MyMoneySplit             m_phonySplit;
};


InvestTransactionEditor::InvestTransactionEditor() :
  d(new Private(this))
{
}

InvestTransactionEditor::~InvestTransactionEditor()
{
  delete d;
}

InvestTransactionEditor::InvestTransactionEditor(TransactionEditorContainer* regForm, KMyMoneyRegister::InvestTransaction* item, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate) :
  TransactionEditor(regForm, item, list, lastPostDate),
  d(new Private(this))
{
  // dissect the transaction into its type, splits, currency, security etc.
  dissectTransaction(m_transaction, m_split,
                     m_assetAccountSplit,
                     m_feeSplits,
                     m_interestSplits,
                     m_security,
                     m_currency,
                     m_transactionType);

  // determine initial activity object
  activityFactory(m_transactionType);
}

void InvestTransactionEditor::activityFactory(MyMoneySplit::investTransactionTypeE type)
{
  if(!d->m_activity || type != d->m_activity->type()) {
    delete d->m_activity;
    switch(type) {
      default:
      case MyMoneySplit::BuyShares:
        d->m_activity = new Buy(this);
        break;
      case MyMoneySplit::SellShares:
        d->m_activity = new Sell(this);
        break;
      case MyMoneySplit::Dividend:
      case MyMoneySplit::Yield:
        d->m_activity = new Div(this);
        break;
      case MyMoneySplit::ReinvestDividend:
        d->m_activity = new Reinvest(this);
        break;
      case MyMoneySplit::AddShares:
        d->m_activity = new Add(this);
        break;
      case MyMoneySplit::RemoveShares:
        d->m_activity = new Remove(this);
        break;
      case MyMoneySplit::SplitShares:
        d->m_activity = new Split(this);
        break;
    }
  }
}

void InvestTransactionEditor::dissectTransaction(const MyMoneyTransaction& transaction, const MyMoneySplit& split, MyMoneySplit& assetAccountSplit, QValueList<MyMoneySplit>& feeSplits, QValueList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency, MyMoneySplit::investTransactionTypeE& transactionType)
{
  // collect the splits. split references the stock account and should already
  // be set up. assetAccountSplit references the corresponding asset account (maybe
  // empty), feeSplits is the list of all expenses and interestSplits
  // the list of all incomes
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    MyMoneyAccount acc = file->account((*it_s).accountId());
    if((*it_s).id() == split.id()) {
      security = file->security(acc.currencyId());
    } else if(acc.accountGroup() == MyMoneyAccount::Expense) {
      feeSplits.append(*it_s);
      // feeAmount += (*it_s).value();
    } else if(acc.accountGroup() == MyMoneyAccount::Income) {
      interestSplits.append(*it_s);
      // interestAmount += (*it_s).value();
    } else {
      assetAccountSplit = *it_s;
    }
  }

  // determine transaction type
  if(split.action() == MyMoneySplit::ActionAddShares) {
    transactionType = (!split.shares().isNegative()) ? MyMoneySplit::AddShares : MyMoneySplit::RemoveShares;
  } else if(split.action() == MyMoneySplit::ActionBuyShares) {
    transactionType = (!split.value().isNegative()) ? MyMoneySplit::BuyShares : MyMoneySplit::SellShares;
  } else if(split.action() == MyMoneySplit::ActionDividend) {
    transactionType = MyMoneySplit::Dividend;
  } else if(split.action() == MyMoneySplit::ActionReinvestDividend) {
    transactionType = MyMoneySplit::ReinvestDividend;
  } else if(split.action() == MyMoneySplit::ActionYield) {
    transactionType = MyMoneySplit::Yield;
  } else if(split.action() == MyMoneySplit::ActionSplitShares) {
    transactionType = MyMoneySplit::SplitShares;
  } else
    transactionType = MyMoneySplit::BuyShares;

  currency.setTradingSymbol("???");
  try {
    currency = file->security(transaction.commodity());
  } catch(MyMoneyException *e) {
    delete e;
  }
}

void InvestTransactionEditor::createEditWidgets(void)
{
  KMyMoneyActivityCombo* activity = new KMyMoneyActivityCombo();
  m_editWidgets["activity"] = activity;
  connect(activity, SIGNAL(activitySelected(MyMoneySplit::investTransactionTypeE)), this, SLOT(slotUpdateActivity(MyMoneySplit::investTransactionTypeE)));
  connect(activity, SIGNAL(activitySelected(MyMoneySplit::investTransactionTypeE)), this, SLOT(slotUpdateButtonState()));

  m_editWidgets["postdate"] = new kMyMoneyDateInput;
  connect(m_editWidgets["postdate"], SIGNAL(dateChanged(const QDate&)), this, SLOT(slotUpdateButtonState()));

  KMyMoneySecurity* security = new KMyMoneySecurity;
  security->setHint(i18n("Security"));
  m_editWidgets["security"] = security;
  connect(security, SIGNAL(itemSelected(const QString&)), this, SLOT(slotUpdateSecurity(const QString&)));
  connect(security, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(security, SIGNAL(createItem(const QString&, QString&)), this, SLOT(slotCreateSecurity(const QString&, QString&)));
  connect(security, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));

  KMyMoneyCategory* asset = new KMyMoneyCategory(0, 0, false);
  asset->setHint(i18n("Asset account"));
  m_editWidgets["asset-account"] = asset;
  connect(asset, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(asset, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));

  KMyMoneyCategory* fees = new KMyMoneyCategory(0, 0, true);
  fees->setHint(i18n("Fees"));
  m_editWidgets["fee-account"] = fees;
  connect(fees, SIGNAL(itemSelected(const QString&)), this, SLOT(slotUpdateFeeCategory(const QString&)));
  connect(fees, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(fees, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateFeeVisibility(const QString&)));
  connect(fees, SIGNAL(createItem(const QString&, QString&)), this, SLOT(slotCreateFeeCategory(const QString&, QString&)));
  connect(fees, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));
  connect(fees->splitButton(), SIGNAL(clicked()), this, SLOT(slotEditFeeSplits()));

  KMyMoneyCategory* interest = new KMyMoneyCategory(0, 0, true);
  interest->setHint(i18n("Interest"));
  m_editWidgets["interest-account"] = interest;
  connect(interest, SIGNAL(itemSelected(const QString&)), this, SLOT(slotUpdateInterestCategory(const QString&)));
  connect(interest, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(interest, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateInterestVisibility(const QString&)));
  connect(interest, SIGNAL(createItem(const QString&, QString&)), this, SLOT(slotCreateInterestCategory(const QString&, QString&)));
  connect(interest, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));
  connect(interest->splitButton(), SIGNAL(clicked()), this, SLOT(slotEditInterestSplits()));

  KTextEdit* memo = new KTextEdit;
  memo->setTabChangesFocus(true);
  m_editWidgets["memo"] = memo;

  kMyMoneyEdit* value = new kMyMoneyEdit;
  value->setHint(i18n("Shares"));
  value->setResetButtonVisible(false);
  m_editWidgets["shares"] = value;
  connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateTotalAmount()));

  value = new kMyMoneyEdit;
  value->setHint(i18n("Price"));
  value->setResetButtonVisible(false);
  value->setPrecision(KMyMoneyGlobalSettings::pricePrecision());
  m_editWidgets["price"] = value;
  connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateTotalAmount()));

  value = new kMyMoneyEdit;
  // TODO once we have the selected transactions as array of Transaction
  // we can allow multiple splits for fee and interest
  value->setResetButtonVisible(false);
  m_editWidgets["fee-amount"] = value;
  connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateTotalAmount()));

  value = new kMyMoneyEdit;
  // TODO once we have the selected transactions as array of Transaction
  // we can allow multiple splits for fee and interest
  value->setResetButtonVisible(false);
  m_editWidgets["interest-amount"] = value;
  connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateTotalAmount()));

  KMyMoneyReconcileCombo* reconcile = new KMyMoneyReconcileCombo;
  m_editWidgets["status"] = reconcile;
  connect(reconcile, SIGNAL(itemSelected(const QString&)), this, SLOT(slotUpdateButtonState()));

  KMyMoneyRegister::QWidgetContainer::iterator it_w;
  for(it_w = m_editWidgets.begin(); it_w != m_editWidgets.end(); ++it_w) {
    (*it_w)->installEventFilter(this);
  }

  QLabel* label = new QLabel("", 0);
  label->setAlignment(Qt::AlignVCenter | Qt::AlignRight | Qt::DontClip);
  m_editWidgets["total"] = label;

  label = new QLabel("", 0);
  label->setAlignment(Qt::AlignVCenter | Qt::DontClip);
  m_editWidgets["total-label"] = label;

  label = new QLabel("", 0);
  label->setAlignment(Qt::AlignVCenter | Qt::DontClip);
  m_editWidgets["asset-label"] = label;

  label = new QLabel("", 0);
  label->setAlignment(Qt::AlignVCenter | Qt::DontClip);
  m_editWidgets["fee-label"] = label;

  label = new QLabel("", 0);
  label->setAlignment(Qt::AlignVCenter | Qt::DontClip);
  m_editWidgets["fee-amount-label"] = label;

  label = new QLabel("", 0);
  label->setAlignment(Qt::AlignVCenter | Qt::DontClip);
  m_editWidgets["interest-label"] = label;

  label = new QLabel("", 0);
  label->setAlignment(Qt::AlignVCenter | Qt::DontClip);
  m_editWidgets["interest-amount-label"] = label;

  label = new QLabel("", 0);
  label->setAlignment(Qt::AlignVCenter | Qt::DontClip);
  m_editWidgets["price-label"] = label;

  label = new QLabel("", 0);
  label->setAlignment(Qt::AlignVCenter | Qt::DontClip);
  m_editWidgets["shares-label"] = label;

  // if we don't have more than 1 selected transaction, we don't need
  // the "don't change" item in some of the combo widgets
  if(m_transactions.count() < 2) {
    reconcile->removeDontCare();
  }
}

int InvestTransactionEditor::slotEditFeeSplits(void)
{
  return editSplits("fee-account", "fee-amount", m_feeSplits, false, SLOT(slotEditFeeSplits()));
}

int InvestTransactionEditor::slotEditInterestSplits(void)
{
  return editSplits("interest-account", "interest-amount", m_interestSplits, true, SLOT(slotEditInterestSplits()));
}

int InvestTransactionEditor::editSplits(const QString& categoryWidgetName, const QString& amountWidgetName, QValueList<MyMoneySplit>& splits, bool isIncome, const char* slotEditSplits)
{
  int rc = QDialog::Rejected;

  if(!m_openEditSplits) {
  // only get in here in a single instance
    m_openEditSplits = true;

    // force focus change to update all data
    KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets[categoryWidgetName]);
    QWidget* w = category->splitButton();
    if(w)
      w->setFocus();

    kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(haveWidget(amountWidgetName));

    MyMoneyTransaction transaction;
    transaction.setCommodity(m_currency.id());
    if(splits.count() == 0 && category->selectedItem()) {
      MyMoneySplit s;
      s.setAccountId(category->selectedItem());
      s.setShares(amount->value());
      s.setValue(s.shares());
      splits << s;
    }
    // use the transactions commodity as the currency indicator for the splits
    // this is used to allow some useful setting for the fractions in the amount fields
    try {
      d->m_phonyAccount.setCurrencyId(m_transaction.commodity());
      d->m_phonyAccount.fraction(MyMoneyFile::instance()->security(m_transaction.commodity()));
    } catch(MyMoneyException *e) {
      qDebug("Unable to setup precision");
      delete e;
    }

    if(createPseudoTransaction(transaction, splits)) {
      MyMoneyMoney value;

      KSplitTransactionDlg* dlg = new KSplitTransactionDlg(transaction,
                                                          d->m_phonySplit,
                                                          d->m_phonyAccount,
                                                          false,
                                                          isIncome,
                                                          0,
                                                          m_priceInfo,
                                                          m_regForm);
      // connect(dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));

      if((rc = dlg->exec()) == QDialog::Accepted) {
        transaction = dlg->transaction();
        // collect splits out of the transaction
        splits.clear();
        QValueList<MyMoneySplit>::const_iterator it_s;
        MyMoneyMoney fees;
        for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
          if((*it_s).accountId() == d->m_phonyAccount.id())
            continue;
          splits << *it_s;
          fees += (*it_s).shares();
        }
        if(isIncome)
          fees = -fees;

        QString categoryId;
        setupCategoryWidget(category, splits, categoryId, slotEditSplits);
        amount->setValue(fees);
        slotUpdateTotalAmount();
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

bool InvestTransactionEditor::createPseudoTransaction(MyMoneyTransaction& t, const QValueList<MyMoneySplit>& splits)
{
  t.removeSplits();

  MyMoneySplit split;
  split.setAccountId(d->m_phonyAccount.id());
  split.setValue(-subtotal(splits));
  split.setShares(split.value());
  t.addSplit(split);
  d->m_phonySplit = split;

  QValueList<MyMoneySplit>::const_iterator it_s;
  for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
    split = *it_s;
    split.clearId();
    t.addSplit(split);
  }
  return true;
}

void InvestTransactionEditor::slotCreateSecurity(const QString& name, QString& id)
{
  MyMoneyAccount acc;
  QRegExp exp("([^:]+)");
  if(exp.search(name) != -1) {
    acc.setName(exp.cap(1));

    emit createSecurity(acc, m_account);

    // return id
    id = acc.id();

    if(!id.isEmpty()) {
      slotUpdateSecurity(id);
    }
  }
}

void InvestTransactionEditor::slotCreateFeeCategory(const QString& name, QString& id)
{
  MyMoneyAccount acc;
  acc.setName(name);

  emit createCategory(acc, MyMoneyFile::instance()->expense());

  // return id
  id = acc.id();
}

void InvestTransactionEditor::slotUpdateFeeCategory(const QString& id)
{
  haveWidget("fee-amount")->setDisabled(id.isEmpty());
}

void InvestTransactionEditor::slotUpdateFeeVisibility(const QString& txt)
{
  haveWidget("fee-amount")->setHidden(txt.isEmpty());
  QWidget* w = haveWidget("fee-amount-label");
  if(w)
    w->setShown(haveWidget("fee-amount")->isVisible());
}

void InvestTransactionEditor::slotUpdateInterestCategory(const QString& id)
{
  haveWidget("interest-amount")->setDisabled(id.isEmpty());
}

void InvestTransactionEditor::slotUpdateInterestVisibility(const QString& txt)
{
  KMyMoneyCategory* interest = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
  QWidget* w = haveWidget("interest-amount-label");

  if(dynamic_cast<Reinvest*>(d->m_activity)) {
    interest->splitButton()->hide();
    haveWidget("interest-amount")->setHidden(true);
    // for the reinvest case, we don't ever hide the label do avoid a shine through
    // of the underlying transaction data.
    w = 0;
  } else {
    haveWidget("interest-amount")->setHidden(txt.isEmpty());
    // FIXME once we can handle split interest, we need to uncomment the next line
    // interest->splitButton()->show();
  }

  if(w)
    w->setShown(haveWidget("interest-amount")->isVisible());
}

void InvestTransactionEditor::slotCreateInterestCategory(const QString& name, QString& id)
{
  MyMoneyAccount acc;
  acc.setName(name);

  emit createCategory(acc, MyMoneyFile::instance()->income());

  // return id
  id = acc.id();
}

void InvestTransactionEditor::slotReloadEditWidgets(void)
{
  KMyMoneyCategory* interest = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
  KMyMoneyCategory* fees = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
  KMyMoneySecurity* security = dynamic_cast<KMyMoneySecurity*>(haveWidget("security"));

  AccountSet aSet;
  QString id;

  // interest-account
  aSet.clear();
  aSet.addAccountGroup(MyMoneyAccount::Income);
  aSet.load(interest->selector());
  setupCategoryWidget(interest, m_interestSplits, id, SLOT(slotEditInterestSplits()));

  // fee-account
  aSet.clear();
  aSet.addAccountGroup(MyMoneyAccount::Expense);
  aSet.load(fees->selector());
  setupCategoryWidget(fees, m_feeSplits, id, SLOT(slotEditFeeSplits()));

  // security
  aSet.clear();
  aSet.load(security->selector(), i18n("Security"), m_account.accountList(), true);
}

void InvestTransactionEditor::loadEditWidgets(KMyMoneyRegister::Action /* action */)
{
  QString id;

  kMyMoneyDateInput* postDate = dynamic_cast<kMyMoneyDateInput*>(haveWidget("postdate"));
  KMyMoneyReconcileCombo* reconcile = dynamic_cast<KMyMoneyReconcileCombo*>(haveWidget("status"));
  KMyMoneySecurity* security = dynamic_cast<KMyMoneySecurity*>(haveWidget("security"));
  KMyMoneyActivityCombo* activity = dynamic_cast<KMyMoneyActivityCombo*>(haveWidget("activity"));
  KMyMoneyCategory* asset = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));
  KTextEdit* memo = dynamic_cast<KTextEdit*>(m_editWidgets["memo"]);
  kMyMoneyEdit* value;
  KMyMoneyCategory* interest = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
  KMyMoneyCategory* fees = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));

  // check if the current transaction has a reference to an equity account
  bool haveEquityAccount = false;
  QValueList<MyMoneySplit>::const_iterator it_s;
  for(it_s = m_transaction.splits().begin(); !haveEquityAccount && it_s != m_transaction.splits().end(); ++it_s) {
    MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
    if(acc.accountType() == MyMoneyAccount::Equity)
      haveEquityAccount = true;
  }

  // asset-account
  AccountSet aSet;
  aSet.clear();
  aSet.addAccountType(MyMoneyAccount::Checkings);
  aSet.addAccountType(MyMoneyAccount::Savings);
  aSet.addAccountType(MyMoneyAccount::Cash);
  aSet.addAccountType(MyMoneyAccount::Asset);
  aSet.addAccountType(MyMoneyAccount::Currency);
  aSet.addAccountType(MyMoneyAccount::CreditCard);
  if(KMyMoneyGlobalSettings::expertMode() || haveEquityAccount)
    aSet.addAccountGroup(MyMoneyAccount::Equity);
  aSet.load(asset->selector());

  // security
  security->setSuppressObjectCreation(false);    // allow object creation on the fly
  aSet.clear();
  aSet.load(security->selector(), i18n("Security"), m_account.accountList(), true);

  if(!isMultiSelection()) {
    // date
    if(m_transaction.postDate().isValid())
      postDate->setDate(m_transaction.postDate());
    else if(m_lastPostDate.isValid())
      postDate->setDate(m_lastPostDate);
    else
      postDate->setDate(QDate::currentDate());

    // security (but only if it's not the investment account)
    if(m_split.accountId() != m_account.id()) {
      security->completion()->setSelected(m_split.accountId());
      security->slotItemSelected(m_split.accountId());
    }

    // activity
    activity->setActivity(d->m_activity->type());
    slotUpdateActivity(activity->activity());

    asset->completion()->setSelected(m_assetAccountSplit.accountId());
    asset->slotItemSelected(m_assetAccountSplit.accountId());

    // interest-account
    aSet.clear();
    aSet.addAccountGroup(MyMoneyAccount::Income);
    aSet.load(interest->selector());
    setupCategoryWidget(interest, m_interestSplits, id, SLOT(slotEditInterestSplits()));
    slotUpdateInterestVisibility(interest->currentText());

    // fee-account
    aSet.clear();
    aSet.addAccountGroup(MyMoneyAccount::Expense);
    aSet.load(fees->selector());
    setupCategoryWidget(fees, m_feeSplits, id, SLOT(slotEditFeeSplits()));
    slotUpdateFeeVisibility(fees->currentText());

    // memo
    memo->setText(m_split.memo());

    // shares
    // don't set the value if the number of shares is zero so that
    // we can see the hint
    value = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
    if(typeid(*(d->m_activity)) != typeid(Invest::Split(this)))
      value->setPrecision(MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction()));
    else
      value->setPrecision(-1);

    if(!m_split.shares().isZero())
      value->setValue(m_split.shares().abs());

    // price
    updatePriceMode(m_split);

    // fee amount
    value = dynamic_cast<kMyMoneyEdit*>(haveWidget("fee-amount"));
    value->setValue(subtotal(m_feeSplits));

    // interest amount
    value = dynamic_cast<kMyMoneyEdit*>(haveWidget("interest-amount"));
    value->setValue(-subtotal(m_interestSplits));

    // total
    slotUpdateTotalAmount();

    // status
    if(m_split.reconcileFlag() == MyMoneySplit::Unknown)
      m_split.setReconcileFlag(MyMoneySplit::NotReconciled);
    reconcile->setState(m_split.reconcileFlag());

  } else {
    postDate->loadDate(QDate());
    reconcile->setState(MyMoneySplit::Unknown);
    memo->setText(QString());

    // We don't allow to change the activity
    activity->setActivity(d->m_activity->type());
    slotUpdateActivity(activity->activity());
    activity->setDisabled(true);

    // scan the list of selected transactions and check that they have
    // the same activity.
    KMyMoneyRegister::SelectedTransactions::iterator it_t = m_transactions.begin();
    const QString& action = m_item->split().action();
    bool isNegative = m_item->split().shares().isNegative();
    bool allSameActivity = true;
    for(it_t = m_transactions.begin(); allSameActivity && (it_t != m_transactions.end()); ++it_t) {
      allSameActivity = (action == (*it_t).split().action() && (*it_t).split().shares().isNegative() == isNegative);
    }

    QStringList fields;
    fields << "shares" << "price" << "fee-amount" << "interest-amount";
    QStringList::const_iterator it_f;
    for(it_f = fields.begin(); it_f != fields.end(); ++it_f) {
      value = dynamic_cast<kMyMoneyEdit*>(haveWidget((*it_f)));
      value->setText("");
      value->setAllowEmpty();
    }

    // if we have transactions with different activities, disable some more widgets
    if(!allSameActivity) {
      fields << "asset-account" << "fee-account" << "interest-account";
      QStringList::const_iterator it_f;
      for(it_f = fields.begin(); it_f != fields.end(); ++it_f) {
        haveWidget(*it_f)->setDisabled(true);
      }
    }
  }
}

QWidget* InvestTransactionEditor::firstWidget(void) const
{
  return 0; // let the creator use the first widget in the tab order
}

bool InvestTransactionEditor::isComplete(QString& reason) const
{
  reason = QString();
  // reason.clear();  // for Qt4
  return d->m_activity->isComplete(reason);
}

MyMoneyMoney InvestTransactionEditor::subtotal(const QValueList<MyMoneySplit>& splits) const
{
  QValueList<MyMoneySplit>::const_iterator it_s;
  MyMoneyMoney sum;

  for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
    sum += (*it_s).value();
  }

  return sum;
}

void InvestTransactionEditor::slotUpdateSecurity(const QString& stockId)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount stock = file->account(stockId);
  m_security = file->security(stock.currencyId());
  m_currency = file->security(m_security.tradingCurrency());
  bool currencyKnown = !m_currency.id().isEmpty();
  if(!currencyKnown) {
    m_currency.setTradingSymbol("???");
  } else {
    if(typeid(*(d->m_activity)) != typeid(Invest::Split(this))) {
      dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"))->setPrecision(MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction()));
    } else {
      dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"))->setPrecision(-1);
    }
  }

  updatePriceMode();

  d->m_activity->preloadAssetAccount();

  haveWidget("shares")->setEnabled(currencyKnown);
  haveWidget("price")->setEnabled(currencyKnown);
  haveWidget("fee-amount")->setEnabled(currencyKnown);
  haveWidget("interest-amount")->setEnabled(currencyKnown);

  slotUpdateTotalAmount();
}

void InvestTransactionEditor::totalAmount(MyMoneyMoney& amount) const
{
  KMyMoneyActivityCombo* activityCombo = dynamic_cast<KMyMoneyActivityCombo*>(haveWidget("activity"));
  kMyMoneyEdit* sharesEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
  kMyMoneyEdit* priceEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("price"));
  kMyMoneyEdit* feesEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("fee-amount"));
  kMyMoneyEdit* interestEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("interest-amount"));

  if(priceMode() == InvestTransactionEditor::PricePerTransaction)
    amount = priceEdit->value().abs();
  else
    amount = sharesEdit->value().abs() * priceEdit->value().abs();

  if(feesEdit->isVisible()) {
    MyMoneyMoney fee = feesEdit->value();
    MyMoneyMoney factor(-1,1);
    switch(activityCombo->activity()) {
      case MyMoneySplit::BuyShares:
      case MyMoneySplit::ReinvestDividend:
        factor = MyMoneyMoney(1,1);
        break;
      default:
        break;
    }
    amount += (fee * factor);
  }

  if(interestEdit->isVisible()) {
    MyMoneyMoney interest = interestEdit->value();
    MyMoneyMoney factor(1,1);
    switch(activityCombo->activity()) {
      case MyMoneySplit::BuyShares:
        factor = MyMoneyMoney(-1,1);
        break;
      default:
        break;
    }
    amount += (interest * factor);
  }
}

void InvestTransactionEditor::slotUpdateTotalAmount(void)
{
  QLabel* total = dynamic_cast<QLabel*>(haveWidget("total"));

  if(total && total->isVisible()) {
    MyMoneyMoney amount;
    totalAmount(amount);
    total->setText(amount.formatMoney(m_currency.tradingSymbol(), MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction())));
  }
}

void InvestTransactionEditor::slotUpdateActivity(MyMoneySplit::investTransactionTypeE activity)
{
  // create new activity object if required
  activityFactory(activity);

  KMyMoneyCategory* cat;

  // hide all dynamic widgets (make sure to use the parentWidget for the
  // category widgets)
  haveWidget("interest-account")->parentWidget()->hide();
  haveWidget("fee-account")->parentWidget()->hide();

  QStringList dynwidgets;
  dynwidgets << "total-label" << "asset-label" << "fee-label" << "fee-amount-label" << "interest-label" << "interest-amount-label" << "price-label" << "shares-label";

  // hiding labels works by clearing them. hide() does not do the job
  // as the underlying text in the QTable object will shine through
  QStringList::const_iterator it_s;
  for(it_s = dynwidgets.begin(); it_s != dynwidgets.end(); ++it_s) {
    QLabel* w = dynamic_cast<QLabel*>(haveWidget(*it_s));
    if(w)
      w->setText(" ");
  }

  // real widgets can be hidden
  dynwidgets.clear();
  dynwidgets << "asset-account" << "interest-amount" << "fee-amount" << "shares" << "price" << "total";

  for(it_s = dynwidgets.begin(); it_s != dynwidgets.end(); ++it_s) {
    QWidget* w = haveWidget(*it_s);
    if(w)
      w->hide();
  }

  d->m_activity->showWidgets();

  d->m_activity->preloadAssetAccount();

  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
  if(cat->parentWidget()->isVisible())
    slotUpdateInterestVisibility(cat->currentText());

  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
  if(cat->parentWidget()->isVisible())
    slotUpdateFeeVisibility(cat->currentText());
}

InvestTransactionEditor::priceModeE InvestTransactionEditor::priceMode(void) const
{
  priceModeE mode = static_cast<priceModeE>(0);
  KMyMoneySecurity* sec = dynamic_cast<KMyMoneySecurity*>(m_editWidgets["security"]);
  QString accId;
  if(!sec->currentText().isEmpty()) {
    accId = sec->selectedItem();
    if(accId.isEmpty())
      accId = m_account.id();
  }
  while(!accId.isEmpty() && mode == 0) {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(accId);
    if(acc.value("priceMode").isEmpty())
      accId = acc.parentAccountId();
    else
      mode = static_cast<priceModeE>(acc.value("priceMode").toInt());
  }

  // if it's still <default> then use that default
  if(mode == 0)
    mode = PricePerShare;
  return mode;
}

bool InvestTransactionEditor::setupPrice(const MyMoneyTransaction& t, MyMoneySplit& split)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount acc = file->account(split.accountId());
  MyMoneySecurity toCurrency(file->security(acc.currencyId()));
  int fract = acc.fraction();

  if(acc.currencyId() != t.commodity()) {
    QMap<QString, MyMoneyMoney>::Iterator it_p;
    QString key = t.commodity() + "-" + acc.currencyId();
    it_p = m_priceInfo.find(key);

    // if it's not found, then collect it from the user first
    MyMoneyMoney price;
    if(it_p == m_priceInfo.end()) {
      MyMoneySecurity fromCurrency = file->security(t.commodity());
      MyMoneyMoney fromValue, toValue;

      fromValue = split.value();
      MyMoneyPrice priceInfo = MyMoneyFile::instance()->price(fromCurrency.id(), toCurrency.id());
      toValue = split.value() * priceInfo.rate(toCurrency.id());

      KCurrencyCalculator calc(fromCurrency,
                                toCurrency,
                                fromValue,
                                toValue,
                                t.postDate(),
                                fract,
                                m_regForm, "currencyCalculator");

      if(calc.exec() == QDialog::Rejected) {
        return false;
      }
      price = calc.price();
      m_priceInfo[key] = price;
    } else {
      price = (*it_p);
    }

    // update shares if the transaction commodity is the currency
    // of the current selected account
    split.setShares((split.value() * price).convert(fract));
  } else {
    split.setShares(split.value().convert(fract));
  }

  return true;
}

bool InvestTransactionEditor::createTransaction(MyMoneyTransaction& t, const MyMoneyTransaction& torig, const MyMoneySplit& sorig, bool /* skipPriceDialog */)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  // we start with the previous values, make sure we can add them later on
  t = torig;
  MyMoneySplit s0 = sorig;
  s0.clearId();

  KMyMoneySecurity* sec = dynamic_cast<KMyMoneySecurity*>(m_editWidgets["security"]);
  if(!isMultiSelection() || (isMultiSelection() && !sec->currentText().isEmpty())) {
    QString securityId = sec->selectedItem();
    if(!securityId.isEmpty()) {
      s0.setAccountId(securityId);
      MyMoneyAccount stockAccount = file->account(securityId);
      QString currencyId = stockAccount.currencyId();
      MyMoneySecurity security = file->security(currencyId);

      t.setCommodity(security.tradingCurrency());
    } else {
      s0.setAccountId(m_account.id());
      t.setCommodity(m_account.currencyId());
    }
  }

  // extract price info from original transaction
  m_priceInfo.clear();
  QValueList<MyMoneySplit>::const_iterator it_s;
  if(!torig.id().isEmpty()) {
    for(it_s = torig.splits().begin(); it_s != torig.splits().end(); ++it_s) {
      if((*it_s).id() != sorig.id()) {
        MyMoneyAccount cat = file->account((*it_s).accountId());
        if(cat.currencyId() != m_account.currencyId()) {
          if(!(*it_s).shares().isZero() && !(*it_s).value().isZero()) {
            m_priceInfo[cat.currencyId()] = ((*it_s).shares() / (*it_s).value()).reduce();
          }
        }
      }
    }
  }

  t.removeSplits();

  kMyMoneyDateInput* postDate = dynamic_cast<kMyMoneyDateInput*>(m_editWidgets["postdate"]);
  if(postDate->date().isValid()) {
    t.setPostDate(postDate->date());
  }

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

  MyMoneySplit assetAccountSplit;
  QValueList<MyMoneySplit> feeSplits;
  QValueList<MyMoneySplit> interestSplits;
  MyMoneySecurity security, currency;
  MyMoneySplit::investTransactionTypeE transactionType;

  // extract the splits from the original transaction
  dissectTransaction(torig, sorig,
                     assetAccountSplit,
                     feeSplits,
                     interestSplits,
                     security,
                     currency,
                     transactionType);

  // check if the trading currency is the same if the security has changed
  // in case it differs, check that we have a price (request from user)
  // and convert all splits
  // TODO

  // do the conversions here
  // TODO

  // keep the current activity object and create a new one
  // that can be destroyed later on
  Activity* activity = d->m_activity;
  d->m_activity = 0;      // make sure we create a new one
  activityFactory(activity->type());

  // if the activity is not set in the combo widget, we keep
  // the one which is used in the original transaction
  KMyMoneyActivityCombo* activityCombo = dynamic_cast<KMyMoneyActivityCombo*>(haveWidget("activity"));
  if(activityCombo->activity() == MyMoneySplit::UnknownTransactionType) {
    activityFactory(transactionType);
  }

  // if we mark the split reconciled here, we'll use today's date if no reconciliation date is given
  KMyMoneyReconcileCombo* status = dynamic_cast<KMyMoneyReconcileCombo*>(m_editWidgets["status"]);
  if(status->state() != MyMoneySplit::Unknown)
    s0.setReconcileFlag(status->state());

  if(s0.reconcileFlag() == MyMoneySplit::Reconciled && !s0.reconcileDate().isValid())
    s0.setReconcileDate(QDate::currentDate());

  // call the creation logic for the current selected activity
  bool rc = d->m_activity->createTransaction(t, s0, assetAccountSplit, feeSplits, m_feeSplits, interestSplits, m_interestSplits, security, currency);

  // now switch back to the original activity
  delete d->m_activity;
  d->m_activity = activity;

  // add the splits to the transaction
  if(rc) {
    if(!assetAccountSplit.accountId().isEmpty()) {
      assetAccountSplit.clearId();
      t.addSplit(assetAccountSplit);
    }

    t.addSplit(s0);

    QValueList<MyMoneySplit>::iterator it_s;
    for(it_s = feeSplits.begin(); it_s != feeSplits.end(); ++it_s) {
      (*it_s).clearId();
      t.addSplit(*it_s);
    }

    for(it_s = interestSplits.begin(); it_s != interestSplits.end(); ++it_s) {
      (*it_s).clearId();
      t.addSplit(*it_s);
    }
  }

  // adjust the value to the smallestAccountFraction found
  // for the commodity of the transaction.
  for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    MyMoneySplit s = (*it_s);
    s.setValue((*it_s).value().convert(currency.smallestAccountFraction()));
    t.modifySplit(s);
  }

  return rc;
}

void InvestTransactionEditor::updatePriceMode(const MyMoneySplit& split)
{
  QLabel* label = dynamic_cast<QLabel*>(haveWidget("price-label"));
  if(label) {
    kMyMoneyEdit* sharesEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
    kMyMoneyEdit* priceEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("price"));
    MyMoneyMoney price;
    if(!split.id().isEmpty())
      price = split.price().reduce();
    else
      price = priceEdit->value().abs();

    if(priceMode() == PricePerTransaction && label->text() != i18n("Price")) {
      label->setText(i18n("Price"));
      if(!sharesEdit->value().isZero())
        priceEdit->setValue(sharesEdit->value().abs() * price);

    } else if(priceMode() == PricePerShare && label->text() == i18n("Price")) {
      label->setText(i18n("Price/Share"));
      if(!sharesEdit->value().isZero())
        priceEdit->setValue(price / sharesEdit->value().abs());

    } else if(priceMode() == PricePerTransaction) {
      priceEdit->setValue(sharesEdit->value().abs() * price);

    } else
      priceEdit->setValue(price);
  }
}

void InvestTransactionEditor::setupFinalWidgets(void)
{
  addFinalWidget(haveWidget("memo"));
}

#include "investtransactioneditor.moc"

