/***************************************************************************
                          kpayeesview.h
                          -------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Andreas Nicolai <Andreas.Nicolai@gmx.net>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KPAYEESVIEW_H
#define KPAYEESVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>
#include <kpopupmenu.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kpayeesviewdecl.h"
#include "kledgerview.h"
#include "../mymoney/mymoneyobserver.h"
#include "../mymoney/mymoneypayee.h"

/**
  *@author Michael Edwardes
  */

/**
  * This class is a base class that maintains payee information
  * for the KPayeesListItem.
  */
class KPayeeItem
{
public:
  KPayeeItem() {} ;
  virtual ~KPayeeItem() {};

  /**
    * This method allows to set the payees id.
    *
    * @param id payee id to be stored in m_payeeID;
    */
  void setPayeeID(const QCString& id) { m_payeeID = id; };

  /**
    * This method returns the payee's id for this object
    *
    * @return const QCString of the Id
    */
  const QCString payeeID(void) const { return m_payeeID; };

private:
  QCString    m_payeeID;
};


/**
  * This class represents an item in the account list view. It is used
  * by the KPayeesView to select between the payees.
  */
class KPayeeListItem : public KListViewItem, public KPayeeItem, MyMoneyObserver
{
public:
  /**
    * Constructor to be used to construct a payee entry object.
    *
    * @param parent pointer to the KListView object this entry should be
    *               added to.
    * @param payee const reference to MyMoneyPayee for which
    *               the KListView entry is constructed
    */
  KPayeeListItem(KListView *parent, const MyMoneyPayee& payee);
  ~KPayeeListItem();

  /**
    * This method is re-implemented from QListViewItem::paintCell().
    * Besides the standard implementation, the QPainter is set
    * according to the applications settings.
    */
  void paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align);

  //void paintFocus(QPainter *p, const QColorGroup & cg, const QRect& rect);

  /**
    * This method is called by the MyMoneyFile object, whenever the
    * payee that is represented by this object changes within the
    * MyMoneyFile engine.
    *
    * @param id reference to QCString of the payee's id
    */
  void update(const QCString& id);

  /**
    */
  void suspendUpdate(const bool suspend) { m_suspendUpdate = suspend; };

private:
  bool    m_suspendUpdate;
};


class KPayeesView : public KPayeesViewDecl, MyMoneyObserver
{
   Q_OBJECT
public:
  KPayeesView(QWidget *parent=0, const char *name=0);
  ~KPayeesView();
  void show();
  void update(const QCString &id);

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
  void slotSelectPayeeAndTransaction(const QCString& payeeId, const QCString& accountId, const QCString& transactionId);
  void slotRefreshView(void);
  void slotReloadView(void);

protected:
  void resizeEvent(QResizeEvent*);

protected slots:
  /**
    * This method loads the m_transactionList, clears
    * the m_TransactionPtrVector and rebuilds and sorts
    * it according to the current settings. Then it
    * loads the m_transactionView with the transaction data.
    */
  void showTransactions(void);

  /**
    * This slot is called whenever the selection in m_payeesList
    * has been changed.
    */
  void slotSelectPayee();

  /**
    * This slot is called whenever the action "New payee" is executed
    * from the context menu and adds a new payee with some default name.
    */
  void slotAddPayee();

  /**
    * This slot is called whenever the action "Delete payee" is executed
    * from the context menu, deletes selected payees and reassigns
    * transactions to an alternative payee.
    */
  void slotDeletePayee();

  /**
    * This slot marks the current selected payee as modified (dirty).
    */
  void slotPayeeDataChanged();

  /**
    * This slot is called when the name of a payee is changed inside
    * the payee list view and only a single payee is selected.
    */
  void slotRenamePayee(QListViewItem *p, int col, const QString& txt);

  /**
    * Updates the payee data in m_payee from the information in the
    * payee information widget.
    */
  void slotUpdatePayee();

  void slotTransactionDoubleClicked(QListViewItem *);

private slots:
  void rearrange(void);

  /**
    * This slot receives the signal from the listview control that an item was right-clicked,
    * Pass this signal along to the main view to display the RMB menu.
    */
  void slotListRightMouse(QListViewItem* item, const QPoint& point, int);

private:
  void readConfig(void);
  void writeConfig(void);

signals:
  void signalViewActivated();
  void transactionSelected(const QCString& accountId, const QCString& transactionId);

private:
  MyMoneyPayee m_payee;
  QString      m_newName;

  /**
    * This member holds a list of all transactions
    */
  QValueList<KMyMoneyTransaction> m_transactionList;

  /**
    * This member keeps a vector of pointers to all visible (filtered)
    * transaction in m_transactionList in sorted order. Sorting is done
    * in KTransactionPtrVector::compareItems
    */
  KTransactionPtrVector m_transactionPtrVector;

  /**
    * This member holds the state of the toggle switch used
    * to suppress updates due to MyMoney engine data changes
    */
  bool m_suspendUpdate;

  KPopupMenu*   m_contextMenu;
};

#endif
