/***************************************************************************
                          kaccounttemplateselector.cpp  -  description
                             -------------------
    begin                : Tue Feb 5 2008
    copyright            : (C) 2008 by Thomas Baumgart
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qdir.h>
#include <qheader.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <klistview.h>
#include <ktextedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneytemplate.h>

#include "kaccounttemplateselector.h"

class KTemplateListItem : public KListViewItem
{
  public:
    KTemplateListItem(QListViewItem* parent, const QString& text);
    void setAvailable(void);
  public:
    bool m_isAvailable;
};

KTemplateListItem::KTemplateListItem(QListViewItem* parent, const QString& text) :
  KListViewItem(parent, text),
  m_isAvailable(false)
{
}

void KTemplateListItem::setAvailable(void)
{
  m_isAvailable = true;
}

class KAccountTemplateSelectorPrivate
{
  public:
    KAccountTemplateSelectorPrivate(KAccountTemplateSelector* p) { m_parent = p; }
#ifndef KMM_DESIGNER
    QValueList<MyMoneyTemplate> selectedTemplates(void) const;
    QListViewItem* hierarchyItem(const QString& parent, const QString& name);
    void loadHierarchy(void);
#endif

  public:
    KAccountTemplateSelector*         m_parent;
    QMap<QString, QListViewItem*>     m_templateHierarchy;
#ifndef KMM_DESIGNER
    QMap<QString, MyMoneyTemplate>    m_templates;
#endif
};


#ifndef KMM_DESIGNER
QListViewItem* KAccountTemplateSelectorPrivate::hierarchyItem(const QString& parent, const QString& name)
{
  if(!m_templateHierarchy.contains(parent)
  || m_templateHierarchy[parent] == 0) {
    QRegExp exp("(.*):(.*)");
    if(exp.search(parent) != -1)
      m_templateHierarchy[parent] = hierarchyItem(exp.cap(1), exp.cap(2));
  }
  return new KTemplateListItem(m_templateHierarchy[parent], name);
}

void KAccountTemplateSelectorPrivate::loadHierarchy(void)
{
  m_templateHierarchy.clear();
  QListViewItemIterator it(m_parent->m_groupList, QListViewItemIterator::Selected);
  QListViewItem* it_v;
  while((it_v = it.current()) != 0) {
    m_templates[it_v->text(2)].hierarchy(m_templateHierarchy);
    ++it;
  }

  // I need to think about this some more. The code works and shows
  // the current account hierarchy. It might be usefull, to show
  // existing accounts dimmed and the new ones in bold or so.
#if 0

  // add the hierarchy from the MyMoneyFile object
  QValueList<MyMoneyAccount> aList;
  QValueList<MyMoneyAccount>::const_iterator it_a;
  MyMoneyFile* file = MyMoneyFile::instance();
  file->accountList(aList);
  if(aList.count() > 0) {
    m_templateHierarchy[file->accountToCategory(file->asset().id(), true)] = 0;
    m_templateHierarchy[file->accountToCategory(file->liability().id(), true)] = 0;
    m_templateHierarchy[file->accountToCategory(file->income().id(), true)] = 0;
    m_templateHierarchy[file->accountToCategory(file->expense().id(), true)] = 0;
    m_templateHierarchy[file->accountToCategory(file->equity().id(), true)] = 0;
  }

  for(it_a = aList.begin(); it_a != aList.end(); ++it_a) {
    m_templateHierarchy[file->accountToCategory((*it_a).id(), true)] = 0;
  }
#endif

  m_parent->m_accountList->clear();
  QMap<QString, QListViewItem*>::iterator it_m;

  QRegExp exp("(.*):(.*)");
  for(it_m = m_templateHierarchy.begin(); it_m != m_templateHierarchy.end(); ++it_m) {
    if(exp.search(it_m.key()) == -1) {
      (*it_m) = new KListViewItem(m_parent->m_accountList, it_m.key());
    } else {
      (*it_m) = hierarchyItem(exp.cap(1), exp.cap(2));
    }
    (*it_m)->setOpen(true);
  }

  m_parent->m_description->clear();
  if(m_parent->m_groupList->currentItem()) {
    m_parent->m_description->setText(m_templates[m_parent->m_groupList->currentItem()->text(2)].longDescription());
  }
}

QValueList<MyMoneyTemplate> KAccountTemplateSelectorPrivate::selectedTemplates(void) const
{
  QValueList<MyMoneyTemplate> list;
  QListViewItemIterator it(m_parent->m_groupList, QListViewItemIterator::Selected);
  QListViewItem* it_v;
  while((it_v = it.current()) != 0) {
    list << m_templates[it_v->text(2)];
    ++it;
  }
  return list;
}
#endif


KAccountTemplateSelector::KAccountTemplateSelector(QWidget* parent, const char* name) :
  KAccountTemplateSelectorDecl(parent, name),
  d(new KAccountTemplateSelectorPrivate(this))
{
  m_accountList->header()->hide();
  loadTemplateList();
  slotLoadHierarchy();
  connect(m_groupList, SIGNAL(selectionChanged()), this, SLOT(slotLoadHierarchy()));
}

KAccountTemplateSelector::~KAccountTemplateSelector()
{
  delete d;
}

void KAccountTemplateSelector::loadTemplateList(void)
{
#ifndef KMM_DESIGNER
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
  d->m_templates.clear();
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
          d->m_templates[QString("%1").arg(id)] = templ;
          new KListViewItem(parent, templ.title(), templ.shortDescription(), QString("%1").arg(id));
          ++id;
        }
      }
    }
  }
#endif
}

void KAccountTemplateSelector::slotLoadHierarchy(void)
{
#ifndef KMM_DESIGNER
  d->loadHierarchy();
#endif
}

QValueList<MyMoneyTemplate> KAccountTemplateSelector::selectedTemplates(void) const
{
#ifndef KMM_DESIGNER
  return d->selectedTemplates();
#else
  return QValueList<MyMoneyTemplate>();
#endif
}

#include "kaccounttemplateselector.moc"
