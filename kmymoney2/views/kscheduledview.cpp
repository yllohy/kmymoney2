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
#include <klistviewsearchline.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kscheduledview.h"
#include "kscheduledlistitem.h"
#include "../widgets/kmymoneyscheduleddatetbl.h"
// #include "../dialogs/kenterscheduledialog.h"
#include <kmymoney/kmymoneyutils.h>
#include <kmymoney/kmymoneyglobalsettings.h>

#include "../kmymoney2.h"

KScheduledView::KScheduledView(QWidget *parent, const char *name ) :
  KScheduledViewDecl(parent,name, false),
  m_openBills(true),
  m_openDeposits(true),
  m_openTransfers(true),
  m_openLoans(true)
{
  // create the searchline widget
  // and insert it into the existing layout
  m_searchWidget = new KListViewSearchLineWidget(m_qlistviewScheduled, m_listTab);
  m_listTabLayout->insertWidget(0, m_searchWidget);

  m_qlistviewScheduled->addColumn(i18n("Type/Name"));
  m_qlistviewScheduled->addColumn(i18n("Account"));
  m_qlistviewScheduled->addColumn(i18n("Payee"));
  m_qlistviewScheduled->addColumn(i18n("Amount"));
  m_qlistviewScheduled->addColumn(i18n("Next Due Date"));
  m_qlistviewScheduled->addColumn(i18n("Frequency"));
  m_qlistviewScheduled->addColumn(i18n("Payment Method"));
  m_qlistviewScheduled->setColumnAlignment(3, Qt::AlignRight);

  readConfig();

  m_qlistviewScheduled->setMultiSelection(false);
  m_qlistviewScheduled->header()->setResizeEnabled(true);
  if(m_qlistviewScheduled->sortColumn() == -1)
    m_qlistviewScheduled->setSorting(0);

  connect(m_qbuttonNew, SIGNAL(clicked()), kmymoney2->action("schedule_new"), SLOT(activate()));

  // attach popup to 'Filter...' button
  m_kaccPopup = new KPopupMenu(this);
  m_kaccPopup->setCheckable(true);
  m_accountsCombo->setPopup(m_kaccPopup);
  connect(m_kaccPopup, SIGNAL(activated(int)), this, SLOT(slotAccountActivated(int)));

  m_qbuttonNew->setGuiItem(KMyMoneyUtils::scheduleNewGuiItem());
  m_accountsCombo->setGuiItem(KMyMoneyUtils::accountsFilterGuiItem());

  KIconLoader *il = KGlobal::iconLoader();
  m_tabWidget->setTabIconSet(m_listTab, QIconSet(il->loadIcon("contents", KIcon::Small, KIcon::SizeSmall)));
  m_tabWidget->setTabIconSet(m_calendarTab, QIconSet(il->loadIcon("calendartab", KIcon::User, KIcon::SizeSmall)));

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

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotReloadView()));
}

KScheduledView::~KScheduledView()
{
  writeConfig();
}

void KScheduledView::refresh(bool full, const QCString& schedId)
{
  m_qlistviewScheduled->header()->setFont(KMyMoneyGlobalSettings::listHeaderFont());

  QPoint startPoint = QPoint(m_qlistviewScheduled->contentsX(), m_qlistviewScheduled->contentsY());

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

    MyMoneyFile *file = MyMoneyFile::instance();
    QValueList<MyMoneySchedule> scheduledItems = file->scheduleList();

    if (scheduledItems.count() == 0)
      return;

    KScheduledListItem *itemBills = new KScheduledListItem(m_qlistviewScheduled, i18n("Bills"), KMyMoneyUtils::billScheduleIcon(KIcon::Small), "0");
    KScheduledListItem *itemDeposits = new KScheduledListItem(m_qlistviewScheduled, i18n("Deposits"), KMyMoneyUtils::depositScheduleIcon(KIcon::Small), "1");
    KScheduledListItem *itemLoans = new KScheduledListItem(m_qlistviewScheduled, i18n("Loans"), KMyMoneyUtils::transferScheduleIcon(KIcon::Small), "2");
    KScheduledListItem *itemTransfers = new KScheduledListItem(m_qlistviewScheduled, i18n("Transfers"), KMyMoneyUtils::transferScheduleIcon(KIcon::Small), "3");

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

      KScheduledListItem* parent = 0;
      switch (schedData.type())
      {
        case MyMoneySchedule::TYPE_BILL:
          parent = itemBills;
          break;

        case MyMoneySchedule::TYPE_DEPOSIT:
          parent = itemDeposits;
          break;

        case MyMoneySchedule::TYPE_TRANSFER:
          parent = itemTransfers;
          break;

        case MyMoneySchedule::TYPE_LOANPAYMENT:
          parent = itemLoans;
          break;

        case MyMoneySchedule::TYPE_ANY:
          break; // Should we display an error ?
      }
      if(parent) {
        if(!KMyMoneyGlobalSettings::hideFinishedSchedules() || !schedData.isFinished()) {
          item = new KScheduledListItem(parent, schedData);
          if (schedData.id() == schedId)
            openItem = item;
        }
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

    // force repaint in case the filter is set
    m_searchWidget->searchLine()->updateSearch(QString::null);

    if (m_openBills)
      itemBills->setOpen(true);

    if (m_openDeposits)
      itemDeposits->setOpen(true);

    if (m_openTransfers)
      itemTransfers->setOpen(true);

    if (m_openLoans)
      itemLoans->setOpen(true);

    m_qlistviewScheduled->setContentsPos(startPoint.x(), startPoint.y());

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
  m_needReload = true;
  if(isVisible()) {
    m_qbuttonNew->setEnabled(true);
    m_tabWidget->setEnabled(true);

    refresh(true, m_selectedSchedule);

    m_needReload = false;
    QTimer::singleShot(50, this, SLOT(slotRearrange()));
  }
}

void KScheduledView::show()
{
  KScheduledViewDecl::show();

  if(m_needReload)
    slotReloadView();
}

void KScheduledView::slotRearrange(void)
{
  resizeEvent(0);
}

void KScheduledView::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_openBills = config->readBoolEntry("KScheduleView_openBills", true);
  m_openDeposits = config->readBoolEntry("KScheduleView_openDeposits", true);
  m_openTransfers = config->readBoolEntry("KScheduleView_openTransfers", true);
  m_openLoans = config->readBoolEntry("KScheduleView_openLoans", true);
  m_tabWidget->setCurrentPage(config->readNumEntry("KScheduleView_tab", 0));

  m_qlistviewScheduled->header()->setFont(KMyMoneyGlobalSettings::listHeaderFont());
  m_qlistviewScheduled->restoreLayout(KGlobal::config(), "Schedule View Settings");

}

void KScheduledView::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KScheduleView_openBills", m_openBills);
  config->writeEntry("KScheduleView_openDeposits", m_openDeposits);
  config->writeEntry("KScheduleView_openTransfers", m_openTransfers);
  config->writeEntry("KScheduleView_openLoans", m_openLoans);
  config->writeEntry("KScheduleView_tab", m_tabWidget->currentPageIndex());
  config->sync();

  m_qlistviewScheduled->saveLayout(KGlobal::config(), "Schedule View Settings");
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
        emit scheduleSelected(schedule);
        m_selectedSchedule = schedule.id();
      }
      emit openContextMenu();
    } catch (MyMoneyException *e)
    {
      KMessageBox::detailedSorry(this, i18n("Error activating context menu"), e->what());
      delete e;
    }
  }
  else
  {
    emit openContextMenu();
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
      m_selectedSchedule = schedule.id();
      emit editSchedule();
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
  Q_UNUSED(date);

  emit scheduleSelected(schedule);
  emit enterSchedule();
}

void KScheduledView::slotSetSelectedItem(QListViewItem* item)
{
  emit scheduleSelected(MyMoneySchedule());
  KScheduledListItem* schedItem = static_cast<KScheduledListItem*>(item);
  if(item) {
    try {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(schedItem->scheduleId());
      emit scheduleSelected(schedule);
      m_selectedSchedule = schedItem->scheduleId();
    } catch(MyMoneyException* e) {
      qDebug("KScheduledView::slotSetSelectedItem: %s", e->what().data());
      delete e;
    }
  }
}


#include "kscheduledview.moc"
