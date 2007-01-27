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

KScheduledListItem::KScheduledListItem(KListView *parent, const QString& name)
 : KListViewItem(parent,name)/*, m_even(false)*/, m_base(true)
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
 : KListViewItem(parent), m_base(false)
{
  m_schedule = schedule;
  setPixmap(0, KMyMoneyUtils::scheduleIcon(KIcon::Small));

//  m_even = even;
  try
  {
    m_id = schedule.id();
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

  QColor colour = KMyMoneyGlobalSettings::listColor();
  QColor bgColour = KMyMoneyGlobalSettings::listBGColor();
  QColor textColour = KGlobalSettings::textColor();
  QFont cellFont = KMyMoneyGlobalSettings::listCellFont();

  // avoid colorizing lines that do not contain a schedule
  if(!m_schedule.id().isEmpty()) {
    if (m_schedule.isFinished())
      textColour = Qt::darkGreen;
    else if (m_schedule.isOverdue())
      textColour = Qt::red;
    // else
    //   keep the same colour
  }

  p->setFont(cellFont);
  cg2.setColor(QColorGroup::Text, textColour);

  if (m_base)
  {
    QFont font(p->font());
    font.setBold(true);
    p->setFont(font);
  }

  if (isAlternate())
  {
    cg2.setColor(QColorGroup::Base, colour);
  }
  else
  {
    cg2.setColor(QColorGroup::Base, bgColour);
  }

  QListViewItem::paintCell(p, cg2, column, width, align);

  if (column == 0)
  {
    int ts = listView()->treeStepSize();
    int indent = ts * (depth()+1);
    p->save();
    p->translate(-indent, 0);

    paintBranches(p, cg, 0, 0, 0);

    p->restore();
  }
}

void KScheduledListItem::paintBranches(QPainter* p, const QColorGroup& cg, int/* w*/, int/* y*/, int/* h*/)
// void KScheduledListItem::paintBranches(QPainter* p, const QColorGroup& cg, int w, int y, int h)
{
  // qDebug("paintBranches(%d,%d,%d)", w, y, h);
  QColorGroup cg2(cg);

  QColor colour = KMyMoneyGlobalSettings::listColor();
  QColor bgColour = KMyMoneyGlobalSettings::listBGColor();

  if (isAlternate())
  {
    cg2.setColor(QColorGroup::Base, colour);
  }
  else
  {
    cg2.setColor(QColorGroup::Base, bgColour);
  }

  int ts = listView()->treeStepSize();
  int ofs;
  int indent = ts * (depth()+1);

  if ( isSelected()) {
    p->fillRect( 0, 0, indent, height(), cg2.brush( QColorGroup::Highlight ) );
    if ( isEnabled() || !listView() )
      p->setPen( cg2.highlightedText() );
    else if ( !isEnabled() && listView())
      p->setPen( listView()->palette().disabled().highlightedText() );

  } else
    p->fillRect( 0, 0, indent, height(), cg2.base() );

  // draw dotted lines in upper levels to the left of us
  QListViewItem *parent = this;
  for(int j = depth()-1; j >= 0; --j) {
    if(!parent)
      break;
    parent = parent->parent();
    if(parent->nextSibling()) {
      ofs = (j * ts) + ts/2 - 1;
      for(int j = 0; j < height(); j += 2)
        p->drawPoint(ofs, j);
    }
  }

  if(childCount() == 0) {
    // if we have no children, the we need to draw a vertical line
    // which length depends if we have a sibling or not.
    // also a horizontal line to the right is required.
    ofs = depth()*ts + ts/2 - 1;
    int end = nextSibling() ? height() : height()/2;
    for(int i = 0; i < end; i += 2)
      p->drawPoint(ofs, i);

    for(int i = ofs; i < (depth()+1)*ts; i += 2)
      p->drawPoint(i, height()/2);

  } else {
    // draw upper part of vertical line
    ofs = depth()*ts + ts/2 - 1;
    for(int i = 0; i < height()/2-(ts-2)/4; i += 2)
      p->drawPoint(ofs, i);

    // draw horizontal part
    for(int i = ofs + ts/4 ; i < (depth()+1)*ts; i += 2)
      p->drawPoint(i, height()/2);

    // need to draw box with +/- in it
    ofs = depth() * ts;
    p->drawRect( ofs + ts/4, height() / 2 - (ts-2)/4, (ts-2)/2, (ts-2)/2 );
    p->drawLine( ofs + ts/2-3, height() / 2, ofs + ts/2+1, height() / 2 );
    if ( !isOpen() )
        p->drawLine( ofs + ts/2-1, height() / 2 - 2, ofs + ts/2-1, height() / 2 + 2 );

    // if there are more siblings, we need to draw
    // the remainder of the vertical line
    if(nextSibling()) {
      ofs = depth()*ts + ts/2 - 1;
      for(int i = height() / 2 + (ts-2)/4; i < height(); i += 2)
        p->drawPoint(ofs, i);
    }
  }
}

void KScheduledListItem::paintFocus(QPainter* p, const QColorGroup& cg, const QRect& r)
{
  QColorGroup cg2(cg);

  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  QColor textColour = cg2.highlightedText();
  textColour = config->readColorEntry("listGridColor", &textColour);

  if (m_schedule.isFinished())
    textColour = Qt::darkGreen;
  else if (m_schedule.isOverdue())
    textColour = Qt::red;

  cg2.setColor(QColorGroup::HighlightedText, textColour);

  int indent = listView()->treeStepSize() * (depth()+1);

  QRect r2(r);
  r2.setLeft(r2.left() + -indent);
  if (isSelected())
    p->fillRect(  r2.left(),
                  r2.top(),
                  -indent,
                  r2.height(),
                  cg2.highlight());

  listView()->style().drawPrimitive(
                QStyle::PE_FocusRect, p, r2, cg2,
                (isSelected() ? QStyle::Style_FocusAtBorder : QStyle::Style_Default),
                QStyleOption(isSelected() ? cg2.highlight() : cg2.base()));

}
