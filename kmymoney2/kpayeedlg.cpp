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
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <qpixmap.h>

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qpushbutton.h>
#include "kpayeedlg.h"

KPayeeDlg::KPayeeDlg(MyMoneyFile *file, QWidget *parent, const char *name)
 : KPayeeDlgDecl(parent,name,true)
{
  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_payees.png");
  QPixmap pm(filename);
  m_qpixmaplabel->setPixmap(pm);
	m_file = file;

  readConfig();

  int pos=0, k=0;
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  for ( ; it.current(); ++it, k++) {
    payeeCombo->insertItem(it.current()->name());
    if (it.current()->name()==m_lastPayee)
      pos = k;
  }
  payeeCombo->setCurrentItem(pos);

  payeeHighlighted(payeeCombo->currentText());

  connect(payeeCombo, SIGNAL(activated(const QString&)), this, SLOT(payeeHighlighted(const QString&)));
  connect(addButton, SIGNAL(clicked()), this, SLOT(slotAddClicked()));
  connect(payeeEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeTextChanged(const QString&)));
  connect(updateButton, SIGNAL(clicked()), this, SLOT(slotUpdateClicked()));
  connect(deleteButton, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));
}

KPayeeDlg::~KPayeeDlg()
{
  writeConfig();
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
      deleteButton->setEnabled(true);
    }
  }
}

void KPayeeDlg::slotAddClicked()
{
  m_file->addPayee(payeeEdit->text());
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  payeeCombo->clear();
  int pos=0;
  for (int k=0; it.current(); ++it, k++) {
    payeeCombo->insertItem(it.current()->name());
    if (it.current()->name()==payeeEdit->text())
      pos=k;
  }

  payeeEdit->setText("");
  payeeCombo->setCurrentItem(pos);
  payeeHighlighted(payeeCombo->currentText());
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

void KPayeeDlg::slotDeleteClicked()
{
  QString prompt(i18n("Remove this payee: "));
  prompt += nameLabel->text();

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Payee"))==KMessageBox::No)
    return;

  m_file->removePayee(nameLabel->text());
  QListIterator<MyMoneyPayee> it = m_file->payeeIterator();
  payeeCombo->clear();
  for ( ; it.current(); ++it)
    payeeCombo->insertItem(it.current()->name());
  payeeHighlighted(payeeCombo->currentText());
}

void KPayeeDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_lastPayee = config->readEntry("KPayeeDlg_LastPayee");
}

void KPayeeDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KPayeeDlg_LastPayee", payeeCombo->currentText());
  config->sync();
}
