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
#include "kpayeedlg.h"

KPayeeDlg::KPayeeDlg(MyMoneyFile *file, QWidget *parent, const char *name)
 : KPayeeDlgDecl(parent,name,true)
{
	m_file = file;
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  for ( ; it.current(); ++it)
    payeeCombo->insertItem(it.current()->name());    	

  connect(addressEdit, SIGNAL(textChanged()), SLOT(addressEditChanged()));
  connect(postcodeEdit, SIGNAL(textChanged(const QString&)), SLOT(postcodeEditChanged(const QString&)));
  connect(telephoneEdit, SIGNAL(textChanged(const QString&)), SLOT(telephoneEditChanged(const QString&)));
  connect(emailEdit, SIGNAL(textChanged(const QString&)), SLOT(emailEditChanged(const QString&)));

  connect(payeeCombo, SIGNAL(activated(const QString&)), this, SLOT(payeeHighlighted(const QString&)));
//  connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
}

KPayeeDlg::~KPayeeDlg()
{
}

void KPayeeDlg::payeeHighlighted(const QString& text)
{
  MyMoneyPayee *payee;
  bool found=false;
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  for ( ; it.current(); ++it) {
    payee = it.current();
    if (payee->name() == text)
      found=true;
  }
  if (found) {
    nameLabel->setText(payee->name());
    addressEdit->setText(payee->address());
    postcodeEdit->setText(payee->postcode());
    telephoneEdit->setText(payee->telephone());
    emailEdit->setText(payee->email());
  }
}

void KPayeeDlg::addressEditChanged()
{
  QString text = addressEdit->text();
  MyMoneyPayee *payee;
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  for ( ; it.current(); ++it) {
    payee = it.current();
    if (payee->name() == nameLabel->text())
      payee->setAddress(text);
  }
}

void KPayeeDlg::postcodeEditChanged(const QString& text)
{
  MyMoneyPayee *payee;
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  for ( ; it.current(); ++it) {
    payee = it.current();
    if (payee->name() == nameLabel->text())
      payee->setPostcode(text);
  }
}

void KPayeeDlg::telephoneEditChanged(const QString& text)
{
  MyMoneyPayee *payee;
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  for ( ; it.current(); ++it) {
    payee = it.current();
    if (payee->name() == nameLabel->text())
      payee->setTelephone(text);
  }
}

void KPayeeDlg::emailEditChanged(const QString& text)
{
  MyMoneyPayee *payee;
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  for ( ; it.current(); ++it) {
    payee = it.current();
    if (payee->name() == nameLabel->text())
      payee->setEmail(text);
  }
}
