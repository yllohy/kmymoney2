/***************************************************************************
                          kpayeesview.h
                          -------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
                                    2005 by Andrea Nicolai
                                    2006 by Thomas Baumgart
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
class QSplitter;

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>
#include <kpopupmenu.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kpayeesviewdecl.h"
#include "kmymoneytransaction.h"
#include <kmymoney/mymoneypayee.h>

class KListViewSearchLineWidget;

/**
  * @author Michael Edwardes, Thomas Baumgart
  */

/**
  * This class is used to store a sorted vector of pointers to
  * the transactions that are visible in a ledger view. When the
  * vector is created, the sort method is set to SortPostDate.
  * The sort type can be changed using the method setSortType().
  */
class KTransactionPtrVector : public QPtrVector<KMyMoneyTransaction> {
public:
  /**
    * This enumerator defines the possible sort methods.
    * Possible values are:
    *
    */
  enum TransactionSortE {
    SortEntryDate = 0,      /**< Sort the vector so that the transactions appear sorted
                              *  according to their entry date
                              */
    SortPostDate,           /**< Sort the vector so that the transactions appear sorted
                              *     according to their post date
                              */
    SortTypeNr,             /**< Sort the vector so that the transactions appear sorted
                              *     according to their action and nr
                              */
    SortReceiver,           /**< Sort the vector so that the transactions appear sorted
                              *     according to their receiver
                              */
    SortValue,              /**< Sort the vector so that the transactions appear sorted
                              *     according to their value
                              */
    SortNr,                 /**< Sort the vector so that the transactions appear sorted
                              *     according to nr field contents
                              */
    SortEntryOrder          /**< Sort the vector so that the transactions appear sorted
                              *     according to order of entry
                              */
  };

  KTransactionPtrVector() { m_sortType = SortPostDate; };
  ~KTransactionPtrVector() {}

  /**
    * This method is used to set a different sort type.
    * The vector is resorted. See KTransactionPtrVector::TransactionSortE
    * for possible values.
    */
  void setSortType(const TransactionSortE type);

  /**
    * This method returns the current sort type.
    *
    * @return transactionSortE value of sort order. See
    *         KTransactionPtrVector::TransactionSortE for possible values.
    */
  TransactionSortE sortType(void) const { return m_sortType; };

  /**
    * This method is used to set the account id to have a chance to
    * get information about the split referencing the current account
    * during the sort phase.
    */
  void setAccountId(const QString& id);

  /**
    * This method is used to set the payee id to have a chance to
    * get information about the split referencing the current payee
    * during the sort phase.
    */
  void setPayeeId(const QString& id);

protected:
  int compareItems(KTransactionPtrVector::Item d1, KTransactionPtrVector::Item d2);

private:
  int compareItems(const QString& s1, const QString& s2) const;

private:
  enum {
    AccountMode = 0,
    PayeeMode
  };
  short             m_idMode;
  QString           m_id;
  TransactionSortE  m_sortType;
};



/**
  * This class represents an item in the payees list view.
  */
class KPayeeListItem : public KListViewItem
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

  const MyMoneyPayee& payee(void) const { return m_payee; };

private:
  MyMoneyPayee  m_payee;
};

/**
  * This class represents an item in the transaction list view. It is used
  * by the KPayeesView to select between transactions.
  */
class KTransactionListItem : public KListViewItem
{
public:
  KTransactionListItem(KListView* view, KTransactionListItem* parent, const QString& accountId, const QString& transaction);
  ~KTransactionListItem();

  const QString& transactionId(void) const { return m_transactionId; };

  const QString& accountId(void) const { return m_accountId; };

  /**
    * use my own paint method
    */
  void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);

  /**
    * use my own backgroundColor method
    */
  const QColor backgroundColor();

private:
  QString m_transactionId;
  QString m_accountId;
};

class KPayeesView : public KPayeesViewDecl
{
  Q_OBJECT

public:
  KPayeesView(QWidget *parent=0, const char *name=0);
  ~KPayeesView();
  void show(void);

public slots:
  void slotSelectPayeeAndTransaction(const QString& payeeId, const QString& accountId = QString(), const QString& transactionId = QString());
  void slotLoadPayees(void);
  void slotStartRename(void);
  void slotHelp(void);

protected:
  void resizeEvent(QResizeEvent*);
  void loadPayees(void);
  void selectedPayees(QValueList<MyMoneyPayee>& payeesList) const;
  void ensurePayeeVisible(const QString& id);
  void clearItemData(void);

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
  void slotSelectPayee(void);

  /**
    * This slot marks the current selected payee as modified (dirty).
    */
  void slotPayeeDataChanged(void);
  void slotKeyListChanged(void);

  /**
    * This slot is called when the name of a payee is changed inside
    * the payee list view and only a single payee is selected.
    */
  void slotRenamePayee(QListViewItem *p, int col, const QString& txt);

  /**
    * Updates the payee data in m_payee from the information in the
    * payee information widget.
    */
  void slotUpdatePayee(void);

  void slotTransactionDoubleClicked(QListViewItem *);

private slots:
  void rearrange(void);

  /**
    * This slot receives the signal from the listview control that an item was right-clicked,
    * If @p item points to a real payee item, emits openContextMenu().
    *
    * @param lv pointer to the listview sending the signal
    * @param item the item on which the cursor resides
    * @param p position of the pointer device
    */
  void slotOpenContextMenu(KListView* lv, QListViewItem* item, const QPoint& p);

  void slotQueueUpdate(void);

  void slotActivateUpdate(void);

  void slotChooseDefaultAccount(void);

private:
  void readConfig(void);

signals:
  void transactionSelected(const QString& accountId, const QString& transactionId);
  void openContextMenu(const MyMoneyObject& obj);
  void selectObjects(const QValueList<MyMoneyPayee>& payees);

private:
  MyMoneyPayee m_payee;
  QString      m_newName;

  QSplitter*    m_splitter;

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
  bool m_needReload;

  /**
    * Search widget for the list
    */
  KListViewSearchLineWidget*  m_searchWidget;
  bool m_needConnection;

  /**
    * Counting semaphore to collect updates
    */
  int m_updatesQueued;

  /**
   * Semaphore to suppress loading during selection
   */
  bool m_inSelection;
};

#endif
