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

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneyinstitution.h>
#include <kmymoney/export.h>
class KMyMoneyViewBase;

namespace KMyMoneyPlugin {

/**
  * This abstract class represents the ViewInterface to
  * add new view pages to the JanusWidget of KMyMoney. It
  * also gives access to the account context menu.
  */
class KMYMONEY_EXPORT ViewInterface : public QObject {
  Q_OBJECT

public:
  ViewInterface(QObject* parent, const char* name = 0);
  ~ViewInterface() {};

  /**
    * This method creates a new page in the application.
    * See KJanusWidget::addPage() for details.
    */
  virtual KMyMoneyViewBase* addPage(const QString& item, const QPixmap& pixmap) = 0;

  /**
    * This method adds a widget to the layout of the view
    * created with addPage()
    *
    * @param view pointer to view widget
    * @param w widget to be added to @p page
    */
  virtual void addWidget(KMyMoneyViewBase* view, QWidget* w) = 0;

signals:
  void accountSelected(const MyMoneyAccount& acc);
  void institutionSelected(const MyMoneyInstitution& institution);
  void viewStateChanged(bool);
  void kmmFilePlugin(unsigned int);
};

}; // namespace
#endif
