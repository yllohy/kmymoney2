/***************************************************************************
                          kmymoneyview.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
                               2004 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qlabel.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qprogressdialog.h>
#include <qtextcodec.h>
#include <qsignalmapper.h>
#include <qstatusbar.h>

#include <qcursor.h>
#include <qregexp.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"

#include <kfiledialog.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kicontheme.h>
#include <kiconloader.h>

#include <kmessagebox.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <ktempfile.h>
#include <ksavefile.h>
#include <kfilterdev.h>
#include <kfilterbase.h>
#include <kfileitem.h>
#include <kpushbutton.h>
#include <kapplication.h>

// ----------------------------------------------------------------------------
// Project Includes

// This is include is required here, because later it will produce
// compile errors on gcc 3.2 as we redefine new() in case of _CHECK_MEMORY
// being defined. To avoid these problems, we just include the header
// already here in this case
#ifdef _CHECK_MEMORY
#include <string>
#endif

#include "../dialogs/knewaccountwizard.h"
#include "../dialogs/knewbankdlg.h"
#include "../dialogs/knewaccountdlg.h"
#include "../dialogs/kendingbalancedlg.h"
#include "../dialogs/knewfiledlg.h"
#include "../dialogs/kchooseimportexportdlg.h"
#include "../dialogs/kcsvprogressdlg.h"
#include "../dialogs/kimportdlg.h"
#include "../dialogs/kexportdlg.h"
#include "../dialogs/knewloanwizard.h"
#include "../dialogs/kcurrencyeditdlg.h"
#include "../dialogs/kofxdirectconnectdlg.h"

#include "../mymoney/storage/mymoneyseqaccessmgr.h"
#include "../mymoney/storage/imymoneystorageformat.h"
#include "../mymoney/storage/mymoneystoragebin.h"
#include "../mymoney/mymoneyexception.h"
#include "../mymoney/storage/mymoneystoragedump.h"
#include "../mymoney/storage/mymoneystoragexml.h"
#include "../mymoney/storage/mymoneystoragegnc.h"
#include "../mymoney/storage/mymoneystorageanon.h"

#include "kmymoneyview.h"
#include "kbanksview.h"
#include "khomeview.h"
#include "kcategoriesview.h"
#include "kpayeesview.h"
#include "kscheduledview.h"
#include "kgloballedgerview.h"
#include "kinvestmentview.h"
#include "kreportsview.h"

#include "../kmymoney2.h"
#include "../kmymoneyutils.h"

#include <libkgpgfile/kgpgfile.h>

#define COMPRESSION_MIME_TYPE "application/x-gzip"
#define RECOVER_KEY_ID        "0xD2B08440"

KMyMoneyView::KMyMoneyView(QWidget *parent, const char *name)
  : KJanusWidget(parent, name, KJanusWidget::IconList),
  m_searchDlg(0),
  m_fileOpen(false),
  m_bankRightClick(false)
{
  // the global variable kmymoney2 is not yet assigned. So we construct it here
  QObject* kmymoney2 = parent->parent();

  // create an empty file
  // m_file = new KMyMoneyFile;
  newStorage();

  QSignalMapper* signalMap = new QSignalMapper(this);
  // Page 0
  m_homeViewFrame = addVBoxPage( i18n("Home"), i18n("Home"),
    DesktopIcon("home"));
  m_homeView = new KHomeView(m_homeViewFrame, "HomeView");
  signalMap->setMapping(m_homeView, HomeView);
  connect(m_homeView, SIGNAL(signalViewActivated()), signalMap, SLOT(map()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_homeView, SLOT(slotReloadView()));

  // Page 1
  m_institutionsViewFrame = addVBoxPage( i18n("Institutions"), i18n("Institutions"),
    DesktopIcon("institutions"));
  m_institutionsView = new KAccountsView(m_institutionsViewFrame, "InstitutionsView", true);
  signalMap->setMapping(m_institutionsView, InstitutionsView);
  connect(m_institutionsView, SIGNAL(signalViewActivated()), signalMap, SLOT(map()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_institutionsView, SLOT(slotReloadView()));

  // Page 2
  m_accountsViewFrame = addVBoxPage( i18n("Accounts"), i18n("Accounts"),
    DesktopIcon("accounts"));
  m_accountsView = new KAccountsView(m_accountsViewFrame, "AccountsView");
  signalMap->setMapping(m_accountsView, AccountsView);
  connect(m_accountsView, SIGNAL(signalViewActivated()), signalMap, SLOT(map()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_accountsView, SLOT(slotReloadView()));

  // Page 3
  m_scheduleViewFrame = addVBoxPage( i18n("Schedule"), i18n("Bills & Reminders"),
    DesktopIcon("schedule"));
  m_scheduledView = new KScheduledView(m_scheduleViewFrame, "ScheduledView");
  signalMap->setMapping(m_scheduledView, SchedulesView);
  connect(m_scheduledView, SIGNAL(signalViewActivated()), signalMap, SLOT(map()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_scheduledView, SLOT(slotReloadView()));

  // Page 4
  m_categoriesViewFrame = addVBoxPage( i18n("Categories"), i18n("Categories"),
    DesktopIcon("categories"));
  m_categoriesView = new KCategoriesView(m_categoriesViewFrame, "CategoriesView");
  signalMap->setMapping(m_categoriesView, CategoriesView);
  connect(m_categoriesView, SIGNAL(signalViewActivated()), signalMap, SLOT(map()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_categoriesView, SLOT(slotReloadView()));

  // Page 5
  m_payeesViewFrame = addVBoxPage( i18n("Payees"), i18n("Payees"),
    DesktopIcon("payee"));
  m_payeesView = new KPayeesView(m_payeesViewFrame, "PayeesView");
  signalMap->setMapping(m_payeesView, PayeesView);
  connect(m_payeesView, SIGNAL(signalViewActivated()), signalMap, SLOT(map()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_payeesView, SLOT(slotReloadView()));

  // Page 6
  m_ledgerViewFrame = addVBoxPage( i18n("Ledgers"), i18n("Ledgers"),
    DesktopIcon("ledger"));
  m_ledgerView = new KGlobalLedgerView(m_ledgerViewFrame, "GlobalLedgerView");
  // the next line causes the ledgers to get a hide() signal to be able
  // to end any pending edit activities
  connect(this, SIGNAL(aboutToShowPage(QWidget*)), m_ledgerView, SLOT(slotCancelEdit()));
  signalMap->setMapping(m_ledgerView, LedgersView);
  connect(m_ledgerView, SIGNAL(signalViewActivated()), signalMap, SLOT(map()));
  connect(m_ledgerView, SIGNAL(accountSelected(const QCString&, const QCString&)),
      this, SLOT(slotLedgerSelected(const QCString&, const QCString&)));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_ledgerView, SLOT(slotReloadView()));

  // Page 7
  m_investmentViewFrame = addVBoxPage( i18n("Investments"), i18n("Investments"),
    DesktopIcon("investments"));

  m_investmentView = new KInvestmentView(m_investmentViewFrame, "InvestmentView");
  connect(this, SIGNAL(aboutToShowPage(QWidget*)), m_investmentView, SLOT(slotCancelEdit()));
  signalMap->setMapping(m_investmentView, InvestmentsView);
  connect(m_investmentView, SIGNAL(signalViewActivated()), signalMap, SLOT(map()));
  connect(m_investmentView, SIGNAL(accountSelected(const QCString&, const QCString&)),
      this, SLOT(slotLedgerSelected(const QCString&, const QCString&)));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_investmentView, SLOT(slotReloadView()));

  // Page 8
  m_reportsViewFrame = addVBoxPage(i18n("Reports"), i18n("Reports"),
    DesktopIcon("report"));
  m_reportsView = new KReportsView(m_reportsViewFrame, "ReportsView");
  signalMap->setMapping(m_reportsView, ReportsView);
  connect(m_reportsView, SIGNAL(signalViewActivated()), signalMap, SLOT(map()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_reportsView, SLOT(slotReloadView()));

  // connect the view activation signal mapper
  connect(signalMap, SIGNAL(mapped(int)), this, SIGNAL(viewActivated(int)));

  connect(m_accountsView, SIGNAL(accountRightMouseClick()), this, SLOT(slotAccountRightMouse()));
  connect(m_accountsView, SIGNAL(accountDoubleClick()), this, SLOT(slotAccountDoubleClick()));
  connect(m_accountsView, SIGNAL(categoryDoubleClick()), this, SLOT(slotAccountEdit()));
  //connect(accountsView, SIGNAL(accountSelected()), this, SLOT(slotAccountSelected()));
  connect(m_accountsView, SIGNAL(bankRightMouseClick()), this, SLOT(slotBankRightMouse()));
  connect(m_accountsView, SIGNAL(rightMouseClick()), this, SLOT(slotRightMouse()));

  connect(m_institutionsView, SIGNAL(bankRightMouseClick()), this, SLOT(slotBankRightMouse()));
  connect(m_institutionsView, SIGNAL(accountRightMouseClick()), this, SLOT(slotAccountRightMouse()));

  connect(m_categoriesView, SIGNAL(categoryRightMouseClick()),
    this, SLOT(slotAccountRightMouse()));


  connect(m_payeesView, SIGNAL(transactionSelected(const QCString&, const QCString&)),
          this, SLOT(slotLedgerSelected(const QCString&, const QCString&)));
  connect(m_ledgerView, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
          this, SLOT(slotPayeeSelected(const QCString&, const QCString&, const QCString&)));

  connect(m_homeView, SIGNAL(ledgerSelected(const QCString&, const QCString&)),
          this, SLOT(slotLedgerSelected(const QCString&, const QCString&)));

  connect(m_homeView, SIGNAL(scheduleSelected(const QCString&)),
    this, SLOT(slotScheduleSelected(const QCString&)));

  connect(m_homeView, SIGNAL(reportSelected(const QCString&)),
    this, SLOT(slotReportSelected(const QCString&)));

  // construct account context menu
  KIconLoader *kiconloader = KGlobal::iconLoader();

  m_accountMenu = new KPopupMenu(this);
  m_accountMenu->insertTitle(kiconloader->loadIcon("account", KIcon::MainToolbar), i18n("Account Options"));
  m_accountMenu->insertItem(kiconloader->loadIcon("account_add", KIcon::Small), i18n("New account..."), this, SLOT(slotAccountNew()), 0, AccountNew);
  m_accountMenu->insertItem(kiconloader->loadIcon("account_open", KIcon::Small), i18n("Open..."), this, SLOT(slotAccountDoubleClick()), 0, AccountOpen);
  m_accountMenu->insertSeparator();
  m_accountMenu->insertItem(kiconloader->loadIcon("reconcile", KIcon::Small), i18n("Reconcile..."), this, SLOT(slotAccountReconcile()), 0, AccountReconcile);
  m_accountMenu->insertSeparator();
  m_accountMenu->insertItem(kiconloader->loadIcon("account", KIcon::Small), i18n("Edit..."), this, SLOT(slotAccountEdit()), 0, AccountEdit);
  m_accountMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small), i18n("Delete..."), this, SLOT(slotAccountDelete()), 0, AccountDelete);

  m_accountMenu->insertItem(kiconloader->loadIcon("account", KIcon::Small),
                              i18n("Online update using OFX..."),
                              this, SLOT(slotAccountOfxConnect()), 0,
                              AccountOfxConnect);

  m_bankMenu = new KPopupMenu(this);
  m_bankMenu->insertTitle(kiconloader->loadIcon("bank", KIcon::MainToolbar), i18n("Institution Options"));
  // Use a proxy slot
  m_bankMenu->insertItem(kiconloader->loadIcon("account_add", KIcon::Small), i18n("New Account..."), this, SLOT(slotBankAccountNew()));
  m_bankMenu->insertItem(kiconloader->loadIcon("bank", KIcon::Small), i18n("Edit..."), this, SLOT(slotBankEdit()));
  m_bankMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small), i18n("Delete..."), this, SLOT(slotBankDelete()));

  m_rightMenu = new KPopupMenu(this);
  m_rightMenu->insertTitle(kiconloader->loadIcon("bank", KIcon::MainToolbar), i18n("KMyMoney Options"));
  m_rightMenu->insertItem(kiconloader->loadIcon("bank", KIcon::Small), i18n("New Institution..."), this, SLOT(slotBankNew()));

  // construct an empty file
  newFile(true);

  // get rid of the title text
  QWidget* widget = dynamic_cast<QWidget*>(child("KJanusWidgetTitleLabel", "QLabel"));
  if(widget)
    widget->hide();
  // and the separator below it
  widget = dynamic_cast<QWidget*>(child(0, "KSeparator"));
  if(widget)
    widget->hide();

  // select the page first, before connecting the aboutToShow signal
  // because we don't want to override the information stored in the config file
  showPage(0);
  connect(this, SIGNAL(aboutToShowPage(QWidget*)), this, SLOT(slotRememberPage(QWidget*)));
}

KMyMoneyView::~KMyMoneyView()
{
  removeStorage();
  if(m_searchDlg)
    delete m_searchDlg;
}

void KMyMoneyView::newStorage(void)
{
  removeStorage();
  MyMoneyFile* file = MyMoneyFile::instance();
  file->attachStorage(new MyMoneySeqAccessMgr);
}

void KMyMoneyView::removeStorage(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  IMyMoneyStorage* p = file->storage();
  if(p != 0) {
    file->detachStorage(p);
    delete p;
  }
}

void KMyMoneyView::enableViews(int state)
{
  if(state == -1)
    state = m_fileOpen;

  m_accountsViewFrame->setEnabled(state);
  m_institutionsViewFrame->setEnabled(state);
  m_scheduleViewFrame->setEnabled(state);
  m_categoriesViewFrame->setEnabled(state);
  m_payeesViewFrame->setEnabled(state);
  m_ledgerViewFrame->setEnabled(state);
  m_reportsViewFrame->setEnabled(state);

  emit viewStateChanged(state != 0);
}

void KMyMoneyView::slotRightMouse()
{
  m_rightMenu->exec(QCursor::pos());
}

void KMyMoneyView::slotAccountRightMouse()
{
  bool  ok = false;
  QCString acc;

  if(pageIndex(m_accountsViewFrame) == activePageIndex())
    acc = m_accountsView->currentAccount(ok);
  else if(pageIndex(m_institutionsViewFrame) == activePageIndex())
    acc = m_institutionsView->currentAccount(ok);
  else
    acc = m_categoriesView->currentAccount(ok);

  // turn off all available options in the menu except New
  m_accountMenu->setItemEnabled(AccountNew, true);
  m_accountMenu->setItemEnabled(AccountOpen, false);
  m_accountMenu->setItemEnabled(AccountEdit, false);
  m_accountMenu->setItemEnabled(AccountReconcile, false);
  m_accountMenu->setItemEnabled(AccountDelete, false);
  m_accountMenu->setItemEnabled(AccountOfxConnect, false);

  m_accountMenu->disconnectItem(AccountNew, this, SLOT(slotAccountNew()));
  m_accountMenu->disconnectItem(AccountNew, this, SLOT(slotCategoryNew()));

  if(ok == true) {
    try {
      MyMoneyFile* file = MyMoneyFile::instance();
      MyMoneyAccount account = file->account(acc);
      switch(file->accountGroup(account.accountType())) {
        case MyMoneyAccount::Asset:
        case MyMoneyAccount::Liability:
          if(!file->isStandardAccount(acc)) {
            m_accountMenu->setItemEnabled(AccountOpen, true);
            m_accountMenu->setItemEnabled(AccountReconcile, true);
            m_accountMenu->setItemEnabled(AccountEdit, true);
            m_accountMenu->setItemEnabled(AccountDelete, file->transactionCount(account.id())==0);

            QCString iid = account.institutionId();
            if ( !iid.isEmpty() )
            {
              MyMoneyInstitution institution = file->institution(iid);
              if ( institution.ofxConnectionSettings().pairs().count() )
                m_accountMenu->setItemEnabled(AccountOfxConnect, true);
            }
          }
          m_accountMenu->changeItem(AccountNew, i18n("New account..."));
          m_accountMenu->connectItem(AccountNew, this, SLOT(slotAccountNew()));
          break;

        case MyMoneyAccount::Income:
        case MyMoneyAccount::Expense:
          if(!file->isStandardAccount(acc)) {
            m_accountMenu->setItemEnabled(AccountEdit, true);
            m_accountMenu->setItemEnabled(AccountDelete, file->transactionCount(account.id())==0);
          }
          m_accountMenu->changeItem(AccountNew, i18n("New category..."));
          m_accountMenu->connectItem(AccountNew, this, SLOT(slotCategoryNew()));
          break;

        default:
          break;
      }
      // notify others about the selection
      emit accountSelectedForContextMenu(account);

    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception in KMyMoneyView::slotAccountRightMouse");
      delete e;
    }
  }
  m_accountMenu->exec(QCursor::pos());
}

void KMyMoneyView::slotLedgerSelected(const QCString& accId, const QCString& transaction)
{
  MyMoneyAccount acc = MyMoneyFile::instance()->account(accId);
  switch(acc.accountType()) {
    case MyMoneyAccount::Investment:
      showPage(pageIndex(m_investmentViewFrame));
      m_investmentView->slotSelectAccount(accId, transaction);
      break;

    case MyMoneyAccount::Stock:
      showPage(pageIndex(m_investmentViewFrame));
      m_investmentView->slotSelectAccount(accId, transaction);
      break;

    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::CreditCard:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::AssetLoan:
      showPage(pageIndex(m_ledgerViewFrame));
      m_ledgerView->slotSelectAccount(accId, transaction);
      break;

    case MyMoneyAccount::CertificateDep:
    case MyMoneyAccount::MoneyMarket:
    case MyMoneyAccount::Currency:
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
      qDebug("No view available for account type %d", acc.accountType());
      break;

    default:
      qDebug("Unknown account type %d in KMyMoneyView::slotLedgerSelected", acc.accountType());
      break;
  }
}

void KMyMoneyView::slotPayeeSelected(const QCString& payee, const QCString& account, const QCString& transaction)
{
  showPage(pageIndex(m_payeesViewFrame));
  m_payeesView->slotSelectPayeeAndTransaction(payee, account, transaction);
}

void KMyMoneyView::slotScheduleSelected(const QCString& schedule)
{
  showPage(pageIndex(m_scheduleViewFrame));
  m_scheduledView->slotSelectSchedule(schedule);
}

void KMyMoneyView::slotReportSelected(const QCString& reportid)
{
  showPage(pageIndex(m_reportsViewFrame));
  m_reportsView->slotOpenReport(reportid);
}

void KMyMoneyView::slotAccountDoubleClick(void)
{
  bool  ok = false;
  QCString acc;

  if(pageIndex(m_accountsViewFrame) == activePageIndex())
    acc = m_accountsView->currentAccount(ok);
  else if(pageIndex(m_institutionsViewFrame) == activePageIndex())
    acc = m_institutionsView->currentAccount(ok);
  else
    acc = m_categoriesView->currentAccount(ok);

  if(ok == true) {
    // select the correct view
    MyMoneyAccount account = MyMoneyFile::instance()->account(acc);
    if(account.accountType() == MyMoneyAccount::Investment) {
      showPage(pageIndex(m_investmentViewFrame));
    } else {
      showPage(pageIndex(m_ledgerViewFrame));
      m_ledgerView->slotSelectAccount(acc);
    }
  }
}

void KMyMoneyView::slotBankRightMouse()
{
  int editId = m_bankMenu->idAt(2);
  int deleteId = m_bankMenu->idAt(3);
  bool bankSuccess;
  bool state = false;

  if(pageIndex(m_accountsViewFrame) == activePageIndex())
    state = !m_accountsView->currentInstitution(bankSuccess).isEmpty();
  else if(pageIndex(m_institutionsViewFrame) == activePageIndex())
    state = !m_institutionsView->currentInstitution(bankSuccess).isEmpty();
  else
    state = !m_institutionsView->currentInstitution(bankSuccess).isEmpty();

  m_bankMenu->setItemEnabled(editId, state);
  m_bankMenu->setItemEnabled(deleteId, state);

  m_bankMenu->exec(QCursor::pos());
}

void KMyMoneyView::slotBankEdit()
{
  if (!fileOpen())
    return;

  bool bankSuccess=false;
  try
  {
    MyMoneyFile* file = MyMoneyFile::instance();

    //grab a pointer to the view, regardless of it being a account or institution view.
    KAccountsView* pView = NULL;
    pView = (pageIndex(m_accountsViewFrame) == activePageIndex()) ? m_accountsView : m_institutionsView;

    MyMoneyInstitution institution = file->institution(pView->currentInstitution(bankSuccess));

    // bankSuccess is not checked anymore because m_file->institution will throw anyway
    KNewBankDlg dlg(institution, true, this, "NewBankDlg");
    if (dlg.exec())
    {
      file->modifyInstitution(dlg.institution());
    }
  }
  catch(MyMoneyException *e)
  {
    if (bankSuccess)  // we got the bank but unable to modify
    {
      QString message(i18n("Unable to edit institution: "));
      message += e->what();
      KMessageBox::information(this, message);
    }
    delete e;
  }
}

void KMyMoneyView::slotBankDelete()
{
  if (!fileOpen())
    return;

  bool bankSuccess=false;
  try
  {
    MyMoneyFile *file = MyMoneyFile::instance();

    //grab a pointer to the view, regardless of it being a account or institution view.
    KAccountsView* pView = NULL;
    pView = (pageIndex(m_accountsViewFrame) == activePageIndex()) ? m_accountsView : m_institutionsView;

    MyMoneyInstitution institution = file->institution(pView->currentInstitution(bankSuccess));
    QString msg = i18n("Really delete this institution: ");
    msg += institution.name();
    if ((KMessageBox::questionYesNo(this, msg))==KMessageBox::No)
      return;
    file->removeInstitution(institution);
  }
  catch (MyMoneyException *e)
  {
    if (bankSuccess)
    {
      QString errorString = i18n("Cannot delete institution: ");
      errorString += e->what();
      KMessageBox::information(this, errorString);
    }
    delete e;
  }
}

void KMyMoneyView::slotAccountEdit()
{
  if (!fileOpen())
    return;

  bool accountSuccess=false;

  try
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount account;

    if(pageIndex(m_accountsViewFrame) == activePageIndex()
    || pageIndex(m_institutionsViewFrame) == activePageIndex()) {

      //grab a pointer to the view, regardless of it being a account or institution view.
      KAccountsView* pView = (pageIndex(m_accountsViewFrame) == activePageIndex()) ? m_accountsView : m_institutionsView;

      QCString accountId = pView->currentAccount(accountSuccess);
      if(!accountId.isEmpty()) {
        account = file->account(accountId);
        switch(MyMoneyAccount::accountGroup(account.accountType())) {
          case MyMoneyAccount::Asset:
          case MyMoneyAccount::Liability:
            pView->slotEditClicked();
            break;
          case MyMoneyAccount::Income:
          case MyMoneyAccount::Expense:
            m_categoriesView->slotEditClicked(account);
            break;
          default:
            qDebug("%s:%d This should not happen!", __FILE__ , __LINE__);
            break;
        }
      }
    } else {
      m_categoriesView->slotEditClicked();
    }
  }
  catch (MyMoneyException *e)
  {
    if (accountSuccess)
    {
      QString errorString = i18n("Cannot edit account/category: ");
      errorString += e->what();

      KMessageBox::information(this, errorString);
    }
    delete e;
    return;
  }
}


void KMyMoneyView::slotAccountDelete()
{
  if (!fileOpen())
    return;

  bool accountSuccess=false;

  try {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount account;

    if(pageIndex(m_accountsViewFrame) == activePageIndex()
    || pageIndex(m_institutionsViewFrame) == activePageIndex()) {
      //grab a pointer to the view, regardless of it being a account or institution view.
      KAccountsView* pView = (pageIndex(m_accountsViewFrame) == activePageIndex()) ? m_accountsView : m_institutionsView;

      QCString accountId = pView->currentAccount(accountSuccess);
      if(!accountId.isEmpty()) {
        account = file->account(accountId);
        switch(MyMoneyAccount::accountGroup(account.accountType())) {
          case MyMoneyAccount::Asset:
          case MyMoneyAccount::Liability:
            pView->slotDeleteClicked();
            break;
          case MyMoneyAccount::Income:
          case MyMoneyAccount::Expense:
            m_categoriesView->slotDeleteClicked(account);
            break;
          default:
            qDebug("%s:%d This should not happen!", __FILE__ , __LINE__);
            break;
        }
      }
    } else {
      m_categoriesView->slotDeleteClicked();
    }
  } catch (MyMoneyException *e) {
    if (accountSuccess) {
      QString errorString = i18n("Cannot delete account: ");
      errorString += e->what();
      KMessageBox::information(this, errorString);
    }
    delete e;
    return;
  }
}

void KMyMoneyView::slotAccountOfxConnect(void)
{
  bool accountSuccess=false;
  try
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount account;

    if(pageIndex(m_accountsViewFrame) == activePageIndex()
    || pageIndex(m_institutionsViewFrame) == activePageIndex()) {
      //grab a pointer to the view, regardless of it being a account or institution view.
      KAccountsView* pView = (pageIndex(m_accountsViewFrame) == activePageIndex()) ? m_accountsView : m_institutionsView;

      QCString accountId = pView->currentAccount(accountSuccess);
      if(!accountId.isEmpty())
      {
        account = file->account(accountId);

        KOfxDirectConnectDlg dlg(account);

        KMyMoney2App* mw = dynamic_cast<KMyMoney2App*>(kapp->mainWidget());
        connect(&dlg,SIGNAL(statementReady(const MyMoneyOfxStatement&)),mw,SLOT(slotOfxStatementImport(const MyMoneyOfxStatement&)));

        dlg.init();
        dlg.exec();
      }
    }
  }
  catch (MyMoneyException *e)
  {
    KMessageBox::information(this,i18n("Error connecting to bank: %1").arg(e->what()));
    delete e;
  }

}

bool KMyMoneyView::fileOpen(void)
{
  return m_fileOpen;
}

void KMyMoneyView::closeFile(void)
{
  if ( m_reportsView )
    m_reportsView->slotCloseAll();

  newStorage();
  m_fileOpen = false;
}

void KMyMoneyView::ungetString(QIODevice *qfile, char *buf, int len)
{
  buf = &buf[len-1];
  while(len--) {
    qfile->ungetch(*buf--);
  }
}

bool KMyMoneyView::readFile(const KURL& url)
{
  QString filename;

  newStorage();
  m_fileOpen = false;

  IMyMoneyStorageFormat* pReader = NULL;

#if KDE_IS_VERSION(3,2,0)
  if(!url.isValid()) {
#else
  if(url.isMalformed()) {
#endif
    qDebug("Invalid URL '%s'", url.url().latin1());
    return false;
  }

  if(url.isLocalFile()) {
    filename = url.path();

  } else {
    if(!KIO::NetAccess::download(url, filename)) {
      KMessageBox::detailedError(this,
             i18n("Error while loading file '%1'!").arg(url.url()),
             KIO::NetAccess::lastErrorString(),
             i18n("File access error"));
      return false;
    }
  }

  // let's glimps into the file to figure out, if it's one
  // of the old (uncompressed) or new (compressed) files.
  QFile file(filename);
  QFileInfo info(file);
  if(!info.isFile()) {
    QString msg=i18n("<b>%1</b> is not a KMyMoney file.").arg(filename);
    KMessageBox::error(this, QString("<p>")+msg, i18n("Filetype Error"));
    return false;
  }

  QIODevice *qfile = 0;
  bool rc = true;

  // There's a problem with the KFilterDev and KGPGFile classes:
  // One supports the at(n) member but not ungetch() together with
  // readBlock() and the other does not provide an at(n) method but
  // supports readBlock() that considers the ungetch() buffer. QFile
  // supports everything so this is not a problem. We solve the problem
  // for now by keeping track of which method can be used.
  bool haveAt = true;

  if(file.open(IO_ReadOnly)) {
    QByteArray hdr(2);
    int cnt;
    cnt = file.readBlock(hdr.data(), 2);
    file.close();

    if(cnt == 2) {
      if(QString(hdr) == QString("\037\213")) {         // gzipped?
        qfile = KFilterDev::deviceForFile(filename, COMPRESSION_MIME_TYPE);
      } else if(QString(hdr) == QString("--")){         // PGP ASCII armored?
        if(KGPGFile::GPGAvailable()) {
          qfile = new KGPGFile(filename);
          haveAt = false;
        } else {
          KMessageBox::sorry(this, i18n("GPG is not available for decryption of file '%1'").arg(filename));
          qfile = new QFile(file.name());
        }
      } else {
        // we can't use file directly, as we delete qfile later on
        qfile = new QFile(file.name());
      }

      if(qfile->open(IO_ReadOnly)) {
        try {
          hdr.resize(8);
          if(qfile->readBlock(hdr.data(), 8) == 8) {
            if(haveAt)
              qfile->at(0);
            else
              ungetString(qfile, hdr.data(), 8);

            // Ok, we got the first block of 8 bytes. Read in the two
            // unsigned long int's by preserving endianess. This is
            // achieved by reading them through a QDataStream object
            Q_INT32 magic0, magic1;
            QDataStream s(hdr, IO_ReadOnly);
            s >> magic0;
            s >> magic1;

            // If both magic numbers match (we actually read in the
            // text 'KMyMoney' then we assume a binary file and
            // construct a reader for it. Otherwise, we construct
            // an XML reader object.
            //
            // The expression magic0 < 30 is only used to create
            // a binary reader if we assume an old binary file. This
            // should be removed at some point. An alternative is to
            // check the beginning of the file against an pattern
            // of the XML file (e.g. '?<xml' ).
            if((magic0 == MAGIC_0_50 && magic1 == MAGIC_0_51)
            || magic0 < 30)
              pReader = new MyMoneyStorageBin;

            else {
              // Scan the first 70 bytes to see if we find something
              // we know. For now, we support our own XML format and
              // GNUCash XML format. If the file is smaller, then it
              // contains no valid data and we reject it anyway.
              hdr.resize(70);
              if(qfile->readBlock(hdr.data(), 70) == 70) {
                if(haveAt)
                  qfile->at(0);
                else
                  ungetString(qfile, hdr.data(), 70);
                QRegExp kmyexp("<!DOCTYPE KMYMONEY-FILE>");
                QRegExp gncexp("<gnc-v(\\d+)>");
                QCString txt(hdr);
                if(kmyexp.search(txt) != -1) {
                  pReader = new MyMoneyStorageXML;
                } else if(gncexp.search(txt) != -1) {
                  loadDefaultCurrencies(); // currency list required for gnc
                  pReader = new MyMoneyStorageGNC;
                }
              }
            }
            if(pReader) {
              pReader->setProgressCallback(&KMyMoneyView::progressCallback);
              pReader->readFile(qfile, dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()));
            } else {
              KMessageBox::sorry(this, i18n("File '%1' contains an unknown file format!").arg(filename));
              rc = false;
            }
          } else {
            KMessageBox::sorry(this, i18n("Cannot read from file '%1'!").arg(filename));
            rc = false;
          }
        } catch (MyMoneyException *e) {
          QString msg = e->what();
          qDebug("%s", msg.latin1());
          delete e;
        }
        if(pReader) {
          pReader->setProgressCallback(0);
          delete pReader;
        }
        qfile->close();
      } else {
        KMessageBox::sorry(this, i18n("File '%1' not found!").arg(filename));
        rc = false;
      }
      delete qfile;
    }
  } else {
    KMessageBox::sorry(this, i18n("File '%1' not found!").arg(filename));
    rc = false;
  }

  if(rc == false)
    return rc;

  // if a temporary file was constructed by NetAccess::download,
  // then it will be removed with the next call. Otherwise, it
  // stays untouched on the local filesystem
  KIO::NetAccess::removeTempFile(filename);

  MyMoneyFile::instance()->suspendNotify(true);
  // we check, if we have any currency in the file. If not, we load
  // all the default currencies we know.
  if(MyMoneyFile::instance()->currencyList().count() == 0)
    loadDefaultCurrencies();

  // make sure, we have a base currency and all accounts are
  // also assigned to a currency.
  if(MyMoneyFile::instance()->baseCurrency().id().isEmpty()) {
    // Stay in this endless loop until we have a base currency,
    // as without it the application does not work anymore.
    while(MyMoneyFile::instance()->baseCurrency().id().isEmpty())
      selectBaseCurrency();

  } else {
    // in some odd intermediate cases there could be files out there
    // that have a base currency set, but still have accounts that
    // do not have a base currency assigned. This call will take
    // care of it. We can safely remove it later.
    //
    // Another work-around for this scenario is to remove the base
    // currency setting from the XML file by removing the line
    //
    //   <PAIR key="kmm-baseCurrency" value="xxx" />
    //
    // and restart the application with this file. This will for to
    // run the above loop.
    selectBaseCurrency();
  }

  MyMoneyFile::instance()->suspendNotify(false);

  KConfig *config = KGlobal::config();
  int page;
  config->setGroup("General Options");
  if(config->readBoolEntry("StartLastViewSelected", false) == true) {
    config->setGroup("Last Use Settings");
    page = config->readNumEntry("LastViewSelected", 0);
  } else {
    page = pageIndex(m_homeViewFrame);
  }

  // For debugging purposes, we can turn off the automatic fix manually
  // by setting the entry in kmymoney2rc to true
  config->setGroup("General Options");
  if(config->readBoolEntry("SkipFix", false) != true) {
    MyMoneyFile::instance()->suspendNotify(true);
    try {
      // Check if we have to modify the file before we allow to work with it
      fixFile();
    } catch(MyMoneyException *e) {
      delete e;
      MyMoneyFile::instance()->suspendNotify(false);
      return false;
    }
    MyMoneyFile::instance()->suspendNotify(false);
  } else {
    qDebug("Skipping automatic transaction fix!");
  }

  // FIXME: we need to check, if it's necessary to have this
  //        automatic funcitonality
  // if there's no asset account, then automatically start the
  // new account wizard
  // kmymoney2->createInitialAccount();

  // if we currently see a different page, then select the right one
  if(page != activePageIndex()) {
    showPage(page);
  }

  m_fileOpen = true;
  return true;
}

void KMyMoneyView::saveToLocalFile(QFile* qfile, IMyMoneyStorageFormat* pWriter, bool plaintext)
{
  QIODevice *dev = qfile;
  KFilterBase *base = 0;

  KConfig *config = KGlobal::config();
  config->setGroup("General Options");

  bool encryptedOk = true;
  bool encryptRecover = false;
  if(config->readBoolEntry("WriteDataEncrypted", false) != false) {
    qDebug("Write encrypted");
    if(!KGPGFile::GPGAvailable()) {
      KMessageBox::sorry(this, i18n("GPG does not seem to be installed on your system. Please make sure, that GPG can be found using the standard search path. This time, encryption is disabled."), i18n("GPG not found"));
      encryptedOk = false;
    }

    if(config->readBoolEntry("EncryptRecover", false) != false) {
      encryptRecover = true;
      if(!KGPGFile::keyAvailable(QString(RECOVER_KEY_ID))) {
        KMessageBox::sorry(this, QString("<p>")+i18n("You have selected to encrypt your data also with the KMyMoney recover key, but the key with id</p><p><center><b>%1</b></center></p>has not been found in your keyring at this time. Please make sure to import this key into your keyring. You can find it on the <a href=\"http://kmymoney2.sourceforge.net/\">KMyMoney web-site</a>. This time your data will not be encrypted with the KMyMoney recover key.").arg(RECOVER_KEY_ID), i18n("GPG-Key not found"));
        encryptRecover = false;
      }
    }
    if(config->readEntry("GPG-Recipient").length() > 0) {
      if(!KGPGFile::keyAvailable(config->readEntry("GPG-Recipient"))) {
        KMessageBox::sorry(this, QString("<p>")+i18n("You have specified to encrypt your data for the user-id</p><p><center><b>%1</b>.</center></p>Unfortunately, a valid key for this user-id was not found in your keyring. Please make sure to import a valid key for this user-id. This time, encryption is disabled.").arg(config->readEntry("GPG-Recipient")), i18n("GPG-Key not found"));
        encryptedOk = false;
      }
    } else {
      KMessageBox::sorry(this, QString("<p>")+i18n("You have specified to encrypt your data but you have not  provided a user-id. Please make sure to setup a valid user id. This time, encryption is disabled.").arg(config->readEntry("GPG-Recipient")), i18n("GPG-Key not found"));
      encryptedOk = false;
    }

    if(encryptedOk == true) {
      QString msg = QString("<p>") + i18n("You have configured to save your data in encrypted form using GPG. Please be aware, that this is a brand new feature which is yet untested. Make sure, you have the necessary understanding that you might loose all your data if you store it encrypted and cannot decrypt it later on! If unsure, answer <b>No</b>.");

      if(KMessageBox::questionYesNo(this, msg, i18n("Store GPG encrypted"), KStdGuiItem::yes(), KStdGuiItem::no(), "StoreEncrypted") == KMessageBox::No) {
        encryptedOk = false;
      }
    }
  }

  if(config->readBoolEntry("WriteDataEncrypted", false) != false
  && encryptedOk == true && !plaintext ) {
    qfile->close();
    base++;
    KGPGFile *kgpg = new KGPGFile(qfile->name());
    if(kgpg) {
      kgpg->addRecipient(config->readEntry("GPG-Recipient").latin1());
      if(encryptRecover) {
        kgpg->addRecipient(RECOVER_KEY_ID);
      }
    }
    dev = kgpg;
    if(!dev || !dev->open(IO_WriteOnly))
      throw new MYMONEYEXCEPTION(i18n("Unable to open file '%1' for writing.").arg(qfile->name()));

  } else if(config->readBoolEntry("WriteDataUncompressed", false) == false && !plaintext) {
    base = KFilterBase::findFilterByMimeType( COMPRESSION_MIME_TYPE );
    if(base) {
      base->setDevice(qfile, false);
      qfile->close();
      // we need to reopen the file to set the mode inside the filter stuff
      dev = new KFilterDev(base, true);
      if(!dev || !dev->open(IO_WriteOnly))
        throw new MYMONEYEXCEPTION(i18n("Unable to open file '%1' for writing.").arg(qfile->name()));
    }
  }

  pWriter->setProgressCallback(&KMyMoneyView::progressCallback);
  dev->resetStatus();
  pWriter->writeFile(dev, dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()));
  if(dev->status() != IO_Ok) {
    throw new MYMONEYEXCEPTION(i18n("Failure while writing to '%1'").arg(qfile->name()));
  }
  pWriter->setProgressCallback(0);

  if(base != 0) {
    dev->flush();
    dev->close();
    delete dev;
  } else
    qfile->close();
}

const bool KMyMoneyView::saveFile(const KURL& url)
{
  QString filename = url.path();

  if (!fileOpen()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return false;
  }

  if(KMessageBox::warningContinueCancel(this, i18n(
      "Since this version of KMyMoney only writes data files in it's new "
      "format, files written with this version cannot be read by KMyMoney version 0.4. "
      "If you still want to use older versions of KMyMoney with your data files, "
      "please make sure you keep a backup-file of your finance data. "
      "If you want to abort this operation, please press Cancel now"),
      QString::null, KStdGuiItem::cont(), "WarningNewFileVersion0.5") == KMessageBox::Cancel)
    return false;

  IMyMoneyStorageFormat* pWriter = NULL;

  // If this file ends in ".ANON.XML" then this should be written using the
  // anonymous writer.
  bool plaintext = false;
  if (filename.right(9).lower() == ".anon.xml")
  {
    pWriter = new MyMoneyStorageANON;
    plaintext = true;
  }
  else
  {
    // only use XML writer. The binary format will be depreacated sometime
    pWriter = new MyMoneyStorageXML;
  }

  // actually, url should be the parameter to this function
  // but for now, this would involve too many changes
  bool rc = true;
  try {
    if(url.isMalformed()) {
      throw new MYMONEYEXCEPTION(i18n("Malformed URL '%1'").arg(url.url()));
    }

    if(url.isLocalFile()) {
      filename = url.path();
      KSaveFile qfile(filename, 0600);
      if(qfile.status() == 0) {
        saveToLocalFile(qfile.file(), pWriter,plaintext);
        if(!qfile.close()) {
          throw new MYMONEYEXCEPTION(i18n("Unable to write changes to '%1'").arg(filename));
        }
      } else {
        throw new MYMONEYEXCEPTION(i18n("Unable to write changes to '%1'").arg(filename));
      }
    } else {
      KTempFile tmpfile;
      saveToLocalFile(tmpfile.file(), pWriter,plaintext);
      if(!KIO::NetAccess::upload(tmpfile.name(), url))
        throw new MYMONEYEXCEPTION(i18n("Unable to upload to '%1'").arg(url.url()));
      tmpfile.unlink();
    }
  } catch (MyMoneyException *e) {
    KMessageBox::error(this, e->what());
    delete e;
    MyMoneyFile::instance()->setDirty();
    rc = false;
  }
  delete pWriter;
  return rc;
}

bool KMyMoneyView::dirty(void)

{
  if (!fileOpen())
    return false;

  return MyMoneyFile::instance()->dirty();
}

void KMyMoneyView::slotBankNew(void)
{
  if (!fileOpen())
    return;

  MyMoneyInstitution institution;

  KNewBankDlg dlg(institution, false, this, "NewBankDlg");
  if (dlg.exec())
  {
    try
    {
      MyMoneyFile* file = MyMoneyFile::instance();

      institution = dlg.institution();

      file->addInstitution(institution);
    }
    catch (MyMoneyException *e)
    {
      delete e;
      KMessageBox::information(this, i18n("Cannot add bank"));
      return;
    }
  }
}

void KMyMoneyView::slotCategoryNew(void)
{
  m_bankRightClick=false;
  accountNew(true);
}

void KMyMoneyView::slotAccountNew(void)
{
  m_bankRightClick=false;
  accountNew(false);
}

void KMyMoneyView::accountNew(const bool createCategory)
{

  if (!fileOpen())
    return;

  MyMoneyAccount newAccount;
  MyMoneyAccount parentAccount;
  MyMoneyAccount brokerageAccount;
  KNewAccountWizard *newAccountWizard = 0;
  int dialogResult;

  if(createCategory == false) {
    // create account, use wizard
    newAccountWizard = new KNewAccountWizard(this, "NewAccountWizard");
    connect(newAccountWizard, SIGNAL(newInstitutionClicked()), this, SLOT(slotBankNew()));

    newAccountWizard->setAccountName(QString());
    newAccountWizard->setOpeningBalance(0);

    // Preselect the institution if we right clicked on a bank
    if (m_bankRightClick)
      newAccountWizard->setInstitution(m_accountsInstitution);

    if((dialogResult = newAccountWizard->exec()) == QDialog::Accepted) {
      newAccount = newAccountWizard->account();
      parentAccount = newAccountWizard->parentAccount();
      brokerageAccount = newAccountWizard->brokerageAccount();
    }
  } else {
    // regular dialog selected
    MyMoneyAccount account;
    QString title;
    QCString accId;
    bool ok;

    if(pageIndex(m_accountsViewFrame) == activePageIndex())
      accId = m_accountsView->currentAccount(ok);
    else if(pageIndex(m_institutionsViewFrame) == activePageIndex())
      accId = m_institutionsView->currentAccount(ok);
    else if(pageIndex(m_categoriesViewFrame) == activePageIndex())
      accId = m_categoriesView->currentAccount(ok);

    if(ok) {
      try {
        MyMoneyFile* file = MyMoneyFile::instance();
        MyMoneyAccount parent = file->account(accId);
        account.setParentAccountId(accId);
        account.setAccountType( parent.accountType() );
      } catch(MyMoneyException *e) {
        delete e;
      }
    }

    if(createCategory == false)
      title = i18n("Create a new Account");
    else
      title = i18n("Create a new Category");
    KNewAccountDlg dialog(account, false, createCategory, 0, "NewAccountDlg", title);

    if((dialogResult = dialog.exec()) == QDialog::Accepted) {
      newAccount = dialog.account();
      parentAccount = dialog.parentAccount();
    }
  }

  if(dialogResult == QDialog::Accepted) {

    // The dialog/wizard doesn't check the parent.
    // An exception will be thrown on the next line instead.
    try
    {
      // Check the opening balance
      MyMoneyMoney openingBal = newAccount.openingBalance();
      if (openingBal.isPositive() && newAccount.accountGroup() == MyMoneyAccount::Liability)
      {
        openingBal = -openingBal;
        QString message = i18n("This account is a liability and if the "
            "opening balance represents money owed, then it should be negative.  "
            "Negate the amount?\n\n"
            "Please click Yes to change the opening balance to %1,\n"
            "Please click No to leave the amount as %2,\n"
            "Please click Cancel to abort the account creation.")
            .arg(openingBal.formatMoney())
            .arg(newAccount.openingBalance().formatMoney());

        int ans = KMessageBox::questionYesNoCancel(this, message);
        if (ans == KMessageBox::Yes)
        {
          newAccount.setOpeningBalance(openingBal);
        }
        else if (ans == KMessageBox::Cancel)
          return;
      }

      MyMoneyFile::instance()->addAccount(newAccount, parentAccount);

      // We MUST add the schedule AFTER adding the account because
      // otherwise an unknown account will be thrown.
      if(newAccountWizard != 0)
        createSchedule(newAccountWizard->schedule(), newAccount);

      // in case of a loan account, we add the initial payment
      if((newAccount.accountType() == MyMoneyAccount::Loan
      || newAccount.accountType() == MyMoneyAccount::AssetLoan)
      && !newAccount.value("kmm-loan-payment-acc").isEmpty()
      && !newAccount.value("kmm-loan-payment-date").isEmpty()) {
        MyMoneyAccountLoan acc(newAccount);
        MyMoneyTransaction t;
        MyMoneySplit a, b;
        a.setAccountId(acc.id());
        b.setAccountId(acc.value("kmm-loan-payment-acc").latin1());
        a.setValue(acc.loanAmount());
        if(acc.accountType() == MyMoneyAccount::Loan)
          a.setValue(-a.value());
        b.setValue(-a.value());
        a.setMemo(i18n("Loan payout"));
        b.setMemo(i18n("Loan payout"));
        t.setPostDate(QDate::fromString(acc.value("kmm-loan-payment-date"), Qt::ISODate));
        try {
          newAccount.deletePair("kmm-loan-payment-acc");
          newAccount.deletePair("kmm-loan-payment-date");
          MyMoneyFile::instance()->modifyAccount(newAccount);

          t.addSplit(a);
          t.addSplit(b);
          MyMoneyFile::instance()->addTransaction(t);
        } catch(MyMoneyException *e) {
          qDebug("Cannot add loan payout transaction: %s", e->what().latin1());
          delete e;
        }

      // in case of an investment account we check if we should create
      // a brokerage account
      } else if(newAccount.accountType() == MyMoneyAccount::Investment
             && !brokerageAccount.name().isEmpty()) {
        try {
          MyMoneyFile::instance()->addAccount(brokerageAccount, parentAccount);

          // set a link from the investment account to the brokerage account
          newAccount.setValue("kmm-brokerage-account", brokerageAccount.id());
          MyMoneyFile::instance()->modifyAccount(newAccount);
        } catch(MyMoneyException *e) {
          qDebug("Cannot add brokerage account: %s", e->what().latin1());
          delete e;
        }
      }
    }
    catch (MyMoneyException *e)
    {
      QString message("Unable to add account: ");
      message += e->what();
      KMessageBox::information(this, message);
      delete e;
    }
  }
  if(newAccountWizard != 0) {
    delete newAccountWizard;
    newAccountWizard = 0;
  }
}

void KMyMoneyView::slotAccountReconcile(void)
{
  bool  ok = false;
  QCString acc;
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount account;

  if(pageIndex(m_accountsViewFrame) == activePageIndex())
    acc = m_accountsView->currentAccount(ok);
  else if(pageIndex(m_institutionsViewFrame) == activePageIndex())
    acc = m_institutionsView->currentAccount(ok);
  else
    acc = m_categoriesView->currentAccount(ok);

  // we cannot reconcile standard accounts
  if(file->isStandardAccount(acc))
    ok = false;

  // check if we can reconcile this account
  // it make's sense for asset and liability accounts
  if(ok == true) {
    try {
      account = file->account(acc);
      switch(file->accountGroup(account.accountType())) {
        case MyMoneyAccount::Asset:
        case MyMoneyAccount::Liability:
          break;
        default:
          ok = false;
      }
    } catch(MyMoneyException *e) {
      delete e;
      ok = false;
    }
  }

  if(ok == true) {
    showPage(pageIndex(m_ledgerViewFrame));
    m_ledgerView->slotSelectAccount(acc, true);
  }
}

void KMyMoneyView::slotAccountImportAscii(void)
{
/*
  KChooseImportExportDlg dlg(0, this);
  if (dlg.exec()) {
    if (dlg.importExportType()=="QIF") {
      KImportDlg importDlg(getAccount(), this);
      if (importDlg.exec()) {
        transactionView->refresh();
        accountsView->refresh(m_file);
      }


    }
    else {
      KCsvProgressDlg kcsvprogressdlg(0, getAccount(), this);
      if (kcsvprogressdlg.exec()) {
        transactionView->refresh();
        accountsView->refresh(m_file);
      }
    }
  }

*/

}

void KMyMoneyView::slotAccountExportAscii(void)
{
/*
  KChooseImportExportDlg dlg(1, this);
  if (dlg.exec()) {
    if (dlg.importExportType()=="QIF") {
      KExportDlg exportDlg(getAccount(), this);
      exportDlg.exec();
//      slotAccountExportQIF();
    }
    else {
      KCsvProgressDlg kcsvprogressdlg(1, getAccount(), this);
      kcsvprogressdlg.exec();
    }
  }
*/
}

void KMyMoneyView::newFile(const bool createEmtpyFile)
{
  closeFile();

  MyMoneyFile *file = MyMoneyFile::instance();

  if(!createEmtpyFile) {
    KNewFileDlg newFileDlg(this, "NewFileDlg", i18n("Create new KMyMoney file"));
    newFileDlg.cancelButton()->hide();

    newFileDlg.exec();

    if(newFileDlg.userNameText.length() != 0)
      file->setUserName(newFileDlg.userNameText);
    if(newFileDlg.userStreetText.length() != 0)
      file->setUserStreet(newFileDlg.userStreetText);
    if(newFileDlg.userTownText.length() != 0)
      file->setUserTown(newFileDlg.userTownText);
    if(newFileDlg.userCountyText.length() != 0)
      file->setUserCounty(newFileDlg.userCountyText);
    if(newFileDlg.userPostcodeText.length() != 0)
      file->setUserPostcode(newFileDlg.userPostcodeText);
    if(newFileDlg.userTelephoneText.length() != 0)
      file->setUserTelephone(newFileDlg.userTelephoneText);
    if(newFileDlg.userEmailText.length() != 0)
      file->setUserEmail(newFileDlg.userEmailText);

#if 0
    KMessageBox::information(this,
                  i18n("The next dialog allows you to add a set of predefined accounts and categories to the new file. Different languages are available to select from. You can skip loading any predefined categories by selecting \"Cancel\" from the next dialog."),
                  i18n("Load predefined accounts/categories"));

    loadDefaultCategories();
#endif

    loadDefaultCurrencies();

    // Stay in this endless loop until we have a base currency,
    // as without it the application does not work anymore.
    while(MyMoneyFile::instance()->baseCurrency().id().isEmpty())
      selectBaseCurrency();
  }
  m_fileOpen = true;
}

void KMyMoneyView::viewPersonal(void)
{
  if (!fileOpen()) {
    KMessageBox::information(this, i18n("Cannot edit personal data"));
    return;
  }

  MyMoneyFile* file = MyMoneyFile::instance();

  KNewFileDlg newFileDlg(file->userName(), file->userStreet(),
    file->userTown(), file->userCounty(), file->userPostcode(), file->userTelephone(),
    file->userEmail(), this, "NewFileDlg", i18n("Edit Personal Data"));

  if (newFileDlg.exec())
  {
    file->setUserName(newFileDlg.userNameText);
    file->setUserStreet(newFileDlg.userStreetText);
    file->setUserTown(newFileDlg.userTownText);
    file->setUserCounty(newFileDlg.userCountyText);
    file->setUserPostcode(newFileDlg.userPostcodeText);
    file->setUserTelephone(newFileDlg.userTelephoneText);
    file->setUserEmail(newFileDlg.userEmailText);
  }
}

void KMyMoneyView::selectBaseCurrency(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  // check if we have a base currency. If not, we need to select one
  if(file->baseCurrency().id().isEmpty()) {
    KCurrencyEditDlg dlg(this, "CurrencyEditDlg");
    dlg.exec();
  }

  if(!file->baseCurrency().id().isEmpty()) {
    // check that all accounts have a currency
    QValueList<MyMoneyAccount> list = file->accountList();
    QValueList<MyMoneyAccount>::Iterator it;

    // don't forget those standard accounts
    list << file->asset();
    list << file->liability();
    list << file->income();
    list << file->expense();
    list << file->equity();

    for(it = list.begin(); it != list.end(); ++it) {
      if((*it).currencyId().isEmpty() || (*it).currencyId().length() == 0) {
        (*it).setCurrencyId(file->baseCurrency().id());
        try {
          fixOpeningBalance(*it);
          file->modifyAccount(*it);
        } catch(MyMoneyException *e) {
          qDebug("Unable to setup base currency in account %s (%s): %s", (*it).name().latin1(), (*it).id().data(), e->what().latin1());
          delete e;
        }
      }
    }
  }
}

void KMyMoneyView::loadDefaultCurrencies(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  try {
    file->addCurrency(MyMoneySecurity("AFA", i18n("Afghanistan Afghani")));
    file->addCurrency(MyMoneySecurity("ALL", i18n("Albanian Lek")));
    file->addCurrency(MyMoneySecurity("DZD", i18n("Algerian Dinar")));
    file->addCurrency(MyMoneySecurity("ADF", i18n("Andorran Franc")));
    file->addCurrency(MyMoneySecurity("ADP", i18n("Andorran Peseta")));
    file->addCurrency(MyMoneySecurity("AON", i18n("Angolan New Kwanza")));
    file->addCurrency(MyMoneySecurity("ARS", i18n("Argentine Peso")));
    file->addCurrency(MyMoneySecurity("AWG", i18n("Aruban Florin")));
    file->addCurrency(MyMoneySecurity("AUD", i18n("Australian Dollar"),      "$"));
    file->addCurrency(MyMoneySecurity("AZM", i18n("Azerbaijani Manat")));
    file->addCurrency(MyMoneySecurity("BSD", i18n("Bahamian Dollar"),        "$"));
    file->addCurrency(MyMoneySecurity("BHD", i18n("Bahraini Dinar"),         "BHD", 1000, 1000));
    file->addCurrency(MyMoneySecurity("BDT", i18n("Bangladeshi Taka")));
    file->addCurrency(MyMoneySecurity("BBD", i18n("Barbados Dollar"),        "$"));
    file->addCurrency(MyMoneySecurity("BYR", i18n("Belarussian Ruble"),      "BYR", 1, 1));
    file->addCurrency(MyMoneySecurity("BZD", i18n("Belize Dollar"),          "$"));
    file->addCurrency(MyMoneySecurity("BMD", i18n("Bermudian Dollar"),       "$"));
    file->addCurrency(MyMoneySecurity("BTN", i18n("Bhutan Ngultrum")));
    file->addCurrency(MyMoneySecurity("BOB", i18n("Bolivian Boliviano")));
    file->addCurrency(MyMoneySecurity("BAM", i18n("Bosnian Convertible Mark")));
    file->addCurrency(MyMoneySecurity("BWP", i18n("Botswana Pula")));
    file->addCurrency(MyMoneySecurity("BRL", i18n("Brazilian Real"),         "R$"));
    file->addCurrency(MyMoneySecurity("GBP", i18n("British Pound"),          QChar(0x00A3)));
    file->addCurrency(MyMoneySecurity("BND", i18n("Brunei Dollar"),          "$"));
    file->addCurrency(MyMoneySecurity("BGL", i18n("Bulgarian Lev")));
    file->addCurrency(MyMoneySecurity("BIF", i18n("Burundi Franc")));
    file->addCurrency(MyMoneySecurity("XAF", i18n("CFA Franc BEAC")));
    file->addCurrency(MyMoneySecurity("XOF", i18n("CFA Franc BCEAO")));
    file->addCurrency(MyMoneySecurity("XPF", i18n("CFP Franc Pacifique")));
    file->addCurrency(MyMoneySecurity("KHR", i18n("Cambodia Riel")));
    file->addCurrency(MyMoneySecurity("CAD", i18n("Canadian Dollar"),        "$"));
    file->addCurrency(MyMoneySecurity("CVE", i18n("Cape Verde Escudo")));
    file->addCurrency(MyMoneySecurity("KYD", i18n("Cayman Islands Dollar"),  "$"));
    file->addCurrency(MyMoneySecurity("CLP", i18n("Chilean Peso")));
    file->addCurrency(MyMoneySecurity("CNY", i18n("Chinese Yuan Renminbi")));
    file->addCurrency(MyMoneySecurity("COP", i18n("Colombian Peso")));
    file->addCurrency(MyMoneySecurity("KMF", i18n("Comoros Franc")));
    file->addCurrency(MyMoneySecurity("CRC", i18n("Costa Rican Colon"),      QChar(0x20A1)));
    file->addCurrency(MyMoneySecurity("HRK", i18n("Croatian Kuna")));
    file->addCurrency(MyMoneySecurity("CUP", i18n("Cuban Peso")));
    file->addCurrency(MyMoneySecurity("CYP", i18n("Cyprus Pound"),           QChar(0x00A3)));
    file->addCurrency(MyMoneySecurity("CZK", i18n("Czech Koruna")));
    file->addCurrency(MyMoneySecurity("DKK", i18n("Danish Krone"),           "kr"));
    file->addCurrency(MyMoneySecurity("DJF", i18n("Djibouti Franc")));
    file->addCurrency(MyMoneySecurity("DOP", i18n("Dominican Peso")));
    file->addCurrency(MyMoneySecurity("XCD", i18n("East Caribbean Dollar"),  "$"));
    file->addCurrency(MyMoneySecurity("EGP", i18n("Egyptian Pound"),         QChar(0x00A3)));
    file->addCurrency(MyMoneySecurity("SVC", i18n("El Salvador Colon")));
    file->addCurrency(MyMoneySecurity("ERN", i18n("Eritrean Nakfa")));
    file->addCurrency(MyMoneySecurity("EEK", i18n("Estonian Kroon")));
    file->addCurrency(MyMoneySecurity("ETB", i18n("Ethiopian Birr")));
    file->addCurrency(MyMoneySecurity("EUR", i18n("Euro"),                   QChar(0x20ac)));
    file->addCurrency(MyMoneySecurity("FKP", i18n("Falkland Islands Pound"), QChar(0x00A3)));
    file->addCurrency(MyMoneySecurity("FJD", i18n("Fiji Dollar"),            "$"));
    file->addCurrency(MyMoneySecurity("GMD", i18n("Gambian Dalasi")));
    file->addCurrency(MyMoneySecurity("GEL", i18n("Georgian Lari")));
    file->addCurrency(MyMoneySecurity("GHC", i18n("Ghanaian Cedi")));
    file->addCurrency(MyMoneySecurity("GIP", i18n("Gibraltar Pound"),        QChar(0x00A3)));
    file->addCurrency(MyMoneySecurity("GTQ", i18n("Guatemalan Quetzal")));
    file->addCurrency(MyMoneySecurity("GWP", i18n("Guinea-Bissau Peso")));
    file->addCurrency(MyMoneySecurity("GYD", i18n("Guyanan Dollar"),         "$"));
    file->addCurrency(MyMoneySecurity("HTG", i18n("Haitian Gourde")));
    file->addCurrency(MyMoneySecurity("HNL", i18n("Honduran Lempira")));
    file->addCurrency(MyMoneySecurity("HKD", i18n("Hong Kong Dollar"),       "$"));
    file->addCurrency(MyMoneySecurity("HUF", i18n("Hungarian Forint"),       "HUF", 1, 1, 100));
    file->addCurrency(MyMoneySecurity("ISK", i18n("Iceland Krona")));
    file->addCurrency(MyMoneySecurity("INR", i18n("Indian Rupee"),           QChar(0x20A8)));
    file->addCurrency(MyMoneySecurity("IDR", i18n("Indonesian Rupiah"),      "IDR", 100, 1));
    file->addCurrency(MyMoneySecurity("IRR", i18n("Iranian Rial"),           "IRR", 1, 1));
    file->addCurrency(MyMoneySecurity("IQD", i18n("Iraqi Dinar"),            "IQD", 1000, 1000));
    file->addCurrency(MyMoneySecurity("NIS", i18n("Israeli New Shekel"),     QChar(0x20AA)));
    file->addCurrency(MyMoneySecurity("JMD", i18n("Jamaican Dollar"),        "$"));
    file->addCurrency(MyMoneySecurity("JPY", i18n("Japanese Yen"),           QChar(0x00A5), 100, 1));
    file->addCurrency(MyMoneySecurity("JOD", i18n("Jordanian Dinar"),        "JOD", 1000, 1000));
    file->addCurrency(MyMoneySecurity("KZT", i18n("Kazakhstan Tenge")));
    file->addCurrency(MyMoneySecurity("KES", i18n("Kenyan Shilling")));
    file->addCurrency(MyMoneySecurity("KWD", i18n("Kuwaiti Dinar"),          "KWD", 1000, 1000));
    file->addCurrency(MyMoneySecurity("KGS", i18n("Kyrgyzstan Som")));
    file->addCurrency(MyMoneySecurity("LAK", i18n("Laos Kip"),               QChar(0x20AD)));
    file->addCurrency(MyMoneySecurity("LVL", i18n("Latvian Lats")));
    file->addCurrency(MyMoneySecurity("LBP", i18n("Lebanese Pound"),         QChar(0x00A3)));
    file->addCurrency(MyMoneySecurity("LSL", i18n("Lesotho Loti")));
    file->addCurrency(MyMoneySecurity("LRD", i18n("Liberian Dollar"),        "$"));
    file->addCurrency(MyMoneySecurity("LYD", i18n("Libyan Dinar"),           "LYD", 1000, 1000));
    file->addCurrency(MyMoneySecurity("LTL", i18n("Lithuanian Litas")));
    file->addCurrency(MyMoneySecurity("MOP", i18n("Macau Pataca")));
    file->addCurrency(MyMoneySecurity("MKD", i18n("Macedonian Denar")));
    file->addCurrency(MyMoneySecurity("MGF", i18n("Malagasy Franc"),         "MGF", 500, 500));
    file->addCurrency(MyMoneySecurity("MWK", i18n("Malawi Kwacha")));
    file->addCurrency(MyMoneySecurity("MYR", i18n("Malaysian Ringgit")));
    file->addCurrency(MyMoneySecurity("MVR", i18n("Maldive Rufiyaa")));
    file->addCurrency(MyMoneySecurity("MLF", i18n("Mali Republic Franc")));
    file->addCurrency(MyMoneySecurity("MTL", i18n("Maltese Lira")));
    file->addCurrency(MyMoneySecurity("MRO", i18n("Mauritanian Ouguiya"),    "MRO", 5, 5));
    file->addCurrency(MyMoneySecurity("MUR", i18n("Mauritius Rupee")));
    file->addCurrency(MyMoneySecurity("MXP", i18n("Mexican Peso"),           "$"));
    file->addCurrency(MyMoneySecurity("MNT", i18n("Mongolian Tugrik"),       QChar(0x20AE)));
    file->addCurrency(MyMoneySecurity("MAD", i18n("Moroccan Dirham")));
    file->addCurrency(MyMoneySecurity("MZM", i18n("Mozambique Metical")));
    file->addCurrency(MyMoneySecurity("MMK", i18n("Myanmar Kyat")));
    file->addCurrency(MyMoneySecurity("NAD", i18n("Namibian Dollar"),        "$"));
    file->addCurrency(MyMoneySecurity("NPR", i18n("Nepalese Rupee")));
    file->addCurrency(MyMoneySecurity("NZD", i18n("New Zealand Dollar"),     "$"));
    file->addCurrency(MyMoneySecurity("NIC", i18n("Nicaraguan Cordoba Oro")));
    file->addCurrency(MyMoneySecurity("NGN", i18n("Nigerian Naira"),         QChar(0x20A6)));
    file->addCurrency(MyMoneySecurity("KPW", i18n("North Korean Won"),       QChar(0x20A9)));
    file->addCurrency(MyMoneySecurity("NOK", i18n("Norwegian Kroner"),       "kr"));
    file->addCurrency(MyMoneySecurity("OMR", i18n("Omani Rial"),             "OMR", 1000, 1000));
    file->addCurrency(MyMoneySecurity("PKR", i18n("Pakistan Rupee")));
    file->addCurrency(MyMoneySecurity("PAB", i18n("Panamanian Balboa")));
    file->addCurrency(MyMoneySecurity("PGK", i18n("Papua New Guinea Kina")));
    file->addCurrency(MyMoneySecurity("PYG", i18n("Paraguay Guarani")));
    file->addCurrency(MyMoneySecurity("PEN", i18n("Peruvian Nuevo Sol")));
    file->addCurrency(MyMoneySecurity("PHP", i18n("Philippine Peso"),        QChar(0x20B1)));
    file->addCurrency(MyMoneySecurity("PLN", i18n("Polish Zloty")));
    file->addCurrency(MyMoneySecurity("QAR", i18n("Qatari Rial")));
    file->addCurrency(MyMoneySecurity("ROL", i18n("Romanian Leu")));
    file->addCurrency(MyMoneySecurity("RUR", i18n("Russian Rouble")));
    file->addCurrency(MyMoneySecurity("RWF", i18n("Rwanda Franc")));
    file->addCurrency(MyMoneySecurity("WST", i18n("Samoan Tala")));
    file->addCurrency(MyMoneySecurity("STD", i18n("Sao Tome and Principe Dobra")));
    file->addCurrency(MyMoneySecurity("SAR", i18n("Saudi Riyal")));
    file->addCurrency(MyMoneySecurity("SCR", i18n("Seychelles Rupee")));
    file->addCurrency(MyMoneySecurity("SLL", i18n("Sierra Leone Leone")));
    file->addCurrency(MyMoneySecurity("SGD", i18n("Singapore Dollar"),       "$"));
    file->addCurrency(MyMoneySecurity("SKK", i18n("Slovak Koruna")));
    file->addCurrency(MyMoneySecurity("SIT", i18n("Slovenian Tolar")));
    file->addCurrency(MyMoneySecurity("SBD", i18n("Solomon Islands Dollar"), "$"));
    file->addCurrency(MyMoneySecurity("SOS", i18n("Somali Shilling")));
    file->addCurrency(MyMoneySecurity("ZAR", i18n("South African Rand")));
    file->addCurrency(MyMoneySecurity("KRW", i18n("South Korean Won"),       QChar(0x20A9)));
    file->addCurrency(MyMoneySecurity("LKR", i18n("Sri Lanka Rupee")));
    file->addCurrency(MyMoneySecurity("SHP", i18n("St. Helena Pound"),       QChar(0x00A3)));
    file->addCurrency(MyMoneySecurity("SDD", i18n("Sudanese Dinar")));
    file->addCurrency(MyMoneySecurity("SRG", i18n("Suriname Guilder")));
    file->addCurrency(MyMoneySecurity("SZL", i18n("Swaziland Lilangeni")));
    file->addCurrency(MyMoneySecurity("SEK", i18n("Swedish Krona")));
    file->addCurrency(MyMoneySecurity("CHF", i18n("Swiss Franc"),            "SFr"));
    file->addCurrency(MyMoneySecurity("SYP", i18n("Syrian Pound"),           QChar(0x00A3)));
    file->addCurrency(MyMoneySecurity("TWD", i18n("Taiwan Dollar"),          "$"));
    file->addCurrency(MyMoneySecurity("TJS", i18n("Tajikistan Somani")));
    file->addCurrency(MyMoneySecurity("TZS", i18n("Tanzanian Shilling")));
    file->addCurrency(MyMoneySecurity("THB", i18n("Thai Baht"),              QChar(0x0E3F)));
    file->addCurrency(MyMoneySecurity("TOP", i18n("Tongan Pa'anga")));
    file->addCurrency(MyMoneySecurity("TTD", i18n("Trinidad and Tobago Dollar"), "$"));
    file->addCurrency(MyMoneySecurity("TND", i18n("Tunisian Dinar"),         "TND", 1000, 1000));
    file->addCurrency(MyMoneySecurity("TRL", i18n("Turkish Lira")));
    file->addCurrency(MyMoneySecurity("YTL", i18n("Turkish Lira (new)")));
    file->addCurrency(MyMoneySecurity("TMM", i18n("Turkmenistan Manat")));
    file->addCurrency(MyMoneySecurity("USD", i18n("US Dollar"),              "$"));
    file->addCurrency(MyMoneySecurity("UGX", i18n("Uganda Shilling")));
    file->addCurrency(MyMoneySecurity("UAH", i18n("Ukraine Hryvnia")));
    file->addCurrency(MyMoneySecurity("AED", i18n("United Arab Emirates Dirham")));
    file->addCurrency(MyMoneySecurity("UYU", i18n("Uruguayan Peso")));
    file->addCurrency(MyMoneySecurity("UZS", i18n("Uzbekistani Sum")));
    file->addCurrency(MyMoneySecurity("VUV", i18n("Vanuatu Vatu")));
    file->addCurrency(MyMoneySecurity("VEB", i18n("Venezuelan Bolivar")));
    file->addCurrency(MyMoneySecurity("VND", i18n("Vietnamese Dong"),        QChar(0x20AB)));
    file->addCurrency(MyMoneySecurity("YUM", i18n("Yugoslav Dinar")));
    file->addCurrency(MyMoneySecurity("ZMK", i18n("Zambian Kwacha")));
    file->addCurrency(MyMoneySecurity("ZWD", i18n("Zimbabwe Dollar"),        "$"));

    file->addCurrency(MyMoneySecurity("XAU", i18n("Gold"),       "XAU", 1, 1000000));
    file->addCurrency(MyMoneySecurity("XPD", i18n("Palladium"),  "XPD", 1, 1000000));
    file->addCurrency(MyMoneySecurity("XPT", i18n("Platinum"),   "XPT", 1, 1000000));
    file->addCurrency(MyMoneySecurity("XAG", i18n("Silver"),     "XAG", 1, 1000000));

  } catch(MyMoneyException *e) {
    qDebug("Error %s loading default currencies", e->what().data());
    delete e;
  }
}

# if 0
// I leave that in here for now to be able to see what
// the tax specific code looks like as I did not have that in yet
void KMyMoneyView::loadDefaultCategories(void)
{
  KFileDialog* dialog = new KFileDialog(KGlobal::dirs()->findResourceDir("appdata", "default_accounts_enC.dat"),
                                        i18n("*.dat|Account templates"),
                                        this, "defaultaccounts",
                                        true);
  dialog->setMode(KFile::File | KFile::ExistingOnly | KFile::LocalOnly);
  dialog->setCaption(i18n("Select account template"));

  if(dialog->exec() == QDialog::Accepted) {
    readDefaultCategories(dialog->selectedFile());
  }
  delete dialog;
  m_accountsView->slotReloadView();
  m_categoriesView->slotReloadView();
}

void KMyMoneyView::readDefaultCategories(const QString& filename)
{
  if (filename == QString::null) {
    KMessageBox::error(this, i18n("Cannot find the data file containing the default categories"));
    return;
  }

  MyMoneyFile::instance()->suspendNotify(true);
  QFile f(filename);
  QTextCodec* codec = QTextCodec::codecForName("ISO8859-1");
  if (f.open(IO_ReadOnly) ) {
    kmymoney2->slotStatusMsg(i18n("Loading default accounts"));
    kmymoney2->slotStatusProgressBar(0, f.size());

    QTextStream t(&f);
    QString s;
    QMap<QString, MyMoneyAccount> accounts;
    int line = 0;
    while ( !t.eof() ) {        // until end of file...
      s = t.readLine();       // line of text excluding '\n'
      if(codec) {
        s = codec->toUnicode(s);
      }
      ++line;

      // update progress bar every ten lines
      if(!(line % 10))
        kmymoney2->slotStatusProgressBar(f.at());

      if (!s.isEmpty() && s[0]!='#') {
        // first strip off any flags, which are delimited by "::"
        QStringList flags = QStringList::split("::",s);
        s = flags.front();
        flags.pop_front();

        MyMoneyAccount account, parentAccount;
        QString type, parent, child;
        QString msg;
        int pos1, pos2;
        // search the first and the last colon in the line
        // stuff before the first colon is the type (income/expense)
        // stuff after the last colon is the name of the account to be
        // created. stuff between them is the parent account. If pos1 == pos2
        // then the parent account is the standard account.
        pos1 = s.find(':');
        pos2 = s.findRev(':');
        if(pos1 == -1 || pos2 == -1) {
          qWarning("Format error in line %d of %s", line, filename.latin1());
          continue;
        }
        type = s.left(pos1).lower();

        if(type == "income") {
          account.setAccountType(MyMoneyAccount::Income);
          parentAccount = MyMoneyFile::instance()->income();

        } else if(type == "expense") {
          account.setAccountType(MyMoneyAccount::Expense);
          parentAccount = MyMoneyFile::instance()->expense();

        } else if(type == "codec") {
          QTextCodec* newCodec = QTextCodec::codecForName(s.mid(pos2+1));
          if(newCodec) {
            codec = newCodec;
          }
          continue;

        } else {
          QString msg("Unknown type '");
          msg += type + "' in line %d of " + filename;
          qWarning(msg, line);
          continue;
        }

        parent = s.left(pos2);

        if(pos1 != pos2) {
          QMap<QString, MyMoneyAccount>::ConstIterator it;
          it = accounts.find(parent);
          if(it == accounts.end()) {
            QString msg("Unknown parent account '");
            msg += parent + "' in line %d of " + filename;
            qWarning(msg, line);
            continue;
          }
          parentAccount = *it;
        }

        child = s.mid(pos2+1);
        account.setName(child);

        // process flags
        QStringList::const_iterator it_flag = flags.begin();
        while ( it_flag != flags.end() )
        {
          account.setValue((*it_flag).utf8(),"Yes");
          ++it_flag;
        }

        try {
          MyMoneyFile::instance()->addAccount(account, parentAccount);
          accounts[parent + ":" + child] = account;

        } catch(MyMoneyException *e) {

          QString msg("Unable to add account '");
          msg += account.name() + ": " + e->what();
          // qDebug(msg);
          delete e;
          continue;
        }
      }
    }
    kmymoney2->slotStatusMsg(i18n("Ready"));
    kmymoney2->slotStatusProgressBar(-1, -1);
    f.close();
  }
  MyMoneyFile::instance()->suspendNotify(false);
}

bool KMyMoneyView::parseDefaultCategory(QString& line, bool& income, QString& name, QStringList& minors)
{
  // Parse the argument line separating into 3 other arguments
  // if third is missing then assume no minor categories.
  if (line.isEmpty() || line.isNull())
    return false;

  QString buffer;
  unsigned int count=0;
  int tokenCount=0;
  bool done1=false, done2=false, done3=false, b_inEnclosed=false;

  QChar commentChar('#');

  QChar encloseChar('\"');
  QChar separatorChar(',');

  while (count <= line.length()) {
    if (line[count]==commentChar)
        return false;
    else if (count==line.length()) {
      tokenCount++;
      unsigned int inner_count=0;
      QString inner_buffer;
      while (inner_count <= buffer.length()) {
        if (buffer[inner_count]==separatorChar) {
          if (inner_buffer.length()>=1) {
            minors.append(inner_buffer);
            inner_count++;
            inner_buffer = QString::null;
          }
        }
        else if (inner_count==buffer.length()) {
          if (inner_buffer.length()>=1) {
            minors.append(inner_buffer);
          }
          break;
        }
        else
          inner_buffer += buffer[inner_count++];
      }
      done3=true;
      if (done1 && done2)
        return true;
      else
        return false;
    }
    else if (line[count].isSpace()) {
      if (!b_inEnclosed) {
        while (line[count].isSpace())
          count++;

        switch (tokenCount) {
          case 0: // income
            if (buffer.upper() == "TRUE")
              income = true;

            else if (buffer.upper() == "FALSE")

              income = false;

            else
              return false;

            tokenCount++;
            done1=true;
            buffer = QString::null;
            break;
          case 1: // name
            if (buffer.length()<=0)
              return false;


            name = buffer;
            tokenCount++;
            done2=true;
            buffer = QString::null;
            break;

          default:
            return false;
        }
      } else {
        if (line[count]==QChar('\n'))
          return false;
        buffer += ' ';
        count++;
      }
    } else if (line[count]==encloseChar) {
      if (b_inEnclosed)
        b_inEnclosed = false;
      else
        b_inEnclosed = true;
      count++;
    } else {
      buffer += line[count++];
    }
  }

  if (done1 && done2 && done3)
    return true;
  return false;
}
#endif

void KMyMoneyView::viewUp(void)
{
  if (!fileOpen())
    return;
}

void KMyMoneyView::viewAccountList(const QCString& /*selectAccount*/)
{
  if(pageIndex(m_accountsViewFrame) != activePageIndex())
    showPage(1);

  m_accountsView->show();
}

void KMyMoneyView::slotRefreshViews()
{
  // force update of settings
  KMyMoneyUtils::updateSettings();

  m_accountsView->slotRefreshView();
  m_categoriesView->slotRefreshView();
  m_ledgerView->slotRefreshView();
  m_payeesView->slotRefreshView();
  m_homeView->slotRefreshView();
  m_investmentView->slotRefreshView();
  m_reportsView->slotRefreshView();

  m_scheduledView->slotReloadView();
}

void KMyMoneyView::slotFindTransaction(void)
{
  if(m_searchDlg == 0) {
    m_searchDlg = new KFindTransactionDlg();
    connect(m_searchDlg, SIGNAL(destroyed()), this, SLOT(slotCloseSearchDialog()));
    connect(m_searchDlg, SIGNAL(transactionSelected(const QCString&, const QCString&)),
            this, SLOT(slotLedgerSelected(const QCString&, const QCString&)));
  }
  m_searchDlg->show();
  m_searchDlg->raise();
}

void KMyMoneyView::slotCloseSearchDialog(void)
{
  if(m_searchDlg)
    m_searchDlg->deleteLater();
  m_searchDlg = 0;
}

QString KMyMoneyView::currentAccountName(void)
{
  bool accountSuccess=false;
    MyMoneyFile* file = MyMoneyFile::instance();
    try
    {

      MyMoneyAccount account = file->account(m_accountsView->currentAccount(accountSuccess));
      if (accountSuccess)

        return account.name();
    }
    catch (MyMoneyException *e)
    {
      // Try an institution

      try
      {
        MyMoneyInstitution institution = file->institution(m_accountsView->currentInstitution(accountSuccess));
        if (accountSuccess)
          return institution.name();
      }
      catch (MyMoneyException *ex)
      {
        delete ex;
      }
      delete e;
    }


  return i18n("Unknown Account");
}

void KMyMoneyView::slotShowTransactionDetail(bool detailed)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  config->writeEntry("ShowRegisterDetailed", detailed);
  config->sync();

  slotRefreshViews();
}


void KMyMoneyView::memoryDump()
{
  QFile g( "kmymoney2.dump" );
  g.open( IO_WriteOnly );
  QDataStream st(&g);
  MyMoneyStorageDump dumper;
  dumper.writeStream(st, dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()));
  g.close();
}

void KMyMoneyView::slotCancelEdit(void) const
{
  if(m_ledgerView != 0)
    m_ledgerView->slotCancelEdit();
  if(m_investmentView != 0)
    m_investmentView->slotCancelEdit();
}

void KMyMoneyView::progressCallback(int current, int total, const QString& msg)
{
  kmymoney2->progressCallback(current, total, msg);
}

void KMyMoneyView::suspendUpdate(const bool suspend)
{
  m_accountsView->suspendUpdate(suspend);
  m_categoriesView->suspendUpdate(suspend);
  m_ledgerView->suspendUpdate(suspend);
  m_payeesView->suspendUpdate(suspend);
}

void KMyMoneyView::slotRememberPage(QWidget* w)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("LastViewSelected", pageIndex(w));
  config->sync();
}

void KMyMoneyView::slotBankAccountNew()
{
  m_bankRightClick = true;
  try
  {
    m_accountsInstitution = MyMoneyFile::instance()->institution(m_accountsView->currentInstitution(m_bankRightClick));
  }
  catch (MyMoneyException *e)
  {
    delete e;
  }
  accountNew(false);
}

void KMyMoneyView::fixFile(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyAccount> accountList = file->accountList();
  QValueList<MyMoneyAccount>::Iterator it_a;
  QValueList<MyMoneySchedule> scheduleList = file->scheduleList();
  QValueList<MyMoneySchedule>::Iterator it_s;

  for(it_a = accountList.begin(); it_a != accountList.end(); ++it_a) {
    fixOpeningBalance(*it_a);
    if((*it_a).accountType() == MyMoneyAccount::Loan
    || (*it_a).accountType() == MyMoneyAccount::AssetLoan) {
      fixLoanAccount(*it_a);
    }
  }

  for(it_s = scheduleList.begin(); it_s != scheduleList.end(); ++it_s) {
    fixSchedule(*it_s);
  }

  fixTransactions();
}

void KMyMoneyView::fixOpeningBalance(MyMoneyAccount& acc)
{
  if(!acc.openingBalance().isZero()) {
    try {
      MyMoneyFile::instance()->createOpeningBalanceTransaction(acc, acc.openingBalance());
      acc.setOpeningBalance(MyMoneyMoney(0, 1));
      MyMoneyFile::instance()->modifyAccount(acc);
    } catch(MyMoneyException *e) {
      qWarning("Unable to create opening balance transaction: %s", e->what().latin1());
      delete e;
    }
  }
}

void KMyMoneyView::fixSchedule(MyMoneySchedule sched)
{
  MyMoneyTransaction t = sched.transaction();
  QValueList<MyMoneySplit> splitList = t.splits();
  QValueList<MyMoneySplit>::ConstIterator it_s;
  bool updated = false;

  try {
    // Check if the splits contain valid data and set it to
    // be valid.
    for(it_s = splitList.begin(); it_s != splitList.end(); ++it_s) {
      if((*it_s).reconcileFlag() != MyMoneySplit::NotReconciled) {
        MyMoneySplit split = *it_s;
        split.setReconcileDate(QDate());
        split.setReconcileFlag(MyMoneySplit::NotReconciled);
        t.modifySplit(split);
        updated = true;
      }
    }

    // If there have been changes, update the schedule and
    // the engine data.
    if(updated) {
      sched.setTransaction(t);
      MyMoneyFile::instance()->modifySchedule(sched);
    }
  } catch(MyMoneyException *e) {
    qWarning("Unable to update broken schedule: %s", e->what().latin1());
    delete e;
  }
}

void KMyMoneyView::fixLoanAccount(MyMoneyAccount acc)
{
  if(acc.value("final-payment").isEmpty()
  || acc.value("term").isEmpty()
  || acc.value("periodic-payment").isEmpty()
  || acc.value("loan-amount").isEmpty()
  || acc.value("interest-calculation").isEmpty()
  || acc.value("schedule").isEmpty()
  || acc.value("fixed-interest").isEmpty()) {
    KMessageBox::information(this,
        i18n("The account \"%1\" was previously created as loan account but some information "
             "is missing. The new loan wizard will be started to collect all relevant "
             "information. If you cancel the wizard, then the file will be "
             "closed.").arg(acc.name()),
        i18n("Account problem"));
    KNewLoanWizard* wiz = new KNewLoanWizard(this);
    wiz->loadWidgets(acc);
    if(wiz->exec() == QDialog::Accepted) {
      MyMoneyAccount newAcc = wiz->account();
      acc.setAccountType(newAcc.accountType());
      acc.setName(newAcc.name());
      acc.setNumber(QString());
      acc.setOpeningBalance(newAcc.openingBalance());
      acc.setOpeningDate(newAcc.openingDate());
      acc.setPairs(newAcc.pairs());

      try {
        MyMoneyFile::instance()->modifyAccount(acc);
        createSchedule(wiz->schedule(), acc);

      } catch(MyMoneyException *e) {
        delete e;
        qDebug("Unable to update loan account and/or create schedule");
      }
    } else
      throw new MYMONEYEXCEPTION("Fix failed");
    delete wiz;
  }
}

void KMyMoneyView::createSchedule(MyMoneySchedule newSchedule, MyMoneyAccount& newAccount)
{
  // Add the schedule only if one exists
  //
  // Remember to modify the first split to reference the newly created account
  if (!newSchedule.name().isEmpty())
  {
    try
    {
      // We assume at least 2 splits in the transaction
      MyMoneyTransaction t = newSchedule.transaction();
      if(t.splitCount() < 2) {
        throw new MYMONEYEXCEPTION("Transaction for schedule has less than 2 splits!");
      }
      // now search the split that does not have an account reference
      // and set it up to be the one of the account we just added
      // to the account pool. Note: the schedule code used to leave
      // this always the first split, but the loan code leaves it as
      // the second one. So I thought, searching is a good alternative ....
      QValueList<MyMoneySplit>::ConstIterator it_s;
      for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
        if((*it_s).accountId().isEmpty()) {
          MyMoneySplit s = (*it_s);
          s.setAccountId(newAccount.id());
          t.modifySplit(s);
          break;
        }
      }
      newSchedule.setTransaction(t);

      MyMoneyFile::instance()->addSchedule(newSchedule);

      // in case of a loan account, we keep a reference to this
      // schedule in the account
      if(newAccount.accountType() == MyMoneyAccount::Loan
      || newAccount.accountType() == MyMoneyAccount::AssetLoan) {
        newAccount.setValue("schedule", newSchedule.id());
        MyMoneyFile::instance()->modifyAccount(newAccount);
      }
    }
    catch (MyMoneyException *e)
    {
      KMessageBox::information(this, i18n("Unable to add schedule: "), e->what());
      delete e;
    }
  }
}

void KMyMoneyView::fixTransactions(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QValueList<MyMoneySchedule> scheduleList = file->scheduleList();
  MyMoneyTransactionFilter filter;
  QValueList<MyMoneyTransaction> transactionList = file->transactionList(filter);

  QValueList<MyMoneySchedule>::Iterator it_x;
  QCStringList interestAccounts;

  kmymoney2->slotStatusMsg(i18n("Fix transactions"));
  kmymoney2->slotStatusProgressBar(0, scheduleList.count() + transactionList.count());

  int cnt = 0;
  // scan the schedules to find interest accounts
  for(it_x = scheduleList.begin(); it_x != scheduleList.end(); ++it_x) {
    MyMoneyTransaction t = (*it_x).transaction();
    QValueList<MyMoneySplit>::ConstIterator it_s;
    QCStringList accounts;
    bool hasDuplicateAccounts = false;

    for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
      if(accounts.contains((*it_s).accountId())) {
        hasDuplicateAccounts = true;
      } else {
        accounts << (*it_s).accountId();
      }

      if((*it_s).action() == MyMoneySplit::ActionInterest) {
        if(interestAccounts.contains((*it_s).accountId()) == 0) {
          interestAccounts << (*it_s).accountId();
        }
      }
    }
    if(hasDuplicateAccounts) {
      fixDuplicateAccounts(t);
    }
    ++cnt;
    if(!(cnt % 10))
      kmymoney2->slotStatusProgressBar(cnt);
  }

  // scan the transactions and modify loan transactions
  QValueList<MyMoneyTransaction>::Iterator it_t;
  for(it_t = transactionList.begin(); it_t != transactionList.end(); ++it_t) {
    const char *defaultAction = 0;
    QValueList<MyMoneySplit> splits = (*it_t).splits();
    QValueList<MyMoneySplit>::Iterator it_s;
    QCStringList accounts;

    // check if base commodity is set. if not, set baseCurrency
    if((*it_t).commodity().isEmpty()) {
      (*it_t).setCommodity(file->baseCurrency().id());
      file->modifyTransaction(*it_t);
    }

    bool isLoan = false;
    // Determine default action
    if((*it_t).splitCount() == 2) {
      // check for transfer
      int accountCount = 0;
      MyMoneyMoney val;
      for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
        MyMoneyAccount acc = file->account((*it_s).accountId());
        if(acc.accountGroup() == MyMoneyAccount::Asset
        || acc.accountGroup() == MyMoneyAccount::Liability) {
          val = (*it_s).value();
          accountCount++;
          if(acc.accountType() == MyMoneyAccount::Loan
          || acc.accountType() == MyMoneyAccount::AssetLoan)
            isLoan = true;
        } else
          break;
      }
      if(accountCount == 2) {
        if(isLoan)
          defaultAction = MyMoneySplit::ActionAmortization;
        else
          defaultAction = MyMoneySplit::ActionTransfer;
      } else {
        if(val.isNegative())
          defaultAction = MyMoneySplit::ActionWithdrawal;
        else
          defaultAction = MyMoneySplit::ActionDeposit;
      }
    }

    isLoan = false;
    for(it_s = splits.begin(); defaultAction == 0 && it_s != splits.end(); ++it_s) {
      MyMoneyAccount acc = file->account((*it_s).accountId());
      MyMoneyMoney val = (*it_s).value();
      if(acc.accountGroup() == MyMoneyAccount::Asset
      || acc.accountGroup() == MyMoneyAccount::Liability) {
        if(!val.isPositive())
          defaultAction = MyMoneySplit::ActionWithdrawal;
        else
          defaultAction = MyMoneySplit::ActionDeposit;
      }
    }

    // Check for correct actions in transactions referencing credit cards
    bool needModify = false;
#if 0
    // The action fields are actually not used anymore in the ledger view logic
    // so we might as well skip this whole thing here!
    for(it_s = splits.begin(); needModify == false && it_s != splits.end(); ++it_s) {
      MyMoneyAccount acc = file->account((*it_s).accountId());
      MyMoneyMoney val = (*it_s).value();
      if(acc.accountType() == MyMoneyAccount::CreditCard) {
        if(val < 0 && (*it_s).action() != MyMoneySplit::ActionWithdrawal && (*it_s).action() != MyMoneySplit::ActionTransfer )
          needModify = true;
        if(val >= 0 && (*it_s).action() != MyMoneySplit::ActionDeposit && (*it_s).action() != MyMoneySplit::ActionTransfer)
          needModify = true;
      }
    }
#endif
    if(needModify == true) {
      for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
        (*it_s).setAction(defaultAction);
        (*it_t).modifySplit(*it_s);
        file->modifyTransaction(*it_t);
      }
      splits = (*it_t).splits();    // update local copy
      qDebug("Fixed credit card assignment in %s", (*it_t).id().data());
    }

    bool hasDuplicateAccounts = false;
    // Check for correct assignment of ActionInterest in all splits
    // and check if there are any duplicates in this transactions
    for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
      MyMoneyAccount splitAccount = file->account((*it_s).accountId());
      if(accounts.contains((*it_s).accountId())) {
        hasDuplicateAccounts = true;
      } else {
        accounts << (*it_s).accountId();
      }
      // if this split references an interest account, the action
      // must be of type ActionInterest
      if(interestAccounts.contains((*it_s).accountId())) {
        if((*it_s).action() != MyMoneySplit::ActionInterest) {
          (*it_s).setAction(MyMoneySplit::ActionInterest);
          (*it_t).modifySplit(*it_s);
          file->modifyTransaction(*it_t);
          qDebug("Fixed interest action in %s", (*it_t).id().data());
        }
      // if it does not reference an interest account, it must not be
      // of type ActionInterest
      } else {
        if((*it_s).action() == MyMoneySplit::ActionInterest) {
          (*it_s).setAction(defaultAction);
          (*it_t).modifySplit(*it_s);
          file->modifyTransaction(*it_t);
          qDebug("Fixed interest action in %s", (*it_t).id().data());
        }
      }

      // check that for splits referencing an account that has
      // the same currency as the transactions commodity the value
      // and shares field are the same.
      if((*it_t).commodity() == splitAccount.currencyId()
      && (*it_s).value() != (*it_s).shares()) {
        (*it_s).setShares((*it_s).value());
        (*it_t).modifySplit(*it_s);
        file->modifyTransaction(*it_t);
      }
    }

/*
    // if there are at least two splits referencing the same account,
    // we need to combine them into one and get rid of the others
    if(hasDuplicateAccounts) {
      fixDuplicateAccounts(*it_t);
    }
*/
    ++cnt;
    if(!(cnt % 10))
      kmymoney2->slotStatusProgressBar(cnt);
  }

  kmymoney2->slotStatusProgressBar(-1, -1);
  kmymoney2->slotStatusMsg(i18n("Ready"));
}

void KMyMoneyView::fixDuplicateAccounts(MyMoneyTransaction& t)
{
  qDebug("Duplicate account in transaction %s", t.id().data());
}

void KMyMoneyView::slotPrintView(void)
{
  if(pageIndex(m_reportsViewFrame) == activePageIndex())
    m_reportsView->slotPrintView();
}

KMyMoneyViewBase* KMyMoneyView::addPage(const QString& title, const QPixmap& pixmap)
{
  QFrame* frm = KJanusWidget::addVBoxPage(title, title, pixmap);
  return new KMyMoneyViewBase(frm, title.latin1(), title);
}

/* ------------------------------------------------------------------------ */
/*                 KMyMoneyViewBase                                         */
/* ------------------------------------------------------------------------ */

// ----------------------------------------------------------------------------
// QT Includes

#include <qlayout.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../widgets/kmymoneytitlelabel.h"

KMyMoneyViewBase::KMyMoneyViewBase(QWidget* parent, const char* name, const QString& title) :
  QWidget(parent, name)
{
  m_viewLayout = new QVBoxLayout(this);
  m_viewLayout->setSpacing( 6 );
  m_viewLayout->setMargin( 11 );

  m_titleLabel = new kMyMoneyTitleLabel( this, "titleLabel" );
  m_titleLabel->setMinimumSize( QSize( 100, 30 ) );
  m_titleLabel->setRightImageFile("pics/titlelabel_background.png" );
  m_titleLabel->setText(title);
  m_viewLayout->addWidget( m_titleLabel );

  QFrame* titleLine = new QFrame( this, "titleLine" );
  titleLine->setFrameShape( QFrame::HLine );
  titleLine->setFrameShadow( QFrame::Sunken );
  titleLine->setFrameShape( QFrame::HLine );
  m_viewLayout->addWidget( titleLine );
}

void KMyMoneyViewBase::setTitle(const QString& title)
{
  m_titleLabel->setText(title);
}

void KMyMoneyViewBase::addWidget(QWidget* w)
{
  m_viewLayout->addWidget(w);
}
