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
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kglobalsettings.h>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "kimportdlg.h"
#include <kmymoney/mymoneyfile.h>
#include "mymoneyqifprofileeditor.h"
#include "../converter/mymoneyqifprofile.h"

KImportDlg::KImportDlg(QWidget *parent, const char * name)
  : KImportDlgDecl(parent, name, TRUE)
{
  // Set all the last used options
  readConfig();

  loadProfiles(true);
  loadAccounts();

  // load button icons
  m_qbuttonCancel->setGuiItem(KStdGuiItem::cancel());

  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem okButtenItem( i18n( "&Import" ),
                      QIconSet(il->loadIcon("fileimport", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Start operation"),
                      i18n("Use this to start the import operation"));
  m_qbuttonOk->setGuiItem(okButtenItem);

  KGuiItem browseButtenItem( i18n( "&Browse..." ),
                      QIconSet(il->loadIcon("fileopen", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Select filename"),
                      i18n("Use this to select a filename to export to"));
  m_qbuttonBrowse->setGuiItem(browseButtenItem);

  KGuiItem newButtenItem( i18n( "&New..." ),
                      QIconSet(il->loadIcon("filenew", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Create a new profile"),
                      i18n("Use this to open the profile editor"));
  m_profileEditorButton->setGuiItem(newButtenItem);

  // connect the buttons to their functionality
  connect(m_qbuttonBrowse, SIGNAL( clicked() ), this, SLOT( slotBrowse() ) );
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_profileEditorButton, SIGNAL(clicked()), this, SLOT(slotNewProfile()));

  // connect the change signals to the check slot and perform initial check
  connect(m_qlineeditFile, SIGNAL(textChanged(const QString&)), this,
    SLOT(slotFileTextChanged(const QString&)));

  // Don't show them for now.
  m_scanButton->hide();
  m_accountComboBox->hide();
  m_textLabel->hide();

  // setup button enable status
  slotFileTextChanged(m_qlineeditFile->text());
}

KImportDlg::~KImportDlg()
{
}

void KImportDlg::slotBrowse()
{
  // determine what the browse prefix should be from the current profile

  MyMoneyQifProfile tmpprofile;
  tmpprofile.loadProfile("Profile-" + profile());

  KFileDialog dialog(KGlobalSettings::documentPath(),
                     i18n("%1|Import files\n%2|All files (*.*)").arg(tmpprofile.filterFileType()).arg("*"),
                     this, i18n("Import File..."), true);
  dialog.setMode(KFile::File | KFile::ExistingOnly);

  if(dialog.exec() == QDialog::Accepted) {
#if KDE_IS_VERSION(3,4,0)
    m_qlineeditFile->setText(dialog.selectedURL().pathOrURL());
#else
    m_qlineeditFile->setText(dialog.selectedURL().prettyURL(0, KURL::StripFileProtocol));
#endif
  }
}

void KImportDlg::slotOkClicked()
{
  // Save the used options.
  writeConfig();
  // leave dialog directly
  accept();
}

void KImportDlg::readConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Last Use Settings");
  m_qlineeditFile->setText(kconfig->readEntry("KImportDlg_LastFile"));
  m_payeeCreation->setChecked(kconfig->readBoolEntry("KImportDlg_CreatePayees",false));
}

void KImportDlg::writeConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Last Use Settings");
  kconfig->writeEntry("KImportDlg_LastFile", m_qlineeditFile->text());
  kconfig->writeEntry("KImportDlg_LastProfile", m_profileComboBox->currentText());
  kconfig->writeEntry("KImportDlg_CreatePayees",m_payeeCreation->isChecked());
  kconfig->sync();
}

/** Make sure the text input is ok */
void KImportDlg::slotFileTextChanged(const QString& text)
{
  if (!text.isEmpty() && KIO::NetAccess::exists(text, true, qApp->mainWidget())) {
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
  // Creating an editor object here makes sure that
  // we have at least the default profile available
  MyMoneyQifProfileEditor* edit = new MyMoneyQifProfileEditor(true, 0, 0);
  edit->slotOk();
  delete edit;

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

void KImportDlg::addCategories(QStringList& strList, const QString& id, const QString& leadIn) const
{
  MyMoneyFile *file = MyMoneyFile::instance();
  QString name;

  MyMoneyAccount account = file->account(id);

  QStringList accList = account.accountList();
  QStringList::ConstIterator it_a;

  for(it_a = accList.begin(); it_a != accList.end(); ++it_a) {
    account = file->account(*it_a);
    strList << leadIn + account.name();
    addCategories(strList, *it_a, leadIn + account.name() + MyMoneyFile::AccountSeperator);
  }
}


#include "kimportdlg.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
