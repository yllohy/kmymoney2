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

// ----------------------------------------------------------------------------
// QT Headers

#include <qlineedit.h>
#include <qlabel.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "kexportdlg.h"
#include "../mymoney/mymoneycategory.h"
#include "../dialogs/mymoneyqifprofileeditor.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneyutils.h"

KExportDlg::KExportDlg(QWidget *parent)
  : KExportDlgDecl(parent, 0, true)
{
  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_qif_export.png");
  m_qpixmaplabel->setPixmap(QPixmap(filename));

  // Set (almost) all the last used options
  readConfig();

  loadProfiles(true);
  loadAccounts();

  // load button icons
  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem cancelButtenItem( i18n( "&Cancel" ),
                      QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Abort operation"),
                      i18n("Use this to abort the export operation"));
  m_qbuttonCancel->setGuiItem(cancelButtenItem);
  
  KGuiItem okButtenItem( i18n( "&Export" ),
                      QIconSet(il->loadIcon("fileexport", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Start operation"),
                      i18n("Use this to start the export operation"));
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
  connect(m_profileEditorButton, SIGNAL(clicked()), this, SLOT(slotNewProfile()));
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

  // connect the change signals to the check slot and perform initial check
  connect(m_qlineeditFile, SIGNAL(textChanged(const QString&)), this, SLOT(checkData()));
  connect(m_qcheckboxAccount, SIGNAL(toggled(bool)), this, SLOT(checkData()));
  connect(m_qcheckboxCategories, SIGNAL(toggled(bool)), this, SLOT(checkData()));
  connect(m_accountComboBox, SIGNAL(highlighted(const QString&)), this, SLOT(checkData(const QString&)));
  connect(m_profileComboBox, SIGNAL(highlighted(int)), this, SLOT(checkData()));
  connect(m_kmymoneydateStart, SIGNAL(dateChanged(const QDate&)), this, SLOT(checkData()));
  connect(m_kmymoneydateEnd, SIGNAL(dateChanged(const QDate&)), this, SLOT(checkData()));

  checkData(m_accountComboBox->currentText());
}

KExportDlg::~KExportDlg()
{
}

void KExportDlg::slotBrowse()
{
  QString newName(KFileDialog::getSaveFileName(QString::null,"*.QIF"));
  KMyMoneyUtils::appendCorrectFileExt(newName, QString("qif"));
  if (!newName.isEmpty())
    m_qlineeditFile->setText(newName);
}

void KExportDlg::slotNewProfile(void)
{
  MyMoneyQifProfileEditor* editor = new MyMoneyQifProfileEditor(true, this, "QIF Profile Editor");
  if(editor->exec()) {
    m_profileComboBox->setCurrentText(editor->selectedProfile());
    loadProfiles();
  }
  delete editor;
}

void KExportDlg::loadProfiles(const bool selectLast)
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
    current = config->readEntry("KExportDlg_LastProfile");
  }

  m_profileComboBox->setCurrentItem(0);
  if(list.contains(current) > 0)
    m_profileComboBox->setCurrentText(current);
}

void KExportDlg::slotOkClicked()
{
  // Make sure we save the last used settings for use next time,
  writeConfig();
  accept();
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
  // m_profileComboBox is loaded in loadProfiles(), so we don't worry here
  // m_accountComboBox is loaded in loadAccounts(), so we don't worry here
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
  kconfig->writeEntry("KExportDlg_LastProfile", m_profileComboBox->currentText());
  kconfig->writeEntry("KExportDlg_LastAccount", m_accountComboBox->currentText());
  kconfig->sync();
}

void KExportDlg::checkData(const QString& account)
{
  bool  okEnabled = false;

  if(!m_qlineeditFile->text().isEmpty()) {
    QString strFile(m_qlineeditFile->text());
    if(KMyMoneyUtils::appendCorrectFileExt(strFile, QString("qif")))
      m_qlineeditFile->setText(strFile);
  }

  if(m_lastAccount != account) {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyTransactionFilter filter(accountId(account));
    QValueList<MyMoneyTransaction> list = file->transactionList(filter);
    QValueList<MyMoneyTransaction>::Iterator it;
    
    if(!list.isEmpty()) {
      it = list.begin();
      m_kmymoneydateStart->loadDate((*it).postDate());
      it = list.end();
      --it;
      m_kmymoneydateEnd->loadDate((*it).postDate());
    }
    m_lastAccount = account;
  }
  
  if(!m_qlineeditFile->text().isEmpty()
  && !m_accountComboBox->currentText().isEmpty()
  && !m_profileComboBox->currentText().isEmpty()
  && m_kmymoneydateStart->getQDate() <= m_kmymoneydateEnd->getQDate()
  && (m_qcheckboxAccount->isChecked() || m_qcheckboxCategories->isChecked()))
    okEnabled = true;

  m_qbuttonOk->setEnabled(okEnabled);
}

void KExportDlg::loadAccounts(void)
{
  QStringList strList;

  try {
    MyMoneyFile *file = MyMoneyFile::instance();

    // read all account items from the MyMoneyFile objects and add them to the listbox
    addCategories(strList, file->liability().id(), QString());
    addCategories(strList, file->asset().id(), QString());

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

void KExportDlg::addCategories(QStringList& strList, const QCString& id, const QString& leadIn) const
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

const QCString KExportDlg::accountId() const
{
  return accountId(m_accountComboBox->currentText());
}

const QCString KExportDlg::accountId(const QString& account) const
{
  return MyMoneyFile::instance()->nameToAccount(account);
}
