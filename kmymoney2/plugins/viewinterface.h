/***************************************************************************
                          viewinterface.h
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

#ifndef VIEWINTERFACE_H
#define VIEWINTERFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qobject.h>
#include <qstring.h>
#include <qpixmap.h>
class QFrame;

// ----------------------------------------------------------------------------
// KDE Includes

class KPopupMenu;

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyaccount.h"

namespace KMyMoneyPlugin {

/**
  * This abstract class represents the ViewInterface to
  * add new view pages to the JanusWidget of KMyMoney. It
  * also gives access to the account context menu.
  */
class ViewInterface : public QObject {
  Q_OBJECT

public:
  ViewInterface(QObject* parent, const char* name = 0);
  ~ViewInterface() {};

  /**
    * This method returns a pointer to the account context menu
    * which is opened when right clicking on an account.
    *
    * @return pointer to KPopupMenu
    */
  virtual KPopupMenu*   accountContextMenu() = 0;

  /**
    * This method creates a new page in the application.
    * See KJanusWidget::addPage() for details.
    */
  virtual QFrame* addPage(const QString& item, const QString& header, const QPixmap& pixmap) = 0;

signals:
  void accountSelectedForContextMenu(const MyMoneyAccount& acc);
  void viewStateChanged(bool);
};

}; // namespace
#endif
