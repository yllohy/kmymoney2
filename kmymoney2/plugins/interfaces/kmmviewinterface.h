/***************************************************************************
                          kmmviewinterface.h
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMMVIEWINTERFACE_H
#define KMMVIEWINTERFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

class KMyMoney2App;
class KMyMoneyView;
class KMyMoneyViewBase;

// ----------------------------------------------------------------------------
// Project Includes

#include "../viewinterface.h"

namespace KMyMoneyPlugin {

/**
  * This class represents the implementation of the
  * ViewInterface.
  */
class KMMViewInterface : public ViewInterface {
  Q_OBJECT

public:
  KMMViewInterface(KMyMoney2App* app, KMyMoneyView* view, QObject* parent, const char* name = 0);
  ~KMMViewInterface() {}

  /**
    * This method returns a pointer to a newly created view
    * with title @p item and icon @p pixmap.
    *
    * @param item Name of view
    * @param icon name for the icon to be used for the view
    *
    * @return pointer to KMyMoneyViewBase object
    */
  KMyMoneyViewBase* addPage(const QString& item, const QString& icon);

  /**
    * This method allows to add a widget to the view
    * created with addPage()
    *
    * @param view pointer to view object
    * @param w pointer to widget
    */
  void addWidget(KMyMoneyViewBase* view, QWidget* w);

private:
  KMyMoney2App*    m_app;
  KMyMoneyView*    m_view;
};

}; // namespace
#endif
