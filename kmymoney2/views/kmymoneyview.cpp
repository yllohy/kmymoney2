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
#include <kfiledialog.h>
#include <kglobal.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <kmessagebox.h>
#include <qlabel.h>
#include <qfile.h>
#include <qtextstream.h>

#if QT_VERSION > 300
#include <qcursor.h>
#endif

#include <stdio.h>

#include "../dialogs/knewbankdlg.h"
#include "../dialogs/knewaccountdlg.h"
#include "../dialogs/kendingbalancedlg.h"
#include "../dialogs/kcategoriesdlg.h"
#include "../dialogs/kpayeedlg.h"
#include "../dialogs/knewfiledlg.h"
#include "../dialogs/kfileinfodlg.h"
#include "kmymoneyview.h"
#include "../dialogs/kchooseimportexportdlg.h"
#include "../dialogs/kcsvprogressdlg.h"
#include "../dialogs/kimportdlg.h"
#include "../dialogs/kexportdlg.h"

#include "../mymoney/storage/mymoneystoragebin.h"
#include "../mymoney/mymoneyexception.h"
#include "../mymoney/storage/mymoneystoragedump.h"

#if QT_VERSION > 300
#include <kicontheme.h>
#include <kiconloader.h>
#include <qregexp.h>
#endif

KMyMoneyView::KMyMoneyView(QWidget *parent, const char *name)
  : KJanusWidget(parent, name, KJanusWidget::IconList)
{
  m_storage = 0;
  m_file = 0;
  
  QVBox *qvboxMainFrame1 = addVBoxPage( i18n("Home"), i18n("Home"),
    DesktopIcon("home"));
  m_homeView = new KHomeView(qvboxMainFrame1);
  connect(m_homeView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedHomePage()));

  QVBox *qvboxMainFrame2 = addVBoxPage( i18n("Accounts"), i18n("Insitutions/Accounts"),
    DesktopIcon("bank"));
  accountsView = new KAccountsView(qvboxMainFrame2, "accountsView");
  connect(accountsView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedAccountsView()));

  transactionView = new KTransactionView(qvboxMainFrame2, "transactionsView");
  connect(transactionView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedAccountsView()));

  QVBox *qvboxMainFrame3 = addVBoxPage( i18n("Bills & Reminders"), i18n("Bills & Reminders"),
    DesktopIcon("scheduled"));
  m_scheduledView = new KScheduledView(qvboxMainFrame3, "scheduledView");
  connect(m_scheduledView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedScheduledView()));

  QVBox *qvboxMainFrame4 = addVBoxPage( i18n("Categories"), i18n("Categories"),
    DesktopIcon("categories"));
  m_categoriesView = new KCategoriesView(qvboxMainFrame4, "categoriesView");
  connect(m_categoriesView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedCategoriesView()));

  QVBox *qvboxMainFrame5 = addVBoxPage( i18n("Payees"), i18n("Payees"),
    DesktopIcon("pay_edit"));
  m_payeesView = new KPayeesView(qvboxMainFrame5, "payeesView");
  connect(m_payeesView, SIGNAL(signalViewActivated()), this, SLOT(slotActivatedPayeeView()));

  m_investmentView = new KInvestmentView(qvboxMainFrame2, "investmentView");

  // Need to show it, although the user wont see it.
  // At the bottom of this method we choose what to show.
  accountsView->show();
  transactionView->hide();
  m_investmentView->hide();

  connect(accountsView, SIGNAL(accountRightMouseClick(const QCString&, bool)),
    this, SLOT(slotAccountRightMouse(const QCString&, bool)));
  connect(accountsView, SIGNAL(accountDoubleClick()), this, SLOT(slotAccountDoubleClick()));
  //connect(accountsView, SIGNAL(accountSelected()), this, SLOT(slotAccountSelected()));

  connect(transactionView, SIGNAL(viewTypeSearchActivated()),
    this, SLOT(accountFind()));
  connect(transactionView, SIGNAL(viewTypeNormalActivated()),
    this, SLOT(viewTransactionList()));

  m_inReconciliation=false;
  m_reconcileInited=false;
  reconcileDlg=0;
  transactionFindDlg=0;

  // construct account context menu
  KIconLoader *kiconloader = KGlobal::iconLoader();

  m_accountMenu = new KPopupMenu(this);
  m_accountMenu->insertTitle(kiconloader->loadIcon("account", KIcon::MainToolbar), i18n("Account Options"));
  m_accountMenu->insertItem(kiconloader->loadIcon("account_open", KIcon::Small), i18n("Open..."), this, SLOT(slotAccountDoubleClick()));
  m_accountMenu->insertSeparator();
  m_accountMenu->insertItem(kiconloader->loadIcon("reconcile", KIcon::Small), i18n("Reconcile..."), this, SLOT(slotAccountReconcile()));
  m_accountMenu->insertSeparator();
  m_accountMenu->insertItem(kiconloader->loadIcon("account", KIcon::Small), i18n("Edit..."), this, SLOT(slotAccountEdit()));
  m_accountMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small), i18n("Delete..."), this, SLOT(slotAccountDelete()));

/*  We can paste this somewhere else later
  m_bankMenu = new KPopupMenu(this);
  m_bankMenu->insertTitle(kiconloader->loadIcon("bank", KIcon::MainToolbar), i18n("Institution Options"));
  m_bankId = m_bankMenu->insertItem(kiconloader->loadIcon("bank", KIcon::Small), i18n("New Institution..."), this, SLOT(slotBankNew()));
  m_accountId = m_bankMenu->insertItem(kiconloader->loadIcon("account", KIcon::Small), i18n("New Account..."), this, SLOT(slotAccountNew()));
  m_editId = m_bankMenu->insertItem(kiconloader->loadIcon("bank", KIcon::Small), i18n("Edit..."), this, SLOT(slotBankEdit()));
  m_deleteId = m_bankMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small), i18n("Delete..."), this, SLOT(slotBankDelete()));
*/
  m_realShowing = HomeView;
  showPage(0);
}

KMyMoneyView::~KMyMoneyView()
{
}

void KMyMoneyView::slotAccountRightMouse(const QCString&, bool/* inList*/)
{
  m_accountMenu->exec(QCursor::pos());
}

void KMyMoneyView::slotAccountDoubleClick(void)
{
  viewTransactionList();
}

/*  Keep this here for now in case we need to cut&paste in elsewhere
    We'll keep the menu entry and toolbar entry but no right click
void KMyMoneyView::slotBankRightMouse(const MyMoneyBank, bool inList)
{
  emit bankOperations(true);

  // enable and disable items according to position of right-click
  m_bankMenu->setItemEnabled(m_bankId, !inList);
  m_bankMenu->setItemEnabled(m_accountId, inList);
  m_bankMenu->setItemEnabled(m_editId, inList);
  m_bankMenu->setItemEnabled(m_deleteId, inList);

  m_bankMenu->exec(QCursor::pos());
}


void KMyMoneyView::slotBankEdit()
{
  if (!m_file)
    return;

  bool bankSuccess=false;
  try
  {
    MyMoneyInstitution institution = m_file->institution(banksView->currentInstitution(bankSuccess)->id());
    // bankSuccess is not checked anymore because m_file->institution will throw anyway
    KNewBankDlg dlg(institution, i18n("Edit Institution"), this);
    if (dlg.exec())
    {
      m_file.modifyInstitution(dlg->institution());
      banksView->refresh(m_file);
    }
  }
  catch(MyMoneyException *e)
  {
    delete e;
    if (bankSuccess)  // we got the bank but unable to modify
      KMessageBox::information(this, i18n("Unable to edit institution"));
    return;
  }
}

void KMyMoneyView::slotBankDelete()
{
  if (!m_file)
    return;

  bool bankSuccess=false;
  try
  {
    MyMoneyInstitution institution = m_file->institution(banksView->currentInstitution(bankSuccess)->id());
    QString msg = i18n("Really delete this institution: ");
    msg += institution.name();
    if ((KMessageBox::questionYesNo(this, msg))==KMessageBox::No)
      return;

    m_file->removeInstitution(institution);

    if (m_file->institutionCount()==0)  // If no more banks exist
      emit bankOperations(false);

    banksView->refresh(m_file);
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
    return;
  }
}
*/

void KMyMoneyView::slotAccountEdit()
{
  if (!fileOpen())
    return;

  bool accountSuccess=false;

  try
  {
    MyMoneyAccount account = m_file->account(accountsView->currentAccount(accountSuccess));

    KNewAccountDlg dlg(account, m_file, this, "hi", i18n("Edit an Account"));

    if (dlg.exec())
    {
      m_file->modifyAccount(dlg.account());
      accountsView->refresh(m_file, "");
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
    MyMoneyAccount account = m_file->account(accountsView->currentAccount(accountSuccess));
    QString prompt = i18n("Delete this account ? :-\n");
    prompt += account.name();

    if ((KMessageBox::questionYesNo(this, prompt))==KMessageBox::No)
      return;

    m_file->removeAccount(account);
  
    accountsView->refresh(m_file, "");
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
  return (m_file)?true:false;
}

void KMyMoneyView::closeFile(void)
{
  if (fileOpen()) {
    delete m_file; // Is that enough?
  }

  accountsView->clear();
  transactionView->clear();
  emit signalEnableKMyMoneyOperations(false);
}

bool KMyMoneyView::readFile(QString filename)
{
  if (!fileOpen())
  {
    m_storage = new MyMoneySeqAccessMgr;
    m_file = new MyMoneyFile(m_storage);
  }
  else
  {
    qDebug("Trying to read into an already open file.");
    return false;
  }

  // Use the old reader for now
  MyMoneyStorageBin *binaryReader = new MyMoneyStorageBin;
  QFile qfile(filename);
  qfile.open(IO_ReadOnly);
  QDataStream s(&qfile);
  binaryReader->readStream(s, m_storage);
  qfile.close();
  delete binaryReader;

  accountsView->refresh(m_file, "");

  return true;
}

void KMyMoneyView::saveFile(QString filename)
{
  if (!fileOpen()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }

//  m_file.saveAllData(filename);
}

bool KMyMoneyView::dirty(void)
{
  if (!fileOpen())
    return false;

  return m_file->dirty();
}

/*
void KMyMoneyView::setDirty(bool dirty)
{
  if (!fileOpen()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }
  m_file.setDirty(dirty);
}
*/

void KMyMoneyView::slotBankNew(void)
{
  if (!fileOpen())
    return;

  MyMoneyInstitution institution;
  
  KNewBankDlg dlg(institution, this, "newbankdlg");
  if (dlg.exec())
  {
    try
    {
      institution = dlg.institution();
      m_file->addInstitution(institution);
      accountsView->refresh(m_file, "");
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

  try
  {
    MyMoneyAccount account;

    KNewAccountDlg dialog(account, m_file, this, "hi", i18n("Create a new Account"));

    if (dialog.exec())
    {
      // The dialog doesn't check the parent.
      // An exception will be thrown on the next line instead.
      MyMoneyAccount newAccount = dialog.account();
      MyMoneyAccount parentAccount = dialog.parentAccount();
      m_file->addAccount(newAccount, parentAccount);
      accountsView->refresh(m_file, "");
      viewAccountList(newAccount.id());
    }
  }
  catch (MyMoneyException *e)
  {
    KMessageBox::information(this, "Unable to add account.");
    delete e;
    return;
  }
}

void KMyMoneyView::slotAccountReconcile(void)
{
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

void KMyMoneyView::editCategories(void)
{
  if (!fileOpen()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }

  KCategoriesDlg dlg(m_file, this);
  dlg.exec();
}

void KMyMoneyView::editPayees(void)
{
  if (!fileOpen()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }

  KPayeeDlg dlg(m_file, this);
  dlg.exec();
}

void KMyMoneyView::slotReconcileFinished(bool success)
{
  if (success)
  {
    transactionView->refresh();
  }

  // Remember to disconnect.
  disconnect(transactionView,SIGNAL(transactionListChanged()),reconcileDlg,SLOT(slotTransactionChanged()));
  reconcileDlg->hide();
  m_inReconciliation=false;
}

void KMyMoneyView::newFile(void)
{
  if (fileOpen())
    return;

  m_storage = new MyMoneySeqAccessMgr;
  m_file = new MyMoneyFile(m_storage);

  KNewFileDlg newFileDlg(this, "e", i18n("Create new KMyMoneyFile"));
  if (newFileDlg.exec())
  {
    m_file->setUserName(newFileDlg.userNameText);
    m_file->setUserStreet(newFileDlg.userStreetText);
    m_file->setUserTown(newFileDlg.userTownText);
    m_file->setUserCounty(newFileDlg.userCountyText);
    m_file->setUserPostcode(newFileDlg.userPostcodeText);
    m_file->setUserTelephone(newFileDlg.userTelephoneText);
    m_file->setUserEmail(newFileDlg.userEmailText);
    //m_file->setCreateDate(QDate::currentDate() );  // This doesn't seem to exist.  Do we want it anymore, im not bothered.

    loadDefaultCategories();
/*
  MyMoneyAccount l = m_file->liability();
  MyMoneyAccount a = m_file->asset();
  MyMoneyAccount ia = m_file->income();
  MyMoneyAccount e = m_file->expense();

  // TESTING
	try
	{

  // To Test the normal view set this to true
  // otherwise set it to false.
  // Dont forget to change the same variable in
  // KAccountsView::refresh otherwise it just
  // gets ignored.  Eventually this variable
  // will be read in by KConfig.
  bool bShowingNormalAccountsView = true;
  MyMoneyInstitution lI;
  lI.setName("Inst Liability");
  MyMoneyInstitution aI;
  aI.setName("Inst Asset");
  MyMoneyInstitution iI;
  iI.setName("Inst Income");
  MyMoneyInstitution eI;
  eI.setName("Inst Expense");

  if (bShowingNormalAccountsView)
  {
    m_file->addInstitution(lI);
    m_file->addInstitution(aI);
    m_file->addInstitution(iI);
    m_file->addInstitution(eI);
  }

	int i=0, j=0, k=0;
	for (i=0; i<4; i++)
	{
		MyMoneyAccount account;
		QString name("Lia: AccountNo: ");
		name += QString::number(i);
		account.setName(name);
    account.setAccountType(MyMoneyAccount::Checkings);
		m_file->addAccount(account, l);
    if (bShowingNormalAccountsView)
    {
      lI.addAccountId(account.id());
      m_file->modifyInstitution(lI);
    }

		for (j=0; j<3; j++)
		{
			MyMoneyAccount subAccount;
			QString name("Lia: SubAccountNo: ");
			name += QString::number(j);
			subAccount.setName(name);
      subAccount.setAccountType(MyMoneyAccount::Checkings);
			m_file->addAccount(subAccount, account);
      if (bShowingNormalAccountsView)
      {
        lI.addAccountId(subAccount.id());
        m_file->modifyInstitution(lI);
      }

			for (k=0; k<2; k++)
			{
				MyMoneyAccount subSubAccount;
				QString name("Lia: SubSubAccountNo: ");
				name += QString::number(k);
				subSubAccount.setName(name);
        subSubAccount.setAccountType(MyMoneyAccount::Checkings);
				m_file->addAccount(subSubAccount, subAccount);
        if (bShowingNormalAccountsView)
        {
          lI.addAccountId(subSubAccount.id());
          m_file->modifyInstitution(lI);
        }
			}
		}
	}

	for (i=0; i<4; i++)
	{
		MyMoneyAccount account;
		QString name("Ass: AccountNo: ");
		name += QString::number(i);
		account.setName(name);
    account.setAccountType(MyMoneyAccount::Checkings);
		m_file->addAccount(account, a);
    if (bShowingNormalAccountsView)
    {
      aI.addAccountId(account.id());
      m_file->modifyInstitution(aI);
    }

		for (j=0; j<3; j++)
		{
			MyMoneyAccount subAccount;
			QString name("Ass: SubAccountNo: ");
			name += QString::number(j);
			subAccount.setName(name);
      subAccount.setAccountType(MyMoneyAccount::Checkings);
			m_file->addAccount(subAccount, account);
      if (bShowingNormalAccountsView)
      {
        aI.addAccountId(subAccount.id());
        m_file->modifyInstitution(aI);
      }

			for (k=0; k<2; k++)
			{
				MyMoneyAccount subSubAccount;
				QString name("Ass: SubSubAccountNo: ");
				name += QString::number(k);
				subSubAccount.setName(name);
        subSubAccount.setAccountType(MyMoneyAccount::Checkings);
				m_file->addAccount(subSubAccount, subAccount);
        if (bShowingNormalAccountsView)
        {
          aI.addAccountId(subSubAccount.id());
          m_file->modifyInstitution(aI);
        }
			}
		}
	}

	for (i=0; i<4; i++)
	{
		MyMoneyAccount account;
		QString name("Exp: AccountNo: ");
		name += QString::number(i);
		account.setName(name);
    account.setAccountType(MyMoneyAccount::Checkings);
		m_file->addAccount(account, e);
    if (bShowingNormalAccountsView)
    {
      eI.addAccountId(account.id());
      m_file->modifyInstitution(eI);
    }

		for (j=0; j<3; j++)
		{
			MyMoneyAccount subAccount;
			QString name("Exp: SubAccountNo: ");
			name += QString::number(j);
			subAccount.setName(name);
      subAccount.setAccountType(MyMoneyAccount::Checkings);
			m_file->addAccount(subAccount, account);
      if (bShowingNormalAccountsView)
      {
        eI.addAccountId(subAccount.id());
        m_file->modifyInstitution(eI);
      }

			for (k=0; k<2; k++)
			{
				MyMoneyAccount subSubAccount;
				QString name("Exp: SubSubAccountNo: ");
				name += QString::number(k);
				subSubAccount.setName(name);
        subSubAccount.setAccountType(MyMoneyAccount::Checkings);
				m_file->addAccount(subSubAccount, subAccount);
        if (bShowingNormalAccountsView)
        {
          eI.addAccountId(subSubAccount.id());
          m_file->modifyInstitution(eI);
        }
			}
		}
	}

	for (i=0; i<4; i++)
	{
		MyMoneyAccount account;
		QString name("Inc: AccountNo: ");
		name += QString::number(i);
		account.setName(name);
    account.setAccountType(MyMoneyAccount::Checkings);
		m_file->addAccount(account, ia);
    if (bShowingNormalAccountsView)
    {
      iI.addAccountId(account.id());
      m_file->modifyInstitution(iI);
    }

		for (j=0; j<3; j++)
		{
			MyMoneyAccount subAccount;
			QString name("Inc: SubAccountNo: ");
			name += QString::number(j);
			subAccount.setName(name);
      subAccount.setAccountType(MyMoneyAccount::Checkings);
			m_file->addAccount(subAccount, account);
      if (bShowingNormalAccountsView)
      {
        iI.addAccountId(subAccount.id());
        m_file->modifyInstitution(iI);
      }

			for (k=0; k<2; k++)
			{
				MyMoneyAccount subSubAccount;
				QString name("Inc: SubSubAccountNo: ");
				name += QString::number(k);
				subSubAccount.setName(name);
        subSubAccount.setAccountType(MyMoneyAccount::Checkings);
				m_file->addAccount(subSubAccount, subAccount);
        if (bShowingNormalAccountsView)
        {
          iI.addAccountId(subSubAccount.id());
          m_file->modifyInstitution(iI);
        }
			}
		}
	}
	}
	catch (MyMoneyException *e)
	{
    QString message("adding all the accounts failed: ");
    message += e->what();
		qDebug(message);
    delete e;
	}
*/
  }
}

void KMyMoneyView::viewPersonal(void)
{
  if (!fileOpen()) {
    KMessageBox::information(this, i18n("Cannot start wizard on an unopened file"));
    return;
  }

  KNewFileDlg newFileDlg(m_file->userName(), m_file->userStreet(),
    m_file->userTown(), m_file->userCounty(), m_file->userPostcode(), m_file->userTelephone(),
    m_file->userEmail(), this, "e", i18n("Edit Personal Data"));
  if (newFileDlg.exec()) {
    m_file->setUserName(newFileDlg.userNameText);
    m_file->setUserStreet(newFileDlg.userStreetText);
    m_file->setUserTown(newFileDlg.userTownText);
    m_file->setUserCounty(newFileDlg.userCountyText);
    m_file->setUserPostcode(newFileDlg.userPostcodeText);
    m_file->setUserTelephone(newFileDlg.userTelephoneText);
    m_file->setUserEmail(newFileDlg.userEmailText);
  }
}

void KMyMoneyView::loadDefaultCategories(void)
{
/*
  QString filename = KGlobal::dirs()->findResource("appdata", "default_categories.dat");
  if (filename == QString::null) {
    KMessageBox::error(this, i18n("Cannot find the data file containing the default categories"));
    return;
  }

  QFile f(filename);
  if (f.open(IO_ReadOnly) ) {
    QTextStream t(&f);
    QString s;
    while ( !t.eof() ) {        // until end of file...
      s = t.readLine();       // line of text excluding '\n'
      if (!s.isEmpty() && s[0]!='#') {
        bool l_income=false;
        QString l_categoryName;
        QStringList l_minorList;
        if (parseDefaultCategory(s, l_income, l_categoryName, l_minorList)) {
          m_file.addCategory(l_income, l_categoryName, l_minorList);
        }
        l_minorList.clear(); // clear the list
      }
    }
    f.close();
  }
*/
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
  transactionView->hide();
  m_showing = BankList;

  if (fileOpen())
  {
    accountsView->refresh(m_file, selectAccount);
  }
}

void KMyMoneyView::viewTransactionList(void)
{
/*
  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

  pBank = m_file.bank(accountsView->currentBank(bankSuccess));
  if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotAccountEdit: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(accountsView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::slotAccountEdit: Unable to grab the current account");
    return;
  }

  //set up stock account view
  if(pAccount->accountType() == MyMoneyAccount::Investment)
  {
    accountsView->hide();
    transactionView->hide();
    m_investmentView->show();
    m_showing = InvestmentList;
    m_investmentView->init(pAccount);
  }
  else
  {
    m_showing = TransactionList;
    m_investmentView->hide();
    accountsView->hide();
    transactionView->show();

    if (!fileOpen())
      return;


    KConfig *config = KGlobal::config();
    QDateTime defaultDate = QDate::currentDate();
    QDate qdateStart = QDate::currentDate();//config->readDateTimeEntry("StartDate", &defaultDate).date();

    if (qdateStart != defaultDate.date())
    {
      MyMoneyTransaction *transaction;
      m_transactionList.clear();
      for ( transaction=pAccount->transactionFirst(); transaction; transaction=pAccount->transactionNext()) {
        if (transaction->date() >= qdateStart)
        {
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
    }
    else
      transactionView->init(&m_file, *pBank, *pAccount, pAccount->getTransactionList(), KTransactionView::NORMAL);
  }
  emit transactionOperations(true);
*/
}

/*
void KMyMoneyView::fileInfo(void)
{
  KFileInfoDlg dlg(m_file->createdDate(), m_file->lastAccessDate(), m_file->lastModifyDate(), this);
  dlg.exec();
}
*/

void KMyMoneyView::settingsLists()
{
/*
  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

  pBank = m_file.bank(accountsView->currentBank(bankSuccess));
  if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::settingsLists: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(accountsView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::settingsLists: Unable to grab the current account");
    return;
  }

  KConfig *config = KGlobal::config();
  QDateTime defaultDate = QDate::currentDate();
  QDate qdateStart;

  // See which list we are viewing and then refresh it
  switch(m_showing) {
    case KMyMoneyView::BankList:
      accountsView->refresh(m_file);
      break;
    case KMyMoneyView::TransactionList:
      qdateStart = config->readDateTimeEntry("StartDate", &defaultDate).date();

      if (qdateStart != defaultDate.date())
      {
        MyMoneyTransaction *transaction;
        m_transactionList.clear();
        for ( transaction=pAccount->transactionFirst(); transaction; transaction=pAccount->transactionNext()) {
          if (transaction->date() >= qdateStart)
          {
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
      }
      else
        transactionView->refresh();
      break;
    default:
      break;
  }
*/
}

void KMyMoneyView::accountFind()
{
  if (!transactionFindDlg) {
    transactionFindDlg = new KFindTransactionDlg(m_file, 0);
    connect(transactionFindDlg, SIGNAL(searchReady()), this, SLOT(doTransactionSearch()));
  }

  transactionFindDlg->show();
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
  if (m_file)
  {
    try
    {
      MyMoneyAccount account = m_file->account(accountsView->currentAccount(accountSuccess));
      if (accountSuccess)
        return account.name();
    }
    catch (MyMoneyException *e)
    {
      // When we dont catch the exception we crash.
      // We dont need to do anything with it though.
      delete e;
    }
  }
  return i18n("Unknown Account");
}

void KMyMoneyView::fileBackup(){
}
/*
void KMyMoneyView::slotBankSelected()
{
  emit bankOperations(true);
}

void KMyMoneyView::slotAccountSelected()
{
  emit accountOperations(true);
}


MyMoneyAccount* KMyMoneyView::getAccount(void)
{
  bool bBankSuccess=false, bAccountSuccess=false;
  MyMoneyBank *mymoneybank;
  MyMoneyAccount *mymoneyaccount;

  mymoneybank = m_file.bank(banksView->currentBank(bBankSuccess));
  if (!mymoneybank || !bBankSuccess) {
    qDebug("KMyMoneyView::getAccount: Unable to get the current bank");
    return 0;
  }
  mymoneyaccount = mymoneybank->account(banksView->currentAccount(bAccountSuccess));
  if (!mymoneyaccount || !bAccountSuccess) {
    qDebug("KMyMoneyView::getAccount: Unable to get the current account");
    return 0;
  }

  return mymoneyaccount;
}
*/
void KMyMoneyView::slotActivatedHomePage()
{
  m_realShowing = HomeView;
  emit signalHomeView();
}

void KMyMoneyView::slotActivatedAccountsView()
{
  m_realShowing = AccountsView;

/*******************************************************
 *  DAMNED AWFUL HACK
.* If anybody can think of an elegant way round this please
 * email mte@users.sourceforge.net.  09/02/2002.
*******************************************************/
accountsView->setSignals(false);
transactionView->setSignals(false);

  if (m_realShowing != AccountsView)
    showPage(1);

  if (m_showing == KMyMoneyView::TransactionList || m_showing == KMyMoneyView::InvestmentList)
  {
    accountsView->hide();
    if (m_showing == KMyMoneyView::TransactionList)
      transactionView->hide();
    else
      m_investmentView->hide();
  }
 else
  {
    transactionView->hide();
    m_investmentView->hide();
    accountsView->show();
    if (fileOpen())
    {
      accountsView->refresh(m_file, "");
    }
    m_showing = BankList;
  }

accountsView->setSignals(true);
transactionView->setSignals(true);
/******************************************************
 * END OF AWFUL HACK
*******************************************************/

  emit signalAccountsView();
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


void KMyMoneyView::memoryDump()
{
  QFile g( "kmymoney2.dump" );
  g.open( IO_WriteOnly );
  QDataStream st(&g);
  MyMoneyStorageDump dumper;
  dumper.writeStream(st, m_storage);
  g.close();
}