/***************************************************************************
                          kfindtransactiondlg.h
                             -------------------
    copyright            : (C) 2003 by Thomas Baumgart
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

#ifndef KFINDTRANSACTIONDLG_H
#define KFINDTRANSACTIONDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qlistview.h>
#include <qdatetime.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyutils.h"
#include "../views/kledgerview.h"
#include "../mymoney/mymoneytransactionfilter.h"

#include "kfindtransactiondlgdecl.h"

class QListView;
class QListViewItem;

/**
  * @author Thomas Baumgart
  */

class KFindTransactionDlg : public KFindTransactionDlgDecl, public IMyMoneyRegisterParent {
  Q_OBJECT
public:

  // Make sure to keep the following enum valus in sync with the values
  // used by the GUI in kfindtransactiondlgdecl.ui
  enum dateOptionE {
    allDates = 0,
    untilToday,
    currentMonth,
    currentYear,
    monthToDate,
    yearToDate,
    lastMonth,
    lastYear,
    last30Days,
    last3Months,
    last6Months,
    last12Months,
    next30Days,
    next3Months,
    next6Months,
    next12Months,
    userDefined,
    // insert new constants above of this line
    dateOptionCount
  };

  KFindTransactionDlg(QWidget *parent=0, const char *name=0);
  ~KFindTransactionDlg();

  /**
    * This method returns a pointer to the transaction data
    * in the ledger of this account. The transaction is identified
    * by the parameter @p idx.
    *
    * @param idx index into ledger starting at 0
    * @return pointer to MyMoneyTransaction object representing the
    *         selected transaction. If idx is out of bounds,
    *         0 will be returned.
    */
  KMyMoneyTransaction* transaction(const int idx) const;

  /**
    * This method returns the id of the account that should be
    * shown by the view. The implementation depends on the type of view
    * and might return a constant value or a value depending on the
    * transaction passed as @p transaction.
    *
    * @param transaction pointer to a transaction
    *
    * @return const QCString containing the account's id.
    */
  const QCString accountId(const MyMoneyTransaction * const transaction, const int match) const;

  /**
    * This method returns the balance of any visible transaction
    * in the ledger of this account. The balance depends on filters
    * and is automatically calculated when any view option is changed
    * (e.g. filters, sort order, etc.)
    *
    * @param idx index into the ledger starting at 0
    * @return Value of the balance for the account after the selected
    *         transaction has been taken into account. If idx is out
    *         of bounds, 0 will be returned as value. For liability type
    *         accounts, the sign will be inverted for display purposes.
    *
    * @note Currently the fixed value 0 will be returned as the balance
    *       is not shown in the find transaction dialog but the method
    *       must be provided as part of the IMyMoneyRegisterParent interface.
    */
  const MyMoneyMoney balance(const int /* idx */) const { return 0; };

public slots:
  void slotUpdateSelections(void);
  void slotReset(void);

protected:
  void resizeEvent(QResizeEvent*);
  
protected slots:
  void slotDateRangeChanged(int);
  void slotDateChanged(void);

  void slotAmountSelected(void);
  void slotAmountRangeSelected(void);

  void slotSelectAllPayees(void);
  void slotDeselectAllPayees(void);
  
  void slotNrSelected(void);
  void slotNrRangeSelected(void);

  void slotSearch(void);

  void slotRefreshView(void);
    
  /**
    *
    */
  void slotRegisterClicked(int row, int col, int button, const QPoint &mousePos);
  void slotRegisterDoubleClicked(int row, int col, int button, const QPoint &mousePos);
  void slotNextTransaction(void);
  void slotPreviousTransaction(void);
  void slotSelectTransaction(void);
  void slotSelectTransaction(const QCString& id);
  
private slots:
  void slotRightSize(void);
  
signals:
  void transactionSelected(const QCString& accountId, const QCString& transactionId);
  
private:
  enum opTypeE {
    addAccountToFilter = 0,
    addCategoryToFilter,
    addPayeeToFilter
  };

  void setupCategoriesPage(void);
  void setupDatePage(void);
  void setupAccountsPage(void);
  void setupAmountPage(void);
  void setupPayeesPage(void);
  void setupDetailsPage(void);
    
  void selectAllItems(QListView* view, const bool state);  
  void selectAllSubItems(QListViewItem* item, const bool state);
  
  void readConfig(void);
  void writeConfig(void);

  /**
    * This method loads the m_payeesView with the payees name
    * found in the engine.
    */
  void loadPayees(void);
  
  /**
    * This method returns information about the selection state
    * of the items in the m_accountsView.
    *
    * @param view pointer to the listview to scan
    *
    * @retval true if all items in the view are marked
    * @retval false if at least one item is not marked
    *
    * @note If the view contains no items the method returns @p true.
    */
  const bool allItemsSelected(const QListView* view) const;
  const bool allItemsSelected(const QListViewItem *item) const;

  void scanCheckListItems(QListView* view, const opTypeE op);
  void scanCheckListItems(QListViewItem* item, const opTypeE op);
  void addItemToFilter(const opTypeE op, const QCString& id);

private:
  QDate                m_startDates[dateOptionCount];
  QDate                m_endDates[dateOptionCount];

  /**
    * This member holds a list of all transactions matching the filter criteria
    */
  QValueList<KMyMoneyTransaction> m_transactionList;

  /**
    * This member keeps a vector of pointers to all visible (filtered)
    * transaction in m_transactionList in sorted order. Sorting is done
    * in KTransactionPtrVector::compareItems
    */
  KTransactionPtrVector           m_transactionPtrVector;

  /**
    * This member keeps a pointer to the currently selected transaction
    * It is NULL, if an empty (new) transaction is selected.
    */
  KMyMoneyTransaction*            m_transactionPtr;

  MyMoneyTransactionFilter        m_filter;
  
};

#endif
