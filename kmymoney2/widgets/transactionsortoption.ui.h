/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kiconloader.h>
#include <klocale.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/register.h>
#include "sortoptionlistitem.h"


void TransactionSortOption::init()
{
  KIconLoader* il = KGlobal::iconLoader();
  m_addButton->setIconSet(QIconSet(il->loadIcon("1rightarrow", KIcon::Small, KIcon::SizeSmall)));
  m_removeButton->setIconSet(QIconSet(il->loadIcon("1leftarrow", KIcon::Small, KIcon::SizeSmall)));
  m_upButton->setIconSet(QIconSet(il->loadIcon("1uparrow", KIcon::Small, KIcon::SizeSmall)));
  m_downButton->setIconSet(QIconSet(il->loadIcon("1downarrow", KIcon::Small, KIcon::SizeSmall)));

  // don't allow sorting of the selected entries
  m_selectedList->setSortColumn(-1);

  // defaults to "post date, value" sorting
  // setSettings(QString("1,4"));
  setSettings(QString());

  QListViewItem* p;
  if((p = m_availableList->firstChild()) != 0) {
    m_availableList->setSelected(p, true);
  }
}

/**
  * Setup the two lists according to the elements found in @a list.
  * If negative, the will show up in the available list, if positive,
  * they show up in the selected list.
  */
void TransactionSortOption::setSettings(const QString& settings)
{
  m_availableList->clear();
  m_selectedList->clear();

  QStringList list = QStringList::split(',', settings);
  QMap<int, bool> selectedMap;

  // fill selected list
  QStringList::const_iterator it_s;
  QListViewItem* last = 0;
  for(it_s = list.begin(); it_s != list.end(); ++it_s) {
    int val = (*it_s).toInt();
    selectedMap[abs(val)] = true;
    last = addEntry(m_selectedList, last, val);
  }

  // fill available list
  QMap<int, bool>::const_iterator it_m;
  for(int i = static_cast<int>(KMyMoneyRegister::PostDateSort);
      i < static_cast<int>(KMyMoneyRegister::MaxSortFields); ++i) {
    if(selectedMap.find(i) == selectedMap.end()) {
      int val = i;
      if(i == static_cast<int>(KMyMoneyRegister::ValueSort))
        val = -val;
      addEntry(m_availableList, 0, val);
    }
  }
}

QListViewItem* TransactionSortOption::addEntry( KListView * p, QListViewItem* after, int idx )
{
  QString txt = KMyMoneyRegister::sortOrderToText(static_cast<KMyMoneyRegister::TransactionSortField>(abs(idx)));
  if(txt.isEmpty())
    txt = "Unknown";    // i18n should be handled in sortOptionToText()

  return new SortOptionListItem(p, after, txt, idx);
}

void TransactionSortOption::toggleDirection(QListViewItem* item)
{
  SortOptionListItem* p = dynamic_cast<SortOptionListItem*>(item);
  if(p) {
    p->toggleDirection();
    emit settingsChanged(settings());
  }
}

QString TransactionSortOption::settings( void ) const
{
  QString rc;
  SortOptionListItem* item = dynamic_cast<SortOptionListItem*>(m_selectedList->firstChild());
  while(item) {
    rc += QString::number(KMyMoneyRegister::textToSortOrder(item->text(0))*item->direction());
    item = dynamic_cast<SortOptionListItem*>(item->itemBelow());
    if(item != 0)
      rc += ",";
  }
  return rc;
}

void TransactionSortOption::slotAvailableSelected( QListViewItem * item )
{
  m_addButton->setEnabled(item != 0);
  m_removeButton->setDisabled(true);
  m_upButton->setDisabled(true);
  m_downButton->setDisabled(true);

  QListViewItem* p = m_selectedList->currentItem();
  if(p) {
    m_selectedList->setSelected(p, false);
  }
}

void TransactionSortOption::slotSelectedSelected( QListViewItem * item )
{
  m_addButton->setDisabled(true);
  m_removeButton->setEnabled(item != 0);
  if(item) {
    m_upButton->setEnabled(item->itemAbove() != 0);
    m_downButton->setEnabled(item->itemBelow() != 0);
  } else {
    m_upButton->setEnabled(false);
    m_downButton->setEnabled(false);
  }

  QListViewItem* p = m_availableList->currentItem();
  if(p) {
    m_availableList->setSelected(p, false);
  }
}

void TransactionSortOption::slotAddItem( void )
{
  QListViewItem* item;
  if((item = m_availableList->currentItem()) != 0) {
    QListViewItem* next = item->itemBelow();
    if(!next)
      next = item->itemAbove();
    m_availableList->takeItem(item);
    m_selectedList->insertItem(item);
    m_addButton->setEnabled(m_availableList->firstChild() != 0);
    if(next) {
      m_availableList->setCurrentItem(next);
      m_availableList->setSelected(next, true);
    }
    emit settingsChanged(settings());
  }
}

void TransactionSortOption::slotRemoveItem( void )
{
  QListViewItem* item;
  if((item = m_selectedList->currentItem()) != 0) {
    QListViewItem* next = item->itemBelow();
    if(!next)
      next = item->itemAbove();
    m_selectedList->takeItem(item);
    m_availableList->insertItem(item);
    m_removeButton->setEnabled(m_selectedList->firstChild() != 0);
    if(next) {
      m_selectedList->setCurrentItem(next);
      m_selectedList->setSelected(next, true);
    }
    emit settingsChanged(settings());
  }
}

void TransactionSortOption::slotUpItem( void )
{
  QListViewItem* item;
  if((item = m_selectedList->currentItem()) != 0) {
    QListViewItem* prev = item->itemAbove();
    if(prev) {
      prev->moveItem(item);
      m_selectedList->setCurrentItem(item);
      m_selectedList->setSelected(item, true);
      m_upButton->setEnabled(item->itemAbove() != 0);
      m_downButton->setEnabled(item->itemBelow() != 0);
      emit settingsChanged(settings());
    }
  }
}

void TransactionSortOption::slotDownItem( void )
{
  QListViewItem* item;
  if((item = m_selectedList->currentItem()) != 0) {
    QListViewItem* next = item->itemBelow();
    if(next) {
      item->moveItem(next);
      m_selectedList->setCurrentItem(item);
      m_selectedList->setSelected(item, true);
      m_upButton->setEnabled(item->itemAbove() != 0);
      m_downButton->setEnabled(item->itemBelow() != 0);
      emit settingsChanged(settings());
    }
  }
}
