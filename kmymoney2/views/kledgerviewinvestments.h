/***************************************************************************
                          kledgerviewinvestments.h  -  description
                             -------------------
    begin                : Sun Mar 7 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

// ----------------------------------------------------------------------------
// QT Includes
#include <qlayout.h>
#include <qtabbar.h>
#include <qtabwidget.h>

class QLabel;

// ----------------------------------------------------------------------------
// KDE Includes
#include <ktextbrowser.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneypayee.h"

#ifndef KLEDGERVIEWINVESTMENTS_H
#define KLEDGERVIEWINVESTMENTS_H

#include "kledgerview.h"

/**
  *@author Kevin Tambascio
  */

class KLedgerViewInvestments : public KLedgerView
{
  Q_OBJECT
public:

  enum investTransactionTypeE {
    AddShares = 100,
    RemoveShares,
    Deposit,
    Withdrawal
  };

  KLedgerViewInvestments(QWidget *parent = NULL, const char *name = NULL);
  ~KLedgerViewInvestments();

public slots:
  void slotRegisterDoubleClicked(int row, int col, int button, const QPoint &mousePos);

protected slots:
  /**
    * Calling this slot opens the account edit dialog for the current
    * selected account.
    */
  virtual void slotAccountDetail(void);
  virtual void slotTypeSelected(int type);
  virtual void slotReconciliation(void);
  virtual void slotNew();

protected:
  virtual void createEditWidgets();
  virtual void fillForm();
  virtual void fillSummary();
  virtual void showWidgets();
  virtual void hideWidgets();
  virtual void reloadEditWidgets(const MyMoneyTransaction& t);
  virtual void updateTabBar(const MyMoneyTransaction& t, const MyMoneySplit& s);

private:
  /**
    * This method is used by the constructor to create the necessary widgets
    * for the register of the view and set it up.
    */
  void createRegister(void);

  /**
    * This method is used by the constructor to create the transaction form
    * provided by the view.
    */
  void createForm(void);

  /**
    * This method is used by the constructor to create the summary line underneath
    * the register widget in the view.
    */
  void createSummary(void);

  /**
    * This method is used by the constructor to create the info stack on
    * the right of the register widget. The stack widget itself is created
    * by the base class member of this function.
    */
  void createInfoStack(void);

protected:
  int transactionType(const MyMoneyTransaction& t, const MyMoneySplit& split) const;
  const QCString transactionType(int type) const;

protected:
  KPushButton*  m_detailsButton;
  KPushButton*  m_reconcileButton;

private:
  /**
    * This member keeps a pointer to the summary line
    * which is located underneath the register. The
    * widget itself is created in createSummary()
    */
  QLabel          *m_summaryLine;

  QHBoxLayout*    m_summaryLayout;

  QLabel*         m_lastReconciledLabel;

  QTab *m_tabAddShares, *m_tabRemoveShares;//, *m_tabTransfer,
       //*m_tabWithdrawal, *m_tabAtm;

  QCString m_action;
/*
  KTextBrowser *textBrowser;
  QGridLayout *mainGrid;
  QTabWidget *m_InvestmentTabs;
  QWidget *m_SummaryTab,
       *m_TransactionTab;
*/
};

#endif
