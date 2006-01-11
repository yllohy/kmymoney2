/***************************************************************************
                          kscheduledview.cpp  -  description
                             -------------------
    begin                : Sun Jan 27 2002
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

#include <qheader.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qtimer.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <klistview.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kscheduledview.h"
#include "kscheduledlistitem.h"
#include "../widgets/kmymoneyscheduleddatetbl.h"
#include "../dialogs/kenterscheduledialog.h"
#include "../kmymoneyutils.h"
#include "../kmymoneysettings.h"

#include "../kmymoney2.h"

KScheduledView::KScheduledView(QWidget *parent, const char *name )
 : kScheduledViewDecl(parent,name, false),
 m_openBills(true),
 m_openDeposits(true),
 m_openTransfers(true),
 m_openLoans(true)
{
  m_qlistviewScheduled->setRootIsDecorated(true);
  m_qlistviewScheduled->addColumn(i18n("Type/Name"));
  m_qlistviewScheduled->addColumn(i18n("Account"));
  m_qlistviewScheduled->addColumn(i18n("Payee"));
  m_qlistviewScheduled->addColumn(i18n("Amount"));
  m_qlistviewScheduled->addColumn(i18n("Next Due Date"));
  m_qlistviewScheduled->addColumn(i18n("Frequency"));
  m_qlistviewScheduled->addColumn(i18n("Payment Method"));
  m_qlistviewScheduled->setMultiSelection(false);
  m_qlistviewScheduled->header()->setResizeEnabled(false);
  m_qlistviewScheduled->setAllColumnsShowFocus(true);
  // never show a horizontal scroll bar
  m_qlistviewScheduled->setHScrollBarMode(QScrollView::AlwaysOff);
  m_qlistviewScheduled->setSorting(-1);
  m_qlistviewScheduled->setColumnAlignment(3, Qt::AlignRight);

  // attach popup to 'New schedule ...' button
  QWidget* w = kmymoney2->factory()->container("schedule_create_menu", kmymoney2);
  QPopupMenu *menu = dynamic_cast<QPopupMenu*>(w);
  if(menu)
    m_qbuttonNew->setPopup(menu);

  // attach popup to 'Filter...' button
  m_kaccPopup = new KPopupMenu(this);
  m_kaccPopup->setCheckable(true);
  m_accountsCombo->setPopup(m_kaccPopup);
  connect(m_kaccPopup, SIGNAL(activated(int)), this, SLOT(slotAccountActivated(int)));

  m_qbuttonNew->setGuiItem(KMyMoneyUtils::scheduleNewGuiItem());
  m_accountsCombo->setGuiItem(KMyMoneyUtils::accountsFilterGuiItem());

  KIconLoader *il = KGlobal::iconLoader();
  m_tabWidget->setTabIconSet(listTab, QIconSet(il->loadIcon("contents", KIcon::Small, KIcon::SizeSmall)));
  m_tabWidget->setTabIconSet(calendarTab, QIconSet(il->loadIcon("calendartab", KIcon::User, KIcon::SizeSmall)));

  readConfig();

  connect(m_qlistviewScheduled, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
    this, SLOT(slotListViewContextMenu(KListView*, QListViewItem*, const QPoint&)));
  connect(m_qlistviewScheduled, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSetSelectedItem(QListViewItem*)));

  connect(m_qlistviewScheduled, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotListItemExecuted(QListViewItem*, const QPoint&, int)));
  connect(m_qlistviewScheduled, SIGNAL(expanded(QListViewItem*)),
    this, SLOT(slotListViewExpanded(QListViewItem*)));
  connect(m_qlistviewScheduled, SIGNAL(collapsed(QListViewItem*)),
    this, SLOT(slotListViewCollapsed(QListViewItem*)));

  connect(m_calendar, SIGNAL(enterClicked(const MyMoneySchedule&, const QDate&)), this, SLOT(slotBriefEnterClicked(const MyMoneySchedule&, const QDate&)));

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassSchedule, this);
}

KScheduledView::~KScheduledView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassSchedule, this);
  writeConfig();
}

void KScheduledView::refresh(bool full, const QCString schedId)
{
  m_qlistviewScheduled->header()->setFont(KMyMoneySettings::listHeaderFont());

  m_qlistviewScheduled->clear();

  try
  {
    if (full)
    {
      try
      {
        int accountCount=0;

        m_kaccPopup->clear();

        MyMoneyFile* file = MyMoneyFile::instance();
        MyMoneyAccount acc;
        QCStringList::ConstIterator it_s;

        acc = file->asset();
        for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
        {
          MyMoneyAccount a = file->account(*it_s);
          m_kaccPopup->insertItem(a.name(), accountCount);
          m_kaccPopup->setItemChecked(accountCount, true);
          accountCount++;
        }
      }
      catch (MyMoneyException *e)
      {
        KMessageBox::detailedError(this, i18n("Unable to load accounts: "), e->what());
        delete e;
      }
    }

    // Refresh the calendar view first
    m_calendar->refresh();

    if (MyMoneyFile::instance()->scheduleList().count() == 0)
      return;

    MyMoneyFile *file = MyMoneyFile::instance();

    KScheduledListItem *itemTransfers = new KScheduledListItem(m_qlistviewScheduled, i18n("Transfers"));
    KScheduledListItem *itemDeposits = new KScheduledListItem(m_qlistviewScheduled, i18n("Deposits"));
    KScheduledListItem *itemBills = new KScheduledListItem(m_qlistviewScheduled, i18n("Bills"));
    KScheduledListItem *itemLoans = new KScheduledListItem(m_qlistviewScheduled, i18n("Loans"));

    QValueList<MyMoneySchedule> scheduledItems = file->scheduleList();

    QValueList<MyMoneySchedule>::Iterator it;

    KScheduledListItem *openItem=0;

    for (it = scheduledItems.begin(); it != scheduledItems.end(); ++it)
    {
      MyMoneySchedule schedData = (*it);
      KScheduledListItem* item=0;

      bool bContinue=true;
      QCStringList::iterator accIt;
      for (accIt=m_filterAccounts.begin(); accIt!=m_filterAccounts.end(); ++accIt)
      {
        if (*accIt == schedData.account().id())
        {
          bContinue=false; // Filter it out
          break;
        }
      }

      if (!bContinue)
        continue;

      switch (schedData.type())
      {
        case MyMoneySchedule::TYPE_BILL:
          item = new KScheduledListItem(itemBills, schedData);
          if (schedData.id() == schedId)
            openItem = item;
          break;

        case MyMoneySchedule::TYPE_DEPOSIT:
          item = new KScheduledListItem(itemDeposits, schedData);
          if (schedData.id() == schedId)
            openItem = item;
          break;

        case MyMoneySchedule::TYPE_TRANSFER:
          item = new KScheduledListItem(itemTransfers, schedData);
          if (schedData.id() == schedId)
            openItem = item;
          break;

        case MyMoneySchedule::TYPE_LOANPAYMENT:
          item = new KScheduledListItem(itemLoans, schedData);
          if (schedData.id() == schedId)
            openItem = item;
          break;

        case MyMoneySchedule::TYPE_ANY:
          break; // Should we display an error ?
      }
    }

    if (openItem)
    {
      m_qlistviewScheduled->setSelected(openItem, true);
    }
    // using a timeout is the only way, I got the 'ensureTransactionVisible'
    // working when coming from hidden form to visible form. I assume, this
    // has something to do with the delayed update of the display somehow.
    resize(width(), height()-1);
    QTimer::singleShot(10, this, SLOT(slotTimerDone()));
    m_qlistviewScheduled->update();

    if (m_openBills)
      itemBills->setOpen(true);

    if (m_openDeposits)
      itemDeposits->setOpen(true);

    if (m_openTransfers)
      itemTransfers->setOpen(true);

    if (m_openLoans)
      itemLoans->setOpen(true);

  } catch (MyMoneyException *e)
  {
    KMessageBox::error(this, e->what());
    delete e;
  }
}

void KScheduledView::slotTimerDone(void)
{
  QListViewItem* item;

  item = m_qlistviewScheduled->selectedItem();
  if(item) {
    m_qlistviewScheduled->ensureItemVisible(item);
  }

  // force a repaint of all items to update the branches
  for(item = m_qlistviewScheduled->firstChild(); item != 0; item = item->itemBelow()) {
    m_qlistviewScheduled->repaintItem(item);
  }
  resize(width(), height()+1);
}

void KScheduledView::slotReloadView(void)
{
  m_qbuttonNew->setEnabled(true);
  m_tabWidget->setEnabled(true);

  refresh(true, m_selectedSchedule);

  QTimer::singleShot(50, this, SLOT(slotRearrange()));
  QWidget::show();
}

void KScheduledView::show()
{
  slotReloadView();

  emit signalViewActivated();
}

void KScheduledView::slotRearrange(void)
{
  resizeEvent(0);
}

void KScheduledView::resizeEvent(QResizeEvent* e)
{
  m_qlistviewScheduled->setColumnWidth(1, 100);
  m_qlistviewScheduled->setColumnWidth(2, 100);
  m_qlistviewScheduled->setColumnWidth(3, 80);
  m_qlistviewScheduled->setColumnWidth(4, 120);
  m_qlistviewScheduled->setColumnWidth(5, 100);
  m_qlistviewScheduled->setColumnWidth(6, 120);
  m_qlistviewScheduled->setColumnWidth(0, m_qlistviewScheduled->visibleWidth()-620);

  // call base class resizeEvent()
  kScheduledViewDecl::resizeEvent(e);
}


void KScheduledView::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_openBills = config->readBoolEntry("KScheduleView_openBills", true);
  m_openDeposits = config->readBoolEntry("KScheduleView_openDeposits", true);
  m_openTransfers = config->readBoolEntry("KScheduleView_openTransfers", true);
  m_openLoans = config->readBoolEntry("KScheduleView_openLoans", true);

  m_qlistviewScheduled->header()->setFont(KMyMoneySettings::listHeaderFont());
}

void KScheduledView::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KScheduleView_openBills", m_openBills);
  config->writeEntry("KScheduleView_openDeposits", m_openDeposits);
  config->writeEntry("KScheduleView_openTransfers", m_openTransfers);
  config->writeEntry("KScheduleView_openLoans", m_openLoans);
  config->sync();
}

void KScheduledView::slotListViewContextMenu(KListView* /* view */, QListViewItem *item, const QPoint& /* pos */)
{
  KScheduledListItem *scheduleItem = dynamic_cast<KScheduledListItem *>(item);
  if (scheduleItem)
  {
    try
    {
      QCString scheduleId = scheduleItem->scheduleId();

      if (!scheduleId.isEmpty()) // Top level item
      {
        MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(scheduleId);
        kmymoney2->slotSelectSchedule(schedule);
        m_selectedSchedule = schedule.id();
      }
      kmymoney2->showContextMenu("schedule_context_menu");
    } catch (MyMoneyException *e)
    {
      KMessageBox::detailedSorry(this, i18n("Error activating context menu"), e->what());
      delete e;
    }
  }
  else
  {
    kmymoney2->showContextMenu("schedule_context_menu");
  }
}

void KScheduledView::slotListItemExecuted(QListViewItem* item, const QPoint&, int)
{
  KScheduledListItem* scheduleItem = (KScheduledListItem*)item;
  if (!scheduleItem)
    return;

  try
  {
    QCString scheduleId = scheduleItem->scheduleId();

    if (!scheduleId.isEmpty()) // Top level item
    {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(scheduleId);

      m_calendar->setDate(schedule.nextPayment(schedule.lastPayment()));
      m_tabWidget->showPage(calendarTab);
      m_selectedSchedule = schedule.id();
    }
  } catch (MyMoneyException *e)
  {
    KMessageBox::detailedSorry(this, i18n("Error executing item"), e->what());
    delete e;
  }
}

void KScheduledView::slotAccountActivated(int id)
{
  m_filterAccounts.clear();

  m_kaccPopup->setItemChecked(id, ((m_kaccPopup->isItemChecked(id))?false:true));

  try
  {
    int accountCount=0;
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount acc;
    QCStringList::ConstIterator it_s;

    acc = file->asset();
    for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
    {
      if (!m_kaccPopup->isItemChecked(accountCount))
      {
        m_filterAccounts.append(*it_s);
      }
      accountCount++;
    }

    m_calendar->setFilterAccounts(m_filterAccounts);

    refresh(false, m_selectedSchedule);
  }
  catch (MyMoneyException *e)
  {
    KMessageBox::detailedError(this, i18n("Unable to filter account"), e->what());
    delete e;
  }
}

void KScheduledView::slotListViewExpanded(QListViewItem* item)
{
  KScheduledListItem *scheduleItem = (KScheduledListItem*)item;
  if (scheduleItem)
  {
    if (scheduleItem->text(0) == i18n("Bills"))
      m_openBills = true;
    else if (scheduleItem->text(0) == i18n("Deposits"))
      m_openDeposits = true;
    else if (scheduleItem->text(0) == i18n("Transfers"))
      m_openTransfers = true;
    else if (scheduleItem->text(0) == i18n("Loans"))
      m_openLoans = true;
  }
}

void KScheduledView::slotListViewCollapsed(QListViewItem* item)
{
  KScheduledListItem *scheduleItem = (KScheduledListItem*)item;
  if (scheduleItem)
  {
    if (scheduleItem->text(0) == i18n("Bills"))
      m_openBills = false;
    else if (scheduleItem->text(0) == i18n("Deposits"))
      m_openDeposits = false;
    else if (scheduleItem->text(0) == i18n("Transfers"))
      m_openTransfers = false;
    else if (scheduleItem->text(0) == i18n("Loans"))
      m_openLoans = false;
  }
}
void KScheduledView::slotSelectSchedule(const QCString& schedule)
{
  refresh(true, schedule);
}

void KScheduledView::slotBriefEnterClicked(const MyMoneySchedule& schedule, const QDate& date)
{
  KEnterScheduleDialog *dlg = new KEnterScheduleDialog(this, schedule, date);
  connect(dlg, SIGNAL(newCategory(MyMoneyAccount&)), kmymoney2, SLOT(slotCategoryNew(MyMoneyAccount&)));
  if (dlg->exec())
  {
    refresh(false);
  }
}

void KScheduledView::slotSetSelectedItem(QListViewItem* item)
{
  kmymoney2->slotSelectSchedule();
  KScheduledListItem* schedItem = static_cast<KScheduledListItem*>(item);
  if(item) {
    try {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(schedItem->scheduleId());
      kmymoney2->slotSelectSchedule(schedule);
      m_selectedSchedule = schedItem->scheduleId();
    } catch(MyMoneyException* e) {
      qDebug("KScheduledView::slotSetSelectedItem: %s", e->what().data());
      delete e;
    }
  }
}

void KScheduledView::update(const QCString& /* id */)
{
  KScheduledListItem *p = dynamic_cast<KScheduledListItem*>(m_qlistviewScheduled->selectedItem());
  QCString schedId;
  if(p)
    schedId = p->scheduleId();
  refresh(true, schedId);
}


#include "kscheduledview.moc"
