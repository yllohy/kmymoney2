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

KScheduledView::KScheduledView(QWidget *parent, const char *name )
 : kScheduledViewDecl(parent,name, false)
{
  m_qlistviewScheduled->setRootIsDecorated(true);
  m_qlistviewScheduled->addColumn(i18n("Type"));
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

  connect(m_qlistviewScheduled, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSelectionChanged(QListViewItem*)));
  connect(m_qbuttonEdit, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
  connect(m_qbuttonDelete, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));
  connect(m_accountsCombo, SIGNAL(activated(const QString&)),
    this, SLOT(slotAccountSelected(const QString&)));
}

KScheduledView::~KScheduledView()
{
  writeConfig();
}

void KScheduledView::refresh(void)
{
  loadAccounts();

  m_qlistviewScheduled->clear();
  //KScheduledListItem *itemBills = new KScheduledListItem(m_qlistviewScheduled, i18n("Bills"));
  //KScheduledListItem *itemDeposits = new KScheduledListItem(m_qlistviewScheduled, i18n("Deposits"));
}

void KScheduledView::show()
{
  refresh();
  emit signalViewActivated();

  m_qlistviewScheduled->setColumnWidth(0, 100);
  m_qlistviewScheduled->setColumnWidth(2, 120);
  m_qlistviewScheduled->setColumnWidth(3, 120);
  m_qlistviewScheduled->setColumnWidth(4, 120);
  m_qlistviewScheduled->setColumnWidth(5, 120);
  m_qlistviewScheduled->setColumnWidth(6, 120);
  m_qlistviewScheduled->setColumnWidth(1, m_qlistviewScheduled->width()-700);

  QWidget::show();
}

void KScheduledView::resizeEvent(QResizeEvent* e)
{
  m_qlistviewScheduled->setColumnWidth(0, 100);
  m_qlistviewScheduled->setColumnWidth(2, 120);
  m_qlistviewScheduled->setColumnWidth(3, 120);
  m_qlistviewScheduled->setColumnWidth(4, 120);
  m_qlistviewScheduled->setColumnWidth(5, 120);
  m_qlistviewScheduled->setColumnWidth(6, 120);
  m_qlistviewScheduled->setColumnWidth(1, m_qlistviewScheduled->width()-700);

  // call base class resizeEvent()
  kScheduledViewDecl::resizeEvent(e);
}

void KScheduledView::slotDeleteClicked()
{
}

void KScheduledView::slotSelectionChanged(QListViewItem* item)
{
}

void KScheduledView::slotEditClicked()
{
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
  KEditScheduledBillDlg *m_keditschedbilldlg = new KEditScheduledBillDlg(m_accountId, this);
  if (m_keditschedbilldlg->exec() == QDialog::Accepted)
  {
    MyMoneySchedule sched = m_keditschedbilldlg->schedule();

    QString s;
    s.sprintf("autoEnter: %d\n"
      "endDate: %s\n"
      "fixed: %d\n"
      "id: %s\n"
      "lastPayment: %s\n"
      "occurnce: %d\n"
      "paytype: %d\n"
      "startDtae: %s\n"
      "transaremainaing: %d\n"
      "type: %d\n"
      "willEnd: %d\n"
      "transactionSplitCount: %d\n",
      sched.autoEnter(),
      sched.endDate().toString().latin1(),
      sched.isFixed(),
      sched.id().latin1(),
      sched.lastPayment().toString().latin1(),
      sched.occurence(),
      sched.paymentType(),
      sched.startDate().toString().latin1(),
      sched.transactionsRemaining(),
      sched.type(),
      sched.willEnd(),
      sched.transaction().splitCount());

    KMessageBox::information(this, s);

//    MyMoneyScheduled *scheduled = MyMoneyFile::instance()->scheduledInstance();
    MyMoneyScheduled *scheduled = MyMoneyScheduled::instance();

    try
    {
      QString schedId = scheduled->addSchedule(m_accountId, sched);

      KMessageBox::information(this, schedId);
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
  KEditScheduledDepositDlg *m_keditscheddepdlg = new KEditScheduledDepositDlg(m_accountId, this);
  if (m_keditscheddepdlg->exec() == QDialog::Accepted)
  {
    MyMoneySchedule sched = m_keditscheddepdlg->schedule();

    QString s;
    s.sprintf("autoEnter: %d\n"
      "endDate: %s\n"
      "fixed: %d\n"
      "id: %s\n"
      "lastPayment: %s\n"
      "occurnce: %d\n"
      "paytype: %d\n"
      "startDtae: %s\n"
      "transaremainaing: %d\n"
      "type: %d\n"
      "willEnd: %d\n"
      "transactionSplitCount: %d\n",
      sched.autoEnter(),
      sched.endDate().toString().latin1(),
      sched.isFixed(),
      sched.id().latin1(),
      sched.lastPayment().toString().latin1(),
      sched.occurence(),
      sched.paymentType(),
      sched.startDate().toString().latin1(),
      sched.transactionsRemaining(),
      sched.type(),
      sched.willEnd(),
      sched.transaction().splitCount());

    KMessageBox::information(this, s);

//    MyMoneyScheduled *scheduled = MyMoneyFile::instance()->scheduledInstance();
    MyMoneyScheduled *scheduled = MyMoneyScheduled::instance();

    try
    {
      QString schedId = scheduled->addSchedule(m_accountId, sched);

      KMessageBox::information(this, schedId);
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
  KEditScheduledTransferDlg *m_keditschedtransdlg = new KEditScheduledTransferDlg(m_accountId, this);
  if (m_keditschedtransdlg->exec() == QDialog::Accepted)
  {
    MyMoneySchedule sched = m_keditschedtransdlg->schedule();

    QString s;
    s.sprintf("autoEnter: %d\n"
      "endDate: %s\n"
      "fixed: %d\n"
      "id: %s\n"
      "lastPayment: %s\n"
      "occurnce: %d\n"
      "paytype: %d\n"
      "startDtae: %s\n"
      "transaremainaing: %d\n"
      "type: %d\n"
      "willEnd: %d\n"
      "transactionSplitCount: %d\n",
      sched.autoEnter(),
      sched.endDate().toString().latin1(),
      sched.isFixed(),
      sched.id().latin1(),
      sched.lastPayment().toString().latin1(),
      sched.occurence(),
      sched.paymentType(),
      sched.startDate().toString().latin1(),
      sched.transactionsRemaining(),
      sched.type(),
      sched.willEnd(),
      sched.transaction().splitCount());

    KMessageBox::information(this, s);

//    MyMoneyScheduled *scheduled = MyMoneyFile::instance()->scheduledInstance();
    MyMoneyScheduled *scheduled = MyMoneyScheduled::instance();

    try
    {
      QString schedId = scheduled->addSchedule(m_accountId, sched);

      KMessageBox::information(this, schedId);
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
      return;
    }
  }
}
