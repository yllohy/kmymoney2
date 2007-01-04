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
#include <kmymoney/kmymoneypayee.h>
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

#include "../kmymoneysettings.h"

#include "investactivities.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;
using namespace Invest;

class InvestTransactionEditorPrivate {
public:
  InvestTransactionEditorPrivate() {
    m_activity = 0;
  }

  ~InvestTransactionEditorPrivate() {
    delete m_activity;
  }

  void activityFactory(KMyMoneyRegister::investTransactionTypeE type)
  {
    if(!m_activity || type != m_activity->type()) {
      delete m_activity;
      switch(type) {
        default:
        case BuyShares:
          m_activity = new Buy;
          break;
        case SellShares:
          m_activity = new Sell;
          break;
        case Dividend:
        case Yield:
          m_activity = new Div;
          break;
        case ReinvestDividend:
          m_activity = new Reinvest;
          break;
        case AddShares:
          m_activity = new Add;
          break;
        case RemoveShares:
          m_activity = new Remove;
          break;
        case SplitShares:
          m_activity = new Split;
          break;
      }
    }
  }
  Activity*      m_activity;
};


InvestTransactionEditor::InvestTransactionEditor()
{
  d = new InvestTransactionEditorPrivate;
}

InvestTransactionEditor::~InvestTransactionEditor()
{
  delete d;
}

InvestTransactionEditor::InvestTransactionEditor(TransactionEditorContainer* regForm, MyMoneyObjectContainer* objects, KMyMoneyRegister::InvestTransaction* item, const QValueList<KMyMoneyRegister::SelectedTransaction>& list, const QDate& lastPostDate) :
  TransactionEditor(regForm, objects, item, list, lastPostDate)
{
  d = new InvestTransactionEditorPrivate;

  // determine transaction type
  const MyMoneySplit& split = item->split();
  if(split.action() == MyMoneySplit::ActionAddShares) {
    d->activityFactory(!split.shares().isNegative() ? AddShares : RemoveShares);
  } else if(split.action() == MyMoneySplit::ActionBuyShares) {
    d->activityFactory(!split.shares().isNegative() ? BuyShares : SellShares);
  } else if(split.action() == MyMoneySplit::ActionDividend) {
    d->activityFactory(Dividend);
  } else if(split.action() == MyMoneySplit::ActionReinvestDividend) {
    d->activityFactory(ReinvestDividend);
  } else if(split.action() == MyMoneySplit::ActionYield) {
    d->activityFactory(Yield);
  } else if(split.action() == MyMoneySplit::ActionSplitShares) {
    d->activityFactory(SplitShares);
  } else {
    // we get here if we create an editor for a new transaction. So setup the defaults
    m_split = MyMoneySplit();
    d->activityFactory(BuyShares);
  }
  // collect the dissected splits
  item->splits(m_assetAccountSplit, m_interestSplits, m_feeSplits);
}


void InvestTransactionEditor::createEditWidgets(void)
{
  KMyMoneyActivityCombo* activity = new KMyMoneyActivityCombo();
  m_editWidgets["activity"] = activity;
  connect(activity, SIGNAL(activitySelected(KMyMoneyRegister::investTransactionTypeE)), this, SLOT(slotUpdateActivity(KMyMoneyRegister::investTransactionTypeE)));
  connect(activity, SIGNAL(activitySelected(KMyMoneyRegister::investTransactionTypeE)), this, SLOT(slotUpdateButtonState()));

  m_editWidgets["postdate"] = new kMyMoneyDateInput;

  KMyMoneySecurity* security = new KMyMoneySecurity;
  security->setHint(i18n("Security"));
  m_editWidgets["security"] = security;
  connect(security, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotUpdateSecurity(const QCString&)));
  connect(security, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(security, SIGNAL(createItem(const QString&, QCString&)), this, SLOT(slotCreateSecurity(const QString&, QCString&)));
  connect(security, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));

  KMyMoneyCategory* asset = new KMyMoneyCategory(0, 0, false);
  asset->setHint(i18n("Asset account"));
  m_editWidgets["asset-account"] = asset;
  connect(asset, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(asset, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));

  KMyMoneyCategory* fees = new KMyMoneyCategory(0, 0, true);
  fees->setHint(i18n("Fees"));
  m_editWidgets["fee-account"] = fees;
  connect(fees, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotUpdateFeeCategory(const QCString&)));
  connect(fees, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(fees, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateFeeVisibility(const QString&)));
  connect(fees, SIGNAL(createItem(const QString&, QCString&)), this, SLOT(slotCreateFeeCategory(const QString&, QCString&)));
  connect(fees, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));
  connect(fees->splitButton(), SIGNAL(clicked()), this, SLOT(slotEditFeeSplits()));
  // FIXME for now, hide the split button
  fees->splitButton()->hide();

  KMyMoneyCategory* interest = new KMyMoneyCategory(0, 0, true);
  interest->setHint(i18n("Interest"));
  m_editWidgets["interest-account"] = interest;
  connect(interest, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotUpdateInterestCategory(const QCString&)));
  connect(interest, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(interest, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateInterestVisibility(const QString&)));
  connect(interest, SIGNAL(createItem(const QString&, QCString&)), this, SLOT(slotCreateInterestCategory(const QString&, QCString&)));
  connect(interest, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));
  connect(interest->splitButton(), SIGNAL(clicked()), this, SLOT(slotEditInterestSplits()));
  // FIXME for now, hide the split button
  interest->splitButton()->hide();

  KTextEdit* memo = new KTextEdit;
  memo->setTabChangesFocus(true);
  m_editWidgets["memo"] = memo;

  kMyMoneyEdit* value = new kMyMoneyEdit;
  value->setHint(i18n("Shares"));
  value->setResetButtonVisible(false);
  m_editWidgets["shares"] = value;
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateShares(const QString&)));
  connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateTotalAmount()));

  value = new kMyMoneyEdit;
  value->setHint(i18n("Price"));
  value->setResetButtonVisible(false);
  m_editWidgets["price"] = value;
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdatePrice(const QString&)));
  connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateTotalAmount()));

  value = new kMyMoneyEdit;
  value->setResetButtonVisible(false);
  m_editWidgets["fee-amount"] = value;
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateFeeAmount(const QString&)));
  connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateTotalAmount()));

  value = new kMyMoneyEdit;
  value->setResetButtonVisible(false);
  m_editWidgets["interest-amount"] = value;
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateInterestAmount(const QString&)));
  connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateButtonState()));
  connect(value, SIGNAL(valueChanged(const QString&)), this, SLOT(slotUpdateTotalAmount()));

  QLabel* label = new QLabel("", 0);
  label->setAlignment(Qt::AlignVCenter | Qt::AlignRight | Qt::DontClip);
  m_editWidgets["total"] = label;

  KMyMoneyReconcileCombo* reconcile = new KMyMoneyReconcileCombo;
  m_editWidgets["status"] = reconcile;
  connect(reconcile, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotUpdateButtonState()));

  // if we don't have more than 1 selected transaction, we don't need
  // the "don't change" item in some of the combo widgets
  if(m_transactions.count() < 2) {
    reconcile->removeDontCare();
  }
}

int InvestTransactionEditor::slotEditFeeSplits(void)
{
  int rc = QDialog::Rejected;

  // force focus change to update all data
  QWidget* w = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["fee-account"])->splitButton();
  if(w)
    w->setFocus();

  kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(haveWidget("fee-amount"));
#if 0
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
#endif

  MyMoneyTransaction transaction;
  transaction.setCommodity(m_currency.id());
#if 0
  if(createPseudoTransaction(transaction, m_feeSplits)) {
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
#endif
  return rc;
}

#if 0
bool InvestTransactionEditor::createPseudoTransaction(MyMoneyTransaction& t, const QValueList<MyMoneySplit>& splits, KMyMoneyCategory* category, kMyMoneyEdit* amount)
{

}
#endif

int InvestTransactionEditor::slotEditInterestSplits(void)
{
}

void InvestTransactionEditor::slotCreateSecurity(const QString& name, QCString& id)
{
  MyMoneyAccount acc;
  QRegExp exp("([^:]+)");
  if(exp.search(name) != -1) {
    acc.setName(exp.cap(1));

    emit createSecurity(acc, m_account);

    // return id
    id = acc.id();
  }
}

void InvestTransactionEditor::slotCreateFeeCategory(const QString& name, QCString& id)
{
  MyMoneyAccount acc;
  acc.setName(name);

  emit createCategory(acc, MyMoneyFile::instance()->expense());

  // return id
  id = acc.id();
}

void InvestTransactionEditor::slotUpdateFeeCategory(const QCString& id)
{
}

void InvestTransactionEditor::slotUpdateFeeVisibility(const QString& txt)
{
  haveWidget("fee-amount")->setHidden(txt.isEmpty());
}

void InvestTransactionEditor::slotUpdateInterestCategory(const QCString& id)
{
}

void InvestTransactionEditor::slotUpdateInterestVisibility(const QString& txt)
{
  haveWidget("interest-amount")->setHidden(txt.isEmpty());
}

void InvestTransactionEditor::slotCreateInterestCategory(const QString& name, QCString& id)
{
  MyMoneyAccount acc;
  acc.setName(name);

  emit createCategory(acc, MyMoneyFile::instance()->income());

  // return id
  id = acc.id();
}

void InvestTransactionEditor::loadEditWidgets(KMyMoneyRegister::Action /* action */)
{
  QCString id;

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
    MyMoneyAccount acc = m_objects->account((*it_s).accountId());
    if(acc.accountType() == MyMoneyAccount::Equity)
      haveEquityAccount = true;
  }

  // asset-account
  AccountSet aSet(m_objects);
  aSet.clear();
  aSet.addAccountType(MyMoneyAccount::Checkings);
  aSet.addAccountType(MyMoneyAccount::Savings);
  aSet.addAccountType(MyMoneyAccount::Cash);
  aSet.addAccountType(MyMoneyAccount::Asset);
  aSet.addAccountType(MyMoneyAccount::Currency);
  if(KMyMoneySettings::expertMode() || haveEquityAccount)
    aSet.addAccountGroup(MyMoneyAccount::Equity);
  aSet.load(asset->selector());

  // security
  security->setSuppressObjectCreation(false);    // allow object creation on the fly
  aSet.load(security->selector(), i18n("Security"), m_account.accountList(), true);

  if(m_transactions.count() < 2) {
    // date
    postDate->setDate(m_transaction.postDate());

    // security
    security->completion()->setSelected(m_split.accountId());
    security->slotItemSelected(m_split.accountId());

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
    if(!m_split.shares().isZero())
      value->setValue(m_split.shares().abs());

    // price
    value = dynamic_cast<kMyMoneyEdit*>(haveWidget("price"));
    if(!m_split.shares().isZero()) {
      value->setValue(m_split.value() / m_split.shares());
    }

    // fee amount
    value = dynamic_cast<kMyMoneyEdit*>(haveWidget("fee-amount"));
    value->setValue(subtotal(m_feeSplits).abs());

    // interest amount
    value = dynamic_cast<kMyMoneyEdit*>(haveWidget("interest-amount"));
    value->setValue(subtotal(m_interestSplits).abs());

    // total
    slotUpdateTotalAmount();

    // status
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
    QValueList<KMyMoneyRegister::SelectedTransaction>::iterator it_t = m_transactions.begin();
    const QCString& action = m_item->split().action();
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
    }

    // if this is not the case, disable some more widgets
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
  return haveWidget("postdate");
}

bool InvestTransactionEditor::isComplete(void) const
{
  return d->m_activity->isComplete();
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

void InvestTransactionEditor::slotUpdateSecurity(const QCString& stockId)
{
  MyMoneyAccount stock = m_objects->account(stockId);
  m_security = m_objects->security(stock.currencyId());
  m_currency = m_objects->security(m_security.tradingCurrency());
  bool currencyKnown = !m_currency.id().isEmpty();
  if(!currencyKnown) {
    m_currency.setTradingSymbol("???");
  }

  haveWidget("shares")->setEnabled(currencyKnown);
  haveWidget("price")->setEnabled(currencyKnown);
  haveWidget("fee-amount")->setEnabled(currencyKnown);
  haveWidget("interest-amount")->setEnabled(currencyKnown);

  slotUpdateTotalAmount();
}

void InvestTransactionEditor::slotUpdateTotalAmount(void)
{
  QLabel* total = dynamic_cast<QLabel*>(haveWidget("total"));

  if(total && total->isVisible()) {
    KMyMoneyActivityCombo* activityCombo = dynamic_cast<KMyMoneyActivityCombo*>(haveWidget("activity"));
    kMyMoneyEdit* sharesEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
    kMyMoneyEdit* priceEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("price"));
    kMyMoneyEdit* feesEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("fee-amount"));
    kMyMoneyEdit* interestEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("interest-amount"));

    MyMoneyMoney amount;
    if(activityCombo->activity() != ReinvestDividend) {
      amount = sharesEdit->value().abs() * priceEdit->value().abs();
    }

    if(feesEdit->isVisible()) {
      MyMoneyMoney fee = feesEdit->value();
      if(activityCombo->activity() == BuyShares)
        amount += fee;
      else
        amount -= fee;
    }

    if(interestEdit->isVisible()) {
      amount += interestEdit->value();
    }

    total->setText(amount.formatMoney(m_currency.tradingSymbol(), MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction())));
  }
}

void InvestTransactionEditor::slotUpdateActivity(KMyMoneyRegister::investTransactionTypeE activity)
{
  // create new activity object if required
  d->activityFactory(activity);

  KMyMoneyCategory* cat;

  // hide all dynamic widgets (make sure to use the parentWidget for the
  // category widgets)
  haveWidget("interest-account")->parentWidget()->hide();
  haveWidget("fee-account")->parentWidget()->hide();
  haveWidget("asset-account")->hide();
  haveWidget("interest-amount")->hide();
  haveWidget("fee-amount")->hide();
  haveWidget("shares")->hide();
  haveWidget("price")->hide();
  haveWidget("total")->hide();

  d->m_activity->showWidgets(m_editWidgets);

  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
  if(cat->parentWidget()->isVisible())
    slotUpdateInterestVisibility(cat->currentText());

  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
  if(cat->parentWidget()->isVisible())
    slotUpdateFeeVisibility(cat->currentText());
}

bool InvestTransactionEditor::enterTransactions(QCString& newId)
{
  newId = QCString();

  // make sure to run through all stuff that is tied to 'focusout events'.
  m_regForm->parentWidget()->setFocus();
  QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 10);

  // we don't need to update our widgets anymore, so we just disconnect the signal
  disconnect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotReloadEditWidgets()));

  // for now, we don't store the transactions created
  return false;
}

#include "investtransactioneditor.moc"

