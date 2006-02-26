/***************************************************************************
                         kmymoneyaccounttreebudget.cpp  -  description
                            -------------------
   begin                : Tue Feb 21 2006
   copyright            : (C) 2005 by Darren Gould
   email                : Darren Gould <darren_gould@gmx.de>
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

#include <qpoint.h>
#include <qevent.h>
#include <qdragobject.h>
#include <qtimer.h>
#include <qcursor.h>
#include <qheader.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes
#include <kmymoneyaccounttreebudget.h>

/**
  * @todo drag/drop in KMyMoneyAccountTree
  */

KMyMoneyAccountTreeBudget::KMyMoneyAccountTreeBudget(QWidget* parent, const char* name) :
  KMyMoneyAccountTree::KMyMoneyAccountTree(parent, name)
{
}

KMyMoneyAccountTreeBudgetItem* KMyMoneyAccountTreeBudget::selectedItem(void) const
{
  return dynamic_cast<KMyMoneyAccountTreeBudgetItem *>(KMyMoneyAccountTree::selectedItem());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

KMyMoneyAccountTreeBudgetItem::KMyMoneyAccountTreeBudgetItem(KListView *parent, const MyMoneyAccount& account, const MyMoneyBudget &budget, const MyMoneySecurity& security, const QString& name) :
  KMyMoneyAccountTreeItem(parent, account, security, name),
  m_budget(budget)
{
}

KMyMoneyAccountTreeBudgetItem::KMyMoneyAccountTreeBudgetItem(KMyMoneyAccountTreeBudgetItem *parent, const MyMoneyAccount& account, const MyMoneyBudget& budget, const QValueList<MyMoneyPrice>& price, const MyMoneySecurity& security) :
  KMyMoneyAccountTreeItem(parent, account, price, security),
  m_budget(budget)
{
}


KMyMoneyAccountTreeBudgetItem::~KMyMoneyAccountTreeBudgetItem()
{
}


MyMoneyMoney KMyMoneyAccountTreeBudgetItem::balance( const MyMoneyAccount& account ) const
{
  MyMoneyBudget::AccountGroup budgetAccount = m_budget.account( account.id() );
  if ( budgetAccount.id() != account.id() )
    return budgetAccount.balance();
  else
    return MyMoneyMoney();
}
#include "kmymoneyaccounttreebudget.moc"
