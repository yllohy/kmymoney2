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

#include "../mymoney/storage/imymoneystorageformat.h"
#include "../mymoney/storage/mymoneystoragebin.h"
#include "../mymoney/mymoneyexception.h"
#include "../mymoney/storage/mymoneystoragedump.h"
#include "../mymoney/storage/mymoneystoragexml.h"

#include "kmymoneyview.h"
// #include "kmymoneyfile.h"

#include "../kmymoney2.h"

#define COMPRESSION_MIME_TYPE "application/x-gzip"

KMyMoneyView::KMyMoneyView(QWidget *parent, const char *name)
  : KJanusWidget(parent, name, KJanusWidget::IconList),
  m_fileOpen(false)
{
  // create an empty file
  // m_file = new KMyMoneyFile;
  newStorage();

  // Page 0
  m_homeViewFrame = addVBoxPage( i18n("Home"), i18n("Home"),
    DesktopIcon("home"));
  m_homeView = new KHomeView(m_homeViewFrame);
  connect(m_homeView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedHomePage()));

  // Page 1
  m_accountsViewFrame = addVBoxPage( i18n("Accounts"), i18n("Insitutions/Accounts"),
    DesktopIcon("kmy"));
  accountsView = new KAccountsView(m_accountsViewFrame, "accountsView");
  connect(accountsView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedAccountsView()));

  // Page 2
  m_scheduleViewFrame = addVBoxPage( i18n("Schedule"), i18n("Bills & Reminders"),
    DesktopIcon("schedule"));
  m_scheduledView = new KScheduledView(m_scheduleViewFrame, "scheduledView");
  connect(m_scheduledView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedScheduledView()));

  // Page 3
  m_categoriesViewFrame = addVBoxPage( i18n("Categories"), i18n("Categories"),
    DesktopIcon("categories"));
  m_categoriesView = new KCategoriesView(m_categoriesViewFrame, "categoriesView");
  connect(m_categoriesView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedCategoriesView()));

  // Page 4
  m_payeesViewFrame = addVBoxPage( i18n("Payees"), i18n("Payees"),
    DesktopIcon("payee"));
  m_payeesView = new KPayeesView(m_payeesViewFrame, "payeesView");
  connect(m_payeesView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedPayeeView()));

  // Page 5
  m_ledgerViewFrame = addVBoxPage( i18n("Ledgers"), i18n("Ledgers"),
    DesktopIcon("ledger"));
  m_ledgerView = new KGlobalLedgerView(m_ledgerViewFrame, "ledgerView");
  // the next line causes the ledgers to get a hide() signal to be able
  // to end any pending edit activities
  connect(this, SIGNAL(aboutToShowPage(QWidget*)), m_ledgerView, SLOT(hide()));
  connect(m_ledgerView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedAccountView()));

/*
  m_investmentView = new KInvestmentView(qvboxMainFrame2, "investmentView");

  // Need to show it, although the user wont see it.
  // At the bottom of this method we choose what to show.

  accountsView->show();
  transactionView->hide();
  m_investmentView->hide();
*/

  connect(accountsView, SIGNAL(accountRightMouseClick()),
    this, SLOT(slotAccountRightMouse()));
  connect(accountsView, SIGNAL(accountDoubleClick()), this, SLOT(slotAccountDoubleClick()));
  //connect(accountsView, SIGNAL(accountSelected()), this, SLOT(slotAccountSelected()));
  connect(accountsView, SIGNAL(bankRightMouseClick()),
    this, SLOT(slotBankRightMouse()));
  connect(accountsView, SIGNAL(rightMouseClick()),
    this, SLOT(slotRightMouse()));


  connect(m_categoriesView, SIGNAL(categoryRightMouseClick()),
    this, SLOT(slotAccountRightMouse()));


  connect(m_payeesView, SIGNAL(transactionSelected(const QCString&, const QCString&)),
          this, SLOT(slotLedgerSelected(const QCString&, const QCString&)));
  connect(m_ledgerView, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
          this, SLOT(slotPayeeSelected(const QCString&, const QCString&, const QCString&)));

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
  m_accountMenu->insertItem(kiconloader->loadIcon("account", KIcon::Small), i18n("New..."), this, SLOT(slotAccountNew()), 0, AccountNew);
  m_accountMenu->insertItem(kiconloader->loadIcon("account_open", KIcon::Small), i18n("Open..."), this, SLOT(slotAccountDoubleClick()), 0, AccountOpen);
  m_accountMenu->insertSeparator();
  m_accountMenu->insertItem(kiconloader->loadIcon("reconcile", KIcon::Small), i18n("Reconcile..."), this, SLOT(slotAccountReconcile()), 0, AccountReconcile);
  m_accountMenu->insertSeparator();
  m_accountMenu->insertItem(kiconloader->loadIcon("account", KIcon::Small), i18n("Edit..."), this, SLOT(slotAccountEdit()), 0, AccountEdit);
  m_accountMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small), i18n("Delete..."), this, SLOT(slotAccountDelete()), 0, AccountDelete);

  m_bankMenu = new KPopupMenu(this);
  m_bankMenu->insertTitle(kiconloader->loadIcon("bank", KIcon::MainToolbar), i18n("Institution Options"));
  m_bankMenu->insertItem(kiconloader->loadIcon("account", KIcon::Small), i18n("New Account..."), this, SLOT(slotAccountNew()));
  m_bankMenu->insertItem(kiconloader->loadIcon("bank", KIcon::Small), i18n("Edit..."), this, SLOT(slotBankEdit()));
  m_bankMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small), i18n("Delete..."), this, SLOT(slotBankDelete()));
  
  m_rightMenu = new KPopupMenu(this);
  m_rightMenu->insertTitle(kiconloader->loadIcon("bank", KIcon::MainToolbar), i18n("KMyMoney Options"));
  m_rightMenu->insertItem(kiconloader->loadIcon("bank", KIcon::Small), i18n("New Institution..."), this, SLOT(slotBankNew()));
  
  m_realShowing = HomeView;
  showPage(0);
}

KMyMoneyView::~KMyMoneyView()
{
  removeStorage();
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
    acc = accountsView->currentAccount(ok);
  else
    acc = m_categoriesView->currentAccount(ok);

  // turn off all available options in the menu except New
  m_accountMenu->setItemEnabled(AccountNew, true);
  m_accountMenu->setItemEnabled(AccountOpen, false);
  m_accountMenu->setItemEnabled(AccountEdit, false);
  m_accountMenu->setItemEnabled(AccountReconcile, false);
  m_accountMenu->setItemEnabled(AccountDelete, false);

  if(ok == true) {
    try {
      MyMoneyFile* file = MyMoneyFile::instance();
      MyMoneyAccount account = file->account(acc);
      if(!file->isStandardAccount(acc)) {
        m_accountMenu->setItemEnabled(AccountEdit, true);
        m_accountMenu->setItemEnabled(AccountDelete, true);
        switch(file->accountGroup(account.accountType())) {
          case MyMoneyAccount::Asset:
          case MyMoneyAccount::Liability:
            m_accountMenu->setItemEnabled(AccountOpen, true);
            m_accountMenu->setItemEnabled(AccountReconcile, true);
            break;
          default:
            break;
        }
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

void KMyMoneyView::slotAccountDoubleClick(void)
{
  bool  ok = false;
  QCString acc;

  if(pageIndex(m_accountsViewFrame) == activePageIndex())
    acc = accountsView->currentAccount(ok);
  else
    acc = m_categoriesView->currentAccount(ok);

  if(ok == true) {
    showPage(pageIndex(m_ledgerViewFrame));
    m_ledgerView->selectAccount(acc);
  }
}

void KMyMoneyView::slotBankRightMouse()
{
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
    MyMoneyInstitution institution = file->institution(accountsView->currentInstitution(bankSuccess));




    // bankSuccess is not checked anymore because m_file->institution will throw anyway
    KNewBankDlg dlg(institution, true, this, "edit_bank");
    if (dlg.exec())
    {
      file->modifyInstitution(dlg.institution());
      // FIXME: remove accountsView->refresh("");
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
    MyMoneyInstitution institution = file->institution(accountsView->currentInstitution(bankSuccess));
    QString msg = i18n("Really delete this institution: ");
    msg += institution.name();
    if ((KMessageBox::questionYesNo(this, msg))==KMessageBox::No)
      return;
    file->removeInstitution(institution);
    // FIXME: remove accountsView->refresh("");
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

    if(pageIndex(m_accountsViewFrame) == activePageIndex())
      account = file->account(accountsView->currentAccount(accountSuccess));
    else
      account = file->account(m_categoriesView->currentAccount(accountSuccess));

    KNewAccountDlg dlg(account, true, false, this, "hi", i18n("Edit an Account"));

    if (dlg.exec())
    {
      file->modifyAccount(dlg.account());
    }
  }
  catch (MyMoneyException *e)
  {
    if (accountSuccess)
    {
      QString errorString = i18n("Cannot edit account: ");
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

  try
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount account;

    if(pageIndex(m_accountsViewFrame) == activePageIndex())
      account = file->account(accountsView->currentAccount(accountSuccess));
    else
      account = file->account(m_categoriesView->currentAccount(accountSuccess));

    QString prompt = i18n("Do you really want to delete the account '%1'")
      .arg(account.name());

    if ((KMessageBox::questionYesNo(this, prompt))==KMessageBox::No)
      return;

    file->removeAccount(account);
  
    // FIXME: remove  accountsView->refresh("");
  }
  catch (MyMoneyException *e)
  {
    if (accountSuccess)
    {
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

  accountsView->clear();
  emit signalEnableKMyMoneyOperations(false);
}

bool KMyMoneyView::readFile(const KURL& url)
{
  QString filename;
/*
  KMyMoneyFile *kfile = m_file;
  if (fileOpen())
  {
    kfile->close();
    kfile->open();
  }
  else
    kfile->open();
*/
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

  QString strFileExtension = MyMoneyUtils::getFileExtension(url.path());

#ifdef _COMPILE_XML
#if HAVE_LIBXMLPP
  if(strFileExtension.find("XML") != -1)
  {
    pReader = new MyMoneyStorageXML;
  }
  else
#endif
#endif
  {
    // Use the binary reader 
    pReader = new MyMoneyStorageBin;
  }

  // let's glimps into the file to figure out, if it's one
  // of the old (uncompressed) or new (compressed) files.

  pReader->setProgressCallback(&KMyMoneyView::progressCallback);

  QFile file(filename);
  QIODevice *qfile = 0;

  if(file.open(IO_ReadOnly)) {
    QString  buffer;
    int ch;
    for(unsigned int i = 0; i < 2; ++i) {
      ch = file.getch();
      if(ch == -1)
        break;
      buffer += QChar(ch);
    }
    file.close();

    if(buffer.length() == 2) {
      if(buffer == QString("\037\213")) {         // gzipped?
        qfile = KFilterDev::deviceForFile(filename, COMPRESSION_MIME_TYPE);
      } else {
        // we can't use file directly, as we delete qfile later on
        qfile = new QFile(file.name());
      }

      if(qfile->open(IO_ReadOnly)) {
        try {
          pReader->readFile(qfile, dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()));
        } catch (MyMoneyException *e) {
          QString msg = e->what();
          qDebug("%s", msg.latin1());
          delete e;
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

  pReader->setProgressCallback(0);

  delete pReader;

  // if a temporary file was constructed by NetAccess::download,
  // then it will be removed with the next call. Otherwise, it
  // stays untouched on the local filesystem
  KIO::NetAccess::removeTempFile(filename);

  // update all views
  m_categoriesView->refreshView();
  accountsView->refreshView();
  m_ledgerView->reloadView();
  m_payeesView->refreshView();
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
      "If you want to abort this operation, please press Cancel now")) == KMessageBox::Cancel)
    return;

  IMyMoneyStorageFormat* pWriter = NULL;

  QString strFileExtension = MyMoneyUtils::getFileExtension(filename);

#ifdef _COMPILE_XML
#if HAVE_LIBXMLPP
  if(strFileExtension.find("XML") != -1)
  {
    pWriter = new MyMoneyStorageXML;
  }
  else
#endif
#endif

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
    }
    catch (MyMoneyException *e)
    {
      delete e;
      KMessageBox::information(this, i18n("Cannot add bank"));
      return;
    }
  }
}

void KMyMoneyView::slotAccountNew(void)
{

  if (!fileOpen())
    return;


  MyMoneyAccount newAccount;
  MyMoneyAccount parentAccount;
  int dialogResult;

  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  if(config->readBoolEntry("NewAccountWizard", true) == true) {
    // wizard selected
    if((dialogResult = m_newAccountWizard->exec()) == QDialog::Accepted) {
      newAccount = m_newAccountWizard->account();
      parentAccount = m_newAccountWizard->parentAccount();
    }
  } else {
    // regular dialog selected
    MyMoneyAccount account;
    KNewAccountDlg dialog(account, false, false, this, "hi", i18n("Create a new Account"));

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
      MyMoneyFile::instance()->addAccount(newAccount, parentAccount);

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

  if(pageIndex(m_accountsViewFrame) == activePageIndex())
    acc = accountsView->currentAccount(ok);
  else
    acc = m_categoriesView->currentAccount(ok);

  // we cannot reconcile standard accounts
  if(file->isStandardAccount(acc))
    ok = false;

  // check if we can reconcile this account
  // it make's sense for asset and liability accounts
  if(ok == true) {
    try {
      MyMoneyAccount account = file->account(acc);
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
    m_ledgerView->selectAccount(acc, "", true);
  }

/*
  MyMoneyMoney l_previousBal, l_endingBalGuess;

  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

  pBank = m_file.bank(accountsView->currentBank(bankSuccess));
  if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotAccountReconcile: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(accountsView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::slotAccountReconcile: Unable to grab the current account");



    return;
  }

  MyMoneyTransaction *transaction;
  // Calculate the previous balance and the guess
  for ( transaction=pAccount->transactionFirst(); transaction; transaction=pAccount->transactionNext()) {
    if (transaction->state() == MyMoneyTransaction::Reconciled)
      if (transaction->type() == MyMoneyTransaction::Credit)
        l_previousBal += transaction->amount();
      else
        l_previousBal -= transaction->amount();

    if (transaction->type() == MyMoneyTransaction::Credit)

      l_endingBalGuess += transaction->amount();
    else
      l_endingBalGuess -= transaction->amount();
  }

  KEndingBalanceDlg dlg(l_previousBal, l_endingBalGuess, this);
  if (dlg.exec()) {
    if (!m_reconcileInited) {
      reconcileDlg = new KReconcileDlg(dlg.previousBalance, dlg.endingBalance, dlg.endingDate, *pBank, pAccount, m_file, 0);
      connect(reconcileDlg, SIGNAL(reconcileFinished(bool)), this, SLOT(slotReconcileFinished(bool)));
      connect(transactionView,SIGNAL(transactionListChanged()),reconcileDlg,SLOT(slotTransactionChanged()));
      reconcileDlg->exec();
      m_inReconciliation = true;
      m_reconcileInited=true;
    } else {
      reconcileDlg->resetData(dlg.previousBalance, dlg.endingBalance, dlg.endingDate, *pBank, pAccount, m_file);
      connect(transactionView,SIGNAL(transactionListChanged()),reconcileDlg,SLOT(slotTransactionChanged()));
      reconcileDlg->exec();
      m_inReconciliation = true;
    }
  }
*/
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

void KMyMoneyView::slotReconcileFinished(bool success)
{

/*
  if (success)
  {
    transactionView->refresh();
  }

  // Remember to disconnect.
  // disconnect(transactionView,SIGNAL(transactionListChanged()),reconcileDlg,SLOT(slotTransactionChanged()));
  reconcileDlg->hide();
  m_inReconciliation=false;
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
    //m_file->setCreateDate(QDate::currentDate() );  // This doesn't seem to exist.  Do we want it anymore, im not bothered.

    loadDefaultCategories();
    // FIXME: remove  accountsView->refresh("");
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
  KFileDialog* dialog = new KFileDialog(KGlobal::dirs()->findResourceDir("appdata", "default_accounts.dat"),
                                        i18n("*.dat|Account templates"),
                                        this, i18n("Select account template"),
                                        true);
  dialog->setMode(KFile::File | KFile::ExistingOnly | KFile::LocalOnly);
  if(dialog->exec()) {
    readDefaultCategories(dialog->selectedFile());
  }
  delete dialog;
  accountsView->refreshView();
  m_categoriesView->refreshView();
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

  switch (m_showing) {
    case KMyMoneyView::TransactionList:
      viewAccountList("");
      break;
    default:
      break;
  }
}

void KMyMoneyView::viewAccountList(const QCString& selectAccount)
{
  if (m_realShowing != AccountsView)
    showPage(1);

  accountsView->show();
  // transactionView->hide();
  m_showing = BankList;

  if (fileOpen())
  {
    // FIXME: remove accountsView->refresh(selectAccount);
  }

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
  accountsView->refreshView();
  m_categoriesView->refreshView();
  m_ledgerView->refreshView();
  m_payeesView->refreshView();
}

void KMyMoneyView::accountFind()
{
/*
  if (!transactionFindDlg) {
    transactionFindDlg = new KFindTransactionDlg(0);
    connect(transactionFindDlg, SIGNAL(searchReady()), this, SLOT(doTransactionSearch()));
  }

  transactionFindDlg->show();
*/
}

void KMyMoneyView::doTransactionSearch()
{
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

bool KMyMoneyView::checkTransactionDates(const MyMoneyTransaction *transaction, const bool enabled, const QDate start, const QDate end)
{
  if (enabled) {
    if (transaction->postDate()>=start && transaction->postDate()<=end)
      return true;
    else
      return false;
  }
  return true;

}

bool KMyMoneyView::checkTransactionAmount(const MyMoneyTransaction *transaction, const bool enabled, const QString id, const MyMoneyMoney amount)
{
/*
  if (!enabled)

    return true;

  if (id==i18n("At least")) {
    if (transaction->amount() >= amount)
      return true;
  } else if (id==i18n("At most")) {
    if (transaction->amount() <= amount)
      return true;
  } else {
    if (transaction->amount() == amount)
      return true;
  }
*/
  return false;
}

bool KMyMoneyView::checkTransactionCredit(const MyMoneyTransaction *transaction, const bool enabled, const QString id)

{
/*

  if (!enabled)
    return true;

  if (id==i18n("Credit or Debit") && (transaction->type()==MyMoneyTransaction::Credit || transaction->type()==MyMoneyTransaction::Debit))
    return true;
  else if (id==i18n("Credit") && transaction->type()==MyMoneyTransaction::Credit)
    return true;
  else if (id==i18n("Debit") && transaction->type()==MyMoneyTransaction::Debit)
    return true;
  else if (id==i18n("Cheque") && transaction->method()==MyMoneyTransaction::Cheque)
    return true;

  else if (id==i18n("Deposit") && transaction->method()==MyMoneyTransaction::Deposit)
    return true;
  else if (id==i18n("Transfer") && transaction->method()==MyMoneyTransaction::Transfer)
    return true;
  else if (id==i18n("Withdrawal") && transaction->method()==MyMoneyTransaction::Withdrawal)
    return true;
  else if (id==i18n("ATM") && transaction->method()==MyMoneyTransaction::ATM)
    return true;
*/
  return false;
}

bool KMyMoneyView::checkTransactionStatus(const MyMoneyTransaction *transaction, const bool enabled, const QString id)
{
/*
  if (!enabled)
    return true;

  if (id==i18n("Cleared") && transaction->state()==MyMoneyTransaction::Cleared)
    return true;
  if (id==i18n("Reconciled") && transaction->state()==MyMoneyTransaction::Reconciled)
    return true;
  if (id==i18n("Unreconciled") && transaction->state()==MyMoneyTransaction::Unreconciled)
    return true;
*/
  return false;
}

bool KMyMoneyView::checkTransactionDescription(const MyMoneyTransaction *transaction, const bool enabled, const QString description, const bool isRegExp)


{
/*
  if (!enabled)
    return true;

  if (!isRegExp) {
    if (transaction->memo().contains(description))
      return true;
    else
      return false;
  } else {
    QRegExp regExp(description);
    if (!regExp.isValid())
      return false;
    if (regExp.match(transaction->memo())==-1)
      return false;
    else
      return true;
  }
*/
}

bool KMyMoneyView::checkTransactionNumber(const MyMoneyTransaction *transaction, const bool enabled, const QString number, const bool isRegExp)

{

/*
  if (!enabled)

    return true;

  if (!isRegExp) {
    if (transaction->number().contains(number))
      return true;
    else

      return false;
  } else {
    QRegExp regExp(number);
    if (!regExp.isValid())
      return false;
    if (regExp.match(transaction->number())==-1)
      return false;
    else
      return true;
  }
*/
}

bool KMyMoneyView::checkTransactionPayee(const MyMoneyTransaction *transaction, const bool enabled, const QString payee, const bool isRegExp)
{
/*
  if (!enabled)
    return true;

  if (!isRegExp) {


    if (transaction->payee().contains(payee))
      return true;
    else
      return false;
  } else {
    QRegExp regExp(payee);
    if (!regExp.isValid())
      return false;
    if (regExp.match(transaction->payee())==-1)
      return false;
    else

      return true;
  }
*/
}

bool KMyMoneyView::checkTransactionCategory(const MyMoneyTransaction *transaction, const bool enabled, const QString category)
{

/*
  if (!enabled)
    return true;

  QString left, right;
  if (category.contains(':')) {
    left = category.left(category.find(':'));
    right = category.mid(category.find(':')+1, category.length());
    if (transaction->categoryMajor()==left &&
        transaction->categoryMinor()==right)
      return true;
    else

      return false;
  }

  if (transaction->categoryMajor() == category)
    return true;

*/
  return false;
}
/*
QString KMyMoneyView::currentBankName(void)

{
  bool bankSuccess=false;
  if (m_file) {
    MyMoneyInstiution = accountsView->currentInstitution(bankSuccess);
    if (bankSuccess)
      return institution.name();
  }
  return i18n("Unknown Institution");
}
*/
QString KMyMoneyView::currentAccountName(void)
{
  bool accountSuccess=false;
    MyMoneyFile* file = MyMoneyFile::instance();
    try
    {

      MyMoneyAccount account = file->account(accountsView->currentAccount(accountSuccess));
      if (accountSuccess)

        return account.name();
    }
    catch (MyMoneyException *e)
    {
      // Try an instiution

      try
      {
        MyMoneyInstitution institution = file->institution(accountsView->currentInstitution(accountSuccess));
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
  m_realShowing = HomeView;
  emit signalHomeView();
}

void KMyMoneyView::slotActivatedAccountsView()
{
  m_realShowing = AccountsView;

  emit signalAccountsView();
}

void KMyMoneyView::slotActivatedAccountView()
{
  m_realShowing = AccountView;

  emit signalAccountView();
}

void KMyMoneyView::slotActivatedScheduledView()
{
  m_realShowing = ScheduledView;

  emit signalScheduledView();
}

void KMyMoneyView::slotActivatedCategoriesView()
{

  m_realShowing = CategoryView;

  emit signalCategoryView();
}

void KMyMoneyView::slotActivatedPayeeView()
{
  m_realShowing = PayeeView;
  emit signalPayeeView();
}

void KMyMoneyView::slotShowTransactionForm(bool show)
{
  if(m_ledgerView != 0)
    m_ledgerView->slotShowTransactionForm(show);
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
}

void KMyMoneyView::progressCallback(int current, int total, const QString& msg)
{
  if(!msg.isEmpty())
    kmymoney2->slotStatusMsg(msg);
  
  kmymoney2->slotStatusProgressBar(current, total);
}
