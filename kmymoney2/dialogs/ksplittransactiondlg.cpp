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
#include <qptrlist.h>
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
#include <kguiitem.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ksplittransactiondlg.h"
#include "../widgets/kmymoneysplittable.h"
#include "../dialogs/ksplitcorrectiondlg.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"
#include "../mymoney/mymoneyfile.h"

KSplitTransactionDlg::KSplitTransactionDlg(const MyMoneyTransaction& t,
                                           const MyMoneyAccount& acc,
                                           const bool amountValid,
                                           const bool deposit,
                                           const MyMoneyMoney& calculatedValue,
                                           QWidget* parent, const char* name)
  : kSplitTransactionDlgDecl(parent, name, true),
  m_transaction(t),
  m_account(acc),
  m_amountWidth(80),
  m_amountValid(amountValid),
  m_numExtraLines(0),
  m_editRow(-1),
  m_isDeposit(deposit),
  m_calculatedValue(calculatedValue)
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

  // add icons to buttons
  KIconLoader *il = KGlobal::iconLoader();
  KGuiItem finishButtenItem( i18n( "&Finish" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Accept splits and return to transaction form"),
                    i18n("Use this to accept all changes to the splits and return to the transaction"));
  finishBtn->setGuiItem(finishButtenItem);

  KGuiItem cancelButtenItem( i18n( "&Cancel" ),
                    QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Reject all changes to the splits and return to transaction form"),
                    i18n("Use this to reject all changes to the splits and return to the transaction"));
  cancelBtn->setGuiItem(cancelButtenItem);

  KGuiItem clearButtenItem( i18n( "Clear &All" ),
                    QIconSet(il->loadIcon("edittrash", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Clear all splits"),
                    i18n("Use this to clear all splits of this transaction"));
  clearAllBtn->setGuiItem(clearButtenItem);


  // make finish the default
  finishBtn->setDefault(true);

  // setup the focus
  cancelBtn->setFocusPolicy(QWidget::NoFocus);
  finishBtn->setFocusPolicy(QWidget::NoFocus);
  clearAllBtn->setFocusPolicy(QWidget::NoFocus);
  transactionsTable->setFocus();

  transactionsTable->horizontalHeader()->setFont(KMyMoneyUtils::headerFont());

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
  connect(transactionsTable, SIGNAL(signalDelete()),
    this, SLOT(slotDeleteSplit()));
  connect(transactionsTable, SIGNAL(signalCancelEdit(int)),
    this, SLOT(slotEndEdit(int)));

  connect(transactionsTable, SIGNAL(signalEsc(void)),
    this, SLOT(slotCancelClicked(void)));
  connect(transactionsTable, SIGNAL(signalEnter()),
    this, SLOT(slotFinishClicked()));

  // setup the context menu
  m_contextMenu = new KPopupMenu(this);
  m_contextMenu->insertTitle(il->loadIcon("transaction", KIcon::MainToolbar), i18n("Transaction Options"));
  m_contextMenu->insertItem(il->loadIcon("edit", KIcon::Small), i18n("Edit ..."), this, SLOT(slotStartEdit()));
  m_contextMenuDelete = m_contextMenu->insertItem(il->loadIcon("delete", KIcon::Small),
                        i18n("Delete ..."),
                        this, SLOT(slotDeleteSplit()));

  // Trick: it seems, that the initial sizing of the dialog does
  // not work correctly. At least, the columns do not get displayed
  // correct. Reason: the return value of transactionsTable->visibleWidth()
  // is incorrect. If the widget is visible, resizing works correctly.
  // So, we let the dialog show up and resize it then. It's not really
  // clean, but the only way I got the damned thing working.
  QTimer::singleShot( 10, this, SLOT(initSize()) );
}

void KSplitTransactionDlg::createInputWidgets(const unsigned row)
{
  QFont cellFont = KMyMoneyUtils::cellFont();

  // create the widgets
  m_editAmount = new kMyMoneyEdit(0);
  m_editAmount->setFont(cellFont);
  m_editCategory = new kMyMoneyCategory(0);
  m_editCategory->setFont(cellFont);
  m_editMemo = new kMyMoneyLineEdit(0, 0, AlignLeft|AlignVCenter);
  m_editMemo->setFont(cellFont);

  if(!m_split.accountId().isEmpty()) {
    m_editCategory->slotSelectAccount(m_split.accountId());
  }
  m_editMemo->loadText(m_split.memo());
  m_editAmount->loadText(m_split.value().formatMoney());
  // don't allow automatically calculated values to be modified
  if(m_split.value() == MyMoneyMoney::minValue+1) {
    m_editAmount->setEnabled(false);
    m_editAmount->loadText("will be calculated");
  }

  transactionsTable->setCellWidget(row, 0, m_editCategory);
  transactionsTable->setCellWidget(row, 1, m_editMemo);
  transactionsTable->setCellWidget(row, 2, m_editAmount);

  connect(m_editAmount, SIGNAL(signalTab(void)),
    this, SLOT(slotEndEdit(void)));
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
  connect(m_editCategory, SIGNAL(categoryChanged(const QCString&)),
    this, SLOT(slotCategoryChanged(const QCString&)));
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

void KSplitTransactionDlg::resizeEvent(QResizeEvent* /* ev */)
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
    slotEndEdit();
  }

  if(diffAmount() != 0) {
    MyMoneySplit split = m_transaction.splitByAccount(m_account.id());
    kSplitCorrectionDlgDecl* dlg = new kSplitCorrectionDlgDecl(0, 0, true);

    // add icons to buttons
    KIconLoader *il = KGlobal::iconLoader();
    KGuiItem okButtenItem( i18n("&Ok" ),
                      QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Accepts the selected action and continues"),
                      i18n("Use this to accept the action and perform it"));
    dlg->okBtn->setGuiItem(okButtenItem);

    KGuiItem cancelButtenItem( i18n( "&Cancel" ),
                      QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Return to split transaction dialog"),
                      i18n("Use this to continue editing the splits"));
    dlg->cancelBtn->setGuiItem(cancelButtenItem);

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
            accept();
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
  answer = KMessageBox::warningContinueCancel (NULL,
     i18n("You are about to delete all splits of this transaction. "
          "Do you really want to continue?"),
     i18n("KMyMoney"),
     i18n("Continue")
     );

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
  MyMoneySplit split = m_transaction.splitByAccount(m_account.id());

  if(m_amountValid == false) {
    split.setValue(-splits);
    m_transaction.modifySplit(split);
  }

  splitSum->setText("<b>" + splits.formatMoney() + " ");
  splitUnassigned->setText("<b>" + diffAmount().formatMoney() + " ");
  transactionAmount->setText("<b>" + (-split.value()).formatMoney() + " ");
}

MyMoneyMoney KSplitTransactionDlg::splitsValue(void)
{
  MyMoneyMoney splitsValue(m_calculatedValue);
  QValueList<MyMoneySplit> list = getSplits();
  QValueList<MyMoneySplit>::ConstIterator it;

  // calculate the current sum of all split parts
  for(it = list.begin(); it != list.end(); ++it) {
    if((*it).value() != MyMoneyMoney::minValue+1)
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
    MyMoneySplit split = m_transaction.splitByAccount(m_account.id());

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
  if(m_numExtraLines < 1)
    m_numExtraLines = 1;

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

void KSplitTransactionDlg::updateSplit(int row, int /* col */)
{
  unsigned long rowCount=0;

  QValueList<MyMoneySplit> list = getSplits();

  transactionsTable->horizontalHeader()->setFont(KMyMoneyUtils::headerFont());

  if (row==-1) { // We are going to refresh the whole list

    updateTransactionTableSize();
    initAmountWidth();

    // fill the part that is used by transactions
    QValueList<MyMoneySplit>::Iterator it;
    for(it = list.begin(); it != list.end(); ++it) {
      QString colText;
      MyMoneyMoney value = (*it).value();
      if(!(*it).accountId().isEmpty()) {
        try {
          colText = MyMoneyFile::instance()->accountToCategory((*it).accountId());
        } catch(MyMoneyException *e) {
          qDebug("Unexpected exception in KSplitTransactionDlg::updateSplit()");
          delete e;
        }
      }
      QString amountTxt = value.formatMoney();
      if(value == MyMoneyMoney::minValue+1) {
        amountTxt = i18n("will be calculated");
      }

      if(colText.isEmpty() && (*it).memo().isEmpty() && value == 0)
        amountTxt = QString();

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

      if(!s.accountId().isEmpty()) {
        try {
          colText = MyMoneyFile::instance()->accountToCategory(s.accountId());
        } catch(MyMoneyException *e) {
          qDebug("Unexpected exception in KSplitTransactionDlg::updateSplit()");
          delete e;
        }
      }
      QString amountTxt = value.formatMoney();
      if(value == MyMoneyMoney::minValue+1) {
        amountTxt = i18n("will be calculated");
      }
      if(colText.isEmpty() && s.memo().isEmpty() && value == 0)
        amountTxt = QString();

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

void KSplitTransactionDlg::slotDeleteSplit(void)
{
  slotDeleteSplit(transactionsTable->currentRow());
}

void KSplitTransactionDlg::slotDeleteSplit(int row)
{
  int answer;

  if(row < static_cast<int> (m_transaction.splits().count()-1)) {
    answer = KMessageBox::warningContinueCancel (NULL,
       i18n("You are about to delete this part of the transaction. "
            "Do you really want to continue?"),
       i18n("KMyMoney"),
       i18n("Continue")
       );
    if(answer == KMessageBox::Continue) {
      deleteSplit(row);
      updateSplit();
    }
  }
}

void KSplitTransactionDlg::deleteSplit(int row)
{
  QValueList<MyMoneySplit> list = getSplits();
  m_transaction.removeSplit(list[row]);

  updateTransactionTableSize();
}

void KSplitTransactionDlg::slotStartEdit(void)
{
  slotStartEdit(transactionsTable->currentRow(), 0, Qt::LeftButton, QPoint(0, 0));
}

void KSplitTransactionDlg::slotStartEdit(int row, int col, int button, const QPoint&  /* point */)
{
  // only start editing if inside used area
  if(row <= static_cast<int> (m_transaction.splits().count()-1)) {
    // make sure the row will be on the screen
    transactionsTable->ensureCellVisible(row, col);

    if(button == Qt::LeftButton             // left button?
    && !transactionsTable->inlineEditingMode()) {          // and not in edit mode?
      if(row == static_cast<int> (m_transaction.splits().count()-1)) {
        // need a new split. preset it with the value to match the difference
        MyMoneySplit sp;
        sp.setValue(diffAmount());
        m_split = sp;
      } else
        m_split = getSplits()[row];

      showWidgets(row);
      // if it's a loan payment transfer, we don't allow to modify the category ;-)
      if(m_split.action() == MyMoneySplit::ActionAmortization
      || m_split.action() == MyMoneySplit::ActionInterest) {
        m_editCategory->setEnabled(false);
        m_editMemo->setFocus();
      } else {
        m_editCategory->setFocus();
      }
    }
  }
}

void KSplitTransactionDlg::slotEndEdit(int key)
{
  // we must move the focus first as this might update some
  // date of the split we just edited
  transactionsTable->setFocus();

  int row;
  switch(key) {
    case Qt::Key_Up:
      row = transactionsTable->currentRow() - 1;
      break;

    case Qt::Key_Down:
    default:
      row = transactionsTable->currentRow() + 1;
      break;
  }

  slotFocusChange(row, 0, Qt::LeftButton, QPoint(0, 0));
}

void KSplitTransactionDlg::slotQuitEdit(void)
{
  int   row = transactionsTable->currentRow();

  if(transactionsTable->inlineEditingMode()) {         // in edit mode?
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

void KSplitTransactionDlg::slotFocusChange(int realrow, int col, int button, const QPoint& /* point */)
{
  int   row = realrow;

  // adjust row to used area
  if(row > static_cast<int> (m_transaction.splits().count()-1))
    row = m_transaction.splits().count()-1;

  // make sure the row will be on the screen
  transactionsTable->ensureCellVisible(row, col);

  if(button == Qt::LeftButton) {          // left mouse button
    if(transactionsTable->inlineEditingMode()) {         // in edit mode?
      if(!m_editAmount->text().isEmpty()
      && !m_editCategory->text().isEmpty()) {
        // there's data in the split -> update it
        try {
          if(!m_split.id().isEmpty()) {
            m_transaction.modifySplit(m_split);
          } else {
            m_transaction.addSplit(m_split);
            updateTransactionTableSize();
          }
        } catch (MyMoneyException *e) {
          KMessageBox::detailedSorry(0, i18n("Unable to add/modify split"),
              (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
          delete e;
        }
        updateSplit(transactionsTable->currentRow());
      }
      hideWidgets();
    }
    if(row != static_cast<int> (transactionsTable->currentRow())) {
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
    destroyInputWidgets();
    transactionsTable->setInlineEditingMode(false);
  }
}


void KSplitTransactionDlg::slotCategoryChanged(const QCString& id)
{
  if(!m_editCategory || m_editRow == -1)
    return;

  MyMoneySplit s = m_split;

  try {
    // First, we check if the category exists

    if(id == m_account.id()) {
      KMessageBox::error(this, i18n("You cannot use this account here"));
      // m_editCategory->resetText();
      m_editCategory->setFocus();
      return;
    }

    m_split.setAccountId(id);
  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify category"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    // m_editCategory->resetText();

    m_split = s;
  }
}

void KSplitTransactionDlg::slotAmountChanged(const QString& value)
{
  if(!m_editAmount || m_editRow == -1)
    return;

  MyMoneySplit s = m_split;

  try {
    m_split.setValue(MyMoneyMoney(value));
    m_editAmount->loadText(value);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify amount"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editAmount->resetText();
    m_split = s;
  }
}

void KSplitTransactionDlg::slotMemoChanged(const QString& memo)
{
  if(!m_editMemo || m_editRow == -1)
    return;

  m_split.setMemo(memo);
  m_editMemo->loadText(memo);
}
