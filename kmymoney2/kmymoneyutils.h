/***************************************************************************
                          kmymoneyutils.h  -  description
                             -------------------
    begin                : Wed Feb 5 2003
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

#ifndef KMYMONEYUTILS_H
#define KMYMONEYUTILS_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/mymoneyaccount.h"

/**
  * @author Thomas Baumgart
  */

class KMyMoneyUtils
{
public: 
  KMyMoneyUtils();
  ~KMyMoneyUtils();

  /**
    * This method is used to convert the internal representation of
    * an account type into a human readable format
    *
    * @param accountType numerical representation of the account type.
                         For possible values, see MyMoneyAccount::accountTypeE
    * @return QString representing the human readable form
    */
  static const QString accountTypeToString(const MyMoneyAccount::accountTypeE accountType);

  /**
    * This method is used to convert an account type from it's
    * string form to the internal used numeric value.
    *
    * @param accountType reference to a QString containing the string to convert
    * @return accountTypeE containing the internal used numeric value. For possible
    *         values see MyMoneyAccount::accountTypeE
    */
  static const MyMoneyAccount::accountTypeE stringToAccountType(const QString& type);

};

#endif
