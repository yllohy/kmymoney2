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
#include <qstyle.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kconfig.h>
#include <klocale.h>
#include <kglobalsettings.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kscheduledlistitem.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneyglobalsettings.h"
#include "../kmymoneyutils.h"

KScheduledListItem::KScheduledListItem(KListView *parent, const QString& name) :
  KListViewItem(parent,name)
{
  if (name == i18n("Bills"))
    setPixmap(0, KMyMoneyUtils::billScheduleIcon(KIcon::Small));
  else if (name == i18n("Deposits"))
    setPixmap(0, KMyMoneyUtils::depositScheduleIcon(KIcon::Small));
  else if (name == i18n("Transfers"))
    setPixmap(0, KMyMoneyUtils::transferScheduleIcon(KIcon::Small));
  else if (name == i18n("Loans"))
    setPixmap(0, KMyMoneyUtils::transferScheduleIcon(KIcon::Small));
}

KScheduledListItem::KScheduledListItem(KScheduledListItem *parent, const MyMoneySchedule& schedule/*, bool even*/)
 : KListViewItem(parent)
{
  m_schedule = schedule;
  setPixmap(0, KMyMoneyUtils::scheduleIcon(KIcon::Small));

  try
  {
    MyMoneyTransaction transaction = schedule.transaction();
    MyMoneySplit s1 = transaction.splits()[0];
    MyMoneySplit s2 = transaction.splits()[1];
    QValueList<MyMoneySplit>::ConstIterator it_s;
    MyMoneySplit split;
    MyMoneyAccount acc;

    switch(schedule.type()) {
      case MyMoneySchedule::TYPE_DEPOSIT:
        if (!s1.value().isNegative())
          split = s1;
        else
          split = s2;
        break;

      case MyMoneySchedule::TYPE_LOANPAYMENT:
        for(it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
          acc = MyMoneyFile::instance()->account((*it_s).accountId());
          if(acc.accountGroup() == MyMoneyAccount::Asset
          || acc.accountGroup() == MyMoneyAccount::Liability) {
            if(acc.accountType() != MyMoneyAccount::Loan
            && acc.accountType() != MyMoneyAccount::AssetLoan) {
              split = *it_s;
              break;
            }
          }
        }
        if(it_s == transaction.splits().end()) {
          qFatal("Split for payment account not found in %s:%d.", __FILE__, __LINE__);
        }
        break;

      default:
        if (s1.value().isNegative())
          split = s1;
        else
          split = s2;
        break;
    }
    acc = MyMoneyFile::instance()->account(split.accountId());

/*
    if (schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
    {
      if (s1.value() >= 0)
        split = s1;
      else
        split = s2;
    }
    else if(schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT)
    {

    }
    else
    {
      if (s1.value() < 0)
        split = s1;
      else
        split = s2;
    }
*/
    setText(0, schedule.name());
    MyMoneySecurity currency = MyMoneyFile::instance()->currency(acc.currencyId());

    setText(1, acc.name());
    if(!split.payeeId().isEmpty())
      setText(2, MyMoneyFile::instance()->payee(split.payeeId()).name());
    else
      setText(2, "---");
    MyMoneyMoney amount = split.value();
    amount = amount.abs();
    setText(3, amount.formatMoney(currency.tradingSymbol()));
    // Do the real next payment like ms-money etc
    if (schedule.isFinished())
    {
      setText(4, i18n("Finished"));
    }
    else
      setText(4, schedule.nextDueDate().toString());

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

  QColor textColour = KGlobalSettings::textColor();
  QFont cellFont = KMyMoneyGlobalSettings::listCellFont();

  // avoid colorizing lines that do not contain a schedule
  if(!m_schedule.id().isEmpty()) {
    if (m_schedule.isFinished())
      textColour = Qt::darkGreen;
    else if (m_schedule.isOverdue())
      textColour = Qt::red;
  }

  cg2.setColor(QColorGroup::Text, textColour);

  // display group items in bold
  if (!parent())
    cellFont.setBold(true);

  p->setFont(cellFont);

  if (isAlternate())
    cg2.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listColor());
  else
    cg2.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::listBGColor());

  QListViewItem::paintCell(p, cg2, column, width, align);
}
