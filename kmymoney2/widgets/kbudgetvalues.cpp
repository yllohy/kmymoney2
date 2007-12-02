/***************************************************************************
                          kbudgetvalues  -  description
                             -------------------
    begin                : Wed Nov 28 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include <qtabwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kbudgetvalues.h"
#include <kmymoney/kmymoneyedit.h>

KBudgetValues::KBudgetValues(QWidget* parent, const char* name) :
  KBudgetValuesDecl(parent, name)
{
  m_field[0] = m_amountJan;
  m_field[1] = m_amountFeb;
  m_field[2] = m_amountMar;
  m_field[3] = m_amountApr;
  m_field[4] = m_amountMay;
  m_field[5] = m_amountJun;
  m_field[6] = m_amountJul;
  m_field[7] = m_amountAug;
  m_field[8] = m_amountSep;
  m_field[9] = m_amountOct;
  m_field[10] = m_amountNov;
  m_field[11] = m_amountDec;

  connect(m_budgetLevel, SIGNAL(currentChanged(QWidget*)), this, SIGNAL(valuesChanged()));
  connect(m_amountMonthly, SIGNAL(valueChanged(const QString&)), this, SIGNAL(valuesChanged()));
  connect(m_amountYearly, SIGNAL(valueChanged(const QString&)), this, SIGNAL(valuesChanged()));
  for(int i=0; i < 12; ++i)
    connect(m_field[i], SIGNAL(valueChanged(const QString&)), this, SIGNAL(valuesChanged()));

}


KBudgetValues::~KBudgetValues()
{
}

void KBudgetValues::clear(void)
{
  blockSignals(true);
  for(int i=0; i < 12; ++i)
    m_field[i]->setValue(MyMoneyMoney());
  m_amountMonthly->setValue(MyMoneyMoney());
  m_amountYearly->setValue(MyMoneyMoney());
  blockSignals(false);
}

void KBudgetValues::setBudgetValues(const MyMoneyBudget& budget, const MyMoneyBudget::AccountGroup& budgetAccount)
{
  MyMoneyBudget::PeriodGroup period;
  QDate date = budget.budgetStart();

  blockSignals(true);
  switch(budgetAccount.budgetLevel()) {
    case MyMoneyBudget::AccountGroup::eMonthly:
    default:
      m_budgetLevel->showPage(m_monthTab);
      m_amountMonthly->setValue(budgetAccount.period(date).amount());
      break;
    case MyMoneyBudget::AccountGroup::eYearly:
      m_budgetLevel->showPage(m_yearTab);
      m_amountYearly->setValue(budgetAccount.period(date).amount());
      break;
    case MyMoneyBudget::AccountGroup::eMonthByMonth:
      m_budgetLevel->showPage(m_individualTab);
      date.setYMD(date.year(), 1, 1);
      for(int i = 0; i < 12; ++i) {
        m_field[i]->setValue(budgetAccount.period(date).amount());
        date = date.addMonths(1);
      }
      break;
  }
  blockSignals(false);
}

void KBudgetValues::budgetValues(const MyMoneyBudget& budget, MyMoneyBudget::AccountGroup& budgetAccount)
{
  MyMoneyBudget::PeriodGroup period;
  QDate date = budget.budgetStart();
  period.setStartDate(date);

  budgetAccount.clearPeriods();
  if(m_budgetLevel->currentPage() == m_monthTab) {
    budgetAccount.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthly);
    period.setAmount(m_amountMonthly->value());
    budgetAccount.addPeriod(date, period);

  } else if(m_budgetLevel->currentPage() == m_yearTab) {
    budgetAccount.setBudgetLevel(MyMoneyBudget::AccountGroup::eYearly);
    period.setAmount(m_amountYearly->value());
    budgetAccount.addPeriod(date, period);

  } else if(m_budgetLevel->currentPage() == m_individualTab) {
    budgetAccount.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthByMonth);
    date.setYMD(date.year(), 1, 1);
    for(int i = 0; i < 12; ++i) {
      period.setStartDate(date);
      period.setAmount(m_field[i]->value());
      budgetAccount.addPeriod(date, period);
      date = date.addMonths(1);
    }

  } else {
    qDebug("Budget unchanged. Unknown tab");
  }
}

#include "kbudgetvalues.moc"
