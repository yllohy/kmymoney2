/***************************************************************************
                             knewuserwizard.h
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
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

#ifndef KNEWUSERWIZARD_H
#define KNEWUSERWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneywizard.h>

/**
  * @author Thomas Baumgart
  */

namespace NewUserWizard {

class GeneralPage;
class CurrencyPage;
class PasswordPage;

class Wizard : public KMyMoneyWizard
{
  Q_OBJECT
public:
  Wizard(QWidget* parent = 0, const char* name = 0, bool modal = false, WFlags flags = 0);

private:

  GeneralPage*      m_generalPage;
  CurrencyPage*     m_currencyPage;
  PasswordPage*     m_passwordPage;

  friend class GeneralPage;
  friend class CurrencyPage;
  friend class PasswordPage;
};

} // namespace
#endif
