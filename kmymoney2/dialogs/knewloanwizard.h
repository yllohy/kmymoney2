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

/**
  * @author Thomas Baumgart
  */

class KNewLoanWizard : public KNewLoanWizardDecl
{
  Q_OBJECT
public: 
  KNewLoanWizard(QWidget *parent=0, const char *name=0);
  ~KNewLoanWizard();

public slots:
  void next();

protected slots:
  void slotLiabilityLoan(void);
  void slotAssetLoan(void);
  void slotFixedInterestRate(void);
  void slotVariableInterestRate(void);
  void slotCheckPageFinished(void);
  void slotPaymentsMade(void);
  void slotNoPaymentsMade(void);
  void slotRecordAllPayments(void);
  void slotRecordThisYearsPayments(void);
  void slotInterestOnPayment(void);
  void slotInterestOnReception(void);
  void slotCreateCategory(void);
  void slotAdditionalFees(void);
    
private:
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
  int calculateLoan(void);
  int occurenceToFrequency(const MyMoneySchedule::occurenceE occurence) const;
  int occurenceToPeriod(const MyMoneySchedule::occurenceE occurence) const;

};

#endif
