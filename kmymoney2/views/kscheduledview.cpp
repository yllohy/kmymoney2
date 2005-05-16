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
#include "../dialogs/ieditscheduledialog.h"
#include "../dialogs/keditloanwizard.h"
#include "../dialogs/kenterscheduledialog.h"
#include "../kmymoneyutils.h"

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

  KPopupMenu* kpopupmenuNew = new KPopupMenu(this);
  kpopupmenuNew->insertItem(KMyMoneyUtils::billScheduleIcon(KIcon::SizeSmall), i18n("Bill"), this, SLOT(slotNewBill()));
  kpopupmenuNew->insertItem(KMyMoneyUtils::depositScheduleIcon(KIcon::SizeSmall), i18n("Deposit"), this, SLOT(slotNewDeposit()));
  kpopupmenuNew->insertItem(KMyMoneyUtils::transferScheduleIcon(KIcon::SizeSmall), i18n("Transfer"), this, SLOT(slotNewTransfer()));
  m_qbuttonNew->setPopup(kpopupmenuNew);

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

  connect(m_qlistviewScheduled, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotListViewContextMenu(QListViewItem*, const QPoint&, int)));
  connect(m_qlistviewScheduled, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotListItemExecuted(QListViewItem*, const QPoint&, int)));
  connect(m_qlistviewScheduled, SIGNAL(expanded(QListViewItem*)),
    this, SLOT(slotListViewExpanded(QListViewItem*)));
  connect(m_qlistviewScheduled, SIGNAL(collapsed(QListViewItem*)),
    this, SLOT(slotListViewCollapsed(QListViewItem*)));
  connect(m_qlistviewScheduled, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSetSelectedItem(QListViewItem*)));

  connect(m_calendar, SIGNAL(enterClicked(const MyMoneySchedule&, const QDate&)), this, SLOT(slotBriefEnterClicked(const MyMoneySchedule&, const QDate&)));
}

KScheduledView::~KScheduledView()
{
  writeConfig();
}

void KScheduledView::refresh(bool full, const QCString schedId)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont headerFont(m_qlistviewScheduled->font());
  headerFont = config->readFontEntry("listHeaderFont", &headerFont);
  m_qlistviewScheduled->header()->setFont(headerFont);

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

void KScheduledView::slotDeleteClicked()
{
  if (!m_selectedSchedule.isEmpty())
  {
    try
    {
      MyMoneySchedule sched = MyMoneyFile::instance()->schedule(m_selectedSchedule);
      QString msg = i18n("Are you sure you want to delete the selected schedule?");
      if(sched.type() == MyMoneySchedule::TYPE_LOANPAYMENT)
      {
        msg += QString(" ");
        msg += i18n("In case of loan payments it is currently not possible to recreate the schedules");
      }
      if (KMessageBox::questionYesNo(this, msg) == KMessageBox::No)
        return;

      MyMoneyFile::instance()->removeSchedule(sched);

      refresh();

    } catch (MyMoneyException *e)
    {
      KMessageBox::detailedSorry(this, i18n("Unable to remove schedule"), e->what());
      delete e;
    }
  }
}

void KScheduledView::slotEditClicked()
{
  if (!m_selectedSchedule.isEmpty())
  {
    try
    {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(m_selectedSchedule);

      const char *action = 0;
      switch (schedule.type())
      {
        case MyMoneySchedule::TYPE_BILL:
          action = MyMoneySplit::ActionWithdrawal;
          break;

        case MyMoneySchedule::TYPE_DEPOSIT:
          action = MyMoneySplit::ActionDeposit;
          break;

        case MyMoneySchedule::TYPE_TRANSFER:
          action = MyMoneySplit::ActionTransfer;
          break;

        case MyMoneySchedule::TYPE_LOANPAYMENT:
        case MyMoneySchedule::TYPE_ANY:
          break;
      }

      KEditScheduleDialog* sched_dlg;
      KEditLoanWizard* loan_wiz;

      switch (schedule.type())
      {
        case MyMoneySchedule::TYPE_BILL:
        case MyMoneySchedule::TYPE_DEPOSIT:
        case MyMoneySchedule::TYPE_TRANSFER:
          sched_dlg = new KEditScheduleDialog(action, schedule, this);
          if (sched_dlg->exec() == QDialog::Accepted)
          {
            MyMoneySchedule sched = sched_dlg->schedule();
            MyMoneyFile::instance()->modifySchedule(sched);
            refresh(false, m_selectedSchedule);
          }
          delete sched_dlg;
          break;

        case MyMoneySchedule::TYPE_LOANPAYMENT:
          loan_wiz = new KEditLoanWizard(schedule.account(2));
          if (loan_wiz->exec() == QDialog::Accepted)
          {
            MyMoneyFile::instance()->modifySchedule(loan_wiz->schedule());
            MyMoneyFile::instance()->modifyAccount(loan_wiz->account());
            refresh(false, m_selectedSchedule);
          }
          delete loan_wiz;
          break;

        case MyMoneySchedule::TYPE_ANY:
          break;
      }
    } catch (MyMoneyException *e)
    {
      delete e;
    }
  }
}

void KScheduledView::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_openBills = config->readBoolEntry("KScheduleView_openBills", true);
  m_openDeposits = config->readBoolEntry("KScheduleView_openDeposits", true);
  m_openTransfers = config->readBoolEntry("KScheduleView_openTransfers", true);
  m_openLoans = config->readBoolEntry("KScheduleView_openLoans", true);

  config->setGroup("List Options");
  QFont headerFont(m_qlistviewScheduled->font());
  headerFont = config->readFontEntry("listHeaderFont", &headerFont);
  m_qlistviewScheduled->header()->setFont(headerFont);
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

void KScheduledView::slotNewBill()
{
  MyMoneySchedule schedule;

  KEditScheduleDialog *m_keditschedbilldlg = new KEditScheduleDialog(MyMoneySplit::ActionWithdrawal, schedule, this);
  if (m_keditschedbilldlg->exec() == QDialog::Accepted)
  {
    MyMoneySchedule sched = m_keditschedbilldlg->schedule();
    try
    {
      MyMoneyFile::instance()->addSchedule(sched);
      refresh(false, sched.id());
    } catch (MyMoneyException *e)
    {
      KMessageBox::information(this, i18n("Unable to add schedule: "), e->what());
      delete e;
    }
  }

  delete m_keditschedbilldlg;
}

void KScheduledView::slotNewDeposit()
{
  MyMoneySchedule schedule;

  KEditScheduleDialog *m_keditscheddepdlg = new KEditScheduleDialog(MyMoneySplit::ActionDeposit, schedule, this);
  if (m_keditscheddepdlg->exec() == QDialog::Accepted)
  {
    MyMoneySchedule sched = m_keditscheddepdlg->schedule();

    try
    {
      MyMoneyFile::instance()->addSchedule(sched);
      refresh(false, sched.id());
    } catch (MyMoneyException *e)
    {
      KMessageBox::information(this, i18n("Unable to add schedule: "), e->what());
      delete e;
    }
  }

  delete m_keditscheddepdlg;
}

void KScheduledView::slotNewTransfer()
{
  MyMoneySchedule schedule;

  KEditScheduleDialog *m_keditschedtransdlg = new KEditScheduleDialog(MyMoneySplit::ActionTransfer, schedule, this);
  if (m_keditschedtransdlg->exec() == QDialog::Accepted)
  {
    MyMoneySchedule sched = m_keditschedtransdlg->schedule();

    try
    {
      MyMoneyFile::instance()->addSchedule(sched);
      refresh(false, sched.id());
    } catch (MyMoneyException *e)
    {
      KMessageBox::information(this, i18n("Unable to add schedule: "), e->what());
      delete e;
    }
  }

  delete m_keditschedtransdlg;
}

void KScheduledView::slotListViewContextMenu(QListViewItem *item, const QPoint& pos, int/* col*/)
{
  KScheduledListItem *scheduleItem = (KScheduledListItem*)item;
  if (scheduleItem)
  {
    try
    {
      QCString scheduleId = scheduleItem->scheduleId();

      KIconLoader *kiconloader = KGlobal::iconLoader();
      KPopupMenu *listViewMenu = new KPopupMenu(m_qlistviewScheduled);

      if (scheduleId.isEmpty()) // Top level item
      {
        QString text = scheduleItem->text(0);
        if (text == i18n("Bills"))
        {
          listViewMenu->insertTitle(i18n("Bill Options"));
          listViewMenu->insertItem(kiconloader->loadIcon("new_bill", KIcon::Small), i18n("New Bill..."), this, SLOT(slotNewBill()));
        }
        else if (text == i18n("Deposits"))
        {
          listViewMenu->insertTitle(i18n("Deposit Options"));
          listViewMenu->insertItem(kiconloader->loadIcon("new_deposit", KIcon::Small), i18n("New Deposit..."), this, SLOT(slotNewDeposit()));
        }
        else if (text == i18n("Transfers"))
        {
          listViewMenu->insertTitle(i18n("Transfer Options"));
          listViewMenu->insertItem(kiconloader->loadIcon("new_transfer", KIcon::Small), i18n("New Transfer..."), this, SLOT(slotNewTransfer()));
        }
      }
      else // schedule item
      {
        MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(scheduleId);
        m_selectedSchedule = schedule.id();

        switch (schedule.type())
        {
          case MyMoneySchedule::TYPE_BILL:
            listViewMenu->insertTitle(i18n("Bill Options"));
            listViewMenu->insertItem(kiconloader->loadIcon("new_bill", KIcon::Small), i18n("New Bill..."), this, SLOT(slotNewBill()));
            break;
          case MyMoneySchedule::TYPE_DEPOSIT:
            listViewMenu->insertTitle(i18n("Deposit Options"));
            listViewMenu->insertItem(kiconloader->loadIcon("new_deposit", KIcon::Small), i18n("New Deposit..."), this, SLOT(slotNewDeposit()));
            break;
          case MyMoneySchedule::TYPE_TRANSFER:
            listViewMenu->insertTitle(i18n("Transfer Options"));
            listViewMenu->insertItem(kiconloader->loadIcon("new_transfer", KIcon::Small), i18n("New Transfer..."), this, SLOT(slotNewTransfer()));
            break;
          case MyMoneySchedule::TYPE_LOANPAYMENT:
          case MyMoneySchedule::TYPE_ANY:
            break;
        }
        listViewMenu->insertSeparator();
        listViewMenu->insertItem(kiconloader->loadIcon("enter", KIcon::Small), i18n("Enter..."), this, SLOT(slotEnterClicked()));
        listViewMenu->insertSeparator();
        listViewMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("Edit..."), this, SLOT(slotEditClicked()));
        listViewMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small), i18n("Delete..."), this, SLOT(slotDeleteClicked()));
      }

      listViewMenu->popup(pos);
    } catch (MyMoneyException *e)
    {
      KMessageBox::detailedSorry(this, i18n("Error building context menu"), e->what());
      delete e;
    }
  }
  else
  {
    KIconLoader *kiconloader = KGlobal::iconLoader();
    KPopupMenu *listViewMenu = new KPopupMenu(m_qlistviewScheduled);

    listViewMenu->insertItem(kiconloader->loadIcon("new_bill", KIcon::Small), i18n("New Bill..."), this, SLOT(slotNewBill()));
    listViewMenu->insertItem(kiconloader->loadIcon("new_deposit", KIcon::Small), i18n("New Deposit..."), this, SLOT(slotNewDeposit()));
    listViewMenu->insertItem(kiconloader->loadIcon("new_transfer", KIcon::Small), i18n("New Transfer..."), this, SLOT(slotNewTransfer()));
    listViewMenu->popup(pos);
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

void KScheduledView::slotEnterClicked()
{
  if (!m_selectedSchedule.isEmpty())
  {
    try
    {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(m_selectedSchedule);

      KEnterScheduleDialog *dlg = new KEnterScheduleDialog(this, schedule);
      if (dlg->exec())
      {
        refresh(false/*, schedule.id()*/);
      }
    } catch (MyMoneyException *e)
    {
      delete e;
    }
  }
}

void KScheduledView::slotBriefEnterClicked(const MyMoneySchedule& schedule, const QDate& date)
{
  KEnterScheduleDialog *dlg = new KEnterScheduleDialog(this, schedule, date);
  if (dlg->exec())
  {
    refresh(false);
  }
}

void KScheduledView::slotSetSelectedItem(QListViewItem* item)
{
  KScheduledListItem* schedItem = static_cast<KScheduledListItem*>(item);
  if(item)
    m_selectedSchedule = schedItem->scheduleId();
}

#include "kscheduledview.moc"
