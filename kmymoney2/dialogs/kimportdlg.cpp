/***************************************************************************
                          kimportdlg.cpp  -  description
                             -------------------
    begin                : Wed May 16 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
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
#include <kmessagebox.h>
#include "kimportdlg.h"
#include <qlineedit.h>
#include <qcombobox.h>
#include <kfiledialog.h>
#include <qtextstream.h>
#include <qmessagebox.h>

KImportDlg::KImportDlg(MyMoneyAccount *account):KImportDlgDecl(0,0,TRUE)
{
	m_mymoneyaccount = account;
	
	m_qcomboboxDateFormat->insertItem("MM/DD\'YY");
	m_qcomboboxDateFormat->insertItem("MM/DD/YYYY");
	m_qcomboboxDateFormat->setEditable(false);

  readConfig();

	if (m_qstringLastFormat == "MM/DD\'YY")
		m_qcomboboxDateFormat->setCurrentItem(0);
	else
		m_qcomboboxDateFormat->setCurrentItem(1);

  connect(m_qbuttonBrowse, SIGNAL( clicked() ), this, SLOT( slotBrowse() ) );
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

KImportDlg::~KImportDlg()
{
  writeConfig();
}
/** No descriptions */
void KImportDlg::slotBrowse()
{
	//KFileDialog *browseFile = new KFileDialog();
	QString s(KFileDialog::getOpenFileName(QString::null,"*.QIF"));
  //delete browseFile;
	m_qlineeditFile->setText(s);

}

void KImportDlg::slotOkClicked()
{
  if (m_qlineeditFile->text().isEmpty()) {
    KMessageBox::information(this, "Please enter the path to the QIF file", "Import");
    m_qlineeditFile->setFocus();
    return;
  }

  if (!m_mymoneyaccount->readQIFFile(m_qlineeditFile->text(), m_qcomboboxDateFormat->currentText()))
    KMessageBox::error(this, "Import from QIF file failed");

  accept();
}

void KImportDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_qlineeditFile->setText(config->readEntry("KImportDlg_LastFile"));
	m_qstringLastFormat = config->readEntry("KImportDlg_LastFormat");
}

void KImportDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KImportDlg_LastFile", m_qlineeditFile->text());
	config->writeEntry("KImportDlg_LastFormat", m_qcomboboxDateFormat->currentText());
  config->sync();
}
