/***************************************************************************
                          kcategoriesview.h  -  description
                             -------------------
    begin                : Sun Jan 20 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

#ifndef KCATEGORIESVIEW_H
#define KCATEGORIESVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qcstring.h>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneyobserver.h"
#include "kcategoriesviewdecl.h"
#include "kbanklistitem.h"

/**
  *@author Michael Edwardes, Thomas Baumgart
  */

class KCategoriesView : public kCategoriesViewDecl, MyMoneyObserver  {
   Q_OBJECT
private:
	QString m_lastCat;
	bool m_suspendUpdate;

  QMap<QCString, MyMoneyAccount> m_accountMap;

  void readConfig(void);
  void writeConfig(void);
  void refresh(void);
  void update(const QCString& id);
  void showSubAccounts(const QCStringList& accounts, KAccountListItem *parentItem, const QString&);

protected:
  void resizeEvent(QResizeEvent *);

protected slots:
  void slotEditClicked();
  void slotNewClicked();
  void slotDeleteClicked();
  void slotSelectionChanged(QListViewItem*);

signals:
  void signalViewActivated();

public:
	KCategoriesView(QWidget *parent=0, const char *name=0);
	~KCategoriesView();
  void show();


  void refreshView(void);

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
};

#endif
