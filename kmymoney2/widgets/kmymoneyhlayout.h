/***************************************************************************
                          kmymoneyhlayout.h  -  description
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

#ifndef KMYMONEYHLAYOUT_H
#define KMYMONEYHLAYOUT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qlayout.h>
#include <qptrlist.h>

/**
  *******************************
   Class     : kMyMoneyHLayout
   Purpose  : Horizontal Layout Widget
  *******************************
  *@author Michael Edwardes
  */

class kMyMoneyHLayout : public QWidget  {
	Q_OBJECT
public: 
	kMyMoneyHLayout(QWidget* parent);
	~kMyMoneyHLayout();
public slots: // Public slots
  /** No descriptions */
  virtual void show();
  /** No descriptions */
  virtual void hide();
  /** No descriptions */
  void addWidget(QWidget *w);
  /** No descriptions */
  void move(const QPoint& p);
  /** No descriptions */
  virtual void move(int x, int y);
  /** No descriptions */
  virtual void setGeometry(const QRect& rect);
  /** No descriptions */
  virtual void reparent(QWidget* parent, WFlags f, const QPoint& p,bool showIt);
  /** No descriptions */
  virtual void setMinimumSize(int minw, int minh);
  /** No descriptions */
  virtual void setMaximumSize(int maxw, int maxh);
  /** No descriptions */
  void setFixedSize(const QSize& size);
  /** No descriptions */
  void setFixedHeight(int h);
  /** No descriptions */
  void setFixedWidth(int w);
  /** No descriptions */
  void setFixedSize(int w, int h);
  /** No descriptions */
  void setMaximumHeight(int maxh);
  /** No descriptions */
  void setMaximumWidth(int maxw);
private: // Private attributes
  /**  */
  QBoxLayout *m_HBoxLayout;
  QList<QWidget> m_widgets;
  /**  */
  int m_widgetwidth;
  /**  */
  int m_widgetheight;
  /**  */
  int m_widgetx;
  /**  */
  int m_widgety;
protected: // Protected methods
  /** No descriptions */
  virtual void paintEvent(QPaintEvent *p);
};

#endif
