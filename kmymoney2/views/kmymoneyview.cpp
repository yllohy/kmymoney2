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
#include <kstddirs.h>
#include <kmessagebox.h>
#include <qlabel.h>
#include <qfile.h>
#include <qtextstream.h>

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

#if QT_VERSION > 300
#include <kicontheme.h>
#include <kiconloader.h>
#include <qregexp.h>
#endif

KMyMoneyView::KMyMoneyView(QWidget *parent, const char *name)
  : KJanusWidget(parent, name, KJanusWidget::IconList)
{
  QVBox *qvboxMainFrame1 = addVBoxPage( i18n("Home"), i18n("Home"),
    DesktopIcon("home"));
  m_homeView = new KHomeView(qvboxMainFrame1);

  QVBox *qvboxMainFrame2 = addVBoxPage( i18n("Accounts"), i18n("Insitutions/Accounts"),
    DesktopIcon("bank"));
  banksView = new KBanksView(qvboxMainFrame2, "banksView");
  transactionView = new KTransactionView(qvboxMainFrame2, "transactionsView");

  QVBox *qvboxMainFrame3 = addVBoxPage( i18n("Bills & Reminders"), i18n("Bills & Reminders"),
    DesktopIcon("scheduled"));
  m_scheduledView = new KScheduledView(&m_file, qvboxMainFrame3, "scheduledView");

  QVBox *qvboxMainFrame4 = addVBoxPage( i18n("Categories"), i18n("Categories"),
    DesktopIcon("categories"));
  m_categoriesView = new KCategoriesView(&m_file, qvboxMainFrame4, "categoriesView");

  QVBox *qvboxMainFrame5 = addVBoxPage( i18n("Payees"), i18n("Payees"),
    DesktopIcon("pay_edit"));
  m_payeesView = new KPayeesView(&m_file, qvboxMainFrame5, "payeesView");

  m_investmentView = new KInvestmentView(qvboxMainFrame2, "investmentView");

  banksView->hide();
  transactionView->hide();
  m_investmentView->hide();

  connect(banksView, SIGNAL(accountRightMouseClick(const MyMoneyAccount, bool)), this, SLOT(slotAccountRightMouse(const MyMoneyAccount, bool)));
  connect(banksView, SIGNAL(accountDoubleClick()), this, SLOT(slotAccountDoubleClick()));

  connect(banksView, SIGNAL(bankRightMouseClick(const MyMoneyBank, bool)), this, SLOT(slotBankRightMouse(const MyMoneyBank, bool)));
  connect(banksView, SIGNAL(bankSelected()), this, SLOT(slotBankSelected()));
  connect(banksView, SIGNAL(accountSelected()), this, SLOT(slotAccountSelected()));

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

  m_bankMenu = new KPopupMenu(this);
  m_bankMenu->insertTitle(kiconloader->loadIcon("bank", KIcon::MainToolbar), i18n("Institution Options"));
  m_bankId = m_bankMenu->insertItem(kiconloader->loadIcon("bank", KIcon::Small), i18n("New Institution..."), this, SLOT(slotBankNew()));
  m_accountId = m_bankMenu->insertItem(kiconloader->loadIcon("account", KIcon::Small), i18n("New Account..."), this, SLOT(slotAccountNew()));
  m_editId = m_bankMenu->insertItem(kiconloader->loadIcon("bank", KIcon::Small), i18n("Edit..."), this, SLOT(slotBankEdit()));
  m_deleteId = m_bankMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small), i18n("Delete..."), this, SLOT(slotBankDelete()));
}

KMyMoneyView::~KMyMoneyView()
{
}

void KMyMoneyView::slotAccountRightMouse(const MyMoneyAccount, bool/* inList*/)
{
  m_accountMenu->exec(QCursor::pos());
}

void KMyMoneyView::slotAccountDoubleClick(void)
{
	
  viewTransactionList();
}

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
  if ( !m_file.isInitialised() ) {
    KMessageBox::information(this, i18n("No MyMoneyFile open"));
    return;
  }

  bool bankSuccess=false;
  MyMoneyBank bank = banksView->currentBank(bankSuccess);
  if (bankSuccess) {
    KNewBankDlg dlg(bank.name(),
      bank.sortCode(),
      bank.city(),
      bank.street(),
      bank.postcode(),
      bank.telephone(),
      bank.manager(),
      i18n("Edit Institution"), this);

    if (dlg.exec()) {
      MyMoneyBank *bankWrite;
      if ((bankWrite=m_file.bank(bank))) {
        bankWrite->setName(dlg.m_name);
        bankWrite->setCity(dlg.m_city);
        bankWrite->setStreet(dlg.m_street);
        bankWrite->setPostcode(dlg.m_postcode);
        bankWrite->setTelephone(dlg.m_telephone);
        bankWrite->setManager(dlg.m_managerName);
        m_file.setDirty(true);
        banksView->refresh(m_file);
      } else {
        KMessageBox::information(this, i18n("Unable to grab a pointer to a bank"));
      }
    } else
      qDebug("WARNING: unable to get currf2ent bank");
  }
}

void KMyMoneyView::slotBankDelete()
{
  bool bankSuccess=false;
  MyMoneyBank bank = banksView->currentBank(bankSuccess);
  if (bankSuccess) {
    QString msg = i18n("Delete this bank: ");
    msg += bank.name();
    if ((KMessageBox::questionYesNo(this, msg))==KMessageBox::No)
      return;

    m_file.removeBank(bank);

    if (m_file.bankCount()<=0)  // If no more banks exist
      emit bankOperations(false);

    banksView->refresh(m_file);
  } else
    qDebug("WARNING: unable to get currfent bank");
}

void KMyMoneyView::slotAccountEdit()
{
  if (!m_file.isInitialised()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }

  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(banksView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotAccountEdit: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(banksView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::slotAccountEdit: Unable to grab the current account");
    return;
  }

  KNewAccountDlg dlg(pBank->name(), pAccount->name(),
                         pAccount->accountNumber(),
                         pAccount->accountType(),
                         pAccount->description(),
                         pAccount->openingDate(),
                         pAccount->openingBalance(),
                         this, "hi", i18n("Edit an Account"));

  if (!dlg.exec())
    return;

  pAccount->setName(dlg.accountNameText);
  pAccount->setAccountNumber(dlg.accountNoText);
  pAccount->setAccountType(dlg.type);
  pAccount->setAccountNumber(dlg.accountNoText);
  pAccount->setDescription(dlg.descriptionText);
  pAccount->setOpeningDate(dlg.startDate);
  MyMoneyMoney money(dlg.startBalance);
  pAccount->setOpeningBalance(money);

  m_file.setDirty(true);

  banksView->refresh(m_file);
}


void KMyMoneyView::slotAccountDelete()
{
  if (!m_file.isInitialised()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }

  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(banksView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotAccountDelete: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(banksView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::slotAccountDelete: Unable to grab the current account");
    return;
  }

  QString prompt = i18n("Delete this account ? :-\n");
  prompt += pAccount->name();

  if ((KMessageBox::questionYesNo(this, prompt))==KMessageBox::No)
    return;

  pBank->removeAccount(*pAccount);
  if (pBank->accountCount()<=0) // If no more accounts exist
    emit accountOperations(false);

  banksView->refresh(m_file);
  m_file.setDirty(true);
}

bool KMyMoneyView::fileOpen(void)
{
  return m_file.isInitialised();
}

void KMyMoneyView::closeFile(void)
{
  if (m_file.isInitialised()) {
    m_file.resetAllData();  // Make sure all memory is released
  }

  banksView->clear();
  transactionView->clear();
  emit fileOperations(false);
}

bool KMyMoneyView::readFile(QString filename)
{
  if (!m_file.isInitialised()) {
//    m_file = new MyMoneyFile;
    m_file.init();
  }
  m_file.resetAllData();

  QString error;
  int ret;	
  if ((ret=m_file.readAllData(filename))!=0) {
    switch (ret) {
      case 1: // bad version  number
        error = i18n("Error while reading file: Bad version number");
        KMessageBox::information(this, error);
        break;
      case 2: // bad magic number
        error = i18n("Error while reading file: Bad magic number");
        KMessageBox::information(this, error);
        break;
      case 3: // File doesn't exist
        error = i18n("Error while reading file: File doesn't exist");
        KMessageBox::information(this, error);
        break;
      default:
        error = i18n("Error while reading file, (");
        error += QString::number(ret);
        error += ").";
        KMessageBox::information(this, error);
        break;
    }
    m_file.resetAllData();
//    delete m_file;
//    m_file=0;
    return false;
  }
  else {
    viewBankList();
    banksView->refresh(m_file);
    emit bankOperations(true);
    m_file.init();
  }
  return true;
}

void KMyMoneyView::saveFile(QString filename)
{
  if (!m_file.isInitialised()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }

  m_file.saveAllData(filename);
}

bool KMyMoneyView::dirty(void)
{
  if (!m_file.isInitialised())
    return false;
  return m_file.dirty();
}

void KMyMoneyView::setDirty(bool dirty)
{
  if (!m_file.isInitialised()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }
  m_file.setDirty(dirty);
}

void KMyMoneyView::slotBankNew(void)
{
  if (!m_file.isInitialised()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }

  KNewBankDlg dlg(this);
  if (dlg.exec()) {
//    clear();

    MyMoneyBank *mymoneybank = m_file.addBank(dlg.m_name, dlg.m_sortCode, dlg.m_city, dlg.m_street, dlg.m_postcode, dlg.m_telephone, dlg.m_managerName);
    if (mymoneybank)
    {
      banksView->refresh(m_file);
      viewBankList(NULL, mymoneybank);
      emit bankOperations(true);
    }
  }
}

void KMyMoneyView::slotAccountNew(void)
{
  if (!m_file.isInitialised()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }

  if (m_file.bankCount()<=0)
    return;

  bool bankSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(banksView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotAccountNew: Unable to get the current bank");
    return;
  }

  KNewAccountDlg dialog(pBank->name(), this, "hi", i18n("Create a new Account"));

  if (dialog.exec()) {
    MyMoneyMoney money(dialog.startBalance);

    pBank->newAccount(dialog.accountNameText,
                            dialog.accountNoText,
                            dialog.type,
                            dialog.descriptionText,
                            dialog.startDate,
                            money,
                            QDate(1923, 1, 1)
                            );
    banksView->refresh(m_file);

    MyMoneyAccount accountTmp(pBank, dialog.accountNameText,
                          dialog.accountNoText,
                          dialog.type,
                          dialog.descriptionText,
                          dialog.startDate,
                          money,
                          QDate(1923, 1, 1));
    pAccount = pBank->account(accountTmp);
    if (!pAccount) {
      qDebug("Unable to grab the newly created account");
      return;
    }

    if (!money.isZero()) {
      pAccount->addTransaction(MyMoneyTransaction::Deposit,
          0,
          i18n("Initial account balance"),
          money,
          dialog.startDate,
          "", "", "", "", "", "",
          MyMoneyTransaction::Unreconciled );
    }
    viewBankList(pAccount);
    emit accountOperations(true);
  }
}

void KMyMoneyView::slotAccountReconcile(void)
{
  MyMoneyMoney l_previousBal, l_endingBalGuess;

  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

  pBank = m_file.bank(banksView->currentBank(bankSuccess));
  if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotAccountReconcile: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(banksView->currentAccount(accountSuccess));
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
}

void KMyMoneyView::slotAccountImportAscii(void)
{
  KChooseImportExportDlg dlg(0, this);
  if (dlg.exec()) {
    if (dlg.importExportType()=="QIF") {
      KImportDlg importDlg(getAccount(), this);
      if (importDlg.exec()) {
        transactionView->refresh();
        banksView->refresh(m_file);
      }
    }
    else {
      KCsvProgressDlg kcsvprogressdlg(0, getAccount(), this);
      if (kcsvprogressdlg.exec()) {
        transactionView->refresh();
        banksView->refresh(m_file);
      }
    }
  }
}

void KMyMoneyView::slotAccountExportAscii(void)
{
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
}

void KMyMoneyView::editCategories(void)
{
  if (!m_file.isInitialised()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }

  KCategoriesDlg dlg(&m_file, this);
  dlg.exec();
}

void KMyMoneyView::editPayees(void)
{
  if (!m_file.isInitialised()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return;
  }

  KPayeeDlg dlg(&m_file, this);
  dlg.exec();
}

void KMyMoneyView::slotReconcileFinished(bool success)
{
  if (success) {
    transactionView->refresh();
  }

  // Remember to disconnect.
  disconnect(transactionView,SIGNAL(transactionListChanged()),reconcileDlg,SLOT(slotTransactionChanged()));
  reconcileDlg->hide();
  m_inReconciliation=false;
}

void KMyMoneyView::newFile(void)
{
   // create the money file
  KNewFileDlg newFileDlg(this, "e", i18n("Create new KMyMoneyFile"));
  if (newFileDlg.exec()) {
    m_file.set_userName(newFileDlg.userNameText);
    m_file.set_userStreet(newFileDlg.userStreetText);
    m_file.set_userTown(newFileDlg.userTownText);
    m_file.set_userCounty(newFileDlg.userCountyText);
    m_file.set_userPostcode(newFileDlg.userPostcodeText);
    m_file.set_userTelephone(newFileDlg.userTelephoneText);
    m_file.set_userEmail(newFileDlg.userEmailText);
    m_file.setCreateDate(QDate::currentDate() );

    loadDefaultCategories();

    m_file.init();
    emit bankOperations(true);
    m_file.setDirty(true);
    viewBankList();
  }
}

void KMyMoneyView::viewPersonal(void)
{
  if (!m_file.isInitialised()) {
    KMessageBox::information(this, i18n("Cannot start wizard on an unopened file"));
    return;
  }

  KNewFileDlg newFileDlg(m_file.userName(), m_file.userStreet(),
    m_file.userTown(), m_file.userCounty(), m_file.userPostcode(), m_file.userTelephone(),
    m_file.userEmail(), this, "e", i18n("Edit Personal Data"));
  if (newFileDlg.exec()) {
    m_file.set_userName(newFileDlg.userNameText);
    m_file.set_userStreet(newFileDlg.userStreetText);
    m_file.set_userTown(newFileDlg.userTownText);
    m_file.set_userCounty(newFileDlg.userCountyText);
    m_file.set_userPostcode(newFileDlg.userPostcodeText);
    m_file.set_userTelephone(newFileDlg.userTelephoneText);
    m_file.set_userEmail(newFileDlg.userEmailText);

    m_file.setDirty(true);
  }
}

void KMyMoneyView::loadDefaultCategories(void)
{
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
  if (!m_file.isInitialised())
    return;

  switch (m_showing) {
//    case KMainView::AccountList:
//      qDebug("currently viewing account list");
//      viewBankList();
//      break;
    case KMyMoneyView::TransactionList:
      viewBankList();
      break;
    default:
      break;
  }
}

void KMyMoneyView::viewBankList(MyMoneyAccount *selectAccount, MyMoneyBank *selectBank)
{
  banksView->show();
  transactionView->hide();
  m_showing = BankList;

  if (m_file.isInitialised())
  {
    banksView->refresh(m_file, selectAccount, selectBank);
    emit bankOperations(true);
  }
}

void KMyMoneyView::viewTransactionList(void)
{
	bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(banksView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotAccountEdit: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(banksView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::slotAccountEdit: Unable to grab the current account");
    return;
  }

  //set up stock account view
	if(pAccount->accountType() == MyMoneyAccount::Investment)
  {
  	banksView->hide();
  	transactionView->hide();
  	m_investmentView->show();
  	m_showing = InvestmentList;
		m_investmentView->init(pAccount);  	
  }
  else
  {
    m_investmentView->hide();	
		banksView->hide();
    transactionView->show();
    m_showing = TransactionList;

    if (!m_file.isInitialised())
      return;

/*  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(banksView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::viewTransactionList: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(banksView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::viewTransactionList: Unable to grab the current account");
    return;
  } */

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
}

void KMyMoneyView::fileInfo(void)
{
  KFileInfoDlg dlg(m_file.createdDate(), m_file.lastAccessDate(), m_file.lastModifyDate(), this);
  dlg.exec();
}

void KMyMoneyView::settingsLists()
{
  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(banksView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::settingsLists: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(banksView->currentAccount(accountSuccess));
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
      banksView->refresh(m_file);
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
}

void KMyMoneyView::accountFind()
{
  if (!transactionFindDlg) {
    transactionFindDlg = new KFindTransactionDlg(&m_file, 0);
    connect(transactionFindDlg, SIGNAL(searchReady()), this, SLOT(doTransactionSearch()));
  }

  transactionFindDlg->show();
}

void KMyMoneyView::doTransactionSearch()
{
  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(banksView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::doTransactionSearch: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(banksView->currentAccount(accountSuccess));
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
}

bool KMyMoneyView::checkTransactionDates(const MyMoneyTransaction *transaction, const bool enabled, const QDate start, const QDate end)
{
  if (enabled) {
    if (transaction->date()>=start && transaction->date()<=end)
      return true;
    else
      return false;
  }
  return true;
}

bool KMyMoneyView::checkTransactionAmount(const MyMoneyTransaction *transaction, const bool enabled, const QString id, const MyMoneyMoney amount)
{
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

  return false;
}

bool KMyMoneyView::checkTransactionCredit(const MyMoneyTransaction *transaction, const bool enabled, const QString id)
{
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

  return false;
}

bool KMyMoneyView::checkTransactionStatus(const MyMoneyTransaction *transaction, const bool enabled, const QString id)
{
  if (!enabled)
    return true;

  if (id==i18n("Cleared") && transaction->state()==MyMoneyTransaction::Cleared)
    return true;
  if (id==i18n("Reconciled") && transaction->state()==MyMoneyTransaction::Reconciled)
    return true;
  if (id==i18n("Unreconciled") && transaction->state()==MyMoneyTransaction::Unreconciled)
    return true;

  return false;
}

bool KMyMoneyView::checkTransactionDescription(const MyMoneyTransaction *transaction, const bool enabled, const QString description, const bool isRegExp)
{
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
}

bool KMyMoneyView::checkTransactionNumber(const MyMoneyTransaction *transaction, const bool enabled, const QString number, const bool isRegExp)
{
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
}

bool KMyMoneyView::checkTransactionPayee(const MyMoneyTransaction *transaction, const bool enabled, const QString payee, const bool isRegExp)
{
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
}

bool KMyMoneyView::checkTransactionCategory(const MyMoneyTransaction *transaction, const bool enabled, const QString category)
{
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
  return false;
}

QString KMyMoneyView::currentBankName(void)
{
  bool bankSuccess=false;
  if (m_file.isInitialised()) {
    MyMoneyBank bank = banksView->currentBank(bankSuccess);
    if (bankSuccess)
      return bank.name();
  }
  return i18n("Unknown Institution");
}

QString KMyMoneyView::currentAccountName(void)
{
  bool accountSuccess=false;
  if (m_file.isInitialised()) {
    MyMoneyAccount account = banksView->currentAccount(accountSuccess);
    if (accountSuccess)
      return account.name();
  }
  return i18n("Unknown Account");
}

void KMyMoneyView::fileBackup(){
}

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
