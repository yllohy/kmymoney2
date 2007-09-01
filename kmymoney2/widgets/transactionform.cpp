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
#include <qtabbar.h>
#include <qpalette.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneycategory.h>

#include "transactionform.h"
#include "../kmymoneyutils.h"
#include "../kmymoneyglobalsettings.h"

using namespace KMyMoneyTransactionForm;

TabBar::TabBar(QWidget* parent, const char* name) :
  QTabBar(parent, name),
  m_signalType(SignalNormal)
{
  connect(this, SIGNAL(selected(int)), this, SLOT(slotTabSelected(int)));
}

TabBar::SignalEmissionE TabBar::setSignalEmission(TabBar::SignalEmissionE type)
{
  TabBar::SignalEmissionE _type = m_signalType;
  m_signalType = type;
  return _type;
}

int TabBar::currentTab(void) const
{
  QMap<int, int>::const_iterator it;
  it = m_idMap.find(QTabBar::currentTab());
  if(it != m_idMap.end())
    return *it;
  return -1;
}

void TabBar::setCurrentTab(int id)
{
  QMap<int, int>::const_iterator it;
  for(it = m_idMap.begin(); it != m_idMap.end(); ++it) {
    if(*it == id) {
      QTabBar::setCurrentTab(it.key());
    }
  }
}

QTab* TabBar::tab(int id) const
{
  QMap<int, int>::const_iterator it;
  for(it = m_idMap.begin(); it != m_idMap.end(); ++it) {
    if(*it == id) {
      return QTabBar::tab(it.key());
    }
  }
  return 0;
}

void TabBar::setCurrentTab(QTab* tab)
{
  if(m_signalType != SignalNormal)
    blockSignals(true);

  QTabBar::setCurrentTab(tab);

  if(m_signalType != SignalNormal)
    blockSignals(false);

  if(m_signalType == SignalAlways)
    emit selected(tab->identifier());
}

void TabBar::addTab(QTab* tab, int id)
{
  QTabBar::addTab(tab);
  setIdentifier(tab, id);
}

void TabBar::setIdentifier(QTab* tab, int newId)
{
  m_idMap[tab->identifier()] = newId;
}

void TabBar::slotTabSelected(int id)
{
  QMap<int, int>::const_iterator it;
  it = m_idMap.find(id);
  if(it != m_idMap.end())
    emit tabSelected(*it);
  else
    emit tabSelected(id);
}

void TabBar::show(void)
{
  // make sure we don't emit a signal when simply showing the widget
  if(m_signalType != SignalNormal)
    blockSignals(true);

  QTabBar::show();

  if(m_signalType != SignalNormal)
    blockSignals(false);
}

void TabBar::copyTabs(const TabBar* otabbar)
{
  // remove all existing tabs
  while(count()) {
    removeTab(tabAt(0));
  }
  // now create new ones. copy text, icon and identifier
  for(int i=0; i < otabbar->count(); ++i) {
    QTab* otab = otabbar->tabAt(i);
    QTab* ntab = new QTab(otab->text());
    int nid = QTabBar::addTab(ntab);
    m_idMap[nid] = otabbar->m_idMap[otab->identifier()];
    ntab->setEnabled(otab->isEnabled());
    if(otab->identifier() == otabbar->currentTab())
      setCurrentTab(ntab);
  }
}

TransactionForm::TransactionForm(QWidget *parent, const char *name) :
  TransactionEditorContainer(parent, name),
  m_transaction(0),
  m_tabBar(0)
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

  // never show vertical scroll bars
  setVScrollBarMode(QScrollView::AlwaysOff);

  slotSetTransaction(0);
}

void TransactionForm::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
  // the QTable::drawContents() method does not honor the block update flag
  // so we take care of it here
  if ( testWState(WState_Visible|WState_BlockUpdates) != WState_Visible )
    return;

  QTable::drawContents(p, cx, cy, cw, ch);
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

  bool enabled = isUpdatesEnabled();
  setUpdatesEnabled(false);

  if(m_transaction) {
    // the next call sets up a back pointer to the form and also sets up the col and row span
    // as well as the tab of the form
    m_transaction->setupForm(this);

  } else {
    setNumRows(5);
    setNumCols(1);
  }

  kMyMoneyDateInput dateInput(0, "editDate");
  KMyMoneyCategory category(0, "category", true);

  // extract the maximal sizeHint height
  int height = QMAX(dateInput.sizeHint().height(), category.sizeHint().height());

  for(int row = 0; row < numRows(); ++row) {
    if(!transaction || transaction->showRowInForm(row)) {
      showRow(row);
      QTable::setRowHeight(row, height);
    } else
      hideRow(row);
  }

  // adjust vertical size of form table
  height *= numRows();
  setMaximumHeight(height);
  setMinimumHeight(height);

  setUpdatesEnabled(enabled);

  // force resizeing of the columns
  QTimer::singleShot(0, this, SLOT(resize()));
}

void TransactionForm::paintCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& /* cg */)
{
  if(m_transaction) {
    m_transaction->paintFormCell(painter, row, col, r, selected, m_cellColorGroup);
  }
}

TabBar* TransactionForm::tabBar(QWidget* parent)
{
  if(!m_tabBar && parent) {
    // determine the height of the objects in the table
    // create the tab bar
    m_tabBar = new TabBar( parent );
    m_tabBar->setSignalEmission(TabBar::SignalAlways);
    m_tabBar->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, m_tabBar->sizePolicy().hasHeightForWidth() ) );
    connect(m_tabBar, SIGNAL(tabSelected(int)), this, SLOT(slotActionSelected(int)));
  }
  return m_tabBar;
}

void TransactionForm::slotActionSelected(int id)
{
  emit newTransaction(static_cast<KMyMoneyRegister::Action>(id));
}

void TransactionForm::setupForm(const MyMoneyAccount& acc)
{
  // remove all tabs from the tabbar
  QTab* tab;
  for(tab = m_tabBar->tabAt(0); tab; tab = m_tabBar->tabAt(0)) {
    m_tabBar->removeTab(tab);
  }

  m_tabBar->show();

  // important: one needs to add the new tabs first and then
  // change the identifier. Otherwise, addTab() will assign
  // a different value
  switch(acc.accountType()) {
    default:
      tab = new QTab(i18n("&Deposit"));
      m_tabBar->addTab(tab, KMyMoneyRegister::ActionDeposit);
      tab = new QTab(i18n("&Transfer"));
      m_tabBar->addTab(tab, KMyMoneyRegister::ActionTransfer);
      tab = new QTab(i18n("&Withdrawal"));
      m_tabBar->addTab(tab, KMyMoneyRegister::ActionWithdrawal);
      break;

    case MyMoneyAccount::CreditCard:
      tab = new QTab(i18n("&Payment"));
      m_tabBar->addTab(tab, KMyMoneyRegister::ActionDeposit);
      tab = new QTab(i18n("&Transfer"));
      m_tabBar->addTab(tab, KMyMoneyRegister::ActionTransfer);
      tab = new QTab(i18n("&Charge"));
      m_tabBar->addTab(tab, KMyMoneyRegister::ActionWithdrawal);
      break;

    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Loan:
      tab = new QTab(i18n("&Decrease"));
      m_tabBar->addTab(tab, KMyMoneyRegister::ActionDeposit);
      tab = new QTab(i18n("&Transfer"));
      m_tabBar->addTab(tab, KMyMoneyRegister::ActionTransfer);
      tab = new QTab(i18n("&Increase"));
      m_tabBar->addTab(tab, KMyMoneyRegister::ActionWithdrawal);
      break;

    case MyMoneyAccount::Asset:
    case MyMoneyAccount::AssetLoan:
      tab = new QTab(i18n("&Increase"));
      m_tabBar->addTab(tab, KMyMoneyRegister::ActionDeposit);
      tab = new QTab(i18n("&Transfer"));
      m_tabBar->addTab(tab, KMyMoneyRegister::ActionTransfer);
      tab = new QTab(i18n("&Decrease"));
      m_tabBar->addTab(tab, KMyMoneyRegister::ActionWithdrawal);
      break;

    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
    case MyMoneyAccount::Investment:
    case MyMoneyAccount::Stock:
      m_tabBar->hide();
      break;
  }
}

void TransactionForm::resize(void)
{
  resize(ValueColumn1);
}

void TransactionForm::resize(int col)
{
  bool enabled = isUpdatesEnabled();
  setUpdatesEnabled(false);

  // resize the register
  int w = visibleWidth();
  int nc = numCols();

  // check which space we need
  if(nc >= LabelColumn1 && columnWidth(LabelColumn1))
    adjustColumn(LabelColumn1);
  if(nc >= LabelColumn2 && columnWidth(LabelColumn2))
    adjustColumn(LabelColumn2);
  if(nc >= ValueColumn2 && columnWidth(ValueColumn2))
    adjustColumn(ValueColumn2);

  for(int i = 0; i < nc; ++i) {
    if(i == col)
      continue;

    w -= columnWidth(i);
  }
  if(col < nc && w >= 0)
    setColumnWidth(col, w);

  setUpdatesEnabled(enabled);
  updateContents();
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

#ifndef KMM_DESIGNER
  if(m_transaction) {
    QString txt;
    QFontMetrics fontMetrics(KMyMoneyGlobalSettings::listCellFont());

    // scan through the rows
    for ( int i = numRows()-1; i >= 0; --i ) {
      int align;
      m_transaction->formCellText(txt, align, i, static_cast<int>(col), 0);
      QWidget* cw = cellWidget(i, col);
      if(cw) {
        w = QMAX(w, cw->sizeHint().width()+10);
      }
      w = QMAX(w, fontMetrics.width(txt)+10);
    }
  }
#endif

  if(col < numCols())
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
}

void TransactionForm::removeEditWidgets(QMap<QString, QWidget*>& editWidgets)
{
  QMap<QString, QWidget*>::iterator it;
  for(it = editWidgets.begin(); it != editWidgets.end(); ) {
    if((*it)->parentWidget() == this) {
      editWidgets.remove(it);
      it = editWidgets.begin();
    } else
      ++it;
  }

  for(int row = 0; row < numRows(); ++row) {
    for(int col = 0; col < numCols(); ++col) {
      if(cellWidget(row, col))
        clearCellWidget(row, col);
    }
  }
  resize(ValueColumn1);

  // delete all remaining edit widgets   (e.g. tabbar)
  for(it = editWidgets.begin(); it != editWidgets.end(); ) {
    delete (*it); // ->deleteLater();
    editWidgets.remove(it);
    it = editWidgets.begin();
  }
}

#include "transactionform.moc"
