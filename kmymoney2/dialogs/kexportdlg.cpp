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

// ----------------------------------------------------------------------------
// Project Headers

#include "kexportdlg.h"
#include "../mymoney/mymoneycategory.h"
#include "../dialogs/mymoneyqifprofileeditor.h"
#include "../mymoney/mymoneyfile.h"

KExportDlg::KExportDlg(QWidget *parent)
  : KExportDlgDecl(parent, 0, true)
{
  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_qif_export.png");
  m_qpixmaplabel->setPixmap(QPixmap(filename));

  // Set (almost) all the last used options
  readConfig();

  loadProfiles(true);
  loadAccounts();

  // connect the buttons to their functionality
  connect(m_qbuttonBrowse, SIGNAL( clicked() ), this, SLOT( slotBrowse() ) );
  connect(m_profileEditorButton, SIGNAL(clicked()), this, SLOT(slotNewProfile()));
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));

  // connect the change signals to the check slot and perform initial check
  connect(m_qlineeditFile, SIGNAL(textChanged(const QString&)), this, SLOT(checkData()));
  connect(m_qcheckboxAccount, SIGNAL(toggled(bool)), this, SLOT(checkData()));
  connect(m_qcheckboxCategories, SIGNAL(toggled(bool)), this, SLOT(checkData()));
  connect(m_accountComboBox, SIGNAL(highlighted(int)), this, SLOT(checkData()));
  connect(m_profileComboBox, SIGNAL(highlighted(int)), this, SLOT(checkData()));
  connect(m_kmymoneydateStart, SIGNAL(dateChanged(const QDate&)), this, SLOT(checkData()));
  connect(m_kmymoneydateEnd, SIGNAL(dateChanged(const QDate&)), this, SLOT(checkData()));

  checkData();
}

KExportDlg::~KExportDlg()
{
}

void KExportDlg::slotBrowse()
{
  QString newName(KFileDialog::getSaveFileName(QString::null,"*.QIF"));
  appendCorrectFileExt(newName, QString("qif"));
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

///////////////////////////////////////////////////////////////////////////////////////////////
/**
*	Adds the file extension to the end of the file name.
*
*	@return		bool
*						- true if name was changed
*						- false if it wasn't.
*
*	@todo			This function should be moved to a separate file, or utility file somewhere
*						in the library files, because it appears in numerous places.
*/
///////////////////////////////////////////////////////////////////////////////////////////////
bool KExportDlg::appendCorrectFileExt(QString& str, const QString strExtToUse)
{
	if(!str.isEmpty())
  {
		//find last . delminator
		int nLoc = str.findRev('.');
    if(nLoc != -1)
		{
			QString strExt, strTemp;
      strTemp = str.left(nLoc + 1);
			strExt = str.right(str.length() - (nLoc + 1));
			if(strExt.find(strExtToUse, 0, FALSE) == -1)
			{
				//append to make complete file name
				strTemp.append(strExtToUse);
				str = strTemp;
			}
			else
			{
				return false;
			}
		}
		else
		{
			str.append(".");
			str.append(strExtToUse);
		}
	}
	else
	{
		return false;
	}

	return true;
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

void KExportDlg::checkData(void)
{
  bool  okEnabled = false;

  if(!m_qlineeditFile->text().isEmpty()) {
    QString strFile(m_qlineeditFile->text());
    if(appendCorrectFileExt(strFile, QString("qif")))
      m_qlineeditFile->setText(strFile);
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
  return MyMoneyFile::instance()->nameToAccount(m_accountComboBox->currentText());
}

