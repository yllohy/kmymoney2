/***************************************************************************
                          knewloanwizard.h  -  description
                             -------------------
    begin                : Wed Oct 8 2003
    copyright            : (C) 2000-2003 by Thomas Baumgart
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

#ifndef KNEWLOANWIZARD_H
#define KNEWLOANWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <../dialogs/knewloanwizarddecl.h>
#include <../mymoney/mymoneyscheduled.h>
#include <../widgets/kmymoneyaccountselector.h>
#include <../widgets/kmymoneydateinput.h>

/**
  * @author Thomas Baumgart
  */

/**
  * This class implementes a wizard for the creation of loan accounts.
  * The user is asked a set of questions and according to the answers
  * the respective MyMoneyAccount object can be requested from the
  * wizard when accept() has been called. A MyMoneySchedule is also
  * available to create a schedule entry for the payments to the newly
  * created loan.
  */
class KNewLoanWizard : public KNewLoanWizardDecl
{
  Q_OBJECT
public:
  KNewLoanWizard(QWidget *parent=0, const char *name=0);
  ~KNewLoanWizard();

  /**
    * This method returns a MyMoneyAccount object with all data
    * filled out as provided by the wizard. The institution reference
    * will be empty.
    *
    * @return MyMoneyAccount object to be used to create a new account
    */
  const MyMoneyAccount account(void) const;

  /**
    * This method returns the schedule for the payments. The account
    * where the amortization should be transferred to is the one
    * we currently try to create with this wizard. The appropriate split
    * will be returned as the first split of the transaction inside
    *
    * as parameter @p accountId as this is the account that was created
    * after this wizard was left via the accept() method.
    *
    * @return MyMoneySchedule object for payments
    */
  const MyMoneySchedule schedule(void) const;

  /**
    * This method returns the id of the account to/from which
    * the payout should be created. If the checkbox that allows
    * to skip the creation of this transaction is checked, this
    * method returns QCString()
    *
    * @return id of account or empty QCString
    */
  const QCString initialPaymentAccount(void) const;

  /**
    * This method returns the date of the payout transaction.
    * If the checkbox that allows to skip the creation of
    * this transaction is checked, this method returns QDate()
    *
    * @return selected date or invalid QDate if checkbox is selected.
    */
  const QDate initialPaymentDate(void) const;

  void loadWidgets(const MyMoneyAccount& acc);

protected:
  /**
    * This method returns the transaction that is stored within
    * the schedule. See schedule().
    *
    * @return MyMoneyTransaction object to be used within the schedule
    */
  const MyMoneyTransaction transaction(void) const;

public slots:
  void next();

protected slots:
  void slotLiabilityLoan(void);
  void slotAssetLoan(void);
  virtual void slotCheckPageFinished(void);
  void slotPaymentsMade(void);
  void slotNoPaymentsMade(void);
  void slotRecordAllPayments(void);
  void slotRecordThisYearsPayments(void);
  void slotInterestOnPayment(void);
  void slotInterestOnReception(void);
  void slotCreateCategory(void);
  virtual void slotAdditionalFees(void);
  void slotNewPayee(const QString&);

protected:
  void loadComboBoxes(void);
  void loadAccountList(void);
  void resetCalculator(void);
  void updateLoanAmount(void);
  void updateInterestRate(void);
  void updateDuration(void);
  void updatePayment(void);
  void updateFinalPayment(void);
  void updateLoanInfo(void);
  const QString updateTermWidgets(const long double v);
  void updatePeriodicPayment(void);
  void updateSummary(void);
  int calculateLoan(void);
  int occurenceToPeriod(const MyMoneySchedule::occurenceE occurence) const;
  int term(void) const;

protected:
  MyMoneyAccountLoan  m_account;
  MyMoneyTransaction  m_transaction;
};

#endif
