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

#include "kfindtransactiondlgdecl.h"

/**
  * @author Thomas Baumgart
  */

class KFindTransactionDlg;

class KMyMoneyCheckListItem : public QCheckListItem {
public:
  KMyMoneyCheckListItem(QListView *parent, const QString& txt, Type type, const QCString& id, KFindTransactionDlg* dlg);
  KMyMoneyCheckListItem(QListViewItem *parent, const QString& txt, Type type, const QCString& id, KFindTransactionDlg* dlg);
  ~KMyMoneyCheckListItem();
  const QCString& accountId(void) const { return m_id; };

protected:
  virtual void stateChange(bool);
  
private:
  QCString             m_id;
  KFindTransactionDlg* m_dlg;
};

class KFindTransactionDlg : public KFindTransactionDlgDecl  {
  Q_OBJECT
public:
  KFindTransactionDlg(QWidget *parent=0, const char *name=0);
  ~KFindTransactionDlg();

protected:
  // Make sure to keep the following enum valus in sync with the values
  // defined in kfindtransactiondlgdecl.ui
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

  // Make sure to keep the following enum valus in sync with the values
  // defined in kfindtransactiondlgdecl.ui
  enum typeOptionE {
    allTypes = 0,
    payments,
    deposits,
    transfers,
    // insert new constants above of this line
    typeOptionCount
  };

  // Make sure to keep the following enum valus in sync with the values
  // defined in kfindtransactiondlgdecl.ui
  enum stateOptionE {
    allStates = 0,
    reconciled,
    notReconciled,
    // insert new constants above of this line
    stateOptionCount
  };

public slots:
  void slotUpdateSelections(void);

protected slots:
  void slotDateRangeChanged(int);
  void slotDateChanged(void);
  
  void slotSelectAllAccounts(void);
  void slotDeselectAllAccounts(void);
  
  void slotSelectAllCategories(void);
  void slotDeselectAllCategories(void);
  void slotSelectIncomeCategories(void);
  void slotSelectExpenseCategories(void);
      
  void slotAmountSelected(void);
  void slotAmountRangeSelected(void);

  void slotSelectAllPayees(void);
  void slotDeselectAllPayees(void);
  
  void slotNrSelected(void);
  void slotNrRangeSelected(void);

signals:

private:
  void setupCategoriesPage(void);
  void setupDatePage(void);
  void setupAccountsPage(void);
  void setupAmountPage(void);
  void setupPayeesPage(void);
  void setupDetailsPage(void);
    
  void selectAllItems(QListView* view, const bool state);  
  void selectAllCategories(const bool income, const bool expense);
  void selectAllSubItems(QListViewItem* item, const bool state);
  
  void readConfig(void);
  void writeConfig(void);

  /**
    * This method loads the m_accountsView with the asset and liability
    * accounts found in the engine. Accounts that are in the engine,
    * but not in the view will be added to the view and marked selected.
    * Accounts that are in the view but not in the engine will be removed
    * from the view.
    */
  void loadAccounts(void);
  void loadCategories(void);

  void loadSubAccounts(QListViewItem* parent, const QCStringList& list);
  
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

private:
  QDate                m_startDates[dateOptionCount];
  QDate                m_endDates[dateOptionCount];
};

#endif
