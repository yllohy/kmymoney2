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

// ----------------------------------------------------------------------------
// KDE Includes
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kscheduledview.h"
#include "kscheduledlistitem.h"
//#include "../dialogs/knewscheduleddlg.h"

KScheduledView::KScheduledView(MyMoneyFile *file, QWidget *parent, const char *name )
 : kScheduledViewDecl(parent,name, false)
{
  m_file = file;
  m_qlistviewScheduled->setRootIsDecorated(true);
  m_qlistviewScheduled->addColumn(i18n("Type"));
  m_qlistviewScheduled->addColumn(i18n("Payee"));
  m_qlistviewScheduled->addColumn(i18n("Amount"));
  m_qlistviewScheduled->addColumn(i18n("Next Due Date"));
  m_qlistviewScheduled->addColumn(i18n("Frequency"));
  m_qlistviewScheduled->addColumn(i18n("Payment Method"));
  m_qlistviewScheduled->setMultiSelection(false);
//  m_qlistviewScheduled->setColumnWidthMode(0, QListView::Manual);
  m_qlistviewScheduled->header()->setResizeEnabled(false);
  m_qlistviewScheduled->setAllColumnsShowFocus(true);

  // never show a horizontal scroll bar
  m_qlistviewScheduled->setHScrollBarMode(QScrollView::AlwaysOff);

  m_qlistviewScheduled->setSorting(-1);

  readConfig();

  connect(m_qlistviewScheduled, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSelectionChanged(QListViewItem*)));
  connect(m_qbuttonEdit, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
  connect(m_qbuttonNew, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
  connect(m_qbuttonDelete, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));
}

KScheduledView::~KScheduledView()
{
  writeConfig();
}

void KScheduledView::refresh(void)
{
  KScheduledListItem *itemBills = new KScheduledListItem(m_qlistviewScheduled, i18n("Bills"));
  KScheduledListItem *itemDeposits = new KScheduledListItem(m_qlistviewScheduled, i18n("Deposits"));
}

void KScheduledView::show()
{
  refresh();
  emit signalViewActivated();
  QWidget::show();
}

void KScheduledView::resizeEvent(QResizeEvent* e)
{
  m_qlistviewScheduled->setColumnWidth(0, 50);
  m_qlistviewScheduled->setColumnWidth(2, 120);
  m_qlistviewScheduled->setColumnWidth(3, 120);
  m_qlistviewScheduled->setColumnWidth(4, 120);
  m_qlistviewScheduled->setColumnWidth(5, 120);
  m_qlistviewScheduled->setColumnWidth(6, 120);
  m_qlistviewScheduled->setColumnWidth(1, m_qlistviewScheduled->width()-650);

  // call base class resizeEvent()
  kScheduledViewDecl::resizeEvent(e);
}

void KScheduledView::slotNewClicked()
{
/*
  KNewScheduledDlg *knewscheduleddlg = new KNewScheduledDlg(this);
  knewscheduleddlg->show();
*/
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
