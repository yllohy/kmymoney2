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

/*
#if QT_VERSION > 300
// #include <qcursor.h>
#endif

#include <qpushbutton.h>
#include <qtable.h>
#include <qinputdialog.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
*/

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
#include "../mymoney/mymoneyequity.h"
#include "../mymoney/mymoneytransaction.h"
#include "../mymoney/mymoneyinvesttransaction.h"
#include "../mymoney/mymoneyaccount.h"
#include "kinvestmentlistitem.h"
#include "../dialogs/knewequityentrydlg.h"
#include "../dialogs/kupdatestockpricedlg.h"
#include "../dialogs/keditequityentrydlg.h"
#include "kinvestmentview.h"
#include "kinvestmentlistitem.h"
#include "kledgerviewinvestments.h"
#include "../widgets/kmymoneyaccountcombo.h"

KInvestmentView::KInvestmentView(QWidget *parent, const char *name)
 :  kInvestmentViewDecl(parent,name)
{
  m_account = MyMoneyAccount();
  m_popMenu = NULL;
  // FIXME: check if we really want to remove the margin
  // kInvestmentViewDeclLayout->setMargin(0);

  qDebug("KInvestmentView::KInvestmentView: Investment View starting up");

  initSummaryTab();
  initTransactionTab();
  
  m_tabMap[m_summaryTab] = VIEW_SUMMARY;
  m_tabMap[m_transactionTab] = VIEW_TRANSACTIONS;
  qDebug("KInvestmentView::KInvestmentView: widgets in map = %d", m_tabMap.size());

  // never show a horizontal scroll bar
 // investmentTable->setHScrollBarMode(QScrollView::AlwaysOff);

  //no sorting yet...
 // investmentTable->setSorting(-1);

//  connect(investmentTable, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
//    this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));

  //connects the signal when a radio button is checked.
 // connect(m_btnGroupView, SIGNAL(clicked(int)), this, SLOT(slotViewChanged(int)));

  //hide transaction view, since we show the summary view by default.
//  transactionTable->hide();

  //set the summary button to be true.
 // btnSummary->setChecked(TRUE);

   connect(m_accountComboBox, SIGNAL(accountSelected(const QCString&)), this, SLOT(slotSelectAccount(const QCString&)));
   connect(m_tab, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotTabSelected(QWidget*)));
 //const bool KInvestmentView::slotSelectAccount(const QCString& id, const bool reconciliation)

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccount, this);

  connect(investmentTable, SIGNAL(doubleClicked(QListViewItem*,const QPoint&, int)), this, SLOT(slotItemDoubleClicked(QListViewItem*,const QPoint&, int)));
}

KInvestmentView::~KInvestmentView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccount, this);
}

void KInvestmentView::initSummaryTab(void)
{
  investmentTable->setRootIsDecorated(true);
  investmentTable->setColumnText(0, QString(i18n("Symbol")));
  investmentTable->addColumn(i18n("Name"));
  investmentTable->addColumn(i18n("Symbol"));
  investmentTable->addColumn(i18n("Quantity"));
  investmentTable->addColumn(i18n("Current Price"));
  investmentTable->addColumn(i18n("Original Price"));
  investmentTable->addColumn(i18n("$ Gain"));
  investmentTable->addColumn(i18n("1 week %"));
  investmentTable->addColumn(i18n("4 weeks %"));
  investmentTable->addColumn(i18n("3 Months %"));
  investmentTable->addColumn(i18n("YTD %"));

  investmentTable->setMultiSelection(false);
  investmentTable->setColumnWidthMode(0, QListView::Maximum);
  investmentTable->header()->setResizeEnabled(true);
  investmentTable->setAllColumnsShowFocus(true);
}

void KInvestmentView::initTransactionTab(void)
{
  QHBoxLayout* m_TransactionTabLayout = new QHBoxLayout( m_transactionTab, 11, 6, "m_summaryTabLayout");

  m_ledgerView = new KLedgerViewInvestments(m_transactionTab, "ledgerview");
  m_TransactionTabLayout->addWidget(m_ledgerView);
}

/** No descriptions */
bool KInvestmentView::init(const MyMoneyAccount& account)
{
  m_account = account;

//  KConfig *config = KGlobal::config();
  QDateTime defaultDate = QDate::currentDate();
  QDate qdateStart = QDate::currentDate();//config->readDateTimeEntry("StartDate", &defaultDate).date();

  if(qdateStart != defaultDate.date())
  {
    MyMoneyInvestTransaction *pInvestTransaction = NULL;
    MyMoneyTransaction *transaction = NULL;
//    m_transactionList.clear();

//    for(transaction=pAccount->transactionFirst(); transaction; transaction=pAccount->transactionNext())
    {
 //     if(transaction->date() >= qdateStart)
      {
//        if(transaction)
        {
          /*pInvestTransaction = static_cast<MyMoneyInvestTransaction*>(transaction);
          m_transactionList.append(new MyMoneyInvestTransaction(
            pAccount,
            transaction->id(),
            transaction->method(),
            transaction->number(),
            transaction->memo(),
            transaction->amount(),
            transaction->date(),
            transaction->categoryMajor(),
            transaction->categoryMinor(),
            transaction->atmBankName(),
            transaction->payee(),
            transaction->accountFrom(),
            transaction->accountTo(),
            transaction->state()));  */
        }
      }
    }
  }
  return true;
}
/** No descriptions */
void KInvestmentView::updateDisplay()
{
  //remove all current items
  investmentTable->clear();

  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyEquity> equities = file->equityList();
  QValueList<MyMoneyTransaction> transactions;
  qDebug("slotRefreshView: Number of equity objects: %d", equities.size());

  for(QValueList<MyMoneyEquity>::ConstIterator it = equities.begin(); it != equities.end(); ++it)
  {
    KInvestmentListItem* item = new KInvestmentListItem(investmentTable, (*it), transactions);
    investmentTable->insertItem(item);
  }

}

void KInvestmentView::slotItemDoubleClicked(QListViewItem* pItem, const QPoint& pos, int c)
{
  if(COLUMN_NAME_INDEX == c || COLUMN_SYMBOL_INDEX == c)
  {
    KInvestmentListItem *pInvestListItem = dynamic_cast<KInvestmentListItem*>(pItem);
    if(pInvestListItem)
    {
      MyMoneyFile* file = MyMoneyFile::instance();

      //get the ID of the equity that was double-clicked, to look up to pass to the dialog.
      QCString id = pInvestListItem->equityId();
      MyMoneyEquity equity = file->equity(id);
      KEditEquityEntryDlg *pDlg = new KEditEquityEntryDlg(equity, this);
      if(pDlg->exec())
      {
        //copies all of the modified object's data into our local copy.
        pDlg->updatedEquity(equity);

        //puts this in the storage container.
        file->modifyEquity(equity);
        
        //update the summary display to show the new data.
        updateDisplay();
      }
    }
  }
}



/*void KInvestmentView::slotNewInvestment()
{
  MyMoneyEquity equity;
  KNewEquityEntryDlg *pDlg = new KNewEquityEntryDlg(this);
  pDlg->exec();
  int nResult = pDlg->result();
  if(nResult)
  {
    //populate this equity entry with information from the dialog.
    QString strTemp;
    strTemp = pDlg->edtEquityName->text();
    kdDebug(1) << "Equity name is: " << strTemp << endl;
//    pEquity->setEquityName(strTemp);

    strTemp = pDlg->edtMarketSymbol->text();
    kdDebug(1) << "Equity Symbol is: " << strTemp << endl;
//  pEquity->setEquitySymbol(strTemp);

    strTemp = pDlg->cmbInvestmentType->currentText();
    kdDebug(1) << "Equity Type is: " << strTemp << endl;
//  pEquity->setEquityType(strTemp);

    const double price = pDlg->getStockPrice();
    kdDebug(1) << "Current Equity Price is: " << price << endl;
    MyMoneyMoney money(price);
//  pEquity->setCurrentPrice(QDate::currentDate(), &money);

    //add to equity database
    addEquityEntry(equity);

    //display new equity in the list view.
    //displayNewEquity(equity);
  }
}   */

void KInvestmentView::addEquityEntry(MyMoneyEquity* /*pEntry*/)
{/*
  if(m_pAccount)
  {
    MyMoneyBank *pBank = m_pAccount->bank();
    if(pBank)
    {
      MyMoneyFile *pFile = pBank->file();
      if(pFile)
      {
        pFile->addEquityEntry(pEntry);
      }
    }
  }
*/
}

void KInvestmentView::displayNewEquity(MyMoneyEquity* /*pEntry*/)
{
/*
  KInvestmentListItem *pItem = new KInvestmentListItem(investmentTable, pEntry);
  investmentTable->insertItem(pItem);
*/
}

void KInvestmentView::slotEditInvestment()
{

}

void KInvestmentView::slotUpdatePrice()
{
/*
  KUpdateStockPriceDlg *pDlg = new KUpdateStockPriceDlg;
  if(pDlg)
  {
    pDlg->exec();
    int nResult = pDlg->result();
  }
*/
}

void KInvestmentView::slotListRightMouse(QListViewItem* item, const QPoint& point, int x)
{
/*
  // setup the context menu
  KIconLoader *kiconloader = KGlobal::iconLoader();
  m_popMenu = new KPopupMenu(this);
  m_popMenu->insertTitle(kiconloader->loadIcon("transaction", KIcon::MainToolbar), i18n("Transaction Options"));
  m_popMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("New Investment"), this, SLOT(slotNewInvestment()));
  m_popMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("Edit Investment Properties"), this, SLOT(slotEditInvestment()));
  m_popMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("Update Investment Price"), this, SLOT(slotUpdatePrice()));
  //m_popMenu = m_contextMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small),
  //                      i18n("Delete ..."),
  //                      this, SLOT(slotDeleteSplitTransaction()));
  if(m_popMenu)
  {
    m_popMenu->exec(QCursor::pos());
  }
*/
}

void KInvestmentView::slotTabSelected(QWidget *pWidget)
{
  qDebug("KInvestmentView::slotTabSelected() called, size=%d", m_tabMap.size());
  tabmap_iterator it = m_tabMap.find(pWidget);
  if(it == m_tabMap.end())
  {
    return;
  }
  else
  {
    int ID = it.data();
    qDebug("KInvestmentView::slotTabSelected(), id=%d", ID);
    switch(ID)
    {
      case VIEW_SUMMARY:
      {
        updateDisplay();
        break;
      }
      case VIEW_TRANSACTIONS:
      {
        break;
      }
      default:
      {
        break;
      }
    }
  }
}

void KInvestmentView::slotReloadView(void)
{
  updateDisplay();

  qDebug("KInvestmentView::slotReloadView()");

  // make sure to determine the current account from scratch
  m_account = MyMoneyAccount();

  slotRefreshView();
}

void KInvestmentView::slotRefreshView(void)
{
  updateDisplay();

  QCString id = m_account.id();

  // qDebug("KGlobalLedgerView::slotRefreshView()");

  // load the combobox from scratch and determine the current account
  loadAccounts();

  // if the current account differs from the previous selection
  // then select the correct ledgerview first and force loading
  // the newly selected account
  if(m_account.id() != id) {
    slotSelectAccount(m_account.id());
  } else if(m_account.id().isEmpty()) {
    slotSelectAccount(QCString());
  } else {
    m_ledgerView->refreshView();
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

  // qDebug("KGlobalLedgerView::loadAccounts()");

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
      acc = file->account(*(list.begin()));
    }
  }

  if(!acc.id().isEmpty()) {
    slotSelectAccount(acc.id());
  }
}

const bool KInvestmentView::slotSelectAccount(const QCString& id, const bool reconciliation)
{

  bool    rc = false;

  qDebug("KInvestmentView::slotSelectAccount id=%s", id.data());

  // cancel any pending edit operation in the ledger views
  //emit cancelEdit();

  if(!id.isEmpty()) {
    // if the account id differs, then we have to do something
    MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
    if(m_accountId != id) {
      if(acc.accountType() == MyMoneyAccount::Investment) {
        m_account = acc;
        m_accountComboBox->setSelected(acc);
        m_ledgerView->slotSelectAccount(acc.id());
        rc = true;
      } else {
        // let's see, if someone else can handle this request
        emit accountSelected(id, QCString());
      }
    } else {
#if KDE_VERSION < 310
      // in KDE 3.1 and above, QWidgetStack::show() takes care of this
//      m_accountStack->raiseWidget(acc.accountType());
#endif
      rc = true;
    }
  } else {
    m_accountComboBox->setText("");
  }

  // keep this as the current account
  m_accountId = id;

  return rc;
}

const bool KInvestmentView::slotSelectAccount(const QString& accountName)
{
  // qDebug("KGlobalLedgerView::slotSelectAccount(const QString& accountName)");

  QCString id = MyMoneyFile::instance()->nameToAccount(accountName);
  bool     rc = false;
  if(!id.isEmpty()) {
    rc = slotSelectAccount(id);
  }
  return rc;
}

void KInvestmentView::slotSelectAccountAndTransaction(const QCString& accountId,
                                                const QCString& transactionId)
{
  // if the selection of the account succeeded then select the desired transaction
  if(slotSelectAccount(accountId)) {
    // FIXME: select ledger tab and then call the selectTransaction method
    // m_ledgerView->selectTransaction(transactionId);
  }
}


void KInvestmentView::show(void)
{
  kInvestmentViewDecl::show();
  emit signalViewActivated();
}

void KInvestmentView::update(const QCString& /* id */)
{
  QCString lastUsed = m_account.id();
  loadAccounts();
  if(m_account.id() != lastUsed) {
    slotRefreshView();
  }
}
