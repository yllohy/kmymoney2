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
  bool m_hideCategory;

  QMap<QCString, MyMoneyAccount> m_accountMap;
  QMap<QCString, unsigned long> m_transactionCountMap;

  void readConfig(void);
  void writeConfig(void);
  void update(const QCString& id);
  const bool showSubAccounts(const QCStringList& accounts, KAccountListItem *parentItem, const QString&);

protected:
  void resizeEvent(QResizeEvent *);

public slots:
  void slotEditClicked(void);
  void slotEditClicked(MyMoneyAccount& account);
  void slotDeleteClicked(void);
  void slotDeleteClicked(MyMoneyAccount& account);
  void slotRefreshView(void);
  void slotReloadView(void) { slotRefreshView(); };
    
protected slots:
  void slotListRightMouse(QListViewItem* item, const QPoint& , int col);

signals:
  void signalViewActivated();
  void categoryRightMouseClick();

public:
  KCategoriesView(QWidget *parent=0, const char *name=0);
  ~KCategoriesView();
  void show();


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

  /**
    * This method returns the id of the current selected account. If
    * no account is selected, then an empty QCString will be returned.
    * The flag referenced by the parameter @p success will be set
    * to true if an account was found, false in all other cases.
    *
    * @param success Reference to boolean flag for success
    */
  const QCString currentAccount(bool& success) const;

};

#endif
