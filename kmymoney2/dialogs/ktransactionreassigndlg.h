/***************************************************************************
                          ktransactionreassigndlg.cpp
                             -------------------
    copyright            : (C) 2005 by Andreas Nicolai, Thomas Baumgart
    author               : Andreas Nicolai
    email                : ghorwin@users.sourceforge.net
                           ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KTRANSACTIONREASSIGNDLG_H
#define KTRANSACTIONREASSIGNDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qvaluelist.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneypayee.h>
#include "kmymoney2/dialogs/ktransactionreassigndlgdecl.h"

/**
 *  Implementation of the dialog that lets the user select a payee in order
 *  to re-assign transactions (for instance, if payees are deleted).
 */
class KTransactionReassignDlg : public KTransactionReassignDlgDecl
{
  Q_OBJECT
public:
  /** Default constructor */
  KTransactionReassignDlg( QWidget* parent = 0, const char* name = 0);

  /** Destructor */
  ~KTransactionReassignDlg();

  /**
    * This function sets up the dialog, lets the user select a payee and returns
    * the index of the selected payee in the payeeslist.
    *
    * @param payeeslist reference to QValueList of MyMoneyPayee objects to be contained in the list
    *
    * @return Returns the index of the selected payee in the list or -1 if
    *         the dialog was aborted. -1 is also returned if the payeeslist is empty.
    */
  int show(const QValueList<MyMoneyPayee>& payeeslist);
};

#endif // KTRANSACTIONREASSIGNDLG_H
