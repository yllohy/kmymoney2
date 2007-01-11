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
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneyaccountselector.h>

#include "investactivities.h"

using namespace Invest;
using namespace KMyMoneyRegister;

bool Activity::isComplete(void) const
{
  bool rc = false;
  KMyMoneySecurity* security = dynamic_cast<KMyMoneySecurity*>(haveWidget("security"));
  if(!security->currentText().isEmpty()) {
    rc = security->selector()->contains(security->currentText());
  }
  return rc;
}

bool Activity::haveAssetAccount(void) const
{
  KMyMoneyCategory* cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));

  bool rc = true;
  if(!isMultiSelection())
    rc = !cat->currentText().isEmpty();

  if(rc && !cat->currentText().isEmpty()) {
    rc = cat->selector()->contains(cat->currentText());
  }
  return rc;
}

bool Activity::haveCategoryAndAmount(const QString& category, const QString& amount, bool optional) const
{
  KMyMoneyCategory* cat = dynamic_cast<KMyMoneyCategory*>(haveWidget(category));

  bool rc = true;
  if(!isMultiSelection() && !optional)
    rc = !cat->currentText().isEmpty();

  if(rc && !cat->currentText().isEmpty()) {
    rc = cat->selector()->contains(cat->currentText()) || cat->isSplitTransaction();
    if(rc) {
      MyMoneyMoney value = dynamic_cast<kMyMoneyEdit*>(haveWidget(amount))->value();
      if(!isMultiSelection())
        rc = !value.isZero();
    }
  }
  return rc;
}

bool Activity::haveShares(void) const
{
  kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
  if(isMultiSelection() && amount->text().isEmpty())
    return true;

  return !amount->value().isZero();
}

bool Activity::havePrice(void) const
{
  kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(haveWidget("price"));
  if(isMultiSelection() && amount->text().isEmpty())
    return true;

  return !amount->value().isZero();
}

void Buy::showWidgets(void) const
{
  KMyMoneyCategory* cat;
  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
  cat->parentWidget()->show();
  haveWidget("asset-account")->show();
  haveWidget("shares")->show();
  haveWidget("price")->show();
  haveWidget("total")->show();
}

bool Buy::isComplete(void) const
{
  bool rc = Activity::isComplete();
  rc &= haveAssetAccount();
  rc &= haveFees(true);
  rc &= haveShares();
  rc &= havePrice();

  return rc;
}

void Sell::showWidgets(void) const
{
  KMyMoneyCategory* cat;
  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
  cat->parentWidget()->show();
  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
  cat->parentWidget()->show();
  haveWidget("asset-account")->show();
  haveWidget("shares")->show();
  haveWidget("price")->show();
  haveWidget("total")->show();
}

bool Sell::isComplete(void) const
{
  bool rc = Activity::isComplete();
  rc &= haveAssetAccount();
  rc &= haveFees(true);
  rc &= haveInterest(true);
  rc &= haveShares();
  rc &= havePrice();
  return rc;
}

void Div::showWidgets(void) const
{
  KMyMoneyCategory* cat;
  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
  cat->parentWidget()->show();
  haveWidget("asset-account")->show();
  haveWidget("total")->show();
}

bool Div::isComplete(void) const
{
  bool rc = Activity::isComplete();
  rc &= haveAssetAccount();
  rc &= haveInterest(false);
  return rc;
}

void Reinvest::showWidgets(void) const
{
  KMyMoneyCategory* cat;
  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
  cat->parentWidget()->show();
  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
  cat->parentWidget()->show();
  haveWidget("shares")->show();
  haveWidget("price")->show();
  haveWidget("total")->show();
}

bool Reinvest::isComplete(void) const
{
  bool rc = Activity::isComplete();
  rc &= haveInterest(false);
  rc &= haveFees(true);
  rc &= haveShares();
  rc &= havePrice();
  return rc;
}

void Add::showWidgets(void) const
{
  haveWidget("shares")->show();
}

bool Add::isComplete(void) const
{
  bool rc = Activity::isComplete();
  rc &= haveShares();
  return rc;
}

void Remove::showWidgets(void) const
{
  haveWidget("shares")->show();
}

bool Remove::isComplete(void) const
{
  bool rc = Activity::isComplete();
  rc &= haveShares();
  return rc;
}

void Split::showWidgets(void) const
{
  // FIXME do we need a split ratio widget?
  haveWidget("shares")->show();
}

bool Split::isComplete(void) const
{
  bool rc = Activity::isComplete();
  rc = false; // TODO to be implemented

  return rc;
}


