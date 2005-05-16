/***************************************************************************
                          kmymoneycompletion.cpp  -  description
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycompletion.h"
#include "kmymoneyaccountselector.h"    // for kMyMoneyListViewItem

#define MAX_ITEMS   16

kMyMoneyCompletion::kMyMoneyCompletion(QWidget *parent, const char *name ) :
  QVBox(parent, name, WType_Popup)
{
  m_parent = parent;
  setFocusProxy((parent) ? parent : (QWidget*) NoFocus);
  setFrameStyle(QFrame::PopupPanel | QFrame::Raised);
  m_widget = 0;
  m_lv = 0;
}

void kMyMoneyCompletion::connectSignals(QWidget* widget, KListView* lv)
{
  m_widget = widget;
  m_lv = lv;
  connect(lv, SIGNAL(executed(QListViewItem*,const QPoint&,int)), this, SLOT(slotItemSelected(QListViewItem*,const QPoint&,int)));
}

kMyMoneyCompletion::~kMyMoneyCompletion()
{
}

void kMyMoneyCompletion::adjustSize(const int count)
{
  int w = m_widget->sizeHint().width();
  if(m_parent && w < m_parent->width())
    w = m_parent->width();

  QFontMetrics fm(font());
  if(w < fm.maxWidth()*15)
    w = fm.maxWidth()*15;

  QListViewItem* item = m_lv->firstChild();
  int h = item->height() * (count > MAX_ITEMS ? MAX_ITEMS : count);

  // the offset of 4 in the next statement avoids the
  // display of a scroll bar if count < MAX_ITEMS.
  resize(w, h+4);

  if(m_parent) {
    // the code of this basic block is taken from KCompletionBox::show()
    // and modified to our local needs

    // this is probably better, once kde switches to requiring qt3.1
    // QRect screenSize = QApplication::desktop()->availableGeometry(d->m_parent);
    // for now use this since it's qt3.0.x-safe
    QRect screenSize = QApplication::desktop()->screenGeometry(QApplication::desktop()->screenNumber(m_parent));

    QPoint orig = m_parent->mapToGlobal( QPoint(0, m_parent->height()) );
    int x = orig.x();
    int y = orig.y();

    if ( x + width() > screenSize.right() )
        x = screenSize.right() - width();

    // check for the maximum height here to avoid flipping
    // of the completion box from top to bottom of the
    // edit widget. The offset (y) is certainly based
    // on the actual height.
    if ((y + item->height()*MAX_ITEMS) > screenSize.bottom() )
        y = y - height() - m_parent->height();

    move( x, y);
  }
}

void kMyMoneyCompletion::show(void)
{
  if(m_parent)
    qApp->installEventFilter(this);

  QVBox::show();
}

void kMyMoneyCompletion::hide(void)
{
  if(m_parent)
    qApp->removeEventFilter(this);
  QVBox::hide();
}

bool kMyMoneyCompletion::eventFilter(QObject* o, QEvent* e)
{
  int type = e->type();

  if(o == m_parent) {
    if(isVisible()) {
      if(type == QEvent::KeyPress) {
        QKeyEvent* ev = static_cast<QKeyEvent*> (e);
        QKeyEvent evt(QEvent::KeyPress,
                      Key_Down, 0, ev->state(), QString::null,
                      ev->isAutoRepeat(), ev->count());
        QKeyEvent evbt(QEvent::KeyPress,
                      Key_Up, 0, ev->state(), QString::null,
                      ev->isAutoRepeat(), ev->count());

        switch(ev->key()) {
          case Key_Tab:
            return QApplication::sendEvent(m_lv, &evt);

          case Key_BackTab:
            return QApplication::sendEvent(m_lv, &evbt);

          case Key_Up:
          case Key_Down:
          case Key_Prior:
          case Key_Next:
            return QApplication::sendEvent(m_lv, e);

          case Key_Escape:
            hide();
            ev->accept();
            return true;

          case Key_Enter:
          case Key_Return:
            slotItemSelected(m_lv->currentItem(), QPoint(0,0), 0);
            ev->accept();
            return true;

          case Key_Home:
          case Key_End:
            if(ev->state() & ControlButton) {
              return QApplication::sendEvent(m_lv, e);
            }
            // tricky fall through here

          default:
            break;

        }
      }
    }

  } else if(type == QEvent::MouseButtonPress) {
    // any mouse click on something else than "this" makes us hide
    QMouseEvent* ev = static_cast<QMouseEvent*> (e);
    if(!rect().contains(ev->pos()))
      hide();
  }
  return QVBox::eventFilter(o, e);
}

void kMyMoneyCompletion::slotMakeCompletion(const QString& /* txt */)
{
  qFatal("slotMakeCompletion(const QString& txt) not overridden by %s", className());
}

void kMyMoneyCompletion::slotItemSelected(QListViewItem *item, const QPoint&, int)
{
  kMyMoneyListViewItem* it_v = static_cast<kMyMoneyListViewItem*>(item);
  if(it_v && it_v->isSelectable()) {
    QCString id = it_v->id();
    // hide the widget, so we can debug the slots that are connect
    // to the signal we emit very soon
    hide();
    m_id = id;
    emit itemSelected(id);
  }
}

#include "kmymoneycompletion.moc"
