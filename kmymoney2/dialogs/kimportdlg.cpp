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
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// QT Headers
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qtextstream.h>
#include <qprogressbar.h>
#include <qlabel.h>
#include <qbuttongroup.h>

// ----------------------------------------------------------------------------
// KDE Headers
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Headers
#include "kimportdlg.h"

KImportDlg::KImportDlg(MyMoneyAccount *account, QWidget *parent)
  : KImportDlgDecl(parent,0,TRUE)
{
  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_qif_import.png");
  QPixmap pm(filename);
  m_qpixmaplabel->setPixmap(pm);

  // We have to be careful of nulls though
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

  if (m_mymoneyaccount->validateQIFDateFormat("", m_qstringLastFormat.latin1(), nErrorReturn, false)) {
// setEditText() does not work for me. Don't know why. The doc says, it has been
// removed in qt 2.0
//    m_qcomboboxDateFormat->setEditText(m_qstringLastFormat);
    for(int i=0; i < m_qcomboboxDateFormat->count(); ++i) {
      if(m_qcomboboxDateFormat->text(i) == m_qstringLastFormat) {
        m_qcomboboxDateFormat->setCurrentItem(i);
        break;
      }
    }
    slotDateFormatChanged(m_qstringLastFormat);
  } else {
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
  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

	connect(m_qcomboboxDateFormat, SIGNAL( activated(const QString &)), this,
		SLOT(slotDateFormatChanged(const QString&)));
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

void KImportDlg::slotDateFormatChanged(const QString& selectedDateFormat)
{
	// activate the apostrophe handling buttons when a
	// slashed two digit year format is selected
	if(selectedDateFormat.find("/%yy") != -1
	&& selectedDateFormat.find("%yyyy") == -1) {
		m_qApostropheGroup->setEnabled(true);
	} else
		m_qApostropheGroup->setDisabled(true);
}

/** Main work horse of the dialog. */
void KImportDlg::slotOkClicked()
{
  if (m_qlineeditFile->text().isEmpty()) {
    KMessageBox::information(this, i18n("Please enter the path to the QIF file"), i18n("Import QIF"));
    m_qlineeditFile->setFocus();
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

    if (!m_mymoneyaccount->readQIFFile(m_qlineeditFile->text(),
                                       m_qcomboboxDateFormat->currentText(),
                                       m_qApostropheGroup->id(m_qApostropheGroup->selected()),
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

  // leave dialog directly
  accept();
}

void KImportDlg::readConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Last Use Settings");
  m_qlineeditFile->setText(kconfig->readEntry("KImportDlg_LastFile"));
  m_qstringLastFormat = kconfig->readEntry("KImportDlg_LastFormat", "%d/%m/%yy");
  if (m_qlineeditFile->text().length()>=1  && fileExists(m_qlineeditFile->text())) {
    m_qcomboboxDateFormat->setEnabled(true);
    m_qbuttonOk->setEnabled(true);
  } else {
    m_qcomboboxDateFormat->setEnabled(false);
    m_qbuttonOk->setEnabled(false);
  }
	int rc = kconfig->readEntry("KImportDlg_LastApostrophe").toUShort()-1;
	if(rc >= 0 && rc <= 2)
		m_qApostropheGroup->setButton(rc);
}

void KImportDlg::writeConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Last Use Settings");
  kconfig->writeEntry("KImportDlg_LastFile", m_qlineeditFile->text());
  kconfig->writeEntry("KImportDlg_LastFormat", m_qcomboboxDateFormat->currentText());
	kconfig->writeEntry("KImportDlg_LastApostrophe",
											m_qApostropheGroup->id(m_qApostropheGroup->selected())+1);
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
  // force update of modified text on screen every ten iterations
  if((progress % 10) == 0)
    m_qlabelTransaction->repaint();
}

/** Make sure the text input is ok */
void KImportDlg::slotFileTextChanged(const QString& text)
{
  if (!text.isEmpty() && fileExists(text)) {
    m_qcomboboxDateFormat->setEnabled(true);
    m_qbuttonOk->setEnabled(true);
    m_qlineeditFile->setText(text);
  } else {
    m_qcomboboxDateFormat->setEnabled(false);
    m_qbuttonOk->setEnabled(false);
  }
}

bool KImportDlg::fileExists(KURL url)
{
  if (url.isLocalFile()) {
    // Lets make sure it exists first
    if (url.fileName().length()>=1) {
      QFile f(url.directory(false,true)+url.fileName());
      return f.exists();
    }
  }
  // We don't bother checking URL's or showing them
  // because at the moment MyMoneyFile can't read them
  // anyway
  return false;
}
