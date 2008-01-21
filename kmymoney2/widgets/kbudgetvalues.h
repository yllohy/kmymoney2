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

#ifndef KBUDGETVALUES_H
#define KBUDGETVALUES_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qdatetime.h>
class QLabel;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../widgets/kbudgetvaluesdecl.h"
#include <kmymoney/mymoneybudget.h>
class kMyMoneyEdit;

/**
 * @author Thomas Baumgart <ipwizard@users.sourceforge.net>
 */
class KBudgetValues : public KBudgetValuesDecl
{
  Q_OBJECT
  public:
    KBudgetValues(QWidget* parent = 0, const char* name = 0);
    ~KBudgetValues();

    void setBudgetValues(const MyMoneyBudget& budget, const MyMoneyBudget::AccountGroup& budgetAccount);
    void budgetValues(const MyMoneyBudget& budget, MyMoneyBudget::AccountGroup& budgetAccount);
    void clear(void);

  private:
    void enableMonths(bool enabled);
    void fillMonthLabels(void);

  protected slots:
    void slotChangePeriod(int id);

    /**
     * This slot clears the value in the value widgets of the selected budget type.
     * Values of the other types are unaffected.
     */
    void slotClearAllValues(void);

    /**
     * Helper slot used to postpone sending the valuesChanged() signal.
     */
    void slotNeedUpdate(void);

    void slotUpdateClearButton(void);

  protected:
    bool eventFilter(QObject* o, QEvent* e);

  private:
    kMyMoneyEdit*   m_field[12];
    QLabel*         m_label[12];
    QWidget*        m_currentTab;
    QDate           m_budgetDate;

  signals:
    void valuesChanged(void);
};

#endif
