/***************************************************************************
                          kbudgetview.h
                          -------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2006 by Darren Gould
    email                : darren_gould@gmx.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KBUDGETVIEW_H
#define KBUDGETVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>
#include <kpopupmenu.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kbudgetviewdecl.h"
#include "kledgerview.h"
#include "../mymoney/mymoneyobserver.h"
#include "../mymoney/mymoneypayee.h"

/**
  *@author Darren Gould
  */

class KBudgetView : public KBudgetViewDecl, MyMoneyObserver
{
   Q_OBJECT
public:
  KBudgetView(QWidget *parent=0, const char *name=0);
  ~KBudgetView();
  void show();
  virtual void update(const QCString& id);
  /**
    * This method is used to suppress updates for specific times
    * (e.g. during creation of a new MyMoneyFile object when the
    * default accounts are loaded). The behaviour of update() is
    * controlled with the parameter.
    *
    * @param suspend Suspend updates or not. Possible values are
    *
    * @li true updates are suspended
    * @li false updates will be performed immediately
    *
    * When a true/false transition of the parameter between
    * calls to this method is detected,
    * refresh() will be invoked once automatically.
    */
  void suspendUpdate(const bool suspend);

public slots:
  /**
    * Displays the New Budget Dialog when the user presses
    * this button
    *
    */
    virtual void m_bNewBudget_clicked();
    void slotReloadView(void);
    void slotRefreshView(void);

protected:
  void resizeEvent(QResizeEvent*);

private slots:
  void rearrange(void);

signals:
  void signalViewActivated();

private:
  /**
    * This member holds the state of the toggle switch used
    * to suppress updates due to MyMoney engine data changes
    */
  bool m_suspendUpdate;
};

#endif
