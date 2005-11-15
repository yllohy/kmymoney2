/***************************************************************************
                          keditloanwizard.h  -  description
                             -------------------
    begin                : Wed Nov 12 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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

#ifndef KEDITLOANWIZARD_H
#define KEDITLOANWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


#include <kmymoney/mymoneyaccount.h>

/**
  * @author Thomas Baumgart
  */

class KEditLoanWizard : public KNewLoanWizard
{
  Q_OBJECT
public:
  KEditLoanWizard(const MyMoneyAccount& account, QWidget *parent=0, const char *name=0);
  ~KEditLoanWizard();

  /**
    * This method returns the schedule for the payments. The account
    * where the amortization should be transferred to is the one
    * we currently edited with this wizard.
    *
    * @return MyMoneySchedule object for payments
    */
  const MyMoneySchedule schedule(void) const;

  /**
    * This method returns a MyMoneyAccount object with all data
    * filled out as provided by the wizard.
    *
    * @return updated MyMoneyAccount object
    */
  const MyMoneyAccount account(void) const;


  void loadWidgets(const MyMoneyAccount& acc);

  const MyMoneyTransaction transaction() const;

public slots:
  void next();

protected slots:
  virtual void slotCheckPageFinished(void);

protected:
  void updateEditSummary(void);

private:
  //MyMoneyAccountLoan  m_account;
  MyMoneySchedule     m_schedule;
  int                 m_lastSelection;
  bool                m_fullyRepayLoan;
};

#endif
