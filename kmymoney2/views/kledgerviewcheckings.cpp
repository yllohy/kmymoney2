/***************************************************************************
                          kledgerviewcheckings.cpp  -  description
                             -------------------
    begin                : Sun Jul 14 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

#include <qlayout.h>
#include <qfocusdata.h>
#include <qwidgetstack.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerviewcheckings.h"
#include "../widgets/kmymoneytransactionform.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneyregistercheckings.h"
#include "../widgets/kmymoneyaccountselector.h"
#include "../dialogs/kendingbalancedlg.h"
#include "../dialogs/ksplittransactiondlg.h"
#include "../dialogs/knewaccountdlg.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyutils.h"
#include "../views/kmymoneyview.h"

#define PAYEE_ROW       0
#define CATEGORY_ROW    1
#define MEMO_ROW        2
#define NR_ROW          0
#define DATE_ROW        1
#define AMOUNT_ROW      2

#define PAYEE_TXT_COL     0
#define PAYEE_DATA_COL    (PAYEE_TXT_COL+1)
#define CATEGORY_TXT_COL  0
#define CATEGORY_DATA_COL (CATEGORY_TXT_COL+1)
#define MEMO_TXT_COL      0
#define MEMO_DATA_COL     (MEMO_TXT_COL+1)
#define NR_TXT_COL        3
#define NR_DATA_COL       (NR_TXT_COL+1)
#define DATE_TXT_COL      3
#define DATE_DATA_COL     (DATE_TXT_COL+1)
#define AMOUNT_TXT_COL    3
#define AMOUNT_DATA_COL   (AMOUNT_TXT_COL+1)

KLedgerViewCheckingsSummaryLine::KLedgerViewCheckingsSummaryLine(QWidget* parent, const char* name) :
  QFrame(parent, name)
{
  QHBoxLayout* summaryLayout = new QHBoxLayout(this, 0, 6, "SummaryLayout");

  m_date = new QLabel("", this);
  summaryLayout->addWidget(m_date);

  QSpacerItem* spacer = new QSpacerItem( 20, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );
  summaryLayout->addItem(spacer);

  m_balance = new QLabel(this);

  summaryLayout->addWidget(m_balance);
}

void KLedgerViewCheckingsSummaryLine::setBalance(const QString& txt)
{
  m_balance->setText(txt);
}

void KLedgerViewCheckingsSummaryLine::setReconciliationDate(const QString& txt)
{
  m_date->setText(txt);
}

void KLedgerViewCheckingsSummaryLine::clear(void)
{
  m_balance->setText("");
  m_date->setText("");
}

KLedgerViewCheckings::KLedgerViewCheckings(QWidget *parent, const char *name )
  : KLedgerView(parent,name)
{
  QGridLayout* formLayout = new QGridLayout( this, 1, 2, 11, 6, "FormLayout");
  QVBoxLayout* ledgerLayout = new QVBoxLayout( 6, "LedgerLayout");

  createRegister();
  ledgerLayout->addWidget(m_register, 3);

  createSummary();
  ledgerLayout->addWidget(m_summaryLine);

  createReconciliationFrame();
  ledgerLayout->addWidget(m_reconciliationFrame);

  createForm();
  // make sure, transfers are disabled if required
  m_tabTransfer->setEnabled(transfersPossible());
  ledgerLayout->addWidget(m_form);

  formLayout->addLayout( ledgerLayout, 0, 0);

  // create the context menu that is accessible via RMB
  createContextMenu();

  // create the context menus that are accessible via transaction form buttons
  createMoreMenu();
  createAccountMenu();

  connect(m_contextMenu, SIGNAL(aboutToShow()), SLOT(slotConfigureContextMenu()));
  connect(m_moreMenu, SIGNAL(aboutToShow()), SLOT(slotConfigureMoreMenu()));

  // load the form with inital settings. Always consider transaction type Deposit
  m_form->tabBar()->blockSignals(true);
  m_form->tabBar()->setCurrentTab(m_form->tabBar()->tabAt(m_form->tabBar()->indexOf(1)));
  m_form->tabBar()->blockSignals(false);

  // setup the form to be visible or not
  slotShowTransactionForm(m_transactionFormActive);

  // and the register has the focus
  m_register->setFocus();
}

KLedgerViewCheckings::~KLedgerViewCheckings()
{
  delete m_register;
}

void KLedgerViewCheckings::refreshView(void)
{
  if(m_inReconciliation)
    KLedgerView::refreshView(m_transactionCheckBox->isChecked());
  else
    KLedgerView::refreshView();

  QDate date;
  if(!m_account.value("lastStatementDate").isEmpty())
    date = QDate::fromString(m_account.value("lastStatementDate"), Qt::ISODate);
  m_tabTransfer->setEnabled(transfersPossible());
}

void KLedgerViewCheckings::resizeEvent(QResizeEvent* /* ev */)
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
  if(!m_transactionFormActive) {
    kMyMoneyDateInput* dateField = new kMyMoneyDateInput(0, "editDate");
    kMyMoneyEdit* valField = new kMyMoneyEdit(0, "valEdit");

    dateField->setFont(m_register->cellFont());
    m_register->setColumnWidth(1, dateField->minimumSizeHint().width());
    valField->setMinimumWidth(width);
    width = valField->minimumSizeHint().width();

    delete valField;
    delete dateField;
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

  resizeForm();
  m_register->setUpdatesEnabled(true);
  m_register->repaintContents(false);
}

void KLedgerViewCheckings::resizeForm(void)
{
  // now resize the form
  kMyMoneyDateInput dateInput(0, "editDate");
  KPushButton splitButton(i18n("Split"), 0, "editSplit");

  kMyMoneyTransactionFormTable* table = static_cast<kMyMoneyTransactionFormTable *>(m_form->table());
  table->adjustColumn(0);
  table->setColumnWidth(2, splitButton.sizeHint().width());
  table->adjustColumn(3);
  table->adjustColumn(4, dateInput.minimumSizeHint().width()+10);

  int w = table->visibleWidth();
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

void KLedgerViewCheckings::enableWidgets(const bool enable)
{
  KLedgerView::enableWidgets(enable);
}

void KLedgerViewCheckings::slotNew(void)
{
  switch( m_form->tabBar()->currentTab() ) {
    case 0:
      m_action = MyMoneySplit::ActionCheck;
      break;
    case 4:
      m_action = MyMoneySplit::ActionATM;
      break;
    default:
      m_action = QCString();
      break;
  }
  KLedgerView::slotNew();
}

void KLedgerViewCheckings::slotAmountChanged(const QString& amount)
{
  MyMoneyMoney value(amount);

  // if the direction of the payment is yet unknown, we take
  // the sign of the value and the selected tab into account
  // and determine the value. The adjusted string is passed
  // on to KLedgerView::slotAmountChanged().
  int dir = transactionDirection(m_split);
  if(dir == UnknownDirection) {
    MyMoneyMoney sign(1,1);
    if(value.isNegative()) {
      sign = -sign;
      value = value.abs();
    }
    switch(m_form->tabBar()->currentTab()) {
      case 1:    // Deposit
        break;
      default:
        sign = -sign;
        break;
    }
    value = value * sign;
  }

  KLedgerView::slotAmountChanged(value.formatMoney());

  // check if we need to update the tab
  if(dir != UnknownDirection && dir != transactionDirection(m_split)) {
    // update the tab bar (this will call slotActionSelected() below)
    updateTabBar(m_transaction, m_split);
  }
}

int KLedgerViewCheckings::currentActionTab(void) const
{
  QTabBar* bar = m_form->tabBar();
  return bar->currentTab();
}

void KLedgerViewCheckings::updateTabBar(const MyMoneyTransaction& t, const MyMoneySplit& s, const bool enableAll)
{
  int selectedTab = actionTab(t, s);
  QTabBar* bar = m_form->tabBar();
  MyMoneySplit otherSplit;
  if(t.splitCount() > 1)
    otherSplit = t.splitByAccount(m_account.id(), false);

  if(isEditMode()) {
    // determine the tab differently if edit-mode or not
    if(otherSplit.accountId().isEmpty() || s.value().isZero()) {
      // don't touch current setting if info ambigious
      selectedTab = bar->currentTab();
    }
  }

  QPtrList<QTab> list;
  QTab* it;

  bar->setCurrentTab(selectedTab);

  if(m_tabAtm)        list.append(m_tabAtm);
  if(m_tabDeposit)    list.append(m_tabDeposit);
  if(m_tabCheck)      list.append(m_tabCheck);
  if(m_tabWithdrawal) list.append(m_tabWithdrawal);

  if(otherSplit.accountId().isEmpty() || enableAll) {
    for(it = list.first(); it; it = list.next())
      it->setEnabled(true);
    if(m_tabTransfer)
      m_tabTransfer->setEnabled(transfersPossible());

  } else {
    bool isXfer = false;
    if(t.splitCount() == 2) {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(otherSplit.accountId());
      isXfer = acc.accountGroup() == MyMoneyAccount::Asset
               || acc.accountGroup() == MyMoneyAccount::Liability;

    }
    for(it = list.first(); it; it = list.next())
      it->setEnabled(!isXfer);
    if(m_tabTransfer)
      m_tabTransfer->setEnabled(isXfer && transfersPossible());
  }

  // force repaint, unfortunately, setEnabled() does not do that for us
  bar->update();

  // update the edit type widget if present
  if(m_editType) {
    m_editType->clear();
    int j = 0;
    for(int i = 0; i < 5; ++i) {
      QTab* tab = bar->tab(i);
      if(tab && tab->isEnabled()) {
        m_actionIdx[j++] = i;
        QString txt = tab->text();
        txt = txt.replace(QRegExp("&"), "");
        m_editType->insertItem(txt);
        if(selectedTab == i)
          m_editType->loadCurrentItem(j-1);
      }
    }
    while(j < 5) {
      m_actionIdx[j++] = -1;
    }
  }
}

void KLedgerViewCheckings::slotTypeSelected(int type)
{
  if(m_actionIdx[type] != -1)
    slotActionSelected(m_actionIdx[type]);
  else
    qWarning("Invalid type %d passed to KLedgerViewCheckings::slotTypeSelected", type);
}

void KLedgerViewCheckings::slotActionSelected(int type)
{
  if(!m_form->tabBar()->signalsBlocked()
  && !isEditMode()) {
    slotNew();

  } else if(isEditMode()) {
    // if a transaction is currently edited, we need to change
    // it's type here. Also, we have to check if the sign of amount
    // changes.

    // if the direction of the payment changes, we revert the
    // sign of all splits
    if((transactionDirection(m_split) == Credit && type != 1)
    || (transactionDirection(m_split) == Debit && type == 1)) {
      QValueList<MyMoneySplit>::Iterator it;
      QValueList<MyMoneySplit> list = m_transaction.splits();

      for(it = list.begin(); it != list.end(); ++it) {
        (*it).setValue(-((*it).value()));
        (*it).setShares(-((*it).shares()));
        m_transaction.modifySplit(*it);
      }
      // update the local member copy
      m_split = m_transaction.splitByAccount(accountId());
    }

    switch( type ) {
      case 0:
        actionChanged(MyMoneySplit::ActionCheck);
        break;
      case 4:
        actionChanged(MyMoneySplit::ActionATM);
        break;
      default:
        actionChanged(QCString());
        break;
    }

    createEditWidgets();
    reloadEditWidgets(m_transaction);

    QWidget* focusWidget;
    if(m_transactionFormActive) {
      focusWidget = arrangeEditWidgetsInForm();
    } else {
      focusWidget = arrangeEditWidgetsInRegister();
    }

    // setup the keyboard filter for all widgets
    for(QWidget* w = m_tabOrderWidgets.first(); w; w = m_tabOrderWidgets.next()) {
      w->installEventFilter(this);
    }

    // select the focus widget
    if(m_tabOrderWidgets.find(qApp->focusWidget()) == -1) {
      m_tabOrderWidgets.find(focusWidget);
      focusWidget->setFocus();
    }
  }
  autoIncCheckNumber();
}

void KLedgerViewCheckings::slotRegisterDoubleClicked(int /* row */,
                                                     int /* col */,
                                                     int /* button */,
                                                     const QPoint & /* mousePos */)
{
  slotStartEdit();
}

void KLedgerViewCheckings::createRegister(void)
{
  KLedgerView::createRegister(new kMyMoneyRegisterCheckings(this, "kMyMoneyRegisterCheckings"));

#define LOADACTION(a) m_register->setAction(QCString(a), action2str(a))
  LOADACTION(MyMoneySplit::ActionATM);
  LOADACTION(MyMoneySplit::ActionCheck);
  LOADACTION(MyMoneySplit::ActionDeposit);
  LOADACTION(MyMoneySplit::ActionWithdrawal);
  LOADACTION(MyMoneySplit::ActionTransfer);
}

void KLedgerViewCheckings::createAccountMenu(void)
{
  // get the basic entries for all ledger views
  KLedgerView::createAccountMenu();

  m_form->accountButton()->setPopup(m_accountMenu);
}

void KLedgerViewCheckings::createMoreMenu(void)
{
  // get the basic entries for all ledger views
  KLedgerView::createMoreMenu();

  // and now the specific entries for checkings/savings etc.
  KIconLoader *kiconloader = KGlobal::iconLoader();
  m_moreMenu->insertItem(i18n("Edit splits ..."), this, SLOT(slotOpenSplitDialog()),
      QKeySequence(), -1, 1);

  m_moreMenu->insertItem(kiconloader->loadIcon("goto", KIcon::Small),
                         i18n("Goto payee/receiver"), this, SLOT(slotPayeeSelected()),
                         QKeySequence(), -1, 2);
  m_moreMenu->insertItem(kiconloader->loadIcon("bookmark_add", KIcon::Small),
                         i18n("Create schedule..."), this, SLOT(slotCreateSchedule()),
                         QKeySequence(), -1, 3);

  m_form->moreButton()->setPopup(m_moreMenu);
}

void KLedgerViewCheckings::createContextMenu(void)
{
  // get the basic entries for all ledger views
  KLedgerView::createContextMenu();

  // and now the specific entries for checkings/savings etc.
  KIconLoader *kiconloader = KGlobal::iconLoader();

  m_contextMenu->insertItem(i18n("Edit splits ..."), this, SLOT(slotOpenSplitDialog()),
      QKeySequence(), -1, 2);

  m_contextMenu->insertItem(kiconloader->loadIcon("goto", KIcon::Small),
                            i18n("Goto payee/receiver"), this, SLOT(slotPayeeSelected()),
                            QKeySequence(), -1, 3);
  m_contextMenu->insertItem(kiconloader->loadIcon("bookmark_add", KIcon::Small),
                            i18n("Create schedule..."), this, SLOT(slotCreateSchedule()),
                            QKeySequence(), -1, 4);
}

int KLedgerViewCheckings::actionTab(const MyMoneyTransaction& t, const MyMoneySplit& split) const
{
  if(transactionType(t) == KMyMoneyUtils::Transfer) {
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

void KLedgerViewCheckings::createForm(void)
{
  // determine the height of the objects in the table
  kMyMoneyDateInput dateInput(0, "editDate");
  KPushButton splitButton(i18n("Split"), 0, "editSplit");
  kMyMoneyCategory category(0, "category");

  // extract the maximal sizeHint height and subtract 8
  int h = QMAX(dateInput.sizeHint().height(), splitButton.sizeHint().height());
  h = QMAX(h, category.sizeHint().height())-4;

  m_form = new kMyMoneyTransactionForm(this, "kMyMoneyTransactionForm", 0, 3, 5, h);
  m_tabCheck = new QTab(action2str(MyMoneySplit::ActionCheck, true));
  m_tabDeposit = new QTab(action2str(MyMoneySplit::ActionDeposit, true));
  m_tabTransfer = new QTab(action2str(MyMoneySplit::ActionTransfer, true));
  m_tabWithdrawal = new QTab(action2str(MyMoneySplit::ActionWithdrawal, true));
  m_tabAtm = new QTab(action2str(MyMoneySplit::ActionATM, true));

  m_form->addTab(m_tabCheck);
  m_form->addTab(m_tabDeposit);
  m_form->addTab(m_tabTransfer);
  m_form->addTab(m_tabWithdrawal);
  m_form->addTab(m_tabAtm);

  // never show horizontal scroll bars
  m_form->table()->setHScrollBarMode(QScrollView::AlwaysOff);

  // adjust size of form table
  m_form->table()->setMaximumHeight(m_form->table()->rowHeight(0)*m_form->table()->numRows());
  m_form->table()->setMinimumHeight(m_form->table()->rowHeight(0)*m_form->table()->numRows());

  // connections
  connect(m_form->tabBar(), SIGNAL(selected(int)), this, SLOT(slotActionSelected(int)));

  connect(m_form->editButton(), SIGNAL(clicked()), this, SLOT(slotStartEdit()));
  connect(m_form->cancelButton(), SIGNAL(clicked()), this, SLOT(slotCancelEdit()));
  connect(m_form->enterButton(), SIGNAL(clicked()), this, SLOT(slotEndEdit()));
  connect(m_form->newButton(), SIGNAL(clicked()), this, SLOT(slotNew()));
}

void KLedgerViewCheckings::createReconciliationFrame(void)
{
  m_reconciliationFrame = new QFrame(this);
  QVBoxLayout* vlayout = new QVBoxLayout(m_reconciliationFrame, 0, 6, "ReconcileLayout");
  QGridLayout* llayout = new QGridLayout(2, 3, 6, "LabelReconcileLayout");
  QHBoxLayout* blayout = new QHBoxLayout(6, "ButtonReconcileLayout");

  m_statementLabel = new QLabel(m_reconciliationFrame, 0);
  llayout->addWidget(m_statementLabel, 0, 0, Qt::AlignLeft);
  m_clearedLabel = new QLabel(m_reconciliationFrame, 0);
  llayout->addWidget(m_clearedLabel, 0, 1, Qt::AlignHCenter);
  m_differenceLabel = new QLabel(m_reconciliationFrame, 0);
  llayout->addWidget(m_differenceLabel, 0, 2, Qt::AlignRight);

  m_transactionCheckBox = new QCheckBox(i18n("Show transactionform"), m_reconciliationFrame, "showBox");
  blayout->addWidget(m_transactionCheckBox);

  QSpacerItem* spacer = new QSpacerItem( 20, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );
  blayout->addItem(spacer);

  KIconLoader *il = KGlobal::iconLoader();
  KPushButton* m_finishButton = new KPushButton(m_reconciliationFrame, "FinishButton");
  KGuiItem finishButtenItem( i18n("&Finish"),
                    QIconSet(il->loadIcon("player_end", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Finialize reconciliation"),
                    i18n("Use this to finish reconciling your account against the bank statement."));
  m_finishButton->setGuiItem(finishButtenItem);
  blayout->addWidget(m_finishButton);

  connect(m_finishButton, SIGNAL(clicked()), this, SLOT(slotEndReconciliation()));

  KPushButton* m_postponeButton = new KPushButton(m_reconciliationFrame, "PostponeButton");
  KGuiItem postponeButtenItem( i18n("&Postpone"),
                    QIconSet(il->loadIcon("player_pause", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Postpone reconciliation"),
                    i18n("Use this to postpone reconciling your account against the bank statement to a later point in time."));
  m_postponeButton->setGuiItem(postponeButtenItem);
  blayout->addWidget(m_postponeButton);

  connect(m_postponeButton, SIGNAL(clicked()), this, SLOT(slotPostponeReconciliation()));
  // initially, this part is hidden
  m_reconciliationFrame->hide();

  vlayout->addLayout(llayout);
  vlayout->addLayout(blayout);
}

void KLedgerViewCheckings::createSummary(void)
{
  m_summaryLine = new KLedgerViewCheckingsSummaryLine(this, 0);
/*
  m_summaryLayout = new QHBoxLayout(6, "SummaryLayout");

  m_lastReconciledLabel = new QLabel("", this);
  m_summaryLayout->addWidget(m_lastReconciledLabel);

  QSpacerItem* spacer = new QSpacerItem( 20, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );
  m_summaryLayout->addItem(spacer);

  m_summaryLine = new QLabel(this);

  m_summaryLayout->addWidget(m_summaryLine);
*/
}

#if 0
void KLedgerViewCheckings::createInfoStack(void)
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

  m_detailsButton = new KPushButton(frame, "KPushButton_Details" );
  KGuiItem detailsButtenItem( i18n("&Account Details"),
                    QIconSet(il->loadIcon("viewmag", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Open the account dialog"),
                    i18n("Use this view and modify the account details."));
  m_detailsButton->setGuiItem(detailsButtenItem);
  buttonLayout->addWidget(m_detailsButton);

  m_reconcileButton = new KPushButton(frame, KAppTest::widgetName(this, "KPushButton/Reconcile"));
  KGuiItem reconcileButtenItem( i18n("&Reconcile ..."),
                    QIconSet(il->loadIcon("reconcile", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Start the account reconciliation"),
                    i18n("Use this to reconcile your account against the bank statement."));
  m_reconcileButton->setGuiItem(reconcileButtenItem);
  buttonLayout->addWidget(m_reconcileButton);

  m_lastReconciledLabel = new QLabel("", frame);
  buttonLayout->addWidget(m_lastReconciledLabel);

  connect(m_reconcileButton, SIGNAL(clicked()), this, SLOT(slotReconciliation()));
  connect(m_detailsButton, SIGNAL(clicked()), this, SLOT(slotAccountDetail()));

  QSpacerItem* spacer = new QSpacerItem( 20, 20,
                   QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonLayout->addItem( spacer );

  m_infoStack->addWidget(frame, KLedgerView::TransactionEdit);


  // Second page reconciliation info
  frame = new QFrame(m_infoStack, "ReconcileFrame");

  QVBoxLayout* reconcileLayout = new QVBoxLayout( frame, 0, 6, "ReconcileLayout");
#if 0

  frame->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                     QSizePolicy::Minimum,
                                     0, 0,
                                     frame->sizePolicy().hasHeightForWidth() ) );

  QFrame* innerFrame = new QFrame(frame, "InnerFrame");

  innerFrame->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                     QSizePolicy::Minimum,
                                     0, 0,
                                     frame->sizePolicy().hasHeightForWidth() ) );

  innerFrame->setFrameShadow( QFrame::Plain );
  innerFrame->setFrameShape( QFrame::Panel );
  innerFrame->setMaximumWidth(125);
  innerFrame->setLineWidth(1);

  QVBoxLayout* innerLayout = new QVBoxLayout( innerFrame, 6, 0, "InnerLayout");

  QLabel* txt;
  txt = new QLabel(i18n("<center><b>Reconcile account</b></center><hr>\n"
                        "<b>1.</b> Click on column 'C' "
                        "to clear the transactions "
                        "appearing on your bank statement."), innerFrame);


  QFont defaultFont = KMyMoneyUtils::cellFont();
  txt->setFont(defaultFont);
  innerLayout->addWidget(txt);

  txt = new QLabel(i18n("<b>2.</b> Match the cleared "
                        "transactions with the amount "
                        "noted on your bank statement.<br>"), innerFrame);
  txt->setFont(defaultFont);
  innerLayout->addWidget(txt);


  QHBoxLayout* boxLayout = new QHBoxLayout(innerLayout, 0, "ClearedLayout");

  txt = new QLabel(i18n("Cleared:"), innerFrame);
  txt->setFont(defaultFont);
  boxLayout->addWidget(txt);

  spacer = new QSpacerItem( 1, 1,
                   QSizePolicy::Expanding, QSizePolicy::Minimum );
  boxLayout->addItem( spacer );

  m_clearedLabel = new QLabel("", innerFrame);
  m_clearedLabel->setFont(defaultFont);
  m_clearedLabel->setAlignment(Right);
  boxLayout->addWidget(m_clearedLabel);

  boxLayout = new QHBoxLayout(innerLayout, 0, "StatementLayout");

  txt = new QLabel(i18n("Statement:"), innerFrame);
  txt->setFont(defaultFont);
  boxLayout->addWidget(txt);

  spacer = new QSpacerItem( 1, 1,
                   QSizePolicy::Expanding, QSizePolicy::Minimum );
  boxLayout->addItem( spacer );

  m_statementLabel = new QLabel("", innerFrame);
  m_statementLabel->setFont(defaultFont);
  m_statementLabel->setAlignment(Right);
  boxLayout->addWidget(m_statementLabel);

  txt = new QLabel("<hr>", innerFrame);
  txt->setFont(defaultFont);
  txt->setAlignment(Top);
  txt->setMaximumHeight(10);
  innerLayout->addWidget(txt);


  boxLayout = new QHBoxLayout(innerLayout, 0, "DiffLayout");

  txt = new QLabel(i18n("Difference:"), innerFrame);
  txt->setFont(defaultFont);
  boxLayout->addWidget(txt);

  spacer = new QSpacerItem( 1, 1,
                   QSizePolicy::Expanding, QSizePolicy::Minimum );
  boxLayout->addItem( spacer );

  m_differenceLabel = new QLabel("", innerFrame);
  m_differenceLabel->setFont(defaultFont);
  m_differenceLabel->setAlignment(Right);
  boxLayout->addWidget(m_differenceLabel);


  txt = new QLabel(i18n("<p><b>3.</b> Hit the Finish "
                        "button when you're done."), innerFrame);
  txt->setFont(defaultFont);
  innerLayout->addWidget(txt);

  m_transactionCheckBox = new QCheckBox(i18n("Show transactionform"), innerFrame, "showBox");
  m_transactionCheckBox->setFont(defaultFont);
  innerLayout->addWidget(m_transactionCheckBox);


  reconcileLayout->addWidget(innerFrame);
#endif

  KPushButton* m_finishButton = new KPushButton(frame, "FinishButton");
  m_finishButton->setText(i18n("&Finish"));
  reconcileLayout->addWidget(m_finishButton);

  connect(m_finishButton, SIGNAL(clicked()), this, SLOT(slotEndReconciliation()));

  KPushButton* m_postponeButton = new KPushButton(frame, "PostponeButton");
  m_postponeButton->setText(i18n("&Postpone"));
  reconcileLayout->addWidget(m_postponeButton);

  connect(m_postponeButton, SIGNAL(clicked()), this, SLOT(slotPostponeReconciliation()));

  spacer = new QSpacerItem( 1, 1,
                   QSizePolicy::Minimum, QSizePolicy::Expanding );
  reconcileLayout->addItem( spacer );

  m_infoStack->addWidget(frame, KLedgerView::Reconciliation);

  // Initially show the page with the buttons
  m_infoStack->raiseWidget(KLedgerView::TransactionEdit);
  m_infoStack->hide();
}
#endif

void KLedgerViewCheckings::fillSummary(void)
{
  MyMoneyMoney balance;
  MyMoneyFile* file = MyMoneyFile::instance();

  KLedgerViewCheckingsSummaryLine* summary = dynamic_cast<KLedgerViewCheckingsSummaryLine*>(m_summaryLine);
  if(summary) {
    summary->clear();

    if(accountId().length() > 0) {
      try {
        balance = file->balance(accountId());
        QString txt = balance.formatMoney(file->currency(m_account.currencyId()).tradingSymbol());
        if(balance.isNegative())
          txt = "<font color=\"red\"><b>" + txt + "</b></font>";
        txt = "<nobr>"+ i18n("Current balance: ") + txt + "</nobr>";

        summary->setBalance(txt);

        QDate date;
        if(!m_account.value("lastStatementDate").isEmpty())
          date = QDate::fromString(m_account.value("lastStatementDate"), Qt::ISODate);
        if(date.isValid())
          summary->setReconciliationDate(i18n("Reconciled: %1").arg(KGlobal::locale()->formatDate(date, true)));

      } catch(MyMoneyException *e) {
          qDebug("Unexpected exception in KLedgerViewCheckings::fillSummary");
      }
    }
  }
}

void KLedgerViewCheckings::fillFormStatics(void)
{
  kMyMoneyTransactionFormTable* formTable = m_form->table();

  // clear complete table, but leave the cell widgets while editing
  for(int r = 0; r < formTable->numRows(); ++r) {
    for(int c = 0; c < formTable->numCols(); ++c) {
      if(formTable->cellWidget(r, c) == 0)
        formTable->setText(r, c, " ");
    }
  }

  // common elements
  formTable->setText(MEMO_ROW, MEMO_TXT_COL, i18n("Memo"));
  formTable->setText(DATE_ROW, DATE_TXT_COL, i18n("Date"));
  formTable->setText(AMOUNT_ROW, AMOUNT_TXT_COL, i18n("Amount"));

  switch(transactionType(m_transaction)) {
    case KMyMoneyUtils::Transfer:
      switch( transactionDirection(m_split) ){
        case Credit:
          formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("From"));
          formTable->setText(CATEGORY_ROW, CATEGORY_TXT_COL, i18n("Transfer from"));
          break;
        case UnknownDirection:
        case Debit:
          formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("Pay to"));
          formTable->setText(CATEGORY_ROW, CATEGORY_TXT_COL, i18n("Transfer to"));
          break;
      }
      break;

    default:
      formTable->setText(CATEGORY_ROW, CATEGORY_TXT_COL, i18n("Category"));
      switch( transactionDirection(m_split) ){
        case UnknownDirection:
          switch(m_form->tabBar()->currentTab()) {
            case 1:  // Deposit
              formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("From"));
              break;
            default:
              formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("Pay to"));
              break;
          }
          break;
        case Credit:
          formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("From"));
          break;
        case Debit:
          formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("Pay to"));
          break;
      }
      // For investment transactions we display the activity instead of the payee
      if(KMyMoneyUtils::transactionType(m_transaction) == KMyMoneyUtils::InvestmentTransaction) {
        formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("Activity"));
      }
      break;
  }

  if(showNrField(m_transaction, m_split))
    formTable->setText(NR_ROW, NR_TXT_COL, i18n("No."));
  resizeForm();
}

void KLedgerViewCheckings::fillForm(void)
{
  QTable* formTable = m_form->table();
  m_transactionPtr = transaction(m_register->currentTransactionIndex());

  if(m_transactionPtr != 0) {
    // get my local copy of the selected transaction
    m_transaction = *m_transactionPtr;
    m_split = m_transaction.splitByAccount(accountId());

    fillFormStatics();

    kMyMoneyTransactionFormTableItem* item;

    // setup the fields first
    m_form->tabBar()->blockSignals(true);
    updateTabBar(m_transaction, m_split, true);
    m_form->tabBar()->blockSignals(false);

    // fill in common fields
    formTable->setText(MEMO_ROW, MEMO_DATA_COL, m_split.memo());

    // make sure, that the date is aligned to the right of the cell
    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, KGlobal::locale()->formatDate(m_transaction.postDate(), true));
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    formTable->setItem(DATE_ROW, DATE_DATA_COL, item);

    // collect values
    QString payee;
    if(KMyMoneyUtils::transactionType(m_transaction) == KMyMoneyUtils::InvestmentTransaction) {
      MyMoneySplit split = KMyMoneyUtils::stockSplit(m_transaction);
      payee = MyMoneyFile::instance()->account(split.accountId()).name();
      QString addon;
      if(split.action() == MyMoneySplit::ActionBuyShares) {
        if(split.value().isNegative()) {
          addon = i18n("Sell");
        } else {
          addon = i18n("Buy");
        }
      } else if(split.action() == MyMoneySplit::ActionDividend) {
          addon = i18n("Dividend");
      } else if(split.action() == MyMoneySplit::ActionYield) {
          addon = i18n("Yield");
      }
      if(!addon.isEmpty()) {
        payee += QString(" (%1)").arg(addon);
      }
    } else {
      try {
        payee = MyMoneyFile::instance()->payee(m_split.payeeId()).name();
      } catch (MyMoneyException *e) {
        delete e;
        payee = " ";
      }
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
            category = i18n("Split transaction (category replacement)", "Split transaction");
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

    formTable->setText(PAYEE_ROW, PAYEE_DATA_COL, payee);
    formTable->setText(CATEGORY_ROW, CATEGORY_DATA_COL, category);
    switch( actionTab(m_transaction, m_split) ){
      case 0:      // check
      case 4:      // ATM
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, m_split.number());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(NR_ROW, NR_DATA_COL, item);
        break;

      default:
        break;
    }

    MyMoneyMoney amount = m_split.value(m_transaction.commodity(), m_account.currencyId()).abs();

    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never,
                 m_split.value(m_transaction.commodity(), m_account.currencyId()).abs().formatMoney());
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    formTable->setItem(AMOUNT_ROW, AMOUNT_DATA_COL, item);

    m_form->newButton()->setEnabled(true);
    m_form->editButton()->setEnabled(true);
    enableOkButton(false);
    enableCancelButton(false);
    enableMoreButton(true);
  } else {
    m_transaction = MyMoneyTransaction();
    m_split = MyMoneySplit();
    m_split.setAccountId(accountId());
    m_split.setAction(m_action);

    m_transaction.addSplit(m_split);
    m_transaction.setCommodity(m_account.currencyId());

    fillFormStatics();

    m_form->newButton()->setEnabled(true);
    m_form->editButton()->setEnabled(false);
    enableOkButton(false);
    enableCancelButton(false);
    enableMoreButton(false);
  }

  // make sure, fields can use all available space
  // by spanning items over multiple cells if necessary
  QTableItem* item;
  // payee
  item = formTable->item(PAYEE_ROW, PAYEE_DATA_COL);
  if(item)
    item->setSpan(PAYEE_DATA_COL, 2);
  // category
  item = formTable->item(CATEGORY_ROW, CATEGORY_DATA_COL);
  if(item)
    item->setSpan(CATEGORY_DATA_COL, 2);
  // memo
  item = formTable->item(MEMO_ROW, MEMO_DATA_COL);
  if(item)
    item->setSpan(MEMO_DATA_COL, 2);
}

void KLedgerViewCheckings::createEditWidgets(void)
{
  if(!m_editPayee) {
    m_editPayee = new kMyMoneyPayee(0, "Payee");
    connect(m_editPayee, SIGNAL(newPayee(const QString&)), this, SLOT(slotNewPayee(const QString&)));
    connect(m_editPayee, SIGNAL(payeeChanged(const QString&)), this, SLOT(slotPayeeChanged(const QString&)));
  }

  if(!m_editCategory) {
    m_editCategory = new kMyMoneyCategory(0, "Category");
    connect(m_editCategory, SIGNAL(categoryChanged(const QCString&)), this, SLOT(slotCategoryChanged(const QCString&)));
  }
  if(!m_editMemo) {
    m_editMemo = new kMyMoneyLineEdit(0, "Memo", AlignLeft|AlignVCenter);
    connect(m_editMemo, SIGNAL(lineChanged(const QString&)), this, SLOT(slotMemoChanged(const QString&)));
  }

  if(!m_editAmount) {
    m_editAmount = new kMyMoneyEdit(0, "Amount");
    connect(m_editAmount, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAmountChanged(const QString&)));
  }

  if(!m_editDate) {
    m_editDate = new kMyMoneyDateInput(0, "Date");
    connect(m_editDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged(const QDate&)));
  }

  if(!m_editNr) {
    m_editNr = new kMyMoneyLineEdit(0, "Nr");
    connect(m_editNr, SIGNAL(lineChanged(const QString&)), this, SLOT(slotNrChanged(const QString&)));
  }

  if(!m_editSplit) {
    m_editSplit = new KPushButton(i18n("Split"), 0, "Split");
    connect(m_editSplit, SIGNAL(clicked()), this, SLOT(slotOpenSplitDialog()));
  }

  if(!m_editPayment) {
    m_editPayment = new kMyMoneyEdit(0, "Payment");
    connect(m_editPayment, SIGNAL(valueChanged(const QString&)), this, SLOT(slotPaymentChanged(const QString&)));
  }

  if(!m_editDeposit) {
    m_editDeposit = new kMyMoneyEdit(0, "Deposit");
    connect(m_editDeposit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotDepositChanged(const QString&)));
  }

  if(!m_editType) {
    m_editType = new kMyMoneyCombo(0, "Type");
    m_editType->setFocusPolicy(QWidget::StrongFocus);
    connect(m_editType, SIGNAL(selectionChanged(int)), this, SLOT(slotTypeSelected(int)));

    // need to load the combo box with the required values?
    // copy them from the tab's in the transaction form
    for(int i = 0; i < m_form->tabBar()->count(); ++i) {
      QString txt = m_form->tabBar()->tabAt(i)->text();
      txt = txt.replace(QRegExp("&"), "");
      m_editType->insertItem(txt);
    }
  }
}

void KLedgerViewCheckings::reloadEditWidgets(const MyMoneyTransaction& t)
{
  MyMoneyMoney amount;
  QString payee;

  m_transaction = t;
  m_split = m_transaction.splitByAccount(accountId());
  m_action = m_split.action();
  amount = m_split.value(m_transaction.commodity(), m_account.currencyId());

  if(m_editCategory)
    disconnect(m_editCategory, SIGNAL(signalFocusIn()), this, SLOT(slotOpenSplitDialog()));

  try {
    if(!m_split.payeeId().isEmpty())
      payee = MyMoneyFile::instance()->payee(m_split.payeeId()).name();

    MyMoneySplit s;
    MyMoneyAccount acc;
    if(m_transaction.isLoanPayment()) {
      // Make sure that we do not allow the user to modify the category
      // directly, but only through the split edit dialog
      if(m_editCategory) {
        connect(m_editCategory, SIGNAL(signalFocusIn()), this, SLOT(slotOpenSplitDialog()));
        m_editCategory->loadText(i18n("Loan payment"));
      }
    } else {
      switch(t.splitCount()) {
        case 2:
          s = t.splitByAccount(accountId(), false);
          if(m_editCategory)
            m_editCategory->loadAccount(s.accountId());
          break;

        case 1:
          if(m_editCategory)
            m_editCategory->loadAccount(QCString());
          break;

        default:
          // Make sure that we do not allow the user to modify the category
          // directly, but only through the split edit dialog
          if(m_editCategory) {
            connect(m_editCategory, SIGNAL(signalFocusIn()), this, SLOT(slotOpenSplitDialog()));
            m_editCategory->loadText(i18n("Split transaction (category replacement)", "Split transaction"));
          }
          break;
      }
    }
  } catch(MyMoneyException *e) {
    qDebug("Exception '%s' thrown in %s, line %ld caught in KLedgerViewCheckings::reloadEditWidgets():%d",
      e->what().latin1(), e->file().latin1(), e->line(), __LINE__-2);
    delete e;
  }

  if(m_editPayee)
    m_editPayee->loadText(payee);
  if(m_editMemo)
    m_editMemo->loadText(m_split.memo());
  if(m_editAmount)
    m_editAmount->loadText(amount.abs().formatMoney());
  if(m_editDate && m_transactionPtr)
    m_editDate->loadDate(m_transactionPtr->postDate());
  if(m_editNr)
    m_editNr->loadText(m_split.number());

  if(m_editPayment && m_editDeposit) {
    if(amount.isNegative()) {
      m_editPayment->loadText((-amount).formatMoney());
      m_editDeposit->loadText(QString());
    } else {
      m_editPayment->loadText(QString());
      m_editDeposit->loadText(amount.formatMoney());
    }
  }
}

void KLedgerViewCheckings::autoIncCheckNumber(void)
{
  if(m_editNr && m_editNr->text().isEmpty() && m_action == MyMoneySplit::ActionCheck) {
    KConfig* kconfig = KGlobal::config();
    kconfig->setGroup("General Options");
    if(kconfig->readBoolEntry("AutoIncCheckNumber", false)) {
      unsigned64 no = MyMoneyFile::instance()->highestCheckNo(m_account.id()).toULongLong();
      m_split.setNumber(QString::number(no+1));
      m_transaction.modifySplit(m_split);
      m_editNr->loadText(m_split.number());
    }
  }
}

void KLedgerViewCheckings::loadEditWidgets(void)
{
  if(m_transactionPtr != 0) {
    reloadEditWidgets(*m_transactionPtr);
  } else {
    m_editDate->setDate(m_lastPostDate);
    // transType = m_form->tabBar()->currentTab();

    try {
      if(m_editNr) {
        // if the CopyTypeToNr switch is set, we copy the m_action string
        // into the nr field of this transaction.
        KConfig *config = KGlobal::config();
        config->setGroup("General Options");
        if(config->readBoolEntry("CopyTypeToNr", false) == true) {
          m_split.setNumber(m_action);
          m_transaction.modifySplit(m_split);
          m_editNr->loadText(m_split.number());
        } else {
          autoIncCheckNumber();
        }
      }
    } catch(MyMoneyException *e) {
      qDebug("Exception '%s' thrown in %s, line %ld caught in KLedgerViewCheckings::loadEditWidgets()",
        e->what().latin1(), e->file().latin1(), e->line());
      delete e;
    }
  }
}

const bool KLedgerViewCheckings::showNrField(const MyMoneyTransaction& t, const MyMoneySplit& s) const
{
  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  if(config->readBoolEntry("AlwaysShowNrField", false) == true)
    return true;

  int tab = actionTab(t,s);
  return (tab == 0) || (tab == 4);
}

QWidget* KLedgerViewCheckings::arrangeEditWidgetsInForm(void)
{
  // make sure, that the category has an associated button
  QTableItem* item;
  kMyMoneyTransactionFormTable* table = m_form->table();

  // category
  item = table->item(CATEGORY_ROW, CATEGORY_DATA_COL);
  item->setSpan(CATEGORY_DATA_COL, 1);

  // make sure, we're using the right palette
  QPalette palette = m_register->palette();
  m_editPayee->setPalette(palette);
  m_editCategory->setPalette(palette);
  m_editMemo->setPalette(palette);
  m_editAmount->setPalette(palette);
  m_editDate->setPalette(palette);
  if(m_editNr)
    m_editNr->setPalette(palette);

  // delete widgets that are used for the register edit mode only
  delete m_editPayment;
  delete m_editDeposit;
  delete m_editType;

  table->clearEditable();

  setFormCellWidget(MEMO_ROW, MEMO_DATA_COL, m_editMemo);
  setFormCellWidget(DATE_ROW, DATE_DATA_COL, m_editDate);
  setFormCellWidget(AMOUNT_ROW, AMOUNT_DATA_COL, m_editAmount);

  table->setEditable(PAYEE_ROW, PAYEE_DATA_COL);
  table->setEditable(CATEGORY_ROW, CATEGORY_DATA_COL);
  table->setEditable(MEMO_ROW, MEMO_DATA_COL);
  table->setEditable(DATE_ROW, DATE_DATA_COL);
  table->setEditable(AMOUNT_ROW, AMOUNT_DATA_COL);

  if(showNrField(m_transaction, m_split)) {
    table->setEditable(NR_ROW, NR_DATA_COL);
    setFormCellWidget(NR_ROW, NR_DATA_COL, m_editNr);
  } else {
    if(m_editNr) {
      table->clearCellWidget(NR_ROW, NR_DATA_COL);
      delete m_editNr;
    }
  }

  setFormCellWidget(PAYEE_ROW, PAYEE_DATA_COL, m_editPayee);
  setFormCellWidget(CATEGORY_ROW, CATEGORY_DATA_COL, m_editCategory);
  setFormCellWidget(CATEGORY_ROW, CATEGORY_DATA_COL+1, m_editSplit);

  // now setup the tab order
  m_tabOrderWidgets.clear();
  addToTabOrder(m_form->enterButton());
  addToTabOrder(m_form->cancelButton());
  addToTabOrder(m_form->moreButton());
  addToTabOrder(m_editPayee);
  addToTabOrder(m_editCategory);
  addToTabOrder(m_editSplit);
  addToTabOrder(m_editMemo);
  addToTabOrder(m_editNr);
  addToTabOrder(m_editDate);
  addToTabOrder(m_editAmount);

  return m_editPayee;
}

QWidget* KLedgerViewCheckings::arrangeEditWidgetsInRegister(void)
{
  QWidget* focusWidget = m_editDate;
  int firstRow = m_register->currentTransactionIndex() * m_register->rpt();

  delete m_editAmount;

  // place edit widgets in the register
  if(m_editNr)
    setRegisterCellWidget(firstRow, 0, m_editNr);
  setRegisterCellWidget(firstRow, 1, m_editDate);
  setRegisterCellWidget(firstRow+1, 1, m_editType);
  setRegisterCellWidget(firstRow, 2, m_editPayee);
  setRegisterCellWidget(firstRow+1, 2, m_editCategory);
  setRegisterCellWidget(firstRow+2, 2, m_editMemo);
  setRegisterCellWidget(firstRow, 4, m_editPayment);
  setRegisterCellWidget(firstRow, 5, m_editDeposit);

  // place buttons
  setRegisterCellWidget(firstRow+2, 1, m_registerButtonFrame);

  // now setup the tab order
  m_tabOrderWidgets.clear();

  if(m_editNr) {
    addToTabOrder(m_editNr);
    focusWidget = m_editNr;
  }

  addToTabOrder(m_editDate);
  addToTabOrder(m_editType);
  addToTabOrder(m_editPayee);
  addToTabOrder(m_editCategory);
  addToTabOrder(m_editMemo);
  addToTabOrder(m_editPayment);
  addToTabOrder(m_editDeposit);
  addToTabOrder(m_registerEnterButton);
  addToTabOrder(m_registerCancelButton);
  addToTabOrder(m_registerMoreButton);

  if(m_editSplit) {
    delete m_editSplit;
  }
  return focusWidget;
}

void KLedgerViewCheckings::destroyWidgets(void)
{
  KLedgerView::destroyWidgets();

  m_form->table()->clearEditable();
  m_form->tabBar()->setEnabled(true);

  // make sure, category can use all available space (incl. split button)
  QTableItem* item;
  item = m_form->table()->item(CATEGORY_ROW, CATEGORY_DATA_COL);
  item->setSpan(CATEGORY_DATA_COL, 2);
}

bool KLedgerViewCheckings::focusNextPrevChild(bool next)
{
  return KLedgerView::focusNextPrevChild(next);
}

void KLedgerViewCheckings::slotReconciliation(void)
{
  slotCancelEdit();

/*
  MyMoneyMoney lastReconciledBalance(m_account.value("lastReconciledBalance"));
  if(lastReconciledBalance == 0)
    lastReconciledBalance = MyMoneyMoney(m_account.value("lastStatementBalance"));

  MyMoneyMoney statementBalance(m_account.value("statementBalance"));

  QDate statementDate;
  if(!m_account.value("statementDate").isEmpty())
    statementDate = QDate::fromString(m_account.value("statementDate"), Qt::ISODate);

  if(!statementDate.isValid())
    statementDate = QDate::currentDate();
*/
  KEndingBalanceDlg dlg(m_account);

  if(dlg.exec()) {
    QCString transactionId;

    // check if the user requests us to create interest
    // or charge transactions.
    MyMoneyTransaction ti = dlg.interestTransaction();
    MyMoneyTransaction tc = dlg.chargeTransaction();
    if(ti != MyMoneyTransaction()) {
      try {
        MyMoneyFile::instance()->addTransaction(ti);
      } catch(MyMoneyException *e) {
        qWarning("interest transaction not stored: '%s'", e->what().data());
        delete e;
      }
    }
    if(tc != MyMoneyTransaction()) {
      try {
        MyMoneyFile::instance()->addTransaction(tc);
      } catch(MyMoneyException *e) {
        qWarning("charge transaction not stored: '%s'", e->what().data());
        delete e;
      }
    }

    m_inReconciliation = true;
    m_summaryLine->hide();
    m_reconciliationFrame->show();
    m_transactionFormActive = false;

    m_prevBalance = dlg.previousBalance();
    m_endingBalance = dlg.endingBalance();
    m_endingDate = dlg.statementDate();

    refreshView();
    fillReconcileData();

    // allow SPACE to be used to toggle the clear state
    connect(m_register, SIGNAL(signalSpace()), this, SLOT(slotToggleClearFlag()));

    m_transactionCheckBox->setChecked(false);
    slotShowTransactionForm(false);
    connect(m_transactionCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotShowTransactionForm(bool)));
  }
}

void KLedgerViewCheckings::fillReconcileData(void)
{
  MyMoneyMoney cleared(m_prevBalance);
  MyMoneyMoney endingBalance(m_endingBalance);

  for(unsigned int i = 0; i < m_transactionPtrVector.size(); ++i) {
    MyMoneyTransaction* t = m_transactionPtrVector[i];
    MyMoneySplit sp = t->splitByAccount(m_account.id());
    if(sp.reconcileFlag() == MyMoneySplit::Cleared)
      cleared += sp.value(t->commodity(), m_account.currencyId());
  }
  // We need to invert all values for liability accounts
  if(m_account.accountGroup() == MyMoneyAccount::Liability) {
    cleared = -cleared;
    endingBalance = -endingBalance;
  }
  m_clearedLabel->setText(i18n("Cleared: %1").arg(cleared.formatMoney()));
  m_statementLabel->setText(i18n("Statement: %1").arg(endingBalance.formatMoney()));
  m_differenceLabel->setText(i18n("Difference: %1").arg((cleared - endingBalance).formatMoney()));
}

void KLedgerViewCheckings::slotRegisterClicked(int row, int col, int button, const QPoint &mousePos)
{
  if(!m_account.id().isEmpty()) {
    KLedgerView::slotRegisterClicked(row, col, button, mousePos);

    if(m_inReconciliation == true
    && col == 3) {    // reconcileFlag column
      slotToggleClearFlag();
    }
  }
}

void KLedgerViewCheckings::slotConfigureMoreMenu(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  int splitEditId = m_moreMenu->idAt(1);
  int gotoPayeeId = m_moreMenu->idAt(2);
  int moveAccountId = m_moreMenu->idAt(6);

  // we need to disconnect the slots, otherwise we cannot connect unconditionally
  m_moreMenu->disconnectItem(splitEditId, this, SLOT(slotGotoOtherSideOfTransfer()));
  m_moreMenu->disconnectItem(splitEditId, this, SLOT(slotOpenSplitDialog()));

  m_moreMenu->changeItem(splitEditId, i18n("Edit splits ..."));
  m_moreMenu->connectItem(splitEditId, this, SLOT(slotOpenSplitDialog()));
  if(m_transactionPtr != 0) {
    if(!m_split.payeeId().isEmpty() && !m_split.accountId().isEmpty() && !m_transaction.id().isEmpty()) {
      MyMoneyPayee payee = file->payee(m_split.payeeId());
      m_moreMenu->changeItem(gotoPayeeId, i18n("Goto '%1'").arg(payee.name()));
      m_moreMenu->setItemEnabled(gotoPayeeId, true);
    } else {
      m_moreMenu->changeItem(gotoPayeeId, i18n("Goto payer/receiver"));
      m_moreMenu->setItemEnabled(gotoPayeeId, false);
    }

    int type = transactionType(*m_transactionPtr);
    if ( ( type == KMyMoneyUtils::Transfer) || ( type == KMyMoneyUtils::InvestmentTransaction) ) {
      QString dest;
      try {
        MyMoneySplit split = m_transaction.splitByAccount(m_account.id(), false);
        MyMoneyAccount acc = file->account(split.accountId());
        dest = acc.name();
      } catch(MyMoneyException *e) {
        delete e;
        dest = "opposite account";
      }
      m_moreMenu->changeItem(splitEditId, i18n("Goto '%1'").arg(dest));
      m_moreMenu->connectItem(splitEditId, this, SLOT(slotGotoOtherSideOfTransfer()));
      m_moreMenu->disconnectItem(splitEditId, this, SLOT(slotOpenSplitDialog()));
    }
    loadAccountList(m_accountListMoreMenu);
    m_moreMenu->setItemEnabled(moveAccountId, true);
    m_moreMenu->setItemEnabled(splitEditId, true);
  } else {
    m_moreMenu->setItemEnabled(moveAccountId, false);
    m_moreMenu->setItemEnabled(splitEditId, isEditMode());
  }
}

void KLedgerViewCheckings::slotConfigureContextMenu(void)
{
  int splitEditId = m_contextMenu->idAt(2);
  int gotoPayeeId = m_contextMenu->idAt(3);
  int deleteId = m_contextMenu->idAt(9);
  int moveAccountId = m_contextMenu->idAt(7);

  MyMoneyFile* file = MyMoneyFile::instance();

  // we need to disconnect the slots, otherwise we cannot connect unconditionally
  m_contextMenu->disconnectItem(splitEditId, this, SLOT(slotGotoOtherSideOfTransfer()));
  m_contextMenu->disconnectItem(splitEditId, this, SLOT(slotOpenSplitDialog()));

  m_contextMenu->changeItem(splitEditId, i18n("Edit splits ..."));
  m_contextMenu->connectItem(splitEditId, this, SLOT(slotOpenSplitDialog()));
  if(m_transactionPtr != 0) {
    if(!m_split.payeeId().isEmpty() && !m_split.accountId().isEmpty() && !m_transaction.id().isEmpty()) {
      MyMoneyPayee payee = file->payee(m_split.payeeId());
      QString name = payee.name();
      name.replace(QRegExp("&(?!&)"), "&&");
      m_contextMenu->changeItem(gotoPayeeId, i18n("Goto '%1'").arg(name));
      m_contextMenu->setItemEnabled(gotoPayeeId, true);
    } else {
      m_contextMenu->changeItem(gotoPayeeId, i18n("Goto payer/receiver"));
      m_contextMenu->setItemEnabled(gotoPayeeId, false);
    }
    int type = transactionType(*m_transactionPtr);
    if ( ( type == KMyMoneyUtils::Transfer) || ( type == KMyMoneyUtils::InvestmentTransaction) ) {
      QString dest;
      try {
        MyMoneySplit split = m_transaction.splitByAccount(m_account.id(), false);
        MyMoneyAccount acc = MyMoneyFile::instance()->account(split.accountId());
        dest = acc.name();
      } catch(MyMoneyException *e) {
        delete e;
        dest = i18n("opposite account");
      }
      m_contextMenu->changeItem(splitEditId, i18n("Goto '%1'").arg(dest));
      m_contextMenu->connectItem(splitEditId, this, SLOT(slotGotoOtherSideOfTransfer()));
      m_contextMenu->disconnectItem(splitEditId, this, SLOT(slotOpenSplitDialog()));
    }
    loadAccountList(m_accountListContextMenu);

    m_contextMenu->setItemEnabled(moveAccountId, true);
    m_contextMenu->setItemEnabled(splitEditId, true);
    m_contextMenu->setItemEnabled(deleteId, true);
  } else {
    m_contextMenu->setItemEnabled(moveAccountId, false);
    m_contextMenu->setItemEnabled(splitEditId, false);
    m_contextMenu->setItemEnabled(deleteId, false);
  }
}

void KLedgerViewCheckings::slotToggleClearFlag(void)
{
  if(m_transactionPtr != 0) {
    MyMoneySplit sp = m_transactionPtr->splitByAccount(m_account.id());
    switch(sp.reconcileFlag()) {
      case MyMoneySplit::NotReconciled:
        sp.setReconcileFlag(MyMoneySplit::Cleared);
        break;
      default:
        sp.setReconcileFlag(MyMoneySplit::NotReconciled);
        break;
    }
    try {
      m_transactionPtr->modifySplit(sp);
      MyMoneyFile::instance()->modifyTransaction(*m_transactionPtr);

    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception when setting reconcile flag");
      delete e;
    }

    // update the display is partially done through the observer
    // on the account in KLedgerView::update(). We add what's missing.
    fillReconcileData();
  }
}

void KLedgerViewCheckings::slotPostponeReconciliation(void)
{
  // force any pending
  slotCancelEdit();

  if(m_inReconciliation == true) {
    m_account.setValue("lastReconciledBalance", m_prevBalance.toString());
    m_account.setValue("statementBalance", m_endingBalance.toString());
    m_account.setValue("statementDate", m_endingDate.toString(Qt::ISODate));

    try {
      MyMoneyFile::instance()->modifyAccount(m_account);
    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception when setting last reconcile info into account");
      delete e;
    }

    endReconciliation();
  }
}

void KLedgerViewCheckings::endReconciliation(void)
{
  QCString transactionId;
  KConfig *config = KGlobal::config();
  config->setGroup("General Options");

  m_inReconciliation = false;
  m_reconciliationFrame->hide();
  m_summaryLine->show();
  m_transactionFormActive = config->readBoolEntry("TransactionForm", true);

  refreshView();

  disconnect(m_register, SIGNAL(signalSpace()), this, SLOT(slotToggleClearFlag()));
  disconnect(m_transactionCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotShowTransactionForm(bool)));
}

void KLedgerViewCheckings::slotEndReconciliation(void)
{
  // force any pending
  slotCancelEdit();

  if(m_inReconciliation == true) {
    m_account.setValue("lastStatementBalance", m_endingBalance.toString());
    m_account.setValue("lastStatementDate", m_endingDate.toString(Qt::ISODate));

    m_account.deletePair("lastReconciledBalance");
    m_account.deletePair("statementBalance");
    m_account.deletePair("statementDate");

    MyMoneyFile::instance()->suspendNotify(true);
    try {
      MyMoneyFile::instance()->modifyAccount(m_account);
      MyMoneyTransactionFilter filter(m_account.id());
      QValueList<MyMoneyTransaction> list = MyMoneyFile::instance()->transactionList(filter);
      QValueList<MyMoneyTransaction>::Iterator it;

      for(it = list.begin(); it != list.end(); ++it) {
        MyMoneySplit sp = (*it).splitByAccount(m_account.id());
        if(sp.reconcileFlag() == MyMoneySplit::Cleared) {
          sp.setReconcileFlag(MyMoneySplit::Reconciled);
          sp.setReconcileDate(m_endingDate);
          (*it).modifySplit(sp);
          MyMoneyFile::instance()->modifyTransaction(*it);
        }
      }
    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception when setting cleared to reconcile");
      delete e;
    }
    MyMoneyFile::instance()->suspendNotify(false);
    endReconciliation();
  }
}

void KLedgerViewCheckings::slotOpenSplitDialog(void)
{
  // if we're not in edit mode, then start editing
  if(!isEditMode())
    slotStartEdit();

  // but the user may have decided not to edit, so we bail out
  if(!isEditMode())
    return;

  // force focus change to update all data
  if(m_editSplit)
    m_editSplit->setFocus();
  else if(m_registerMoreButton)
    m_registerMoreButton->setFocus();

  bool isValidAmount = false;

  if(m_transactionFormActive) {
    isValidAmount = m_editAmount->text().length() != 0;
  } else {
    if ( (m_editPayment && m_editPayment->text().length() != 0)
    || (m_editDeposit && m_editDeposit->text().length() != 0)) {
      isValidAmount = true;
    }
  }
  KSplitTransactionDlg* dlg = new KSplitTransactionDlg(m_transaction,
                                                       m_account,
                                                       isValidAmount,
                                                       transactionDirection(m_split) == Credit,
                                                       0,
                                                       this);

  if(dlg->exec()) {
    reloadEditWidgets(dlg->transaction());
  }

  delete dlg;

  if ( m_editMemo )
    m_editMemo->setFocus();
}

void KLedgerViewCheckings::slotAccountDetail(void)
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

void KLedgerViewCheckings::slotPayeeSelected(void)
{
  slotCancelEdit();
  if(!m_split.payeeId().isEmpty() && !m_split.accountId().isEmpty() && !m_transaction.id().isEmpty())
    emit payeeSelected(m_split.payeeId(), m_split.accountId(), m_transaction.id());
}

bool KLedgerViewCheckings::eventFilter( QObject *o, QEvent *e )
{
  return KLedgerView::eventFilter(o, e);
}

int KLedgerViewCheckings::transactionType(const MyMoneyTransaction& t) const
{
  int rc = KMyMoneyUtils::transactionType(t);
  if(rc == KMyMoneyUtils::Unknown) {
    if(m_transactionFormActive) {
      switch(m_form->tabBar()->currentTab()) {
        case 2:    // Transfer
          rc = KMyMoneyUtils::Transfer;
          break;
        default:
          rc = KMyMoneyUtils::Normal;
          break;
      }
    } else {
      if(m_editType) {
        switch(m_editType->currentItem()) {
          case 2:    // Transfer
            rc = KMyMoneyUtils::Transfer;
            break;
          default:
            rc = KMyMoneyUtils::Normal;
            break;
        }
      } else
        rc = KMyMoneyUtils::Unknown;
    }
  }
  return rc;
}

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef PAYEE_ROW
#undef CATEGORY_ROW
#undef MEMO_ROW
#undef NR_ROW
#undef DATE_ROW
#undef AMOUNT_ROW

#undef PAYEE_TXT_COL
#undef PAYEE_DATA_COL
#undef CATEGORY_TXT_COL
#undef CATEGORY_DATA_COL
#undef MEMO_TXT_COL
#undef MEMO_DATA_COL
#undef NR_TXT_COL
#undef NR_DATA_COL
#undef DATE_TXT_COL
#undef DATE_DATA_COL
#undef AMOUNT_TXT_COL
#undef AMOUNT_DATA_COL

#include "kledgerviewcheckings.moc"
