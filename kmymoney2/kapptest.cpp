/***************************************************************************
                          kmymoneyutils.cpp  -  description
                             -------------------
    begin                : Wed Feb 5 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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

#include <kdecompat.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qapplication.h>
#include <qwidgetlist.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kpushbutton.h>
#include <kmainwindow.h>

#if KDE_IS_VERSION(3,2,0)
  #include <kactioncollection.h>
#else
  #include <kaction.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kapptest.h"

// definitions copied from X11/X.h
#define XKeyPress 2
#define XKeyRelease 3

typedef int (*QX11EventFilter)(XEvent*);
extern QX11EventFilter qt_set_x11_event_filter (QX11EventFilter filter);

class KAppTestPrivate {
public:
  KAppTestPrivate() {};
  ~KAppTestPrivate() {};

  QMap<QString, QWidget*>    m_widgetMap;
  int                        (*m_oldFilter)(XEvent* e);
};

static int x11EventFilter(XEvent* e)
{
  e = e;
/*
  if(e->type == XKeyPress || e->type == XKeyRelease) {
    qDebug("type   : %d", e->xkey.type);
    qDebug(" key   : %d", e->xkey.keycode);
    qDebug(" state : %d", e->xkey.state);
    qDebug(" window: %d", e->xkey.window);
  }
*/
  return false;
}


KAppTest::KAppTest() :
  QObject(0, 0),
  DCOPObject("AppTest")
{
  d = new KAppTestPrivate();
  d->m_oldFilter = qt_set_x11_event_filter(x11EventFilter);
}

KAppTest::~KAppTest()
{
  qt_set_x11_event_filter(d->m_oldFilter);
  delete d;
}

const char* KAppTest::widgetName(QObject *w, const char* name)
{
  static QString fullName;

  fullName = QString(name);

  for(;w; w = w->parent()) {
    QString parent(w->name());
    if(!parent.isEmpty()) {
      fullName = parent + "/" + fullName;
      break;
    }
  }
  return fullName.data();
}

void KAppTest::enterString(const QString& txt)
{
  qDebug("enterString '%s'", txt.data());
  QWidget* kw = qApp->focusWidget();
  QWidget* ka = qApp->activeWindow();
  qDebug("focusWidget '%p'", (void *)kw);
  qDebug("activeWindow '%p'", (void *)ka);
  if(kw) {
    for(unsigned i = 0; i < txt.length(); ++i) {
      QChar k = txt[i];
      QKeyEvent kp(QEvent::KeyPress, k, k, 0, k);
      QKeyEvent kr(QEvent::KeyRelease, k, k, 0, k);

      QApplication::sendEvent(kw, &kp);
      qApp->processEvents(1);

      QApplication::sendEvent(kw, &kr);
      qApp->processEvents(1);
    }
  }
}

void KAppTest::updateWidgetMap(void)
{
  d->m_widgetMap.clear();

  QWidgetList  *list = QApplication::allWidgets();
  QWidgetListIt it( *list );         // iterate over the widgets
  QWidget * w;
  while ( (w=it.current()) != 0 ) {  // for each widget...
    if(w->isVisible()) {
      d->m_widgetMap[w->name()] = w;
    }
    ++it;
  }
  delete list;                      // delete the list, not the widgets

}

const QStringList KAppTest::widgetList(void)
{
  QStringList wlist;
  QMap<QString, QWidget*>::ConstIterator it;

  updateWidgetMap();
  for(it = d->m_widgetMap.begin(); it != d->m_widgetMap.end(); ++it) {
    wlist << it.key();
  }
  return wlist;
}

const int KAppTest::setFocus(const QString& widget)
{
  updateWidgetMap();

  qApp->mainWidget()->setActiveWindow();
  if(widget.isEmpty())
    return 0;

  QMap<QString, QWidget*>::Iterator it;

  it = d->m_widgetMap.find(widget);
  if(it == d->m_widgetMap.end())
    return -1;

  (*it)->setFocus();
  return 0;
}

const int KAppTest::animateClick(const QString& widget)
{
  updateWidgetMap();

  QMap<QString, QWidget*>::Iterator it;

  it = d->m_widgetMap.find(widget);
  if(it == d->m_widgetMap.end())
    return -1;

  if(!(*it)->inherits("QPushButton"))
    return -2;

  QPushButton* button = static_cast<QPushButton *>(*it);
  button->animateClick();
  return 0;
}

ASYNC KAppTest::actionActivate(const QString& actionName)
{
  QWidget* qmw = qApp->mainWidget();
  if(qmw && qmw->inherits("KMainWindow")) {
    KMainWindow* kmw = static_cast<KMainWindow*>(qmw);
    KActionCollection* collection = kmw->actionCollection();
    if(!collection)
      return;

    KAction* action = collection->action(actionName);
    if(!action)
      return;

    action->activate();
  }
  return;
}

