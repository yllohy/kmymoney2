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

#ifndef KLEDGERVIEWINVESTMENTS_H
#define KLEDGERVIEWINVESTMENTS_H

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

#if 0
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneypayee.h"
#endif

#include "../widgets/kmymoneyaccountcombo.h"
#include "kledgerview.h"

class kMyMoneyEdit;
class kMyMoneyEquity;

/**
  *@author Kevin Tambascio
  */

class KLedgerViewInvestments : public KLedgerView
{
  Q_OBJECT
public:

  KLedgerViewInvestments(QWidget *parent = NULL, const char *name = NULL);
  ~KLedgerViewInvestments();

  int transactionType(const MyMoneyTransaction& t, const MyMoneySplit& split) const;

public slots:
  void slotRegisterDoubleClicked(int row, int col, int button, const QPoint &mousePos);

  virtual void refreshView(const bool transactionFormVisible);

protected slots:
  /**
    * Calling this slot opens the account edit dialog for the current
    * selected account.
    */
  virtual void slotAccountDetail(void);
  virtual void slotTypeSelected(int type);
  virtual void slotReconciliation(void);
  virtual void slotNew();
  virtual void slotStartEdit();
  virtual void slotEndEdit();
  virtual void slotEquityChanged(const QCString& id);

protected:
  virtual void createEditWidgets();
  virtual void fillForm();
  /**
    * This method updates the static text areas within the transaction form
    * according to the currently loaded values in m_transaction and m_split.
    */
  virtual void fillFormStatics(void);

  virtual void fillSummary();
  virtual void showWidgets();
  virtual void hideWidgets();
  virtual void reloadEditWidgets(const MyMoneyTransaction& t);
  virtual void updateTabBar(const MyMoneyTransaction& t, const MyMoneySplit& s);

  void resizeEvent(QResizeEvent*);

  QWidget* arrangeEditWidgetsInForm(void);

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
  int actionTab(const MyMoneyTransaction& t, const MyMoneySplit& split) const;

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

  QCString        m_action;

  kMyMoneyEdit *m_editShares, *m_editPPS, *m_editTotalAmount, *m_editFees;
  kMyMoneyEquity *m_editSymbolName;

  MyMoneySplit    m_accountSplit;
  MyMoneySplit    m_feeSplit;
  MyMoneySplit    m_interestSplit;
  kMyMoneyAccountCombo  *m_editCashAccount;

/*
  KTextBrowser *textBrowser;
  QGridLayout *mainGrid;
  QTabWidget *m_InvestmentTabs;
  QWidget *m_SummaryTab,
       *m_TransactionTab;
*/
};

#endif
