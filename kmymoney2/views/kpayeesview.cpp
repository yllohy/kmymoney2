/***************************************************************************
                          kpayeesview.cpp
                          ---------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Andreas Nicolai <Andreas.Nicolai@gmx.net>
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
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qpixmap.h>
#include <qtabwidget.h>
#include <qcursor.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kapplication.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneyaccounttree.h>

#include "kpayeesview.h"
#include "../kmymoneysettings.h"

// *** KPayeeListItem Implementation ***

KPayeeListItem::KPayeeListItem(KListView *parent, const MyMoneyPayee& payee) :
  KListViewItem(parent),
  m_payee(payee)
{
  setText(0, payee.name());
  // allow in column rename
  setRenameEnabled(0, true);
}

KPayeeListItem::~KPayeeListItem()
{
}

void KPayeeListItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
  p->setFont(KMyMoneySettings::listCellFont());

  QColor colour = KMyMoneySettings::listColor();
  QColor bgColour = KMyMoneySettings::listBGColor();

  QColorGroup cg2(cg);

  if (isAlternate())
    cg2.setColor(QColorGroup::Base, colour);
  else
    cg2.setColor(QColorGroup::Base, bgColour);

  QListViewItem::paintCell(p, cg2, column, width, align);
}

KTransactionListItem::KTransactionListItem(KListView* view, KTransactionListItem* parent, const QCString& accountId, const QCString& transactionId) :
  KListViewItem(view, parent)
{
  m_accountId = accountId;
  m_transactionId = transactionId;
}

KTransactionListItem::~KTransactionListItem()
{
}

void KTransactionListItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
  _cg.setColor(QColorGroup::Base, backgroundColor());
  QListViewItem::paintCell(p, _cg, column, width, alignment);
}

const QColor KTransactionListItem::backgroundColor()
{
  return isAlternate() ? KMyMoneySettings::listBGColor() : KMyMoneySettings::listColor();
}




// *** KPayeesView Implementation ***

KPayeesView::KPayeesView(QWidget *parent, const char *name ) :
  KPayeesViewDecl(parent,name),
  m_needReload(false)
{
  m_transactionView->setSorting(-1);
  m_transactionView->setAllColumnsShowFocus(true);
  m_transactionView->setColumnWidthMode(2, QListView::Manual);
  m_transactionView->setColumnAlignment(3, Qt::AlignRight);
  // never show horizontal scroll bars
  m_transactionView->setHScrollBarMode(QScrollView::AlwaysOff);

  m_payeesList->addColumn(i18n("Name"));

  m_updateButton->setEnabled(false);

  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem updateButtenItem( i18n("&Update" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Accepts the entered data and stores it"),
                    i18n("Use this to accept the modified data."));
  m_updateButton->setGuiItem(updateButtenItem);


  connect(m_payeesList, SIGNAL(selectionChanged()), this, SLOT(slotSelectPayee()));
  connect(m_payeesList, SIGNAL(itemRenamed(QListViewItem*,int,const QString&)), this, SLOT(slotRenamePayee(QListViewItem*,int,const QString&)));

  connect(addressEdit, SIGNAL(textChanged()), this, SLOT(slotPayeeDataChanged()));
  connect(postcodeEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeDataChanged()));
  connect(telephoneEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeDataChanged()));
  connect(emailEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeDataChanged()));

  connect(radioNoMatch, SIGNAL(toggled(bool)), this, SLOT(slotPayeeDataChanged()));
  connect(radioNameMatch, SIGNAL(toggled(bool)), this, SLOT(slotPayeeDataChanged()));
  connect(radioKeyMatch, SIGNAL(toggled(bool)), this, SLOT(slotPayeeDataChanged()));
  connect(checkMatchIgnoreCase, SIGNAL(toggled(bool)), this, SLOT(slotPayeeDataChanged()));
  
  connect(m_updateButton, SIGNAL(pressed()), this, SLOT(slotUpdatePayee()));
  connect(m_helpButton, SIGNAL(pressed()), this, SLOT(slotHelp()));
  
  connect(m_payeesList, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotOpenContextMenu(QListViewItem*)));

  connect(m_transactionView, SIGNAL(doubleClicked(QListViewItem*)),
          this, SLOT(slotTransactionDoubleClicked(QListViewItem*)));

  connect(m_tabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(rearrange(void)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadPayees()));
}

KPayeesView::~KPayeesView()
{
}

void KPayeesView::slotStartRename(void)
{
  QListViewItemIterator it_l(m_payeesList, QListViewItemIterator::Selected);
  QListViewItem* it_v;
  if((it_v = it_l.current()) != 0) {
    it_v->startRename(0);
  }
}

// This variant is only called when a single payee is selected and renamed.
void KPayeesView::slotRenamePayee(QListViewItem* p , int /* col */, const QString& txt)
{
  //kdDebug() << "[KPayeesView::slotRenamePayee]" << endl;
  // create a copy of the new name without appended whitespaces
  QString new_name = txt.stripWhiteSpace();
  if (m_payee.name() != new_name) {
    try {
      // check if we already have a payee with the new name
      try {
        // this function call will throw an exception, if the payee
        // hasn't been found.
        MyMoneyFile::instance()->payeeByName(new_name);
        // the name already exists, ask the user whether he's sure to keep the name
        if (KMessageBox::questionYesNo(this,
          i18n("A payee with the name '%1' already exists. It is not advisable to have "
            "multiple payees with the same identification name. Are you sure you would like "
            "to rename the payee?").arg(new_name)) != KMessageBox::Yes)
        {
          p->setText(0,m_payee.name());
          return;
        }
      } catch(MyMoneyException *e) {
        // all ok, the name is unique
        delete e;
      }

      m_payee.setName(new_name);
      m_newName = new_name;
      MyMoneyFile::instance()->modifyPayee(m_payee);

      // the above call to modifyPayee will reload the view so
      // all references and pointers to the view have to be
      // re-established.

      // make sure, that the record is visible even if it moved
      // out of sight due to the rename operation
      ensurePayeeVisible(m_payee.id());


    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to modify payee"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
    }
  }
  else {
    p->setText(0, new_name);
  }
}

void KPayeesView::ensurePayeeVisible(const QCString& id)
{
  for (QListViewItem * item = m_payeesList->firstChild(); item; item = item->itemBelow()) {
    KPayeeListItem* p = dynamic_cast<KPayeeListItem*>(item);
    if(p && p->payee().id() == id) {
      if(p->itemAbove())
        m_payeesList->ensureItemVisible(p->itemAbove());
      if(p->itemBelow())
        m_payeesList->ensureItemVisible(p->itemBelow());

      m_payeesList->setCurrentItem(p);      // active item and deselect all others
      m_payeesList->setSelected(p, true);   // and select it
      m_payeesList->ensureItemVisible(p);
      break;
    }
  }
}

void KPayeesView::selectedPayees(QValueList<MyMoneyPayee>& payeesList) const
{
  QListViewItemIterator it_l(m_payeesList, QListViewItemIterator::Selected);
  QListViewItem* it_v;
  while((it_v = it_l.current()) != 0) {
    KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(it_v);
    if(item)
      payeesList << item->payee();
    ++it_l;
  }
}

void KPayeesView::slotSelectPayee()
{
  // check if the content of a currently selected payee was modified
  // and ask to store the data
  if (m_updateButton->isEnabled()) {
    if (KMessageBox::questionYesNo(this,
          i18n("Do you want to discard the changes for '%1'").arg(m_newName),
          i18n("Discard changes")) == KMessageBox::No) {
      slotUpdatePayee();
    }
  }

  // loop over all payees and count the number of payees, also
  // optain last selected payee
  QValueList<MyMoneyPayee> payeesList;
  selectedPayees(payeesList);

  emit selectObjects(payeesList);

  if (payeesList.count() == 0) return; // make sure we don't access an undefined payee

  // if we have multiple payees selected, clear and disable the payee informations
  if (payeesList.count() > 1) {
    m_tabWidget->setCurrentPage(0); // activate transaction view
    m_transactionView->clear();     // clear transaction view
    m_tabWidget->setEnabled(false); // disable tab widget
    // disable renaming in all listviewitem
    for (QListViewItem * i = m_payeesList->firstChild(); i; i = i->itemBelow())
      i->setRenameEnabled(0, false);
    return;
  }
  // otherwise we have just one selected, enable payee information widget
  m_tabWidget->setEnabled(true);
  // enable renaming in all listviewitem
  for (QListViewItem * i = m_payeesList->firstChild(); i; i = i->itemBelow())
    i->setRenameEnabled(0, true);

  // as of now we are updating only the last selected payee, and until
  // selection mode of the QListView has been changed to Extended, this
  // will also be the only selection and behave exactly as before - Andreas
  try {
    m_payee = payeesList[0];
    m_newName = m_payee.name();

    addressEdit->setEnabled(true);
    addressEdit->setText(m_payee.address());
    postcodeEdit->setEnabled(true);
    postcodeEdit->setText(m_payee.postcode());
    telephoneEdit->setEnabled(true);
    telephoneEdit->setText(m_payee.telephone());
    emailEdit->setEnabled(true);
    emailEdit->setText(m_payee.email());

    QString key;
    bool ignorecase = true;
    bool enabled = m_payee.matchData(key,ignorecase);
    bool namematch = (enabled && (key == m_payee.name()));

    radioNoMatch->setEnabled(true);
    radioNoMatch->setChecked(!enabled);
    radioNameMatch->setEnabled(true);
    radioNameMatch->setChecked(namematch);
    radioKeyMatch->setEnabled(true);
    radioKeyMatch->setChecked(enabled && !namematch);
    matchKeyEdit->setEnabled(true);
    matchKeyEdit->setText(key);
    checkMatchIgnoreCase->setEnabled(true);
    checkMatchIgnoreCase->setChecked(ignorecase);
    
    slotPayeeDataChanged();

    showTransactions();

  } catch(MyMoneyException *e) {
    qDebug("exception during display of payee: %s at %s:%ld", e->what().latin1(), e->file().latin1(), e->line());
    m_transactionView->clear();
    m_payee = MyMoneyPayee();
    delete e;
  }
}

void KPayeesView::showTransactions(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyMoney balance(0);
  unsigned int   i;

  // clear the current transaction listview
  m_transactionView->clear();

  if(m_payee.id().isEmpty()) {
    m_balanceLabel->setText(i18n("Balance: %1").arg(balance.formatMoney()));
    return;
  }

  // setup the list and the pointer vector
  MyMoneyTransactionFilter filter;
  filter.addPayee(m_payee.id());
  filter.setDateFilter(KMyMoneySettings::startDate().date(), QDate());

  QValueList<MyMoneyTransaction> list = file->transactionList(filter);
  m_transactionList.clear();

  m_transactionPtrVector.clear();
  m_transactionPtrVector.resize(list.size());
  m_transactionPtrVector.setPayeeId(m_payee.id());
  m_transactionPtrVector.setSortType(KTransactionPtrVector::SortPostDate);

  QValueList<MyMoneyTransaction>::ConstIterator it_t;
  QCString lastId;
  int ofs = 0;

  for(i = 0, it_t = list.begin(); it_t != list.end(); ++it_t) {
    KMyMoneyTransaction k(*it_t);

    filter.match(*it_t, MyMoneyFile::instance()->storage());
    if(lastId != (*it_t).id()) {
      ofs = 0;
      lastId = (*it_t).id();
    } else
      ofs++;

    k.setSplitId(filter.matchingSplits()[ofs].id());
    MyMoneyAccount acc = MyMoneyFile::instance()->account(filter.matchingSplits()[ofs].accountId());
    if(acc.accountGroup() == MyMoneyAccount::Asset
    || acc.accountGroup() == MyMoneyAccount::Liability) {
      QValueList<KMyMoneyTransaction>::ConstIterator it_k;
      it_k = m_transactionList.append(k);
      balance += k.splitById(k.splitId()).value();
      m_transactionPtrVector.insert(i, &(*it_k));
      ++i;
    }
  }
  m_transactionPtrVector.resize(i);

  // sort the transactions
  m_transactionPtrVector.sort();

  // and fill the m_transactionView
  KTransactionListItem *item = 0;

  for(i = 0; i < m_transactionPtrVector.size(); ++i) {
    KMyMoneyTransaction* t = m_transactionPtrVector[i];
    MyMoneySplit s = t->splitById(t->splitId());
    MyMoneyAccount acc = file->account(s.accountId());

    item = new KTransactionListItem(m_transactionView, item, s.accountId(), t->id());
    item->setText(0, s.number());
    item->setText(1, KGlobal::locale()->formatDate(t->postDate(), true));

    QString txt;
    if(s.action() == MyMoneySplit::ActionAmortization) {
      if(acc.accountType() == MyMoneyAccount::Loan) {
        if(s.value().isPositive()) {
          txt = i18n("Amortization of %1").arg(acc.name());
        } else {
          txt = i18n("Payment to %1").arg(acc.name());
        }
      } else if(acc.accountType() == MyMoneyAccount::AssetLoan) {
        if(s.value().isNegative()) {
          txt = i18n("Amortization of %1").arg(acc.name());
        } else {
          txt = i18n("Payment to %1").arg(acc.name());
        }
      } else {
        txt = i18n("Loan payment from %1").arg(acc.name());
      }
    } else if(s.action() == MyMoneySplit::ActionTransfer) {
      if(!s.value().isNegative()) {
        txt = i18n("Transfer to %1").arg(acc.name());
      } else {
        txt = i18n("Transfer from %1").arg(acc.name());
      }
    } else if(t->splitCount() > 2) {
      txt = i18n("Split transaction (category replacement)", "Split transaction");
    } else if(t->splitCount() == 2) {
      MyMoneySplit s0 = t->splitByAccount(s.accountId(), false);
      txt = MyMoneyFile::instance()->accountToCategory(s0.accountId());
    }
    item->setText(2, txt);
    item->setText(3, s.value().formatMoney());
  }
  m_balanceLabel->setText(i18n("Balance: %1").arg(balance.formatMoney()));

  // Trick: it seems, that the initial sizing of the view does
  // not work correctly. At least, the columns do not get displayed
  // correct. Reason: the return value of m_transactionView->visibleWidth()
  // is incorrect. If the widget is visible, resizing works correctly.
  // So, we let the dialog show up and resize it then. It's not really
  // clean, but the only way I got the damned thing working.
  QTimer::singleShot(50, this, SLOT(rearrange()));
}

void KPayeesView::slotPayeeDataChanged(void)
{
  kdDebug(2) << "KPayeesView::slotPayeeDataChanged(void)" << endl;

  bool rc = false;

  rc |= ((m_payee.email().isEmpty() != emailEdit->text().isEmpty())
      || (!emailEdit->text().isEmpty() && m_payee.email() != emailEdit->text()));
  rc |= ((m_payee.address().isEmpty() != addressEdit->text().isEmpty())
      || (!addressEdit->text().isEmpty() && m_payee.address() != addressEdit->text()));
  rc |= ((m_payee.postcode().isEmpty() != postcodeEdit->text().isEmpty())
      || (!postcodeEdit->text().isEmpty() && m_payee.postcode() != postcodeEdit->text()));
  rc |= ((m_payee.telephone().isEmpty() != telephoneEdit->text().isEmpty())
      || (!telephoneEdit->text().isEmpty() && m_payee.telephone() != telephoneEdit->text()));
  rc |= ((m_payee.name().isEmpty() != m_newName.isEmpty())
      || (!m_newName.isEmpty() && m_payee.name() != m_newName));

  bool ignorecase = true;
  QString key;
  bool enabled = m_payee.matchData(key,ignorecase);
  int newradiostate = radioNameMatch->isChecked()?1:0 + radioKeyMatch->isChecked()?2:0;
  int oldradiostate = (enabled?1:0) + ((enabled && (key != m_payee.name()))?1:0);

  kdDebug(2) << "enabled=" << enabled << " key=" << key << " m_payee.name()==" << m_payee.name() << endl;
  kdDebug(2) << "radiostates: old=" << oldradiostate << " new=" << newradiostate << endl;
  
  rc |= (newradiostate != oldradiostate);

  if ( enabled )
  {
    rc |= (ignorecase != checkMatchIgnoreCase->isChecked());
    rc |= (key != m_payee.name() && key != matchKeyEdit->text());
  }

  m_updateButton->setEnabled(rc);
}

void KPayeesView::slotUpdatePayee()
{
  if(m_updateButton->isEnabled()) {
    try {
      m_payee.setName(m_newName);
      m_payee.setAddress(addressEdit->text());
      m_payee.setPostcode(postcodeEdit->text());
      m_payee.setTelephone(telephoneEdit->text());
      m_payee.setEmail(emailEdit->text());
      
      m_payee.setMatchData(
          !radioNoMatch->isChecked(),
          radioKeyMatch->isChecked(),
          checkMatchIgnoreCase->isChecked(),
          matchKeyEdit->text()
      );
      
      MyMoneyFile::instance()->modifyPayee(m_payee);

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to modify payee"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
    }
    m_updateButton->setEnabled(false);
  }
}

void KPayeesView::readConfig(void)
{
  m_transactionView->setFont(KMyMoneySettings::listCellFont());

  QFontMetrics fm( KMyMoneySettings::listHeaderFont() );
  int height = fm.lineSpacing()+6;

  m_transactionView->header()->setMinimumHeight(height);
  m_transactionView->header()->setMaximumHeight(height);
  m_transactionView->header()->setFont(KMyMoneySettings::listHeaderFont());

  m_payeesList->setDefaultRenameAction(
           KMyMoneySettings::focusChangeIsEnter() ? QListView::Accept : QListView::Reject);
}

void KPayeesView::show()
{
  if(m_needReload) {
    loadPayees();
    m_needReload = false;
  }

  // fixup the layout
  QTimer::singleShot(0, this, SLOT(rearrange()));

  // don't forget base class implementation
  KPayeesViewDecl::show();

  QValueList<MyMoneyPayee> list;
  selectedPayees(list);
  emit selectObjects(list);
}

void KPayeesView::slotLoadPayees(void)
{
  if(isVisible()) {
    loadPayees();
  } else {
    m_needReload = true;
  }
}

void KPayeesView::loadPayees(void)
{
  QMap<QCString, bool> isSelected;
  QCString id;

  ::timetrace("Start KPayeesView::loadPayees");
  readConfig();

  // remember which items are selected in the list
  QListViewItemIterator it_l(m_payeesList, QListViewItemIterator::Selected);
  QListViewItem* it_v;
  while((it_v = it_l.current()) != 0) {
    KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(it_v);
    if(item)
      isSelected[item->payee().id()] = true;
    ++it_l;
  }

  // keep current selected item
  KPayeeListItem *currentItem = static_cast<KPayeeListItem *>(m_payeesList->currentItem());
  if(currentItem)
    id = currentItem->payee().id();

  // remember the upper left corner of the viewport
  QPoint startPoint = m_payeesList->viewportToContents(QPoint(0, 0));

  // turn off updates to avoid flickering during reload
  m_payeesList->setUpdatesEnabled(false);

  // clear the list
  m_payeesList->clear();
  m_transactionView->clear();
  currentItem = 0;

  QValueList<MyMoneyPayee>list = MyMoneyFile::instance()->payeeList();
  QValueList<MyMoneyPayee>::ConstIterator it;

  for (it = list.begin(); it != list.end(); ++it) {
    KPayeeListItem* item = new KPayeeListItem(m_payeesList, *it);
    if(item->payee().id() == id)
      currentItem = item;
    if(isSelected[item->payee().id()])
      item->setSelected(true);
  }

  if (currentItem) {
    m_payeesList->setCurrentItem(currentItem);
  }

  // reposition viewport
  m_payeesList->setContentsPos(startPoint.x(), startPoint.y());

  // turn updates back on
  m_payeesList->setUpdatesEnabled(true);
  m_payeesList->repaintContents();

  ::timetrace("End KPayeesView::loadPayees");
}

void KPayeesView::rearrange(void)
{
  resizeEvent(0);
}

void KPayeesView::resizeEvent(QResizeEvent* ev)
{
  // resize the register
  int w = m_transactionView->visibleWidth();
  w -= m_transactionView->columnWidth(0);
  w -= m_transactionView->columnWidth(1);
  w -= m_transactionView->columnWidth(3);
  m_transactionView->setColumnWidth(2, w);
  m_transactionView->resizeContents(
    m_transactionView->visibleWidth(),
    m_transactionView->contentsHeight());

  m_payeesList->setColumnWidth(0, m_payeesList->visibleWidth());
  KPayeesViewDecl::resizeEvent(ev);
}

void KPayeesView::slotTransactionDoubleClicked(QListViewItem* i)
{
  KTransactionListItem* item = static_cast<KTransactionListItem *>(i);
  if (item)
    emit transactionSelected(item->accountId(), item->transactionId());
}

void KPayeesView::slotSelectPayeeAndTransaction(const QCString& payeeId, const QCString& accountId, const QCString& transactionId)
{
  if(!isVisible())
    return;

  try {
    // deselect all other selected items
    QListViewItemIterator it_l(m_payeesList, QListViewItemIterator::Selected);
    QListViewItem* it_v;
    while((it_v = it_l.current()) != 0) {
      KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(it_v);
      if(item)
        item->setSelected(false);
      ++it_l;
    }

    // find the payee in the list
    QListViewItem* it;
    for(it = m_payeesList->firstChild(); it; it = it->itemBelow()) {
      KPayeeListItem* item = dynamic_cast<KPayeeListItem *>(it);
      if(item && item->payee().id() == payeeId) {
        if(it->itemAbove())
          m_payeesList->ensureItemVisible(it->itemAbove());
        if(it->itemBelow())
          m_payeesList->ensureItemVisible(it->itemBelow());

        m_payeesList->setCurrentItem(it);     // active item and deselect all others
        m_payeesList->setSelected(it,true);   // and select it
        m_payeesList->ensureItemVisible(it);

        KTransactionListItem* item = dynamic_cast<KTransactionListItem*> (m_transactionView->firstChild());
        while(item != 0) {
          if(item->accountId() == accountId && item->transactionId() == transactionId)
            break;
          item = dynamic_cast<KTransactionListItem*> (item->nextSibling());
        }
        if(!item) {
          item = dynamic_cast<KTransactionListItem*> (m_transactionView->firstChild());
        }
        if(item) {
          m_transactionView->setSelected(item, true);
          m_transactionView->ensureItemVisible(item);
        }
        // quit out of for() loop
        break;
      }
    }

  } catch(MyMoneyException *e) {
    qWarning("Unexpected exception in KPayeesView::slotSelectPayeeAndTransaction");
    delete e;
  }
}

void KPayeesView::slotOpenContextMenu(QListViewItem* i)
{
  KPayeeListItem* item = dynamic_cast<KPayeeListItem*>(i);
  if(item) {
    emit openContextMenu(item->payee());
  }
}

void KPayeesView::slotHelp()
{
  kapp->invokeHelp("details.payees.personalinformation");
}

#include "kpayeesview.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
