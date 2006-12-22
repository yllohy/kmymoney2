/***************************************************************************
                             investactivities.cpp
                             ----------
    begin                : Fri Dec 15 2006
    copyright            : (C) 2006 by Thomas Baumgart
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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneycategory.h>

#include "investactivities.h"

using namespace Invest;
using namespace KMyMoneyRegister;

void Buy::showWidgets(const QWidgetContainer& editWidgets) const
{
  KMyMoneyCategory* cat;
  cat = dynamic_cast<KMyMoneyCategory*>(editWidgets.haveWidget("fee-account"));
  cat->parentWidget()->show();
  editWidgets.haveWidget("asset-account")->show();
  editWidgets.haveWidget("shares")->show();
  editWidgets.haveWidget("price")->show();
  editWidgets.haveWidget("total")->show();
}

bool Buy::isComplete(void) const
{
  return false;
}

void Sell::showWidgets(const QWidgetContainer& editWidgets) const
{
  KMyMoneyCategory* cat;
  cat = dynamic_cast<KMyMoneyCategory*>(editWidgets.haveWidget("interest-account"));
  cat->parentWidget()->show();
  cat = dynamic_cast<KMyMoneyCategory*>(editWidgets.haveWidget("fee-account"));
  cat->parentWidget()->show();
  editWidgets.haveWidget("asset-account")->show();
  editWidgets.haveWidget("shares")->show();
  editWidgets.haveWidget("price")->show();
  editWidgets.haveWidget("total")->show();
}

bool Sell::isComplete(void) const
{
  return false;
}

void Div::showWidgets(const QWidgetContainer& editWidgets) const
{
  KMyMoneyCategory* cat;
  cat = dynamic_cast<KMyMoneyCategory*>(editWidgets.haveWidget("interest-account"));
  cat->parentWidget()->show();
  editWidgets.haveWidget("asset-account")->show();
  editWidgets.haveWidget("total")->show();
}

bool Div::isComplete(void) const
{
  return false;
}

void Reinvest::showWidgets(const QWidgetContainer& editWidgets) const
{
  KMyMoneyCategory* cat;
  cat = dynamic_cast<KMyMoneyCategory*>(editWidgets.haveWidget("interest-account"));
  cat->parentWidget()->show();
  cat = dynamic_cast<KMyMoneyCategory*>(editWidgets.haveWidget("fee-account"));
  cat->parentWidget()->show();
  editWidgets.haveWidget("asset-account")->show();
  editWidgets.haveWidget("shares")->show();
  editWidgets.haveWidget("price")->show();
  editWidgets.haveWidget("total")->show();
}

bool Reinvest::isComplete(void) const
{
  return false;
}

void Add::showWidgets(const QWidgetContainer& editWidgets) const
{
  editWidgets.haveWidget("shares")->show();
}

bool Add::isComplete(void) const
{
  return false;
}

void Remove::showWidgets(const QWidgetContainer& editWidgets) const
{
  editWidgets.haveWidget("shares")->show();
}

bool Remove::isComplete(void) const
{
  return false;
}

void Split::showWidgets(const QWidgetContainer& editWidgets) const
{
  // FIXME do we need a split ratio widget?
  editWidgets.haveWidget("shares")->show();
}

bool Split::isComplete(void) const
{
  return false;
}


