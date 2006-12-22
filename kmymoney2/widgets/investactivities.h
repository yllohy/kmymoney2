/***************************************************************************
                             investactivities.h
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

#ifndef INVESTACTIVITIES_H
#define INVESTACTIVITIES_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/register.h>

namespace Invest {

class Activity
{
protected:
  Activity() {}
public:
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const = 0;
  virtual void showWidgets(const KMyMoneyRegister::QWidgetContainer& editWidgets) const = 0;
  virtual bool isComplete(void) const = 0;
};

class Buy : public Activity
{
public:
  Buy() : Activity() {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::BuyShares; }
  virtual void showWidgets(const KMyMoneyRegister::QWidgetContainer& editWidgets) const;
  virtual bool isComplete(void) const;
};

class Sell : public Activity
{
public:
  Sell() : Activity() {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::SellShares; }
  virtual void showWidgets(const KMyMoneyRegister::QWidgetContainer& editWidgets) const;
  virtual bool isComplete(void) const;
};

class Div : public Activity
{
public:
  Div() : Activity() {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::Dividend; }
  virtual void showWidgets(const KMyMoneyRegister::QWidgetContainer& editWidgets) const;
  virtual bool isComplete(void) const;
};

class Reinvest : public Activity
{
public:
  Reinvest() : Activity() {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::ReinvestDividend; }
  virtual void showWidgets(const KMyMoneyRegister::QWidgetContainer& editWidgets) const;
  virtual bool isComplete(void) const;
};

class Add : public Activity
{
public:
  Add() : Activity() {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::AddShares; }
  virtual void showWidgets(const KMyMoneyRegister::QWidgetContainer& editWidgets) const;
  virtual bool isComplete(void) const;
};

class Remove : public Activity
{
public:
  Remove() : Activity() {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::RemoveShares; }
  virtual void showWidgets(const KMyMoneyRegister::QWidgetContainer& editWidgets) const;
  virtual bool isComplete(void) const;
};

class Split : public Activity
{
public:
  Split() : Activity() {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::SplitShares; }
  virtual void showWidgets(const KMyMoneyRegister::QWidgetContainer& editWidgets) const;
  virtual bool isComplete(void) const;
};

} // namespace Invest



#endif // INVESTACTIVITIES_H

