/***************************************************************************
                          kmymoneyonlinequoteconfig.cpp  -  description
                             -------------------
    begin                : Thu Dec 30 2004
    copyright            : (C) 2004 by Thomas Baumgart
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

#include <qregexp.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <klistview.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyonlinequoteconfig.h"

kMyMoneyOnlineQuoteSource::kMyMoneyOnlineQuoteSource(const QString& name)
{
  m_name = name;
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup(m_name);
  m_sym = kconfig->readEntry("SymbolRegex");
  m_date = kconfig->readEntry("DateRegex");
  m_price = kconfig->readEntry("PriceRegex");
  m_url = kconfig->readEntry("URL");
}

void kMyMoneyOnlineQuoteSource::writeConfig(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup(m_name);
  kconfig->writeEntry("URL", m_url);
  kconfig->writeEntry("PriceRegex", m_price);
  kconfig->writeEntry("DateRegex", m_date);
  kconfig->writeEntry("SymbolRegex", m_sym);
}

void kMyMoneyOnlineQuoteSource::renameConfig(const QString& name)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->deleteGroup(m_name);
  m_name = name;
  writeConfig();
}

kMyMoneyOnlineQuoteConfig::kMyMoneyOnlineQuoteConfig(QWidget *parent, const char *name )
  : kMyMoneyOnlineQuoteConfigDecl(parent, name)
{
  QStringList groups = quoteSourceList();
  KConfig *kconfig = KGlobal::config();

  // compatibility hack to load the 'old' stuff
  if(groups.isEmpty()) {
    groups += "Yahoo";
    kconfig->setGroup("Online Quotes Options");
    QString source(kconfig->readEntry("URL","http://finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1"));
    QString symbolRegExp(kconfig->readEntry("SymbolRegex","\"([^,\"]*)\",.*"));
    QString dateRegExp(kconfig->readEntry("DateRegex","[^,]*,[^,]*,\"([^\"]*)\""));
    QString priceRegExp(kconfig->readEntry("PriceRegex","[^,]*,([^,]*),.*"));
    kconfig->deleteGroup("Online Quotes Options");

    kconfig->setGroup(groupName("Yahoo Stock"));
    kconfig->writeEntry("URL", source);
    kconfig->writeEntry("SymbolRegex", symbolRegExp);
    kconfig->writeEntry("PriceRegex", priceRegExp);
    kconfig->writeEntry("DateRegex", dateRegExp);

    kconfig->setGroup(groupName("Yahoo Currency"));
    kconfig->writeEntry("URL", "http://finance.yahoo.com/d/quotes.csv?s=%1%2=X&f=sl1d1");
    kconfig->writeEntry("SymbolRegex", symbolRegExp);
    kconfig->writeEntry("PriceRegex", priceRegExp);
    kconfig->writeEntry("DateRegex", dateRegExp);
    kconfig->sync();
  }

  loadList(true);

  m_updateButton->setEnabled(false);

  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem updateButtenItem( i18n("&Update" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Accepts the entered data and stores it"),
                    i18n("Use this to accept the modified data."));
  m_updateButton->setGuiItem(updateButtenItem);

  KGuiItem deleteButtenItem( i18n( "&Delete" ),
                      QIconSet(il->loadIcon("editdelete", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Delete the selected source entry"),
                      i18n("Use this to delete the selected online source entry"));
  m_deleteButton->setGuiItem(deleteButtenItem);

  KGuiItem newButtenItem( i18n( "&New..." ),
                      QIconSet(il->loadIcon("filenew", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Create a new source entry for online quotes"),
                      i18n("Use this to create a new entry for online quotes"));
  m_newButton->setGuiItem(newButtenItem);

  connect(m_updateButton, SIGNAL(clicked()), this, SLOT(slotUpdateEntry()));
  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotNewEntry()));

  connect(m_quoteSourceList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotLoadWidgets(QListViewItem*)));
  connect(m_quoteSourceList, SIGNAL(clicked(QListViewItem*)), this, SLOT(slotLoadWidgets(QListViewItem*)));
  connect(m_quoteSourceList, SIGNAL(itemRenamed(QListViewItem*,const QString&,int)), this, SLOT(slotEntryRenamed(QListViewItem*,const QString&,int)));

  connect(m_editURL, SIGNAL(textChanged(const QString&)), this, SLOT(slotEntryChanged()));
  connect(m_editSymbol, SIGNAL(textChanged(const QString&)), this, SLOT(slotEntryChanged()));
  connect(m_editDate, SIGNAL(textChanged(const QString&)), this, SLOT(slotEntryChanged()));
  connect(m_editPrice, SIGNAL(textChanged(const QString&)), this, SLOT(slotEntryChanged()));

  // FIXME deleting a source is not yet implemented
  m_deleteButton->setEnabled(false);
}

void kMyMoneyOnlineQuoteConfig::loadList(const bool updateResetList)
{
  QStringList groups = quoteSourceList();
  KConfig *kconfig = KGlobal::config();

  if(updateResetList)
    m_resetList.clear();
  m_quoteSourceList->clear();
  QStringList::Iterator it;
  for(it = groups.begin(); it != groups.end(); ++it) {
    kconfig->setGroup(groupName(*it));
    new QListViewItem(m_quoteSourceList, *it);
    if(updateResetList)
      m_resetList += kMyMoneyOnlineQuoteSource(groupName(*it));
  }

  QListViewItem* first = m_quoteSourceList->firstChild();
  if(first)
    m_quoteSourceList->setSelected(first, true);
  slotLoadWidgets(first);

  m_newButton->setEnabled(m_quoteSourceList->findItem(i18n("New Quote Source"), 0) == 0);
}

QStringList kMyMoneyOnlineQuoteConfig::quoteSourceList(void)
{
  KConfig *kconfig = KGlobal::config();
  QStringList groups = kconfig->groupList();
  QStringList::Iterator it;
  QRegExp onlineQuoteSource(QString("^Online-Quote-Source-(.*)$"));

  // get rid of all 'non online quote source' entries
  for(it = groups.begin(); it != groups.end(); it = groups.remove(it)) {
    if(onlineQuoteSource.search(*it) >= 0) {
      // Insert the name part
      groups.insert(it, onlineQuoteSource.cap(1));
    }
  }

  return groups;
}

const QString kMyMoneyOnlineQuoteConfig::groupName(const QString& name) const
{
  return QString("Online-Quote-Source-%1").arg(name);
}

void kMyMoneyOnlineQuoteConfig::resetConfig(void)
{
  QStringList::Iterator it;
  QStringList groups = quoteSourceList();
  KConfig *kconfig = KGlobal::config();

  // delete all currently defined entries
  for(it = groups.begin(); it != groups.end(); ++it) {
    kconfig->deleteGroup(groupName(*it));
  }

  // and write back the one's from the reset list
  QValueList<kMyMoneyOnlineQuoteSource>::Iterator itr;
  for(itr = m_resetList.begin(); itr != m_resetList.end(); ++itr) {
    (*itr).writeConfig();
  }

  loadList();
}

void kMyMoneyOnlineQuoteConfig::slotLoadWidgets(QListViewItem* item)
{
  m_editURL->setEnabled(true);
  m_editSymbol->setEnabled(true);
  m_editPrice->setEnabled(true);
  m_editDate->setEnabled(true);
  m_editURL->setText(QString());
  m_editSymbol->setText(QString());
  m_editPrice->setText(QString());
  m_editDate->setText(QString());

  if(item) {
    m_currentItem = kMyMoneyOnlineQuoteSource(groupName(item->text(0)));
    m_editURL->setText(m_currentItem.m_url);
    m_editSymbol->setText(m_currentItem.m_sym);
    m_editPrice->setText(m_currentItem.m_price);
    m_editDate->setText(m_currentItem.m_date);

  } else {
    m_editURL->setEnabled(false);
    m_editSymbol->setEnabled(false);
    m_editPrice->setEnabled(false);
    m_editDate->setEnabled(false);
  }

  m_updateButton->setEnabled(false);

}

void kMyMoneyOnlineQuoteConfig::slotEntryChanged(void)
{
  bool modified = m_editURL->text() != m_currentItem.m_url
               || m_editSymbol->text() != m_currentItem.m_sym
               || m_editDate->text() != m_currentItem.m_date
               || m_editPrice->text() != m_currentItem.m_price;

  m_updateButton->setEnabled(modified);
}

void kMyMoneyOnlineQuoteConfig::slotUpdateEntry(void)
{
  m_currentItem.m_url = m_editURL->text();
  m_currentItem.m_sym = m_editSymbol->text();
  m_currentItem.m_date = m_editDate->text();
  m_currentItem.m_price = m_editPrice->text();
  m_currentItem.writeConfig();
  slotEntryChanged();
}

void kMyMoneyOnlineQuoteConfig::slotNewEntry(void)
{
  kMyMoneyOnlineQuoteSource newSource(groupName(i18n("New Quote Source")));
  newSource.writeConfig();
  loadList();
  QListViewItem* item = m_quoteSourceList->findItem(i18n("New Quote Source"), 0);
  if(item) {
    m_quoteSourceList->setSelected(item, true);
    slotLoadWidgets(item);
  }
}

void kMyMoneyOnlineQuoteConfig::slotEntryRenamed(QListViewItem* item, const QString& text, int /* col */)
{
  int nameCount = 0;
  QListViewItemIterator it(m_quoteSourceList);
  while(it.current()) {
    if(it.current()->text(0) == text)
      ++nameCount;
    ++it;
  }

  // Make sure we get a non-empty and unique name
  if(text.length() > 0 && nameCount == 1) {
    m_currentItem.renameConfig(groupName(text));
  } else {
    QRegExp onlineQuoteSource(QString("^Online-Quote-Source-(.*)$"));
    onlineQuoteSource.search(m_currentItem.m_name);
    item->setText(0, onlineQuoteSource.cap(1));
  }
  m_newButton->setEnabled(m_quoteSourceList->findItem(i18n("New Quote Source"), 0) == 0);
}
