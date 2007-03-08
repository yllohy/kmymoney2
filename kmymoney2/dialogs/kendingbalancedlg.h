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

#include "../widgets/kmymoneyedit.h"
#include "../mymoney/mymoneyaccount.h"
#include "../widgets/kmymoneydateinput.h"
#include "../dialogs/kendingbalancedlgdecl.h"

/**
  * This dialog is wizard based and used to enter additional
  * information required to start the reconciliation process.
  * This version is implements the behaviour for checkings,
  * savings and credit card accounts.
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

  const MyMoneyTransaction interestTransaction(void) const;
  const MyMoneyTransaction chargeTransaction(void) const;

protected:
  const MyMoneyTransaction createTransaction(const int sign, kMyMoneyEdit *amountEdit, kMyMoneyCategory *categoryEdit) const;
  const MyMoneyMoney adjustedReturnValue(const MyMoneyMoney& v) const;

protected slots:
  void slotCheckPageFinished(void);
  void help(void);

signals:
  /**
    * This signal is emitted, when a new category name has been
    * entered by the user and this name is not known as account
    * by the MyMoneyFile object.
    * Before the signal is emitted, a MyMoneyAccount is constructed
    * by this object and filled with the desired name. All other members
    * of MyMoneyAccount will remain in their default state. Upon return,
    * the connected slot should have created the object in the MyMoneyFile
    * engine and filled the member @p id.
    *
    * @param acc reference to MyMoneyAccount object that caries the name
    *            and will return information about the created category.
    */
  void newCategory(MyMoneyAccount& acc);

private:
  MyMoneyAccount m_account;
  QMap<QWidget*, QString>         m_helpAnchor;
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
  MyMoneyAccountLoan m_account;
  QMap<QWidget*, QString>         m_helpAnchor;
};

#endif
