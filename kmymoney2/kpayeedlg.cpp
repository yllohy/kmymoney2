/***************************************************************************
                          kpayeedlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qpushbutton.h>
#include "kpayeedlg.h"

KPayeeDlg::KPayeeDlg(MyMoneyFile *file, QWidget *parent, const char *name)
 : KPayeeDlgDecl(parent,name,true)
{
	m_file = file;
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  for ( ; it.current(); ++it)
    payeeCombo->insertItem(it.current()->name());    	

  connect(payeeCombo, SIGNAL(activated(const QString&)), this, SLOT(payeeHighlighted(const QString&)));
  connect(addButton, SIGNAL(clicked()), this, SLOT(slotAddClicked()));
  connect(payeeEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeTextChanged(const QString&)));
  connect(updateButton, SIGNAL(clicked()), this, SLOT(slotUpdateClicked()));
}

KPayeeDlg::~KPayeeDlg()
{
}

void KPayeeDlg::payeeHighlighted(const QString& text)
{
  MyMoneyPayee *payee=0;
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  for ( ; it.current(); ++it) {
    payee = it.current();
    if (payee->name() == text) {
      nameLabel->setText(payee->name());
      addressEdit->setEnabled(true);
      addressEdit->setText(payee->address());
      postcodeEdit->setEnabled(true);
      postcodeEdit->setText(payee->postcode());
      telephoneEdit->setEnabled(true);
      telephoneEdit->setText(payee->telephone());
      emailEdit->setEnabled(true);
      emailEdit->setText(payee->email());
      updateButton->setEnabled(true);
    }
  }
}

void KPayeeDlg::slotAddClicked()
{
  m_file->addPayee(payeeEdit->text());
  payeeEdit->setText("");
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  payeeCombo->clear();
  for ( ; it.current(); ++it)
    payeeCombo->insertItem(it.current()->name());    	
}

void KPayeeDlg::slotPayeeTextChanged(const QString& text)
{
  if (text.isEmpty())
    addButton->setEnabled(false);
  else
    addButton->setEnabled(true);
}

void KPayeeDlg::slotUpdateClicked()
{
  MyMoneyPayee *payee;
  bool found=false;
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  for ( ; it.current(); ++it) {
    payee = it.current();
    if (payee->name() == nameLabel->text()) {
      payee->setAddress(addressEdit->text());
      payee->setPostcode(postcodeEdit->text());
      payee->setTelephone(telephoneEdit->text());
      payee->setEmail(emailEdit->text());
    }
  }
}
