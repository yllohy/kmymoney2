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

#include "mymoneyfile.h"
#include "mymoneybudget.h"

MyMoneyBudget::MyMoneyBudget(void):
    m_name("Unconfigured Budget")
{
}

MyMoneyBudget::MyMoneyBudget(const QString& _name):
    m_name(_name)
{
}

MyMoneyBudget::MyMoneyBudget(const QDomElement& node) :
  MyMoneyObject(node)
{
  if(!read(node))
    clearId();
}

MyMoneyBudget::~MyMoneyBudget()
{
	m_name = "";
}

void MyMoneyBudget::write(QDomElement& e, QDomDocument *doc, bool anonymous) const
{
  // No matter what changes, be sure to have a 'type' attribute.  Only change
  // the major type if it becomes impossible to maintain compatability with
  // older versions of the program as new features are added to the Budgets.
  // Feel free to change the minor type every time a change is made here.

  e.setAttribute("name", m_name);
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
    m_name = e.attribute("name");
  }
  return result;
}

void MyMoneyBudget::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("BUDGET");
  write(el,&document,false);
  parent.appendChild(el);
}

bool MyMoneyBudget::hasReferenceTo(const QCString& id) const
{
  QCStringList list;

  // collect all ids
  accounts(list);
  categories(list);
  payees(list);

  return (list.contains(id) > 0);
}

// vim:cin:si:ai:et:ts=2:sw=2:
