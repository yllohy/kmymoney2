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
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qlayout.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kscheduledview.h"
#include "kscheduledlistitem.h"
#include "../widgets/kmymoneyscheduleddatetbl.h"

KScheduledView::KScheduledView(QWidget *parent, const char *name )
 : kScheduledViewDecl(parent,name, false)
{
  m_qlistviewScheduled->setRootIsDecorated(true);
  m_qlistviewScheduled->addColumn(i18n("Type/Name"));
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

  readConfig();

  KIconLoader *kiconloader = KGlobal::iconLoader();
  KPopupMenu* kpopupmenuNew = new KPopupMenu(this);
  kpopupmenuNew->insertItem(kiconloader->loadIcon("new_bill", KIcon::Small), i18n("Bill"), this, SLOT(slotNewBill()));
  kpopupmenuNew->insertItem(kiconloader->loadIcon("new_deposit", KIcon::Small), i18n("Deposit"), this, SLOT(slotNewDeposit()));
  kpopupmenuNew->insertItem(kiconloader->loadIcon("new_transfer", KIcon::Small), i18n("Transfer"), this, SLOT(slotNewTransfer()));
  m_qbuttonNew->setPopup(kpopupmenuNew);

  m_qbuttonEdit->setEnabled(false);
  m_qbuttonDelete->setEnabled(false);

  connect(m_qlistviewScheduled, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSelectionChanged(QListViewItem*)));
  connect(m_qlistviewScheduled, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotListViewContextMenu(QListViewItem*, const QPoint&, int)));
  connect(m_qbuttonEdit, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
  connect(m_qbuttonDelete, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));
  connect(m_accountsCombo, SIGNAL(activated(const QString&)),
    this, SLOT(slotAccountSelected(const QString&)));
}

KScheduledView::~KScheduledView()
{
  writeConfig();
}

void KScheduledView::refresh(const QString schedId)
{
  m_qlistviewScheduled->clear();

  try
  {

    // Refresh the calendar view first
    m_calendar->refresh(m_accountId);

    if (MyMoneyScheduled::instance()->count(m_accountId) == 0)
      return;

//    MyMoneyScheduled *scheduled = MyMoneyFile::instance()->scheduledInstance();
    MyMoneyScheduled *scheduled = MyMoneyScheduled::instance();

    KScheduledListItem *itemBills = new KScheduledListItem(m_qlistviewScheduled, i18n("Bills"));
    KScheduledListItem *itemDeposits = new KScheduledListItem(m_qlistviewScheduled, i18n("Deposits"));
    KScheduledListItem *itemTransfers = new KScheduledListItem(m_qlistviewScheduled, i18n("Transfers"));
  
    QStringList scheduledItems = scheduled->getScheduled(m_accountId);

    QStringList::Iterator it;

    KScheduledListItem *openItem=0;

    for (it = scheduledItems.begin(); it != scheduledItems.end(); ++it)
    {
      MyMoneySchedule schedData = scheduled->getSchedule(m_accountId, *it);
      KScheduledListItem* item=0;

      switch (schedData.type())
      {
        case MyMoneySchedule::TYPE_BILL:
          item = new KScheduledListItem(itemBills, m_accountId, schedData);
          if (schedData.id() == schedId)
            openItem = item;
          break;
        case MyMoneySchedule::TYPE_DEPOSIT:
          item = new KScheduledListItem(itemDeposits, m_accountId, schedData);
          if (schedData.id() == schedId)
            openItem = item;
          break;
        case MyMoneySchedule::TYPE_TRANSFER:
          item = new KScheduledListItem(itemTransfers, m_accountId, schedData);
          if (schedData.id() == schedId)
            openItem = item;
          break;
        case MyMoneySchedule::TYPE_ANY:
          break; // Should we display an error ?
      }
    }

    if (openItem)
    {
      m_qlistviewScheduled->ensureItemVisible(openItem);
    }
  } catch (MyMoneyException *e)
  {
    KMessageBox::error(this, e->what());
    delete e;
  }
}

void KScheduledView::show()
{
  loadAccounts();

  refresh();
  
  if (m_accountsCombo->count() == 0)
  {
    // disable operations if no accounts exist
    m_qbuttonNew->setEnabled(false);
    m_qbuttonDelete->setEnabled(false);
    m_qbuttonEdit->setEnabled(false);
    m_accountsCombo->setEnabled(false);
    m_tabWidget->setEnabled(false);
  }

  QWidget::show();

  m_qlistviewScheduled->setColumnWidth(0, 100);
  m_qlistviewScheduled->setColumnWidth(1, 120);
  m_qlistviewScheduled->setColumnWidth(2, 120);
  m_qlistviewScheduled->setColumnWidth(3, 120);
  m_qlistviewScheduled->setColumnWidth(4, 120);
  m_qlistviewScheduled->setColumnWidth(0, m_qlistviewScheduled->width()-480);

  emit signalViewActivated();
}

void KScheduledView::resizeEvent(QResizeEvent* e)
{
  m_qlistviewScheduled->setColumnWidth(0, 100);
  m_qlistviewScheduled->setColumnWidth(1, 120);
  m_qlistviewScheduled->setColumnWidth(2, 120);
  m_qlistviewScheduled->setColumnWidth(3, 120);
  m_qlistviewScheduled->setColumnWidth(4, 120);
  m_qlistviewScheduled->setColumnWidth(0, m_qlistviewScheduled->width()-480);

  // call base class resizeEvent()
  kScheduledViewDecl::resizeEvent(e);
}

void KScheduledView::slotDeleteClicked()
{
  if (m_selectedSchedule != "")
  {
    try
    {
      if (KMessageBox::questionYesNo(this, i18n("Are you sure you want to delete the selected schedule?")) == KMessageBox::No)
        return;
        
      MyMoneyScheduled::instance()->removeSchedule(m_accountId, m_selectedSchedule);

      refresh();
      
    } catch (MyMoneyException *e)
    {
      KMessageBox::detailedSorry(this, i18n("Unable to remove schedule"), e->what());
      delete e;
    }
  }
}

void KScheduledView::slotSelectionChanged(QListViewItem* item)
{
  if (item)
  {
    m_qbuttonDelete->setEnabled(true);
    m_qbuttonEdit->setEnabled(true);
    m_selectedSchedule = ((KScheduledListItem*)item)->scheduleId();
  }
  else
  {
    m_qbuttonDelete->setEnabled(false);
    m_qbuttonEdit->setEnabled(false);
  }
}

void KScheduledView::slotEditClicked()
{
  if (m_selectedSchedule != "")
  {
    try
    {
      MyMoneySchedule schedule = MyMoneyScheduled::instance()->getSchedule(m_accountId, m_selectedSchedule);

      switch (schedule.type())
      {
        case MyMoneySchedule::TYPE_BILL:
        {
          KEditScheduledBillDlg *m_keditschedbilldlg = new KEditScheduledBillDlg(m_accountId, schedule, this);
          if (m_keditschedbilldlg->exec() == QDialog::Accepted)
          {
            MyMoneySchedule sched = m_keditschedbilldlg->schedule();
            MyMoneyScheduled::instance()->replaceSchedule(m_accountId, m_selectedSchedule, sched);
            refresh(m_selectedSchedule);
          }
          delete m_keditschedbilldlg;
          break;
        }
        case MyMoneySchedule::TYPE_DEPOSIT:
        {
          KEditScheduledDepositDlg *m_keditscheddepdlg = new KEditScheduledDepositDlg(m_accountId, schedule, this);
          if (m_keditscheddepdlg->exec() == QDialog::Accepted)
          {
            MyMoneySchedule sched = m_keditscheddepdlg->schedule();
            MyMoneyScheduled::instance()->replaceSchedule(m_accountId, m_selectedSchedule, sched);
            refresh(m_selectedSchedule);
          }
          delete m_keditscheddepdlg;
          break;
        }
        case MyMoneySchedule::TYPE_TRANSFER:
        {
          KEditScheduledTransferDlg *m_keditschedtransdlg = new KEditScheduledTransferDlg(m_accountId, schedule, this);
          if (m_keditschedtransdlg->exec() == QDialog::Accepted)
          {
            MyMoneySchedule sched = m_keditschedtransdlg->schedule();
            MyMoneyScheduled::instance()->replaceSchedule(m_accountId, m_selectedSchedule, sched);
            refresh(m_selectedSchedule);
          }
          delete m_keditschedtransdlg;
          break;
          break;
        }
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
//  m_lastCat = config->readEntry("KCategoriesView_LastCategory");
}

void KScheduledView::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
//  config->writeEntry("KCategoriesView_LastCategory", item->text(0));
  config->sync();
}

void KScheduledView::slotNewBill()
{
  KMessageBox::information(this, "WARNING:\n\n\tALL SCHEDULE DATA WILL BE LOST ONCE KMYMONEY HAS BEEN CLOSED");
  
  MyMoneySchedule schedule;
  
  KEditScheduledBillDlg *m_keditschedbilldlg = new KEditScheduledBillDlg(m_accountId, schedule, this);
  if (m_keditschedbilldlg->exec() == QDialog::Accepted)
  {
    MyMoneySchedule sched = m_keditschedbilldlg->schedule();
    MyMoneyScheduled *scheduled = MyMoneyScheduled::instance();

    try
    {
      QString schedId = scheduled->addSchedule(m_accountId, sched);
      refresh(schedId);
    } catch (MyMoneyException *e)
    {
      QString s("Unable to add schedule: ");
      s += e->what();
      KMessageBox::information(this, s);
    }
  }

  delete m_keditschedbilldlg;
}

void KScheduledView::slotNewDeposit()
{
  KMessageBox::information(this, "WARNING:\n\n\tALL SCHEDULE DATA WILL BE LOST ONCE KMYMONEY HAS BEEN CLOSED");

  MyMoneySchedule schedule;

  KEditScheduledDepositDlg *m_keditscheddepdlg = new KEditScheduledDepositDlg(m_accountId, schedule, this);
  if (m_keditscheddepdlg->exec() == QDialog::Accepted)
  {
    MyMoneySchedule sched = m_keditscheddepdlg->schedule();
    MyMoneyScheduled *scheduled = MyMoneyScheduled::instance();

    try
    {
      QString schedId = scheduled->addSchedule(m_accountId, sched);
      refresh(schedId);
    } catch (MyMoneyException *e)
    {
      QString s("Unable to add schedule: ");
      s += e->what();
      KMessageBox::information(this, s);
    }
  }

  delete m_keditscheddepdlg;
}

void KScheduledView::slotNewTransfer()
{
  KMessageBox::information(this, "WARNING:\n\n\tALL SCHEDULE DATA WILL BE LOST ONCE KMYMONEY HAS BEEN CLOSED");

  MyMoneySchedule schedule;

  KEditScheduledTransferDlg *m_keditschedtransdlg = new KEditScheduledTransferDlg(m_accountId, schedule, this);
  if (m_keditschedtransdlg->exec() == QDialog::Accepted)
  {
    MyMoneySchedule sched = m_keditschedtransdlg->schedule();
    MyMoneyScheduled *scheduled = MyMoneyScheduled::instance();

    try
    {
      QString schedId = scheduled->addSchedule(m_accountId, sched);
      refresh(schedId);
    } catch (MyMoneyException *e)
    {
      QString s("Unable to add schedule: ");
      s += e->what();
      KMessageBox::information(this, s);
    }
  }

  delete m_keditschedtransdlg;
}

void KScheduledView::loadAccounts(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  m_accountsCombo->clear();

  MyMoneyAccount acc;
  QCStringList::ConstIterator it_s;

  QString selectAccountName;
  
  acc = file->asset();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
  {
    m_accountsCombo->insertItem(file->account(*it_s).name());
    selectAccountName = file->account(*it_s).name();
    m_accountsCombo->setCurrentText(file->account(*it_s).name());
  }

  acc = file->liability();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
  {
    m_accountsCombo->insertItem(file->account(*it_s).name());
    selectAccountName = file->account(*it_s).name();
    m_accountsCombo->setCurrentText(file->account(*it_s).name());
  }

  slotAccountSelected(selectAccountName);
}

void KScheduledView::slotAccountSelected(const QString& accountName)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyAccount acc;
  QCStringList::ConstIterator it_s;

  acc = file->asset();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
  {
    MyMoneyAccount a = file->account(*it_s);
    if (a.name() == accountName)
    {
      m_accountId = *it_s;
      refresh();
      return;
    }
  }

  acc = file->liability();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
  {
    MyMoneyAccount a = file->account(*it_s);
    if (a.name() == accountName)
    {
      m_accountId = *it_s;
      refresh();
      return;
    }
  }
}

void KScheduledView::slotListViewContextMenu(QListViewItem *item, const QPoint& pos, int col)
{
  KScheduledListItem *scheduleItem = (KScheduledListItem*)item;
  if (scheduleItem)
  {
    try
    {
      QString scheduleId = scheduleItem->scheduleId();

      KIconLoader *kiconloader = KGlobal::iconLoader();
      KPopupMenu *listViewMenu = new KPopupMenu(m_qlistviewScheduled);

      if (scheduleId == "") // Top level item
      {
        QString text = scheduleItem->text(0);
        if (text == "Bills")
        {
          listViewMenu->insertTitle(i18n("Bill Options"));
          listViewMenu->insertItem(kiconloader->loadIcon("new_bill", KIcon::Small), i18n("New Bill..."), this, SLOT(slotNewBill()));
        }
        else if (text == "Deposits")
        {
          listViewMenu->insertTitle(i18n("Deposit Options"));
          listViewMenu->insertItem(kiconloader->loadIcon("new_deposit", KIcon::Small), i18n("New Deposit..."), this, SLOT(slotNewDeposit()));
        }
        if (text == "Transfers")
        {
          listViewMenu->insertTitle(i18n("Transfer Options"));
          listViewMenu->insertItem(kiconloader->loadIcon("new_transfer", KIcon::Small), i18n("New Transfer..."), this, SLOT(slotNewTransfer()));
        }
      }
      else // schedule item
      {
        MyMoneySchedule schedule = MyMoneyScheduled::instance()->getSchedule(m_accountId, scheduleId);

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
        }
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
