/***************************************************************************
                          kmymoneyhlayout.cpp  -  description
                             -------------------
    begin                : Sun Jun 24 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@users.sourceforge.net>
                             Felix Rodriguez <frodriguez@users.sourceforge.net>
                             John C <thetacoturtle@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "kmymoneyhlayout.h"

kMyMoneyHLayout::kMyMoneyHLayout(QWidget* parent):QWidget(parent)
{
  m_HBoxLayout =  new QHBoxLayout(parent);
}

kMyMoneyHLayout::~kMyMoneyHLayout(){
}

/** No descriptions */
void kMyMoneyHLayout::show()
{
	// QWidget::hide();
  for(unsigned i = 0; i < m_widgets.count(); i++)	{
    m_widgets.at(i)->move(m_widgetx + (m_widgetwidth * i),m_widgety);		
    m_widgets.at(i)->show();
  }
}

/** No descriptions */
void kMyMoneyHLayout::hide()
{
  QWidget::hide();
	for(unsigned i = 0; i < m_widgets.count(); i++)	{
    m_widgets.at(i)->hide();
  }
}

/** No descriptions */
void kMyMoneyHLayout::addWidget(QWidget *w)
{
  m_HBoxLayout->addWidget(w);
  m_widgets.append(w);
}
/** No descriptions */
void kMyMoneyHLayout::setGeometry(const QRect& rect)
{
  QRect myrect = m_HBoxLayout->geometry();
	myrect.setWidth(rect.width());
	myrect.setHeight(rect.height());
	myrect.setX(m_widgetx);
	myrect.setY(m_widgety);
	m_HBoxLayout->setGeometry(myrect);
	m_widgetwidth = rect.width() / m_widgets.count();
	m_widgetheight = rect.height();
	for(unsigned i = 0; i < m_widgets.count(); i++) {
    m_widgets.at(i)->setFixedHeight(m_widgetheight);
    m_widgets.at(i)->setFixedWidth(m_widgetwidth);
		//m_widgets.at(i)->move(rect.x() + (m_widgetwidth * i),rect.y());		
  }

}

/** No descriptions */
void kMyMoneyHLayout::reparent(QWidget* parent, WFlags f, const QPoint& p,bool showIt)
{
  for(unsigned i = 0; i < m_widgets.count(); i++)	{
    m_widgets.at(i)->reparent(parent,f,p,showIt);
  }
}

/** No descriptions */
void kMyMoneyHLayout::move(int x, int y)
{
  QRect myrect =  m_HBoxLayout->geometry();
  myrect.setX(x);
  myrect.setY(y);
  m_widgetx = x;
  m_widgety = y;
  m_HBoxLayout->setGeometry(myrect);
	for(unsigned i = 0; i < m_widgets.count(); i++)	{
    m_widgets.at(i)->move(x + (m_widgetwidth * i),y);		
  }
}

/** No descriptions */
void kMyMoneyHLayout::move(const QPoint& p)
{
  QRect myrect =  m_HBoxLayout->geometry();
  myrect.setX(p.x());
  myrect.setY(p.y());
  m_HBoxLayout->setGeometry(myrect);
}

/** No descriptions */
void kMyMoneyHLayout::setMaximumWidth(int maxw)
{
  QRect myrect =  m_HBoxLayout->geometry();
  myrect.setWidth(maxw);
  m_HBoxLayout->setGeometry(myrect);
}

/** No descriptions */
void kMyMoneyHLayout::setMaximumHeight(int maxh)
{
  QRect myrect =  m_HBoxLayout->geometry();
  myrect.setHeight(maxh);
  m_HBoxLayout->setGeometry(myrect);
}

/** No descriptions */
void kMyMoneyHLayout::setMaximumSize(int maxw, int maxh)
{
  QRect myrect =  m_HBoxLayout->geometry();
  myrect.setWidth(maxw);
  myrect.setHeight(maxh);
  m_HBoxLayout->setGeometry(myrect);

}

/** No descriptions */
void kMyMoneyHLayout::setMinimumSize(int minw, int minh)
{
  QRect myrect =  m_HBoxLayout->geometry();
  myrect.setWidth(minw);
  myrect.setHeight(minh);
  m_HBoxLayout->setGeometry(myrect);
}

/** No descriptions */
void kMyMoneyHLayout::setFixedSize(int w, int h)
{
  QRect myrect =  m_HBoxLayout->geometry();
  myrect.setWidth(w);
  myrect.setHeight(h);
  m_HBoxLayout->setGeometry(myrect);
}

/** No descriptions */
void kMyMoneyHLayout::setFixedWidth(int w)
{
  QRect myrect =  m_HBoxLayout->geometry();
  myrect.setWidth(w);
  m_HBoxLayout->setGeometry(myrect);
}

/** No descriptions */
void kMyMoneyHLayout::setFixedHeight(int h)
{
  QRect myrect =  m_HBoxLayout->geometry();
  myrect.setHeight(h);
  m_HBoxLayout->setGeometry(myrect);
}

/** No descriptions */
void kMyMoneyHLayout::setFixedSize(const QSize& size)
{
  QRect myrect =  m_HBoxLayout->geometry();
  myrect.setHeight(size.height());
  myrect.setWidth(size.width());
  m_HBoxLayout->setGeometry(myrect);
}

/** No descriptions */
void kMyMoneyHLayout::paintEvent(QPaintEvent *p)
{
  for(unsigned i = 0; i < m_widgets.count(); i++) {
    m_widgets.at(i)->repaint();
  }
}
