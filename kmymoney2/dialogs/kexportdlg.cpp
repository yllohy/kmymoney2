/***************************************************************************
                          kexportdlg.cpp  -  description
                             -------------------
    begin                : Tue May 22 2001
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
#include <kmessagebox.h>
#include "kexportdlg.h"
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include "../widgets/kmymoneydateinput.h"
#include <kfiledialog.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <qgroupbox.h>
#include <qlabel.h>

#include "../mymoney/mymoneycategory.h"

KExportDlg::KExportDlg(MyMoneyAccount *account):KExportDlgDecl(0,0,TRUE)
{
  m_mymoneyaccount = account;

  m_qlabelAccount->setText(m_mymoneyaccount->name());
	
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
}
KExportDlg::~KExportDlg()
{
  writeConfig();
}
/** No descriptions */
void KExportDlg::slotBrowse(){

	QString s(KFileDialog::getSaveFileName(QString::null,"*.QIF"));
  m_qlineeditFile->setText(s);
}

void KExportDlg::slotOkClicked()
{
  if (m_qlineeditFile->text().isEmpty()) {
    KMessageBox::information(this, "Please enter the path to the QIF file", "Export");
    m_qlineeditFile->setFocus();
    return;
  }

  connect(m_mymoneyaccount, SIGNAL(signalQIFWriteCount(int)), m_qprogressbar, SLOT(setTotalSteps(int)));
  connect(m_mymoneyaccount, SIGNAL(signalQIFWriteTransaction(int)), this, SLOT(slotSetProgress(int)));

  int nTransCount = 0;
  int nCatCount = 0;

  if (!m_mymoneyaccount->writeQIFFile(m_qlineeditFile->text(), m_qcomboboxDateFormat->currentText(),
        m_qcheckboxAccount->isChecked(), m_qcheckboxCategories->isChecked(), m_kmymoneydateStart->getQDate(),
        m_kmymoneydateEnd->getQDate(), nTransCount, nCatCount)) {
    KMessageBox::error(this, "Error occurred whilst exporting to qif file");
  }
  else {
    QString qstringPrompt = "Number of transactions exported ";
    qstringPrompt += QString::number(nTransCount);
    qstringPrompt += "\nNumber of categories exported ";
    qstringPrompt += QString::number(nCatCount);
    KMessageBox::information(this, qstringPrompt);
  }
//  accept();
}

void KExportDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_qlineeditFile->setText(config->readEntry("KExportDlg_LastFile"));
  m_qcheckboxAccount->setChecked(config->readBoolEntry("KExportDlg_AccountOpt", true));
  m_qcheckboxCategories->setChecked(config->readBoolEntry("KExportDlg_CatOpt", true));
  m_kmymoneydateStart->setDate(config->readDateTimeEntry("KExportDlg_StartDate").date());
  m_kmymoneydateEnd->setDate(config->readDateTimeEntry("KExportDlg_EndDate").date());
	m_qstringLastFormat = config->readEntry("KExportDlg_LastFormat");
}

void KExportDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KExportDlg_LastFile", m_qlineeditFile->text());
  config->writeEntry("KExportDlg_AccountOpt", m_qcheckboxAccount->isChecked());
  config->writeEntry("KExportDlg_CatOpt", m_qcheckboxCategories->isChecked());
  config->writeEntry("KExportDlg_StartDate", QDateTime(m_kmymoneydateStart->getQDate()));
  config->writeEntry("KExportDlg_EndDate", QDateTime(m_kmymoneydateEnd->getQDate()));
	config->writeEntry("KExportDlg_LastFormat", m_qcomboboxDateFormat->currentText());

  config->sync();
}

void KExportDlg::slotSetProgress(int progress)
{
  m_qprogressbar->setProgress(progress);
  QString qstring = QString::number(progress);
  qstring += " of ";
  qstring += QString::number(m_qprogressbar->totalSteps());
  m_qlabelTransaction->setText(qstring);
}
