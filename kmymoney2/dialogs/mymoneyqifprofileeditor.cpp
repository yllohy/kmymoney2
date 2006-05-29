/***************************************************************************
                          kqifprofileeditor.cpp  -  description
                             -------------------
    begin                : Tue Dec 24 2002
    copyright            : (C) 2002 by Thomas Baumgart
    email                : thb@net-bembel.de
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
#include <qlistbox.h>
#include <qlistview.h>
#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <kurlrequester.h>
#include <kiconloader.h>
#include <kapplication.h>

#if KDE_IS_VERSION(3,2,0)
  #include <kinputdialog.h>
#else
  #include <klineeditdlg.h>
#endif
// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyqifprofileeditor.h"

MyMoneyQifProfileNameValidator::MyMoneyQifProfileNameValidator(QObject *o, const char *name)
  : QValidator(o, name)
{
}

MyMoneyQifProfileNameValidator::~MyMoneyQifProfileNameValidator()
{
}

QValidator::State MyMoneyQifProfileNameValidator::validate(QString& name, int&) const
{
  KConfig* config = KGlobal::config();
  config->setGroup("Profiles");
  QStringList list = config->readListEntry("profiles");

  // invalid character?
  if(name.contains(",") != 0)
    return QValidator::Invalid;

  // would not work in this form (empty or existing name)
  if(name.isEmpty() || list.contains(name))
    return QValidator::Intermediate;

  // is OK
  return QValidator::Acceptable;
}

MyMoneyQifProfileEditor::MyMoneyQifProfileEditor(const bool edit, QWidget *parent, const char *name )
  : MyMoneyQifProfileEditorDecl(parent,name),
  m_inEdit(edit),
  m_isDirty(false),
  m_isAccepted(false),
  m_selectedAmountType(0)
{
  loadWidgets();
  loadProfileListFromConfig();

  // load button icons
  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem resetButtonItem( i18n( "&Reset" ),
                    QIconSet(il->loadIcon("undo", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Reset all settings"),
                    i18n("Use this to reset all settings to the state they were when the dialog was opened."));
  m_resetButton->setGuiItem(resetButtonItem);

  KGuiItem cancelButtenItem( i18n( "&Cancel" ),
                      QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Close dialog"),
                      i18n("Use this to close the dialog and abort the operation"));
  m_cancelButton->setGuiItem(cancelButtenItem);

  KGuiItem okButtenItem( i18n( "&Ok" ),
                      QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Close dialog"),
                      i18n("Use this to accept and store data"));
  m_okButton->setGuiItem(okButtenItem);

  KGuiItem deleteButtenItem( i18n( "&Delete" ),
                      QIconSet(il->loadIcon("editdelete", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Delete the selected profile"),
                      i18n("Use this to delete the selected profile"));
  m_deleteButton->setGuiItem(deleteButtenItem);

  KGuiItem newButtenItem( i18n( "&New" ),
                      QIconSet(il->loadIcon("filenew", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Create a new profile"),
                      i18n("Use this to create a new QIF import/export profile"));
  m_newButton->setGuiItem(newButtenItem);

  m_helpButton->setGuiItem(KStdGuiItem::help());

  connect(m_profileListBox, SIGNAL(highlighted(const QString&)), this, SLOT(slotLoadProfileFromConfig(const QString&)));
  connect(m_resetButton, SIGNAL(clicked()), this, SLOT(slotReset()));
  connect(m_okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
  connect(m_renameButton, SIGNAL(clicked()), this, SLOT(slotRename()));
  connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(slotDelete()));
  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotNew()));
  connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));

  connect(m_editDescription, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setProfileDescription(const QString&)));
  connect(m_editType, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setProfileType(const QString&)));
  connect(m_editOpeningBalance, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setOpeningBalanceText(const QString&)));
  connect(m_editAccountDelimiter, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setAccountDelimiter(const QString&)));
  connect(m_editVoidMark, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setVoidMark(const QString&)));

  connect(m_editDateFormat, SIGNAL(highlighted(const QString&)), &m_profile, SLOT(setDateFormat(const QString&)));
  connect(m_editApostrophe, SIGNAL(highlighted(const QString&)), &m_profile, SLOT(setApostropheFormat(const QString&)));

  connect(m_editAmounts, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotAmountTypeSelected(QListViewItem*)));
  connect(m_decimalBox, SIGNAL(activated(const QString&)), this, SLOT(slotDecimalChanged(const QString&)));
  connect(m_thousandsBox, SIGNAL(activated(const QString&)), this, SLOT(slotThousandsChanged(const QString&)));

  connect(m_editInputFilterLocation, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setFilterScriptImport(const QString&)));
  connect(m_editInputFilterLocation, SIGNAL(urlSelected(const QString&)), m_editInputFilterLocation, SLOT(setURL(const QString&)));

  connect(m_editInputFilterFileType, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setFilterFileType(const QString&)));
  
  connect(m_editOutputFilterLocation, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setFilterScriptExport(const QString&)));
  connect(m_editOutputFilterLocation, SIGNAL(urlSelected(const QString&)), m_editOutputFilterLocation, SLOT(setURL(const QString&)));

  connect(m_attemptMatch, SIGNAL(toggled(bool)), &m_profile, SLOT(setAttemptMatchDuplicates(bool)));
}

MyMoneyQifProfileEditor::~MyMoneyQifProfileEditor()
{
  if(m_inEdit && m_isDirty && m_isAccepted) {
    KConfig* config = KGlobal::config();
    config->sync();
  } else {
    slotReset();
  }
}

void MyMoneyQifProfileEditor::loadWidgets(void)
{
  if(m_inEdit)
    setCaption(i18n("QIF Profile Editor"));
  else
    setCaption(i18n("QIF Profile Selector"));

  m_editDateFormat->clear();
  m_editDateFormat->insertItem( "%d/%m/%yy" );
  m_editDateFormat->insertItem( "%d/%mmm/%yy" );
  m_editDateFormat->insertItem( "%d/%m/%yyyy" );
  m_editDateFormat->insertItem( "%d/%mmm/%yyyy" );
  m_editDateFormat->insertItem( "%d/%m%yy" );
  m_editDateFormat->insertItem( "%d/%mmm%yy" );
  m_editDateFormat->insertItem( "%d.%m.%yy" );
  m_editDateFormat->insertItem( "%d.%m.%yyyy" );
  m_editDateFormat->insertItem( "%m.%d.%yy" );
  m_editDateFormat->insertItem( "%m.%d.%yyyy" );
  m_editDateFormat->insertItem( "%m/%d/%yy" );
  m_editDateFormat->insertItem( "%mmm/%d/%yy" );
  m_editDateFormat->insertItem( "%m/%d/%yyyy" );
  m_editDateFormat->insertItem( "%mmm/%d/%yyyy" );
  m_editDateFormat->insertItem( "%m%d%yy" );
  m_editDateFormat->insertItem( "%mmm/%d%yy" );
  m_editDateFormat->insertItem( "%yyyy-%mm-%dd" );
  m_editDateFormat->insertItem( "%m/%d'%yyyy" );

  m_editApostrophe->clear();
  m_editApostrophe->insertItem( "1900-1949" );
  m_editApostrophe->insertItem( "1900-1999" );
  m_editApostrophe->insertItem( "2000-2099" );

  m_editAmounts->setColumnAlignment(1, Qt::AlignCenter);
  m_editAmounts->setColumnAlignment(2, Qt::AlignCenter);
  m_editAmounts->setColumnAlignment(3, Qt::AlignCenter);

  m_editAmounts->setColumnWidth(4, 0);
  m_editAmounts->setColumnWidthMode(4, QListView::Manual);
  m_editAmounts->setSorting(4);
  m_editAmounts->sort();

  m_decimalBox->insertItem( " " );
  m_decimalBox->insertItem( "," );
  m_decimalBox->insertItem( "." );

  m_thousandsBox->insertItem( " " );
  m_thousandsBox->insertItem( "," );
  m_thousandsBox->insertItem( "." );

  m_editDescription->setEnabled(m_inEdit);
  m_editType->setEnabled(m_inEdit);
  m_editDateFormat->setEnabled(m_inEdit);
  m_editApostrophe->setEnabled(m_inEdit);
  m_editAmounts->setEnabled(m_inEdit);
  m_decimalBox->setEnabled(m_inEdit);
  m_thousandsBox->setEnabled(m_inEdit);
  m_editOpeningBalance->setEnabled(m_inEdit);
  m_editAccountDelimiter->setEnabled(m_inEdit);
  m_editVoidMark->setEnabled(m_inEdit);
  m_editInputFilterLocation->setEnabled(m_inEdit);
  m_editOutputFilterLocation->setEnabled(m_inEdit);
  m_editInputFilterFileType->setEnabled(m_inEdit);

  if(!m_inEdit) {
    m_renameButton->hide();
    m_deleteButton->hide();
    m_resetButton->hide();
    m_newButton->hide();
  }
}

void MyMoneyQifProfileEditor::loadProfileListFromConfig(void)
{
  QFontMetrics fontMetrics(m_profileListBox->font());
  int w = 100;      // minimum is 100 pixels width for the list box

  if(m_profile.isDirty()) {
    m_profile.saveProfile();
    m_isDirty = true;
  }

  m_profileListBox->clear();

  QStringList list;
  KConfig* config = KGlobal::config();
  config->setGroup("Profiles");
  list = config->readListEntry("profiles");

  if(list.count() == 0) {
    m_profile.clear();
    m_profile.setProfileDescription(i18n("The default QIF profile"));
    addProfile("Default");

    config->setGroup("Profiles");
    list = config->readListEntry("profiles");
  }

  list.sort();

  m_profileListBox->insertStringList(list);
  if(list.count() > 0) {
    m_profileListBox->setSelected(0, true);
    slotLoadProfileFromConfig(list[0]);
  }
  for(unsigned int i = 0; i < list.count(); ++i) {
    int nw = fontMetrics.width(list[i]) + 10;
    w = QMAX( w, nw );
  }
  w = QMIN(w, 200);
  m_profileListBox->setMinimumWidth(w);
}

void MyMoneyQifProfileEditor::slotLoadProfileFromConfig(const QString& profile)
{
  QString profileName = profile;

  if(m_profile.isDirty()) {
    m_profile.saveProfile();
    m_isDirty = true;
  }

  if(m_profileListBox->findItem(profileName, Qt::ExactMatch | Qt::CaseSensitive) == NULL) {
    profileName = m_profileListBox->text(0);
  }

  m_profile.loadProfile("Profile-" + profileName);

  QListBoxItem *lbi = m_profileListBox->findItem(profileName, Qt::ExactMatch | Qt::CaseSensitive);
  int idx = m_profileListBox->index(lbi);
  showProfile();
  if(idx >= 0) {
    m_profileListBox->setSelected(idx, true);
  }
}

void MyMoneyQifProfileEditor::showProfile(void)
{
  m_editDescription->setText(m_profile.profileDescription());
  m_editType->setText(m_profile.profileType());
  m_editOpeningBalance->setText(m_profile.openingBalanceText());
  m_editAccountDelimiter->setText(m_profile.accountDelimiter());
  m_editVoidMark->setText(m_profile.voidMark());
  m_editInputFilterLocation->setURL(m_profile.filterScriptImport());
  m_editOutputFilterLocation->setURL(m_profile.filterScriptExport());
  m_editInputFilterFileType->setText(m_profile.filterFileType());
	  
  m_editDateFormat->setCurrentText(m_profile.dateFormat());
  m_editApostrophe->setCurrentText(m_profile.apostropheFormat());

  m_attemptMatch->setChecked(m_profile.attemptMatchDuplicates());

  QListViewItem* item;
  QListViewItemIterator it(m_editAmounts);

  while((item = it.current()) != 0) {
    QChar key = item->text(1)[0];
    item->setText(2, m_profile.amountDecimal(key));
    item->setText(3, m_profile.amountThousands(key));
    if(m_selectedAmountType == 0 && key == 'T' && m_inEdit) {
      m_editAmounts->setSelected(item, true);
      slotAmountTypeSelected(item);
    } else if(item == m_selectedAmountType) {
      slotAmountTypeSelected(item);
    }
    ++it;
  }
}

void MyMoneyQifProfileEditor::deleteProfile(const QString& name)
{
  KConfig* config = KGlobal::config();

  config->deleteGroup("Profile-" + name);

  config->setGroup("Profiles");
  QStringList list = config->readListEntry("profiles");
  list.remove(name);

  config->writeEntry("profiles", list);
  m_isDirty = true;
}

void MyMoneyQifProfileEditor::addProfile(const QString& name)
{
  KConfig* config = KGlobal::config();
  config->setGroup("Profiles");
  QStringList list = config->readListEntry("profiles");

  list += name;
  list.sort();
  config->writeEntry("profiles", list);

  m_profile.setProfileName("Profile-" + name);
  m_profile.saveProfile();

  m_isDirty = true;
}

void MyMoneyQifProfileEditor::slotOk(void)
{
  if(m_profile.isDirty())
    m_isDirty = true;

  m_profile.saveProfile();

  KConfig* config = KGlobal::config();
  config->sync();

  m_isAccepted = true;
  accept();
}

void MyMoneyQifProfileEditor::slotReset(void)
{
  // first flush any changes
  m_profile.saveProfile();

  KConfig* config = KGlobal::config();
  config->rollback();
  config->reparseConfiguration();

  QString currentProfile = m_profile.profileName().mid(8);
  loadProfileListFromConfig();
  slotLoadProfileFromConfig(currentProfile);
  m_isDirty = false;
}

void MyMoneyQifProfileEditor::slotRename(void)
{
  bool ok;
  QString newName = enterName(ok);

  if(ok == true) {
    deleteProfile(m_profile.profileName().mid(8));
    addProfile(newName);
    loadProfileListFromConfig();
    slotLoadProfileFromConfig(newName);
  }
}

void MyMoneyQifProfileEditor::slotNew(void)
{
  bool ok;
  QString newName = enterName(ok);

  if(ok == true) {
    m_profile.clear();
    addProfile(newName);
    loadProfileListFromConfig();
    slotLoadProfileFromConfig(newName);
  }
}

const QString MyMoneyQifProfileEditor::enterName(bool& ok)
{
  MyMoneyQifProfileNameValidator val(this, "Validator");
#if KDE_IS_VERSION(3,2,0)
  return KInputDialog::getText(i18n("QIF Profile Editor"),
                               i18n("Enter new profile name"),
                               QString::null,
                               &ok,
                               this,
                               0,
                               &val,
                               0);
#else
  QString rc;

  // the blank in the next line as the value for the edit box is
  // there on purpose, so that with the following call to validateAndSet
  // the state is changed and the OK-Button is greyed
  KLineEditDlg* dlg = new KLineEditDlg(i18n("Enter new profile name"), " ", this);
  dlg->lineEdit()->setValidator(&val);
  dlg->lineEdit()->validateAndSet("", 0, 0, 0);

  ok = false;
  if(dlg->exec()) {
    ok = true;
  }
  rc = dlg->lineEdit()->text();
  delete dlg;

  return rc;
#endif
}

void MyMoneyQifProfileEditor::slotDelete(void)
{
  QString profile = m_profile.profileName().mid(8);

  if(KMessageBox::questionYesNo(this, i18n("Do you really want to delete profile '%1'?").arg(profile)) == KMessageBox::Yes) {
    int idx = m_profileListBox->currentItem();
    m_profile.saveProfile();
    deleteProfile(profile);
    loadProfileListFromConfig();
    if(idx >= static_cast<int> (m_profileListBox->count()))
      idx = m_profileListBox->count() - 1;

    slotLoadProfileFromConfig(m_profileListBox->text(idx));
  }
}

void MyMoneyQifProfileEditor::slotHelp(void)
{
  kapp->invokeHelp("details.impexp.qifimp.profile");
}

void MyMoneyQifProfileEditor::slotAmountTypeSelected(QListViewItem* item)
{
  m_decimalBox->setCurrentText(item->text(2));
  m_thousandsBox->setCurrentText(item->text(3));
  m_selectedAmountType = item;
}

void MyMoneyQifProfileEditor::slotDecimalChanged(const QString& val)
{
  if(m_selectedAmountType != 0) {
    QChar key = m_selectedAmountType->text(1)[0];
    m_profile.setAmountDecimal(key, val[0]);
    m_selectedAmountType->setText(2, val);
  }
}

void MyMoneyQifProfileEditor::slotThousandsChanged(const QString& val)
{
  if(m_selectedAmountType != 0) {
    QChar key = m_selectedAmountType->text(1)[0];
    m_profile.setAmountThousands(key, val[0]);
    m_selectedAmountType->setText(3, val);
  }
}

const QString MyMoneyQifProfileEditor::selectedProfile() const
{
  return m_profileListBox->currentText();
}

#include "mymoneyqifprofileeditor.moc"
