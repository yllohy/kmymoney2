/***************************************************************************
                          kledgerviewinvestments.cpp  -  description
                             -------------------
    begin                : Sun Mar 7 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#include <qwidgetstack.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerview.h"
#include "kledgerviewinvestments.h"

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/storage/imymoneystorage.h"

#include "../dialogs/knewaccountdlg.h"
#include "../dialogs/knewequityentrydlg.h"

#include "../widgets/kmymoneyregisterinvestment.h"
#include "../widgets/kmymoneytransactionform.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneyequity.h"

#include "../kapptest.h"

#define ACTIVITY_ROW      0
#define DATE_ROW          0
#define SYMBOL_ROW        1
#define QUANTITY_ROW      1
#define MEMO_ROW          2
#define PRICE_ROW         2
#define FEES_ROW          3
#define CATEGORY_ROW      3
#define ACCOUNT_ROW       4
#define VALUE_ROW         4

#define ACTIVITY_TXT_COL  0
#define ACTIVITY_DATA_COL (ACTIVITY_TXT_COL+1)
#define DATE_TXT_COL      3
#define DATE_DATA_COL     (DATE_TXT_COL+1)
#define SYMBOL_TXT_COL    0
#define SYMBOL_DATA_COL   (SYMBOL_TXT_COL+1)
#define QUANTITY_TXT_COL  3
#define QUANTITY_DATA_COL (QUANTITY_TXT_COL+1)
#define MEMO_TXT_COL      0
#define MEMO_DATA_COL     (MEMO_TXT_COL+1)
#define PRICE_TXT_COL     3
#define PRICE_DATA_COL    (PRICE_TXT_COL+1)
#define CATEGORY_TXT_COL  0
#define CATEGORY_DATA_COL (CATEGORY_TXT_COL+1)
#define FEES_TXT_COL      3
#define FEES_DATA_COL     (FEES_TXT_COL+1)
#define ACCOUNT_TXT_COL   0
#define ACCOUNT_DATA_COL  (ACCOUNT_TXT_COL+1)
#define VALUE_TXT_COL     3
#define VALUE_DATA_COL    (VALUE_TXT_COL+1)


KLedgerViewInvestments::KLedgerViewInvestments(QWidget *parent, const char *name) :
  KLedgerView(parent, name)
{
  QGridLayout* formLayout = new QGridLayout( this, 1, 2, 11, 6, "InvestmentFormLayout");
  QVBoxLayout* ledgerLayout = new QVBoxLayout(6, "InvestmentLedgerLayout");

  createRegister();
  ledgerLayout->addWidget(m_register, 3);

  createSummary();
  ledgerLayout->addLayout(m_summaryLayout);

  createForm();
  ledgerLayout->addWidget(m_form);

  formLayout->addLayout( ledgerLayout, 0, 0);

  m_editPPS = 0;
  m_editShares = 0;
  m_editStockAccount = 0;
  m_editFees = 0;
  m_editCashAccount = 0;
  m_editFeeCategory = 0;

  // setup the form to be visible or not
  slotShowTransactionForm(m_transactionFormActive);

  // and the register has the focus
  m_register->setFocus();

}

KLedgerViewInvestments::~KLedgerViewInvestments()
{

}

int KLedgerViewInvestments::actionTab(const MyMoneyTransaction& t, const MyMoneySplit& split) const
{
  if(KLedgerView::transactionType(t) == Transfer) {
    return 2;
  } else if(transactionDirection(split) == Credit) {
    return 1;
  } else if( split.action() == MyMoneySplit::ActionCheck){
    return 0;
  } else if(split.action() == MyMoneySplit::ActionATM) {
    return 4;
  }
  return 3;
}

void KLedgerViewInvestments::fillForm()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QTable* formTable = m_form->table();
  m_transactionPtr = transaction(m_register->currentTransactionIndex());

  // make sure, fields can use all available space
  // by spanning items over multiple cells if necessary
  formTable = m_form->table();
  QTableItem* tableItem;
  // activity
  tableItem = formTable->item(ACTIVITY_ROW, ACTIVITY_DATA_COL);
  if(tableItem)
    tableItem->setSpan(ACTIVITY_DATA_COL, 2);
  // symbol
  tableItem = formTable->item(SYMBOL_ROW, SYMBOL_DATA_COL);
  if(tableItem)
    tableItem->setSpan(SYMBOL_DATA_COL, 2);
  // quantity
  tableItem = formTable->item(QUANTITY_ROW, QUANTITY_DATA_COL);
  if(tableItem)
    tableItem->setSpan(QUANTITY_DATA_COL, 2);
  // memo
  tableItem = formTable->item(MEMO_ROW, MEMO_DATA_COL);
  if(tableItem)
    tableItem->setSpan(MEMO_DATA_COL, 2);
  // fee category
  tableItem = formTable->item(CATEGORY_ROW, CATEGORY_DATA_COL);
  if(tableItem)
    tableItem->setSpan(CATEGORY_DATA_COL, 2);
  // price per share
  tableItem = formTable->item(PRICE_ROW, PRICE_DATA_COL);
  if(tableItem)
    tableItem->setSpan(PRICE_DATA_COL, 2);
  // account
  tableItem = formTable->item(ACCOUNT_ROW, ACCOUNT_DATA_COL);
  if(tableItem)
    tableItem->setSpan(ACCOUNT_DATA_COL, 2);

  if(m_transactionPtr != 0)
  {
    // get my local copy of the selected transaction
    m_transaction = *m_transactionPtr;
    // extract specific splits from the transaction
    preloadInvestmentSplits(m_transaction);
    m_transactionType = transactionType(m_transaction, m_split);
    fillFormStatics();

    kMyMoneyTransactionFormTableItem* item;

/*
    // setup the fields first
    m_form->tabBar()->blockSignals(true);
    m_form->tabBar()->setCurrentTab(actionTab(m_transaction, m_split));
    m_form->tabBar()->blockSignals(false);
*/

    // fill in common fields

    MyMoneyAccount acc = file->account(m_split.accountId());
    MyMoneyEquity equity = file->equity(acc.currencyId());
    formTable->setText(SYMBOL_ROW, SYMBOL_DATA_COL, equity.tradingSymbol());
    formTable->setText(MEMO_ROW, MEMO_DATA_COL, m_split.memo());

    // make sure, that the date is aligned to the right of the cell
    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, KGlobal::locale()->formatDate(m_transaction.postDate(), true));
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    formTable->setItem(DATE_ROW, DATE_DATA_COL, item);

    QString fee, interest, accName;
    if(!m_feeSplit.id().isEmpty()) {
      fee = file->accountToCategory(m_feeSplit.accountId());
    }
    if(!m_interestSplit.id().isEmpty()) {
      interest = file->accountToCategory(m_interestSplit.accountId());
    }
    if(!m_accountSplit.id().isEmpty()) {
      accName = file->accountToCategory(m_accountSplit.accountId());
    }

    MyMoneyMoney amount, shares;
    switch(m_transactionType) {
      case BuyShares:
      case SellShares:
        if(m_transactionType == BuyShares)
          formTable->setText(ACTIVITY_ROW, ACTIVITY_DATA_COL, i18n("Buy Shares"));
        else
          formTable->setText(ACTIVITY_ROW, ACTIVITY_DATA_COL, i18n("Sell Shares"));

        formTable->setText(CATEGORY_ROW, CATEGORY_DATA_COL, fee);
        formTable->setText(ACCOUNT_ROW, ACCOUNT_DATA_COL, accName);

        // total amount
        amount = m_accountSplit.value(m_transaction.commodity(), m_account.currencyId()).abs();
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 amount.formatMoney());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(VALUE_ROW, VALUE_DATA_COL, item);

        // shares
        shares = m_split.shares().abs();
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 shares.formatMoney());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(QUANTITY_ROW, QUANTITY_DATA_COL, item);

        // price
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 (m_split.value(m_transaction.commodity(), m_account.currencyId())/m_split.shares()).formatMoney());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(PRICE_ROW, PRICE_DATA_COL, item);

        // fees
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 m_feeSplit.value(m_transaction.commodity(), m_account.currencyId()).abs().formatMoney());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(FEES_ROW, FEES_DATA_COL, item);

        break;

      case ReinvestDividend:
        formTable->setText(ACTIVITY_ROW, ACTIVITY_DATA_COL, i18n("Reinvest Dividend"));
        formTable->setText(CATEGORY_ROW, CATEGORY_DATA_COL, fee);
        formTable->setText(ACCOUNT_ROW, ACCOUNT_DATA_COL, interest);

        // total amount
        amount = m_interestSplit.value(m_transaction.commodity(), m_account.currencyId()).abs();
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 amount.formatMoney());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(VALUE_ROW, VALUE_DATA_COL, item);

        // shares
        shares = m_split.shares().abs();
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 shares.formatMoney());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(QUANTITY_ROW, QUANTITY_DATA_COL, item);

        // price
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 (m_split.value(m_transaction.commodity(), m_account.currencyId())/m_split.shares()).formatMoney());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(PRICE_ROW, PRICE_DATA_COL, item);

        // fees
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 m_feeSplit.value(m_transaction.commodity(), m_account.currencyId()).abs().formatMoney());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(FEES_ROW, FEES_DATA_COL, item);
        break;

      case Dividend:
      case Yield:
        if(m_transactionType == Dividend)
          formTable->setText(ACTIVITY_ROW, ACTIVITY_DATA_COL, i18n("Dividend"));
        else
          formTable->setText(ACTIVITY_ROW, ACTIVITY_DATA_COL, i18n("Yield"));

        formTable->setText(CATEGORY_ROW, CATEGORY_DATA_COL, interest);
        formTable->setText(ACCOUNT_ROW, ACCOUNT_DATA_COL, accName);

        // total amount
        amount = m_accountSplit.value(m_transaction.commodity(), m_account.currencyId()).abs();
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 amount.formatMoney());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(FEES_ROW, FEES_DATA_COL, item);

        break;

      case AddShares:
      case RemoveShares:
        if(m_transactionType == AddShares)
          formTable->setText(ACTIVITY_ROW, ACTIVITY_DATA_COL, i18n("Add Shares"));
        else
          formTable->setText(ACTIVITY_ROW, ACTIVITY_DATA_COL, i18n("Remove Shares"));

        // shares
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 m_split.shares().abs().formatMoney());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(QUANTITY_ROW, QUANTITY_DATA_COL, item);

        break;
    }

/*
    // collect values
    QString payee;
    try {
      payee = MyMoneyFile::instance()->payee(m_split.payeeId()).name();
    } catch (MyMoneyException *e) {
      delete e;
      payee = " ";
    }

    QString category;
    try {
      MyMoneySplit s;
      switch(m_transaction.splitCount()) {
        case 2:
          if(m_transaction.isLoanPayment())
            category = i18n("Loan Payment");
          else {
            s = m_transaction.splitByAccount(accountId(), false);
            category = MyMoneyFile::instance()->accountToCategory(s.accountId());
          }
          break;

        case 1:
          category = " ";
          break;

        default:
          if(m_transaction.isLoanPayment())
            category = i18n("Loan Payment");
          else
            category = i18n("Split transaction");
          break;
      }
    } catch (MyMoneyException *e) {
      delete e;
      category = " ";
    }

    QString from;
    try {
      from = MyMoneyFile::instance()->accountToCategory(m_account.id());
    } catch (MyMoneyException *e) {
      delete e;
      from = " ";
    }

//    formTable->setText(PAYEE_ROW, PAYEE_DATA_COL, payee);
 //   formTable->setText(CATEGORY_ROW, CATEGORY_DATA_COL, category);
    switch( actionTab(m_transaction, m_split) ){
      case 0:      // check
      case 4:      // ATM
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, m_split.number());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
//        formTable->setItem(NR_ROW, NR_DATA_COL, item);
        break;

      default:
        break;
    }

    MyMoneyMoney amount = m_split.value(m_transaction.commodity(), m_account.currencyId()).abs();

    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 m_split.value(m_transaction.commodity(), m_account.currencyId()).abs().formatMoney());
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    //formTable->setItem(AMOUNT_ROW, AMOUNT_DATA_COL, item);

    m_form->newButton()->setEnabled(true);
    m_form->editButton()->setEnabled(true);
    m_form->enterButton()->setEnabled(false);
    m_form->cancelButton()->setEnabled(false);
    m_form->moreButton()->setEnabled(true);
*/
  } else {
    m_transaction = MyMoneyTransaction();
    m_split = MyMoneySplit();
    m_accountSplit = MyMoneySplit();
    m_feeSplit = MyMoneySplit();
    m_interestSplit = MyMoneySplit();

/*
    m_split.setAccountId(accountId());
    m_split.setAction(m_action);

    m_transaction.addSplit(m_split);
    m_transaction.setCommodity(m_account.currencyId());
*/
    fillFormStatics();

    m_form->newButton()->setEnabled(true);
    m_form->editButton()->setEnabled(false);
    m_form->enterButton()->setEnabled(false);
    m_form->cancelButton()->setEnabled(false);
    m_form->moreButton()->setEnabled(false);
  }
}

void KLedgerViewInvestments::fillFormStatics(void)
{
  QTable* formTable = m_form->table();

  // clear complete table, but leave the cell widgets while editing
  for(int r = 0; r < formTable->numRows(); ++r) {
    for(int c = 0; c < formTable->numCols(); ++c) {
      if(formTable->cellWidget(r, c) == 0)
        formTable->setText(r, c, " ");
    }
  }

  // common elements
  formTable->setText(ACTIVITY_ROW, ACTIVITY_TXT_COL, i18n("Activity"));
  formTable->setText(DATE_ROW, DATE_TXT_COL, i18n("Date"));
  formTable->setText(SYMBOL_ROW, SYMBOL_TXT_COL, i18n("Symbol Name"));
  formTable->setText(MEMO_ROW, MEMO_TXT_COL, i18n("Memo"));

  switch(m_transactionType) {
    case BuyShares:
    case SellShares:
      formTable->setText(QUANTITY_ROW, QUANTITY_TXT_COL, i18n("Shares"));
      formTable->setText(PRICE_ROW, PRICE_TXT_COL, i18n("Price per share"));
      formTable->setText(VALUE_ROW, VALUE_TXT_COL, i18n("Total Amount"));
      formTable->setText(FEES_ROW, FEES_TXT_COL, i18n("Fees"));
      formTable->setText(CATEGORY_ROW, CATEGORY_TXT_COL, i18n("Fee Category"));
      formTable->setText(ACCOUNT_ROW, ACCOUNT_TXT_COL, i18n("Account"));
      break;

    case ReinvestDividend:
      formTable->setText(QUANTITY_ROW, QUANTITY_TXT_COL, i18n("Shares"));
      formTable->setText(PRICE_ROW, PRICE_TXT_COL, i18n("Price per share"));
      formTable->setText(VALUE_ROW, VALUE_TXT_COL, i18n("Total Amount"));
      formTable->setText(FEES_ROW, FEES_TXT_COL, i18n("Fees"));
      formTable->setText(CATEGORY_ROW, CATEGORY_TXT_COL, i18n("Fee Category"));
      formTable->setText(ACCOUNT_ROW, ACCOUNT_TXT_COL, i18n("Interest category"));
      break;

    case Dividend:
    case Yield:
      formTable->setText(CATEGORY_ROW, CATEGORY_TXT_COL, i18n("Interest category"));
      formTable->setText(FEES_ROW, FEES_TXT_COL, i18n("Interest"));
      formTable->setText(ACCOUNT_ROW, ACCOUNT_TXT_COL, i18n("Account"));
      break;

    case AddShares:
    case RemoveShares:
      formTable->setText(QUANTITY_ROW, QUANTITY_TXT_COL, i18n("Shares"));
      break;
  }
  resizeEvent(0);
}

void KLedgerViewInvestments::fillSummary()
{

}

QWidget* KLedgerViewInvestments::arrangeEditWidgetsInRegister(void)
{
}

QWidget* KLedgerViewInvestments::arrangeEditWidgetsInForm(void)
{
  kMyMoneyTransactionFormTable* table = m_form->table();

  Q_CHECK_PTR(table);

  // make sure, we're using the right palette
  QPalette palette = m_register->palette();
  m_editMemo->setPalette(palette);
  m_editDate->setPalette(palette);
  m_editPPS->setPalette(palette);
  m_editStockAccount->setPalette(palette);
  m_editShares->setPalette(palette);
  m_editFees->setPalette(palette);
  m_editType->setPalette(palette);
  m_editFeeCategory->setPalette(palette);
  m_editCashAccount->setPalette(palette);

  // arrange common widgets
  setFormCellWidget(ACTIVITY_ROW, ACTIVITY_DATA_COL, m_editType);
  setFormCellWidget(DATE_ROW, DATE_DATA_COL, m_editDate);
  setFormCellWidget(SYMBOL_ROW, SYMBOL_DATA_COL, m_editStockAccount);
  setFormCellWidget(MEMO_ROW, MEMO_DATA_COL, m_editMemo);

  // arrange variable widgets
  setFormCellWidget(QUANTITY_ROW, QUANTITY_DATA_COL, m_editShares);
  setFormCellWidget(PRICE_ROW, PRICE_DATA_COL, m_editPPS);
  setFormCellWidget(FEES_ROW, FEES_DATA_COL, m_editFees);
  setFormCellWidget(CATEGORY_ROW, CATEGORY_DATA_COL, m_editFeeCategory);
  setFormCellWidget(ACCOUNT_ROW, ACCOUNT_DATA_COL, m_editCashAccount);

  table->clearEditable();
  table->setEditable(ACTIVITY_ROW, ACTIVITY_DATA_COL);
  table->setEditable(DATE_ROW, DATE_DATA_COL);
  table->setEditable(SYMBOL_ROW, SYMBOL_DATA_COL);
  table->setEditable(MEMO_ROW, MEMO_DATA_COL);

  // show all variable widgets, we hide the ones we
  // don't need for the current case later on again
  m_editPPS->show();
  m_editShares->show();
  m_editFees->show();
  m_editCashAccount->show();
  m_editFeeCategory->show();

  switch(m_editType->currentItem()) {
    case ReinvestDividend:
      m_editCashAccount->loadList(static_cast<KMyMoneyUtils::categoryTypeE>(KMyMoneyUtils::income));
      // tricky fall through here

    case BuyShares:
    case SellShares:
      table->setEditable(QUANTITY_ROW, QUANTITY_DATA_COL);
      table->setEditable(PRICE_ROW, PRICE_DATA_COL);
      table->setEditable(VALUE_ROW, VALUE_DATA_COL, false);
      table->setEditable(FEES_ROW, FEES_DATA_COL);
      table->setEditable(CATEGORY_ROW, CATEGORY_DATA_COL);
      table->setEditable(ACCOUNT_ROW, ACCOUNT_DATA_COL);
      break;

    case Dividend:
    case Yield:
      m_editShares->hide();
      m_editPPS->hide();

      table->setEditable(CATEGORY_ROW, CATEGORY_DATA_COL);
      table->setEditable(ACCOUNT_ROW, ACCOUNT_DATA_COL);
      break;

    case AddShares:
    case RemoveShares:
      m_editPPS->hide();
      m_editFees->hide();
      m_editFeeCategory->hide();
      m_editCashAccount->hide();

      table->setEditable(QUANTITY_ROW, QUANTITY_DATA_COL);
      break;
  }

  // now setup the tab order
  m_tabOrderWidgets.clear();
  m_tabOrderWidgets.append(m_form->enterButton());
  m_tabOrderWidgets.append(m_form->cancelButton());
  m_tabOrderWidgets.append(m_form->moreButton());
  m_tabOrderWidgets.append(m_editType);
  m_tabOrderWidgets.append(m_editDate);
  m_tabOrderWidgets.append(m_editStockAccount);
  m_tabOrderWidgets.append(m_editShares);
  m_tabOrderWidgets.append(m_editMemo);
  m_tabOrderWidgets.append(m_editPPS);
  m_tabOrderWidgets.append(m_editFeeCategory);
  m_tabOrderWidgets.append(m_editFees);
  m_tabOrderWidgets.append(m_editCashAccount);

  updateTotalAmount();

  return m_editStockAccount;
}

void KLedgerViewInvestments::hideWidgets()
{
  for(int i=0; i < m_form->table()->numRows(); ++i) {
    m_form->table()->clearCellWidget(i, 1);
    m_form->table()->clearCellWidget(i, 2);
    m_form->table()->clearCellWidget(i, 4);
  }

  int   firstRow = m_register->currentTransactionIndex() * m_register->rpt();
  for(int i = 0; i < m_register->maxRpt(); ++i) {
    for(int j = 0; j < m_register->numCols(); ++j) {
      m_register->clearCellWidget(firstRow+i, j);
    }
  }

  m_editPayee = 0;
  m_editCategory = 0;
  m_editMemo = 0;
  m_editAmount = 0;
  m_editNr = 0;
  m_editDate = 0;
  m_editSplit = 0;
  m_editType = 0;
  m_editPayment = 0;
  m_editDeposit = 0;

  m_editPPS = 0;
  m_editShares = 0;
  m_editStockAccount = 0;
  m_editFees = 0;
  m_editCashAccount = 0;
  m_editFeeCategory = 0;

  m_form->table()->clearEditable();
  m_form->tabBar()->setEnabled(true);
}

void KLedgerViewInvestments::preloadInvestmentSplits(const MyMoneyTransaction& t)
{
  m_split =
  m_feeSplit =
  m_interestSplit =
  m_accountSplit = MyMoneySplit();

  // find the split that references the stock account
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
    if(acc.accountType() == MyMoneyAccount::Stock) {
      m_split = *it_s;
    } else if(acc.accountGroup() == MyMoneyAccount::Expense) {
      m_feeSplit = *it_s;
    } else if(acc.accountGroup() == MyMoneyAccount::Income) {
      m_interestSplit = *it_s;
    } else if(acc.accountGroup() == MyMoneyAccount::Asset
            || acc.accountGroup() == MyMoneyAccount::Liability) {
      m_accountSplit = *it_s;
    }
  }
}

void KLedgerViewInvestments::reloadEditWidgets(const MyMoneyTransaction& t)
{
  Q_CHECK_PTR(m_editType);

  m_transaction = t;
  preloadInvestmentSplits(t);

  // Determine the actual action
  if(m_split.action() == MyMoneySplit::ActionBuyShares) {
    if(m_split.value() < 0)
      m_editType->setCurrentItem(SellShares);
    else
      m_editType->setCurrentItem(BuyShares);

  } else if(m_split.action() == MyMoneySplit::ActionAddShares) {
    if(m_split.value() < 0)
      m_editType->setCurrentItem(RemoveShares);
    else
      m_editType->setCurrentItem(AddShares);

  } else if(m_split.action() == MyMoneySplit::ActionDividend) {
      m_editType->setCurrentItem(Dividend);

  } else if(m_split.action() == MyMoneySplit::ActionYield) {
      m_editType->setCurrentItem(Yield);

  } else if(m_split.action() == MyMoneySplit::ActionReinvestDividend) {
      m_editType->setCurrentItem(ReinvestDividend);

  } else {
    qDebug("Unknown MyMoneySplit::Action%s in KLedgerViewInvestments::reloadEditWidgets", m_split.action().data());
    return;
  }

  // Fill the fields - first the ones that are in any transaction
  if(m_editStockAccount)
    m_editStockAccount->slotSelected(m_split.accountId());
  if(m_editDate)
    m_editDate->setDate(m_transaction.postDate());
  if(m_editMemo)
    m_editMemo->loadText(m_split.memo());

  // Fill the fields - next the ones that are specific to the transaction
  MyMoneyMoney shares, price, amount;
  switch(m_editType->currentItem()) {
    case BuyShares:
    case SellShares:
      shares = m_split.shares();
      price = m_split.value() / shares;
      if(m_editType->currentItem() == SellShares)
        shares = -shares;
      if(m_editShares)
        m_editShares->loadText(shares.formatMoney());
      if(m_editPPS)
        m_editPPS->loadText(price.formatMoney());
      if(m_editCashAccount)
        m_editCashAccount->slotSelected(m_accountSplit.accountId());
      if(m_editFeeCategory)
        m_editFeeCategory->loadAccount(m_feeSplit.accountId());
      if(!m_feeSplit.accountId().isEmpty())
        m_editFees->loadText(m_feeSplit.value().formatMoney());
      break;

    case ReinvestDividend:
      shares = m_split.shares();
      price = m_split.value() / shares;
      if(m_editShares)
        m_editShares->loadText(shares.formatMoney());
      if(m_editPPS)
        m_editPPS->loadText(price.formatMoney());
      if(m_editCashAccount)
        m_editCashAccount->slotSelected(m_interestSplit.accountId());
      if(m_editFeeCategory)
        m_editFeeCategory->loadAccount(m_feeSplit.accountId());
      if(!m_feeSplit.accountId().isEmpty())
        m_editFees->loadText(m_feeSplit.value().formatMoney());
      break;

    case Dividend:
    case Yield:
      if(m_editFeeCategory)
        m_editFeeCategory->loadAccount(m_interestSplit.accountId());
      if(!m_interestSplit.accountId().isEmpty())
        m_editFees->loadText((-m_interestSplit.value()).formatMoney());
      if(m_editCashAccount)
        m_editCashAccount->slotSelected(m_accountSplit.accountId());
      break;

    case AddShares:
    case RemoveShares:
      shares = m_split.shares();
      if(m_editType->currentItem() == RemoveShares)
        shares = -shares;
      if(m_editShares)
        m_editShares->loadText(shares.formatMoney());
      break;
  }
#if 0
  try {
    if(!m_split.payeeId().isEmpty())
      payee = MyMoneyFile::instance()->payee(m_split.payeeId()).name();

    MyMoneySplit s;
    MyMoneyAccount acc;

  } catch(MyMoneyException *e) {
    qDebug("Exception '%s' thrown in %s, line %ld caught in KLedgerViewCheckings::reloadEditWidgets():%d",
      e->what().latin1(), e->file().latin1(), e->line(), __LINE__-2);
    delete e;
  }

  if(m_editMemo != 0)
    m_editMemo->loadText(m_split.memo());
  if(m_editAmount != 0)
    m_editAmount->loadText(amount.abs().formatMoney());
  if(m_editDate != 0 && m_transactionPtr)
    m_editDate->loadDate(m_transactionPtr->postDate());
  if(m_editNr != 0)
    m_editNr->loadText(m_split.number());

  if(m_editPayment != 0 && m_editDeposit != 0) {
    if(m_split.value() < 0) {
      m_editPayment->loadText((-m_split.value()).formatMoney());
      m_editDeposit->loadText(QString());
    } else {
      m_editPayment->loadText(QString());
      m_editDeposit->loadText((m_split.value()).formatMoney());
    }
  }
#endif
}

void KLedgerViewInvestments::loadEditWidgets(void)
{
  if(m_transactionPtr != 0) {
    reloadEditWidgets(*m_transactionPtr);
  } else {
    m_editDate->setDate(m_lastPostDate);
  }

  // the next line prevents an endless loop caused in
  // slotDataChanged()
  m_transactionType = static_cast<investTransactionTypeE> (m_editType->currentItem());
  slotDataChanged();
}

void KLedgerViewInvestments::slotReconciliation(void)
{

}

void KLedgerViewInvestments::slotNew()
{
  m_transactionType = BuyShares;
  KLedgerView::slotNew();
}

void KLedgerViewInvestments::createEditWidgets()
{
  if(!m_editMemo) {
    m_editMemo = new kMyMoneyLineEdit(0, "editMemo", AlignLeft|AlignVCenter);
    connect(m_editMemo, SIGNAL(lineChanged(const QString&)), this, SLOT(slotDataChanged()));
    connect(m_editMemo, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editMemo, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }
  if(!m_editDate) {
    m_editDate = new kMyMoneyDateInput(0, "editDate");
    connect(m_editDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged(const QDate&)));
    connect(m_editDate, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editDate, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }
  if(!m_editShares) {
    m_editShares = new kMyMoneyEdit(0, "editShares");
    connect(m_editShares, SIGNAL(valueChanged(const QString& )), this, SLOT(slotDataChanged()));
    connect(m_editShares, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editShares, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }
  if(!m_editPPS) {
    m_editPPS = new kMyMoneyEdit(0, "editPPS");
    connect(m_editPPS, SIGNAL(valueChanged(const QString& )), this, SLOT(slotDataChanged()));
    connect(m_editPPS, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editPPS, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }
  if(!m_editStockAccount) {
    m_editStockAccount = new kMyMoneyAccountCombo(0, "editStockAccount");
    m_editStockAccount->setMinimumWidth(0);  // override the widgets default
    m_editStockAccount->setFocusPolicy(QWidget::StrongFocus);
    m_editStockAccount->loadList(i18n("Stocks"), m_account.accountList());
    connect(m_editStockAccount, SIGNAL(accountSelected(const QCString&)), this, SLOT(slotDataChanged()));
    connect(m_editStockAccount, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editStockAccount, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }
  if(!m_editFees) {
    m_editFees = new kMyMoneyEdit(0, "editFees");
    connect(m_editFees, SIGNAL(valueChanged(const QString& )), this, SLOT(slotDataChanged()));
    connect(m_editFees, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editFees, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }

  if(!m_editType) {
    m_editType = new kMyMoneyCombo(0, "editType");
    m_editType->setFocusPolicy(QWidget::StrongFocus);
    m_editType->insertItem(i18n("Buy Shares"), BuyShares);
    m_editType->insertItem(i18n("Sell Shares"), SellShares);
    m_editType->insertItem(i18n("Dividend"), Dividend);
    m_editType->insertItem(i18n("Reinvest Dividend"), ReinvestDividend);
    m_editType->insertItem(i18n("Yield"), Yield);
    m_editType->insertItem(i18n("Add Shares"), AddShares);
    m_editType->insertItem(i18n("Remove Shares"), RemoveShares);
    connect(m_editType, SIGNAL(activated(int)), this, SLOT(slotDataChanged()));
    connect(m_editType, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editType, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }

  if(!m_editCashAccount) {
    m_editCashAccount = new kMyMoneyAccountCombo(0, "editCashAccount");
    m_editCashAccount->setMinimumWidth(0);
    m_editCashAccount->setFocusPolicy(QWidget::StrongFocus);

    QValueList<int> typeList;
    typeList << MyMoneyAccount::Checkings;
    typeList << MyMoneyAccount::Savings;
    typeList << MyMoneyAccount::Cash;
    typeList << MyMoneyAccount::CreditCard;
    typeList << MyMoneyAccount::Loan;
    typeList << MyMoneyAccount::Asset;
    typeList << MyMoneyAccount::Liability;
    typeList << MyMoneyAccount::Currency;
    typeList << MyMoneyAccount::AssetLoan;
    m_editCashAccount->loadList(typeList);

    connect(m_editCashAccount, SIGNAL(accountSelected(const QCString&)), this, SLOT(slotDataChanged()));
    connect(m_editCashAccount, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editCashAccount, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }

  if(!m_editFeeCategory) {
    m_editFeeCategory = new kMyMoneyCategory(0, "editFeeCategory", KMyMoneyUtils::expense);
    connect(m_editFeeCategory, SIGNAL(categoryChanged(const QCString&)), this, SLOT(slotDataChanged()));
    m_editFeeCategory->setFocusPolicy(QWidget::StrongFocus);
    connect(m_editFeeCategory, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editFeeCategory, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }
}

void KLedgerViewInvestments::createForm(void)
{
  // determine the height of the objects in the table
  kMyMoneyDateInput dateInput(0, "editDate");
  KPushButton splitButton(i18n("Split"), 0, "editSplit");
  kMyMoneyCategory category(0, "category");

  // extract the maximal sizeHint height and subtract 8
  int h = QMAX(dateInput.sizeHint().height(), splitButton.sizeHint().height());
  h = QMAX(h, category.sizeHint().height())-8;

  m_form = new kMyMoneyTransactionForm(this, NULL, 0, 5, 5, h);

  // m_tabAddShares = new QTab(action2str(MyMoneySplit::ActionAddShares, true));
  // m_tabRemoveShares = new QTab(action2str(MyMoneySplit::ActionRemoveShares, true));
  // m_tabTransfer = new QTab(action2str(MyMoneySplit::ActionTransfer, true));
  //m_tabWithdrawal = new QTab(action2str(MyMoneySplit::ActionWithdrawal, true));
  //m_tabAtm = new QTab(action2str(MyMoneySplit::ActionATM, true));

  // m_form->addTab(m_tabAddShares);
  // m_form->addTab(m_tabRemoveShares);
  // m_form->addTab(m_tabTransfer);
  //m_form->addTab(m_tabWithdrawal);
  //m_form->addTab(m_tabAtm);


  // never show horizontal scroll bars
  m_form->table()->setHScrollBarMode(QScrollView::AlwaysOff);

  // adjust size of form table
  m_form->table()->setMaximumHeight(m_form->table()->rowHeight(0)*m_form->table()->numRows());
  m_form->table()->setMinimumHeight(m_form->table()->rowHeight(0)*m_form->table()->numRows());

  // connections
  connect(m_form->tabBar(), SIGNAL(selected(int)), this, SLOT(slotTypeSelected(int)));

  connect(m_form->editButton(), SIGNAL(clicked()), this, SLOT(slotStartEdit()));
  connect(m_form->cancelButton(), SIGNAL(clicked()), this, SLOT(slotCancelEdit()));
  connect(m_form->enterButton(), SIGNAL(clicked()), this, SLOT(slotEndEdit()));
  connect(m_form->newButton(), SIGNAL(clicked()), this, SLOT(slotNew()));

  // m_form->enterButton()->setDefault(true);

  // slotTypeSelected(KLedgerViewInvestments::AddShares);
}

#if 0
void KLedgerViewInvestments::createInfoStack(void)
{
  // create the widget stack first
  KLedgerView::createInfoStack();

  // First page buttons inside a frame with layout
  QFrame* frame = new QFrame(m_infoStack, "ButtonFrame");

  frame->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                     QSizePolicy::Minimum,
                                     0, 0,
                                     frame->sizePolicy().hasHeightForWidth() ) );

  QVBoxLayout* buttonLayout = new QVBoxLayout( frame, 0, 6, "ButtonLayout");

  KIconLoader* il = KGlobal::iconLoader();

  m_detailsButton = new KPushButton(frame, "detailsButton" );
  KGuiItem detailsButtenItem( i18n("&Account Details"),
                    QIconSet(il->loadIcon("viewmag", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Modify the loan details for this loan"),
                    i18n("Use this to start a wizard that allows changing the details for this loan."));
  m_detailsButton->setGuiItem(detailsButtenItem);
  buttonLayout->addWidget(m_detailsButton);

  m_reconcileButton = new KPushButton(frame, "reconcileButton");
  KGuiItem reconcileButtenItem( i18n("&Reconcile ..."),
                    QIconSet(il->loadIcon("reconcile", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Start the account reconciliation"),
                    i18n("Use this to reconcile your account against the bank statement."));
  m_reconcileButton->setGuiItem(reconcileButtenItem);
  buttonLayout->addWidget(m_reconcileButton);

  m_lastReconciledLabel = new QLabel("", frame);
  buttonLayout->addWidget(m_lastReconciledLabel);

/*
  // FIXME: This should not be required anymore as this
  //        this type of stuff is handled in the KEditLoanWizard
  m_interestButton = new KPushButton(frame, "interestButton");
  KGuiItem interestButtonItem( i18n("&Modify interest..."),
                    QIconSet(il->loadIcon("edit", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Modify the interest rate for this loan"),
                    i18n("Use this to start a wizard that allows changing the interest rate."));
  m_interestButton->setGuiItem(interestButtonItem);
  buttonLayout->addWidget(m_interestButton);

  m_loanDetailsButton = new KPushButton(frame, "loanDetailsButton");
  KGuiItem loanDetailsButtonItem( i18n("&Modify loan details..."),
                    QIconSet(il->loadIcon("configure", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Modify the loan details for this loan"),
                    i18n("Use this to start a wizard that allows changing the details for this loan."));
  m_loanDetailsButton->setGuiItem(loanDetailsButtonItem);
  buttonLayout->addWidget(m_loanDetailsButton);

  connect(m_reconcileButton, SIGNAL(clicked()), this, SLOT(slotReconciliation()));
  connect(m_detailsButton, SIGNAL(clicked()), this, SLOT(slotLoanAccountDetail()));
  // connect(m_loanDetailsButton, SIGNAL(clicked()), this, SLOT(slotLoanAccountDetail()));

  // FIXME: add functionality to modify interest rate etc.
  // For now just show the proposed functionality
  m_interestButton->setEnabled(false);
  m_loanDetailsButton->setEnabled(false);
*/
  connect(m_detailsButton, SIGNAL(clicked()), this, SLOT(slotAccountDetail()));
  connect(m_reconcileButton, SIGNAL(clicked()), this, SLOT(slotReconciliation()));

  QSpacerItem* spacer = new QSpacerItem( 20, 20,
                   QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonLayout->addItem( spacer );

  m_infoStack->addWidget(frame, KLedgerView::TransactionEdit);

  // Initially show the page with the buttons
  m_infoStack->raiseWidget(KLedgerView::TransactionEdit);
}
#endif

void KLedgerViewInvestments::slotTypeSelected(int type)
{
  if(!m_form->tabBar()->signalsBlocked())
    slotCancelEdit();


#if 0
  QTable* formTable = m_form->table();
  // clear complete table
  for(int r = 0; r < formTable->numRows(); ++r) {
    for(int c = 0; c < formTable->numCols(); ++c) {
      formTable->setText(r, c, " ");
    }
  }

  // common elements

  formTable->setText(DATE_ROW, DATE_TXT_COL, i18n("Date"));
  formTable->setText(AMOUNT_ROW, AMOUNT_TXT_COL, i18n("Total Amount"));

  m_action = transactionType(type);
  qDebug("TransactionType = %d", type);

  // specific elements (in the order of the tabs)
  switch(type) {
    case KLedgerViewInvestments::AddShares:
    case KLedgerViewInvestments::RemoveShares:
    {
      formTable->setText(SYMBOL_ROW, SYMBOL_TXT_COL, i18n("Symbol Name"));
      formTable->setText(QUANTITY_ROW, QUANTITY_TXT_COL, i18n("Shares"));
      formTable->setText(MEMO_ROW, MEMO_TXT_COL, i18n("Memo"));
      formTable->setText(PRICE_ROW, PRICE_TXT_COL, i18n("Price Per Share"));
      formTable->setText(FEES_ROW, FEES_TXT_COL, i18n("Commission"));
      break;
    }

    case KLedgerViewInvestments::Transfer:
    {
      formTable->setText(SYMBOL_ROW, SYMBOL_TXT_COL, i18n("From"));
      formTable->setText(1, 0, i18n("To"));
      formTable->setText(MEMO_ROW, MEMO_TXT_COL, i18n("Payee"));
      break;
    }
/*
    case 3:   // Withdrawal
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));
      break;

    case 4:   // ATM
      formTable->setText(0, 3, i18n("Nr"));
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));
      break;     */
  }

  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  if(config->readBoolEntry("AlwaysShowNrField", false) == true)
    formTable->setText(0, 3, i18n("Nr"));
#endif

  if(!m_form->tabBar()->signalsBlocked())
    slotNew();
}

void KLedgerViewInvestments::slotRegisterDoubleClicked(int /* row */,
                                                int /* col */,
                                                int /* button */,
                                                const QPoint & /* mousePos */)
{
  if(m_transactionPtr != 0)
    slotStartEdit();
}

void KLedgerViewInvestments::createRegister(void)
{
  m_register = new kMyMoneyRegisterInvestment(this, KAppTest::widgetName(this, "kMyMoneyRegisterInvestment"));
  m_register->setParent(this);

  // m_register->setAction(QCString(MyMoneySplit::ActionAddShares), i18n("Add Shares"));
  // m_register->setAction(QCString(MyMoneySplit::ActionRemoveShares), i18n("Remove Shares"));
  //m_register->setAction(QCString(MyMoneySplit::ActionDeposit), i18n("Deposit"));
  //m_register->setAction(QCString(MyMoneySplit::ActionWithdrawal), i18n("Withdrawal"));
  // m_register->setAction(QCString(MyMoneySplit::ActionTransfer), i18n("Transfer"));

  connect(m_register, SIGNAL(clicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterClicked(int, int, int, const QPoint&)));
  connect(m_register, SIGNAL(doubleClicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterDoubleClicked(int, int, int, const QPoint&)));

  connect(m_register, SIGNAL(signalEnter()), this, SLOT(slotStartEdit()));
  connect(m_register, SIGNAL(signalNextTransaction()), this, SLOT(slotNextTransaction()));
  connect(m_register, SIGNAL(signalPreviousTransaction()), this, SLOT(slotPreviousTransaction()));
  connect(m_register, SIGNAL(signalDelete()), this, SLOT(slotDeleteTransaction()));
  connect(m_register, SIGNAL(signalSelectTransaction(const QCString&)), this, SLOT(selectTransaction(const QCString&)));

  connect(m_register->horizontalHeader(), SIGNAL(clicked(int)), this, SLOT(slotRegisterHeaderClicked(int)));
}

void KLedgerViewInvestments::createSummary(void)
{
  m_summaryLayout = new QHBoxLayout(6, "SummaryLayout");

  QSpacerItem* spacer = new QSpacerItem( 20, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );
  m_summaryLayout->addItem(spacer);

  m_summaryLine = new QLabel(this);

  m_summaryLayout->addWidget(m_summaryLine);
}

void KLedgerViewInvestments::slotAccountDetail(void)
{
  KNewAccountDlg dlg(m_account, true, false, this, "hi", i18n("Edit an Account"));

  if (dlg.exec()) {
    try {
      MyMoneyFile::instance()->modifyAccount(dlg.account());
    } catch (MyMoneyException *e) {
      qDebug("Unexpected exception in KLedgerViewCheckings::slotAccountDetail");
      delete e;
    }
  }
}

const KLedgerView::investTransactionTypeE KLedgerViewInvestments::transactionType(const MyMoneyTransaction& t, const MyMoneySplit& split) const
{
  if(split.action() == MyMoneySplit::ActionAddShares) {
    if(split.shares() >= 0)
      return AddShares;
    return RemoveShares;
  }
  else if(split.action() == MyMoneySplit::ActionBuyShares) {
    if(split.value() >= 0)
      return BuyShares;
    return SellShares;
  }
  else if(split.action() == MyMoneySplit::ActionDividend) {
    return Dividend;
  }
  else if(split.action() == MyMoneySplit::ActionReinvestDividend) {
    return ReinvestDividend;
  }
  else if(split.action() == MyMoneySplit::ActionYield) {
    return Yield;
  }
  return BuyShares;
}

void KLedgerViewInvestments::updateTabBar(const MyMoneyTransaction& /* t */, const MyMoneySplit& /* s */)
{
}

void KLedgerViewInvestments::resizeEvent(QResizeEvent* /* ev */)
{
  // resize the register
  int w = m_register->visibleWidth();

  // check which space we need
  m_register->adjustColumn(0);
  m_register->adjustColumn(4);
  m_register->adjustColumn(5);
  m_register->adjustColumn(6);

  // make amount columns all the same size
  int width = m_register->columnWidth(4);
  int width1 = m_register->columnWidth(5);
  int width2 = m_register->columnWidth(6);

  if(width < width1)
    width = width1;
  if(width < width2)
    width = width2;

  // Resize the date and money fields to either
  // a) the size required by the input widget if no transaction form is shown
  // b) the adjusted value for the input widget if the transaction form is visible
  if(!m_transactionFormActive) {
    kMyMoneyDateInput* datefield = new kMyMoneyDateInput();
    datefield->setFont(m_register->cellFont());
    m_register->setColumnWidth(1, datefield->minimumSizeHint().width());
    delete datefield;
    kMyMoneyEdit* valfield = new kMyMoneyEdit();
    valfield->setMinimumWidth(width);
    width = valfield->minimumSizeHint().width();
    delete valfield;
  } else {
    m_register->adjustColumn(1);
  }

  m_register->setColumnWidth(4, width);
  m_register->setColumnWidth(5, width);
  m_register->setColumnWidth(6, width);

  m_register->setColumnWidth(3, 20);

  for(int i = 0; i < m_register->numCols(); ++i) {
    switch(i) {
      default:
        w -= m_register->columnWidth(i);
        break;
      case 2:     // skip the one, we want to set
        break;
    }
  }
  m_register->setColumnWidth(2, w);

  // now resize the form
  kMyMoneyDateInput dateInput(0, "editDate");
  KPushButton splitButton(i18n("Split"), 0, "editSplit");

  kMyMoneyTransactionFormTable* table = static_cast<kMyMoneyTransactionFormTable *>(m_form->table());
  table->adjustColumn(0);
  table->setColumnWidth(2, splitButton.sizeHint().width());
  table->adjustColumn(3);
  table->adjustColumn(4, dateInput.minimumSizeHint().width()+10);

  w = table->visibleWidth();
  for(int i = 0; i < table->numCols(); ++i) {
    switch(i) {
      default:
        w -= table->columnWidth(i);
        break;
      case 1:     // skip the one, we want to set
        break;
    }
  }
  table->setColumnWidth(1, w);
}

void KLedgerViewInvestments::slotEndEdit()
{
  // check if someone tries to trick us. this works as follows:
  //  * set all conditions, so that the ENTER button is enabled
  //  * modify one condition and don't leave the field (e.g. set a value to 0)
  //  * press the enabled ENTER button
  // so if the check fails here, we better get out
  if(slotDataChanged() == false)
    return;

  MyMoneyFile* file = MyMoneyFile::instance();

  // make sure, the post date is valid
  if(!m_transaction.postDate().isValid())
    m_transaction.setPostDate(QDate::currentDate());

  // remember date for next new transaction
  m_lastPostDate = m_transaction.postDate();

  // grab the source account id for this transaction
  QCString accountId = m_editCashAccount->selectedAccounts().first();
  qDebug("Source account id = %s", accountId.data());

  //determine the transaction type
  int currentAction = m_editType->currentItem();
  qDebug("Current action is %d", currentAction);

  // so far, so good. Now we need to remove all splits from
  // the transaction because we will generate new ones.
  m_transaction.removeSplits();

  MyMoneyMoney total, fees, interest, shares, value;
  switch(currentAction) {
    case BuyShares:
    case SellShares:
      // setup stock account split
      shares = m_editShares->getMoneyValue();
      m_split.setAction(MyMoneySplit::ActionBuyShares);
      if(currentAction == SellShares) {
        shares = -shares;
      }
      m_split.setValue((m_editPPS->getMoneyValue() * shares));
      m_split.setShares(shares);
      m_split.setMemo(m_editMemo->text());
      m_split.setAccountId(m_editStockAccount->selectedAccounts().first());
      m_split.setId(QCString());

      //set up the fee split now
      fees = m_editFees->getMoneyValue();
      m_feeSplit.setValue(fees);
      m_feeSplit.setShares(fees);
      m_feeSplit.setAccountId(m_editFeeCategory->selectedAccountId());
      m_feeSplit.setId(QCString());

      //set up the split for the money that is sourcing this transaction
      total = -(shares*m_editPPS->getMoneyValue() + fees);
      m_accountSplit.setAccountId(accountId);
      m_accountSplit.setValue(total);
      m_accountSplit.setShares(total);
      m_accountSplit.setMemo(m_editMemo->text());
      m_accountSplit.setId(QCString());

      m_transaction.addSplit(m_accountSplit);
      m_transaction.addSplit(m_split);
      if(m_feeSplit.value() != 0)
        m_transaction.addSplit(m_feeSplit);
      break;

    case ReinvestDividend:
      // setup stock account split
      shares = m_editShares->getMoneyValue();
      m_split.setAction(MyMoneySplit::ActionReinvestDividend);
      m_split.setValue((m_editPPS->getMoneyValue() * shares));
      m_split.setShares(shares);
      m_split.setMemo(m_editMemo->text());
      m_split.setAccountId(m_editStockAccount->selectedAccounts().first());
      m_split.setId(QCString());

      //set up the fee split now
      fees = m_editFees->getMoneyValue();
      m_feeSplit.setValue(fees);
      m_feeSplit.setShares(fees);
      m_feeSplit.setAccountId(m_editFeeCategory->selectedAccountId());
      m_feeSplit.setId(QCString());

      //set up the split for the money that is sourcing this transaction
      total = -(shares*m_editPPS->getMoneyValue() + fees);
      m_interestSplit.setAccountId(accountId);
      m_interestSplit.setValue(total);
      m_interestSplit.setShares(total);
      m_interestSplit.setMemo(m_editMemo->text());
      m_interestSplit.setId(QCString());

      m_transaction.addSplit(m_interestSplit);
      m_transaction.addSplit(m_split);
      if(m_feeSplit.value() != 0)
        m_transaction.addSplit(m_feeSplit);
      break;

    case Dividend:
    case Yield:
      // setup stock account split. As each investment transaction
      // must reference a stock account, we better add it, even though
      // it does not seem to be necessary here otherwise.
      m_split.setAction(MyMoneySplit::ActionDividend);
      if(currentAction == Yield)
        m_split.setAction(MyMoneySplit::ActionYield);
      m_split.setValue(0);
      m_split.setShares(0);
      m_split.setMemo(m_editMemo->text());
      m_split.setAccountId(m_editStockAccount->selectedAccounts().first());
      m_split.setId(QCString());

      //set up the interest split now (attention: uses fee widgets)
      interest = -m_editFees->getMoneyValue();
      m_interestSplit.setValue(interest);
      m_interestSplit.setShares(interest);
      m_interestSplit.setAccountId(m_editFeeCategory->selectedAccountId());
      m_interestSplit.setId(QCString());

      //set up the split for the account that receives the payment
      m_accountSplit.setAccountId(accountId);
      m_accountSplit.setValue(-interest);
      m_accountSplit.setShares(-interest);
      m_accountSplit.setMemo(m_editMemo->text());
      m_accountSplit.setId(QCString());

      m_transaction.addSplit(m_accountSplit);
      m_transaction.addSplit(m_split);
      if(m_interestSplit.value() != 0)
        m_transaction.addSplit(m_interestSplit);
      break;

    case AddShares:
    case RemoveShares:
      shares = m_editShares->getMoneyValue();
      m_split.setAction(MyMoneySplit::ActionAddShares);
      if(currentAction == RemoveShares) {
        shares = -shares;
      }
      // setup stock account split
      m_split.setValue(0);
      m_split.setShares(shares);
      m_split.setMemo(m_editMemo->text());
      m_split.setAccountId(m_editStockAccount->selectedAccounts().first());
      m_split.setId(QCString());

      m_transaction.addSplit(m_split);
      break;
  }

  // switch the context to enable refreshView() to work
  m_form->newButton()->setEnabled(true);
  m_form->enterButton()->setEnabled(false);
  m_form->cancelButton()->setEnabled(false);
  m_form->moreButton()->setEnabled(false);

  if(transaction(m_register->currentTransactionIndex()) != 0) {
    m_form->editButton()->setEnabled(true);
    m_form->moreButton()->setEnabled(true);
  }

  hideWidgets();

  MyMoneyTransaction t;

  // so, we now have to save something here.
  // if an existing transaction has been changed, we take it as the base
  if(m_transactionPtr != 0) {
    t = *m_transactionPtr;
  }

  if(!(t == m_transaction)) {
    try {
      // differentiate between add and modify
      QCString id;
      if(m_transactionPtr == 0) {
        // in the add case, we don't have an ID yet. So let's get one
        // and use it down the line

        // From here on, we need to use a local copy of the transaction
        // because m_transaction will be reassigned during the update
        // once the transaction has been entered into the engine. If this
        // happens, we have no idea about the id of the new transaction.
        MyMoneyTransaction t = m_transaction;
        file->addTransaction(t);
        qDebug("Added the transaction to the file");
        id = t.id();

      } else {
        // in the modify case, we have to keep the id. The call to
        // modifyTransaction might change m_transaction due to some
        // callbacks.
        id = m_transaction.id();
        file->modifyTransaction(m_transaction);
        qDebug("Modified the transaction");
      }

      // make sure the transaction stays selected. It's position might
      // have changed within the register (e.g. date changed)
      selectTransaction(id);

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to add/modify transaction"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
    }
  }

 // connect(m_register, SIGNAL(signalEnter()), this, SLOT(slotStartEdit()));
 // m_register->setInlineEditingMode(false);
//  m_register->setFocus();

}

void KLedgerViewInvestments::slotEquityChanged(const QCString& id)
{
  MyMoneyTransaction t;
  MyMoneySplit s;

  t = m_transaction;
  s = m_split;
/*
  try {
      m_editSymbolName->loadEquity(id);
  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify equity"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_transaction = t;
    m_split = s;
  }
*/
}

void KLedgerViewInvestments::refreshView(const bool transactionFormVisible)
{
  // if we're currently editing a transaction, we don't refresh the view
  // this will screw us, if someone creates a category on the fly, as this
  // will come here when the notifications by the engine are send out.
  if(isEditMode())
    return;

  m_transactionFormActive = transactionFormVisible;

  // if a transaction is currently selected, keep the id
  QCString transactionId;
  if(m_transactionPtr != 0)
    transactionId = m_transactionPtr->id();
  m_transactionPtr = 0;
  m_transactionPtrVector.clear();

  // read in the configuration parameters for this view
  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  m_ledgerLens = config->readBoolEntry("LedgerLens", true);

  config->setGroup("List Options");
  QDateTime defaultDate;
  m_dateStart = config->readDateTimeEntry("StartDate", &defaultDate).date();

  m_register->readConfig();

  // in case someone changed the account info and we are called here
  // via the observer's update function, we just reload ourselves.
  MyMoneyFile* file = MyMoneyFile::instance();
  try {
    m_account = file->account(m_account.id());
    // setup the filter to select the transactions we want to display
    MyMoneyTransactionFilter filter;
    filter.addAccount(m_account.accountList());

    if(m_inReconciliation == true) {
      filter.addState(MyMoneyTransactionFilter::notReconciled);
      filter.addState(MyMoneyTransactionFilter::cleared);
    } else
      filter.setDateFilter(m_dateStart, QDate());

    // get the list of transactions
    QValueList<MyMoneyTransaction> list = file->transactionList(filter);
    QValueList<MyMoneyTransaction>::ConstIterator it;
    m_transactionList.clear();
    for(it = list.begin(); it != list.end(); ++it) {
      KMyMoneyTransaction k(*it);
      k.setSplitId((*it).splits()[0].id());
      m_transactionList.append(k);
    }

  } catch(MyMoneyException *e) {
    delete e;
    m_account = MyMoneyAccount();
    m_transactionList.clear();
  }

  updateView(transactionId);
}

void KLedgerViewInvestments::updateTotalAmount(void)
{
  QTable* formTable = m_form->table();
  kMyMoneyTransactionFormTableItem* item = 0;
  MyMoneyMoney total;
  switch(m_editType->currentItem()) {
    case BuyShares:
    case SellShares:
      total = m_editFees->getMoneyValue();
      if(m_editType->currentItem() == SellShares)
        total = -total;
      total = total + (m_editPPS->getMoneyValue() * m_editShares->getMoneyValue());
      item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                total.formatMoney("", 2));
      break;

    case ReinvestDividend:
      total = (m_editPPS->getMoneyValue() * m_editShares->getMoneyValue());
      total += m_editFees->getMoneyValue();
      item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                total.formatMoney("", 2));
      break;

    case Dividend:
    case Yield:
    case AddShares:
    case RemoveShares:
      // no amount field available
      item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, "");
          break;
  }
  if(item != 0) {
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    formTable->setItem(VALUE_ROW, VALUE_DATA_COL, item);
  }
}


const bool KLedgerViewInvestments::slotDataChanged(void)
{
  Q_CHECK_PTR(m_editType);

  // check if a change in widget presentation is required
  if(m_transactionType != m_editType->currentItem()) {
    m_transactionType = static_cast<investTransactionTypeE> (m_editType->currentItem());
    fillFormStatics();
    showWidgets();
  }

  // we don't allow to enter a fee until there's a fee category entered
  if(m_editFees && m_editFeeCategory) {
    m_editFees->setEnabled(!m_editFeeCategory->selectedAccountId().isEmpty());
  }

  // check if we can enable the ENTER buttons. the condition heavily
  // depends on the current action
  bool ok = true;
  if(m_editStockAccount->selectedAccounts().first().isEmpty()) {
    ok = false;
  }

  switch(m_editType->currentItem()) {
    case BuyShares:
    case SellShares:
      if(m_editCashAccount->selectedAccounts().first().isEmpty()) {
        ok = false;
      }
      if(m_editPPS->getMoneyValue() == 0) {
        ok = false;
      }
      if(m_editShares->getMoneyValue() == 0) {
        ok = false;
      }
      break;

    case ReinvestDividend:
      if(m_editCashAccount->selectedAccounts().first().isEmpty()
      || m_editPPS->getMoneyValue() == 0
      || m_editShares->getMoneyValue() == 0)
        ok = false;
      if(!m_editFeeCategory->selectedAccountId().isEmpty()
      && m_editFees->getMoneyValue() == 0)
        ok = false;
      break;

    case Dividend:
    case Yield:
      if(m_editCashAccount->selectedAccounts().first().isEmpty()
      || m_editFeeCategory->selectedAccountId().isEmpty()
      || m_editFees->getMoneyValue() == 0)
        ok = false;
      break;

    case AddShares:
    case RemoveShares:
      if(m_editShares->getMoneyValue() == 0)
        ok = false;
      break;
  }
  m_form->enterButton()->setEnabled(ok);

  updateTotalAmount();

  return m_form->enterButton()->isEnabled();
}
