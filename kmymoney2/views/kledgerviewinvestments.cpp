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
#include <kpopupmenu.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerview.h"
#include "kledgerviewinvestments.h"

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/storage/imymoneystorage.h"

#include "../dialogs/knewaccountdlg.h"
#include "../dialogs/knewequityentrydlg.h"
#include "../dialogs/kcurrencycalculator.h"

#include "../widgets/kmymoneyregisterinvestment.h"
#include "../widgets/kmymoneytransactionform.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneyequity.h"

#include "../kmymoneysettings.h"

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
  KLedgerView(parent, name),
  m_editMapper(parent, "Mapper")
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

  // create the context menu that is accessible via RMB
  createContextMenu();

  // create the context menus that are accessible via transaction form buttons
  createMoreMenu();
  createAccountMenu();

  // If the context menus ever implement some dynamic behaviour, here's
  // where to connect the signals to the slots
  // connect(m_contextMenu, SIGNAL(aboutToShow()), SLOT(slotConfigureContextMenu()));
  // connect(m_moreMenu, SIGNAL(aboutToShow()), SLOT(slotConfigureMoreMenu()));

  // setup the form to be visible or not
  slotShowTransactionForm(KMyMoneySettings::transactionForm());

  // and the register has the focus
  m_register->setFocus();

  connect(&m_editMapper, SIGNAL(mapped(int)), this, SLOT(slotDataChanged(int)));
}

KLedgerViewInvestments::~KLedgerViewInvestments()
{

}

void KLedgerViewInvestments::createAccountMenu(void)
{
  // get the basic entries for all ledger views
  KLedgerView::createAccountMenu();

  m_form->accountButton()->setPopup(m_accountMenu);
}

void KLedgerViewInvestments::createMoreMenu(void)
{
  // get the basic entries for all ledger views
  KLedgerView::createMoreMenu();
#if 0
  // and now the specific entries for checkings/savings etc.
  KIconLoader *kiconloader = KGlobal::iconLoader();
  m_moreMenu->insertItem(i18n("Edit splits ..."), this, SLOT(slotStartEditSplit()),
      QKeySequence(), -1, 1);
  m_moreMenu->insertItem(kiconloader->loadIcon("goto", KIcon::Small),
                         i18n("Goto payee/receiver"), this, SLOT(slotPayeeSelected()),
                         QKeySequence(), -1, 2);
  m_moreMenu->insertItem(kiconloader->loadIcon("bookmark_add", KIcon::Small),
                         i18n("Create schedule..."), this, SLOT(slotCreateSchedule()),
                         QKeySequence(), -1, 3);
#endif

  m_form->moreButton()->setPopup(m_moreMenu);
}

void KLedgerViewInvestments::createContextMenu(void)
{
  // get the basic entries for all ledger views
  KLedgerView::createContextMenu();

#if 0
  // and now the specific entries for checkings/savings etc.
  KIconLoader *kiconloader = KGlobal::iconLoader();

  m_contextMenu->insertItem(i18n("Edit splits ..."), this, SLOT(slotStartEditSplit()),
      QKeySequence(), -1, 2);
  m_contextMenu->insertItem(kiconloader->loadIcon("goto", KIcon::Small),
                            i18n("Goto payee/receiver"), this, SLOT(slotPayeeSelected()),
                            QKeySequence(), -1, 3);
  m_contextMenu->insertItem(kiconloader->loadIcon("bookmark_add", KIcon::Small),
                            i18n("Create schedule..."), this, SLOT(slotCreateSchedule()),
                            QKeySequence(), -1, 4);
#endif
}

int KLedgerViewInvestments::actionTab(const MyMoneyTransaction& t, const MyMoneySplit& split) const
{
  if(KMyMoneyUtils::transactionType(t) == KMyMoneyUtils::Transfer) {
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
  m_transactionPtr = KLedgerView::transaction(m_register->currentTransactionIndex());

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

    // fill in common fields
    MyMoneyAccount acc = file->account(m_split.accountId());
    MyMoneySecurity security = file->security(acc.currencyId());
    formTable->setText(SYMBOL_ROW, SYMBOL_DATA_COL, security.tradingSymbol());
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
    int prec;
    KConfig *kconfig = KGlobal::config();
    kconfig->setGroup("General Options");
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
//        amount = m_accountSplit.value(m_transaction.commodity(), m_account.currencyId()).abs();
        amount = m_accountSplit.value().abs();
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 amount.formatMoney());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(VALUE_ROW, VALUE_DATA_COL, item);

        // shares
        shares = m_split.shares().abs();
        prec = MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction());
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 shares.formatMoney("", prec));
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(QUANTITY_ROW, QUANTITY_DATA_COL, item);

        // price
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
//                 (m_split.value(m_transaction.commodity(), m_account.currencyId())/m_split.shares()).formatMoney());
                 (m_split.value()/m_split.shares()).formatMoney("", kconfig->readNumEntry("PricePrecision", 4)));
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
//        amount = m_interestSplit.value(m_transaction.commodity(), m_account.currencyId()).abs();
        amount = m_interestSplit.value().abs();
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 amount.formatMoney());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(VALUE_ROW, VALUE_DATA_COL, item);

        // shares
        shares = m_split.shares().abs();
        prec = MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction());
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 shares.formatMoney("", prec));
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(QUANTITY_ROW, QUANTITY_DATA_COL, item);

        // price
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
//                 (m_split.value(m_transaction.commodity(), m_account.currencyId())/m_split.shares()).formatMoney());
                 (m_split.value()/m_split.shares()).formatMoney("", kconfig->readNumEntry("PricePrecision", 4)));
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(PRICE_ROW, PRICE_DATA_COL, item);

        // fees
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
//                 m_feeSplit.value(m_transaction.commodity(), m_account.currencyId()).abs().formatMoney());
                 m_feeSplit.value().abs().formatMoney());
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
//        amount = m_accountSplit.value(m_transaction.commodity(), m_account.currencyId()).abs();
        amount = m_accountSplit.value().abs();
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
        prec = MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction());
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 m_split.shares().abs().formatMoney("", prec));
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(QUANTITY_ROW, QUANTITY_DATA_COL, item);

        break;

      case SplitShares:
        formTable->setText(ACTIVITY_ROW, ACTIVITY_DATA_COL, i18n("Split Shares"));

        // shares
        prec = MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction());
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 m_split.shares().abs().formatMoney("", prec));
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(QUANTITY_ROW, QUANTITY_DATA_COL, item);

        break;

      case UnknownTransactionType:
        qWarning("%s","Unknown transaction type!");
        break;
    }

    m_form->newButton()->setEnabled(true);
    m_form->editButton()->setEnabled(true);
    enableOkButton(false);
    enableCancelButton(false);
    enableMoreButton(true);

  } else {
    m_transaction = MyMoneyTransaction();
    m_split = MyMoneySplit();
    m_accountSplit = MyMoneySplit();
    m_feeSplit = MyMoneySplit();
    m_interestSplit = MyMoneySplit();

    fillFormStatics();

    m_form->newButton()->setEnabled(true);
    m_form->editButton()->setEnabled(false);
    enableOkButton(false);
    enableCancelButton(false);
    enableMoreButton(false);
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

    case SplitShares:
      formTable->setText(QUANTITY_ROW, QUANTITY_TXT_COL, i18n("Split Ratio"));
      break;

    case UnknownTransactionType:
      qWarning("%s","Unknown transaction type!");
      break;
  }
}

void KLedgerViewInvestments::fillSummary()
{
  // TODO: write out the summary line (whatever it will contain)
}

QWidget* KLedgerViewInvestments::arrangeEditWidgetsInRegister(void)
{
  int firstRow = m_register->currentTransactionIndex() * m_register->rpt();

  // arrange common widgets
  setRegisterCellWidget(firstRow, 0, m_editDate);
  setRegisterCellWidget(firstRow, 1, m_editStockAccount);
  setRegisterCellWidget(firstRow, 2, m_editType);
  setRegisterCellWidget(firstRow+3, 2, m_editMemo);

  // arrange variable widgets
  setRegisterCellWidget(firstRow, 4, m_editShares);
  setRegisterCellWidget(firstRow, 5, m_editPPS);
  setRegisterCellWidget(firstRow+1, 4, m_editFees);
  setRegisterCellWidget(firstRow+1, 2, m_editFeeCategory);
  setRegisterCellWidget(firstRow+2, 2, m_editCashAccount);
  setRegisterCellWidget(firstRow, 6, m_editAmount);

  // place buttons
  setRegisterCellWidget(firstRow+3, 0, m_registerButtonFrame);

  // add field hints
  //m_editCashAccount->setHint("Account");
  m_editMemo->setHint(i18n("Memo"));
  m_editFeeCategory->setHint(i18n("Fee Category"));

  // show all variable widgets, we hide the ones we
  // don't need for the current case later on again
  m_editPPS->show();
  m_editShares->show();
  m_editFees->show();
  m_editCashAccount->show();
  m_editFeeCategory->show();
  m_editAmount->show();

  switch(m_editType->currentItem()) {
    case ReinvestDividend:
      m_editCashAccount->loadList(static_cast<KMyMoneyUtils::categoryTypeE>(KMyMoneyUtils::income));
      // tricky fall through here

    case BuyShares:
    case SellShares:
      break;

    case Dividend:
    case Yield:
      m_editShares->hide();
      m_editPPS->hide();
      m_editAmount->hide();
      break;

    case AddShares:
    case RemoveShares:
    case SplitShares:
      m_editPPS->hide();
      m_editFees->hide();
      m_editFeeCategory->hide();
      m_editCashAccount->hide();
      m_editAmount->hide();
      break;
  }

  // now setup the tab order
  clearTabOrder();
  addToTabOrder(m_editDate);
  addToTabOrder(m_editStockAccount);
  addToTabOrder(m_editType);
  addToTabOrder(m_editFeeCategory);
  addToTabOrder(m_editCashAccount);
  addToTabOrder(m_editMemo);
  addToTabOrder(m_editShares);
  addToTabOrder(m_editFees);
  addToTabOrder(m_editPPS);
  addToTabOrder(m_editAmount);
  addToTabOrder(m_registerEnterButton);
  addToTabOrder(m_registerCancelButton);
  addToTabOrder(m_registerMoreButton);

  return m_editDate;
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
  m_editAmount->setPalette(palette);

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
  setFormCellWidget(VALUE_ROW, VALUE_DATA_COL, m_editAmount);

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
  m_editAmount->show();

  switch(m_editType->currentItem()) {
    case ReinvestDividend:
      // tricky fall through here

    case BuyShares:
    case SellShares:
      table->setEditable(QUANTITY_ROW, QUANTITY_DATA_COL);
      table->setEditable(PRICE_ROW, PRICE_DATA_COL);
      table->setEditable(VALUE_ROW, VALUE_DATA_COL, false);
      table->setEditable(FEES_ROW, FEES_DATA_COL);
      table->setEditable(CATEGORY_ROW, CATEGORY_DATA_COL);
      table->setEditable(ACCOUNT_ROW, ACCOUNT_DATA_COL);
      table->setEditable(VALUE_ROW, VALUE_DATA_COL);
      break;

    case Dividend:
    case Yield:
      m_editShares->hide();
      m_editPPS->hide();
      m_editAmount->hide();

      table->setEditable(CATEGORY_ROW, CATEGORY_DATA_COL);
      table->setEditable(ACCOUNT_ROW, ACCOUNT_DATA_COL);
      break;

    case AddShares:
    case RemoveShares:
    case SplitShares:
      m_editPPS->hide();
      m_editFees->hide();
      m_editFeeCategory->hide();
      m_editCashAccount->hide();
      m_editAmount->hide();

      table->setEditable(QUANTITY_ROW, QUANTITY_DATA_COL);
      break;
  }

  // now setup the tab order
  clearTabOrder();
  addToTabOrder(m_form->enterButton());
  addToTabOrder(m_form->enterButton());
  addToTabOrder(m_form->cancelButton());
  addToTabOrder(m_form->moreButton());

  addToTabOrder(m_editType);
  addToTabOrder(m_editStockAccount);
  addToTabOrder(m_editMemo);
  addToTabOrder(m_editFeeCategory);
  addToTabOrder(m_editCashAccount);

  addToTabOrder(m_editDate);
  addToTabOrder(m_editShares);
  addToTabOrder(m_editPPS);
  addToTabOrder(m_editFees);
  addToTabOrder(m_editAmount);

  return m_editStockAccount;
}

bool KLedgerViewInvestments::focusNextPrevChild(bool next)
{
  return KLedgerView::focusNextPrevChild(next);
}

void KLedgerViewInvestments::destroyWidgets()
{
  KLedgerView::destroyWidgets();

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
      m_security = MyMoneyFile::instance()->security(acc.currencyId());
      m_split = *it_s;
    } else if(acc.accountGroup() == MyMoneyAccount::Expense
           || acc.accountGroup() == MyMoneyAccount::Income) {

      // buy shares transactions have only one income/expense split which is a fee
      if(m_split.action() == MyMoneySplit::ActionBuyShares) {
        m_feeSplit = *it_s;

      // dividend/yield transactions also have only one income/expense split which is an interest
      } else if(m_split.action() == MyMoneySplit::ActionDividend) {
        m_interestSplit = *it_s;

      // leaves addshares which has no income/expense split so we
      // should never come around here anyway and reinvest dividend
      // transactions which can have two where the first one always
      // is the interest. Attention: older implementations of KMM did not
      // store the m_split as first split in a transaction but rather
      // the interest split. So m_split.action() might be empty in this
      // case.
      } else {
        if(m_interestSplit.id().isEmpty())
          m_interestSplit = *it_s;
        else
          m_feeSplit = *it_s;
      }

    } else if(acc.accountGroup() == MyMoneyAccount::Asset
            || acc.accountGroup() == MyMoneyAccount::Liability) {
      m_accountSplit = *it_s;
    }
  }
}

void KLedgerViewInvestments::preloadEditType(void)
{
  Q_CHECK_PTR(static_cast<void *>(m_editType));

  // Determine the actual action
  if(m_split.action() == MyMoneySplit::ActionBuyShares) {
    if(m_split.value().isNegative())
      m_editType->setCurrentItem(SellShares);
    else
      m_editType->setCurrentItem(BuyShares);

  } else if(m_split.action() == MyMoneySplit::ActionAddShares) {
    if(m_split.value().isNegative())
      m_editType->setCurrentItem(RemoveShares);
    else
      m_editType->setCurrentItem(AddShares);

  } else if(m_split.action() == MyMoneySplit::ActionSplitShares) {
    m_editType->setCurrentItem(SplitShares);

  } else if(m_split.action() == MyMoneySplit::ActionDividend) {
      m_editType->setCurrentItem(Dividend);

  } else if(m_split.action() == MyMoneySplit::ActionYield) {
      m_editType->setCurrentItem(Yield);

  } else if(m_split.action() == MyMoneySplit::ActionReinvestDividend) {
      m_editType->setCurrentItem(ReinvestDividend);

  } else {
    qDebug("Unknown MyMoneySplit::Action%s in KLedgerViewInvestments::preloadEditType", m_split.action().data());
    return;
  }
}

void KLedgerViewInvestments::loadEditWidgets(void)
{
  if(m_transactionPtr != 0) {
    m_transaction = *m_transactionPtr;
    preloadInvestmentSplits(m_transaction);
    preloadEditType();
    reloadEditWidgets(m_transaction);
  } else {
    m_editDate->setDate(m_lastPostDate);
  }

  // the next line prevents an endless loop caused in
  // slotDataChanged()
  m_transactionType = static_cast<investTransactionTypeE> (m_editType->currentItem());
  slotDataChanged(None);
}

void KLedgerViewInvestments::reloadEditWidgets(const MyMoneyTransaction& /*t*/ )
{
  Q_CHECK_PTR(static_cast<void *>(m_editType));

  // Fill the fields - first the ones that are in any transaction
  if(m_editStockAccount)
    m_editStockAccount->setSelected(m_split.accountId());
  if(m_editDate)
    m_editDate->setDate(m_transaction.postDate());
  if(m_editMemo)
    m_editMemo->loadText(m_split.memo());

  // Fill the fields - next the ones that are specific to the transaction
  MyMoneyMoney shares, price, amount;
  int prec;
  switch(m_editType->currentItem()) {
    case BuyShares:
    case SellShares:
      if(m_editAmount)
        m_editAmount->setValue(m_split.value().abs());
      shares = m_split.shares().abs();
      if(shares.isPositive()) {
        price = m_split.value().abs() / shares;
        if(m_editShares) {
          prec = MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction());
          m_editShares->setPrecision(prec);
          m_editShares->setValue(shares.abs());
        }
      }
      if(m_editPPS) {
        m_editPPS->setValue(price.abs());
      }
      if(m_editCashAccount)
        m_editCashAccount->setSelected(m_accountSplit.accountId());
      if(m_editFeeCategory) {
        m_editFeeCategory->loadAccount(m_feeSplit.accountId());
      }
      if(!m_feeSplit.accountId().isEmpty())
        m_editFees->setValue(m_feeSplit.value().abs());
      break;

    case ReinvestDividend:
      if(m_editAmount)
        m_editAmount->setValue(m_split.value().abs());
      shares = m_split.shares().abs();
      if(shares.isPositive()) {
        price = m_split.value().abs() / shares;
        if(m_editShares) {
          prec = MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction());
          m_editShares->setPrecision(prec);
          m_editShares->setValue(shares.abs());
        }
      }
      if(m_editPPS)
        m_editPPS->setValue(price.abs());
      if(m_editCashAccount) {
        // the cash account field is actually used for the interest income
        // in the case of ReinvestDividend. So we have to reload the widget
        // with the right list before we select the current account.
        m_editCashAccount->loadList(static_cast<KMyMoneyUtils::categoryTypeE>(KMyMoneyUtils::income));
        m_editCashAccount->setSelected(m_interestSplit.accountId());
      }
      if(m_editFeeCategory)
        m_editFeeCategory->loadAccount(m_feeSplit.accountId());
      if(!m_feeSplit.accountId().isEmpty())
        m_editFees->setValue(m_feeSplit.value().abs());
      break;

    case Dividend:
    case Yield:
      if(m_editFeeCategory)
        m_editFeeCategory->loadAccount(m_interestSplit.accountId());
      if(!m_interestSplit.accountId().isEmpty())
        m_editFees->setValue((-m_interestSplit.value()).abs());
      if(m_editCashAccount)
        m_editCashAccount->setSelected(m_accountSplit.accountId());
      break;

    case AddShares:
    case RemoveShares:
    case SplitShares:
      shares = m_split.shares().abs();
      if(m_editShares) {
        prec = MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction());
        m_editShares->setPrecision(prec);
        m_editShares->setValue(shares.abs());
      }
      break;
  }
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
  if(!m_editDate) {
    QTable* formTable = m_form->table();

    // clear columns containing edit widgets (1 and 4)
    for(int c = 1; c < formTable->numCols(); ++c) {
      for(int r = 0; r < formTable->numRows(); ++r) {
        formTable->setText(r, c, " ");
      }
      if(c == 1)
        c += 2;
    }

    m_editDate = new kMyMoneyDateInput(0, "editDate");
    connect(m_editDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged(const QDate&)));
  }
  if(!m_editMemo) {
    m_editMemo = new kMyMoneyLineEdit(0, "editMemo", AlignLeft|AlignVCenter);
    m_editMapper.setMapping(m_editMemo, None);
    connect(m_editMemo, SIGNAL(lineChanged(const QString&)), &m_editMapper, SLOT(map()));
  }
  if(!m_editShares) {
    m_editShares = new kMyMoneyEdit(0, "editShares");
    m_editMapper.setMapping(m_editShares, Shares);
    connect(m_editShares, SIGNAL(valueChanged(const QString& )), &m_editMapper, SLOT(map()));
  }
  if(!m_editPPS) {
    m_editPPS = new kMyMoneyEdit(0, "editPPS");
    KConfig *kconfig = KGlobal::config();
    kconfig->setGroup("General Options");
    int prec = kconfig->readNumEntry("PricePrecision", 4);
    m_editPPS->setPrecision(prec);
    m_editMapper.setMapping(m_editPPS, Price);
    connect(m_editPPS, SIGNAL(valueChanged(const QString& )), &m_editMapper, SLOT(map()));
  }
  if(!m_editAmount) {
    m_editAmount = new kMyMoneyEdit(0, "editAmount");
    m_editMapper.setMapping(m_editAmount, Total);
    connect(m_editAmount, SIGNAL(valueChanged(const QString& )), &m_editMapper, SLOT(map()));
  }
  if(!m_editStockAccount) {
    m_editStockAccount = new KMyMoneyAccountCombo(0, "editStockAccount");
    m_editStockAccount->setMinimumWidth(0);  // override the widgets default
    m_editStockAccount->loadList(i18n("Investments"), m_account.accountList());
    connect(m_editStockAccount, SIGNAL(accountSelected(const QCString&)), this, SLOT(slotSecurityChanged(const QCString&)));
  }
  if(!m_editFees) {
    m_editFees = new kMyMoneyEdit(0, "editFees");
    m_editMapper.setMapping(m_editFees, Fees);
    connect(m_editFees, SIGNAL(valueChanged(const QString& )), &m_editMapper, SLOT(map()));
  }

  if(!m_editType) {
    m_editType = new kMyMoneyCombo(0, "editType");
    // m_editType->setFocusPolicy(QWidget::StrongFocus);
    m_editType->insertItem(i18n("Buy Shares"), BuyShares);
    m_editType->insertItem(i18n("Sell Shares"), SellShares);
    m_editType->insertItem(i18n("Dividend"), Dividend);
    m_editType->insertItem(i18n("Reinvest Dividend"), ReinvestDividend);
    m_editType->insertItem(i18n("Yield"), Yield);
    m_editType->insertItem(i18n("Add Shares"), AddShares);
    m_editType->insertItem(i18n("Remove Shares"), RemoveShares);
    m_editType->insertItem(i18n("Split Shares"), SplitShares);
    m_editMapper.setMapping(m_editType, None);
    connect(m_editType, SIGNAL(activated(int)), &m_editMapper, SLOT(map()));
  }

  if(!m_editCashAccount) {
    m_editCashAccount = new KMyMoneyAccountCombo(0, "editCashAccount");
    m_editCashAccount->setMinimumWidth(0);
    // m_editCashAccount->setFocusPolicy(QWidget::StrongFocus);

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

    m_editMapper.setMapping(m_editCashAccount, CashAccount);
    connect(m_editCashAccount, SIGNAL(accountSelected(const QCString&)), &m_editMapper, SLOT(map()));
  }

  if(!m_editFeeCategory) {
    m_editFeeCategory = new kMyMoneyCategory(0, "editFeeCategory", KMyMoneyUtils::expense);
    m_editMapper.setMapping(m_editFeeCategory, FeeCategory);
    connect(m_editFeeCategory, SIGNAL(categoryChanged(const QCString&)), &m_editMapper, SLOT(map()));
    connect(m_editCategory, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));
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
  h = QMAX(h, category.sizeHint().height())-4;

  m_form = new kMyMoneyTransactionForm(this, NULL, 0, 5, 5, h);

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

void KLedgerViewInvestments::slotTypeSelected(int)
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
      formTable->setText(0, 3, i18n("No."));
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));
      break;     */
  }

  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  if(config->readBoolEntry("AlwaysShowNrField", false) == true)
    formTable->setText(0, 3, i18n("No."));
#endif

  if(!m_form->tabBar()->signalsBlocked())
    slotNew();
}

void KLedgerViewInvestments::slotRegisterDoubleClicked(int /* row */,
                                                int /* col */,
                                                int /* button */,
                                                const QPoint & /* mousePos */)
{
  if(static_cast<unsigned> (m_register->currentTransactionIndex()) == m_transactionList.count())
    slotNew();
  else
    slotStartEdit();
}

void KLedgerViewInvestments::createRegister(void)
{
  KLedgerView::createRegister(new kMyMoneyRegisterInvestment(this, "kMyMoneyRegisterInvestment"));
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

const KLedgerView::investTransactionTypeE KLedgerViewInvestments::transactionType(const MyMoneyTransaction& /*t*/, const MyMoneySplit& split) const
{
  if(split.action() == MyMoneySplit::ActionAddShares) {
    if(!split.shares().isNegative())
      return AddShares;
    return RemoveShares;
  }
  else if(split.action() == MyMoneySplit::ActionBuyShares) {
    if(!split.value().isNegative())
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
  else if(split.action() == MyMoneySplit::ActionSplitShares) {
    return SplitShares;
  }
  return BuyShares;
}

void KLedgerViewInvestments::updateTabBar(const MyMoneyTransaction& /* t */, const MyMoneySplit& /* s */, const bool /* enableAll */)
{
}

void KLedgerViewInvestments::resizeEvent(QResizeEvent* /* ev */)
{
  m_register->setUpdatesEnabled(false);

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
  if(!KMyMoneySettings::transactionForm()) {
    kMyMoneyDateInput* datefield = new kMyMoneyDateInput();
    datefield->setFont(m_register->cellFont());
    m_register->setColumnWidth(0, datefield->minimumSizeHint().width());
    delete datefield;
    kMyMoneyEdit* valfield = new kMyMoneyEdit();
    valfield->setMinimumWidth(width);
    width = valfield->minimumSizeHint().width();
    delete valfield;
  } else {
    m_register->adjustColumn(0);
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

  m_register->setUpdatesEnabled(true);
  m_register->repaintContents(false);

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

void KLedgerViewInvestments::createSplits(void)
{
  MyMoneyMoney total, fees, interest, shares, value;

  m_split = MyMoneySplit();
  m_accountSplit = MyMoneySplit();
  m_feeSplit = MyMoneySplit();
  m_interestSplit = MyMoneySplit();

  //determine the transaction type
  int currentAction = m_editType->currentItem();

  switch(currentAction) {
    case BuyShares:
    case SellShares:
      // setup stock account split
      shares = m_editShares->value().abs();
      total = m_editAmount->value().abs();
      if(!m_editFeeCategory->selectedAccountId().isEmpty())
        fees = m_editFees->value();
      m_split.setAction(MyMoneySplit::ActionBuyShares);
      if(currentAction == SellShares) {
        shares = -shares;
        total = -total;
      }
      value = total - fees;
      m_split.setValue(value);
      m_split.setShares(shares);
      m_split.setMemo(m_editMemo->text());
      m_split.setAccountId(m_editStockAccount->selectedAccounts().first());

      //set up the fee split now
      m_feeSplit.setValue(fees);
      m_feeSplit.setShares(fees);
      m_feeSplit.setAccountId(m_editFeeCategory->selectedAccountId());

      //set up the split for the money that is sourcing this transaction
      total = -total;
      m_accountSplit.setAccountId(m_editCashAccount->selectedAccounts().first());
      m_accountSplit.setValue(total);
      m_accountSplit.setShares(total);
      m_accountSplit.setMemo(m_editMemo->text());
      break;

    case ReinvestDividend:
      // setup stock account split
      shares = m_editShares->value().abs();
      total = m_editAmount->value().abs();
      if(!m_editFeeCategory->selectedAccountId().isEmpty())
        fees = m_editFees->value();
      value = total - fees;
      m_split.setAction(MyMoneySplit::ActionReinvestDividend);
      m_split.setValue(value);
      m_split.setShares(shares);
      m_split.setMemo(m_editMemo->text());
      m_split.setAccountId(m_editStockAccount->selectedAccounts().first());

      //set up the fee split now
      m_feeSplit.setValue(fees);
      m_feeSplit.setShares(fees);
      m_feeSplit.setAccountId(m_editFeeCategory->selectedAccountId());

      //set up the split for the money that is sourcing this transaction
      total = -total;
      m_interestSplit.setAccountId(m_editCashAccount->selectedAccounts().first());
      m_interestSplit.setValue(total);
      m_interestSplit.setShares(total);
      m_interestSplit.setMemo(m_editMemo->text());
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

      //set up the interest split now (attention: uses fee widgets)
      interest = -m_editFees->value();
      m_interestSplit.setValue(interest);
      m_interestSplit.setShares(interest);
      m_interestSplit.setAccountId(m_editFeeCategory->selectedAccountId());

      //set up the split for the account that receives the payment
      m_accountSplit.setAccountId(m_editCashAccount->selectedAccounts().first());
      m_accountSplit.setValue(-interest);
      m_accountSplit.setShares(-interest);
      m_accountSplit.setMemo(m_editMemo->text());
      break;

    case AddShares:
    case RemoveShares:
      shares = m_editShares->value().abs();
      m_split.setAction(MyMoneySplit::ActionAddShares);
      if(currentAction == RemoveShares) {
        shares = -shares;
      }
      // setup stock account split
      m_split.setValue(0);
      m_split.setShares(shares);
      m_split.setMemo(m_editMemo->text());
      m_split.setAccountId(m_editStockAccount->selectedAccounts().first());
      break;

    case SplitShares:
      shares = m_editShares->value().abs();
      m_split.setAction(MyMoneySplit::ActionSplitShares);
      // setup stock account split
      m_split.setValue(0);
      m_split.setShares(shares);
      m_split.setMemo(m_editMemo->text());
      m_split.setAccountId(m_editStockAccount->selectedAccounts().first());
      break;
  }
}

void KLedgerViewInvestments::slotEndEdit()
{
  if(!isEditMode())
    return;

  // check if someone tries to trick us. this works as follows:
  //  * set all conditions, so that the ENTER button is enabled
  //  * modify one condition and don't leave the field (e.g. set a value to 0)
  //  * press the enabled ENTER button
  // so if the check fails here, we better get out
  if(slotDataChanged(None) == false)
    return;

  // force focus change to update all data
  m_register->setFocus();

  MyMoneyFile* file = MyMoneyFile::instance();

  // make sure, the post date is valid
  if(!m_transaction.postDate().isValid())
    m_transaction.setPostDate(QDate::currentDate());

  // remember date for next new transaction
  m_lastPostDate = m_transaction.postDate();

  // grab the source account id for this transaction
  QCString accountId = m_editCashAccount->selectedAccounts().first();
  QCString stockId = m_editStockAccount->selectedAccounts().first();

  //determine the transaction type
  int currentAction = m_editType->currentItem();

  // so far, so good. Now we need to remove all splits from
  // the transaction because we will generate new ones.
  m_transaction.removeSplits();

  bool transactionContainsPriceInfo = false;

  MyMoneyAccount stockAccount = file->account(stockId);
  QCString securityId = stockAccount.currencyId();
  MyMoneySecurity security = file->security(securityId);

  m_transaction.setCommodity(security.tradingCurrency());

  createSplits();

  m_priceInfo.clear();
  switch(currentAction) {
    case BuyShares:
    case SellShares:
      // check for possible conversion rate and continue to edit
      // if user cancelled
      if(!setupPrice(m_accountSplit))
        return;

      m_transaction.addSplit(m_accountSplit);
      m_transaction.addSplit(m_split);
      if(!m_feeSplit.value().isZero()) {
        // check for possible conversion rate and continue to edit
        // if user cancelled
        if(!setupPrice(m_feeSplit))
          return;
        m_transaction.addSplit(m_feeSplit);
      }
      transactionContainsPriceInfo = true;
      break;

    case ReinvestDividend:
      // check for possible conversion rate and continue to edit
      // if user cancelled
      if(!setupPrice(m_interestSplit))
        return;

      // the stock account split should be the first one here
      // because we look at it's action() in preloadInvestmentSplits()
      m_transaction.addSplit(m_split);
      m_transaction.addSplit(m_interestSplit);
      if(!m_feeSplit.value().isZero()) {
        // check for possible conversion rate and continue to edit
        // if user cancelled
        if(!setupPrice(m_feeSplit))
          return;
        m_transaction.addSplit(m_feeSplit);
      }
      transactionContainsPriceInfo = true;
      break;

    case Dividend:
    case Yield:
      // check for possible conversion rate and continue to edit
      // if user cancelled
      if(!setupPrice(m_accountSplit))
        return;

      m_transaction.addSplit(m_accountSplit);
      m_transaction.addSplit(m_split);
      if(!m_interestSplit.value().isZero()) {
        // check for possible conversion rate and continue to edit
        // if user cancelled
        if(!setupPrice(m_interestSplit))
          return;
        m_transaction.addSplit(m_interestSplit);
      }
      break;

    case AddShares:
    case RemoveShares:
    case SplitShares:
      m_transaction.addSplit(m_split);
      break;
  }

  // switch the context to enable refreshView() to work
  m_form->newButton()->setEnabled(true);
  enableOkButton(false);
  enableCancelButton(false);
  enableMoreButton(false);

  if(KLedgerView::transaction(m_register->currentTransactionIndex()) != 0) {
    m_form->editButton()->setEnabled(true);
    enableMoreButton(true);
  }

  destroyWidgets();

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
        id = t.id();

      } else {
        // in the modify case, we have to keep the id. The call to
        // modifyTransaction might change m_transaction due to some
        // callbacks.
        id = m_transaction.id();
        file->modifyTransaction(m_transaction);
      }

      // make sure the transaction stays selected. It's position might
      // have changed within the register (e.g. date changed)
      selectTransaction(id);

      // check if we have new or updated price info
      if(transactionContainsPriceInfo == true && ! m_split.shares().isZero()) {
        MyMoneyMoney price = m_split.value() / m_split.shares();
        bool securityChanged = false;
// FIXME PRICE
#if 0
        if(m_equity.hasPrice(m_transaction.postDate(), true) == false) {
          if(KMessageBox::questionYesNo(this, QString("<p>")+i18n("The price history for <b>%1</b> does not contain an entry for <b>%2</b>.  Do you want to add a new entry in the history based on the price of this transaction?").arg(m_equity.name()).arg(KGlobal::locale()->formatDate(m_transaction.postDate(), true)), i18n("Add price info"), KStdGuiItem::yes(), KStdGuiItem::no(), "StoreNewPrice") == KMessageBox::Yes) {
            m_equity.addPriceHistory(m_transaction.postDate(), price);
            equityChanged = true;
          }
        } else if(price != m_equity.price(m_transaction.postDate())){
          if(KMessageBox::questionYesNo(this, QString("<p>")+i18n("The price history for <b>%1</b> contains a different price for <b>%2</b>.  Do you want to update the price in the history to the one of this transaction?").arg(m_equity.name()).arg(KGlobal::locale()->formatDate(m_transaction.postDate(), true)), i18n("Update price info"), KStdGuiItem::yes(), KStdGuiItem::no(), "StoreUpdatedPrice") == KMessageBox::Yes) {
            m_equity.editPriceHistory(m_transaction.postDate(), price);
            equityChanged = true;
          }
        }
#endif

        if(securityChanged) {
          try {
            file->modifySecurity(m_security);
          } catch(MyMoneyException *e) {
            KMessageBox::detailedSorry(0, i18n("Unable to add/modify security"),
              (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
            delete e;
          }
        }
      }

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to add/modify transaction"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
    }
  }

  m_register->setInlineEditingMode(false);
}

void KLedgerViewInvestments::slotSecurityChanged(const QCString& id)
{
  MyMoneyTransaction t;
  MyMoneySplit s;

  t = m_transaction;
  s = m_split;

  try {
    createSplits();
    MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
    m_security = MyMoneyFile::instance()->security(acc.currencyId());
    m_editStockAccount->setSelected(acc);
    m_split.setAccountId(id);
    reloadEditWidgets(m_transaction);
    slotDataChanged(None);
  } catch(MyMoneyException *e) {
    delete e;
  }
}

void KLedgerViewInvestments::refreshView(const bool /* transactionFormVisible */)
{
  // if we're currently editing a transaction, we don't refresh the view
  // this will screw us, if someone creates a category on the fly, as this
  // will come here when the notifications by the engine are send out.
  if(isEditMode())
    return;

  // if a transaction is currently selected, keep the id
  QCString transactionId;
  if(m_transactionPtr != 0)
    transactionId = m_transactionPtr->id();
  m_transactionPtr = 0;
  m_transactionPtrVector.clear();

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
      filter.setDateFilter(KMyMoneySettings::startDate().date(), QDate());

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

void KLedgerViewInvestments::updateValues(int field)
{
  MyMoneyMoney fees, shares, price, total;
  ChangedFieldE calcField = Total;
#if 0
  // in case of negative share value, we might have to adjust the type of transaction
  if(field == Shares && m_editShares->value() < 0) {
    int prec = MyMoneyMoney::denomToPrec(m_equity.smallestAccountFraction());
    switch(m_editType->currentItem()) {
      case BuyShares:
        m_editType->setCurrentItem(SellShares);
        break;
      case SellShares:
        m_editType->setCurrentItem(BuyShares);
        break;
      case AddShares:
        m_editType->setCurrentItem(RemoveShares);
        break;
      case RemoveShares:
        m_editType->setCurrentItem(AddShares);
        break;
      default:
        break;
    }
    m_transactionType = static_cast<investTransactionTypeE> (m_editType->currentItem());
    m_editShares->loadText(m_editShares->value().abs().formatMoney("", prec));
  }
#endif

  switch(m_editType->currentItem()) {
    case BuyShares:
    case SellShares:
    case ReinvestDividend:
      fees = m_editFees->value();
      if(m_editFeeCategory->selectedAccountId().isEmpty())
        fees = MyMoneyMoney(0,1);
      shares = m_editShares->value();
      price = m_editPPS->value();
      total = m_editAmount->value();
      switch(field) {
        case Fees:
          if(!shares.isZero() && !price.isZero())
            calcField = Total;
          else if(!shares.isZero() && !total.isZero())
            calcField = Price;
          else if(!price.isZero() && !total.isZero())
            calcField = Shares;
          break;

        case Price:
          if(m_editFeeCategory->selectedAccountId().isEmpty()) {
            if(!shares.isZero())
              calcField = Total;
            else if(!total.isZero())
              calcField = Shares;
          } else {
            if(!shares.isZero() && !fees.isZero())
              calcField = Total;
            else if(!shares.isZero() && !total.isZero())
              calcField = Fees;
            else if(!total.isZero() && !fees.isZero())
              calcField = Shares;
          }
          break;

        case Shares:
          if(m_editFeeCategory->selectedAccountId().isEmpty()) {
            if(!price.isZero())
              calcField = Total;
            else if(!total.isZero())
              calcField = Price;
          } else {
            if(!price.isZero() && !fees.isZero())
              calcField = Total;
            else if(!price.isZero() && !total.isZero())
              calcField = Fees;
            else if(!fees.isZero() && !total.isZero())
              calcField = Price;
          }
          break;

        case Total:
          if(m_editFeeCategory->selectedAccountId().isEmpty()) {
            if(!shares.isZero()) {
              calcField = Price;
            } else if(!price.isZero()) {
              calcField = Shares;
            }
          } else {
            if(!fees.isZero() && !shares.isZero())
              calcField = Price;
            else if(!fees.isZero() && !price.isZero())
              calcField = Shares;
            else if(!price.isZero() && !shares.isZero())
              calcField = Fees;
          }
          break;

        case None:
          break;
      }
      break;

    case Dividend:
    case Yield:
    case AddShares:
    case RemoveShares:
    case SplitShares:
      // in these cases, there is no automatic update
      break;
  }

  switch(calcField) {
    default:
      break;

    case Shares:
      shares = fees;
      if(m_editType->currentItem() != SellShares)
        shares = -shares;
      shares = (total - shares) / price;
      m_editShares->setValue(shares.abs());
      break;

    case Fees:
      fees = total - (price * shares);
      if(m_editType->currentItem() == SellShares)
        fees = -fees;
      m_editFees->setValue(fees.abs());
      break;

    case Price:
      price = fees;
      if(m_editType->currentItem() != SellShares)
        price = -price;
      price = (total + price) / shares;
      m_editPPS->setValue(price.abs());
      break;

    case Total:
      total = fees;
      if(m_editType->currentItem() == SellShares)
        total = -total;
      total = total + (price * shares);
      m_editAmount->setValue(total.abs());
      break;
  }
}

void KLedgerViewInvestments::updateEditWidgets(void)
{
  QWidget* focusWidget;

  Q_CHECK_PTR(static_cast<void*>(m_editType));

  m_editType->setCurrentItem(m_transactionType);
  createSplits();
  reloadEditWidgets(m_transaction);

  if(KMyMoneySettings::transactionForm()) {
    focusWidget = arrangeEditWidgetsInForm();
  } else {
    focusWidget = arrangeEditWidgetsInRegister();
  }

  // make sure, size of all form columns are correct
  resizeEvent(0);

  m_tabOrderWidgets.find(focusWidget);
  focusWidget->setFocus();
}

const bool KLedgerViewInvestments::slotDataChanged(int field)
{
  Q_CHECK_PTR(static_cast<void*>(m_editType));

  // check if a change in widget presentation is required
  if(m_transactionType != m_editType->currentItem()) {
    m_transactionType = static_cast<investTransactionTypeE> (m_editType->currentItem());
    // hideWidgets();
    fillFormStatics();
    updateEditWidgets();
  }

  // we don't allow to enter a fee until there's a fee category entered
  if(m_editFees && m_editFeeCategory) {
    m_editFees->setEnabled(!m_editFeeCategory->selectedAccountId().isEmpty());
  }

  updateValues(field);

  // check if we can enable the ENTER buttons. the condition heavily
  // depends on the current action
  bool ok = true;
  if(m_editStockAccount->selectedAccounts().first().isEmpty()) {
    ok = false;
  }

  // if there is a difference in securities of the cash account, fee/interest
  // category and the security of the stock then we tell the user that all
  // values will be treated in the security of the stock.
  QCString id;
  QString fieldName;
  switch(field) {
    case CashAccount:
      id = m_editCashAccount->selectedAccounts().first();
      fieldName = i18n("account");
      break;
    case FeeCategory:
      id = m_editFeeCategory->selectedAccountId();
      fieldName = i18n("category");
      break;

    default:
      break;
  }

  if(!id.isEmpty()) {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
    if(acc.currencyId() != m_security.tradingCurrency()) {
      MyMoneySecurity currency = MyMoneyFile::instance()->security(m_security.tradingCurrency());
      KMessageBox::information(this, QString("<p>")+i18n("The %1 <b>%2</b> uses a different currency than the selected investment. Please make sure to enter all values for this transaction in <b>%2</b>. When you save this transaction you will have the chance to enter the necessary conversion rates.").arg(fieldName).arg(acc.name()).arg(currency.name()), i18n("Stock security"), "StockDifferentSecurityWarning");
    }
  }

  switch(m_editType->currentItem()) {
    case BuyShares:
    case SellShares:
      if(m_editCashAccount->selectedAccounts().first().isEmpty()) {
        ok = false;
      }
      if(m_editPPS->value().isZero()) {
        ok = false;
      }
      if(m_editShares->value().isZero()) {
        ok = false;
      }
      break;

    case ReinvestDividend:
      if(m_editCashAccount->selectedAccounts().first().isEmpty())
        ok = false;
      if(m_editPPS->value().isZero())
        ok = false;
      if(m_editShares->value().isZero())
        ok = false;
      if(!m_editFeeCategory->selectedAccountId().isEmpty()
      && m_editFees->value().isZero())
        ok = false;
      break;

    case Dividend:
    case Yield:
      if(m_editCashAccount->selectedAccounts().first().isEmpty()
      || m_editFeeCategory->selectedAccountId().isEmpty()
      || m_editFees->value().isZero())
        ok = false;
      break;

    case AddShares:
    case RemoveShares:
    case SplitShares:
      if(m_editShares->value().isZero())
        ok = false;
      break;
  }
  enableOkButton(ok);

  return m_form->enterButton()->isEnabled();
}

KMyMoneyTransaction* KLedgerViewInvestments::transaction(const int idx) const
{
  KMyMoneyTransaction* p = KLedgerView::transaction(idx);
  if(p) {
    // in case we have a transaction at that index, we check
    // if it's the one we currently edit. If that is the case,
    // we pass on 0 to the caller (kMyMoneyRegisterInvestment resp.
    // it's base class) so that no data is written to the visible
    // parts of the register where no edit widgets are shown for
    // the currently selected action. This is useful when you change
    // the action of an existing transaction.
    if(p == m_transactionPtr && isEditMode() && !KMyMoneySettings::transactionForm())
      p = 0;
  }
  return p;
}

bool KLedgerViewInvestments::eventFilter( QObject *o, QEvent *e )
{
  return KLedgerView::eventFilter(o, e);
}

bool KLedgerViewInvestments::setupPrice(MyMoneySplit& split)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount acc = MyMoneyFile::instance()->account(split.accountId());
  if(acc.currencyId() != m_security.tradingCurrency()) {
    MyMoneySecurity toCurrency(file->currency(acc.currencyId()));
    int fract = toCurrency.smallestAccountFraction();
    if(acc.accountType() == MyMoneyAccount::Cash)
      fract = toCurrency.smallestCashFraction();

    QMap<QCString, MyMoneyMoney>::Iterator it_p;
    QCString key = m_transaction.commodity() + "-" + acc.currencyId();
    it_p = m_priceInfo.find(key);

    // if it's not found, then collect it from the user first
    MyMoneyMoney price;
    if(it_p == m_priceInfo.end()) {
      MyMoneySecurity fromCurrency = file->security(m_security.tradingCurrency());
      MyMoneyMoney fromValue, toValue;

      fromValue = split.value();
      MyMoneyPrice priceInfo = file->price(fromCurrency.id(), toCurrency.id());
      toValue = split.value() * priceInfo.rate();

      KCurrencyCalculator calc(fromCurrency,
                                toCurrency,
                                fromValue,
                                toValue,
                                m_transaction.postDate(),
                                fract,
                                this, "currencyCalculator");

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
  }
  return true;
}

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef ACTIVITY_ROW
#undef DATE_ROW
#undef SYMBOL_ROW
#undef QUANTITY_ROW
#undef MEMO_ROW
#undef PRICE_ROW
#undef FEES_ROW
#undef CATEGORY_ROW
#undef ACCOUNT_ROW
#undef VALUE_ROW

#undef ACTIVITY_TXT_COL
#undef ACTIVITY_DATA_COL
#undef DATE_TXT_COL
#undef DATE_DATA_COL
#undef SYMBOL_TXT_COL
#undef SYMBOL_DATA_COL
#undef QUANTITY_TXT_COL
#undef QUANTITY_DATA_COL
#undef MEMO_TXT_COL
#undef MEMO_DATA_COL
#undef PRICE_TXT_COL
#undef PRICE_DATA_COL
#undef CATEGORY_TXT_COL
#undef CATEGORY_DATA_COL
#undef FEES_TXT_COL
#undef FEES_DATA_COL
#undef ACCOUNT_TXT_COL
#undef ACCOUNT_DATA_COL
#undef VALUE_TXT_COL
#undef VALUE_DATA_COL

#include "kledgerviewinvestments.moc"
