/***************************************************************************
                          objectinfotable.h
                         -------------------
    begin                : Sat 28 jun 2008
    copyright            : (C) 2004-2005 by Ace Jones
                               2008 by Alvaro Soliverez
    email                :  acejones@users.sourceforge.net
                            asoliverez@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OBJECTINFOTABLE_H
#define OBJECTINFOTABLE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qstringlist.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyreport.h"
#include "listtable.h"

namespace reports {

class ReportAccount;

/**
  * Calculates a query of information about the transaction database.
  *
  * This is a middle-layer class, between the UI and the engine.  The
  * MyMoneyReport class holds only the CONFIGURATION parameters.  This
  * class actually does the work of retrieving the data from the engine
  * and formatting it for the user.
  *
  * @author Ace Jones
  *
  * @short
**/

class ObjectInfoTable : public ListTable
{
public:
  ObjectInfoTable ( const MyMoneyReport& );
  void init ( void );

protected:
  void constructScheduleTable ( void );
  void constructAccountTable ( void );
  void constructAccountLoanTable ( void );

private:
  /**
    * @param acc the investment account
    * @return the balance in the currency of the investment account
    */
  MyMoneyMoney investmentBalance(const MyMoneyAccount& acc);
};

}

#endif // QUERYREPORT_H
