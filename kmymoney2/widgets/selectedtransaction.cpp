/***************************************************************************
                          selectedtransaction.cpp  -  description
                             -------------------
    begin                : Fri Jun 2008
    copyright            : (C) 2000-2008 by Thomas Baumgart
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

#include "selectedtransaction.h"

#include "register.h"

namespace KMyMoneyRegister {
  
SelectedTransactions::SelectedTransactions(const Register* r)
{
  r->selectedTransactions(this);
}

} // namespace
