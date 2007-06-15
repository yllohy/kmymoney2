/***************************************************************************
                             knewuserwizard.cpp
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <locale.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qpixmap.h>
#include <qbitmap.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <klistview.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <ktextedit.h>
#include <kuser.h>
#include <kurlrequester.h>
#include <kio/netaccess.h>
#include <kurl.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewuserwizard.h"
#include "knewuserwizard_p.h"
#include <kmymoney/mymoneysecurity.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kguiutils.h>
#include <kmymoney/kmymoneyaccounttree.h>
#include <kmymoney/mymoneypayee.h>
#include <kmymoney/mymoneymoney.h>
#include <kmymoney/mymoneyinstitution.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneyedit.h>

#include "../../kmymoney2.h"

using namespace NewUserWizard;

NewUserWizard::Wizard::Wizard(QWidget *parent, const char *name, bool modal, WFlags flags)
  : KMyMoneyWizard(parent, name, modal, flags)
{
  setTitle(i18n("KMyMoney New File Setup"));
  addStep(i18n("Personal Data"));
  addStep(i18n("Select Currency"));
  addStep(i18n("Select Accounts"));
  addStep(i18n("Set preferences"));
  addStep(i18n("Finish"));

  m_generalPage = new GeneralPage(this);
  m_currencyPage = new CurrencyPage(this);
  m_accountPage = new AccountPage(this);
  m_categoriesPage = new CategoriesPage(this);
  m_preferencePage = new PreferencePage(this);
  m_filePage = new FilePage(this);

  m_accountPage->m_haveCheckingAccountButton->setChecked(true);
  setFirstPage(m_generalPage);
}

MyMoneyPayee NewUserWizard::Wizard::user(void) const
{
  return m_generalPage->user();
}

QString NewUserWizard::Wizard::url(void) const
{
  return m_filePage->m_dataFileEdit->url();
}

MyMoneyInstitution NewUserWizard::Wizard::institution(void) const
{
  MyMoneyInstitution inst;
  if(m_accountPage->m_haveCheckingAccountButton->isChecked()) {
    if(m_accountPage->m_institutionNameEdit->text().length()) {
      inst.setName(m_accountPage->m_institutionNameEdit->text());
      if(m_accountPage->m_institutionNumberEdit->text().length())
        inst.setSortcode(m_accountPage->m_institutionNumberEdit->text());
    }
  }
  return inst;
}

MyMoneyAccount NewUserWizard::Wizard::account(void) const
{
  MyMoneyAccount acc;
  if(m_accountPage->m_haveCheckingAccountButton->isChecked()) {
    acc.setName(m_accountPage->m_accountNameEdit->text());
    if(m_accountPage->m_accountNumberEdit->text().length())
      acc.setNumber(m_accountPage->m_accountNumberEdit->text());
    acc.setOpeningDate(m_accountPage->m_openingDateEdit->date());
    acc.setCurrencyId(m_baseCurrency.id());
    acc.setAccountType(MyMoneyAccount::Checkings);
  }
  return acc;
}

MyMoneyMoney NewUserWizard::Wizard::openingBalance(void) const
{
  return m_accountPage->m_openingBalanceEdit->value();
}

MyMoneySecurity NewUserWizard::Wizard::baseCurrency(void) const
{
  return m_baseCurrency;
}

QValueList<MyMoneyTemplate> NewUserWizard::Wizard::templates(void) const
{
  return m_categoriesPage->selectedTemplates();
}

GeneralPage::GeneralPage(Wizard* wizard, const char* name) :
  UserInfo(wizard),
  WizardPage<Wizard>(1, this, wizard, name)
{
  m_userNameEdit->setFocus();
}

KMyMoneyWizardPage* GeneralPage::nextPage(void) const
{
  return m_wizard->m_currencyPage;
}

CurrencyPage::CurrencyPage(Wizard* wizard, const char* name) :
  Currency(wizard),
  WizardPage<Wizard>(2, this, wizard, name)
{
  QListViewItem *first = 0;
  QValueList<MyMoneySecurity> list = MyMoneyFile::instance()->currencyList();
  QValueList<MyMoneySecurity>::const_iterator it;

  QCString localCurrency(localeconv()->int_curr_symbol);
  localCurrency.truncate(3);

  QCString baseCurrency = MyMoneyFile::instance()->baseCurrency().id();

  // construct a transparent 16x16 pixmap
  QPixmap empty(16, 16);
  empty.setMask(QBitmap(16, 16, true));

  m_currencyList->clear();
  for(it = list.begin(); it != list.end(); ++it) {
    QListViewItem* p = insertCurrency(*it);
    if((*it).id() == baseCurrency) {
      first = p;
      p->setPixmap(0, QPixmap( locate("icon","hicolor/16x16/apps/kmymoney2.png")));
    } else {
      p->setPixmap(0, empty);
    }
    if(!first && (*it).id() == localCurrency)
      first = p;
  }

  if(first == 0)
    first = m_currencyList->firstChild();
  if(first != 0) {
    m_currencyList->setCurrentItem(first);
    m_currencyList->setSelected(first, true);
    m_currencyList->ensureItemVisible(first);
  }
}


KMyMoneyWizardPage* CurrencyPage::nextPage(void) const
{
  m_wizard->m_baseCurrency = MyMoneyFile::instance()->security(selectedCurrency());
  m_wizard->m_accountPage->m_accountCurrencyLabel->setText(m_wizard->m_baseCurrency.tradingSymbol());
  return m_wizard->m_accountPage;
}

AccountPage::AccountPage(Wizard* wizard, const char* name) :
  KAccountPageDecl(wizard, name),
  WizardPage<Wizard>(3, this, wizard, name)
{
  m_mandatoryGroup = new kMandatoryFieldGroup(this);
  m_mandatoryGroup->add(m_accountNameEdit);
  connect(m_mandatoryGroup, SIGNAL(stateChanged()), object(), SIGNAL(completeStateChanged()));
  connect(m_haveCheckingAccountButton, SIGNAL(toggled(bool)), object(), SIGNAL(completeStateChanged()));
  m_accountNameEdit->setFocus();
  m_openingDateEdit->setDate(QDate(QDate::currentDate().year(),1,1));
}

KMyMoneyWizardPage* AccountPage::nextPage(void) const
{
  return m_wizard->m_categoriesPage;
}

bool AccountPage::isComplete(void) const
{
  return !m_haveCheckingAccountButton->isChecked() || m_mandatoryGroup->isEnabled();
}

CategoriesPage::CategoriesPage(Wizard* wizard, const char* name) :
  Accounts(wizard),
  WizardPage<Wizard>(3, this, wizard, name)
{
  loadTemplateList();
  connect(m_groupList, SIGNAL(selectionChanged()), this, SLOT(slotLoadHierarchy()));
}

KMyMoneyWizardPage* CategoriesPage::nextPage(void) const
{
  return m_wizard->m_preferencePage;
}

void CategoriesPage::loadTemplateList(void)
{
  QMap<QString, QString> countries;
  QStringList dirs;
  // get list of template subdirs and scan them for the list of subdirs
  QStringList list = KGlobal::dirs()->findDirs("appdata", "templates");
  QStringList::iterator it;
  for(it = list.begin(); it != list.end(); ++it) {
    QDir dir(*it);
    // qDebug("Reading dir '%s' with %d entries", (*it).data(), dir.count());
    dirs = dir.entryList("*", QDir::Dirs);
    QStringList::iterator it_d;
    for(it_d= dirs.begin(); it_d != dirs.end(); ++it_d) {
      // we don't care about . and ..
      if((*it_d) == ".." || (*it_d) == "." || (*it_d) == "C")
        continue;
      QRegExp exp("(..)_(..)");
      if(exp.search(*it_d) != -1) {
        QString country = KGlobal::locale()->twoAlphaToCountryName(exp.cap(2));
        if(country.isEmpty())
          country = exp.cap(2);
        QString lang = KGlobal::locale()->twoAlphaToLanguageName(exp.cap(1));
        if(countries.contains(country)) {
          if(countries[country] != *it_d) {
            QString oName = countries[country];
            exp.search(oName);
            QString oCountry = KGlobal::locale()->twoAlphaToCountryName(exp.cap(2));
            QString oLang = KGlobal::locale()->twoAlphaToLanguageName(exp.cap(1));
            countries.remove(oName);
            countries[QString("%1 (%2)").arg(oCountry).arg(oLang)] = oName;
            countries[QString("%1 (%2)").arg(country).arg(lang)] = *it_d;
          }
        } else {
          countries[country] = *it_d;
        }
      } else {
        qDebug("'%s/%s' not scanned", (*it).data(), (*it_d).data());
      }
    }
  }

  // now that we know, what we can get at max, we scan everything
  // and parse the templates into memory
  QMap<QString, QString>::iterator it_m;
  m_groupList->clear();
  m_templates.clear();
  int id = 1;
  for(it_m = countries.begin(); it_m != countries.end(); ++it_m) {
    // create new top item for each language
    KListViewItem* parent = new KListViewItem(m_groupList, it_m.key());
    parent->setSelectable(false);
    for(it = list.begin(); it != list.end(); ++it) {
      QStringList::iterator it_f;
      QDir dir(QString("%1%2").arg(*it).arg(*it_m));
      if(dir.exists()) {
        QStringList files = dir.entryList("*", QDir::Files);
        for(it_f = files.begin(); it_f != files.end(); ++it_f) {
          MyMoneyTemplate templ(QString("%1/%2").arg(dir.canonicalPath()).arg(*it_f));
          m_templates[QString("%1").arg(id)] = templ;
          new KListViewItem(parent, templ.title(), templ.shortDescription(), QString("%1").arg(id));
          ++id;
        }
      }
    }
  }
}

QListViewItem* CategoriesPage::hierarchyItem(const QString& parent, const QString& name)
{
  if(!m_templateHierarchy.contains(parent)
  || m_templateHierarchy[parent] == 0) {
    QRegExp exp("(.*):(.*)");
    if(exp.search(parent) != -1)
      m_templateHierarchy[parent] = hierarchyItem(exp.cap(1), exp.cap(2));
  }
  return new KListViewItem(m_templateHierarchy[parent], name);
}

void CategoriesPage::slotLoadHierarchy(void)
{
  m_templateHierarchy.clear();
  QListViewItemIterator it(m_groupList, QListViewItemIterator::Selected);
  QListViewItem* it_v;
  while((it_v = it.current()) != 0) {
    m_templates[it_v->text(2)].hierarchy(m_templateHierarchy);
    ++it;
  }

  m_accountList->clear();
  QMap<QString, QListViewItem*>::iterator it_m;

  QRegExp exp("(.*):(.*)");
  for(it_m = m_templateHierarchy.begin(); it_m != m_templateHierarchy.end(); ++it_m) {
    if(exp.search(it_m.key()) == -1) {
      (*it_m) = new KListViewItem(m_accountList, it_m.key());
    } else {
      (*it_m) = hierarchyItem(exp.cap(1), exp.cap(2));
    }
    (*it_m)->setOpen(true);
  }

  m_description->clear();
  if(m_groupList->currentItem()) {
    m_description->setText(m_templates[m_groupList->currentItem()->text(2)].longDescription());
  }
}

QValueList<MyMoneyTemplate> CategoriesPage::selectedTemplates(void) const
{
  QValueList<MyMoneyTemplate> list;
  QListViewItemIterator it(m_groupList, QListViewItemIterator::Selected);
  QListViewItem* it_v;
  while((it_v = it.current()) != 0) {
    list << m_templates[it_v->text(2)];
    ++it;
  }
  return list;
}

PreferencePage::PreferencePage(Wizard* wizard, const char* name) :
  KPreferencePageDecl(wizard),
  WizardPage<Wizard>(4, this, wizard, name)
{
  connect(m_openConfigButton, SIGNAL(clicked()), kmymoney2, SLOT(slotSettings()));
}

KMyMoneyWizardPage* PreferencePage::nextPage(void) const
{
  return m_wizard->m_filePage;
}

FilePage::FilePage(Wizard* wizard, const char* name) :
  KFilePageDecl(wizard),
  WizardPage<Wizard>(5, this, wizard, name)
{
  m_mandatoryGroup = new kMandatoryFieldGroup(this);
  m_mandatoryGroup->add(m_dataFileEdit->lineEdit());
  connect(m_mandatoryGroup, SIGNAL(stateChanged()), object(), SIGNAL(completeStateChanged()));

  KUser user;
  m_dataFileEdit->setShowLocalProtocol(false);
  m_dataFileEdit->setURL(locateLocal("appdata", QString("%1.kmy").arg(user.loginName())));
}

bool FilePage::isComplete(void) const
{
  bool rc = m_mandatoryGroup->isEnabled();
  if(rc) {
    // if a filename is present, check that
    // a) the file does not exist
    // b) the directory does exist
    rc = !KIO::NetAccess::exists(m_dataFileEdit->url(), false, m_wizard);
    if(rc) {
      QRegExp exp("(.*)/(.*)");
      rc = false;
      if(exp.search(m_dataFileEdit->url()) != -1) {
        if(exp.cap(2).length() > 0) {
          rc = KIO::NetAccess::exists(exp.cap(1), true, m_wizard);
        }
      }
    }
  }
  return rc;
}

#include "knewuserwizard.moc"
