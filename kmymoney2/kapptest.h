/***************************************************************************
                             kapptest.h  -  description
                             -------------------
    begin                : Tue Jun 29 2004
    copyright            : (C) 2004 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KAPPTEST_H
#define KAPPTEST_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qapplication.h>
#include <qmap.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <dcopobject.h>

// ----------------------------------------------------------------------------
// Project Includes

#ifndef KDE_USE_FINAL
  #define CREATE_TEST_CONTAINER() KAppTest* KAppTest_testContainer = new KAppTest()
  #define DESTROY_TEST_CONTAINER() delete KAppTest_testContainer;

#else
  #define CREATE_TEST_CONTAINER()
  #define DESTROY_TEST_CONTAINER()
#endif

/**
  * @author Thomas Baumgart
  */

class KAppTestPrivate;

class KAppTest : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP

public:
  KAppTest();
  virtual ~KAppTest();

  /**
    * This function returns a slash separated path of the
    * QObject class hierarchy. Example: A child widget named "b"
    * should be created for a top level widget named "a" (pointed to by @p this).
    * The following code assigns b the name "a/b":
    *
    * @code
    *
    * QWidget *n = new KWidget(this, KMyMoneyUtils::widgetName(this, "b"));
    *
    * @endcode
    *
    * @param w pointer to parent object
    * @param name pointer to name to be assigned to object
    * @return pointer to path of named
    *
    * @note the return value will only be usable until the next call to
    *       this function
    */
  static const char* widgetName(QObject* w, const char* name);

k_dcop:
  void enterString(const QString& txt);
  const QStringList widgetList();
  const int setFocus(const QString& widget);

  /**
    * This test method allows to click a button. The button
    * with the name as passed in @p widget must be of type
    * QPushButton or KPushButton.
    *
    * @param widget name of button widget
    *
    * @retval 0 button was clicked
    * @retval -1 widget with that name is not available/visible
    * @retval -2 widget with that name is not of correct type
    */
  const int animateClick(const QString& widget);

  /**
    * This test method allows to activate a menu action. The
    * name of the action is passed in argument @p actionName.
    *
    * @param actionName name of the action
    *
    * @retval 0 action was activated
    * @retval -1 actionCollection not available
    * @retval -2 no action with name @p actionName found
    */
  ASYNC actionActivate(const QString& actionName);

private:
  void updateWidgetMap(void);

private:
  KAppTestPrivate*           d;
};

#endif
