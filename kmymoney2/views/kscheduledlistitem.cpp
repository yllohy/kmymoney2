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
#include "kconfig.h"
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kscheduledlistitem.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneyutils.h"

KScheduledListItem::KScheduledListItem(KListView *parent, const char *name)
 : KListViewItem(parent,name)/*, m_even(false)*/, m_base(true)
{
  setText(0, name);
  if (name == i18n("Bills"))
    setPixmap(0, KMyMoneyUtils::billScheduleIcon(KIcon::Small));
  else if (name == i18n("Deposits"))
    setPixmap(0, KMyMoneyUtils::depositScheduleIcon(KIcon::Small));
  else if (name == i18n("Transfers"))
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
    MyMoneySplit split;

    if (schedule.type() == MyMoneySchedule::TYPE_DEPOSIT)
    {
      if (s1.value() >= 0)
        split = s1;
      else
        split = s2;
    }
    else
    {
      if (s1.value() < 0)
        split = s1;
      else
        split = s2;
    }
    
    
    setText(0, schedule.name());
    setText(1, MyMoneyFile::instance()->account(split.accountId()).name());
    setText(2, MyMoneyFile::instance()->payee(split.payeeId()).name());
    MyMoneyMoney amount = split.value();
    if (amount < 0)
      amount = -amount;
    setText(3, amount.formatMoney());
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
  
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  QColor colour = Qt::white;
  QColor bgColour = QColor(224, 253, 182); // Same as for home view
  QColor textColour;
  QFont cellFont(p->font());

  bgColour = config->readColorEntry("listBGColor", &bgColour);
  colour = config->readColorEntry("listColor", &colour);
  textColour = config->readColorEntry("listGridColor", &textColour);
  cellFont = config->readFontEntry("listCellFont", &cellFont);

  if (m_schedule.isFinished())
  {
    textColour = Qt::darkGreen;
  }
  else if (m_schedule.isOverdue())
  {
    textColour = Qt::red;
  }
  // else
  //   keep the same colour

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

  int indent = 0;
  if (column == 0)
  {
    int ts = listView()->treeStepSize();
    int ofs;
    indent = ts * (depth()+1);
    p->save();
    p->translate(-indent, 0);

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

    p->restore();
  }
}

void KScheduledListItem::paintBranches(QPainter*/* p*/, const QColorGroup&/* cg*/, int/* w*/, int/* y*/, int/* h*/)
{
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
