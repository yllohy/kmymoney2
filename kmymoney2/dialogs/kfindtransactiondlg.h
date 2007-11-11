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
#include <qmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

// #include "../views/kledgerview.h"
#include <kmymoney/mymoneyutils.h>
#include <kmymoney/mymoneytransactionfilter.h>

#include "../dialogs/kfindtransactiondlgdecl.h"

class QListView;
class QListViewItem;

/**
  * @author Thomas Baumgart
  */
class KFindTransactionDlg : public KFindTransactionDlgDecl
{
  Q_OBJECT
public:

  /*
  // Make sure to keep the following enum valus in sync with the values
  // used by the GUI in kfindtransactiondlgdecl.ui
  enum dateOptionE {
    allDates = 0,
    untilToday,
    currentMonth,
    currentYear,
    monthToDate,
    yearToDate,
    yearToMonth,
    lastMonth,
    lastYear,
    last7Days,
    last30Days,
    last3Months,
    last6Months,
    last12Months,
    next7Days,
    next30Days,
    next3Months,
    next6Months,
    next12Months,
    userDefined,
    last3ToNext3Months,
    last11Months,
    // insert new constants above of this line
    dateOptionCount
  };
*/
  KFindTransactionDlg(QWidget *parent=0, const char *name=0);
  ~KFindTransactionDlg() {}

  virtual bool eventFilter( QObject *o, QEvent *e );

public slots:
  void show(void);

protected:
  void resizeEvent(QResizeEvent*);

protected slots:
  virtual void slotReset(void);
  virtual void slotSearch(void);

  /**
    * This slot opens the detailed help page in khelpcenter. The
    * anchor for the information is taken from m_helpAnchor.
    */
  virtual void slotShowHelp(void);


  void slotUpdateSelections(void);

  void slotDateRangeChanged(int);
  void slotDateChanged(void);

  void slotAmountSelected(void);
  void slotAmountRangeSelected(void);

  void slotSelectAllPayees(void);
  void slotDeselectAllPayees(void);

  void slotNrSelected(void);
  void slotNrRangeSelected(void);

  void slotRefreshView(void);

  /**
    * This slot selects the current selected transaction/split and emits
    * the signal @a transactionSelected(const QCString& accountId, const QCString& transactionId)
    */
  void slotSelectTransaction(void);

  void slotRightSize(void);

  void slotSortOptions(void);

signals:
  void transactionSelected(const QCString& accountId, const QCString& transactionId);

  /**
    * This signal is sent out when no selection has been made. It is
    * used to control the state of the Search button.
    */
  void selectionEmpty(bool);

protected:
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

  void setupFilter(void);

  void selectAllItems(QListView* view, const bool state);
  void selectAllSubItems(QListViewItem* item, const bool state);
  void selectItems(QListView* view, const QCStringList& list, const bool state);
  void selectSubItems(QListViewItem* item, const QCStringList& list, const bool state);

  /**
    * This method loads the m_payeesView with the payees name
    * found in the engine.
    */
  void loadPayees(void);

  /**
    * This method loads the register with the matching transactions
    */
  void loadView(void);

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

  void scanCheckListItems(const QListView* view, const opTypeE op);
  void scanCheckListItems(const QListViewItem* item, const opTypeE op);
  void addItemToFilter(const opTypeE op, const QCString& id);

protected:
  QDate                m_startDates[MyMoneyTransactionFilter::dateOptionCount];
  QDate                m_endDates[MyMoneyTransactionFilter::dateOptionCount];

  /**
    * This member holds a list of all transactions matching the filter criteria
    */
  QValueList<QPair<MyMoneyTransaction, MyMoneySplit> > m_transactionList;

  MyMoneyTransactionFilter        m_filter;

  QMap<QWidget*, QString>         m_helpAnchor;

  bool                            m_needReload;
};

#endif
