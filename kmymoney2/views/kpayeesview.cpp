/***************************************************************************
                          kpayeesview.cpp  -  description
                             -------------------
    begin                : Thu Jan 24 2002
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
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>

#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kpayeesview.h"
#include "kbanklistitem.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneyutils.h"

KPayeeListItem::KPayeeListItem(KListView *parent, const MyMoneyPayee& payee) :
  KListViewItem(parent)
{
  m_suspendUpdate = false;
  setPayeeID(payee.id());
  MyMoneyFile::instance()->attach(payee.id(), this);
  setText(0, payee.name());
  // allow in column rename
  setRenameEnabled(0, true);
}

KPayeeListItem::~KPayeeListItem()
{
  MyMoneyFile::instance()->detach(payeeID(), this);
}

void KPayeeListItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
  p->setFont(KMyMoneyUtils::cellFont());

  QColor colour = KMyMoneyUtils::listColour();
  QColor bgColour = KMyMoneyUtils::backgroundColour();

  QColorGroup cg2(cg);

  if (isAlternate())
    cg2.setColor(QColorGroup::Base, colour);
  else
    cg2.setColor(QColorGroup::Base, bgColour);

  QListViewItem::paintCell(p, cg2, column, width, align);
}

void KPayeeListItem::update(const QCString& /* id */)
{
  if(listView()->selectedItem() == this) {
    // since we are the current selected item, we have
    // to unselect ourselves and reselect afterwards
    // in order to force QListView to send out signals.
    // The actual update is performed via
    // KPayeesView::slotSelectPayee(QListViewItem*)
    listView()->setSelected(this, false);
    listView()->setSelected(this, true);
  }
}

KPayeesView::KPayeesView(QWidget *parent, const char *name )
  : KPayeesViewDecl(parent,name),
    m_suspendUpdate(false)
{
  m_transactionView->setSorting(-1);
  m_transactionView->setAllColumnsShowFocus(true);
  m_transactionView->setColumnWidthMode(2, QListView::Manual);
  m_transactionView->setColumnAlignment(3, Qt::AlignRight);
  // never show horizontal scroll bars
  m_transactionView->setHScrollBarMode(QScrollView::AlwaysOff);

  m_payeesList->addColumn(i18n("Name"));
  connect(m_payeesList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectPayee(QListViewItem*)));
  connect(m_payeesList, SIGNAL(itemRenamed(QListViewItem*,int,const QString&)), this, SLOT(slotRenamePayee(QListViewItem*,int,const QString&)));

  connect(addressEdit, SIGNAL(textChanged()), this, SLOT(slotPayeeDataChanged()));
  connect(postcodeEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeDataChanged()));
  connect(telephoneEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeDataChanged()));
  connect(emailEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeDataChanged()));
  connect(m_updateButton, SIGNAL(pressed()), this, SLOT(slotUpdatePayee()));

  // somehow, the rightButtonClicked signal does not make it, we use
  // rightButtonPressed instead to show the context menu
  // connect(accountListView, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
  //   this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));
  connect(m_payeesList, SIGNAL(rightButtonPressed(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));

  connect(m_transactionView, SIGNAL(doubleClicked(QListViewItem*)),
          this, SLOT(slotTransactionDoubleClicked(QListViewItem*)));

  m_updateButton->setEnabled(false);

  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem updateButtenItem( i18n("&Update" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Accepts the entered data and stores it"),
                    i18n("Use this to accept the modified data."));
  m_updateButton->setGuiItem(updateButtenItem);

  m_contextMenu = new KPopupMenu(this);
  m_contextMenu->insertTitle(i18n("Payee Options"));
  m_contextMenu->insertItem(il->loadIcon("edit", KIcon::Small), i18n("New payee..."), this, SLOT(slotAddPayee()));
  m_contextMenu->insertItem(il->loadIcon("delete", KIcon::Small),
                        i18n("Delete payee ..."),
                        this, SLOT(slotDeletePayee()));
  
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassPayeeSet, this);
}

KPayeesView::~KPayeesView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassPayeeSet, this);

  writeConfig();
}

void KPayeesView::update(const QCString & id)
{
  if(m_suspendUpdate == false)
    slotRefreshView();
}

void KPayeesView::suspendUpdate(const bool suspend)
{
  // force a refresh, if update was off
  if(m_suspendUpdate == true
  && suspend == false)
    slotRefreshView();

  m_suspendUpdate = suspend;
}

void KPayeesView::slotRenamePayee(QListViewItem* /* p */, int /* col */, const QString& txt)
{
  m_newName = txt;
  slotPayeeDataChanged();
}

void KPayeesView::slotSelectPayee(QListViewItem *p)
{
  KPayeeListItem* item = static_cast<KPayeeListItem *>(p);

  if(m_updateButton->isEnabled()) {
    if (KMessageBox::questionYesNo(this, i18n("Do you want to discard the changes for '%1'").arg(m_newName), i18n("Discard changes")) == KMessageBox::No) {
      slotUpdatePayee();      
    }
  }
  
  try {
    MyMoneyFile* file = MyMoneyFile::instance();

    m_payee = file->payee(item->payeeID());
    m_newName = m_payee.name();

    addressEdit->setEnabled(true);
    addressEdit->setText(m_payee.address());
    postcodeEdit->setEnabled(true);
    postcodeEdit->setText(m_payee.postcode());
    telephoneEdit->setEnabled(true);
    telephoneEdit->setText(m_payee.telephone());
    emailEdit->setEnabled(true);
    emailEdit->setText(m_payee.email());

    slotPayeeDataChanged();

    showTransactions();
    writeConfig();
    
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
      
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QDateTime defaultDate;
  QDate dateStart = config->readDateTimeEntry("StartDate", &defaultDate).date();

  // setup the list and the pointer vector
  MyMoneyTransactionFilter filter;
  filter.addPayee(m_payee.id());
  filter.setDateFilter(dateStart, QDate());

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
        if(s.value() > 0) {
          txt = i18n("Amortization of %1").arg(acc.name());
        } else {
          txt = i18n("Payment to %1").arg(acc.name());
        }
      } else if(acc.accountType() == MyMoneyAccount::AssetLoan) {
        if(s.value() < 0) {
          txt = i18n("Amortization of %1").arg(acc.name());
        } else {
          txt = i18n("Payment to %1").arg(acc.name());
        }
      } else {
        txt = i18n("Loan payment from %1").arg(acc.name());
      }
    } else if(s.action() == MyMoneySplit::ActionTransfer) {
      if(s.value() >= 0) {
        txt = i18n("Transfer to %1").arg(acc.name());
      } else {
        txt = i18n("Transfer from %1").arg(acc.name());
      }
    } else if(t->splitCount() > 2) {
      txt = i18n("Splitted transaction");
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

void KPayeesView::slotAddPayee()
{
  try {
    MyMoneyPayee p;
    p.setName(i18n("New Payee"));
    MyMoneyFile::instance()->addPayee(p);
    // the callbacks should have made sure, that the payees view has been
    // updated already. So we search for the id in the list of items
    // and select it.
    slotSelectPayeeAndTransaction(p.id(), QCString(), QCString());
  } catch (MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to add payee"),
      (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
  }
}

void KPayeesView::slotPayeeDataChanged(void)
{
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

      MyMoneyFile::instance()->modifyPayee(m_payee);

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to modify payee"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
    }
    m_updateButton->setEnabled(false);
  }
}

void KPayeesView::slotDeletePayee()
{
  KPayeeListItem* item = static_cast<KPayeeListItem*>(m_payeesList->selectedItem());
  if(item) {
    QString prompt(i18n("Do you really want to remove this payee: "));
    prompt += item->text(0);

    if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Payee"))==KMessageBox::No)
      return;

    try {
      MyMoneyFile::instance()->removePayee(m_payee);

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to remove payee"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
    }
  }
}

void KPayeesView::readConfig(void)
{
  QFont font = KMyMoneyUtils::cellFont();
  m_transactionView->setFont(font);

  font = KMyMoneyUtils::headerFont();
  QFontMetrics fm( font );
  int height = fm.lineSpacing()+6;

  m_transactionView->header()->setMinimumHeight(height);
  m_transactionView->header()->setMaximumHeight(height);
  m_transactionView->header()->setFont(font);
}

void KPayeesView::writeConfig(void)
{
/*
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KPayeesView_LastPayee", m_lastPayee);
  config->sync();
*/
}

void KPayeesView::show()
{
  QTimer::singleShot(50, this, SLOT(rearrange()));
  emit signalViewActivated();
  QWidget::show();
}

void KPayeesView::slotReloadView(void)
{
  slotRefreshView();
  rearrange();
}

void KPayeesView::slotRefreshView(void)
{
  QCString id;

  readConfig();

  QValueList<MyMoneyPayee>list = MyMoneyFile::instance()->payeeList();
  QValueList<MyMoneyPayee>::ConstIterator it;

  KPayeeListItem *currentItem = static_cast<KPayeeListItem *>(m_payeesList->selectedItem());
  if(currentItem)
    id = currentItem->payeeID();

  m_payeesList->clear();
  m_transactionView->clear();
  currentItem = 0;

  for (it = list.begin(); it != list.end(); ++it) {
    KPayeeListItem* item = new KPayeeListItem(m_payeesList, *it);
    if(item->payeeID() == id)
      currentItem = item;
  }

  if(currentItem) {
    slotSelectPayee(currentItem);
  }
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
    emit transactionSelected(item->accountID(), item->transactionId());
}

void KPayeesView::slotSelectPayeeAndTransaction(const QCString& payeeId, const QCString& accountId, const QCString& transactionId)
{
  try {
    // find the payee in the list
    QListViewItem* it;
    for(it = m_payeesList->firstChild(); it; it = it->itemBelow()) {
      KPayeeListItem* item = static_cast<KPayeeListItem *>(it);
      if(item->payeeID() == payeeId) {
        if(it->itemAbove())
          m_payeesList->ensureItemVisible(it->itemAbove());
        if(it->itemBelow())
          m_payeesList->ensureItemVisible(it->itemBelow());
        
        m_payeesList->setCurrentItem(it);
        m_payeesList->ensureItemVisible(it);
        
        KTransactionListItem* item = static_cast<KTransactionListItem*> (m_transactionView->firstChild());
        while(item != 0) {
          if(item->accountID() == accountId && item->transactionId() == transactionId)
            break;
          item = static_cast<KTransactionListItem*> (item->nextSibling());
        }
        if(!item) {
          item = static_cast<KTransactionListItem*> (m_transactionView->firstChild());
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

void KPayeesView::slotListRightMouse(QListViewItem* it, const QPoint& /* p */, int /* col */)
{
  // Don't enable delete when no payee is selected
  m_contextMenu->setItemEnabled(m_contextMenu->idAt(2), it != 0);
  m_contextMenu->exec(QCursor::pos());
}

