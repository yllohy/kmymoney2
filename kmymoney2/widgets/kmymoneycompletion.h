/***************************************************************************
                          kmymoneycompletion.h  -  description
                             -------------------
    begin                : Mon Apr 26 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#ifndef KMYMONEYCOMPLETION_H
#define KMYMONEYCOMPLETION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qvbox.h>
class KListView;
class QListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

// #include "kmymoneyaccountselector.h"

/**
  * @author Thomas Baumgart
  */

class kMyMoneyCompletion : public QVBox
{
  Q_OBJECT
public:

  kMyMoneyCompletion(QWidget *parent=0, const char *name=0);
  virtual ~kMyMoneyCompletion();

  /**
    * Re-implemented for internal reasons.  API is unaffected.
    */
  virtual void show();

  /**
    * Re-implemented for internal reasons.  API is unaffected.
    */
  virtual void hide();

  virtual const int loadList(void) = 0;

  /**
    * This method sets the current account with id @p id as
    * the current selection.
    *
    * @param id id of account to be selected
    */
  void setSelected(const QCString& id) { m_id = id; };

public slots:
  void slotMakeCompletion(const QString& txt);

  void slotItemSelected(QListViewItem *item, const QPoint& pos, int col);

protected:
  /**
    * Reimplemented from kMyMoneyAccountSelector to get events from the viewport (to hide
    * this widget on mouse-click, Escape-presses, etc.
    */
  virtual bool eventFilter( QObject *, QEvent * );

  /**
    * This method resizes the widget to show a maximum of @p count lines
    */
  void adjustSize(const int count);

  void connectSignals(QWidget *widget, KListView* lv);

signals:
  void itemSelected(const QCString& id);

protected:
  QWidget*                    m_parent;
  QWidget*                    m_widget;
  QCString                    m_id;
  KListView*                  m_lv;
};

#endif
