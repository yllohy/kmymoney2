/***************************************************************************
                          kinvestmentview.cpp  -  description
                             -------------------
    begin                : Tue Jan 29 2002
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
#include <qtabwidget.h>
#include <qheader.h>
#include <qlistbox.h>
#include <qcursor.h>
#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"
#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klistview.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyutils.h"
#include "../mymoney/mymoneysecurity.h"
#include "../mymoney/mymoneytransaction.h"
#include "../mymoney/mymoneyinvesttransaction.h"
#include "../mymoney/mymoneyaccount.h"

#include "../dialogs/knewequityentrydlg.h"
#include "../dialogs/kupdatestockpricedlg.h"
#include "../dialogs/keditequityentrydlg.h"
#include "../dialogs/knewaccountdlg.h"
#include "../dialogs/kequitypriceupdatedlg.h"
#include "../dialogs/knewinvestmentwizard.h"
#include "../dialogs/kcurrencycalculator.h"

#include "../widgets/kmymoneyaccountcombo.h"
#include "../widgets/kmymoneycurrencyselector.h"

#include "../kmymoney2.h"

#include "kinvestmentview.h"
#include "kinvestmentlistitem.h"
#include "kledgerviewinvestments.h"


KInvestmentView::KInvestmentView(QWidget *parent, const char *name) :
  kInvestmentViewDecl(parent,name),
  m_popMenu(0)
{
  // FIXME: check if we really want to remove the margin
  // kInvestmentViewDeclLayout->setMargin(0);

  initSummaryTab();
  initTransactionTab();

  // never show a horizontal scroll bar
  // investmentTable->setHScrollBarMode(QScrollView::AlwaysOff);

  //no sorting yet...
  // investmentTable->setSorting(-1);

  connect(investmentTable, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));

  //connects the signal when a radio button is checked.
 // connect(m_btnGroupView, SIGNAL(clicked(int)), this, SLOT(slotViewChanged(int)));

  //set the summary button to be true.
 // btnSummary->setChecked(TRUE);

  connect(m_accountComboBox, SIGNAL(accountSelected(const QCString&)),
    this, SLOT(slotSelectAccount(const QCString&)));

  connect(m_tab, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotTabSelected(QWidget*)));
 //const bool KInvestmentView::slotSelectAccount(const QCString& id, const bool reconciliation)

  connect(investmentTable, SIGNAL(doubleClicked(QListViewItem*,const QPoint&, int)), this, SLOT(slotItemDoubleClicked(QListViewItem*,const QPoint&, int)));

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccount, this);
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccountHierarchy, this);

  // we temporarily need this to keep the linker happy
  KNewEquityEntryDlg *pDlg = new KNewEquityEntryDlg(this);
  delete pDlg;
}

KInvestmentView::~KInvestmentView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccountHierarchy, this);
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccount, this);
}

void KInvestmentView::initSummaryTab(void)
{
  investmentTable->setRootIsDecorated(false);
  investmentTable->setColumnText(0, i18n("Symbol"));
  investmentTable->addColumn(i18n("Name"));
  investmentTable->addColumn(i18n("Symbol"));
  investmentTable->addColumn(i18n("Value"));
  investmentTable->addColumn(i18n("Quantity"));
  investmentTable->addColumn(i18n("Price"));
#if 0
  investmentTable->addColumn(i18n("Cost Basis"));
  investmentTable->addColumn(i18n("$ Gain"));
  investmentTable->addColumn(i18n("1 Week %"));
  investmentTable->addColumn(i18n("4 Weeks %"));
  investmentTable->addColumn(i18n("3 Months %"));
  investmentTable->addColumn(i18n("YTD %"));
#endif

  investmentTable->setMultiSelection(false);
  investmentTable->setColumnWidthMode(0, QListView::Maximum);
  investmentTable->header()->setResizeEnabled(true);
  investmentTable->setAllColumnsShowFocus(true);
}

void KInvestmentView::initTransactionTab(void)
{
  QHBoxLayout* m_TransactionTabLayout = new QHBoxLayout( m_transactionTab, 0, 2, "m_summaryTabLayout");

  m_ledgerView = new KLedgerViewInvestments(m_transactionTab, "ledgerview");
  m_TransactionTabLayout->addWidget(m_ledgerView);
}

void KInvestmentView::updateDisplay()
{
  //remove all current items
  investmentTable->clear();

  if(m_account.id().isEmpty())
    return;

  MyMoneyFile* file = MyMoneyFile::instance();
  m_account = file->account(m_account.id());
  QCStringList securities = m_account.accountList();

  for(QCStringList::ConstIterator it = securities.begin(); it != securities.end(); ++it) {
    MyMoneyAccount acc = file->account(*it);
    new KInvestmentListItem(investmentTable, acc);
  }
}

void KInvestmentView::slotItemDoubleClicked(QListViewItem* pItem, const QPoint& /*pos*/, int /*c*/)
{
  KInvestmentListItem *pInvestListItem = dynamic_cast<KInvestmentListItem*>(pItem);
  if(pInvestListItem)
  {
    KNewInvestmentWizard dlg(pInvestListItem->account(), this, "InvestmentWizard");
    if(dlg.exec() == QDialog::Accepted) {
      dlg.createObjects(m_account.id());
    }
  }
}


void KInvestmentView::slotNewInvestment(void)
{
  KNewInvestmentWizard dlg(this, "InvestmentWizard");
  if(dlg.exec() == QDialog::Accepted) {
    dlg.createObjects(m_account.id());
  }
}

void KInvestmentView::slotRemoveInvestment()
{
  KInvestmentListItem *pItem = dynamic_cast<KInvestmentListItem*>(investmentTable->selectedItem());
  if(pItem)
  {
    if(KMessageBox::questionYesNo(this, i18n("Do you really want to delete the selected investment?"), i18n("Delete investment"), KStdGuiItem::yes(), KStdGuiItem::no(), "DeleteInvestment") == KMessageBox::Yes) {
      try {
        MyMoneyFile::instance()->removeAccount(pItem->account());
      } catch(MyMoneyException *e) {
        qDebug("Cannot delete investment: %s", e->what().latin1());
        delete e;
      }
    }
  }
}

void KInvestmentView::slotEditInvestment()
{
  slotItemDoubleClicked(investmentTable->selectedItem(), QPoint(), 0);
}

void KInvestmentView::slotUpdatePrice()
{
  KInvestmentListItem *pItem = dynamic_cast<KInvestmentListItem*>(investmentTable->selectedItem());
  if(pItem)
  {
    KEquityPriceUpdateDlg dlg(this, pItem->securityId());
    dlg.exec();
  }
}

void KInvestmentView::slotAddPrice()
{
  KInvestmentListItem *pItem = dynamic_cast<KInvestmentListItem*>(investmentTable->selectedItem());
  if(pItem) {
    KUpdateStockPriceDlg dlg(this);
    dlg.m_commodity->setSecurity(MyMoneyFile::instance()->security(pItem->securityId()));
    dlg.m_currency->setSecurity(pItem->tradingCurrency());
    if(dlg.exec()) {
      KCurrencyCalculator calc(dlg.m_commodity->security(), dlg.m_currency->security(), MyMoneyMoney(1,1), MyMoneyMoney(1,1), dlg.date());

      calc.m_updateButton->setChecked(true);
      calc.m_updateButton->hide();
      calc.exec();

      MyMoneyPrice price(dlg.m_commodity->security().id(), dlg.m_currency->security().id(), dlg.date(), calc.price(), i18n("User"));
      try {
        MyMoneyFile::instance()->addPrice(price);
      } catch(MyMoneyException *e) {
        delete e;
        qDebug("Cannot add price entry");
      }
    }
  }
}

void KInvestmentView::slotListRightMouse(QListViewItem* item, const QPoint& /*point*/, int /*x*/)
{
  int newId, editId, updateId, addId, delId;

  // setup the context menu
  KIconLoader *kiconloader = KGlobal::iconLoader();
  m_popMenu = new KPopupMenu(this);
  m_popMenu->insertTitle(kiconloader->loadIcon("transaction", KIcon::MainToolbar), i18n("Investment Options"));
  newId = m_popMenu->insertItem(kiconloader->loadIcon("file_new", KIcon::Small), i18n("New ..."), this, SLOT(slotNewInvestment()));
  editId = m_popMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("Edit ..."), this, SLOT(slotEditInvestment()));
  addId = m_popMenu->insertItem(i18n("Manual Price Update..."), this, SLOT(slotAddPrice()));
  updateId = m_popMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("On-line Price Update ..."), this, SLOT(slotUpdatePrice()));
  delId = m_popMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small),
                        i18n("Delete ..."),
                        this, SLOT(slotRemoveInvestment()));

  if(!item) {
    m_popMenu->setItemEnabled(editId, false);
    m_popMenu->setItemEnabled(updateId, false);
    m_popMenu->setItemEnabled(addId, false);
    m_popMenu->setItemEnabled(delId, false);
  } else {
    m_popMenu->setItemEnabled(updateId, false);
    KInvestmentListItem *pItem = dynamic_cast<KInvestmentListItem*>(item);
    if(pItem) {
      try {
        MyMoneySecurity security = MyMoneyFile::instance()->security(pItem->securityId());
        if(!security.value("kmm-online-source").isEmpty())
          m_popMenu->setItemEnabled(updateId, true);
        m_popMenu->setItemEnabled(delId, MyMoneyFile::instance()->transactionCount(pItem->account().id()) == 0);

      } catch(MyMoneyException *e) {
        qDebug("Caught exception in KInvestmentView::slotListRightMouse thrown in %s(%ld)): %s", e->file().data(), e->line(), e->what().data());
        delete e;
      }
    }
  }
  if(m_account.accountType() != MyMoneyAccount::Investment)
    m_popMenu->setItemEnabled(newId, false);

  m_popMenu->exec(QCursor::pos());
}

void KInvestmentView::slotTabSelected(QWidget *pWidget)
{
  // make sure any editing action ends
  slotCancelEdit();

  if(pWidget == m_summaryTab) {
    updateDisplay();
  }
}

void KInvestmentView::slotReloadView(void)
{
  ::timetrace("Start KInvestmentView::slotReloadView");
  // make sure to determine the current account from scratch
  m_account = MyMoneyAccount();
  m_accountId = QCString();

  slotRefreshView();
  ::timetrace("Done KInvestmentView::slotReloadView");
}

void KInvestmentView::slotRefreshView(void)
{
  QCString id = m_account.id();

  // qDebug("KGlobalLedgerView::slotRefreshView()");

  // load the combobox from scratch and determine the current account
  loadAccounts();

  // if the current account differs from the previous selection
  // then select the correct ledgerview first and force loading
  // the newly selected account
  if(m_account.id() != id && !id.isEmpty()) {
    slotSelectAccount(m_account.id());
  } else if(m_account.id().isEmpty()) {
    slotSelectAccount(QCString());
    m_ledgerView->refreshView();
    updateDisplay();
  } else {
    m_ledgerView->refreshView();
    updateDisplay();
  }

  // Enable selection widget if we have at least one account
  m_accountComboBox->setEnabled(m_accountComboBox->count() > 0);

  // Enable rest of view only, if we have at least one investment account
  QValueList<MyMoneyAccount::accountTypeE> typeList;
  typeList << MyMoneyAccount::Investment;
  m_tab->setEnabled(m_accountComboBox->accountList(typeList).count() > 0);
}

void KInvestmentView::loadAccounts(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount acc;

  // check if the current account still exists and make it the
  // current account
  if(!m_accountId.isEmpty()) {
    try {
      acc = file->account(m_accountId);
      m_accountId = acc.id();
    } catch(MyMoneyException *e) {
      delete e;
      m_accountId = QCString();
      acc = MyMoneyAccount();
    }
  }

  m_accountComboBox->loadList((KMyMoneyUtils::categoryTypeE)(KMyMoneyUtils::asset | KMyMoneyUtils::liability));

  if(acc.id().isEmpty()) {
    QCStringList list = m_accountComboBox->accountList();
    if(list.count()) {
      QCStringList::Iterator it;
      for(it = list.begin(); it != list.end(); ++it) {
        MyMoneyAccount a = file->account(*it);
        if(a.accountType() == MyMoneyAccount::Investment) {
          if(a.value("PreferredAccount") == "Yes") {
            acc = a;
            break;
          } else if(acc.id().isEmpty()) {
            acc = a;
          }
        }
      }
    }
  }

  slotSelectAccount(acc.id());
}

const bool KInvestmentView::slotSelectAccount(const QCString& id, const QCString& transactionId, const bool /* reconciliation*/)
{
  bool    rc = false;

  if(!id.isEmpty()) {
    // if the account id differs, then we have to do something
    MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
    if(isVisible())
      kmymoney2->selectAccount(acc);
    if(m_accountId != id) {
      // cancel any pending edit operation in the ledger views
      // when switching to a different account
      slotCancelEdit();

      // if we have a stock account here, we need to get it's parent
      if(acc.accountType() == MyMoneyAccount::Stock) {
        acc = MyMoneyFile::instance()->account(acc.parentAccountId());
      }
      if(acc.accountType() == MyMoneyAccount::Investment) {
        m_account = acc;
        m_accountComboBox->setSelected(acc);
        m_ledgerView->slotSelectAccount(acc.id());
        if(!transactionId.isEmpty()) {
          m_tab->showPage(m_transactionTab);
          m_ledgerView->selectTransaction(transactionId);
        }
        updateDisplay();
        rc = true;
      } else {
        // keep the current selection ...
        if(!m_accountId.isEmpty()) {
          acc = MyMoneyFile::instance()->account(m_accountId);
          m_accountComboBox->setSelected(acc);
        } else
          m_accountComboBox->setSelected(QCString());
        // ... and let's see, if someone else can handle this request
        emit accountSelected(id, transactionId);
      }
    } else {
      rc = true;
    }
  } else {
    // cancel any pending edit operation in the ledger views
    // when switching to a non existing account
    slotCancelEdit();
    m_accountComboBox->setSelected(QCString());
    if(isVisible())
      kmymoney2->selectAccount();
  }

  // keep this as the current account if we loaded a different one
  if(rc == true)
    m_accountId = id;

  return rc;
}

void KInvestmentView::show(void)
{
  // only show selection box if filled with at least one account
  m_accountComboBox->setEnabled(m_accountComboBox->count() > 0);

  // if we have a selected account, notify the application about it
  if(!m_accountId.isEmpty()) {
    try {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(m_accountId);
      kmymoney2->selectAccount(acc);
    } catch(MyMoneyException* e) {
      delete e;
    }
  }

  QWidget::show();
  emit signalViewActivated();
}

void KInvestmentView::update(const QCString& id)
{
  if(m_tab->isEnabled()) {
    QCString lastUsed = m_account.id();
    loadAccounts();
    if(m_account.id() != lastUsed) {
      slotRefreshView();
    } else if(id == MyMoneyFile::NotifyClassAccountHierarchy) {
      updateDisplay();
    }
  } else
    slotRefreshView();
}

void KInvestmentView::slotCancelEdit(void)
{
  m_ledgerView->slotCancelEdit();
}

#include "kinvestmentview.moc"
