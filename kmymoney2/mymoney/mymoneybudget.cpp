/***************************************************************************
                          mymoneybudget.cpp
                             -------------------
    begin                : Sun July 4 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : acejones@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qdom.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneybudget.h"

const QStringList MyMoneyBudget::AccountGroup::kBudgetLevelText = QStringList::split(",","none,monthly,monthbymonth,yearly,invalid",true);
const int BUDGET_VERSION = 2;

MyMoneyBudget::MyMoneyBudget(void) :
  m_name("Unconfigured Budget")
{
}

MyMoneyBudget::MyMoneyBudget(const QString& _name) :
  m_name(_name)
{
}

MyMoneyBudget::MyMoneyBudget(const QDomElement& node) :
  MyMoneyObject(node)
{
  if(!read(node))
    clearId();
}

MyMoneyBudget::MyMoneyBudget(const QCString& id, const MyMoneyBudget& budget)
{
  *this = budget;
  m_id = id;
}

MyMoneyBudget::~MyMoneyBudget()
{
}

void MyMoneyBudget::write(QDomElement& e, QDomDocument *doc) const
{
  e.setAttribute("name",  m_name);
  e.setAttribute("id",    m_id );
  e.setAttribute("start", m_start.toString(Qt::ISODate) );
  e.setAttribute("version", BUDGET_VERSION);

  QMap<QCString, AccountGroup>::const_iterator it;
  for(it = m_accounts.begin(); it != m_accounts.end(); ++it) {
    // only add the account if there is a budget entered
    if(!(*it).balance().isZero()) {
      QDomElement domAccount = doc->createElement("ACCOUNT");
      domAccount.setAttribute("id", it.key());
      domAccount.setAttribute("parent", it.data().parentId());
      domAccount.setAttribute("default", it.data().getDefault());
      domAccount.setAttribute("budgetlevel", AccountGroup::kBudgetLevelText[it.data().budgetLevel()]);
      domAccount.setAttribute("budgetsubaccounts", it.data().budgetSubaccounts());

      const QMap<QDate, PeriodGroup> periods = it.data().getPeriods();
      QMap<QDate, PeriodGroup>::const_iterator it_per;
      for(it_per = periods.begin(); it_per != periods.end(); ++it_per) {
        if(!(*it_per).amount().isZero()) {
          QDomElement domPeriod = doc->createElement("PERIOD");

          domPeriod.setAttribute("amount", (*it_per).amount().toString());
          domPeriod.setAttribute("start", (*it_per).startDate().toString(Qt::ISODate));
          domAccount.appendChild(domPeriod);
        }
      }

      e.appendChild(domAccount);
    }
  }
}

bool MyMoneyBudget::read(const QDomElement& e)
{
  // The goal of this reading method is 100% backward AND 100% forward
  // compatability.  Any Budget ever created with any version of KMyMoney
  // should be able to be loaded by this method (as long as it's one of the
  // Budget types supported in this version, of course)

  bool result = false;

  if ("BUDGET" == e.tagName())
  {
    result = true;
    m_name  = e.attribute("name");
    m_start = QDate::fromString(e.attribute("start"), Qt::ISODate);
    m_id    = e.attribute("id");

    QDomNode child = e.firstChild();
    while(!child.isNull() && child.isElement())
    {
      QDomElement c = child.toElement();

      AccountGroup account;

      if("ACCOUNT" == c.tagName() && c.hasAttribute("id"))
      {
        account.setId(c.attribute("id"));
      }

      if("ACCOUNT" == c.tagName() && c.hasAttribute("parent"))
      {
        account.setParentId(c.attribute("parent"));
      }

      if("ACCOUNT" == c.tagName() && c.hasAttribute("default"))
      {
        account.setDefault(c.attribute("default").toUInt());
      }

      if("ACCOUNT" == c.tagName() && c.hasAttribute("budgetlevel"))
      {

        int i = AccountGroup::kBudgetLevelText.findIndex(c.attribute("budgetlevel"));
        if ( i != -1 )
          account.setBudgetLevel(static_cast<AccountGroup::eBudgetLevel>(i));
      }

      if("ACCOUNT" == c.tagName() && c.hasAttribute("budgetsubaccounts"))
      {
        account.setBudgetSubaccounts(c.attribute("budgetsubaccounts").toUInt());
      }

      QDomNode period = c.firstChild();
      while(!period.isNull() && period.isElement())
      {
        QDomElement per = period.toElement();
        PeriodGroup pGroup;

        if("PERIOD" == per.tagName() && per.hasAttribute("amount") && per.hasAttribute("start"))
        {
          pGroup.setAmount( MyMoneyMoney(per.attribute("amount")) );
          pGroup.setStartDate( QDate::fromString(per.attribute("start"), Qt::ISODate) );
          account.addPeriod(pGroup.startDate(), pGroup);
        }

        period = period.nextSibling();
      }

      m_accounts[account.id()] = account;

      child = child.nextSibling();
    }
  }

  return result;
}

void MyMoneyBudget::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("BUDGET");
  write(el,&document);
  parent.appendChild(el);
}

bool MyMoneyBudget::hasReferenceTo(const QCString& id) const
{
  // return true if we have a non-zero assignment for this id
  return (m_accounts.contains(id) && !m_accounts[id].balance().isZero());
}

void MyMoneyBudget::removeReference(const QCString& id)
{
  if(m_accounts.contains(id)) {
    qDebug("%s", (QString("Remove account '%1' from budget").arg(id)).data());
    m_accounts.remove(id);
  }
}

const MyMoneyBudget::AccountGroup& MyMoneyBudget::account(const QCString _id) const
{
  static AccountGroup empty;

  if ( m_accounts.contains(_id) )
    return m_accounts[_id];
  else
    return empty;
}

void MyMoneyBudget::setBudgetStart(const QDate& _start)
{
  QDate oldDate = QDate(m_start.year(), m_start.month(), 1);
  m_start = QDate(_start.year(), _start.month(), 1);
  if(oldDate.isValid()) {
    int adjust = ((m_start.year() - oldDate.year())*12) + (m_start.month() - oldDate.month());
    QMap<QCString, AccountGroup>::iterator it;
    for(it = m_accounts.begin(); it != m_accounts.end(); ++it) {
      const QMap<QDate, PeriodGroup> periods = (*it).getPeriods();
      QMap<QDate, PeriodGroup>::const_iterator it_per;
      (*it).clearPeriods();
      for(it_per = periods.begin(); it_per != periods.end(); ++it_per) {
        PeriodGroup pgroup = (*it_per);
        pgroup.setStartDate(pgroup.startDate().addMonths(adjust));
        (*it).addPeriod(pgroup.startDate(), pgroup);
      }
    }
  }
}

// vim:cin:si:ai:et:ts=2:sw=2:
