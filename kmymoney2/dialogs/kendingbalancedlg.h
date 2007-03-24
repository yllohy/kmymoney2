/***************************************************************************
                          kendingbalancedlg.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KENDINGBALANCEDLG_H
#define KENDINGBALANCEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qdatetime.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

class kMyMoneyEdit;
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/mymoneyaccount.h>
#include "../dialogs/kendingbalancedlgdecl.h"

class KEndingBalanceDlgPrivate;
class KEndingBalanceLoanDlgPrivate;

/**
  * This dialog is wizard based and used to enter additional
  * information required to start the reconciliation process.
  * This version implements the behaviour for checkings,
  * savings and credit card accounts.
  *
  * @author Thomas Baumgart
  */
class KEndingBalanceDlg : public KEndingBalanceDlgDecl
{
  Q_OBJECT
public:
  KEndingBalanceDlg(const MyMoneyAccount& account, QWidget *parent=0, const char *name=0);
  ~KEndingBalanceDlg();

  const MyMoneyMoney endingBalance(void) const;
  const MyMoneyMoney previousBalance(void) const;
  const QDate statementDate(void) const { return m_statementDate->date(); };

  const MyMoneyTransaction interestTransaction(void);
  const MyMoneyTransaction chargeTransaction(void);

protected:
  bool createTransaction(MyMoneyTransaction& t, const int sign, kMyMoneyEdit *amountEdit, KMyMoneyCategory *categoryEdit, kMyMoneyDateInput* dateEdit);
  const MyMoneyMoney adjustedReturnValue(const MyMoneyMoney& v) const;
  void createCategory(const QString& txt, QCString& id, const MyMoneyAccount& parent);

protected slots:
  void slotCheckPageFinished(void);
  void slotReloadEditWidgets(void);
  void help(void);
  void slotCreateInterestCategory(const QString& txt, QCString& id);
  void slotCreateChargesCategory(const QString& txt, QCString& id);
  void accept(void);

signals:
  /**
    * proxy signal for KMyMoneyPayeeCombo::createItem(const QString&, QCString&)
    */
  void createPayee(const QString&, QCString&);

  /**
    * emit when a category is about to be created
    */
  void createCategory(MyMoneyAccount& acc, const MyMoneyAccount& parent);

private:
  KEndingBalanceDlgPrivate* d;
};

/**
  * This dialog is wizard based and used to enter additional
  * information required to start the reconciliation process.
  * This version is implements the behaviour for loan accounts.
  */
class KEndingBalanceLoanDlg : public KEndingBalanceDlgDecl
{
  Q_OBJECT
public:
  KEndingBalanceLoanDlg(const MyMoneyAccount& account, QWidget *parent=0, const char *name=0);
  ~KEndingBalanceLoanDlg();

  /**
    * This method returns the adjustment transaction if one
    * has been created. If not, an empty transaction will be returned.
    */
  const MyMoneyTransaction adjustmentTransaction(void) const;

  /**
    * This method returns the starting date of the statement as provided
    * by the user. The value returned is only valid if the dialog returned
    * with QDialog::accept.
    */
  const QDate startDate(void) const { return m_startDateEdit->date(); };

  /**
    * This method returns the ending date of the statement as provided
    * by the user. The value returned is only valid if the dialog returned
    * with QDialog::accept.
    */
  const QDate endDate(void) const { return m_endDateEdit->date(); };

protected:
  const MyMoneyMoney totalInterest(const QDate& start, const QDate& end) const;
  const MyMoneyMoney totalAmortization(const QDate& start, const QDate& end) const;

public slots:
  void next();

protected slots:
  void slotCheckPageFinished(void);
  void help(void);

private:
  KEndingBalanceLoanDlgPrivate*  d;
};

#endif
