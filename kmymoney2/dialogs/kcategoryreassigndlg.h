/***************************************************************************
                          kcategoryreassigndlg.cpp
                             -------------------
    copyright            : (C) 2007 by Thomas Baumgart
    author               : Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KCATEGORYREASSIGNDLG_H
#define KCATEGORYREASSIGNDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qvaluelist.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
#include "kmymoney2/dialogs/kcategoryreassigndlgdecl.h"

/**
 *  Implementation of the dialog that lets the user select a payee in order
 *  to re-assign transactions (for instance, if payees are deleted).
 */
class KCategoryReassignDlg : public KCategoryReassignDlgDecl
{
  Q_OBJECT
public:
  /** Default constructor */
  KCategoryReassignDlg( QWidget* parent = 0, const char* name = 0);

  /** Destructor */
  ~KCategoryReassignDlg();

  /**
    * This function sets up the dialog, lets the user select a category and returns
    * the id of the selected category in the list of all known income and expense accounts.
    *
    * @param category reference to MyMoneyAccount object of the category to be deleted
    *
    * @return Returns the id of the selected category in the list or QString() if
    *         the dialog was aborted. QString() is also returned if the @a category
    *         does not have an id.
    */
  QString show(const MyMoneyAccount& category);

protected:
  void accept(void);

};

#endif // KCATEGORYREASSIGNDLG_H
