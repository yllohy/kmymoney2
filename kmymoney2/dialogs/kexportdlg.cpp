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
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// QT Headers
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <qgroupbox.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Headers
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Headers
#include "kexportdlg.h"
#include "../widgets/kmymoneydateinput.h"
#include "../mymoney/mymoneycategory.h"

/** If the accoun is null we will have an undefined operation. */
KExportDlg::KExportDlg(MyMoneyAccount *account, QWidget *parent)
  : KExportDlgDecl(parent,0,TRUE)
{
  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_qif_export.png");
  QPixmap *pm = new QPixmap(filename);
  m_qpixmaplabel->setPixmap(*pm);

  m_mymoneyaccount = account;

  // Set the account name that we will be operating on
  if (m_mymoneyaccount)
    m_qlabelAccount->setText(m_mymoneyaccount->name());
  else
    m_qlabelAccount->setText(i18n("NO ACCOUNT AVAILABLE."));
  
  // Typical UK formats, (as well as france etc)
  m_qcomboboxDateFormat->insertItem("%d/%m/%yy");
  m_qcomboboxDateFormat->insertItem("%d/%mmm/%yy");
  m_qcomboboxDateFormat->insertItem("%d/%m/%yyyy");
  m_qcomboboxDateFormat->insertItem("%d/%mmm/%yyyy");
  m_qcomboboxDateFormat->insertItem("%d/%m%yy");
  m_qcomboboxDateFormat->insertItem("%d/%mmm%yy");

  // Typical US formats
  m_qcomboboxDateFormat->insertItem("%m/%d/%yy");
  m_qcomboboxDateFormat->insertItem("%mmm/%d/%yy");
  m_qcomboboxDateFormat->insertItem("%m/%d/%yyyy");
  m_qcomboboxDateFormat->insertItem("%mmm/%d/%yyyy");
  m_qcomboboxDateFormat->insertItem("%m%d%yy");
  m_qcomboboxDateFormat->insertItem("%mmm/%d%yy");

  m_qcomboboxDateFormat->setEditable(true);

  // Set all the last used options
  readConfig();

  int nErrorReturn = 0;

  if (m_mymoneyaccount->validateQIFDateFormat("", m_qstringLastFormat.latin1(), nErrorReturn, false))
    m_qcomboboxDateFormat->setEditText(m_qstringLastFormat);
  else {
    QString qstringError(i18n("QIF date format invalid: "));
    qstringError += m_mymoneyaccount->getQIFDateFormatErrorString(nErrorReturn);
    KMessageBox::error(this, qstringError, i18n("Import QIF"));
  }

  // Now that we've got the text in the combo box, reset the edit status
  m_qcomboboxDateFormat->setEditable(false);

  connect(m_qlineeditFile, SIGNAL(textChanged(const QString&)), this,
    SLOT(slotFileTextChanged(const QString&)));

  connect(m_qbuttonBrowse, SIGNAL( clicked() ), this, SLOT( slotBrowse() ) );
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
}

KExportDlg::~KExportDlg()
{
  // Make sure we save the last used settings for use next time,
  writeConfig();
}

void KExportDlg::slotBrowse()
{
  QString qstring(KFileDialog::getSaveFileName(QString::null,"*.QIF"));
  m_qlineeditFile->setText(qstring);
}

/** Perform the real processing when OK is clicked */
void KExportDlg::slotOkClicked()
{
  // Do some validation on the inputs.
  if (m_qlineeditFile->text().isEmpty()) {
    KMessageBox::information(this, i18n("Please enter the path to the QIF file"), i18n("Export QIF"));
    m_qlineeditFile->setFocus();
    return;
  }

  if (!m_qcheckboxAccount->isChecked() || !m_qcheckboxCategories->isChecked()) {
    KMessageBox::information(this, i18n("Please specify at least one of the content types to export."),
        i18n("Export QIF"));
    return;
  }

  if (m_kmymoneydateEnd->getQDate() < m_kmymoneydateStart->getQDate()) {
    KMessageBox::information(this, i18n("Please enter a start date lower than the end date."));
    return;
  }

  int nErrorReturn = 0;

  if (!m_mymoneyaccount->validateQIFDateFormat("", m_qcomboboxDateFormat->currentText().latin1(), nErrorReturn, false))
  {
    QString qstringError(i18n("QIF date format invalid: "));
    qstringError += m_mymoneyaccount->getQIFDateFormatErrorString(nErrorReturn);
    KMessageBox::error(this, qstringError, i18n("Import QIF"));
    m_qcomboboxDateFormat->setFocus();
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

    // Do the actual write
    if (!m_mymoneyaccount->writeQIFFile(m_qlineeditFile->text(), m_qcomboboxDateFormat->currentText(),
          m_qcheckboxAccount->isChecked(), m_qcheckboxCategories->isChecked(), m_kmymoneydateStart->getQDate(),
          m_kmymoneydateEnd->getQDate(), nTransCount, nCatCount)) {
      KMessageBox::error(this, i18n("Error occurred whilst exporting to qif file."), i18n("Export QIF"));
    }
    else {
      QString qstringPrompt = i18n("Export finished successfully.\n\n");
      qstringPrompt += i18n("Number of transactions exported ");
      qstringPrompt += QString::number(nTransCount);
      qstringPrompt += i18n(".\nNumber of categories exported ");
      qstringPrompt += QString::number(nCatCount);
      qstringPrompt += ".";
      KMessageBox::information(this, qstringPrompt, i18n("Export QIF"));
    }
  }
//  accept();
}

void KExportDlg::readConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Last Use Settings");
  m_qlineeditFile->setText(kconfig->readEntry("KExportDlg_LastFile"));
  m_qcheckboxAccount->setChecked(kconfig->readBoolEntry("KExportDlg_AccountOpt", true));
  m_qcheckboxCategories->setChecked(kconfig->readBoolEntry("KExportDlg_CatOpt", true));
  m_kmymoneydateStart->setDate(kconfig->readDateTimeEntry("KExportDlg_StartDate").date());
  m_kmymoneydateEnd->setDate(kconfig->readDateTimeEntry("KExportDlg_EndDate").date());
  m_qstringLastFormat = kconfig->readEntry("KExportDlg_LastFormat", "%d/%m/%yyyy");

  if (m_qlineeditFile->text().length()>=1) {
    m_qgroupboxDates->setEnabled(true);
    m_qgroupboxContents->setEnabled(true);
    m_qgroupboxFormats->setEnabled(true);
    m_qbuttonOk->setEnabled(true);
  } else {
    m_qgroupboxDates->setEnabled(false);
    m_qgroupboxContents->setEnabled(false);
    m_qgroupboxFormats->setEnabled(false);
    m_qbuttonOk->setEnabled(false);
  }
}

void KExportDlg::writeConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Last Use Settings");
  kconfig->writeEntry("KExportDlg_LastFile", m_qlineeditFile->text());
  kconfig->writeEntry("KExportDlg_AccountOpt", m_qcheckboxAccount->isChecked());
  kconfig->writeEntry("KExportDlg_CatOpt", m_qcheckboxCategories->isChecked());
  kconfig->writeEntry("KExportDlg_StartDate", QDateTime(m_kmymoneydateStart->getQDate()));
  kconfig->writeEntry("KExportDlg_EndDate", QDateTime(m_kmymoneydateEnd->getQDate()));
  kconfig->writeEntry("KExportDlg_LastFormat", m_qcomboboxDateFormat->currentText());

  kconfig->sync();
}

/** Update the progress bar, and update the transaction count indicator. */
void KExportDlg::slotSetProgress(int progress)
{
  m_qprogressbar->setProgress(progress);
  QString qstring = QString::number(progress);
  qstring += i18n(" of ");
  qstring += QString::number(m_qprogressbar->totalSteps());
  m_qlabelTransaction->setText(qstring);
}

/** Make sure the text input is ok */
void KExportDlg::slotFileTextChanged(const QString& text)
{
  if (!text.isEmpty()) {
    m_qgroupboxDates->setEnabled(true);
    m_qgroupboxContents->setEnabled(true);
    m_qgroupboxFormats->setEnabled(true);
    m_qbuttonOk->setEnabled(true);
    m_qlineeditFile->setText(text);
  } else {
    m_qgroupboxDates->setEnabled(false);
    m_qgroupboxContents->setEnabled(false);
    m_qgroupboxFormats->setEnabled(false);
    m_qbuttonOk->setEnabled(false);
  }
}
