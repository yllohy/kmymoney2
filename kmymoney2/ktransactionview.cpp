/***************************************************************************
                          ktransactionview.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
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
#include <klocale.h>

#include <qpushbutton.h>
#include <kcombobox.h>
#include <qtabwidget.h>
#include <qtable.h>
#include <kpopupmenu.h>
#include <qinputdialog.h>

#include "ktransactionview.h"
#include "kmymoneysettings.h"
#include "knewcategorydlg.h"
#include <kmessagebox.h>
#include "widgets/kmymoneyedit.h"
#include "widgets/kmymoneydateinput.h"
#include "kmethodtableitem.h"
#include "kdatetableitem.h"
#include "kmemotableitem.h"
#include "knumbertableitem.h"
#include "kcategorytableitem.h"
#include "kmoneytableitem.h"
#include "kreconciledtableitem.h"
#include "widgets/kmymoneytable.h"

KTransactionView::KTransactionView(QWidget *parent, const char *name)
 : KTransactionViewDecl(parent,name)
{
  transactionsTable->setNumCols(9);
  transactionsTable->horizontalHeader()->setLabel(0, i18n("Method"));
	transactionsTable->horizontalHeader()->setLabel(1, i18n("Date"));
	transactionsTable->horizontalHeader()->setLabel(2, i18n("Number"));
	transactionsTable->horizontalHeader()->setLabel(3, i18n("Description"));
	transactionsTable->horizontalHeader()->setLabel(4, i18n("Category"));
	transactionsTable->horizontalHeader()->setLabel(5, i18n("C"));
	transactionsTable->horizontalHeader()->setLabel(6, i18n("Deposit"));
	transactionsTable->horizontalHeader()->setLabel(7, i18n("Withdrawal"));
	transactionsTable->horizontalHeader()->setLabel(8, i18n("Balance"));
	transactionsTable->setSelectionMode(QTable::NoSelection);

  KMyMoneySettings *p_settings = KMyMoneySettings::singleton();
  if (p_settings)
    transactionsTable->horizontalHeader()->setFont(p_settings->lists_headerFont());
	
	connect(chequeEnterBtn, SIGNAL(clicked()), this, SLOT(enterClicked()));
	connect(depositEnterBtn, SIGNAL(clicked()), this, SLOT(enterClicked()));
	connect(transferEnterBtn, SIGNAL(clicked()), this, SLOT(enterClicked()));
	connect(withdrawalEnterBtn, SIGNAL(clicked()), this, SLOT(enterClicked()));
	connect(atmEnterBtn, SIGNAL(clicked()), this, SLOT(enterClicked()));
	
	connect(chequeEditBtn, SIGNAL(clicked()), this, SLOT(editClicked()));
	connect(depositEditBtn, SIGNAL(clicked()), this, SLOT(editClicked()));
	connect(transferEditBtn, SIGNAL(clicked()), this, SLOT(editClicked()));
	connect(withdrawalEditBtn, SIGNAL(clicked()), this, SLOT(editClicked()));
	connect(atmEditBtn, SIGNAL(clicked()), this, SLOT(editClicked()));
	
	connect(chequeCancelBtn, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	connect(depositCancelBtn, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	connect(transferCancelBtn, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	connect(withdrawalCancelBtn, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	connect(atmCancelBtn, SIGNAL(clicked()), this, SLOT(cancelClicked()));
	
	connect(chequeNewBtn, SIGNAL(clicked()), this, SLOT(newClicked()));
	connect(depositNewBtn, SIGNAL(clicked()), this, SLOT(newClicked()));
	connect(transferNewBtn, SIGNAL(clicked()), this, SLOT(newClicked()));
	connect(withdrawalNewBtn, SIGNAL(clicked()), this, SLOT(newClicked()));
	connect(atmNewBtn, SIGNAL(clicked()), this, SLOT(newClicked()));
	
	connect(chequeCategoryMajorCombo, SIGNAL(activated(const QString&)),
	  this, SLOT(slotMajorCombo(const QString&)));
	connect(depositCategoryMajorCombo, SIGNAL(activated(const QString&)),
	  this, SLOT(slotMajorCombo(const QString&)));
	connect(transferCategoryMajorCombo, SIGNAL(activated(const QString&)),
	  this, SLOT(slotMajorCombo(const QString&)));
	connect(withdrawalCategoryMajorCombo, SIGNAL(activated(const QString&)),
	  this, SLOT(slotMajorCombo(const QString&)));
	connect(atmCategoryMajorCombo, SIGNAL(activated(const QString&)),
	  this, SLOT(slotMajorCombo(const QString&)));

  connect(tabbedInputBox, SIGNAL(currentChanged(QWidget*)),
    this, SLOT(slotTabSelected(QWidget*)));
  m_filePointer=0;

  connect(transactionsTable, SIGNAL(clicked(int, int, int, const QPoint&)),
    this, SLOT(slotFocusChange(int, int, int, const QPoint&)));
  connect(transactionsTable, SIGNAL(valueChanged(int, int)),
    this, SLOT(transactionCellEdited(int, int)));

  m_index = -1;
  m_inEditMode=false;
  m_showingInputBox=true;
}

KTransactionView::~KTransactionView()
{
}

void KTransactionView::slotFocusChange(int row, int, int button, const QPoint& /*point*/)
{
  if ((row != transactionsTable->numRows()-1) && (transactionsTable->numRows()>=1)) {
    if (row!=m_index && button==1) {
      m_focus = m_transactions.at(row)->method();
      switch (m_focus) {
        case MyMoneyTransaction::Cheque:
          tabbedInputBox->showPage(chequeTab);
          break;
        case MyMoneyTransaction::Deposit:
          tabbedInputBox->showPage(depositTab);
          break;
        case MyMoneyTransaction::Transfer:
          tabbedInputBox->showPage(transferTab);
          break;
        case MyMoneyTransaction::Withdrawal:
          tabbedInputBox->showPage(withdrawalTab);
          break;
        case MyMoneyTransaction::ATM:
          tabbedInputBox->showPage(atmTab);
          break;
      }
      setInputData(*m_transactions.at(row));
      viewMode();
    }
    m_index = row;
    QTableSelection sel;
    transactionsTable->clearSelection();
    sel.init(row, 0);
    sel.expandTo(row, 8);
    transactionsTable->addSelection(sel);
    if (button>=2) {
      KPopupMenu setAsMenu(i18n("Set As..."), this);
      setAsMenu.insertItem(i18n("Unreconciled (default)"), this, SLOT(slotTransactionUnReconciled()));
      setAsMenu.insertItem(i18n("Cleared"), this, SLOT(slotTransactionCleared()));

      KPopupMenu menu(i18n("Transaction Options"), this);
      menu.insertItem(i18n("Delete..."), this, SLOT(slotTransactionDelete()));
      menu.insertSeparator();
      menu.insertItem(i18n("Set as"), &setAsMenu);
      menu.exec(QCursor::pos());
    }
  } else
    m_index=-1;
}

void KTransactionView::slotTransactionDelete()
{
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_filePointer->bank(m_bankIndex);
	if (!pBank) {
    qDebug("KMyMoneyView::slotTransactionDelete: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(m_accountIndex);
  if (!pAccount) {
    qDebug("KMyMoneyView::slotTransactionDelete: Unable to grab the current account");
    return;
  }

  QString prompt;
  MyMoneyTransaction *transaction = m_transactions.at(m_index);
  if (!transaction)
    return;

  prompt.sprintf(i18n("Delete this transaction ? :-\n%s"), transaction->memo().latin1());
  if ((KMessageBox::questionYesNo(this, prompt))==KMessageBox::No)
    return;

  pAccount->removeTransaction(*transaction);
  updateTransactionList(-1);
  m_filePointer->setDirty(true);
	
  emit transactionListChanged();
}

void KTransactionView::slotTransactionUnReconciled()
{
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_filePointer->bank(m_bankIndex);
	if (!pBank) {
    qDebug("KMyMoneyView::slotTransactionUnReconciled: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(m_accountIndex);
  if (!pAccount) {
    qDebug("KMyMoneyView::slotTransactionUnReconciled: Unable to grab the current account");
    return;
  }

  MyMoneyTransaction *transaction = m_transactions.at(m_index);
  if (!transaction)
    return;

  transaction->setState(MyMoneyTransaction::Unreconciled);
  updateTransactionList(m_index, 5);
  m_filePointer->setDirty(true);
}

void KTransactionView::slotTransactionCleared()
{
  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_filePointer->bank(m_bankIndex);
	if (!pBank) {
    qDebug("KMyMoneyView::slotTransactionUnReconciled: Unable to get the current bank");
    return;
  }
  pAccount = pBank->account(m_accountIndex);
  if (!pAccount) {
    qDebug("KMyMoneyView::slotTransactionUnReconciled: Unable to grab the current account");
    return;
  }

  MyMoneyTransaction *transaction = m_transactions.at(m_index);
  if (!transaction)
    return;

  transaction->setState(MyMoneyTransaction::Cleared);
  updateTransactionList(m_index, 5);
  m_filePointer->setDirty(true);
}

void KTransactionView::init(MyMoneyFile *file, MyMoneyBank bank, MyMoneyAccount account)
{
  m_filePointer = file;
  m_bankIndex = bank;
  m_accountIndex = account;

  updateInputLists();
  updateTransactionList(-1);
}

void KTransactionView::clear(void)
{
  for (int i=0; i<transactionsTable->numRows(); i++)
    for (int j=0; j<=8; j++)
      transactionsTable->setText(i, j, "");
}

void KTransactionView::enterClicked()
{
  QPushButton* genericNewBtn;
  QPushButton* genericEditBtn;
  QPushButton* genericEnterBtn;
  QPushButton* genericCancelBtn;
  kMyMoneyDateInput* genericDateEdit;
  kMyMoneyEdit* genericAmountEdit;
  KComboBox* genericCategoryMajorCombo;
  KComboBox* genericCategoryMinorCombo;
  QLineEdit* genericNumberEdit;
  QLineEdit* genericMemoEdit;
  KComboBox *genericFromCombo = transferFromCombo;
  KComboBox *genericToCombo = transferToCombo;
  KComboBox *genericPayToCombo = chequePayToCombo;

  QWidget *tab = tabbedInputBox->currentPage();
  if (tab==chequeTab) {
      genericNewBtn = chequeNewBtn;
      genericEditBtn = chequeEditBtn;
      genericEnterBtn = chequeEnterBtn;
      genericCancelBtn = chequeCancelBtn;
      genericDateEdit = chequeDateEdit;
      genericPayToCombo = chequePayToCombo;
      genericAmountEdit = chequeAmountEdit;
      genericCategoryMajorCombo = chequeCategoryMajorCombo;
      genericCategoryMinorCombo = chequeCategoryMinorCombo;
      genericNumberEdit = chequeNumberEdit;
      genericMemoEdit = chequeMemoEdit;
  }
  else if (tab==depositTab) {
      genericNewBtn = depositNewBtn;
      genericEditBtn = depositEditBtn;
      genericEnterBtn = depositEnterBtn;
      genericCancelBtn = depositCancelBtn;
      genericDateEdit = depositDateEdit;
      genericFromCombo = depositFromCombo;
      genericAmountEdit = depositAmountEdit;
      genericCategoryMajorCombo = depositCategoryMajorCombo;
      genericCategoryMinorCombo = depositCategoryMinorCombo;
      genericNumberEdit = depositNumberEdit;
      genericMemoEdit = depositMemoEdit;
  }
  else if (tab==transferTab) {
      genericNewBtn = transferNewBtn;
      genericEditBtn = transferEditBtn;
      genericEnterBtn = transferEnterBtn;
      genericCancelBtn = transferCancelBtn;
      genericDateEdit = transferDateEdit;
      genericAmountEdit = transferAmountEdit;
      genericCategoryMajorCombo = transferCategoryMajorCombo;
      genericCategoryMinorCombo = transferCategoryMinorCombo;
      genericNumberEdit = transferNumberEdit;
      genericMemoEdit = transferMemoEdit;
  }
  else if (tab==withdrawalTab) {
      genericNewBtn = withdrawalNewBtn;
      genericEditBtn = withdrawalEditBtn;
      genericEnterBtn = withdrawalEnterBtn;
      genericCancelBtn = withdrawalCancelBtn;
      genericDateEdit = withdrawalDateEdit;
      genericPayToCombo = withdrawalPayToCombo;
      genericAmountEdit = withdrawalAmountEdit;
      genericCategoryMajorCombo = withdrawalCategoryMajorCombo;
      genericCategoryMinorCombo = withdrawalCategoryMinorCombo;
      genericNumberEdit = withdrawalNumberEdit;
      genericMemoEdit = withdrawalMemoEdit;
  }
  else if (tab==atmTab) {
      genericNewBtn = atmNewBtn;
      genericEditBtn = atmEditBtn;
      genericEnterBtn = atmEnterBtn;
      genericCancelBtn = atmCancelBtn;
      genericDateEdit = atmDateEdit;
      genericAmountEdit = atmAmountEdit;
      genericCategoryMajorCombo = atmCategoryMajorCombo;
      genericCategoryMinorCombo = atmCategoryMinorCombo;
      genericNumberEdit = atmNumberEdit;
      genericMemoEdit = atmMemoEdit;
  } else
      return;

  if (!m_filePointer)
    return;

  // Check all the required inputs, these will become optional at some point in the future
  QString majorText = genericCategoryMajorCombo->currentText();
  QString minorText = genericCategoryMinorCombo->currentText();

  if (majorText=="--- INCOME ---") {
    genericCategoryMajorCombo->setEditText(i18n("Other Income"));
    majorText = i18n("Other Income");
  } else if (majorText=="--- EXPENSE ---") {
    genericCategoryMajorCombo->setEditText(i18n("Other Expense"));
    majorText = i18n("Other Expense");
  }

  if (!majorText.isNull() && !minorText.isEmpty()) {
    bool found=false;
    QListIterator<MyMoneyCategory> it = m_filePointer->categoryIterator();
    for ( ; it.current(); ++it) {
      if (it.current()->name()==majorText) {
        found = true;
        break;
      }
    }
    MyMoneyCategory category;
    if (!found) {
      category.setIncome(true);
      category.setName(majorText);
      if (!minorText.isNull() && !minorText.isEmpty())
        category.addMinorCategory(minorText);
      KNewCategoryDlg dlg(&category, this);
      if (!dlg.exec())
        return;

      m_filePointer->addCategory(category.isIncome(), category.name(), category.minorCategories());
    } else {
      if (!minorText.isNull() && !minorText.isEmpty()) {
        category.setIncome(it.current()->isIncome());
        category.setName(it.current()->name());
        category.addMinorCategory(minorText);
        m_filePointer->addCategory(category.isIncome(), category.name(), category.minorCategories());
      }
    }

    switch (m_focus) {
      case MyMoneyTransaction::Cheque:
      case MyMoneyTransaction::Withdrawal:
      case MyMoneyTransaction::ATM:
        if (category.isIncome()) {
          KMessageBox::error(this, i18n("You have specified an income category for an expense.  This has been rectified\nThis will be optional in a later release"));
          QListIterator<MyMoneyCategory> it = m_filePointer->categoryIterator();
          for ( ; it.current(); ++it) {
            if (it.current()->name()==category.name())
              it.current()->setIncome(false);
            break;
          }
        }
        break;
      case MyMoneyTransaction::Deposit:
        if (!category.isIncome()) {
          KMessageBox::error(this, i18n("You have specified an expense category for an income.  This has been rectified\nThis will be optional in a later release"));
          QListIterator<MyMoneyCategory> it = m_filePointer->categoryIterator();
          for ( ; it.current(); ++it) {
            if (it.current()->name()==category.name())
              it.current()->setIncome(true);
            break;
          }
        }
        break;
      case MyMoneyTransaction::Transfer:
        break;
    }
  }

  MyMoneyMoney money = genericAmountEdit->getMoneyValue();

  MyMoneyTransaction transaction(0, m_focus,
      genericNumberEdit->text(),
      genericMemoEdit->text(),
      money,
      genericDateEdit->getQDate(),
      majorText,
      minorText,
      "",  // future addition
      genericPayToCombo->currentText(),
      genericFromCombo->currentText(),
      genericToCombo->currentText(),
      MyMoneyTransaction::Unreconciled );

  MyMoneyBank *pBank;
  MyMoneyAccount *pAccount;

	pBank = m_filePointer->bank(m_bankIndex);
	if (!pBank) {
    qDebug("KMyMoneyView::slotInputEnterClicked: Unable to get the current bank");
    return;
  }

  pAccount = pBank->account(m_accountIndex);
  if (!pAccount) {
    qDebug("KMyMoneyView::slotInputEnterClicked: Unable to grab the current account");
    return;
  }

  if (!m_inEditMode) {
   qDebug("KMyMoneyView::slotinputEnterClicked: adding a new transaction");
    pAccount->addTransaction(transaction.method(), transaction.number(), transaction.memo(),
            transaction.amount(), transaction.date(), transaction.categoryMajor(), transaction.categoryMinor(),
            transaction.atmBankName(), transaction.payee(), transaction.accountFrom(), transaction.accountTo(),
            transaction.state());
    updateTransactionList(-1);
  } else {
   qDebug("KMyMoneyView::slotinputEnterClicked: editing a transaction");
    MyMoneyTransaction *transactionWrite;
    if ( (transactionWrite=pAccount->transaction(*m_transactions.at(m_index))) ) {
      transactionWrite->setMethod(transaction.method());
      transactionWrite->setNumber(transaction.number());
      transactionWrite->setMemo(transaction.memo());
      transactionWrite->setAmount(transaction.amount());
      transactionWrite->setDate(transaction.date());
      transactionWrite->setCategoryMajor(transaction.categoryMajor());
      transactionWrite->setCategoryMinor(transaction.categoryMinor());
      transactionWrite->setAtmBankName(transaction.atmBankName());
      transactionWrite->setPayee(transaction.payee());
      transactionWrite->setAccountFrom(transaction.accountFrom());
      transactionWrite->setAccountTo(transaction.accountTo());
      transactionWrite->setState(transaction.state());
      updateTransactionList(m_index);
    } else {
      KMessageBox::information(this, i18n("Data not saved, unable to grab a pointer to a transaction"));
    }
  }

	m_filePointer->addPayee(transaction.payee());
	m_filePointer->setDirty(true);
	
	emit transactionListChanged();

  updateInputLists();
  viewMode();
}

void KTransactionView::slotTabSelected(QWidget *tab)
{
  if (tab==chequeTab)
      m_focus = MyMoneyTransaction::Cheque;
  else if (tab==depositTab)
      m_focus = MyMoneyTransaction::Deposit;
  else if (tab==transferTab)
      m_focus = MyMoneyTransaction::Transfer;
  else if (tab==withdrawalTab)
      m_focus = MyMoneyTransaction::Withdrawal;
  else if (tab==atmTab)
      m_focus = MyMoneyTransaction::ATM;
}

void KTransactionView::setInputData(const MyMoneyTransaction transaction)
{
  QPushButton* genericNewBtn;
  QPushButton* genericEditBtn;
  QPushButton* genericEnterBtn;
  QPushButton* genericCancelBtn;
  kMyMoneyDateInput* genericDateEdit;
  kMyMoneyEdit* genericAmountEdit;
  KComboBox* genericCategoryMajorCombo;
  KComboBox* genericCategoryMinorCombo;
  QLineEdit* genericNumberEdit;
  QLineEdit* genericMemoEdit;
  KComboBox *genericFromCombo = transferFromCombo;
  KComboBox *genericToCombo = transferToCombo;
  KComboBox *genericPayToCombo = chequePayToCombo;

  switch (m_focus) {
    case MyMoneyTransaction::Cheque:
      genericNewBtn = chequeNewBtn;
      genericEditBtn = chequeEditBtn;
      genericEnterBtn = chequeEnterBtn;
      genericCancelBtn = chequeCancelBtn;
      genericDateEdit = chequeDateEdit;
      genericPayToCombo = chequePayToCombo;
      genericAmountEdit = chequeAmountEdit;
      genericCategoryMajorCombo = chequeCategoryMajorCombo;
      genericCategoryMinorCombo = chequeCategoryMinorCombo;
      genericNumberEdit = chequeNumberEdit;
      genericMemoEdit = chequeMemoEdit;
      break;
    case MyMoneyTransaction::Deposit:
      genericNewBtn = depositNewBtn;
      genericEditBtn = depositEditBtn;
      genericEnterBtn = depositEnterBtn;
      genericCancelBtn = depositCancelBtn;
      genericDateEdit = depositDateEdit;
      genericFromCombo = depositFromCombo;
      genericAmountEdit = depositAmountEdit;
      genericCategoryMajorCombo = depositCategoryMajorCombo;
      genericCategoryMinorCombo = depositCategoryMinorCombo;
      genericNumberEdit = depositNumberEdit;
      genericMemoEdit = depositMemoEdit;
      break;
    case MyMoneyTransaction::Transfer:
      genericNewBtn = transferNewBtn;
      genericEditBtn = transferEditBtn;
      genericEnterBtn = transferEnterBtn;
      genericCancelBtn = transferCancelBtn;
      genericDateEdit = transferDateEdit;
      genericAmountEdit = transferAmountEdit;
      genericCategoryMajorCombo = transferCategoryMajorCombo;
      genericCategoryMinorCombo = transferCategoryMinorCombo;
      genericNumberEdit = transferNumberEdit;
      genericMemoEdit = transferMemoEdit;
      break;
    case MyMoneyTransaction::Withdrawal:
      genericNewBtn = withdrawalNewBtn;
      genericEditBtn = withdrawalEditBtn;
      genericEnterBtn = withdrawalEnterBtn;
      genericCancelBtn = withdrawalCancelBtn;
      genericDateEdit = withdrawalDateEdit;
      genericPayToCombo = withdrawalPayToCombo;
      genericAmountEdit = withdrawalAmountEdit;
      genericCategoryMajorCombo = withdrawalCategoryMajorCombo;
      genericCategoryMinorCombo = withdrawalCategoryMinorCombo;
      genericNumberEdit = withdrawalNumberEdit;
      genericMemoEdit = withdrawalMemoEdit;
      break;
    case MyMoneyTransaction::ATM:
      genericNewBtn = atmNewBtn;
      genericEditBtn = atmEditBtn;
      genericEnterBtn = atmEnterBtn;
      genericCancelBtn = atmCancelBtn;
      genericDateEdit = atmDateEdit;
      genericAmountEdit = atmAmountEdit;
      genericCategoryMajorCombo = atmCategoryMajorCombo;
      genericCategoryMinorCombo = atmCategoryMinorCombo;
      genericNumberEdit = atmNumberEdit;
      genericMemoEdit = atmMemoEdit;
    default:
      return;
  }

  genericCategoryMajorCombo->setEditText(transaction.categoryMajor());
  genericCategoryMinorCombo->setEditText(transaction.categoryMinor());
  genericFromCombo->setEditText(transaction.accountFrom());
  genericToCombo->setEditText(transaction.accountTo());
  genericPayToCombo->setEditText(transaction.payee());
  genericMemoEdit->setText(transaction.memo());
  genericNumberEdit->setText(transaction.number());
  genericDateEdit->setDate(transaction.date());
  genericAmountEdit->setText(KGlobal::locale()->formatMoney(transaction.amount().amount()));
}

void KTransactionView::slotMajorCombo(const QString& text)
{
  QPushButton* genericNewBtn;
  QPushButton* genericEditBtn;
  QPushButton* genericEnterBtn;
  QPushButton* genericCancelBtn;
  kMyMoneyDateInput* genericDateEdit;
  kMyMoneyEdit* genericAmountEdit;
  KComboBox* genericCategoryMajorCombo;
  KComboBox* genericCategoryMinorCombo;
  QLineEdit* genericNumberEdit;
  QLineEdit* genericMemoEdit;
  KComboBox *genericFromCombo = transferFromCombo;
  KComboBox *genericToCombo = transferToCombo;
  KComboBox *genericPayToCombo = chequePayToCombo;

  QWidget *tab = tabbedInputBox->currentPage();
  if (tab==chequeTab) {
      genericNewBtn = chequeNewBtn;
      genericEditBtn = chequeEditBtn;
      genericEnterBtn = chequeEnterBtn;
      genericCancelBtn = chequeCancelBtn;
      genericDateEdit = chequeDateEdit;
      genericPayToCombo = chequePayToCombo;
      genericAmountEdit = chequeAmountEdit;
      genericCategoryMajorCombo = chequeCategoryMajorCombo;
      genericCategoryMinorCombo = chequeCategoryMinorCombo;
      genericNumberEdit = chequeNumberEdit;
      genericMemoEdit = chequeMemoEdit;
  }
  else if (tab==depositTab) {
      genericNewBtn = depositNewBtn;
      genericEditBtn = depositEditBtn;
      genericEnterBtn = depositEnterBtn;
      genericCancelBtn = depositCancelBtn;
      genericDateEdit = depositDateEdit;
      genericFromCombo = depositFromCombo;
      genericAmountEdit = depositAmountEdit;
      genericCategoryMajorCombo = depositCategoryMajorCombo;
      genericCategoryMinorCombo = depositCategoryMinorCombo;
      genericNumberEdit = depositNumberEdit;
      genericMemoEdit = depositMemoEdit;
  }
  else if (tab==transferTab) {
      genericNewBtn = transferNewBtn;
      genericEditBtn = transferEditBtn;
      genericEnterBtn = transferEnterBtn;
      genericCancelBtn = transferCancelBtn;
      genericDateEdit = transferDateEdit;
      genericAmountEdit = transferAmountEdit;
      genericCategoryMajorCombo = transferCategoryMajorCombo;
      genericCategoryMinorCombo = transferCategoryMinorCombo;
      genericNumberEdit = transferNumberEdit;
      genericMemoEdit = transferMemoEdit;
  }
  else if (tab==withdrawalTab) {
      genericNewBtn = withdrawalNewBtn;
      genericEditBtn = withdrawalEditBtn;
      genericEnterBtn = withdrawalEnterBtn;
      genericCancelBtn = withdrawalCancelBtn;
      genericDateEdit = withdrawalDateEdit;
      genericPayToCombo = withdrawalPayToCombo;
      genericAmountEdit = withdrawalAmountEdit;
      genericCategoryMajorCombo = withdrawalCategoryMajorCombo;
      genericCategoryMinorCombo = withdrawalCategoryMinorCombo;
      genericNumberEdit = withdrawalNumberEdit;
      genericMemoEdit = withdrawalMemoEdit;
  }
  else if (tab==atmTab) {
      genericNewBtn = atmNewBtn;
      genericEditBtn = atmEditBtn;
      genericEnterBtn = atmEnterBtn;
      genericCancelBtn = atmCancelBtn;
      genericDateEdit = atmDateEdit;
      genericAmountEdit = atmAmountEdit;
      genericCategoryMajorCombo = atmCategoryMajorCombo;
      genericCategoryMinorCombo = atmCategoryMinorCombo;
      genericNumberEdit = atmNumberEdit;
      genericMemoEdit = atmMemoEdit;
  } else
      return;

  if (m_filePointer) {
    QString majorText = text;

    QListIterator<MyMoneyCategory> categoryIterator = m_filePointer->categoryIterator();

    if (text=="--- INCOME ---") {
      genericCategoryMajorCombo->setEditText(i18n("Other Income"));
      majorText = i18n("Other Income");
    } else if (text=="--- EXPENSE ---") {
      genericCategoryMajorCombo->setEditText(i18n("Other Expense"));
      majorText = i18n("Other Expense");
    }

    genericCategoryMinorCombo->clear();
    for ( ; categoryIterator.current(); ++categoryIterator) {
      MyMoneyCategory *data = categoryIterator.current();
      if (data->name() == majorText) {
        for ( QStringList::Iterator it = data->minorCategories().begin(); it != data->minorCategories().end(); ++it )
          genericCategoryMinorCombo->insertItem((*it).latin1());
      }
      genericCategoryMajorCombo->setEditText(majorText);
    }
  }
}

void KTransactionView::updateInputLists(void)
{
  if (!m_filePointer)
    return;

  qDebug("updateInputLists called");

  QPushButton* genericNewBtn;
  QPushButton* genericEditBtn;
  QPushButton* genericEnterBtn;
  QPushButton* genericCancelBtn;
  kMyMoneyDateInput* genericDateEdit;
  kMyMoneyEdit* genericAmountEdit;
  KComboBox* genericCategoryMajorCombo;
  KComboBox* genericCategoryMinorCombo;
  QLineEdit* genericNumberEdit;
  QLineEdit* genericMemoEdit;
  KComboBox *genericFromCombo = transferFromCombo;
  KComboBox *genericToCombo = transferToCombo;
  KComboBox *genericPayToCombo = chequePayToCombo;

  QWidget *tab = tabbedInputBox->currentPage();
  if (tab==chequeTab) {
      genericNewBtn = chequeNewBtn;
      genericEditBtn = chequeEditBtn;
      genericEnterBtn = chequeEnterBtn;
      genericCancelBtn = chequeCancelBtn;
      genericDateEdit = chequeDateEdit;
      genericPayToCombo = chequePayToCombo;
      genericAmountEdit = chequeAmountEdit;
      genericCategoryMajorCombo = chequeCategoryMajorCombo;
      genericCategoryMinorCombo = chequeCategoryMinorCombo;
      genericNumberEdit = chequeNumberEdit;
      genericMemoEdit = chequeMemoEdit;
  }
  else if (tab==depositTab) {
      genericNewBtn = depositNewBtn;
      genericEditBtn = depositEditBtn;
      genericEnterBtn = depositEnterBtn;
      genericCancelBtn = depositCancelBtn;
      genericDateEdit = depositDateEdit;
      genericFromCombo = depositFromCombo;
      genericAmountEdit = depositAmountEdit;
      genericCategoryMajorCombo = depositCategoryMajorCombo;
      genericCategoryMinorCombo = depositCategoryMinorCombo;
      genericNumberEdit = depositNumberEdit;
      genericMemoEdit = depositMemoEdit;
  }
  else if (tab==transferTab) {
      genericNewBtn = transferNewBtn;
      genericEditBtn = transferEditBtn;
      genericEnterBtn = transferEnterBtn;
      genericCancelBtn = transferCancelBtn;
      genericDateEdit = transferDateEdit;
      genericAmountEdit = transferAmountEdit;
      genericCategoryMajorCombo = transferCategoryMajorCombo;
      genericCategoryMinorCombo = transferCategoryMinorCombo;
      genericNumberEdit = transferNumberEdit;
      genericMemoEdit = transferMemoEdit;
  }
  else if (tab==withdrawalTab) {
      genericNewBtn = withdrawalNewBtn;
      genericEditBtn = withdrawalEditBtn;
      genericEnterBtn = withdrawalEnterBtn;
      genericCancelBtn = withdrawalCancelBtn;
      genericDateEdit = withdrawalDateEdit;
      genericPayToCombo = withdrawalPayToCombo;
      genericAmountEdit = withdrawalAmountEdit;
      genericCategoryMajorCombo = withdrawalCategoryMajorCombo;
      genericCategoryMinorCombo = withdrawalCategoryMinorCombo;
      genericNumberEdit = withdrawalNumberEdit;
      genericMemoEdit = withdrawalMemoEdit;
  }
  else if (tab==atmTab) {
      genericNewBtn = atmNewBtn;
      genericEditBtn = atmEditBtn;
      genericEnterBtn = atmEnterBtn;
      genericCancelBtn = atmCancelBtn;
      genericDateEdit = atmDateEdit;
      genericAmountEdit = atmAmountEdit;
      genericCategoryMajorCombo = atmCategoryMajorCombo;
      genericCategoryMinorCombo = atmCategoryMinorCombo;
      genericNumberEdit = atmNumberEdit;
      genericMemoEdit = atmMemoEdit;
  } else
      return;

    // Do expense first
    genericCategoryMajorCombo->clear();
    genericCategoryMajorCombo->insertItem("--- EXPENSE ---");
    QListIterator<MyMoneyCategory> categoryIterator = m_filePointer->categoryIterator();
    for ( ; categoryIterator.current(); ++categoryIterator) {
      MyMoneyCategory *data = categoryIterator.current();
      if (!data->isIncome())
        genericCategoryMajorCombo->insertItem(data->name());
    }

    // Then income
    genericCategoryMajorCombo->insertItem("--- INCOME ---");
    QListIterator<MyMoneyCategory> categoryIncomeIterator = m_filePointer->categoryIterator();
    for ( ; categoryIncomeIterator.current(); ++categoryIncomeIterator) {
      MyMoneyCategory *data = categoryIncomeIterator.current();
      if (data->isIncome())
        genericCategoryMajorCombo->insertItem(data->name());
    }

    genericFromCombo->clear();
    genericToCombo->clear();

    MyMoneyBank *bank;
    if (!(bank=m_filePointer->bank(m_bankIndex)))
      return;

    MyMoneyAccount *account;
    for ( account=bank->accountFirst(); account; account=bank->accountNext()) {
      genericFromCombo->insertItem(account->name());
      genericToCombo->insertItem(account->name());
    }

    genericPayToCombo->clear();
    QListIterator<MyMoneyPayee> payeeIterator = m_filePointer->payeeIterator();

    for ( ; payeeIterator.current(); ++payeeIterator) {
      MyMoneyPayee *payee = payeeIterator.current();
      genericPayToCombo->insertItem(payee->name());
    }

  genericPayToCombo->clearEdit();
  genericCategoryMajorCombo->clearEdit();
  genericCategoryMinorCombo->clearEdit();
}

void KTransactionView::updateTransactionList(int row, int col)
{
  if (!m_filePointer)
    return;

  qDebug("updateTransactionList called with %d %d", row, col);
  KMyMoneySettings *p_settings = KMyMoneySettings::singleton();
  if (p_settings) {
    transactionsTable->horizontalHeader()->setFont(p_settings->lists_headerFont());
  }

  MyMoneyBank *bank;
  MyMoneyAccount *account;

  bank = m_filePointer->bank(m_bankIndex);
  if (!bank) {
    qDebug("unable to find bank in updateData");
    return;
  }

  account = bank->account(m_accountIndex);
  if (!account) {
    qDebug("Unable to find account in updateData");
    return;
  }

  MyMoneyMoney balance;
  MyMoneyTransaction *transaction;
  int rowCount=0;

  if (row==-1) { // We are going to refresh the whole list
    transactionsTable->setColumnStretchable(3, true);
    transactionsTable->setColumnWidth(5, 20);
    m_transactions.clear();
    m_index=-1;
    clear();
    transactionsTable->setNumRows(account->transactionCount()+1);

    for ( transaction = account->transactionFirst(); transaction; transaction=account->transactionNext() ) {
      m_transactions.append(transaction);
      QString colText;

      switch (transaction->method()) {
        case MyMoneyTransaction::Cheque:
          colText = "Cheque";
          break;
        case MyMoneyTransaction::Deposit:
          colText = "Deposit";
          break;
        case MyMoneyTransaction::Transfer:
          colText = "Transfer";
          break;
        case MyMoneyTransaction::Withdrawal:
          colText = "Withdrawal";
          break;
        case MyMoneyTransaction::ATM:
          colText = "ATM";
          break;
      }
      KMethodTableItem *item0;
      if (m_showingInputBox)
        item0 = new KMethodTableItem(transactionsTable, QTableItem::Never, colText);
      else
        item0 = new KMethodTableItem(transactionsTable, QTableItem::OnTyping, colText);
      transactionsTable->setItem(rowCount, 0, item0);

      KDateTableItem *item1;
      if (m_showingInputBox)
        item1 = new KDateTableItem(transactionsTable, QTableItem::Never, KGlobal::locale()->formatDate(transaction->date(), true));
      else
        item1 = new KDateTableItem(transactionsTable, QTableItem::OnTyping, KGlobal::locale()->formatDate(transaction->date(), true));
      transactionsTable->setItem(rowCount, 1, item1);

      KNumberTableItem *item2;
      if (m_showingInputBox)
        item2 = new KNumberTableItem(transactionsTable, QTableItem::Never, transaction->number());
      else
        item2 = new KNumberTableItem(transactionsTable, QTableItem::OnTyping, transaction->number());
      transactionsTable->setItem(rowCount, 2, item2);

      KMemoTableItem *item3;
      if (m_showingInputBox)
        item3 = new KMemoTableItem(transactionsTable, QTableItem::Never, transaction->memo());
      else
        item3 = new KMemoTableItem(transactionsTable, QTableItem::OnTyping, transaction->memo());
      transactionsTable->setItem(rowCount, 3, item3);

      QString txt;
      txt.sprintf("%s:%s", transaction->categoryMajor().latin1(), transaction->categoryMinor().latin1());
      KCategoryTableItem *item4;
      if (m_showingInputBox)
        item4 = new KCategoryTableItem(transactionsTable, QTableItem::Never, txt, m_filePointer);
      else
        item4 = new KCategoryTableItem(transactionsTable, QTableItem::OnTyping, txt, m_filePointer);
      transactionsTable->setItem(rowCount, 4, item4);

      QString cLet;
      switch (transaction->state()) {
        case MyMoneyTransaction::Cleared:
          colText = "C";
          break;
        case MyMoneyTransaction::Reconciled:
          colText = "R";
          break;
        default:
          colText = " ";
          break;
      }
      KReconciledTableItem *item5 = new KReconciledTableItem(transactionsTable, QTableItem::Never, colText);
      transactionsTable->setItem(rowCount, 5, item5);

      KMoneyTableItem *item6;
      if (m_showingInputBox)
        item6 = new KMoneyTableItem(transactionsTable, QTableItem::Never, ((transaction->type()==MyMoneyTransaction::Credit) ? KGlobal::locale()->formatMoney(transaction->amount().amount()) : QString("")));
      else
        item6 = new KMoneyTableItem(transactionsTable, QTableItem::OnTyping, ((transaction->type()==MyMoneyTransaction::Credit) ? KGlobal::locale()->formatMoney(transaction->amount().amount()) : QString("")));
      transactionsTable->setItem(rowCount, 6, item6);

      KMoneyTableItem *item7;
      if (m_showingInputBox)
        item7 = new KMoneyTableItem(transactionsTable, QTableItem::Never, ((transaction->type()==MyMoneyTransaction::Debit) ? KGlobal::locale()->formatMoney(transaction->amount().amount()) : QString("")));
      else
        item7 = new KMoneyTableItem(transactionsTable, QTableItem::OnTyping, ((transaction->type()==MyMoneyTransaction::Debit) ? KGlobal::locale()->formatMoney(transaction->amount().amount()) : QString("")));
      transactionsTable->setItem(rowCount, 7, item7);

      if (transaction->type()==MyMoneyTransaction::Credit)
        balance += transaction->amount();
      else
        balance -= transaction->amount();

      KMoneyTableItem *item8 = new KMoneyTableItem(transactionsTable, QTableItem::Never, KGlobal::locale()->formatMoney(balance.amount()));
      transactionsTable->setItem(rowCount, 8, item8);

      rowCount++;
    }

    // Add the last empty row
    KMethodTableItem *item0;
    if (m_showingInputBox)
      item0 = new KMethodTableItem(transactionsTable, QTableItem::Never, "");
    else
      item0 = new KMethodTableItem(transactionsTable, QTableItem::OnTyping, "");
    transactionsTable->setItem(rowCount, 0, item0);

    KDateTableItem *item1;
    if (m_showingInputBox)
      item1 = new KDateTableItem(transactionsTable, QTableItem::Never, "");
    else
      item1 = new KDateTableItem(transactionsTable, QTableItem::OnTyping, "");
    transactionsTable->setItem(rowCount, 1, item1);

    KNumberTableItem *item2;
    if (m_showingInputBox)
      item2 = new KNumberTableItem(transactionsTable, QTableItem::Never, "");
    else
      item2 = new KNumberTableItem(transactionsTable, QTableItem::OnTyping, "");
    transactionsTable->setItem(rowCount, 2, item2);

    KMemoTableItem *item3;
    if (m_showingInputBox)
      item3 = new KMemoTableItem(transactionsTable, QTableItem::Never, "");
    else
      item3 = new KMemoTableItem(transactionsTable, QTableItem::OnTyping, "");
    transactionsTable->setItem(rowCount, 3, item3);

    KCategoryTableItem *item4;
    if (m_showingInputBox)
      item4 = new KCategoryTableItem(transactionsTable, QTableItem::Never, "", m_filePointer);
    else
      item4 = new KCategoryTableItem(transactionsTable, QTableItem::OnTyping, "", m_filePointer);
    transactionsTable->setItem(rowCount, 4, item4);

    KReconciledTableItem *item5 = new KReconciledTableItem(transactionsTable, QTableItem::Never, "");
    transactionsTable->setItem(rowCount, 5, item5);

    KMoneyTableItem *item6;
    if (m_showingInputBox)
      item6 = new KMoneyTableItem(transactionsTable, QTableItem::Never, "");
    else
      item6 = new KMoneyTableItem(transactionsTable, QTableItem::OnTyping, "");
    transactionsTable->setItem(rowCount, 6, item6);

    KMoneyTableItem *item7;
    if (m_showingInputBox)
      item7 = new KMoneyTableItem(transactionsTable, QTableItem::Never, "");
    else
      item7 = new KMoneyTableItem(transactionsTable, QTableItem::OnTyping, "");
    transactionsTable->setItem(rowCount, 7, item7);

    KMoneyTableItem *item8 = new KMoneyTableItem(transactionsTable, QTableItem::Never, "");
    transactionsTable->setItem(rowCount, 8, item8);
  } else { // We are just updating a section of it
    qDebug("update called with %d and %d", row, col);
    QString txt;
    if (row<0 || row>transactionsTable->numRows()-1)
      return;
    if (col<0 || col>transactionsTable->numCols()-1)
      return;
    switch (col) {
      case 0:
        switch (m_transactions.at(row)->method()) {
          case MyMoneyTransaction::Cheque:
            transactionsTable->setText(row, col, i18n("Cheque"));
            break;
          case MyMoneyTransaction::Deposit:
            transactionsTable->setText(row, col, i18n("Deposit"));
            break;
          case MyMoneyTransaction::Transfer:
            transactionsTable->setText(row, col, i18n("Transfer"));
            break;
          case MyMoneyTransaction::Withdrawal:
            transactionsTable->setText(row, col, i18n("Withdrawal"));
            break;
          case MyMoneyTransaction::ATM:
            transactionsTable->setText(row, col, i18n("ATM"));
            break;
        }
        break;
      case 1:
        transactionsTable->setText(row, col, KGlobal::locale()->formatDate(m_transactions.at(row)->date()));
        break;
      case 2:
        transactionsTable->setText(row, col, m_transactions.at(row)->number());
        break;
      case 3:
        transactionsTable->setText(row, col, m_transactions.at(row)->memo());
        break;
      case 4:
        txt.sprintf("%s:%s", m_transactions.at(row)->categoryMajor().latin1(), m_transactions.at(row)->categoryMinor().latin1());
        transactionsTable->setText(row, col, txt);
        break;
      case 5:
        switch (m_transactions.at(row)->state()) {
          case MyMoneyTransaction::Unreconciled:
            transactionsTable->setText(row, col, "");
            break;
          case MyMoneyTransaction::Cleared:
            transactionsTable->setText(row, col, "C");
            break;
          case MyMoneyTransaction::Reconciled:
            transactionsTable->setText(row, col, "R");
            break;
        }
        break;
      case 6:
        transactionsTable->setText(row, col, KGlobal::locale()->formatMoney(m_transactions.at(row)->amount().amount()));
        break;
      case 7:
        transactionsTable->setText(row, col, KGlobal::locale()->formatMoney(m_transactions.at(row)->amount().amount()));
        break;
    }
  }

  transactionsTable->ensureCellVisible(rowCount, 0);
}

void KTransactionView::viewMode(void)
{
  QPushButton* genericNewBtn;
  QPushButton* genericEditBtn;
  QPushButton* genericEnterBtn;
  QPushButton* genericCancelBtn;
  kMyMoneyDateInput* genericDateEdit;
  kMyMoneyEdit* genericAmountEdit;
  KComboBox* genericCategoryMajorCombo;
  KComboBox* genericCategoryMinorCombo;
  QLineEdit* genericNumberEdit;
  QLineEdit* genericMemoEdit;
  KComboBox *genericFromCombo = transferFromCombo;
  KComboBox *genericToCombo = transferToCombo;
  KComboBox *genericPayToCombo = chequePayToCombo;

  QWidget *tab = tabbedInputBox->currentPage();
  if (tab==chequeTab) {
      genericNewBtn = chequeNewBtn;
      genericEditBtn = chequeEditBtn;
      genericEnterBtn = chequeEnterBtn;
      genericCancelBtn = chequeCancelBtn;
      genericDateEdit = chequeDateEdit;
      genericPayToCombo = chequePayToCombo;
      genericAmountEdit = chequeAmountEdit;
      genericCategoryMajorCombo = chequeCategoryMajorCombo;
      genericCategoryMinorCombo = chequeCategoryMinorCombo;
      genericNumberEdit = chequeNumberEdit;
      genericMemoEdit = chequeMemoEdit;
  }
  else if (tab==depositTab) {
      genericNewBtn = depositNewBtn;
      genericEditBtn = depositEditBtn;
      genericEnterBtn = depositEnterBtn;
      genericCancelBtn = depositCancelBtn;
      genericDateEdit = depositDateEdit;
      genericFromCombo = depositFromCombo;
      genericAmountEdit = depositAmountEdit;
      genericCategoryMajorCombo = depositCategoryMajorCombo;
      genericCategoryMinorCombo = depositCategoryMinorCombo;
      genericNumberEdit = depositNumberEdit;
      genericMemoEdit = depositMemoEdit;
  }
  else if (tab==transferTab) {
      genericNewBtn = transferNewBtn;
      genericEditBtn = transferEditBtn;
      genericEnterBtn = transferEnterBtn;
      genericCancelBtn = transferCancelBtn;
      genericDateEdit = transferDateEdit;
      genericAmountEdit = transferAmountEdit;
      genericCategoryMajorCombo = transferCategoryMajorCombo;
      genericCategoryMinorCombo = transferCategoryMinorCombo;
      genericNumberEdit = transferNumberEdit;
      genericMemoEdit = transferMemoEdit;
  }
  else if (tab==withdrawalTab) {
      genericNewBtn = withdrawalNewBtn;
      genericEditBtn = withdrawalEditBtn;
      genericEnterBtn = withdrawalEnterBtn;
      genericCancelBtn = withdrawalCancelBtn;
      genericDateEdit = withdrawalDateEdit;
      genericPayToCombo = withdrawalPayToCombo;
      genericAmountEdit = withdrawalAmountEdit;
      genericCategoryMajorCombo = withdrawalCategoryMajorCombo;
      genericCategoryMinorCombo = withdrawalCategoryMinorCombo;
      genericNumberEdit = withdrawalNumberEdit;
      genericMemoEdit = withdrawalMemoEdit;
  }
  else if (tab==atmTab) {
      genericNewBtn = atmNewBtn;
      genericEditBtn = atmEditBtn;
      genericEnterBtn = atmEnterBtn;
      genericCancelBtn = atmCancelBtn;
      genericDateEdit = atmDateEdit;
      genericAmountEdit = atmAmountEdit;
      genericCategoryMajorCombo = atmCategoryMajorCombo;
      genericCategoryMinorCombo = atmCategoryMinorCombo;
      genericNumberEdit = atmNumberEdit;
      genericMemoEdit = atmMemoEdit;
  } else
      return;

  genericNewBtn->setEnabled(true);
  genericEditBtn->setEnabled(true);
  genericEnterBtn->setEnabled(false);
  genericCancelBtn->setEnabled(false);
  genericDateEdit->setEnabled(false);
  genericAmountEdit->setEnabled(false);
  genericCategoryMajorCombo->setEnabled(false);
  genericCategoryMinorCombo->setEnabled(false);
  genericNumberEdit->setEnabled(false);
  genericMemoEdit->setEnabled(false);
  genericFromCombo->setEnabled(false);
  genericToCombo->setEnabled(false);
  genericPayToCombo->setEnabled(false);
  m_inEditMode=false;
}

void KTransactionView::editMode(void)
{
  QPushButton* genericNewBtn;
  QPushButton* genericEditBtn;
  QPushButton* genericEnterBtn;
  QPushButton* genericCancelBtn;
  kMyMoneyDateInput* genericDateEdit;
  kMyMoneyEdit* genericAmountEdit;
  KComboBox* genericCategoryMajorCombo;
  KComboBox* genericCategoryMinorCombo;
  QLineEdit* genericNumberEdit;
  QLineEdit* genericMemoEdit;
  KComboBox *genericFromCombo = transferFromCombo;
  KComboBox *genericToCombo = transferToCombo;
  KComboBox *genericPayToCombo = chequePayToCombo;

  QWidget *tab = tabbedInputBox->currentPage();
  if (tab==chequeTab) {
      genericNewBtn = chequeNewBtn;
      genericEditBtn = chequeEditBtn;
      genericEnterBtn = chequeEnterBtn;
      genericCancelBtn = chequeCancelBtn;
      genericDateEdit = chequeDateEdit;
      genericPayToCombo = chequePayToCombo;
      genericAmountEdit = chequeAmountEdit;
      genericCategoryMajorCombo = chequeCategoryMajorCombo;
      genericCategoryMinorCombo = chequeCategoryMinorCombo;
      genericNumberEdit = chequeNumberEdit;
      genericMemoEdit = chequeMemoEdit;
  }
  else if (tab==depositTab) {
      genericNewBtn = depositNewBtn;
      genericEditBtn = depositEditBtn;
      genericEnterBtn = depositEnterBtn;
      genericCancelBtn = depositCancelBtn;
      genericDateEdit = depositDateEdit;
      genericFromCombo = depositFromCombo;
      genericAmountEdit = depositAmountEdit;
      genericCategoryMajorCombo = depositCategoryMajorCombo;
      genericCategoryMinorCombo = depositCategoryMinorCombo;
      genericNumberEdit = depositNumberEdit;
      genericMemoEdit = depositMemoEdit;
  }
  else if (tab==transferTab) {
      genericNewBtn = transferNewBtn;
      genericEditBtn = transferEditBtn;
      genericEnterBtn = transferEnterBtn;
      genericCancelBtn = transferCancelBtn;
      genericDateEdit = transferDateEdit;
      genericAmountEdit = transferAmountEdit;
      genericCategoryMajorCombo = transferCategoryMajorCombo;
      genericCategoryMinorCombo = transferCategoryMinorCombo;
      genericNumberEdit = transferNumberEdit;
      genericMemoEdit = transferMemoEdit;
  }
  else if (tab==withdrawalTab) {
      genericNewBtn = withdrawalNewBtn;
      genericEditBtn = withdrawalEditBtn;
      genericEnterBtn = withdrawalEnterBtn;
      genericCancelBtn = withdrawalCancelBtn;
      genericDateEdit = withdrawalDateEdit;
      genericPayToCombo = withdrawalPayToCombo;
      genericAmountEdit = withdrawalAmountEdit;
      genericCategoryMajorCombo = withdrawalCategoryMajorCombo;
      genericCategoryMinorCombo = withdrawalCategoryMinorCombo;
      genericNumberEdit = withdrawalNumberEdit;
      genericMemoEdit = withdrawalMemoEdit;
  }
  else if (tab==atmTab) {
      genericNewBtn = atmNewBtn;
      genericEditBtn = atmEditBtn;
      genericEnterBtn = atmEnterBtn;
      genericCancelBtn = atmCancelBtn;
      genericDateEdit = atmDateEdit;
      genericAmountEdit = atmAmountEdit;
      genericCategoryMajorCombo = atmCategoryMajorCombo;
      genericCategoryMinorCombo = atmCategoryMinorCombo;
      genericNumberEdit = atmNumberEdit;
      genericMemoEdit = atmMemoEdit;
  } else
      return;

  genericNewBtn->setEnabled(false);
  genericEditBtn->setEnabled(false);
  genericEnterBtn->setEnabled(true);
  genericCancelBtn->setEnabled(true);
  genericDateEdit->setEnabled(true);
  genericAmountEdit->setEnabled(true);
  genericCategoryMajorCombo->setEnabled(true);
  genericCategoryMinorCombo->setEnabled(true);
  genericNumberEdit->setEnabled(true);
  genericMemoEdit->setEnabled(true);
  genericFromCombo->setEnabled(true);
  genericToCombo->setEnabled(true);
  genericPayToCombo->setEnabled(true);
  m_inEditMode=true;
}

void KTransactionView::transactionCellEdited(int row, int col)
{
  if (transactionsTable->cellEditedOriginalText()==transactionsTable->text(row, col))
    return;

  MyMoneyBank *bank;
  MyMoneyAccount *account;

  bank = m_filePointer->bank(m_bankIndex);
  if (!bank) {
    qDebug("unable to find bank in updateData");
    return;
  }

  account = bank->account(m_accountIndex);
  if (!account) {
    qDebug("Unable to find account in updateData");
    return;
  }

  bool editing=true;
  MyMoneyTransaction *transaction;
  if (row == transactionsTable->numRows()-1) {
    transaction = new MyMoneyTransaction();
    qDebug("Adding a new transaction");
    editing=false;
  } else {
    qDebug("Editing a transaction");
    transaction = m_transactions.at(row);
    if (!transaction) {
     qDebug("Unable to find transaction in list");
     return;
   }
  }

   QString colText;
   QListIterator<MyMoneyPayee> payeeIterator = m_filePointer->payeeIterator();
   bool done=false;
   bool checkIt=false;
   QStringList options;
   int payeePos=0;
  KDateTableItem *dateItem;
  KMethodTableItem *methodItem;
  KCategoryTableItem *categoryItem;
  KMoneyTableItem *moneyItem;
  bool columnSwitched=false;

  for ( int n=0; payeeIterator.current(); ++payeeIterator, n++) {
    MyMoneyPayee *payee = payeeIterator.current();
    options.append(payee->name());
    if (editing) {
      if (payee->name()==m_transactions.at(row)->payee())
        payeePos=n;
    }
  }

   switch (col) {
     case 0: // The Method column
       methodItem = (KMethodTableItem*)transactionsTable->item(row, col);
       if (!methodItem) {
        qDebug("Unable to get method table item");
        return;
       }

       switch (methodItem->method()) {
        case MyMoneyTransaction::ATM:
         checkIt=false;
         break;
        default:
          checkIt=true;
       }

       done=false;
       if (checkIt&&editing) {
          if (transaction->payee()==QString::null || transaction->payee().isEmpty()) {
            if (methodItem->method()==MyMoneyTransaction::Transfer) {
              qDebug("TODO: Need a dialog to input the bank transfers");
            } else {
              while (!done) {
                bool ok = FALSE;
                QString text = QInputDialog::getItem( i18n( "Additional details needed" ), i18n( "Choose the payee" ), options, payeePos, true, &ok, this );
                if ( ok && !text.isEmpty() ) {
                  done=true;
                  transaction->setPayee(text);
                  m_filePointer->addPayee(text);
                }
              }
            }
          }
        }

       if (editing) {
         transaction->setMethod(methodItem->method());
         if (methodItem->method()==MyMoneyTransaction::ATM) {
            transaction->setPayee(i18n("Cash"));
            m_filePointer->addPayee(i18n("Cash"));
         }
         updateTransactionList(row);
       }
       break;
     case 1: // The Date column
       dateItem = (KDateTableItem*)transactionsTable->item(row, col);
       if (!dateItem) {
        qDebug("Unable to grab pointer to item");
        return;
       }

       if (dateItem->date().isValid()) {
        if (editing) {
          transaction->setDate(dateItem->date());
          updateTransactionList(row);
        }
       }
       break;
     case 2: // The Number column
       colText = transactionsTable->text(row, col);
      if (editing) {
        transaction->setNumber(colText);
        updateTransactionList(row);
       }
       break;
     case 3: // The Description column
       colText = transactionsTable->text(row, col);
       if (editing) {
        transaction->setMemo(colText);
        updateTransactionList(row);
       }
       break;
     case 4: // The Category column
       categoryItem = (KCategoryTableItem*)transactionsTable->item(row, col);
       if (!categoryItem) {
        qDebug("Unable to get category item");
        return;
       }

       if (editing) {
        transaction->setCategoryMajor(categoryItem->category().name());
        transaction->setCategoryMinor(categoryItem->category().firstMinor());
        updateTransactionList(row);
       }
       break;
     case 6: // The Deposit column
      moneyItem = (KMoneyTableItem*)transactionsTable->item(row, col);
      if (!moneyItem) {
        qDebug("Unable to get money item");
        return;
      }

      if (editing) {
        switch (transaction->method()) {
          case MyMoneyTransaction::Cheque:
          case MyMoneyTransaction::Withdrawal:
          case MyMoneyTransaction::ATM:
            columnSwitched=true;
          default:
            columnSwitched=false;
        }
        if (!columnSwitched && transaction->method()==MyMoneyTransaction::Transfer) {
          if (transaction->accountTo()==m_accountIndex.name())
            columnSwitched=true;
        }

        if (columnSwitched) {
          QStringList lst;
          bool ok=false;
          lst << "Deposit" << "Transfer";
          QString res = QInputDialog::getItem(i18n("Additional details neeeded"), i18n("Please select an item"), lst, 0, true, &ok, this );
          if (res=="Transfer") {
            qDebug("TODO: Need a dialog to input the bank transfers");
            transaction->setMethod(MyMoneyTransaction::Transfer);
          } else {
            transaction->setMethod(MyMoneyTransaction::Deposit);
            done=false;
            while (!done) {
              bool ok = false;
              QString text = QInputDialog::getItem( i18n( "Additional details needed" ), i18n( "Choose the payee" ), options, payeePos, true, &ok, this );
              if ( ok && !text.isEmpty() ) {
                done=true;
                transaction->setPayee(text);
                m_filePointer->addPayee(text);
              }
            }
          }
        }
        updateTransactionList(row);
      } else { // Not editing
        // Get all the data
        methodItem = (KMethodTableItem*)transactionsTable->item(row, 0);
        MyMoneyTransaction::transactionMethod transaction_method = methodItem->method();
        dateItem = (KDateTableItem*)transactionsTable->item(row, 1);
        QDate transaction_date = dateItem->date();
        categoryItem = (KCategoryTableItem*)transactionsTable->item(row, 4);
        QString transaction_major = categoryItem->category().name();
        QString transaction_minor = categoryItem->category().firstMinor();

        // Check all the data
        if (transaction_method!=MyMoneyTransaction::Deposit || transaction_method!=MyMoneyTransaction::Transfer)
          transaction_method = MyMoneyTransaction::Deposit;
        if (!transaction_date.isValid())
          transaction_date = QDate::currentDate();

        bool foundCategory=false;
        QListIterator<MyMoneyCategory> it = m_filePointer->categoryIterator();
        for ( ; it.current(); ++it) {
          if (it.current()->name()==categoryItem->category().name()) {
            foundCategory=true;
            switch (transaction_method) {
              case MyMoneyTransaction::Deposit:
              case MyMoneyTransaction::Transfer:
                if (!it.current()->isIncome()) {
                  KMessageBox::error(this, i18n("You have specified an expense category for an income.  This has been rectified\nThis will be optional in a later release"));
                  it.current()->setIncome(true);
                }
                break;
              default:
                break;
            }
          }
        }

        MyMoneyBank *pBank;
        MyMoneyAccount *pAccount;

      	pBank = m_filePointer->bank(m_bankIndex);
      	if (!pBank) {
          qDebug("KMyMoneyView::slotInputEnterClicked: Unable to get the current bank");
          return;
        }

        pAccount = pBank->account(m_accountIndex);
        if (!pAccount) {
          qDebug("KMyMoneyView::slotInputEnterClicked: Unable to grab the current account");
          return;
        }

        pAccount->addTransaction(transaction_method,
          transactionsTable->text(row, 2),
          transactionsTable->text(row, 3),
          moneyItem->money(),
          transaction_date,
          transaction_major,
          transaction_minor,
          "",  // future addition
          "",
          "",
          "",
          MyMoneyTransaction::Unreconciled );
      	
        if (!foundCategory)
        	m_filePointer->addCategory(true, transaction_major, transaction_minor);
      	m_filePointer->setDirty(true);
        	
      	emit transactionListChanged();

        updateInputLists();
        updateTransactionList(-1);
      }
      break;
     case 7: // The Withdrawal column
      moneyItem = (KMoneyTableItem*)transactionsTable->item(row, col);
      if (!moneyItem) {
        qDebug("Unable to get money item");
        return;
      }

      if (editing) {
        switch (transaction->method()) {
          case MyMoneyTransaction::Deposit:
            columnSwitched=true;
          default:
            columnSwitched=false;
        }
        if (!columnSwitched && transaction->method()==MyMoneyTransaction::Transfer) {
          if (transaction->accountFrom()==m_accountIndex.name())
            columnSwitched=true;
        }

        if (columnSwitched) {
          QStringList lst;
          bool ok=false;
          lst << "Cheque" << "Withdrawal" << "ATM" << "Transfer";
          QString res = QInputDialog::getItem(i18n("Additional details neeeded"), i18n("Please select an item"), lst, 0, true, &ok, this );
          if (res=="Transfer") {
            qDebug("TODO: Need a dialog to input the bank transfers");
            transaction->setMethod(MyMoneyTransaction::Transfer);
          } else {
            if (res=="Cheque" || res=="Withdrawal") {
              if (res=="Cheque")
                transaction->setMethod(MyMoneyTransaction::Cheque);
              else
                transaction->setMethod(MyMoneyTransaction::Withdrawal);
              done=false;
              while (!done) {
                bool ok = false;
                QString text = QInputDialog::getItem( i18n( "Additional details needed" ), i18n( "Choose the payee" ), options, payeePos, true, &ok, this );
                if ( ok && !text.isEmpty() ) {
                  done=true;
                  transaction->setPayee(text);
                  m_filePointer->addPayee(text);
                }
              }
            }
            else if (res=="ATM") {
              transaction->setMethod(MyMoneyTransaction::ATM);
              transaction->setPayee("Cash");
              m_filePointer->addPayee("Cash");
            }
          }
        }
        updateTransactionList(row);
      } else { // Not editing
        // Get all the data
        methodItem = (KMethodTableItem*)transactionsTable->item(row, 0);
        MyMoneyTransaction::transactionMethod transaction_method = methodItem->method();
        dateItem = (KDateTableItem*)transactionsTable->item(row, 1);
        QDate transaction_date = dateItem->date();
        categoryItem = (KCategoryTableItem*)transactionsTable->item(row, 4);
        QString transaction_major = categoryItem->category().name();
        QString transaction_minor = categoryItem->category().firstMinor();

        // Check all the data
        if (transaction_method!=MyMoneyTransaction::Withdrawal || transaction_method!=MyMoneyTransaction::ATM
              || transaction_method!=MyMoneyTransaction::Cheque || transaction_method!=MyMoneyTransaction::Transfer)
          transaction_method = MyMoneyTransaction::Withdrawal;
        if (!transaction_date.isValid())
          transaction_date = QDate::currentDate();

        bool foundCategory=false;
        QListIterator<MyMoneyCategory> it = m_filePointer->categoryIterator();
        for ( ; it.current(); ++it) {
          if (it.current()->name()==categoryItem->category().name()) {
            foundCategory=true;
            switch (transaction_method) {
              case MyMoneyTransaction::Cheque:
              case MyMoneyTransaction::Withdrawal:
              case MyMoneyTransaction::ATM:
                if (it.current()->isIncome()) {
                  KMessageBox::error(this, i18n("You have specified an income category for an expense.  This has been rectified\nThis will be optional in a later release"));
                  it.current()->setIncome(false);
                }
                break;
              default:
                break;
            }
          }
        }

        MyMoneyBank *pBank;
        MyMoneyAccount *pAccount;

      	pBank = m_filePointer->bank(m_bankIndex);
      	if (!pBank) {
          qDebug("KMyMoneyView::slotInputEnterClicked: Unable to get the current bank");
          return;
        }

        pAccount = pBank->account(m_accountIndex);
        if (!pAccount) {
          qDebug("KMyMoneyView::slotInputEnterClicked: Unable to grab the current account");
          return;
        }

        pAccount->addTransaction(transaction_method,
          transactionsTable->text(row, 2),
          transactionsTable->text(row, 3),
          moneyItem->money(),
          transaction_date,
          transaction_major,
          transaction_minor,
          "",  // future addition
          "",
          "",
          "",
          MyMoneyTransaction::Unreconciled );
      	
        if (!foundCategory)
        	m_filePointer->addCategory(true, transaction_major, transaction_minor);
      	m_filePointer->setDirty(true);
        	
      	emit transactionListChanged();

        updateInputLists();
        updateTransactionList(-1);
      }
      break;
  }
}

void KTransactionView::showInputBox(bool val)
{
  if (val!=m_showingInputBox) {
    if (val)
      tabbedInputBox->show();
    else
      tabbedInputBox->hide();
    m_showingInputBox=val;
    updateTransactionList(-1);
  }
}

void KTransactionView::editClicked()
{
  editMode();
}

void KTransactionView::cancelClicked()
{
  if (m_inEditMode) {
    viewMode();
    return;
  }

  kMyMoneyDateInput* genericDateEdit;
  kMyMoneyEdit* genericAmountEdit;
  QLineEdit* genericNumberEdit;
  QLineEdit* genericMemoEdit;

  QWidget *tab = tabbedInputBox->currentPage();
  if (tab==chequeTab) {
      genericDateEdit = chequeDateEdit;
      genericAmountEdit = chequeAmountEdit;
      genericNumberEdit = chequeNumberEdit;
      genericMemoEdit = chequeMemoEdit;
  }
  else if (tab==depositTab) {
      genericDateEdit = depositDateEdit;
      genericAmountEdit = depositAmountEdit;
      genericNumberEdit = depositNumberEdit;
      genericMemoEdit = depositMemoEdit;
  }
  else if (tab==transferTab) {
      genericDateEdit = transferDateEdit;
      genericAmountEdit = transferAmountEdit;
      genericNumberEdit = transferNumberEdit;
      genericMemoEdit = transferMemoEdit;
  }
  else if (tab==withdrawalTab) {
      genericDateEdit = withdrawalDateEdit;
      genericAmountEdit = withdrawalAmountEdit;
      genericNumberEdit = withdrawalNumberEdit;
      genericMemoEdit = withdrawalMemoEdit;
  }
  else if (tab==atmTab) {
      genericDateEdit = atmDateEdit;
      genericAmountEdit = atmAmountEdit;
      genericNumberEdit = atmNumberEdit;
      genericMemoEdit = atmMemoEdit;
  } else
      return;

  genericDateEdit->setDate(QDate::currentDate());
  genericAmountEdit->setText("");
  genericNumberEdit->setText("");
  genericMemoEdit->setText("");
}

void KTransactionView::newClicked()
{
  QPushButton* genericNewBtn;
  QPushButton* genericEditBtn;
  QPushButton* genericEnterBtn;
  QPushButton* genericCancelBtn;
  kMyMoneyDateInput* genericDateEdit;
  kMyMoneyEdit* genericAmountEdit;
  KComboBox* genericCategoryMajorCombo;
  KComboBox* genericCategoryMinorCombo;
  QLineEdit* genericNumberEdit;
  QLineEdit* genericMemoEdit;
  KComboBox *genericFromCombo = transferFromCombo;
  KComboBox *genericToCombo = transferToCombo;
  KComboBox *genericPayToCombo = chequePayToCombo;

  QWidget *tab = tabbedInputBox->currentPage();
  if (tab==chequeTab) {
      genericNewBtn = chequeNewBtn;
      genericEditBtn = chequeEditBtn;
      genericEnterBtn = chequeEnterBtn;
      genericCancelBtn = chequeCancelBtn;
      genericDateEdit = chequeDateEdit;
      genericPayToCombo = chequePayToCombo;
      genericAmountEdit = chequeAmountEdit;
      genericCategoryMajorCombo = chequeCategoryMajorCombo;
      genericCategoryMinorCombo = chequeCategoryMinorCombo;
      genericNumberEdit = chequeNumberEdit;
      genericMemoEdit = chequeMemoEdit;
  }
  else if (tab==depositTab) {
      genericNewBtn = depositNewBtn;
      genericEditBtn = depositEditBtn;
      genericEnterBtn = depositEnterBtn;
      genericCancelBtn = depositCancelBtn;
      genericDateEdit = depositDateEdit;
      genericFromCombo = depositFromCombo;
      genericAmountEdit = depositAmountEdit;
      genericCategoryMajorCombo = depositCategoryMajorCombo;
      genericCategoryMinorCombo = depositCategoryMinorCombo;
      genericNumberEdit = depositNumberEdit;
      genericMemoEdit = depositMemoEdit;
  }
  else if (tab==transferTab) {
      genericNewBtn = transferNewBtn;
      genericEditBtn = transferEditBtn;
      genericEnterBtn = transferEnterBtn;
      genericCancelBtn = transferCancelBtn;
      genericDateEdit = transferDateEdit;
      genericAmountEdit = transferAmountEdit;
      genericCategoryMajorCombo = transferCategoryMajorCombo;
      genericCategoryMinorCombo = transferCategoryMinorCombo;
      genericNumberEdit = transferNumberEdit;
      genericMemoEdit = transferMemoEdit;
  }
  else if (tab==withdrawalTab) {
      genericNewBtn = withdrawalNewBtn;
      genericEditBtn = withdrawalEditBtn;
      genericEnterBtn = withdrawalEnterBtn;
      genericCancelBtn = withdrawalCancelBtn;
      genericDateEdit = withdrawalDateEdit;
      genericPayToCombo = withdrawalPayToCombo;
      genericAmountEdit = withdrawalAmountEdit;
      genericCategoryMajorCombo = withdrawalCategoryMajorCombo;
      genericCategoryMinorCombo = withdrawalCategoryMinorCombo;
      genericNumberEdit = withdrawalNumberEdit;
      genericMemoEdit = withdrawalMemoEdit;
  }
  else if (tab==atmTab) {
      genericNewBtn = atmNewBtn;
      genericEditBtn = atmEditBtn;
      genericEnterBtn = atmEnterBtn;
      genericCancelBtn = atmCancelBtn;
      genericDateEdit = atmDateEdit;
      genericAmountEdit = atmAmountEdit;
      genericCategoryMajorCombo = atmCategoryMajorCombo;
      genericCategoryMinorCombo = atmCategoryMinorCombo;
      genericNumberEdit = atmNumberEdit;
      genericMemoEdit = atmMemoEdit;
  } else
      return;

  genericNewBtn->setEnabled(false);
  genericEditBtn->setEnabled(false);
  genericEnterBtn->setEnabled(true);
  genericCancelBtn->setEnabled(true);
  genericDateEdit->setEnabled(true);
  genericAmountEdit->setEnabled(true);
  genericCategoryMajorCombo->setEnabled(true);
  genericCategoryMinorCombo->setEnabled(true);
  genericNumberEdit->setEnabled(true);
  genericMemoEdit->setEnabled(true);
  genericFromCombo->setEnabled(true);
  genericToCombo->setEnabled(true);
  genericPayToCombo->setEnabled(true);
  m_inEditMode=false;

  genericDateEdit->setDate(QDate::currentDate());
  genericAmountEdit->setText("");
  genericNumberEdit->setText("");
  genericMemoEdit->setText("");
}

void KTransactionView::refresh(void)
{
  updateTransactionList(-1);
}
