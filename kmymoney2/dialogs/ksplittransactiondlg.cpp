/***************************************************************************
                          ksplittransactiondlg.cpp  -  description
                             -------------------
    begin                : Thu Jan 10 2002
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

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>

#include <qpushbutton.h>
#include <qlabel.h>
#include <qtable.h>
#include <qtimer.h>
#include <qlist.h>

#include "ksplittransactiondlg.h"
#include "../mymoney/mymoneysplittransaction.h"
#include "../widgets/kmymoneysplittable.h"
#include "../mymoney/mymoneycategory.h"

KSplitTransactionDlg::KSplitTransactionDlg(QWidget* parent,  const char* name,
                                           MyMoneyFile* const filePointer,
                                           MyMoneyBank* const bankPointer,
                                           MyMoneyAccount* const accountPointer,
                                           const QList<MyMoneySplitTransaction>& splitList,
                                           MyMoneyMoney* amount, const bool amountValid) :
  kSplitTransactionDlgDecl(parent, name, true),
  m_filePointer(filePointer),
  m_bankPointer(bankPointer),
  m_accountPointer(accountPointer),
  m_amountTransaction(amount),
  m_amountValid(amountValid),
  m_numExtraLines(0)
{
  transactionsTable->setNumRows(1);
  transactionsTable->setNumCols(3);
  transactionsTable->horizontalHeader()->setLabel(0, i18n("Category"));
	transactionsTable->horizontalHeader()->setLabel(1, i18n("Memo"));
	transactionsTable->horizontalHeader()->setLabel(2, i18n("Amount"));
	transactionsTable->setSelectionMode(QTable::NoSelection);
 	transactionsTable->setLeftMargin(0);
	transactionsTable->verticalHeader()->hide();
  transactionsTable->setColumnStretchable(0, false);
  transactionsTable->setColumnStretchable(1, false);
	transactionsTable->setColumnStretchable(2, false);
	transactionsTable->horizontalHeader()->setResizeEnabled(false);
	transactionsTable->horizontalHeader()->setMovingEnabled(false);

	KConfig *config = KGlobal::config();
  QFont defaultFont = QFont("helvetica", 12);
  transactionsTable->horizontalHeader()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

  // copy the split list
  QListIterator<MyMoneySplitTransaction> it(splitList);
  for(; it.current(); ++it) {
    MyMoneySplitTransaction *tmp = new MyMoneySplitTransaction(*it.current());
    m_splitList.append(tmp);
  }

  // create required input widgets
  createInputWidgets();

  // initialize the display
  updateTransactionList();

  // setup current selection
  transactionsTable->setCurrentRow(m_splitList.count());

  // connect signals with slots
	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
	connect(finishBtn, SIGNAL(clicked()), this, SLOT(slotFinishClicked()));
  connect(clearAllBtn, SIGNAL(clicked()), this, SLOT(slotClearAllClicked()));
  connect(transactionsTable, SIGNAL(clicked(int, int, int, const QPoint&)),
    this, SLOT(slotFocusChange(int, int, int, const QPoint&)));

  // Trick: it seems, that the initial sizing of the dialog does
  // not work correctly. At least, the columns do not get displayed
  // correct. Reason: the return value of transactionsTable->visibleWidth()
  // is incorrect. If the widget is visible, resizing works correctly.
  // So, we let the dialog show up and resize it then. It's not really
  // clean, but the only way I got the damned thing working.
  QTimer::singleShot( 10, this, SLOT(initSize()) );
}

void KSplitTransactionDlg::createInputWidgets(void)
{
  // create the widgets
  m_amount = new kMyMoneyEdit(0);
  m_category = new kMyMoneyCombo(false, 0);
  m_memo = new kMyMoneyLineEdit(0);

  // customize them
  // m_memo->setFrame(false);
  // m_amount->setFrame(false);

  // don't need to see them right away ;-)
  hideWidgets();
}

KSplitTransactionDlg::~KSplitTransactionDlg()
{
}

void KSplitTransactionDlg::initSize(void)
{
  QDialog::resize(width(), height()+1);
}

void KSplitTransactionDlg::resizeEvent(QResizeEvent* ev)
{
  int w = transactionsTable->visibleWidth() - m_amountWidth;

  // resize the columns
  transactionsTable->setColumnWidth(0, w/2);
  transactionsTable->setColumnWidth(1, w/2);
  transactionsTable->setColumnWidth(2, m_amountWidth);

  updateTransactionTableSize();
}

void KSplitTransactionDlg::initAmountWidth(void)
{
  m_amountWidth = 80;
}

void KSplitTransactionDlg::slotFinishClicked()
{
  accept();
}

void KSplitTransactionDlg::slotCancelClicked()
{
  reject();
}

void KSplitTransactionDlg::slotClearAllClicked()
{
}

void KSplitTransactionDlg::updateSums(void)
{
  MyMoneyMoney splits(splitsAmount());

  if(m_amountTransaction == NULL) {
    qDebug("pointer to transaction amount is NULL in KSplitTransactionDlg::updateSums");
    return;
  }

  if(m_amountValid == false) {
    *m_amountTransaction = splits;
  }

  splitSum->setText("<b>" + KGlobal::locale()->formatMoney(splits.amount(), "") + " ");
  splitUnassigned->setText("<b>" + KGlobal::locale()->formatMoney(diffAmount().amount(), "") + " ");
  transactionAmount->setText("<b>" + KGlobal::locale()->formatMoney(m_amountTransaction->amount(), "") + " ");
}

MyMoneyMoney KSplitTransactionDlg::splitsAmount(void)
{
  MyMoneySplitTransaction* transaction;
  MyMoneyMoney splits(0.0);

  // calculate the current sum of all split parts
  for (transaction = m_splitList.first(); transaction; transaction = m_splitList.next()) {
    splits += transaction->amount();
  }
  return splits;
}

MyMoneyMoney KSplitTransactionDlg::diffAmount(void)
{
  MyMoneyMoney diff(0.0);
  // if there is an amount specified in the transaction, we need to calculate the
  // difference, otherwise we display the difference as 0 and display the same sum.
  if(m_amountValid)
    diff = *m_amountTransaction - splitsAmount();
  return diff;
}

void KSplitTransactionDlg::updateTransactionTableSize(void)
{
  // get current size of transactions table
  int rowHeight = transactionsTable->cellGeometry(0, 0).height();
  int tableHeight = transactionsTable->height();

  // see if we need some extra lines to fill the current size with the grid
  m_numExtraLines = (tableHeight / rowHeight) - m_splitList.count();
  if(m_numExtraLines < 0)
    m_numExtraLines = 0;

  transactionsTable->setNumRows(m_splitList.count() + m_numExtraLines);
  transactionsTable->setMaxRows(m_splitList.count());
}

void KSplitTransactionDlg::updateTransactionList(int row, int col)
{
  unsigned long rowCount=0;
  MyMoneySplitTransaction *transaction;

  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  transactionsTable->horizontalHeader()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

  if (row==-1) { // We are going to refresh the whole list

    updateTransactionTableSize();
    initAmountWidth();

    for (transaction = m_splitList.first(); transaction; transaction = m_splitList.next()) {
      QString colText;
      if(transaction->categoryMinor() == "") {
        colText = transaction->categoryMajor();
      } else {
        colText = transaction->categoryMajor()
                  + ":"
                  + transaction->categoryMinor();
      }

      QString amountTxt = KGlobal::locale()->formatMoney(transaction->amount().amount(), "");
      unsigned width = transactionsTable->fontMetrics().width(amountTxt);
      if(width > m_amountWidth)
        m_amountWidth = width;

      transactionsTable->setText(rowCount, 0, colText);
      transactionsTable->setText(rowCount, 1, transaction->memo());
      transactionsTable->setText(rowCount, 2, amountTxt);

      rowCount++;
    }
  } else {
    rowCount = row;
    if(row < static_cast<int> (m_splitList.count())) {
      transaction = m_splitList.at(row);
      QString colText;
      if(transaction->categoryMinor() == "") {
        colText = transaction->categoryMajor();
      } else {
        colText = transaction->categoryMajor()
                  + ":"
                  + transaction->categoryMinor();
      }

      QString amountTxt = KGlobal::locale()->formatMoney(transaction->amount().amount(), "");
      unsigned width = transactionsTable->fontMetrics().width(amountTxt);
      if(width > m_amountWidth)
        m_amountWidth = width;
      transactionsTable->setText(rowCount, 0, colText);
      transactionsTable->setText(rowCount, 1, transaction->memo());
      transactionsTable->setText(rowCount, 2, amountTxt);

    } else {
      transactionsTable->setText(rowCount, 0, "");
      transactionsTable->setText(rowCount, 1, "");
      transactionsTable->setText(rowCount, 2, "");
    }
  }
  updateSums();

  // setup new size values
  resizeEvent(NULL);
}

void KSplitTransactionDlg::slotFocusChange(int row, int col, int button, const QPoint&  point)
{
  // adjust row to used area
  if(row > static_cast<int> (m_splitList.count()))
    row = m_splitList.count();

  // make sure the row will be on the screen
  transactionsTable->ensureCellVisible(row, col);

  if(button == Qt::LeftButton) {          // left mouse button
    if(row == static_cast<int> (transactionsTable->currentRow())) {
                                                  // need to enter edit mode?
      if(row == static_cast<int> (m_splitList.count())) {            // need a new entry?
        MyMoneySplitTransaction* transaction = new MyMoneySplitTransaction;
        transaction->setAmount(diffAmount());
        m_splitList.append(transaction);

        transactionsTable->setText(row, 2,
             KGlobal::locale()->formatMoney(transaction->amount().amount(), ""));

        updateTransactionTableSize();
      }
      showWidgets(row);
    } else {
      if(m_category->isVisible()) {         // in edit mode?
        updateTransaction(m_splitList.at(transactionsTable->currentRow()));
        hideWidgets();
      }
      int prev = transactionsTable->currentRow();
      // setup new current row and update visible selection
      transactionsTable->setCurrentRow(row);
      updateTransactionList(prev);
      updateTransactionList(row);
    }
  }
}

void KSplitTransactionDlg::updateTransaction(MyMoneySplitTransaction *transaction)
{
  transaction->setMemo(m_memo->text());
  transaction->setAmount(m_amount->getMoneyValue());
}

void KSplitTransactionDlg::showWidgets(int row, bool show)
{
  if(show == true) {
    updateInputLists();

    transactionsTable->setCellWidget(row, 0, m_category);
    transactionsTable->setCellWidget(row, 1, m_memo);
    transactionsTable->setCellWidget(row, 2, m_amount);

    m_memo->setText(transactionsTable->text(row, 1));
    m_amount->setText(transactionsTable->text(row, 2));

    m_category->show();
    m_memo->show();
    m_amount->show();
  } else {
    m_category->hide();
    m_memo->hide();
    m_amount->hide();
  }
}

void KSplitTransactionDlg::updateInputLists(void)
{
  QStringList categoryList;
  QStringList qstringlistIncome;
  QStringList qstringlistExpense;
  QStringList qstringlistAccount;
  bool bDoneInsert = false;

  QString theText;
  if (m_filePointer) {
    QListIterator<MyMoneyCategory> categoryIterator = m_filePointer->categoryIterator();
    for ( ; categoryIterator.current(); ++categoryIterator) {
      MyMoneyCategory *category = categoryIterator.current();

      theText = category->name();

      if (category->isIncome()) {
        // Add it alpabetically
        if (qstringlistIncome.count()<=0)
          qstringlistIncome.append(theText);
        else {
          for (QStringList::Iterator it3 = qstringlistIncome.begin(); it3 != qstringlistIncome.end(); ++it3 ) {
            if ((*it3) >= theText && !bDoneInsert) {
              qstringlistIncome.insert(it3, theText);
              bDoneInsert = true;
            }
          }
          if (!bDoneInsert)
            qstringlistIncome.append(theText);
        }
      } else { // is expense
        // Add it alpabetically
        if (qstringlistExpense.count()<=0)
          qstringlistExpense.append(theText);
        else {
          for (QStringList::Iterator it4 = qstringlistExpense.begin(); it4 != qstringlistExpense.end(); ++it4 ) {
            if ((*it4) >= theText && !bDoneInsert) {
              qstringlistExpense.insert(it4, theText);
              bDoneInsert = true;
            }
          }
          if (!bDoneInsert)
            qstringlistExpense.append(theText);
        }
      }

      // Now add all the minor categories
      for ( QStringList::Iterator it = category->minorCategories().begin(); it != category->minorCategories().end(); ++it ) {
        theText = category->name();
				theText += ":";
				theText += (*it);
				
				bDoneInsert = false;
				
        if (category->isIncome()) {
          // Add it alpabetically
          if (qstringlistIncome.count()<=0)
            qstringlistIncome.append(theText);
          else {
            for (QStringList::Iterator it3 = qstringlistIncome.begin(); it3 != qstringlistIncome.end(); ++it3 ) {
              if ((*it3) >= theText && !bDoneInsert) {
                qstringlistIncome.insert(it3, theText);
                bDoneInsert = true;
              }
            }
            if (!bDoneInsert)
              qstringlistIncome.append(theText);
          }
        } else { // is expense
          // Add it alpabetically
          if (qstringlistExpense.count()<=0)
            qstringlistExpense.append(theText);
          else {
            for (QStringList::Iterator it4 = qstringlistExpense.begin(); it4 != qstringlistExpense.end(); ++it4 ) {
              if ((*it4) >= theText && !bDoneInsert) {
                qstringlistExpense.insert(it4, theText);
                bDoneInsert = true;
              }
            }
            if (!bDoneInsert)
              qstringlistExpense.append(theText);
          }
        }
      }  // End minor iterator
    }

    if (m_bankPointer) {
      MyMoneyAccount *currentAccount;

      for(currentAccount = m_bankPointer->accountFirst(); currentAccount != 0; currentAccount = m_bankPointer->accountNext())
      {
        if(currentAccount != m_accountPointer) {
         	theText = "<";
          theText = theText + currentAccount->name();
          theText = theText + ">";
			    qstringlistAccount.append(theText);
        }
  		}
    }
  }

	m_category->clear();
	
	qstringlistIncome.prepend(i18n("--- Income ---"));
	categoryList = qstringlistIncome;
	
	qstringlistExpense.prepend(i18n("--- Expense ---"));
	categoryList += qstringlistExpense;
	
	qstringlistAccount.prepend(i18n("--- Special ---"));
	categoryList += qstringlistAccount;

  m_category->insertStringList(categoryList);
}
