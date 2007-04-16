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
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ktransactionreassigndlg.h"
#include <kmymoney/kmymoneycombo.h>
#include <kmymoney/kguiutils.h>

KTransactionReassignDlg::KTransactionReassignDlg( QWidget* parent, const char* name) :
  KTransactionReassignDlgDecl( parent, name)
{
  buttonOk->setGuiItem(KStdGuiItem::ok());
  buttonCancel->setGuiItem(KStdGuiItem::cancel());
  kMandatoryFieldGroup* mandatory = new kMandatoryFieldGroup(this);
  mandatory->add(payeeCombo);
  mandatory->setOkButton(buttonOk);
}

KTransactionReassignDlg::~KTransactionReassignDlg()
{
}

QCString KTransactionReassignDlg::show(const QValueList<MyMoneyPayee>& payeeslist)
{
  if (payeeslist.isEmpty())
   return QCString(); // no payee available? nothing can be selected...

  payeeCombo->loadPayees(payeeslist);

  // execute dialog and if aborted, return empty string
  if (this->exec() == QDialog::Rejected)
    return QCString();

  // otherwise return index of selected payee
  return payeeCombo->selectedItem();
}


void KTransactionReassignDlg::accept(void)
{
  // force update of payeeCombo
  buttonOk->setFocus();

  if(payeeCombo->selectedItem().isEmpty()) {
    KMessageBox::information(this, i18n("This dialog does not allow to create new payees. Please pick a payee from the list."), i18n("Payee creation"));
  } else {
    KTransactionReassignDlgDecl::accept();
  }
}

#include "ktransactionreassigndlg.moc"
