/***************************************************************************
                          kpayeesview.cpp  -  description
                             -------------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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


// ----------------------------------------------------------------------------
// QT Includes
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>

#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kpayeesview.h"
#include "../mymoney/mymoneyfile.h"

KPayeesView::KPayeesView(QWidget *parent, const char *name )
  : kPayeesViewDecl(parent,name)
{
//  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_payees.png");
//  QPixmap *pm = new QPixmap(filename);
//  m_qpixmaplabel->setPixmap(*pm);

  readConfig();

  connect(payeeCombo, SIGNAL(activated(const QString&)), this, SLOT(payeeHighlighted(const QString&)));
  connect(addButton, SIGNAL(clicked()), this, SLOT(slotAddClicked()));
  connect(payeeEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotPayeeTextChanged(const QString&)));
  connect(updateButton, SIGNAL(clicked()), this, SLOT(slotUpdateClicked()));
  connect(deleteButton, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassPayee, this);

  refresh();
}

KPayeesView::~KPayeesView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassPayee, this);

  writeConfig();
}

void KPayeesView::update(const QCString &id)
{
  refresh();
}

void KPayeesView::payeeHighlighted(const QString& text)
{
  try {
    m_payee = MyMoneyFile::instance()->payeeByName(text);
    m_lastPayee = m_payee.name();

    nameLabel->setText(m_payee.name());
    addressEdit->setEnabled(true);
    addressEdit->setText(m_payee.address());
    postcodeEdit->setEnabled(true);
    postcodeEdit->setText(m_payee.postcode());
    telephoneEdit->setEnabled(true);
    telephoneEdit->setText(m_payee.telephone());
    emailEdit->setEnabled(true);
    emailEdit->setText(m_payee.email());
    updateButton->setEnabled(true);
    deleteButton->setEnabled(true);

  } catch(MyMoneyException *e) {
    m_payee = MyMoneyPayee();
    updateButton->setEnabled(false);
    deleteButton->setEnabled(false);
    delete e;
  }
}

void KPayeesView::slotAddClicked()
{
  try {
    MyMoneyPayee p;

    p = MyMoneyFile::instance()->payeeByName(payeeEdit->text());
    KMessageBox::detailedSorry(0, i18n("Unable to add payee with same name"),
      i18n("Payee already exists"));

  } catch (MyMoneyException *e) {
    m_payee = MyMoneyPayee();

    try {
      m_payee.setName(payeeEdit->text());
      MyMoneyFile::instance()->addPayee(m_payee);

      payeeEdit->setText("");
      m_lastPayee = m_payee.name();

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to add payee"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
    }
  }
}

void KPayeesView::slotPayeeTextChanged(const QString& text)
{
  if (text.isEmpty())
    addButton->setEnabled(false);
  else
    addButton->setEnabled(true);
}

void KPayeesView::slotUpdateClicked()
{
  try {
    m_payee.setName(nameLabel->text());
    m_payee.setAddress(addressEdit->text());
    m_payee.setPostcode(postcodeEdit->text());
    m_payee.setTelephone(telephoneEdit->text());
    m_payee.setEmail(emailEdit->text());

    MyMoneyFile::instance()->modifyPayee(m_payee);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify payee"),
      (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
  }
}

void KPayeesView::slotDeleteClicked()
{
  QString prompt(i18n("Remove this payee: "));
  prompt += nameLabel->text();

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Payee"))==KMessageBox::No)
    return;

  try {
    MyMoneyPayee payee = m_payee;
    m_payee = MyMoneyPayee();
    m_lastPayee = "";
    MyMoneyFile::instance()->removePayee(payee);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to remove payee"),
      (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
  }
}

void KPayeesView::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_lastPayee = config->readEntry("KPayeesView_LastPayee");
}

void KPayeesView::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KPayeesView_LastPayee", payeeCombo->currentText());
  config->sync();
}

void KPayeesView::show()
{
  refresh();
  emit signalViewActivated();
  QWidget::show();
}

void KPayeesView::refresh(void)
{
  bool found = false;

  payeeCombo->clear();

  QValueList<MyMoneyPayee>list = MyMoneyFile::instance()->payeeList();
  QValueList<MyMoneyPayee>::ConstIterator it;
  QStringList payees;

  for (it = list.begin(); it != list.end(); ++it) {
    payees += (*it).name();
    if(m_lastPayee.length() == 0)
      m_lastPayee = (*it).name();
    if((*it).name() == m_lastPayee) {
      m_payee = *it;
      found = true;
    }
  }
  payees.sort();

  payeeCombo->insertStringList(payees);


  if(found == true) {
    payeeCombo->setCurrentText(m_lastPayee);
    payeeHighlighted(payeeCombo->currentText());
  }
}
