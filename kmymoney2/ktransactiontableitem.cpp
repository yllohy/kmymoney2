/***************************************************************************
                          ktransactiontableitem.cpp  -  description
                             -------------------
    begin                : Wed Apr 18 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kglobal.h>
#include <kconfig.h>
#include "ktransactiontableitem.h"

KTransactionTableItem::KTransactionTableItem(QTable *t, EditType et, const QString &txt, int items = 1)
  : QTableItem(t, et, txt)
{
	m_numitems = items;

  setReplaceable(false);
}

void KTransactionTableItem::paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  QFont defaultFont = QFont("helvetica", 12);
  QColor defaultColor = Qt::white;
  QColor defaultBGColor = Qt::gray;
	
  QColorGroup g(cg);
  if ((row()%2))
    g.setColor(QColorGroup::Base, config->readColorEntry("listBGColor", &defaultBGColor));
  else
    g.setColor(QColorGroup::Base, config->readColorEntry("listColor", &defaultColor));

  p->setFont(config->readFontEntry("listCellFont", &defaultFont));
	
  QString firsttext = text();
	QString secondtext = m_item2;
	QRect myrect = cr;
	myrect.setX(0);
	myrect.setY(0);

	if(m_numitems > 1)
	{
		QRect arect = table()->cellGeometry(row(),col());
		QBrush backgroundBrush(g.base());
		p->setPen(g.foreground());
		p->fillRect(myrect,backgroundBrush);
		int newwidth = arect.width() / 2;
		//myrect.setWidth(newwidth);
		p->drawText(myrect,Qt::AlignLeft,firsttext);
		myrect.setX(newwidth);
		myrect.setWidth(newwidth);
		p->fillRect(myrect,backgroundBrush);
		p->drawLine(newwidth, 0,newwidth,myrect.height());
		myrect.setX(newwidth + 1);
		p->setPen(g.foreground());
		p->drawText(myrect,Qt::AlignLeft,secondtext);
    }
	else
	{
		QBrush backgroundBrush(g.base());
		p->setPen(g.foreground());
		p->fillRect(myrect,backgroundBrush);
		p->drawText(myrect,Qt::AlignLeft,firsttext);
	}

}
/** No descriptions */
void KTransactionTableItem::setContentFromEditor(QWidget* w){

}
/** No descriptions */
void KTransactionTableItem::setItem2(const QString& item){

	m_item2 = item;
}
