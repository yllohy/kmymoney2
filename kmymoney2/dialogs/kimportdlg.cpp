/***************************************************************************
                          kimportdlg.cpp  -  description
                             -------------------
    begin                : Wed May 16 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@ctv.es>
                           Felix Rodriguez <frodriguez@mail.wesleyan.edu>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
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
#include <qlineedit.h>
#include <qtextstream.h>
#include <qprogressbar.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// KDE Headers
#include <kpushbutton.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kglobal.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

// ----------------------------------------------------------------------------
// Project Headers
#include "kimportdlg.h"
#include "../mymoney/mymoneyfile.h"
#include "mymoneyqifprofileeditor.h"
// #include "knewaccountwizard.h"

KImportDlg::KImportDlg(QWidget *parent, const char * name)
  : KImportDlgDecl(parent, name, TRUE)
{
  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_qif_import.png");
  m_qpixmaplabel->setPixmap(QPixmap(filename));

  // Set all the last used options
  readConfig();

  loadProfiles(true);
  loadAccounts();
/*
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
  m_qcomboboxDateFormat->insertItem("%d.%m.%yy");
  m_qcomboboxDateFormat->insertItem("%d.%m.%yyyy");

  // Typical US formats
  m_qcomboboxDateFormat->insertItem("%m/%d/%yy");
  m_qcomboboxDateFormat->insertItem("%mmm/%d/%yy");
  m_qcomboboxDateFormat->insertItem("%m/%d/%yyyy");
  m_qcomboboxDateFormat->insertItem("%mmm/%d/%yyyy");
  m_qcomboboxDateFormat->insertItem("%m%d%yy");
  m_qcomboboxDateFormat->insertItem("%mmm/%d%yy");

  m_qcomboboxDateFormat->setEditable(true);

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

*/
  connect(m_qlineeditFile, SIGNAL(textChanged(const QString&)), this,
    SLOT(slotFileTextChanged(const QString&)));

  connect(m_qbuttonBrowse, SIGNAL( clicked() ), this, SLOT( slotBrowse() ) );
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_profileEditorButton, SIGNAL(clicked()), this, SLOT(slotNewProfile()));
  // connect(m_scanButton, SIGNAL(clicked()), this, SLOT(slotScanClicked()));
  // connect(m_profileComboBox, SIGNAL(highlighted(const QString&)), this, SLOT(slotProfileSelected(const QString&)));

  // Don't show them for now.
  m_scanButton->hide();
  m_accountComboBox->hide();
  m_textLabel->hide();
  
  // setup button enable status
  slotFileTextChanged(m_qlineeditFile->text());
/*

  connect(m_qcomboboxDateFormat, SIGNAL( activated(const QString &)), this,
    SLOT(slotDateFormatChanged(const QString&)));
*/
}

KImportDlg::~KImportDlg()
{
}

void KImportDlg::slotBrowse()
{
  QString qstring(KFileDialog::getOpenFileName(QString::null,"*.QIF"));
  if (!qstring.isEmpty())
    m_qlineeditFile->setText(qstring);
}

void KImportDlg::slotScanClicked(void)
{
/*
  QString accountName = m_reader.scanFileForAccount();
  if(accountName.length() != 0) {
    if(!m_accountComboBox->listBox()->findItem(accountName, Qt::ExactMatch | Qt::CaseSensitive)) {
      int rc;
      rc = KMessageBox::questionYesNo(0, i18n("The account '%1' currently does not exist. You can "
                                              "create a new account by pressing the Create button. "
                                              "Press Cancel if you want to select the account manually "
                                              "from the selection box.").arg(accountName),
                                         i18n("Account creation"),
                                         KGuiItem(i18n("Create")),
                                         KGuiItem(i18n("Cancel")));
      if(rc == KMessageBox::Yes) {
        KNewAccountWizard wizard(0);
        wizard.setAccountName(accountName);
        wizard.setAccountType(m_reader.account().accountType());
        wizard.setOpeningBalance(m_reader.account().openingBalance());
        wizard.setOpeningDate(m_reader.account().openingDate());
        if(wizard.exec()) {
          accountName = wizard.account().name();
        }
      }
    }

    if(m_accountComboBox->listBox()->findItem(accountName, Qt::ExactMatch | Qt::CaseSensitive)) {
      m_accountComboBox->setCurrentText(accountName);
    }
  } else {
    KMessageBox::information(0, i18n("No account information has been found in the selected QIF file."
                                     "Please select the account using the selection box in the dialog"),
                                i18n("No account info available"));
  }
*/
}

void KImportDlg::slotDateFormatChanged(const QString& selectedDateFormat)
{
/*
	// activate the apostrophe handling buttons when a
	// slashed or dotted two digit year format is selected
	if((selectedDateFormat.find("/%yy") != -1 || selectedDateFormat.find(".%yy") != -1)
   && selectedDateFormat.find("%yyyy") == -1) {
		m_qApostropheGroup->setEnabled(true);
	} else
		m_qApostropheGroup->setDisabled(true);
*/
}

/** Main work horse of the dialog. */
void KImportDlg::slotOkClicked()
{
  // Save the used options.
  writeConfig();

/*
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
                                       m_qDecimalSymbol->text(),
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
*/
  // leave dialog directly
  accept();
}

void KImportDlg::readConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Last Use Settings");
  m_qlineeditFile->setText(kconfig->readEntry("KImportDlg_LastFile"));
/*
  m_qstringLastFormat = kconfig->readEntry("KImportDlg_LastFormat", "%d/%m/%yy");
  m_qstringLastDecimalSymbol = kconfig->readEntry("KImportDlg_LastDecimalSymbol",
                               KGlobal::locale()->monetaryDecimalSymbol());
  m_qDecimalSymbol->setText(m_qstringLastDecimalSymbol);
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
*/
}

void KImportDlg::writeConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Last Use Settings");
  kconfig->writeEntry("KImportDlg_LastFile", m_qlineeditFile->text());
  kconfig->writeEntry("KImportDlg_LastProfile", m_profileComboBox->currentText());
/*
  kconfig->writeEntry("KImportDlg_LastFormat", m_qcomboboxDateFormat->currentText());
	kconfig->writeEntry("KImportDlg_LastApostrophe",
											m_qApostropheGroup->id(m_qApostropheGroup->selected())+1);
  kconfig->writeEntry("KImportDlg_LastDecimalSymbol", m_qDecimalSymbol->text());
*/
  kconfig->sync();
}

/** Update the progress bar, and update the transaction count indicator. */
void KImportDlg::slotSetProgress(int progress)
{
/*
  m_qprogressbar->setProgress(progress);
  QString qstring = QString::number(progress);
  qstring += i18n(" of ");
  qstring += QString::number(m_qprogressbar->totalSteps());
  m_qlabelTransaction->setText(qstring);
  // force update of modified text on screen every ten iterations
  if((progress % 10) == 0)
    m_qlabelTransaction->repaint();
*/
}

/** Make sure the text input is ok */
void KImportDlg::slotFileTextChanged(const QString& text)
{
  if (!text.isEmpty() && fileExists(text)) {
    // m_qcomboboxDateFormat->setEnabled(true);
    m_qbuttonOk->setEnabled(true);
    m_scanButton->setEnabled(true);
    m_qlineeditFile->setText(text);
  } else {
    // m_qcomboboxDateFormat->setEnabled(false);
    m_qbuttonOk->setEnabled(false);
    m_scanButton->setEnabled(false);
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

void KImportDlg::slotNewProfile(void)
{
  MyMoneyQifProfileEditor* editor = new MyMoneyQifProfileEditor(true, this, "QIF Profile Editor");

  if(editor->exec()) {
    m_profileComboBox->setCurrentText(editor->selectedProfile());
    loadProfiles();
  }

  delete editor;
}

void KImportDlg::slotSelectProfile(const QString& profile)
{
  m_profileComboBox->setCurrentText(profile);
  loadProfiles();
}

void KImportDlg::loadProfiles(const bool selectLast)
{
  QString current = m_profileComboBox->currentText();

  m_profileComboBox->clear();

  QStringList list;
  KConfig* config = KGlobal::config();
  config->setGroup("Profiles");

  list = config->readListEntry("profiles");
  list.sort();
  m_profileComboBox->insertStringList(list);

  if(selectLast == true) {
    config->setGroup("Last Use Settings");
    current = config->readEntry("KImportDlg_LastProfile");
  }

  m_profileComboBox->setCurrentItem(0);
  if(list.contains(current) > 0) {
    m_profileComboBox->setCurrentText(current);
  }
}

void KImportDlg::loadAccounts(void)
{
  QStringList strList;

  try {
    MyMoneyFile *file = MyMoneyFile::instance();

    // read all account items from the MyMoneyFile objects and add them to the listbox
    addCategories(strList, file->liability().id(), "");
    addCategories(strList, file->asset().id(), "");

  } catch (MyMoneyException *e) {
    qDebug("Exception '%s' thrown in %s, line %ld caught in KExportDlg::loadAccounts:%d",
      e->what().latin1(), e->file().latin1(), e->line(), __LINE__);
    delete e;
  }

  strList.sort();
  m_accountComboBox->insertStringList(strList);

  KConfig* config = KGlobal::config();
  config->setGroup("Last Use Settings");
  QString current = config->readEntry("KExportDlg_LastAccount");

  m_accountComboBox->setCurrentItem(0);
  if(strList.contains(current) > 0)
    m_accountComboBox->setCurrentText(current);
}

void KImportDlg::addCategories(QStringList& strList, const QCString& id, const QString& leadIn) const
{
  MyMoneyFile *file = MyMoneyFile::instance();
  QString name;

  MyMoneyAccount account = file->account(id);

  QCStringList accList = account.accountList();
  QCStringList::ConstIterator it_a;

  for(it_a = accList.begin(); it_a != accList.end(); ++it_a) {
    account = file->account(*it_a);
    strList << leadIn + account.name();
    addCategories(strList, *it_a, leadIn + account.name() + ":");
  }
}

