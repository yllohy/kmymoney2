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

#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <qlabel.h>
#include <qfile.h>
#include <qtextstream.h>

#include <stdio.h>

#include "knewbankdlg.h"
#include "knewaccountdlg.h"
#include "kendingbalancedlg.h"
#include "kcategoriesdlg.h"
#include "kpayeedlg.h"
#include "knewfiledlg.h"
#include "kfileinfodlg.h"
#include "kmymoneysettings.h"
#include "kmymoneyview.h"

KMyMoneyView::KMyMoneyView(QWidget *parent, const char *name)
 : KTabCtl(parent,name)
{
  m_mainView = new KMainView(this);
  m_scheduledView = new KScheduleView(this);

  addTab(m_mainView, i18n("Accounts"));
  addTab(m_scheduledView, i18n("Bills & Deposits"));
  QLabel *reportsLabel = new QLabel(i18n("Sorry, Reports not available yet"), this);
  addTab(reportsLabel, i18n("Reports"));
  QLabel *pluginsLabel = new QLabel(i18n("Sorry, Plugins not yet available"), this);
  addTab(pluginsLabel, i18n("Plugins"));

  connect(m_mainView, SIGNAL(transactionListChanged()), this, SLOT(slotTransactionListChanged()));

  connect(m_mainView, SIGNAL(accountRightMouseClick(const MyMoneyAccount, bool)), this, SLOT(slotAccountRightMouse(const MyMoneyAccount, bool)));
//  connect(m_mainView, SIGNAL(accountDoubleClick(const MyMoneyAccount)), this, SLOT(slotAccountDoubleClick(const MyMoneyAccount)));

  connect(m_mainView, SIGNAL(bankRightMouseClick(const MyMoneyBank, bool)), this, SLOT(slotBankRightMouse(const MyMoneyBank, bool)));
  connect(m_mainView, SIGNAL(bankSelected()), this, SLOT(slotBankSelected()));
  connect(m_mainView, SIGNAL(accountSelected()), this, SLOT(slotAccountSelected()));

  m_inReconciliation=false;
  m_reconcileInited=false;
  reconcileDlg=0;
  transactionFindDlg=0;
  transactionResultsDlg=0;
}

KMyMoneyView::~KMyMoneyView()
{
}

void KMyMoneyView::slotTransactionListChanged()
{
	//if (m_inReconciliation)
	  //reconcileDlg->updateData();
}

void KMyMoneyView::slotAccountRightMouse(const MyMoneyAccount, bool inList)
{
  KPopupMenu menu(this);
  menu.insertTitle(QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/22x22/actions/account.png")), i18n("Account Options"));
  int id0 = menu.insertItem(i18n("Open..."), this, SLOT(slotAccountDoubleClick()));
  menu.insertSeparator();
  int id1 = menu.insertItem(QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/16x16/actions/reconcile.png")), i18n("Reconcile..."), this, SLOT(slotAccountReconcile()));
  menu.insertSeparator();
  int id3 = menu.insertItem(QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/16x16/actions/account.png")), i18n("Edit..."), this, SLOT(slotAccountEdit()));
  int id4 = menu.insertItem(i18n("Delete..."), this, SLOT(slotAccountDelete()));
  menu.insertSeparator();
  int id5 = menu.insertItem(QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/16x16/actions/transaction_import.png")), i18n("Import ascii..."), this, SLOT(slotAccountImportAscii()));
  int id6 = menu.insertItem(QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/16x16/actions/transaction_export.png")), i18n("Export ascii..."), this, SLOT(slotAccountExportAscii()));
  if (!inList) {
    menu.setItemEnabled(id1, false);
    menu.setItemEnabled(id3, false);
    menu.setItemEnabled(id4, false);
    menu.setItemEnabled(id5, false);
    menu.setItemEnabled(id6, false);
  }
  menu.exec(QCursor::pos());
}

void KMyMoneyView::slotAccountDoubleClick(void)
{
  viewTransactionList();
}

void KMyMoneyView::slotBankRightMouse(const MyMoneyBank, bool inList)
{
  KPopupMenu menu(this);
  menu.insertTitle(QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/22x22/actions/bank.png")), i18n("Institution Options"));
  int id1 = menu.insertItem(QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/16x16/actions/bank.png")), i18n("New Institution..."), this, SLOT(slotBankNew()));
  int id2 = menu.insertItem(QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/16x16/actions/account.png")), i18n("New Account..."), this, SLOT(slotAccountNew()));
  int id3 = menu.insertItem(QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/16x16/actions/bank.png")), i18n("Edit..."), this, SLOT(slotBankEdit()));
  int id4 = menu.insertItem(i18n("Delete..."), this, SLOT(slotBankDelete()));
  if (!inList) {
    menu.setItemEnabled(id2, false);
    menu.setItemEnabled(id3, false);
    menu.setItemEnabled(id4, false);
  } else
    menu.setItemEnabled(id1, false);
  menu.exec(QCursor::pos());
}

void KMyMoneyView::slotBankEdit()
{
  if ( !m_file.isInitialised() ) {
    KMessageBox::information(this, i18n("No MyMoneyFile open"));
    return;
  }

  bool bankSuccess=false;
  MyMoneyBank bank = m_mainView->currentBank(bankSuccess);
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
        m_mainView->refreshBankView(m_file);
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
  MyMoneyBank bank = m_mainView->currentBank(bankSuccess);
  if (bankSuccess) {
    QString msg;
    msg.sprintf(i18n("Delete this bank: %s ?"), bank.name().latin1());
    if ((KMessageBox::questionYesNo(this, msg))==KMessageBox::No)
      return;

    m_file.removeBank(bank);

    if (m_file.bankCount()<=0)  // If no more banks exist
      emit bankOperations(false);

    m_mainView->refreshBankView(m_file);
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

	pBank = m_file.bank(m_mainView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotAccountEdit: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(m_mainView->currentAccount(accountSuccess));
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

  m_mainView->refreshBankView(m_file);
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

	pBank = m_file.bank(m_mainView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotAccountDelete: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(m_mainView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::slotAccountDelete: Unable to grab the current account");
    return;
  }

  QString prompt;
  prompt.sprintf(i18n("Delete this account ? :-\n%s"),
    pAccount->name().latin1());

  if ((KMessageBox::questionYesNo(this, prompt))==KMessageBox::No)
    return;

  pBank->removeAccount(*pAccount);
  if (pBank->accountCount()<=0) // If no more accounts exist
    emit accountOperations(false);

  m_mainView->refreshBankView(m_file);
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

  m_mainView->clear();
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
        error.sprintf(i18n("Error while reading file: Bad version number"));
        KMessageBox::information(this, error);
        break;
      case 2: // bad magic number
        error.sprintf(i18n("Error while reading file: Bad magic number"));
        KMessageBox::information(this, error);
        break;
      case 3: // File doesn't exist
        error.sprintf(i18n("Error while reading file: File doesn't exist"));
        KMessageBox::information(this, error);
        break;
      default:
        error.sprintf(i18n("Error while reading file %d"), ret);
        KMessageBox::information(this, error);
        break;
    }
    m_file.resetAllData();
//    delete m_file;
//    m_file=0;
    return false;
  }
  else {
    m_mainView->viewBankList();
    m_mainView->refreshBankView(m_file);
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
    m_mainView->clear();

    m_file.addBank(dlg.m_name, dlg.m_sortCode, dlg.m_city, dlg.m_street, dlg.m_postcode, dlg.m_telephone, dlg.m_managerName);
    m_mainView->refreshBankView(m_file);
    viewBankList();
    emit bankOperations(true);
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

	pBank = m_file.bank(m_mainView->currentBank(bankSuccess));
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
    m_mainView->refreshBankView(m_file);

    MyMoneyAccount accountTmp(dialog.accountNameText,
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
    viewBankList();
    emit accountOperations(true);
  }
}

void KMyMoneyView::slotAccountReconcile(void)
{
  MyMoneyMoney l_previousBal, l_endingBalGuess;
//  QListIterator<MyMoneyTransaction> it = m_file.transactionIterator(bankIndex, accountIndex);
	
  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(m_mainView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotAccountReconcile: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(m_mainView->currentAccount(accountSuccess));
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
      connect(m_mainView->getTransactionView(),SIGNAL(transactionListChanged()),reconcileDlg,SLOT(slotTransactionChanged()));
    	reconcileDlg->show();
    	m_inReconciliation = true;
  	  m_reconcileInited=true;
    } else {
      reconcileDlg->resetData(dlg.previousBalance, dlg.endingBalance, dlg.endingDate, *pBank, pAccount, m_file);
      reconcileDlg->show();
  	  m_inReconciliation = true;
    }
  }
}

void KMyMoneyView::slotAccountImportAscii(void)
{
  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(m_mainView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotAccountReconcile: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(m_mainView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::slotAccountReconcile: Unable to grab the current account");
    return;
  }

  KImportDlg *importDlg = new KImportDlg();
  connect( importDlg->btnBrowse, SIGNAL( clicked() ), importDlg, SLOT( slotBrowse() ) );

	int returncode = importDlg->exec();

	if(returncode)
	{
		readQIFFile(importDlg->txtFileImport->text(),pAccount);
    m_mainView->refreshTransactionView();
    qDebug("OK Pressed");
	}
	else
	{
   	qDebug("Cancel Pressed");
	}

	delete importDlg;
}

void KMyMoneyView::slotAccountExportAscii(void)
{
  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(m_mainView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotAccountReconcile: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(m_mainView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::slotAccountReconcile: Unable to grab the current account");
    return;
  }

  KExportDlg *exportDlg = new KExportDlg();
  connect( exportDlg->btnBrowse, SIGNAL( clicked() ), exportDlg, SLOT( slotBrowse() ) );

	int returncode = exportDlg->exec();

	if(returncode)
	{
		bool expCat, expAcct;

    expAcct = exportDlg->cbxAccount->isChecked();
 		expCat = exportDlg->cbxCategories->isChecked();
		QDate startDate = exportDlg->dateStartDate->getQDate();
		QDate endDate = exportDlg->dateEndDate->getQDate();
		writeQIFFile(exportDlg->txtFileExport->text(),pAccount,expCat,expAcct,startDate,endDate);
    qDebug("OK Pressed");
	}
	else
	{
   	qDebug("Cancel Pressed");
	}


	delete exportDlg;

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
  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(m_mainView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::slotreconcileFinished: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(m_mainView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::slotreconcileFinished: Unable to grab the current account");
    return;
  }

  if (success) {
    m_mainView->refreshTransactionView();
  } else
    KMessageBox::information(this, i18n("Reconciliation failed for some reason"));

  reconcileDlg->hide();
  m_inReconciliation=false;
}

void KMyMoneyView::newFile(void)
{
  if (m_file.isInitialised()) {
    if ((KMessageBox::questionYesNo(this, i18n("Money file already open\nClose ?")))==KMessageBox::No) {
      return;
    }
    closeFile();
    m_file.init();
    m_file.resetAllData();
  }
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
          minors.append(inner_buffer);
          inner_count++;
          inner_buffer = QString::null;
        }
        else if (inner_count==buffer.length()) {
          minors.append(inner_buffer);
          break;
        }
        else
          inner_buffer += buffer[inner_count++];
      }
      done3=true;
      if (done1 && done2 && done3)
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
            if (buffer=="true" || buffer=="TRUE")
              income = true;
            else if (buffer=="false" || buffer=="FALSE")
              income = false;
            else
              return false;
              tokenCount++;
              done1=true;
              buffer = QString::null;
              break;
          case 1: // name
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

  switch (m_mainView->viewing()) {
//    case KMainView::AccountList:
//      qDebug("currently viewing account list");
//      viewBankList();
//      break;
    case KMainView::TransactionList:
      viewBankList();
      break;
    default:
      break;
  }
}

void KMyMoneyView::viewBankList(void)
{
  if (!m_file.isInitialised())
    return;

  m_mainView->refreshBankView(m_file);
  m_mainView->viewBankList();
  qDebug("viewBankList called in kmymoneyview");
  emit bankOperations(true);
}

void KMyMoneyView::viewTransactionList(void)
{
  if (!m_file.isInitialised())
    return;

  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(m_mainView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::viewTransactionList: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(m_mainView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::viewTransactionList: Unable to grab the current account");
    return;
  }

  m_mainView->initTransactionView(&m_file, *pBank, *pAccount);
  m_mainView->viewTransactionList();
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

	pBank = m_file.bank(m_mainView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::settingsLists: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(m_mainView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::settingsLists: Unable to grab the current account");
    return;
  }

    // See which list we are viewing and then refresh it
    switch(m_mainView->viewing()) {
      case KMainView::BankList:
        m_mainView->refreshBankView(m_file);
        break;
//      case KMainView::AccountList:
//        m_mainView->refresh(KMainView::BankList, m_file);
//        break;
      case KMainView::TransactionList:
        m_mainView->refreshTransactionView();
        break;
      default:
        break;
    }
}

void KMyMoneyView::accountFind()
{
  if (!transactionFindDlg) {
    transactionFindDlg = new KFindTransactionDlg(0);
    connect(transactionFindDlg, SIGNAL(searchReady()), this, SLOT(doTransactionSearch()));
  }

  transactionFindDlg->show();
}

void KMyMoneyView::doTransactionSearch()
{
  bool bankSuccess=false, accountSuccess=false;
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_file.bank(m_mainView->currentBank(bankSuccess));
	if (!pBank || !bankSuccess) {
    qDebug("KMyMoneyView::doTransactionSearch: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(m_mainView->currentAccount(accountSuccess));
  if (!pAccount || !accountSuccess) {
    qDebug("KMyMoneyView::doTransactionSearch: Unable to grab the current account");
    return;
  }

 	bool doDate;
 	bool doAmount;
 	bool doCredit;
 	bool doStatus;
 	bool doDescription;
 	bool doNumber;
 	QString amountID;
 	QString creditID;
 	QString statusID;
 	QString description;
 	QString number;
  MyMoneyMoney money;
  QDate startDate;
  QDate endDate;
  bool descriptionRegExp;
  bool numberRegExp;

  transactionFindDlg->data(doDate, doAmount, doCredit, doStatus, doDescription, doNumber,
    amountID,
    creditID,
    statusID,
    description,
    number,
    money,
    startDate,
    endDate,
    descriptionRegExp,
    numberRegExp );

  QList<MyMoneyTransaction> transactionList;
//  QListIterator<MyMoneyTransaction> it = m_file.transactionIterator(bankIndex, accountIndex);
  MyMoneyTransaction *transaction;

  for ( transaction=pAccount->transactionFirst(); transaction; transaction=pAccount->transactionNext()) {
//    MyMoneyTransaction *transaction = it.current();
    if (checkTransactionDates(transaction, doDate, startDate, endDate) &&
      checkTransactionAmount(transaction, doAmount, amountID, money) &&
      checkTransactionCredit(transaction, doCredit, creditID) &&
      checkTransactionStatus(transaction, doStatus, statusID) &&
      checkTransactionDescription(transaction, doDescription, description, descriptionRegExp) &&
      checkTransactionNumber(transaction, doNumber, number, numberRegExp) ) {

      transactionList.append(new MyMoneyTransaction(
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

  if (!transactionResultsDlg) {
    transactionResultsDlg = new KTFindResultsDlg(0);
  }
  transactionResultsDlg->setList(transactionList);
  transactionResultsDlg->show();
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

  if (id==i18n("At Least")) {
    if (transaction->amount() >= amount)
      return true;
  } else if (id==i18n("At Most")) {
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

QString KMyMoneyView::currentBankName(void)
{
  bool bankSuccess=false;
  if (m_file.isInitialised()) {
    MyMoneyBank bank = m_mainView->currentBank(bankSuccess);
    if (bankSuccess)
      return bank.name();
  }
  return "Unknown Institution";
}

QString KMyMoneyView::currentAccountName(void)
{
  bool accountSuccess=false;
  if (m_file.isInitialised()) {
    MyMoneyAccount account = m_mainView->currentAccount(accountSuccess);
    if (accountSuccess)
      return account.name();
  }
  return "Unknown Account";
}

/** No descriptions */
void KMyMoneyView::readQIFFile(const QString& name, MyMoneyAccount *account){

	bool catmode = false;
  bool transmode = false;
	bool writecat = false;
	bool writetrans = false;
  bool cleared = false;
	int numcat = 0;
	int numtrans = 0;
  QString catname = "";
	QString expense = "";
	QString date = "";
	QString amount = "";
	QString type = "";
	QString payee = "";
	QString category = "";
  MyMoneyCategory *oldcategory = 0;

    QFile f(name);
    if ( f.open(IO_ReadOnly) ) {    // file opened successfully
        QTextStream t( &f );        // use a text stream
        QString s;
        //int n = 1;
        while ( !t.eof() ) {        // until end of file...
            s = t.readLine();       // line of text excluding '\n'
						if(s.left(9) == "!Type:Cat")
						{
							catmode = true;
							transmode = false;
						}
						else if(s.left(10) == "!Type:Bank")
						{
							qDebug("Found Bank Type");
             				transmode = true;
						  	catmode = false;
						}
						else if(s.left(5) == "!Type")
						{
							qDebug("Found Just Type");
             	catmode = false;
							transmode = false;
						}
						else if(catmode)
						{
							if(s.left(1) == "N")
							{
             		catname = s.mid(1);
							}
							else if(s.left(1) == "E")
							{
             		expense = "E";
							}
							else if(s.left(1) == "I")
							{
								expense = "I";
							}
            	else if(s.left(1) == "^")
							{
								writecat = true;
							}
						}
					  else if(transmode)
						{
             				if(s.left(1) == "^")
							{
               				writetrans = true;
							}
							if(s.left(1) == "D")
							{
			         			date = s.mid(1);
							}
							if(s.left(1) == "T")
							{
               				amount = s.mid(1);
							}
							if(s.left(1) == "N")
							{
								type = s.mid(1);
							}
							if(s.left(1) == "P")
							{
		           			payee = s.mid(1);
							}
							if(s.left(1) == "L")
							{
               				category = s.mid(1);
							}
							if(s.left(1) == "C")
							{
               				cleared = true;
							}
						}
						

						if(transmode && writetrans)
						{
							int slash = -1;
							int apost = -1;
							int checknum = 0;
							bool isnumber = false;
							QString checknumber = "";
							MyMoneyTransaction::transactionMethod transmethod;
							slash = date.find("/");							
							apost = date.find("'");
							QString month = date.left(slash);
							QString day = date.mid(slash + 1,2);
							day = day.stripWhiteSpace();
							QString year = date.mid(apost + 1,2);
							year = year.stripWhiteSpace();
							int intyear = year.toInt();
							if(intyear > 80)
								intyear = 1900 + year.toInt();
							else
								intyear = 2000 + year.toInt();
							int intmonth = month.toInt();
							int intday = day.toInt();
							checknum = type.toInt(&isnumber);
							if(isnumber == false)
							{
               				if(type == "ATM")
								{
								  if(amount.find("-") > -1)
                 					transmethod = MyMoneyTransaction::ATM;
									else
										transmethod = MyMoneyTransaction::Deposit;
								}
								else if(type == "DEP")
								{
									if(amount.find("-") == -1)
                 						transmethod = MyMoneyTransaction::Deposit;
									else
										transmethod = MyMoneyTransaction::Withdrawal;
								}
								else if(type == "TXFR")
								{
									if(amount.find("-") > -1)
                 						transmethod = MyMoneyTransaction::Transfer;
									else
										transmethod = MyMoneyTransaction::Deposit;
								}
								else if(type == "WTHD")
								{
									if(amount.find("-") > -1)
                 						transmethod = MyMoneyTransaction::Withdrawal;
									else
										transmethod = MyMoneyTransaction::Deposit;
								}
								else
								{
									if(amount.find("-") > -1)
                 						transmethod = MyMoneyTransaction::Withdrawal;
									else
										transmethod = MyMoneyTransaction::Deposit;
								}
								
							}
							else
							{
									if(amount.find("-") > -1)
               						transmethod = MyMoneyTransaction::Cheque;
									else
										transmethod = MyMoneyTransaction::Deposit;
									checknumber=type;
							}
							QDate transdate(intyear,intmonth,intday);
             				 int commaindex = amount.find(",");
							double dblamount = 0;
		          if(commaindex != -1)
			          dblamount = amount.remove(commaindex,1).toDouble();
		          else
			          dblamount = amount.toDouble();
							if(dblamount < 0)
								dblamount = dblamount * -1;
							MyMoneyMoney moneyamount(dblamount);
							QString majorcat = "";
							QString minorcat = "";
							int catindex = category.find(":");
							if(catindex == -1)				
								majorcat = category;
							else
							{
               	majorcat = category.left(catindex);
								minorcat = category.mid(catindex + 1);
							}
							
							MyMoneyTransaction::stateE transcleared;

							if(cleared == true)
								transcleared = MyMoneyTransaction::Reconciled;
							else
								transcleared = MyMoneyTransaction::Unreconciled;

              if (!payee.isEmpty())
                m_file.addPayee(payee);
              account->addTransaction(transmethod, checknumber, payee,
                                      moneyamount, transdate, majorcat, minorcat, "",payee,
                                      "", "", transcleared);
							qDebug("Date:%s",date.latin1());
							qDebug("Amount:%s",amount.latin1());
							qDebug("Type:%s",type.latin1());
							qDebug("Payee:%s",payee.latin1());
							qDebug("Category:%s",category.latin1());

			        date = "";
              amount = "";
							type = "";
		          payee = "";
              category = "";
              cleared = false;
							writetrans = false;
							numtrans += 1;
						}
						if(catmode && writecat)
						{
							QString majorcat = "";
							QString minorcat = "";
							bool minorcatexists = false;
							bool majorcatexists = false;
							int catindex = catname.find(":");
							if(catindex == -1)
								majorcat = catname;
							else
							{
               				majorcat = catname.left(catindex);
								minorcat = catname.mid(catindex + 1);
							}
  							QListIterator<MyMoneyCategory> it = m_file.categoryIterator();
  							for ( ; it.current(); ++it ) {
    							MyMoneyCategory *data = it.current();
								if((majorcat == data->name()) && (minorcat == ""))
								{
									majorcatexists = true;
                  					minorcatexists = true;
								}
								else if(majorcat == data->name())
								{
									oldcategory = it.current();
									majorcatexists = true;
    								for ( QStringList::Iterator it2 = data->minorCategories().begin(); it2 != data->minorCategories().end(); ++it2 ) {
                  						 if((*it2) == minorcat)
									 	{
									 		 minorcatexists = true;
									 	}
									}

    							}
  						}

  						MyMoneyCategory category;
							category.setName(majorcat);
							if(expense == "E")
								category.setIncome(false);
							if(expense == "I")
								category.setIncome(true);
							if(majorcatexists == true)
							{
								if((minorcatexists == false) && (minorcat != ""))
								{
									category.addMinorCategory(minorcat);
									oldcategory->addMinorCategory(minorcat);
									numcat += 1;
								}
							}
							else
							{
								if(minorcat != "")
									category.addMinorCategory(minorcat);
								m_file.addCategory(category.isIncome(), category.name(), category.minorCategories());
               	           numcat += 1;
							}
							catname = "";
							expense = "";
							writecat = false;
						}	
						//qDebug("%s",s.latin1());
        }
        f.close();
    }
	QString importmsg;
	importmsg.sprintf("%d Categories imported.\n%d Transactions imported.",numcat,numtrans);
    QMessageBox::information(this,"QIF Import",importmsg);

}
/** No descriptions */
void KMyMoneyView::writeQIFFile(const QString& name, MyMoneyAccount *account,bool expCat,bool expAcct,
																QDate startDate, QDate endDate){
	int numcat = 0;
	int numtrans = 0;

    QFile f(name);
    if ( f.open(IO_WriteOnly) ) {    // file opened successfully
      QTextStream t( &f );        // use a text stream

			if(expCat)
			{
				t << "!Type:Cat" << endl;
  			QListIterator<MyMoneyCategory> it = m_file.categoryIterator();
  			for ( ; it.current(); ++it ) {
    			MyMoneyCategory *data = it.current();
						t << "N" + data->name() << endl;
						if(data->isIncome())
							t << "I" << endl;
						else
							t << "E" << endl;
						t << "^" << endl;
						numcat += 1;
    				for ( QStringList::Iterator it2 = data->minorCategories().begin(); it2 != data->minorCategories().end(); ++it2 ) {
								t << "N" << data->name() << ":" << *it2 << endl;
								if(data->isIncome())
									t << "I" << endl;
								else
									t << "E" << endl;
								t << "^" << endl;
								numcat += 1;
						}
  			}       		
			}
			if(expAcct)
			{
				t << "!Type:Bank" << endl;
				MyMoneyTransaction *transaction;
    		for ( transaction = account->transactionFirst(); transaction; transaction=account->transactionNext())
        {
        	if((transaction->date() >= startDate) && (transaction->date() <= endDate))
					{
          	int year = transaction->date().year();
			if(year >=2000)
            	year -= 2000;
			else
				year -= 1900;
						int month = transaction->date().month();
						int day = transaction->date().day();
						double amount = transaction->amount().amount();
						if(transaction->type() == MyMoneyTransaction::Debit)
						  amount = amount * -1;
						QString transmethod;
						if(transaction->method() == MyMoneyTransaction::ATM)
							transmethod = "ATM";
						if(transaction->method() == MyMoneyTransaction::Deposit)
							transmethod = "DEP";
						if(transaction->method() == MyMoneyTransaction::Transfer)
							transmethod = "TXFR";
						if(transaction->method() == MyMoneyTransaction::Withdrawal)
							transmethod = "WTHD";
						if(transaction->method() == MyMoneyTransaction::Cheque)
							transmethod = transaction->number();
						QString Payee = transaction->payee();
						QString Category;
						if(transaction->categoryMinor() == "")
							Category = transaction->categoryMajor();
						else
							Category = transaction->categoryMajor() + ":" + transaction->categoryMinor();
							

						t << "D" << month << "/" << day << "'" << year << endl;
						t << "U" << amount << endl;
						t << "T" << amount << endl;
						if(transaction->state() == MyMoneyTransaction::Reconciled)
							t << "CX" << endl;
						t << "N" << transmethod << endl;
						t << "P" << Payee << endl;
						t << "L" << Category << endl;
						t << "^" << endl;
						numtrans += 1;

													
					}
				}
			}
      f.close();
		}
	QString exportmsg;
	exportmsg.sprintf("%d Categories exported.\n%d Transactions exported.",numcat,numtrans);
    QMessageBox::information(this,"QIF Export",exportmsg);

}
/** No descriptions */
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
