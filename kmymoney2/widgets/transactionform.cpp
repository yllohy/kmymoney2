/***************************************************************************
                             transactionform.cpp
                             -------------------
    begin                : Sun May 14 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include <qstring.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qlayout.h>
// FIXME remove tabbar
// #include <qtabbar.h>
#include <qpalette.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kcombobox.h>
// #include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyobjectcontainer.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneycategory.h>

#include "transactionform.h"
#include "../kmymoneyutils.h"
#include "../kmymoneyglobalsettings.h"

using namespace KMyMoneyTransactionForm;

TransactionForm::TransactionForm(QWidget *parent, const char *name) :
  TransactionEditorContainer(parent, name),
// FIXME remove tabbar
  // m_tabBar(0),
  m_transaction(0)
{
  setBackgroundOrigin(QTable::WindowOrigin);
  setFrameShape( QTable::NoFrame);
  setShowGrid( false );
  setSelectionMode( QTable::NoSelection );
  verticalHeader()->hide();
  horizontalHeader()->hide();
  setLeftMargin(0);
  setTopMargin(0);
  setReadOnly(true);    // display only

  // make sure, that the table is 'invisible' by setting up the right background
  // keep the original color group for painting the cells though
  QPalette p = palette();
  QColorGroup cg = p.active();
  m_cellColorGroup = cg;
  cg.setBrush(QColorGroup::Base, cg.brush(QColorGroup::Background));
  p.setActive(cg);
  p.setInactive(cg);
  p.setDisabled(cg);
  setPalette(p);

  // determine the height of the objects in the table
  kMyMoneyDateInput dateInput(0, "editDate");
  // FIXME make sure the category has the split button activated
  //kMyMoneyCategory category(0, "category");
  KComboBox category(0, "category");

  // extract the maximal sizeHint height
  m_rowHeight = QMAX(dateInput.sizeHint().height(), category.sizeHint().height());

  // never show vertical scroll bars
  setVScrollBarMode(QScrollView::AlwaysOff);

  slotSetTransaction(0);
}

bool TransactionForm::focusNextPrevChild(bool next)
{
  return QFrame::focusNextPrevChild(next);
}

void TransactionForm::clear(void)
{
  slotSetTransaction(0);
}

void TransactionForm::slotSetTransaction(KMyMoneyRegister::Transaction* transaction)
{
  m_transaction = transaction;

  if(!KMyMoneySettings::transactionForm())
    return;

  setUpdatesEnabled(false);

  if(m_transaction) {
    // the next call sets up a back pointer to the form and also sets up the col and row span
    // as well as the tab of the form
    m_transaction->setupForm(this);

  } else {
    setNumRows(5);
    setNumCols(1);
  }

  for(int row = 0; row < numRows(); ++row)
    QTable::setRowHeight(row, m_rowHeight-4);  // adjust the max by 4 pixels

  // adjust vertical size of form table
  int height = (m_rowHeight-4) * numRows();
  setMaximumHeight(height);
  setMinimumHeight(height);

  setUpdatesEnabled(true);

  // force resizeing of the columns
  QTimer::singleShot(0, this, SLOT(resize()));
}

void TransactionForm::paintCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& /* cg */)
{
  if(m_transaction) {
    m_transaction->paintFormCell(painter, row, col, r, selected, m_cellColorGroup);
  }
}

// FIXME remove tabbar
#if 0
QTabBar* TransactionForm::tabBar(QWidget* parent)
{
  if(!m_tabBar) {
    m_tabBar = new QTabBar( parent );
    m_tabBar->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, m_tabBar->sizePolicy().hasHeightForWidth() ) );

    QTab* t = new QTab(i18n("&Check"));
    t->setIdentifier(KMyMoneyRegister::ActionCheck);
    m_tabBar->addTab(t);
    t = new QTab(i18n("&Deposit"));
    t->setIdentifier(KMyMoneyRegister::ActionDeposit);
    m_tabBar->addTab(t);
    t = new QTab(i18n("&Transfer"));
    t->setIdentifier(KMyMoneyRegister::ActionTransfer);
    m_tabBar->addTab(t);
    t = new QTab(i18n("&Witdrawal"));
    t->setIdentifier(KMyMoneyRegister::ActionWithdrawal);
    m_tabBar->addTab(t);
    t = new QTab(i18n("AT&M"));
    t->setIdentifier(KMyMoneyRegister::ActionAtm);
    m_tabBar->addTab(t);

  }
  return m_tabBar;
}

int TransactionForm::action(QMap<QString, QWidget*>& /* editWidgets */) const
{
  return m_tabBar->currentTab();
}

void TransactionForm::setProtectedAction(QMap<QString, QWidget*>& /* editWidgets */, ProtectedAction action)
{
  for(int i = 0; i < KMyMoneyRegister::MaxAction; ++i) {
    m_tabBar->tab(i)->setEnabled(true);
    if(action == ProtectAll
    || (action == ProtectTransfer && i == KMyMoneyRegister::ActionTransfer)
    || (action == ProtectNonTransfer && i != KMyMoneyRegister::ActionTransfer))
      m_tabBar->tab(i)->setEnabled(false);
  }

  // force repaint, unfortunately, setEnabled() does not do that for us
  m_tabBar->update();
}
#endif

void TransactionForm::resize(void)
{
  resize(ValueColumn1);
}

void TransactionForm::resize(int col)
{
  setUpdatesEnabled(false);

  // resize the register
  int w = visibleWidth();

  // check which space we need
  if(columnWidth(LabelColumn1))
    adjustColumn(LabelColumn1);
  if(columnWidth(LabelColumn2))
    adjustColumn(LabelColumn2);
  if(columnWidth(ValueColumn2))
    adjustColumn(ValueColumn2);

  for(int i = 0; i < numCols(); ++i) {
    if(i == col)
      continue;

    w -= columnWidth(i);
  }
  setColumnWidth(col, w);

  setUpdatesEnabled(true);
  repaintContents(false);
}

// needed to duplicate this here, as the QTable::tableSize method is private :-(
QSize TransactionForm::tableSize(void) const
{
  return QSize(columnPos(numCols()-1) + columnWidth(numCols()-1) + 10,
               rowPos(numRows()-1) + rowHeight(numRows()-1) + 10);
}

QSize TransactionForm::sizeHint(void) const
{
  // I've taken this from qtable.cpp, QTable::sizeHint()
  int vmargin = QApplication::reverseLayout() ? rightMargin() : leftMargin();
  return QSize(tableSize().width() + vmargin + 5, tableSize().height() + topMargin() + 10);
}

void TransactionForm::adjustColumn(Column col)
{
  int w = 0;

  // preset the width of the right value column with the width of
  // the possible edit widgets so that they fit if they pop up
  if(col == ValueColumn2) {
    kMyMoneyDateInput dateInput;
    kMyMoneyEdit valInput;
    w = QMAX(dateInput.sizeHint().width(), valInput.sizeHint().width());
  }

  if(m_transaction) {
    QString txt;
    QFontMetrics fontMetrics(KMyMoneyGlobalSettings::listCellFont());

    // scan through the rows
    for ( int i = numRows()-1; i >= 0; --i ) {
      int align;
      m_transaction->formCellText(txt, align, i, static_cast<int>(col));
      QWidget* cw = cellWidget(i, col);
      if(cw) {
        w = QMAX(w, cw->sizeHint().width()+10);
      }
      w = QMAX(w, fontMetrics.width(txt)+10);
    }
  }
  setColumnWidth( col, w );
}

void TransactionForm::arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, KMyMoneyRegister::Transaction* t)
{
  t->arrangeWidgetsInForm(editWidgets);
  resize(ValueColumn1);
}

void TransactionForm::tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const
{
  t->tabOrderInForm(tabOrderWidgets);
  // FIXME remove tabbar
  // tabOrderWidgets.append(m_tabBar);
}

void TransactionForm::removeEditWidgets(void)
{
  for(int row = 0; row < numRows(); ++row) {
    for(int col = 0; col < numCols(); ++col) {
      if(cellWidget(row, col))
        clearCellWidget(row, col);
    }
  }
  resize(ValueColumn1);
}

#include "transactionform.moc"
