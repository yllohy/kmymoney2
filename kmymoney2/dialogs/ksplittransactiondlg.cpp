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

// ----------------------------------------------------------------------------
// QT Includes

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

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ksplittransactiondlg.h"
#include "../widgets/kmymoneysplittable.h"
#include "../dialogs/ksplitcorrectiondlg.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"

KSplitTransactionDlg::KSplitTransactionDlg(const MyMoneyTransaction& t,
                                           const MyMoneyAccount& acc,
                                           MyMoneyMoney& amount,
                                           const bool amountValid,
                                           const bool deposit,
                                           QWidget* parent, const char* name)
  : kSplitTransactionDlgDecl(parent, name, true),
  m_transaction(t),
  m_account(acc),
  m_amountWidth(80),
  m_amountValid(amountValid),
  m_numExtraLines(0),
  m_editRow(-1),
  m_createdNewSplit(false),
  m_skipStartEdit(false),
  m_isDeposit(deposit)
{
  // setup the transactions table
  transactionsTable->setInlineEditingMode(false);
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

  // for deposits, we invert the sign of all splits.
  // don't forget to revert when we're done ;-)
  if(m_isDeposit) {
    for(unsigned i = 0; i < m_transaction.splits().count(); ++i) {
      MyMoneySplit split = m_transaction.splits()[i];
      split.setValue(-split.value());
      m_transaction.modifySplit(split);
    }
  }

  // setup the focus
  cancelBtn->setFocusPolicy(QWidget::NoFocus);
  finishBtn->setFocusPolicy(QWidget::NoFocus);
  clearAllBtn->setFocusPolicy(QWidget::NoFocus);
  transactionsTable->setFocus();

	KConfig *config = KGlobal::config();
  QFont defaultFont = QFont("helvetica", 12);
  transactionsTable->horizontalHeader()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

  // initialize the display
  updateSplit();

  // setup current selection
  transactionsTable->setCurrentRow(m_transaction.splits().count()-1);

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
  connect(transactionsTable, SIGNAL(signalEsc(void)),
    this, SLOT(slotQuitEdit(void)));

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
}

void KSplitTransactionDlg::createInputWidgets(const int row)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont cellFont = QFont("helvetica", 12);
  cellFont = config->readFontEntry("listCellFont", &cellFont);
  

  // create the widgets
  m_editAmount = new kMyMoneyEdit(0);
  m_editAmount->setFont(cellFont);
  m_editCategory = new kMyMoneyCategory(0);
  m_editCategory->setFont(cellFont);
  m_editMemo = new kMyMoneyLineEdit(0);
  m_editMemo->setFont(cellFont);

  m_editCategory->loadText(transactionsTable->text(row, 0));
  m_editMemo->loadText(transactionsTable->text(row, 1));
  m_editAmount->loadText(transactionsTable->text(row, 2));

  transactionsTable->setCellWidget(row, 0, m_editCategory);
  transactionsTable->setCellWidget(row, 1, m_editMemo);
  transactionsTable->setCellWidget(row, 2, m_editAmount);

  connect(m_editAmount, SIGNAL(signalTab(void)),
    this, SLOT(slotEndEditTab(void)));
  connect(m_editMemo, SIGNAL(signalEnter(void)),
    this, SLOT(slotEndEdit(void)));
  connect(m_editAmount, SIGNAL(signalEnter(void)),
    this, SLOT(slotEndEdit(void)));
  connect(m_editCategory, SIGNAL(signalEnter(void)),
    this, SLOT(slotEndEdit(void)));

  connect(m_editCategory, SIGNAL(signalEsc()),
    this, SLOT(slotQuitEdit()));
  connect(m_editMemo, SIGNAL(signalEsc()),
    this, SLOT(slotQuitEdit()));
  connect(m_editAmount, SIGNAL(signalEsc()),
    this, SLOT(slotQuitEdit()));

  connect(m_editMemo, SIGNAL(lineChanged(const QString&)),
    this, SLOT(slotMemoChanged(const QString&)));
  connect(m_editCategory, SIGNAL(categoryChanged(const QString&)),
    this, SLOT(slotCategoryChanged(const QString&)));
  connect(m_editAmount, SIGNAL(valueChanged(const QString&)),
    this, SLOT(slotAmountChanged(const QString&)));                                                            
}

void KSplitTransactionDlg::destroyInputWidgets(void)
{
  for(int i=0; i < transactionsTable->numRows(); ++i) {
    transactionsTable->clearCellWidget(i, 0);
    transactionsTable->clearCellWidget(i, 1);
    transactionsTable->clearCellWidget(i, 2);
  }
}

KSplitTransactionDlg::~KSplitTransactionDlg()
{
  destroyInputWidgets();
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
  if(transactionsTable->inlineEditingMode()) {         // still in edit mode?
    // we need to end edit mode first
    endEdit(false);
  }

  if(diffAmount() != 0) {
    MyMoneySplit split = m_transaction.split(m_account.id());
    kSplitCorrectionDlgDecl* dlg = new kSplitCorrectionDlgDecl(0, 0, true);
    QString total = (-split.value()).formatMoney();
    QString sums = splitsValue().formatMoney();
    QString diff = diffAmount().formatMoney();

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
            split.setValue(-splitsValue());
            m_transaction.modifySplit(split);
            accept();
            break;
          case 2:       // distribute difference
            qDebug("distribution of difference not yet supported in KSplitTransactionDlg::slotFinishClicked()");
            // accept();
            break;
          case 3:       // leave unassigned
            qDebug("leave unassigned should be changed to assign to standard account");
            // accept();
            break;
        }
      }
    }
    delete dlg;

  } else
    accept();

  if(result() == Accepted) {
    // for deposits, we inverted the sign of all splits.
    // now we revert it back, so that things are left correct
    if(m_isDeposit) {
      for(unsigned i = 0; i < m_transaction.splits().count(); ++i) {
        MyMoneySplit split = m_transaction.splits()[i];
        split.setValue(-split.value());
        m_transaction.modifySplit(split);
      }
    }
  }
}

void KSplitTransactionDlg::slotCancelClicked()
{
  hideWidgets();
  reject();
}

void KSplitTransactionDlg::slotClearAllClicked()
{
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
    QValueList<MyMoneySplit> list = getSplits();
    QValueList<MyMoneySplit>::ConstIterator it;

    // clear all but the one referencing the account
    for(it = list.begin(); it != list.end(); ++it) {
      m_transaction.removeSplit(*it);
    }

    transactionsTable->setCurrentRow(0);
    updateTransactionTableSize();
    updateSplit();
  }
}

void KSplitTransactionDlg::updateSums(void)
{
  MyMoneyMoney splits(splitsValue());
  MyMoneySplit split = m_transaction.split(m_account.id());

  if(m_amountValid == false) {
    split.setValue(splits);
    m_transaction.modifySplit(split);
  }

  splitSum->setText("<b>" + splits.formatMoney() + " ");
  splitUnassigned->setText("<b>" + diffAmount().formatMoney() + " ");
  transactionAmount->setText("<b>" + (-split.value()).formatMoney() + " ");
}

MyMoneyMoney KSplitTransactionDlg::splitsValue(void)
{
  MyMoneyMoney splitsValue(0);
  QValueList<MyMoneySplit> list = getSplits();
  QValueList<MyMoneySplit>::ConstIterator it;

  // calculate the current sum of all split parts
  for(it = list.begin(); it != list.end(); ++it) {
    splitsValue += (*it).value();
  }

  return splitsValue;
}

MyMoneyMoney KSplitTransactionDlg::diffAmount(void)
{
  MyMoneyMoney diff(0);

  // if there is an amount specified in the transaction, we need to calculate the
  // difference, otherwise we display the difference as 0 and display the same sum.
  if(m_amountValid) {
    MyMoneySplit split = m_transaction.split(m_account.id());

    diff = -(splitsValue() + split.value());
  }
  return diff;
}

void KSplitTransactionDlg::updateTransactionTableSize(void)
{
  // get current size of transactions table
  int rowHeight = transactionsTable->cellGeometry(0, 0).height();
  int tableHeight = transactionsTable->height();
  int splitCount = m_transaction.splits().count()-1;

  // see if we need some extra lines to fill the current size with the grid
  m_numExtraLines = (tableHeight / rowHeight) - splitCount;
  if(m_numExtraLines < 0)
    m_numExtraLines = 0;

  transactionsTable->setNumRows(splitCount + m_numExtraLines);
  transactionsTable->setMaxRows(splitCount);
}

const QValueList<MyMoneySplit> KSplitTransactionDlg::getSplits(void) const
{
  QValueList<MyMoneySplit> list;
  QValueList<MyMoneySplit>::Iterator it;

  // get list of splits
  list = m_transaction.splits();

  // and remove the one for this account
  for(it = list.begin(); it != list.end(); ++it) {
    if((*it).accountId() == m_account.id()) {
      list.remove(it);
      break;
    }
  }
  return list;
}

void KSplitTransactionDlg::updateSplit(int row, int col)
{
  unsigned long rowCount=0;

  QValueList<MyMoneySplit> list = getSplits();

  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  transactionsTable->horizontalHeader()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

  if (row==-1) { // We are going to refresh the whole list

    updateTransactionTableSize();
    initAmountWidth();

    // fill the part that is used by transactions
    QValueList<MyMoneySplit>::Iterator it;
    for(it = list.begin(); it != list.end(); ++it) {
      QString colText;
      MyMoneyMoney value = (*it).value();
      if((*it).accountId() != "") {
        try {
          colText = MyMoneyFile::instance()->accountToCategory((*it).accountId());
        } catch(MyMoneyException *e) {
          qDebug("Unexpected exception in KSplitTransactionDlg::updateSplit()");
          delete e;
        }
      }
      QString amountTxt = value.formatMoney();
      if(colText.isEmpty() && (*it).memo().isEmpty() && value == 0)
        amountTxt = "";

      unsigned width = transactionsTable->fontMetrics().width(amountTxt);
      if(width > m_amountWidth)
        m_amountWidth = width;

      transactionsTable->setText(rowCount, 0, colText);
      transactionsTable->setText(rowCount, 1, (*it).memo());
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
    if(row < static_cast<int> (list.count())) {
      MyMoneySplit s = list[row];
      MyMoneyMoney value = s.value();
      QString colText;

      if(s.accountId() != "") {
        try {
          colText = MyMoneyFile::instance()->accountToCategory(s.accountId());
        } catch(MyMoneyException *e) {
          qDebug("Unexpected exception in KSplitTransactionDlg::updateSplit()");
          delete e;
        }
      }
      QString amountTxt = value.formatMoney();
      if(colText.isEmpty() && s.memo().isEmpty() && value == 0)
        amountTxt = "";

      unsigned width = transactionsTable->fontMetrics().width(amountTxt);
      if(width > m_amountWidth)
        m_amountWidth = width;

      transactionsTable->setText(rowCount, 0, colText);
      transactionsTable->setText(rowCount, 1, s.memo());
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

void KSplitTransactionDlg::createSplit(int row)
{
  MyMoneySplit sp;
  m_createdNewSplit = true;
  sp.setValue(diffAmount());
  m_transaction.addSplit(sp);
  updateTransactionTableSize();
  updateSplit(row);
}

void KSplitTransactionDlg::slotDeleteSplitTransaction(void)
{
  slotDeleteSplitTransaction(transactionsTable->currentRow());
}

void KSplitTransactionDlg::slotDeleteSplitTransaction(int row)
{
  int answer;

  if(row < static_cast<int> (m_transaction.splits().count()-1)) {
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
      updateSplit();
    }
  }
}

void KSplitTransactionDlg::deleteSplitTransaction(int row)
{
  QValueList<MyMoneySplit> list = getSplits();
  m_transaction.removeSplit(list[row]);

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
  // only start editing if inside used area
  if(row <= static_cast<int> (m_transaction.splits().count()-1)) {
    // make sure the row will be on the screen
    transactionsTable->ensureCellVisible(row, col);

    if(button == Qt::LeftButton             // left button?
    && !transactionsTable->inlineEditingMode()) {          // and not in edit mode?
      if(row == static_cast<int> (m_transaction.splits().count()-1)) {
        // need a new entry?
        createSplit(row);
      }
      showWidgets(row);
      m_editCategory->setFocus();
    }
  }
}

void KSplitTransactionDlg::endEdit(bool skipNextStart)
{
  // we must move the focus first as this might update some
  // date of the split we just edited
  transactionsTable->setFocus();

  int row = transactionsTable->currentRow() + 1;
  slotFocusChange(row, 0, Qt::LeftButton, QPoint(0, 0));
  m_skipStartEdit = skipNextStart;
}

void KSplitTransactionDlg::slotQuitEdit(void)
{
  int   row = transactionsTable->currentRow();

  if(transactionsTable->inlineEditingMode()) {         // in edit mode?
    if(m_createdNewSplit == true) {
      // it was created when we started editing, so we discard it
      deleteSplitTransaction(m_transaction.splits().count()-1);
      updateSplit();

      if(row > static_cast<int> (m_transaction.splits().count()))
        row = m_transaction.splits().count();
    }
    m_createdNewSplit = false;
    hideWidgets();
  }
  int prev = transactionsTable->currentRow();
  // setup new current row and update visible selection
  transactionsTable->setCurrentRow(row);
  updateSplit(prev);
  updateSplit(row);
}

void KSplitTransactionDlg::slotNavigationKey(int key)
{
  int row = transactionsTable->currentRow();

  switch(key) {
    case Key_Home:
      row = 0;
      break;
    case Key_End:
      row = m_transaction.splits().count()-1;
      break;
    case Key_Up:
      if(row)
        --row;
      break;
    case Key_Down:
      if(row < static_cast<int> (m_transaction.splits().count()-1))
        ++row;
      break;
  }
  slotFocusChange(row, 0, Qt::LeftButton, QPoint(0, 0));
}

void KSplitTransactionDlg::slotFocusChange(int realrow, int col, int button, const QPoint&  point)
{
  int   row = realrow;

  // adjust row to used area
  if(row > static_cast<int> (m_transaction.splits().count()-1))
    row = m_transaction.splits().count()-1;

  // make sure the row will be on the screen
  transactionsTable->ensureCellVisible(row, col);

  if(button == Qt::LeftButton) {          // left mouse button
    if(row != static_cast<int> (transactionsTable->currentRow())) {
      if(transactionsTable->inlineEditingMode()) {         // in edit mode?
        if(!m_editAmount->text().isEmpty()
        || !m_editMemo->text().isEmpty()
        || !m_editCategory->text().isEmpty()) {
          qDebug("split will be updated");
          // there's data in the split -> update it
          updateSplit(transactionsTable->currentRow());

        } else if(m_createdNewSplit == true) {
          qDebug("Newly created split will be destroyed");
          // it's empty and if it was created we discard it
          deleteSplitTransaction(m_transaction.splits().count()-1);
          updateSplit();

          if(row > static_cast<int> (m_transaction.splits().count()-1))
            row = m_transaction.splits().count()-1;
        }
        m_createdNewSplit = false;
        hideWidgets();
      }
      int prev = transactionsTable->currentRow();
      // setup new current row and update visible selection
      transactionsTable->setCurrentRow(row);
      updateSplit(prev);
      updateSplit(row);
    }
  } else if(button == Qt::RightButton) {
    // context menu is only available when cursor is on
    // an existing transaction or the first line after this area
    if(row == realrow) {
      int prev = transactionsTable->currentRow();
      // setup new current row and update visible selection
      transactionsTable->setCurrentRow(row);
      updateSplit(prev);
      updateSplit(row);

      // if the very last entry is selected, the delete
      // operation is not available otherwise it is
      m_contextMenu->setItemEnabled(m_contextMenuDelete,
            row < static_cast<int> (m_transaction.splits().count()-1));

      m_contextMenu->exec(QCursor::pos());
    }
  }
}

/*
void KSplitTransactionDlg::updateTransaction(MyMoneySplitTransaction *transaction)
{

  transaction->setMemo(m_editMemo->text());
  transaction->setAmount(m_editAmount->getMoneyValue());

 	int colonindex = m_editCategory->currentText().find(":");
  if(colonindex == -1) {
    transaction->setCategoryMajor(m_editCategory->currentText());
    transaction->setCategoryMinor("");
	} else {
    int len = m_editCategory->currentText().length();
    len--;
    transaction->setCategoryMajor(m_editCategory->currentText().left(colonindex));
    transaction->setCategoryMinor(m_editCategory->currentText().right(len - colonindex));
	}

// FIXME: Remove if completely empty


// FIXME: add support for transfers

// the following code is just copied from KTransactionView and
// needs to be adapted

  int lessindex = m_editCategory->currentText().find("<");
  int greatindex = m_editCategory->currentText().find(">");
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
    createInputWidgets(row);

    m_editCategory->show();
    m_editMemo->show();
    m_editAmount->show();

    transactionsTable->setInlineEditingMode(true);
    m_editRow = row;

  } else {
    m_editRow = -1;
    transactionsTable->setInlineEditingMode(false);
    destroyInputWidgets();
  }
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
  updateSplit(m_splitList.count()-1);
}
*/

void KSplitTransactionDlg::slotCategoryChanged(const QString& category)
{
  if(!m_editCategory || m_editRow == -1)
    return;

  MyMoneyTransaction t = m_transaction;
  QValueList<MyMoneySplit> list = getSplits();
  MyMoneySplit s = list[m_editRow];

  try {
    // First, we check if the category exists
    QCString id = MyMoneyFile::instance()->categoryToAccount(category);
    if(id == "") {
      // FIXME:
      KMessageBox::sorry(0, i18n("Direct creation of new account not yet implemented"));
      m_editCategory->resetText();
      m_editCategory->setFocus();
      return;
    }

    if(id == m_account.id()) {
      KMessageBox::error(0, i18n("You cannot use this category here"));
      m_editCategory->resetText();
      m_editCategory->setFocus();
      return;
    }

    s.setAccountId(id);

    // Now we check, if a split referencing this account already exists
    // within this transaction. If so, we join them.
    QValueList<MyMoneySplit>::Iterator it;

    for(it = list.begin(); it != list.end(); ++it) {
      // don't check against myself ;-)
      if((*it).id() == s.id())
        continue;

      if((*it).accountId() == s.accountId()) {
        (*it).setValue((*it).value() + s.value());
        m_transaction.modifySplit(*it);
        m_transaction.removeSplit(s);
        hideWidgets();
        updateSplit();
        break;
      }
    }

    if(it == list.end()) {
      m_transaction.modifySplit(s);
      m_editCategory->loadText(category);
    }
/*
    // We have to distinguish between the following cases
    //
    // a) transaction contains just a single split
    // b) transaction contains exactly two splits
    // c) transaction contains more than two splits
    //
    // Here's how we react
    //
    // a) add a split with the account set to the new category
    // b) modify the split and modify the account to the new category
    // c) ask the user that all data about the splits are lost. Upon
    //    positive response remove all splits except the one that
    //    references accountId() then continue with a)

    QValueList<MyMoneySplit> list = m_transaction.splits();
    QValueList<MyMoneySplit>::Iterator it;
    MyMoneySplit split;

    switch(m_transaction.splitCount()) {
      default:
        if(KMessageBox::warningContinueCancel(0,
            i18n("If you press continue, information about all other splits will be lost"),
            i18n("Splitted Transaction")) == KMessageBox::Cancel) {
          m_editCategory->resetText();
          m_editCategory->setFocus();
        }
        for(it = list.begin(); it != list.end(); ++it) {
          if((*it) == m_split)
            continue;
          m_transaction.removeSplit((*it));
        }
        // tricky fall through here
      case 1:
        split.setAccountId(id);
        split.setValue(-m_split.value());
        m_transaction.addSplit(split);
        break;

      case 2:
        // find the 'other' split
        split = m_transaction.split(accountId(), false);
        split.setAccountId(id);
        m_transaction.modifySplit(split);
        break;
    }
*/

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify category"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editCategory->resetText();

    m_transaction = t;
  }
}

void KSplitTransactionDlg::slotAmountChanged(const QString& value)
{
  if(!m_editAmount || m_editRow == -1)
    return;

  MyMoneyTransaction t = m_transaction;
  QValueList<MyMoneySplit> list = getSplits();
  MyMoneySplit s = list[m_editRow];

  try {
    s.setValue(MyMoneyMoney(value));
    m_editAmount->loadText(value);
/*
    switch(transactionType(s)) {
      case Deposit:
        break;
      default:
        // make it negative in case of !deposit
        m_split.setValue(-m_split.value());
        break;
    }
*/
    m_transaction.modifySplit(s);
/* do we need this here?
    if(m_transaction.splitCount() == 2) {
      MyMoneySplit split = m_transaction.split(accountId(), false);
      split.setValue(-m_split.value());
      m_transaction.modifySplit(split);
    }
*/
  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify amount"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editAmount->resetText();
    m_transaction = t;
  }
}

void KSplitTransactionDlg::slotMemoChanged(const QString& memo)
{
  if(!m_editMemo || m_editRow == -1)
    return;

  MyMoneyTransaction t = m_transaction;
  QValueList<MyMoneySplit> list = getSplits();
  MyMoneySplit s = list[m_editRow];

  s.setMemo(memo);

  try {
    m_transaction.modifySplit(s);
    m_editMemo->loadText(memo);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify split"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editMemo->resetText();
    m_transaction = t;
  }
}
