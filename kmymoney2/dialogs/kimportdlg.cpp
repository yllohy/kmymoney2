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
// ----------------------------------------------------------------------------
// QT Headers
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qtextstream.h>
#include <qprogressbar.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Headers
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Headers
#include "kimportdlg.h"

KImportDlg::KImportDlg(MyMoneyAccount *account)
  : KImportDlgDecl(0,0,TRUE)
{
  // We have to be careful of nulls though
  m_mymoneyaccount = account;
  
  // Set the account name that we will be operating on
  if (m_mymoneyaccount)
    m_qlabelAccount->setText(m_mymoneyaccount->name());
  else
    m_qlabelAccount->setText(i18n("NO ACCOUNT AVAILABLE."));

  m_qcomboboxDateFormat->insertItem("MM/DD\'YY");
  m_qcomboboxDateFormat->insertItem("MM/DD/YYYY");
  m_qcomboboxDateFormat->setEditable(false);

  // Set all the last used options
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
  // Save the used options.
  writeConfig();
}

void KImportDlg::slotBrowse()
{
  QString qstring(KFileDialog::getOpenFileName(QString::null,"*.QIF"));
  m_qlineeditFile->setText(qstring);
}

/** Main work horse of the dialog. */
void KImportDlg::slotOkClicked()
{
  if (m_qlineeditFile->text().isEmpty()) {
    KMessageBox::information(this, i18n("Please enter the path to the QIF file"), i18n("Import QIF"));
    m_qlineeditFile->setFocus();
    return;
  }

  // Make sure we have an account to operate on
  if (m_mymoneyaccount) {
    // Connect to the provided signals in MyMoneyAccount
    // These signals will be emitted at appropriate times.
    connect(m_mymoneyaccount, SIGNAL(signalProgressCount(int)), m_qprogressbar, SLOT(setTotalSteps(int)));
    connect(m_mymoneyaccount, SIGNAL(signalProgress(int)), this, SLOT(slotSetProgress(int)));

    int nTransCount = 0;
    int nCatCount = 0;

    if (!m_mymoneyaccount->readQIFFile(m_qlineeditFile->text(), m_qcomboboxDateFormat->currentText(),
      nTransCount, nCatCount)) {
        KMessageBox::error(this, i18n("Import from QIF file failed."));
    } else {
      QString qstringPrompt = i18n("Import finished successfully.\nPlease remember, all categories that already exist have not been imported\n\n");
      qstringPrompt += i18n("Number of transactions imported ");
      qstringPrompt += QString::number(nTransCount);
      qstringPrompt += i18n(".\nNumber of categories imported ");
      qstringPrompt += QString::number(nCatCount);
      qstringPrompt += ".";
      KMessageBox::information(this, qstringPrompt, i18n("Import QIF"));
    }
  }

//  accept();
}

void KImportDlg::readConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Last Use Settings");
  m_qlineeditFile->setText(kconfig->readEntry("KImportDlg_LastFile"));
  m_qstringLastFormat = kconfig->readEntry("KImportDlg_LastFormat");
}

void KImportDlg::writeConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Last Use Settings");
  kconfig->writeEntry("KImportDlg_LastFile", m_qlineeditFile->text());
  kconfig->writeEntry("KImportDlg_LastFormat", m_qcomboboxDateFormat->currentText());
  kconfig->sync();
}

/** Update the progress bar, and update the transaction count indicator. */
void KImportDlg::slotSetProgress(int progress)
{
  m_qprogressbar->setProgress(progress);
  QString qstring = QString::number(progress);
  qstring += i18n(" of ");
  qstring += QString::number(m_qprogressbar->totalSteps());
  m_qlabelTransaction->setText(qstring);
}
