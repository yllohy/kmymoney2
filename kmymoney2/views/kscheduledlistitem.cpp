/***************************************************************************
                          kscheduledlistitem.cpp  -  description
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
#include <qpainter.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include "kconfig.h"
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kscheduledlistitem.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneyutils.h"

KScheduledListItem::KScheduledListItem(QListView *parent, const char *name)
 : QListViewItem(parent,name), m_even(false), m_base(true)
{
  setText(0, name);
  if (name == i18n("Bills"))
    setPixmap(0, KMyMoneyUtils::billScheduleIcon(KIcon::Small));
  else if (name == i18n("Deposits"))
    setPixmap(0, KMyMoneyUtils::depositScheduleIcon(KIcon::Small));
  else if (name == i18n("Transfers"))
    setPixmap(0, KMyMoneyUtils::transferScheduleIcon(KIcon::Small));
}

KScheduledListItem::KScheduledListItem(KScheduledListItem *parent, const MyMoneySchedule& schedule, bool even)
 : QListViewItem(parent), m_base(false)
{
  m_schedule = schedule;
  setPixmap(0, KMyMoneyUtils::scheduleIcon(KIcon::Small));

  m_even = even;
  try
  {
    QCString accountId = schedule.account().id();
    m_id = schedule.id();
    MyMoneyTransaction transaction = schedule.transaction();
    setText(0, schedule.name());
    setText(1, schedule.account().name());
    setText(2, MyMoneyFile::instance()->payee(transaction.splitByAccount(accountId).payeeId()).name());
    MyMoneyMoney amount = transaction.splitByAccount(accountId).value();
    if (amount < 0)
      amount = -amount;
    setText(3, amount.formatMoney());
    // Do the real next payment like ms-money etc
    setText(4, schedule.nextPayment(schedule.lastPayment()).toString());
    setText(5, KMyMoneyUtils::occurenceToString(schedule.occurence()));
    setText(6, KMyMoneyUtils::paymentMethodToString(schedule.paymentType()));
  }
  catch (MyMoneyException *e)
  {
    setText(0, "Error:");
    setText(1, e->what());
    delete e;
  }
}

KScheduledListItem::~KScheduledListItem()
{
}

void KScheduledListItem::paintCell(QPainter* p, const QColorGroup& cg, int column, int width, int align)
{
  QColorGroup cg2(cg);
  
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  QColor colour = Qt::white;
  QColor bgColour = QColor(224, 253, 182); // Same as for home view

  QColor textColour;

  if (m_schedule.isFinished())
  {
    textColour = Qt::darkGreen;
  }
  else if (m_schedule.isOverdue())
  {
    textColour = Qt::red;
  }
  else
  {
    textColour = Qt::black;
  }
    
  QFont cellFont(p->font());
  QColor baseItemColour = QColor(219, 237, 237);  // Same as for home view
  QColor baseItemTextColour = Qt::black;
/*
  bgColour = config->readColorEntry("listBGColor", &bgColour);
  colour = config->readColorEntry("listColor", &colour);
  textColour = config->readColorEntry("listGridColor", &textColour);
  cellFont = config->readFontEntry("listCellFont", &cellFont);
  baseItemColour = config->readColorEntry("BaseListItemColor", &baseItemColour);
  baseItemTextColour = config->readColorEntry("BaseListItemTextColor", &baseItemTextColour);
*/
  p->setFont(cellFont);
  cg2.setColor(QColorGroup::Text, textColour);

  if (m_base)
  {
    cg2.setColor(QColorGroup::Base, baseItemColour);
    QFont font(p->font());
//    font.setPointSize(font.pointSize()+1);
    font.setBold(true);
    p->setFont(font);
    cg2.setColor(QColorGroup::Text, baseItemTextColour);
  }
  else
  {
    if (m_even)
      cg2.setColor(QColorGroup::Base, bgColour);
    else
      cg2.setColor(QColorGroup::Base, colour);
  }
  
  QListViewItem::paintCell(p, cg2, column, width, align);
}
