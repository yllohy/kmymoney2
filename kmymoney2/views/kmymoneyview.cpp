/***************************************************************************
                          kmymoneyview.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// #include <stdio.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qlabel.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qprogressdialog.h>

#if QT_VERSION > 300
#include <qcursor.h>
#include <qregexp.h>
#endif

// ----------------------------------------------------------------------------
// KDE Includes

#include <kfiledialog.h>
#include <kglobal.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#include <kicontheme.h>
#include <kiconloader.h>
#else
#include <kstddirs.h>
#endif

#include <kmessagebox.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <ktempfile.h>
#include <ksavefile.h>
#include <kfilterdev.h>
#include <kfilterbase.h>

// ----------------------------------------------------------------------------
// Project Includes

// This is include is required here, because later it will produce
// compile errors on gcc 3.2 as we redefine new() in case of _CHECK_MEMORY
// being defined. To avoid these problems, we just include the header
// already here in this case
#ifdef _CHECK_MEMORY
#include <string>
#endif

#include <../dialogs/knewaccountwizard.h>

#include "../dialogs/knewbankdlg.h"
#include "../dialogs/knewaccountdlg.h"
#include "../dialogs/kendingbalancedlg.h"
#include "../dialogs/knewfiledlg.h"
#include "../dialogs/kchooseimportexportdlg.h"
#include "../dialogs/kcsvprogressdlg.h"
#include "../dialogs/kimportdlg.h"
#include "../dialogs/kexportdlg.h"
#include "../dialogs/knewloanwizard.h"

#include "../mymoney/storage/mymoneyseqaccessmgr.h"
#include "../mymoney/storage/imymoneystorageformat.h"
#include "../mymoney/storage/mymoneystoragebin.h"
#include "../mymoney/mymoneyexception.h"
#include "../mymoney/storage/mymoneystoragedump.h"
#include "../mymoney/storage/mymoneystoragexml.h"

#include "kmymoneyview.h"
#include "kbanksview.h"
#include "khomeview.h"
#include "kcategoriesview.h"
#include "kpayeesview.h"
#include "kscheduledview.h"
#include "kgloballedgerview.h"

#include "../kmymoney2.h"

#define COMPRESSION_MIME_TYPE "application/x-gzip"

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

  // Page 0
  m_homeViewFrame = addVBoxPage( i18n("Home"), i18n("Home"),
    DesktopIcon("home"));
  m_homeView = new KHomeView(m_homeViewFrame);
  connect(m_homeView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedHomePage()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_homeView, SLOT(slotReloadView()));

  // Page 1
  m_accountsViewFrame = addVBoxPage( i18n("Accounts"), i18n("Insitutions/Accounts"),
    DesktopIcon("kmy"));
  m_accountsView = new KAccountsView(m_accountsViewFrame, "accountsView");
  connect(m_accountsView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedAccountsView()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_accountsView, SLOT(slotReloadView()));

  // Page 2
  m_scheduleViewFrame = addVBoxPage( i18n("Schedule"), i18n("Bills & Reminders"),
    DesktopIcon("schedule"));
  m_scheduledView = new KScheduledView(m_scheduleViewFrame, "scheduledView");
  connect(m_scheduledView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedScheduledView()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_scheduledView, SLOT(slotReloadView()));

  // Page 3
  m_categoriesViewFrame = addVBoxPage( i18n("Categories"), i18n("Categories"),
    DesktopIcon("categories"));
  m_categoriesView = new KCategoriesView(m_categoriesViewFrame, "categoriesView");
  connect(m_categoriesView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedCategoriesView()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_categoriesView, SLOT(slotReloadView()));

  // Page 4
  m_payeesViewFrame = addVBoxPage( i18n("Payees"), i18n("Payees"),
    DesktopIcon("payee"));
  m_payeesView = new KPayeesView(m_payeesViewFrame, "payeesView");
  connect(m_payeesView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedPayeeView()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_payeesView, SLOT(slotReloadView()));

  // Page 5
  m_ledgerViewFrame = addVBoxPage( i18n("Ledgers"), i18n("Ledgers"),
    DesktopIcon("ledger"));
  m_ledgerView = new KGlobalLedgerView(m_ledgerViewFrame, "ledgerView");
  // the next line causes the ledgers to get a hide() signal to be able
  // to end any pending edit activities
  connect(this, SIGNAL(aboutToShowPage(QWidget*)), m_ledgerView, SLOT(hide()));
  connect(m_ledgerView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedAccountView()));
  connect(kmymoney2, SIGNAL(fileLoaded(const KURL&)), m_ledgerView, SLOT(slotReloadView()));
  
/*
  m_investmentView = new KInvestmentView(qvboxMainFrame2, "investmentView");

  // Need to show it, although the user wont see it.
  // At the bottom of this method we choose what to show.

  accountsView->show();
  transactionView->hide();
  m_investmentView->hide();
*/

  connect(m_accountsView, SIGNAL(accountRightMouseClick()),
    this, SLOT(slotAccountRightMouse()));
  connect(m_accountsView, SIGNAL(accountDoubleClick()), this, SLOT(slotAccountDoubleClick()));
  //connect(accountsView, SIGNAL(accountSelected()), this, SLOT(slotAccountSelected()));
  connect(m_accountsView, SIGNAL(bankRightMouseClick()),
    this, SLOT(slotBankRightMouse()));
  connect(m_accountsView, SIGNAL(rightMouseClick()),
    this, SLOT(slotRightMouse()));


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
  
/*
  connect(transactionView, SIGNAL(viewTypeSearchActivated()),
    this, SLOT(accountFind()));
  connect(transactionView, SIGNAL(viewTypeNormalActivated()),
    this, SLOT(viewTransactionList()));
*/

/*
  m_inReconciliation=false;
  m_reconcileInited=false;
  reconcileDlg=0;
  transactionFindDlg=0;
*/
  m_newAccountWizard = new KNewAccountWizard(this, "NewAccountWizard");
  connect(m_newAccountWizard, SIGNAL(newInstitutionClicked()), this, SLOT(slotBankNew()));



  // construct account context menu
  KIconLoader *kiconloader = KGlobal::iconLoader();

  m_accountMenu = new KPopupMenu(this);
  m_accountMenu->insertTitle(kiconloader->loadIcon("account", KIcon::MainToolbar), i18n("Account Options"));
  m_accountMenu->insertItem(kiconloader->loadIcon("account", KIcon::Small), i18n("New account..."), this, SLOT(slotAccountNew()), 0, AccountNew);
  m_accountMenu->insertItem(kiconloader->loadIcon("account_open", KIcon::Small), i18n("Open..."), this, SLOT(slotAccountDoubleClick()), 0, AccountOpen);
  m_accountMenu->insertSeparator();
  m_accountMenu->insertItem(kiconloader->loadIcon("reconcile", KIcon::Small), i18n("Reconcile..."), this, SLOT(slotAccountReconcile()), 0, AccountReconcile);
  m_accountMenu->insertSeparator();
  m_accountMenu->insertItem(kiconloader->loadIcon("account", KIcon::Small), i18n("Edit..."), this, SLOT(slotAccountEdit()), 0, AccountEdit);
  m_accountMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small), i18n("Delete..."), this, SLOT(slotAccountDelete()), 0, AccountDelete);

  m_bankMenu = new KPopupMenu(this);
  m_bankMenu->insertTitle(kiconloader->loadIcon("bank", KIcon::MainToolbar), i18n("Institution Options"));
  // Use a proxy slot
  m_bankMenu->insertItem(kiconloader->loadIcon("account", KIcon::Small), i18n("New Account..."), this, SLOT(slotBankAccountNew()));
  m_bankMenu->insertItem(kiconloader->loadIcon("bank", KIcon::Small), i18n("Edit..."), this, SLOT(slotBankEdit()));
  m_bankMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small), i18n("Delete..."), this, SLOT(slotBankDelete()));
  
  m_rightMenu = new KPopupMenu(this);
  m_rightMenu->insertTitle(kiconloader->loadIcon("bank", KIcon::MainToolbar), i18n("KMyMoney Options"));
  m_rightMenu->insertItem(kiconloader->loadIcon("bank", KIcon::Small), i18n("New Institution..."), this, SLOT(slotBankNew()));
  
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
  else
    acc = m_categoriesView->currentAccount(ok);

  // turn off all available options in the menu except New
  m_accountMenu->setItemEnabled(AccountNew, true);
  m_accountMenu->setItemEnabled(AccountOpen, false);
  m_accountMenu->setItemEnabled(AccountEdit, false);
  m_accountMenu->setItemEnabled(AccountReconcile, false);
  m_accountMenu->setItemEnabled(AccountDelete, false);

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
            m_accountMenu->setItemEnabled(AccountDelete, true);
          }
          m_accountMenu->changeItem(AccountNew, i18n("New account..."));
          m_accountMenu->connectItem(AccountNew, this, SLOT(slotAccountNew()));
          break;
        case MyMoneyAccount::Income:
        case MyMoneyAccount::Expense:
          if(!file->isStandardAccount(acc)) {
            m_accountMenu->setItemEnabled(AccountEdit, true);
            m_accountMenu->setItemEnabled(AccountDelete, true);
          }
          m_accountMenu->changeItem(AccountNew, i18n("New category..."));
          m_accountMenu->connectItem(AccountNew, this, SLOT(slotCategoryNew()));
          break;

        default:
          break;
      }
    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception in KMyMoneyView::slotAccountRightMouse");
      delete e;
    }
  }
  m_accountMenu->exec(QCursor::pos());
}

void KMyMoneyView::slotLedgerSelected(const QCString& acc, const QCString& transaction)
{
  showPage(pageIndex(m_ledgerViewFrame));
  m_ledgerView->slotSelectAccountAndTransaction(acc, transaction);
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

void KMyMoneyView::slotAccountDoubleClick(void)
{
  bool  ok = false;
  QCString acc;

  if(pageIndex(m_accountsViewFrame) == activePageIndex())
    acc = m_accountsView->currentAccount(ok);
  else
    acc = m_categoriesView->currentAccount(ok);

  if(ok == true) {
    showPage(pageIndex(m_ledgerViewFrame));
    m_ledgerView->slotSelectAccount(acc);
  }
}

void KMyMoneyView::slotBankRightMouse()
{
  int editId = m_bankMenu->idAt(2);
  int deleteId = m_bankMenu->idAt(3);
  bool bankSuccess;
  bool state = !m_accountsView->currentInstitution(bankSuccess).isEmpty();

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
    MyMoneyInstitution institution = file->institution(m_accountsView->currentInstitution(bankSuccess));

    // bankSuccess is not checked anymore because m_file->institution will throw anyway
    KNewBankDlg dlg(institution, true, this, "edit_bank");
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
    MyMoneyInstitution institution = file->institution(m_accountsView->currentInstitution(bankSuccess));
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

    if(pageIndex(m_accountsViewFrame) == activePageIndex()) {
      QCString accountId = m_accountsView->currentAccount(accountSuccess);
      if(!accountId.isEmpty()) {
        account = file->account(accountId);
        switch(MyMoneyAccount::accountGroup(account.accountType())) {
          case MyMoneyAccount::Asset:
          case MyMoneyAccount::Liability:
            m_accountsView->slotEditClicked();
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

    if(pageIndex(m_accountsViewFrame) == activePageIndex()) {
      QCString accountId = m_accountsView->currentAccount(accountSuccess);
      if(!accountId.isEmpty()) {
        account = file->account(accountId);
        switch(MyMoneyAccount::accountGroup(account.accountType())) {
          case MyMoneyAccount::Asset:
          case MyMoneyAccount::Liability:
            m_accountsView->slotDeleteClicked();
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

bool KMyMoneyView::fileOpen(void)
{
  return m_fileOpen;
}

void KMyMoneyView::closeFile(void)
{
  newStorage();
  m_fileOpen = false;
}

bool KMyMoneyView::readFile(const KURL& url)
{
  QString filename;

  newStorage();
  m_fileOpen = true;

  IMyMoneyStorageFormat* pReader = NULL;    

  if(url.isMalformed()) {
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
  QIODevice *qfile = 0;

  if(file.open(IO_ReadOnly)) {
    QByteArray hdr(2);
    int cnt;
    cnt = file.readBlock(hdr.data(), 2);
    file.close();

    if(cnt == 2) {
      if(QString(hdr) == QString("\037\213")) {         // gzipped?
        qfile = KFilterDev::deviceForFile(filename, COMPRESSION_MIME_TYPE);
      } else {
        // we can't use file directly, as we delete qfile later on
        qfile = new QFile(file.name());
      }

      if(qfile->open(IO_ReadOnly)) {
        try {
          hdr.resize(8);
          if(qfile->readBlock(hdr.data(), 8) == 8) {
            // Ok, we got the first block of 8 bytes. Read in the two
            // unsigned long int's by preserving endianess. This is
            // achieved by reading them through a QDataStream object
            Q_INT32 magic0, magic1;
            QDataStream s(hdr, IO_ReadOnly);
            s >> magic0;
            s >> magic1;

            // If both magic numbers match (we actually read in the
            // text 'KMyMoney2' then we assume a binary file and
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
            else
              pReader = new MyMoneyStorageXML;

            // rewind the file to give the reader a chance              
            qfile->at(0);            
            pReader->setProgressCallback(&KMyMoneyView::progressCallback);
            pReader->readFile(qfile, dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()));
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
      }
      delete qfile;
    }
  } else {
    KMessageBox::sorry(this, i18n("File '%1' not found!").arg(filename));
  }


  // if a temporary file was constructed by NetAccess::download,
  // then it will be removed with the next call. Otherwise, it
  // stays untouched on the local filesystem
  KIO::NetAccess::removeTempFile(filename);

  // since the new account wizard contains the payees list, we have
  // to create the wizard completely new to get the latest list
  // into the payees widget
  if(m_newAccountWizard != 0)
    delete m_newAccountWizard;
  m_newAccountWizard = new KNewAccountWizard(this, "NewAccountWizard");
  connect(m_newAccountWizard, SIGNAL(newInstitutionClicked()), this, SLOT(slotBankNew()));
  
  KConfig *config = KGlobal::config();
  int page;
  config->setGroup("General Options");
  if(config->readBoolEntry("StartLastViewSelected", false) == true) {
    config->setGroup("Last Use Settings");
    page = config->readNumEntry("LastViewSelected", 0);
  } else {
    page = pageIndex(m_homeViewFrame);
  }

  // if we currently see a different page, then select the right one
  if(page != activePageIndex()) {
    showPage(page);
  }

  return true;
}

void KMyMoneyView::saveToLocalFile(QFile* qfile, IMyMoneyStorageFormat* pWriter)
{
  QIODevice *dev = qfile;
  KFilterBase *base = 0;

  KConfig *config = KGlobal::config();
  config->setGroup("General Options");

  try {
    if(config->readBoolEntry("WriteDataUncompressed", false) == false) {
      base = KFilterBase::findFilterByMimeType( COMPRESSION_MIME_TYPE );
      if(base) {
        base->setDevice(qfile, false);
        qfile->close();
        // we need to reopen the file to set the mode inside the filter stuff
        dev = new KFilterDev(base, true);
        dev->open(IO_WriteOnly);
      }
    }

    pWriter->setProgressCallback(&KMyMoneyView::progressCallback);
    pWriter->writeFile(dev, dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()));
    pWriter->setProgressCallback(0);

  } catch (MyMoneyException *e) {
    QString msg = e->what();
    qDebug("%s", msg.latin1());
    delete e;
  }
  if(base != 0) {
    dev->flush();
    dev->close();
    delete dev;
  } else
    qfile->close();
}

void KMyMoneyView::saveFile(const KURL& url)
{
  QString filename = url.path();

  if (!fileOpen()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }

  if(KMessageBox::warningContinueCancel(this, i18n(
      "Since this version of KMyMoney2 only writes data files in it's new "
      "format, files written with this version cannot be read by older versions "
      "of KMyMoney2. If "
      "you still want to use prior versions of KMyMoney2 with your data files, "
      "please make sure you keep a backup-file of your finance data. "
      "If you want to abort this operation, please press Cancel now"),
      QString::null, KStdGuiItem::cont(), "WarningNewFileVersion0.5") == KMessageBox::Cancel)
    return;

  IMyMoneyStorageFormat* pWriter = NULL;

  QString strFileExtension = MyMoneyUtils::getFileExtension(filename);

  if(strFileExtension.find("XML") != -1)
  {
    pWriter = new MyMoneyStorageXML;
  }
  else
  {
    // Use the binary reader 
    pWriter = new MyMoneyStorageBin;
  }

  // actually, url should be the parameter to this function
  // but for now, this would involve too many changes

  if(url.isMalformed()) {
    qDebug("Invalid URL '%s'", url.url().latin1());
    return;
  }

  if(url.isLocalFile()) {
    filename = url.path();
    KSaveFile qfile(filename);
    if(qfile.status() == 0) {
      saveToLocalFile(qfile.file(), pWriter);
    }
  } else {
    KTempFile tmpfile;
    saveToLocalFile(tmpfile.file(), pWriter);
    KIO::NetAccess::upload(tmpfile.name(), url);
    tmpfile.unlink();
  }

  delete pWriter;
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
  
  KNewBankDlg dlg(institution, false, this, "newbankdlg");
  if (dlg.exec())
  {
    try
    {
      MyMoneyFile* file = MyMoneyFile::instance();

      institution = dlg.institution();

      file->addInstitution(institution);

      // Set the institution member of KNewAccountWizard
      if (m_newAccountWizard->isVisible())
        m_newAccountWizard->setInstitution(institution);
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
  int dialogResult;

  // KConfig *config = KGlobal::config();
  // config->setGroup("General Options");
  if(/* config->readBoolEntry("NewAccountWizard", true) == true && */ createCategory == false) {
    // wizard selected
    m_newAccountWizard->setAccountName(QString());
    m_newAccountWizard->setOpeningBalance(0);

    // Preselect the institution if we right clicked on a bank
    if (m_bankRightClick)
      m_newAccountWizard->setInstitution(m_accountsInstitution);
      
    if((dialogResult = m_newAccountWizard->exec()) == QDialog::Accepted) {
      newAccount = m_newAccountWizard->account();
      parentAccount = m_newAccountWizard->parentAccount();
    }
  } else {
    // regular dialog selected
    MyMoneyAccount account;
    QString title;
    QCString accId;
    bool ok;

    if(pageIndex(m_accountsViewFrame) == activePageIndex())
      accId = m_accountsView->currentAccount(ok);
    else if(pageIndex(m_categoriesViewFrame) == activePageIndex())
      accId = m_categoriesView->currentAccount(ok);

    if(ok) {
      try {
        MyMoneyFile* file = MyMoneyFile::instance();
        file->account(accId);
        account.setParentAccountId(accId);
      } catch(MyMoneyException *e) {
        delete e;
      }
    }
            
    if(createCategory == false)
      title = i18n("Create a new Account");
    else
      title = i18n("Create a new Category");
    KNewAccountDlg dialog(account, false, createCategory, 0, "hi", title);

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
      if (openingBal > 0 && newAccount.accountGroup() == MyMoneyAccount::Liability)
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

      // Add the credit card schedule only if one exists
      // We MUST add the schedule AFTER adding the account because
      // otherwise an unknown account will be thrown.
      //
      // Remember to modify the first split to to reference the newly created account
      MyMoneySchedule newSchedule = m_newAccountWizard->schedule();
      if (!newSchedule.name().isEmpty())
      {
        try
        {
          // We can guarantee 2 splits.
          MyMoneyTransaction t = newSchedule.transaction();
          MyMoneySplit s1 = t.splits()[0];
          s1.setAccountId(MyMoneyFile::instance()->nameToAccount(newAccount.name()));
          t.modifySplit(s1);          
          newSchedule.setTransaction(t);
          
          MyMoneyFile::instance()->addSchedule(newSchedule);
        } catch (MyMoneyException *e)
        {
          KMessageBox::information(this, i18n("Unable to add schedule: "), e->what());
          delete e;
        }
      }
        
      viewAccountList(newAccount.id());
    }
    catch (MyMoneyException *e)
    {
      QString message("Unable to add account: ");
      message += e->what();
      KMessageBox::information(this, message);
      delete e;
    }
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

void KMyMoneyView::newFile(void)
{
  closeFile();

  MyMoneyFile *file = MyMoneyFile::instance();

  KNewFileDlg newFileDlg(this, "e", i18n("Create new KMyMoneyFile"));
  if (newFileDlg.exec())
  {
    file->setUserName(newFileDlg.userNameText);
    file->setUserStreet(newFileDlg.userStreetText);
    file->setUserTown(newFileDlg.userTownText);
    file->setUserCounty(newFileDlg.userCountyText);
    file->setUserPostcode(newFileDlg.userPostcodeText);
    file->setUserTelephone(newFileDlg.userTelephoneText);
    file->setUserEmail(newFileDlg.userEmailText);

    loadDefaultCategories();
    m_fileOpen = true;

  }
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
    file->userEmail(), this, "e", i18n("Edit Personal Data"));

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

void KMyMoneyView::loadDefaultCategories(void)
{
  KFileDialog* dialog = new KFileDialog(KGlobal::dirs()->findResourceDir("appdata", "default_accounts_enC.dat"),
                                        i18n("*.dat|Account templates"),
                                        this, i18n("Select account template"),
                                        true);
  dialog->setMode(KFile::File | KFile::ExistingOnly | KFile::LocalOnly);
  if(dialog->exec()) {
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

  m_categoriesView->suspendUpdate(true);
  QFile f(filename);
  if (f.open(IO_ReadOnly) ) {
    kmymoney2->slotStatusMsg(i18n("Loading default accounts"));
    kmymoney2->slotStatusProgressBar(0, f.size());

    QTextStream t(&f);
    QString s;
    QMap<QString, MyMoneyAccount> accounts;
    int line = 0;
    while ( !t.eof() ) {        // until end of file...
      s = t.readLine();       // line of text excluding '\n'
      ++line;

      // update progress bar every ten lines
      if(!(line % 10))
        kmymoney2->slotStatusProgressBar(f.at());

      if (!s.isEmpty() && s[0]!='#') {
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
        try {
          MyMoneyFile::instance()->addAccount(account, parentAccount);
          accounts[parent + ":" + child] = account;

        } catch(MyMoneyException *e) {

          QString msg("Unable to add account '");
          msg += account.name() + ": " + e->what();
          qDebug(msg);
          delete e;
          continue;
        }
      }
    }
    kmymoney2->slotStatusMsg(i18n("Ready"));
    kmymoney2->slotStatusProgressBar(-1, -1);
    f.close();
  }
  m_categoriesView->suspendUpdate(false);
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

void KMyMoneyView::viewTransactionList(void)
{
/*
  bool accountSuccess=false;

  try
  {
    MyMoneyAccount account = MyMoneyFile::instance()->account(
        accountsView->currentAccount(accountSuccess));

    //set up stock account view
    if(account.accountType() == MyMoneyAccount::Investment)
    {
      accountsView->hide();
      transactionView->hide();
      m_investmentView->show();
      m_showing = InvestmentList;
      //m_investmentView->init(account.id());
    }
    else
    {
      m_showing = TransactionList;


      m_investmentView->hide();

      accountsView->hide();
      transactionView->show();

      bool readOnly=false;
      MyMoneyAccount::accountTypeE type = MyMoneyFile::instance()->accountGroup(account.accountType());
      if (type == MyMoneyAccount::Income || type == MyMoneyAccount::Expense)
        readOnly = true;
      transactionView->init(account.id(), readOnly);
    }
  }
  catch (MyMoneyException *e)
  {
    if (accountSuccess)
    {
      KMessageBox::information(this, i18n("Unable to query account to view the transaction list"));
    }
    delete e;

  }
*/
}

void KMyMoneyView::slotRefreshViews()
{
  m_accountsView->slotRefreshView();
  m_categoriesView->slotRefreshView();
  m_ledgerView->slotRefreshView();
  m_payeesView->slotRefreshView();
  m_homeView->slotRefreshView();

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
/*
  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

  pBank = m_file.bank(accountsView->currentBank(bankSuccess));
  if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::doTransactionSearch: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(accountsView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::doTransactionSearch: Unable to grab the current account");
    return;
  }

   bool doDate, doAmount, doCredit, doStatus, doDescription, doNumber, doPayee, doCategory;
   QString amountID, creditID, statusID, description, number, payee, category;
  MyMoneyMoney money;
  QDate startDate;
  QDate endDate;
  bool descriptionRegExp, numberRegExp, payeeRegExp;

  transactionFindDlg->data(doDate, doAmount, doCredit, doStatus, doDescription, doNumber, doPayee, doCategory,
    amountID,
    creditID,
    statusID,
    description,
    number,
    money,
    startDate,
    endDate,
    payee,
    category,
    descriptionRegExp,

    numberRegExp,
    payeeRegExp );


  MyMoneyTransaction *transaction;
  m_transactionList.clear();
  for ( transaction=pAccount->transactionFirst(); transaction; transaction=pAccount->transactionNext()) {
    if (checkTransactionDates(transaction, doDate, startDate, endDate) &&
      checkTransactionAmount(transaction, doAmount, amountID, money) &&
      checkTransactionCredit(transaction, doCredit, creditID) &&
      checkTransactionStatus(transaction, doStatus, statusID) &&
      checkTransactionDescription(transaction, doDescription, description, descriptionRegExp) &&
      checkTransactionNumber(transaction, doNumber, number, numberRegExp) &&
      checkTransactionPayee(transaction, doPayee, payee, payeeRegExp) &&
      checkTransactionCategory(transaction, doCategory, category )) {

      m_transactionList.append(new MyMoneyTransaction(
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
        transaction->state()));
    }
  }

  transactionView->init(&m_file, *pBank, *pAccount, &m_transactionList, KTransactionView::SUBSET);
  viewTransactionList();
  emit transactionOperations(true);
*/
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
      // Try an instiution

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

void KMyMoneyView::slotActivatedHomePage()
{
  emit signalHomeView();
}

void KMyMoneyView::slotActivatedAccountsView()
{
  emit signalAccountsView();
}

void KMyMoneyView::slotActivatedAccountView()
{
  emit signalAccountView();
}

void KMyMoneyView::slotActivatedScheduledView()
{
  emit signalScheduledView();
}

void KMyMoneyView::slotActivatedCategoriesView()
{
  emit signalCategoryView();
}

void KMyMoneyView::slotActivatedPayeeView()
{
  emit signalPayeeView();
}

#if 0
void KMyMoneyView::slotShowTransactionForm(bool show)
{
// FIXME: For what is this used anyway?
  if(m_ledgerView != 0)
    m_ledgerView->slotShowTransactionForm(show);
}
#endif

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
