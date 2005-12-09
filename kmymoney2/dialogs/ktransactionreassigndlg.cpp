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

// ----------------------------------------------------------------------------
// QT Includes

#include <qcombobox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdialog.h>
#include <klocale.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ktransactionreassigndlg.h"

KTransactionReassignDlg::KTransactionReassignDlg( QWidget* parent, const char* name) :
  KTransactionReassignDlgDecl( parent, name)
{
  buttonOk->setGuiItem(KStdGuiItem::ok());
  buttonCancel->setGuiItem(KStdGuiItem::cancel());
}

KTransactionReassignDlg::~KTransactionReassignDlg()
{
}

int KTransactionReassignDlg::show(const QValueList<MyMoneyPayee>& payeeslist)
{
  if (payeeslist.isEmpty())
   return -1; // no payee available? nothing can be selected...

  // transfer data from payeeslist into combobox
  payeeCombo->clear();
  for (QValueList<MyMoneyPayee>::const_iterator it = payeeslist.begin();
    it != payeeslist.end(); ++it)
  {
    payeeCombo->insertItem( (*it).name(), -1);
  }

  // execute dialog and if aborted, return -1
  if (this->exec() == QDialog::Rejected)
    return -1;

  // otherwise return index of selected payee
  return payeeCombo->currentItem();
}

#include "ktransactionreassigndlg.moc"
