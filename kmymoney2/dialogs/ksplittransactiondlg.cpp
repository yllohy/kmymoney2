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
#include <kmessagebox.h>

#include <qpushbutton.h>
#include <qlabel.h>
#include <qtable.h>
#include <qtimer.h>
#include <qlist.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#if QT_VERSION > 300
#include <qcursor.h>
#endif

#include "ksplittransactiondlg.h"
//#include "../mymoney/mymoneysplittransaction.h"
#include "../widgets/kmymoneysplittable.h"
#include "../mymoney/mymoneycategory.h"
#include "../dialogs/ksplitcorrectiondlg.h"

KSplitTransactionDlg::KSplitTransactionDlg(QWidget* parent,  const char* name,
                                           MyMoneyFile* const filePointer,
                                           MyMoneyInstitution* const bankPointer,
                                           MyMoneyAccount* const accountPointer,
                                           MyMoneyMoney* amount, const bool amountValid) :
  kSplitTransactionDlgDecl(parent, name, true),
  m_filePointer(filePointer),
  m_bankPointer(bankPointer),
  m_accountPointer(accountPointer),
  m_amountTransaction(amount),
  m_amountValid(amountValid),
  m_numExtraLines(0),
  m_createdNewSplit(false),
  m_skipStartEdit(false)
{
/*
  // setup the transactions table
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

  // setup the focus
  cancelBtn->setFocusPolicy(QWidget::NoFocus);
  finishBtn->setFocusPolicy(QWidget::NoFocus);
  clearAllBtn->setFocusPolicy(QWidget::NoFocus);
  transactionsTable->setFocus();

	KConfig *config = KGlobal::config();
  QFont defaultFont = QFont("helvetica", 12);
  transactionsTable->horizontalHeader()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

#if 0
  // copy the split list
  QListIterator<MyMoneySplitTransaction> it(splitList);
  for(; it.current(); ++it) {
    MyMoneySplitTransaction *tmp = new MyMoneySplitTransaction(*it.current());
    m_splitList.append(tmp);
  }
#endif
  m_splitList.setAutoDelete(true);

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
  connect(transactionsTable, SIGNAL(doubleClicked(int, int, int, const QPoint&)),
    this, SLOT(slotStartEdit(int, int, int, const QPoint&)));

  connect(transactionsTable, SIGNAL(signalNavigationKey(int)),
    this, SLOT(slotNavigationKey(int)));
  connect(transactionsTable, SIGNAL(signalTab(void)),
    this, SLOT(slotStartEdit(void)));
  connect(transactionsTable, SIGNAL(signalEscape(void)),
    this, SLOT(slotQuitEdit(void)));

  connect(m_amount, SIGNAL(signalTab(void)),
    this, SLOT(slotEndEditTab(void)));
  connect(m_memo, SIGNAL(signalEnter(void)),
    this, SLOT(slotEndEdit(void)));
  connect(m_amount, SIGNAL(signalEnter(void)),
    this, SLOT(slotEndEdit(void)));

  connect(transactionsTable, SIGNAL(signalDelete(int)),
    this, SLOT(slotDeleteSplitTransaction(int)));

  // setup the context menu
  KIconLoader *kiconloader = KGlobal::iconLoader();
  m_contextMenu = new KPopupMenu(this);
  m_contextMenu->insertTitle(kiconloader->loadIcon("transaction", KIcon::MainToolbar), i18n("Transaction Options"));
  m_contextMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("Edit ..."), this, SLOT(slotStartEdit()));
  m_contextMenuDelete = m_contextMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small),
                        i18n("Delete ..."),
                        this, SLOT(slotDeleteSplitTransaction()));

  // Trick: it seems, that the initial sizing of the dialog does
  // not work correctly. At least, the columns do not get displayed
  // correct. Reason: the return value of transactionsTable->visibleWidth()
  // is incorrect. If the widget is visible, resizing works correctly.
  // So, we let the dialog show up and resize it then. It's not really
  // clean, but the only way I got the damned thing working.
  QTimer::singleShot( 10, this, SLOT(initSize()) );
*/
}

void KSplitTransactionDlg::createInputWidgets(void)
{
  // create the widgets
  m_amount = new kMyMoneyEdit(0);
  m_category = new kMyMoneyCombo(false, 0);
  m_memo = new kMyMoneyLineEdit(0);

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
/*
  if(m_category->isVisible()) {         // still in edit mode?
    // we need to end edit mode first
    endEdit(false);
  }

  if(diffAmount().amount() != 0.0) {
    kSplitCorrectionDlgDecl* dlg = new kSplitCorrectionDlgDecl(0, 0, true);
    QString total = KGlobal::locale()->formatMoney(m_amountTransaction->amount(), "");
    QString sums = KGlobal::locale()->formatMoney(splitsAmount().amount(), "");
    QString diff = KGlobal::locale()->formatMoney(diffAmount().amount(), "");

    // now modify the text items of the dialog to contain the correct values
    QString q = QString(i18n("The total amount of this transaction is %1 while "
                             "the sum of the splits is %2. The remaining %3 are "
                             "unassigned."))
                .arg(total)
                .arg(sums)
                .arg(diff);
    dlg->explanation->setText(q);

    q = QString(i18n("&Change total amount of transaction to %1."))
        .arg(sums);
    dlg->changeBtn->setText(q);

    q = QString(i18n("&Distribute difference of %1 among all splits."))
        .arg(diff);
    dlg->distributeBtn->setText(q);

    q = QString(i18n("&Leave %1 unassigned."))
        .arg(diff);
    dlg->leaveBtn->setText(q);

    connect(dlg->okBtn, SIGNAL(clicked()), dlg, SLOT(accept()));
    connect(dlg->cancelBtn, SIGNAL(clicked()), dlg, SLOT(reject()));

    if(dlg->exec() == QDialog::Accepted) {
      QButton* button = dlg->buttonGroup->selected();
      if(button != NULL) {
        switch(dlg->buttonGroup->id(button)) {
          case 0:       // continue to edit
            break;
          case 1:       // modify total
            m_amountTransaction->setAmount(splitsAmount().amount());
            accept();
            break;
          case 2:       // distribute difference
            qDebug("distribution of difference not yet supported in KSplitTransactionDlg::slotFinishClicked()");
            accept();
            break;
          case 3:       // leave unassigned
            accept();
            break;
        }
      }
    }
    delete dlg;

  } else
*/
    accept();
}

void KSplitTransactionDlg::slotCancelClicked()
{
  reject();
}

void KSplitTransactionDlg::slotClearAllClicked()
{
/*
  int answer;
#if QT_VERSION > 300
  answer = KMessageBox::warningContinueCancel (NULL,
     i18n("You are about to delete all splits of this transaction. "
          "Do you really want to continue?"),
     i18n("KMyMoney2"),
     i18n("Continue")
     );
#else
  answer = KMessageBox::warningContinueCancel (NULL,
     i18n("You are about to delete all splits of this transaction. "
          "Do you really want to continue?"),
     i18n("KMyMoney2"),
     i18n("Continue"),
     false);
#endif
  if(answer == KMessageBox::Continue) {
    while(m_splitList.count()) {
      deleteSplitTransaction(0);
    }
    transactionsTable->setCurrentRow(0);
    updateTransactionTableSize();
    updateTransactionList();
  }
*/
}

void KSplitTransactionDlg::updateSums(void)
{
/*
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
*/
}

MyMoneyMoney KSplitTransactionDlg::splitsAmount(void)
{
/*
  MyMoneySplitTransaction* transaction;
  MyMoneyMoney splits(0.0);

  // calculate the current sum of all split parts
  for (transaction = m_splitList.first(); transaction; transaction = m_splitList.next()) {
    splits += transaction->amount();
  }
  return splits;
*/
}

MyMoneyMoney KSplitTransactionDlg::diffAmount(void)
{
/*
  MyMoneyMoney diff(0.0);
  // if there is an amount specified in the transaction, we need to calculate the
  // difference, otherwise we display the difference as 0 and display the same sum.
  if(m_amountValid)
    diff = *m_amountTransaction - splitsAmount();
  return diff;
*/
}

void KSplitTransactionDlg::updateTransactionTableSize(void)
{
/*
  // get current size of transactions table
  int rowHeight = transactionsTable->cellGeometry(0, 0).height();
  int tableHeight = transactionsTable->height();

  // see if we need some extra lines to fill the current size with the grid
  m_numExtraLines = (tableHeight / rowHeight) - m_splitList.count();
  if(m_numExtraLines < 0)
    m_numExtraLines = 0;

  transactionsTable->setNumRows(m_splitList.count() + m_numExtraLines);
  transactionsTable->setMaxRows(m_splitList.count());
*/
}
/*
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

    // fill the part that is used by transactions
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

    // now clean out the remainder of the table
    while(rowCount < static_cast<unsigned long> (transactionsTable->numRows())) {
      transactionsTable->setText(rowCount, 0, "");
      transactionsTable->setText(rowCount, 1, "");
      transactionsTable->setText(rowCount, 2, "");
      ++rowCount;
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
*/
void KSplitTransactionDlg::createSplitTransaction(int row)
{
/*
  MyMoneySplitTransaction* transaction = new MyMoneySplitTransaction;
  transaction->setAmount(diffAmount());
  m_splitList.append(transaction);

  m_createdNewSplit = true;

  // if the amount is different from zero, we set it in the table
  if(!transaction->amount().isZero()) {
    transactionsTable->setText(row, 2,
       KGlobal::locale()->formatMoney(transaction->amount().amount(), ""));
  }
  updateTransactionTableSize();
*/
}

void KSplitTransactionDlg::slotDeleteSplitTransaction(void)
{
  slotDeleteSplitTransaction(transactionsTable->currentRow());
}

void KSplitTransactionDlg::slotDeleteSplitTransaction(int row)
{
/*
  int answer;

  if(row < static_cast<int> (m_splitList.count())) {
#if QT_VERSION > 300
    answer = KMessageBox::warningContinueCancel (NULL,
       i18n("You are about to delete this part of the transaction. "
            "Do you really want to continue?"),
       i18n("KMyMoney2"),
       i18n("Continue")
       );
#else
    answer = KMessageBox::warningContinueCancel (NULL,
       i18n("You are about to delete this part of the transaction. "
            "Do you really want to continue?"),
       i18n("KMyMoney2"),
       i18n("Continue"),
       false);
#endif
    if(answer == KMessageBox::Continue) {
      deleteSplitTransaction(row);
      updateTransactionList();
    }
  }
*/
}

void KSplitTransactionDlg::deleteSplitTransaction(int row)
{
  //m_splitList.remove(row);

  // FIXME: handle transfers (see updateTransaction() for an example)

  updateTransactionTableSize();
}

void KSplitTransactionDlg::slotStartEdit(void)
{
  if(m_skipStartEdit == true) {
    m_skipStartEdit = false;
    return;
  }

  slotStartEdit(transactionsTable->currentRow(), 0, Qt::LeftButton, QPoint(0, 0));
}

void KSplitTransactionDlg::slotStartEdit(int row, int col, int button, const QPoint&  point)
{
/*
  // only start editing if inside used area
  if(row <= static_cast<int> (m_splitList.count())) {
    // make sure the row will be on the screen
    transactionsTable->ensureCellVisible(row, col);

    if(button == Qt::LeftButton             // left button?
    && !m_category->isVisible()) {          // and not in edit mode?
      if(row == static_cast<int> (m_splitList.count())) {            // need a new entry?
        createSplitTransaction(row);
      }
      showWidgets(row);
      m_category->setFocus();
    }
  }
*/
}

void KSplitTransactionDlg::endEdit(bool skipNextStart)
{
  int row = transactionsTable->currentRow() + 1;
  slotFocusChange(row, 0, Qt::LeftButton, QPoint(0, 0));
  transactionsTable->setFocus();
  m_skipStartEdit = skipNextStart;
}

void KSplitTransactionDlg::slotQuitEdit(void)
{
/*
  int   row = transactionsTable->currentRow();

  if(m_category->isVisible()) {         // in edit mode?
    if(m_createdNewSplit == true) {
      // it was created when we started editing, so we discard it
      deleteSplitTransaction(m_splitList.count()-1);
      updateTransactionList();

      if(row > static_cast<int> (m_splitList.count()))
        row = m_splitList.count();
    }
    m_createdNewSplit = false;
    hideWidgets();
  }
  int prev = transactionsTable->currentRow();
  // setup new current row and update visible selection
  transactionsTable->setCurrentRow(row);
  updateTransactionList(prev);
  updateTransactionList(row);
*/
}

void KSplitTransactionDlg::slotNavigationKey(int key)
{
/*
  int row = transactionsTable->currentRow();

  switch(key) {
    case Key_Home:
      row = 0;
      break;
    case Key_End:
      row = m_splitList.count();
      break;
    case Key_Up:
      if(row)
        --row;
      break;
    case Key_Down:
      if(row < static_cast<int> (m_splitList.count()))
        ++row;
      break;
  }
  slotFocusChange(row, 0, Qt::LeftButton, QPoint(0, 0));
*/
}

void KSplitTransactionDlg::slotFocusChange(int realrow, int col, int button, const QPoint&  point)
{
/*
  int   row = realrow;

  // adjust row to used area
  if(row > static_cast<int> (m_splitList.count()))
    row = m_splitList.count();

  // make sure the row will be on the screen
  transactionsTable->ensureCellVisible(row, col);

  if(button == Qt::LeftButton) {          // left mouse button
    if(row != static_cast<int> (transactionsTable->currentRow())) {
      if(m_category->isVisible()) {         // in edit mode?
        if(!m_amount->text().isEmpty()
        || !m_memo->text().isEmpty()
        || !m_category->text(m_category->currentItem()).isEmpty()) {
          // there's data in the split -> update it
          updateTransaction(m_splitList.at(transactionsTable->currentRow()));

        } else if(m_createdNewSplit == true) {
          // it's empty and if it was created we discard it
          deleteSplitTransaction(m_splitList.count()-1);
          updateTransactionList();

          if(row > static_cast<int> (m_splitList.count()))
            row = m_splitList.count();
        }
        m_createdNewSplit = false;
        hideWidgets();
      }
      int prev = transactionsTable->currentRow();
      // setup new current row and update visible selection
      transactionsTable->setCurrentRow(row);
      updateTransactionList(prev);
      updateTransactionList(row);
    }
  } else if(button == Qt::RightButton) {
    // context menu is only available when cursor is on
    // an existing transaction or the first line after this area
    if(row == realrow) {
      int prev = transactionsTable->currentRow();
      // setup new current row and update visible selection
      transactionsTable->setCurrentRow(row);
      updateTransactionList(prev);
      updateTransactionList(row);

      // if the very last entry is selected, the delete
      // operation is not available otherwise it is
      m_contextMenu->setItemEnabled(m_contextMenuDelete,
            row < static_cast<int> (m_splitList.count()));

      m_contextMenu->exec(QCursor::pos());
    }
  }
*/
}
/*
void KSplitTransactionDlg::updateTransaction(MyMoneySplitTransaction *transaction)
{

  transaction->setMemo(m_memo->text());
  transaction->setAmount(m_amount->getMoneyValue());

 	int colonindex = m_category->currentText().find(":");
  if(colonindex == -1) {
    transaction->setCategoryMajor(m_category->currentText());
    transaction->setCategoryMinor("");
	} else {
    int len = m_category->currentText().length();
    len--;
    transaction->setCategoryMajor(m_category->currentText().left(colonindex));
    transaction->setCategoryMinor(m_category->currentText().right(len - colonindex));
	}

// FIXME: Remove if completely empty


// FIXME: add support for transfers

// the following code is just copied from KTransactionView and
// needs to be adapted

  int lessindex = m_category->currentText().find("<");
  int greatindex = m_category->currentText().find(">");
  QString transferAccount = "";

	if(m_index < static_cast<long> (m_transactions->count())) {
    MyMoneyTransaction *transaction = m_transactions->at(m_index);
		newstate = transaction->state();

	  QDate transdate;
	  MyMoneyMoney transamount;
	  QString transcategory;

	  transdate = transaction->date();
	  transamount = transaction->amount();
	  transcategory = transaction->categoryMajor();

	  QString transferAccount = "";
    if((lessindex != -1) && (greatindex != -1) )
    {
  	  transferAccount =  transcategory;
		  transferAccount = transferAccount.remove(0,1);
		  transferAccount = transferAccount.remove(transferAccount.length() - 1,1);
      MyMoneyAccount *currentAccount;
      for(currentAccount = m_bankIndex.accountFirst(); currentAccount != 0; currentAccount = m_bankIndex.accountNext())
      {
			  if(currentAccount->name() == transferAccount)
        {
				  MyMoneyTransaction *currentTransaction;
				  for(currentTransaction = currentAccount->transactionFirst(); currentTransaction != 0; currentTransaction = currentAccount->transactionNext())
				  {
					  QString matchCategory = "<";
					  matchCategory += account->name();
					  matchCategory += ">";
					  if(currentTransaction->date().toString() == transdate.toString() &&
             currentTransaction->amount().amount() == transamount.amount() &&
						 currentTransaction->categoryMajor() == matchCategory)
					  {
						  currentAccount->removeTransaction(*currentTransaction);
					  }
				  }
			  }
		  }		
	  }
}
*/
void KSplitTransactionDlg::showWidgets(int row, bool show)
{
  if(show == true) {
    updateInputLists();

    transactionsTable->setCellWidget(row, 0, m_category);
    transactionsTable->setCellWidget(row, 1, m_memo);
    transactionsTable->setCellWidget(row, 2, m_amount);

    m_category->setCurrentItem(transactionsTable->text(row, 0));
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
/*
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
  qstringlistIncome.prepend("");
	categoryList = qstringlistIncome;
	
	qstringlistExpense.prepend(i18n("--- Expense ---"));
	categoryList += qstringlistExpense;
	
	qstringlistAccount.prepend(i18n("--- Special ---"));
	categoryList += qstringlistAccount;

  m_category->insertStringList(categoryList);
*/
}

/*
MyMoneySplitTransaction* KSplitTransactionDlg::firstTransaction(void)
{
  //return m_splitList.first();
}

MyMoneySplitTransaction* KSplitTransactionDlg::nextTransaction(void)
{
  //return m_splitList.next();
}

void KSplitTransactionDlg::addTransaction(MyMoneySplitTransaction* const split)
{
  m_splitList.append(split);
  updateTransactionList(m_splitList.count()-1);
}
*/
