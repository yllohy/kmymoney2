/***************************************************************************
                          kcategoriesview.cpp  -  description
                             -------------------
    begin                : Sun Jan 20 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <klistview.h>
#include <klocale.h>
#include <kglobal.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <kmessagebox.h>
#include <kconfig.h>

#include <qheader.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "kcategoriesview.h"
#include "../dialogs/kcategorylistitem.h"
#include "../dialogs/knewcategorydlg.h"

KCategoriesView::KCategoriesView(MyMoneyFile *file, QWidget *parent, const char *name )
  : kCategoriesViewDecl(parent,name)
{
//  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_edit_categories.png");
//  QPixmap *pm = new QPixmap(filename);
//  m_qpixmaplabel->setPixmap(*pm);

  m_file = file;
  categoryListView->setRootIsDecorated(true);
  categoryListView->addColumn(i18n("Category"));
  categoryListView->addColumn(i18n("Type"));
  categoryListView->setMultiSelection(false);
  categoryListView->setColumnWidthMode(0, QListView::Manual);
  categoryListView->header()->setResizeEnabled(false);
  categoryListView->setAllColumnsShowFocus(true);

  // never show a horizontal scroll bar
  categoryListView->setHScrollBarMode(QScrollView::AlwaysOff);

  categoryListView->setSorting(-1);

  readConfig();
	
	connect(categoryListView, SIGNAL(selectionChanged(QListViewItem*)),
	  this, SLOT(slotSelectionChanged(QListViewItem*)));
	connect(buttonEdit, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
	connect(buttonNew, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
  connect(buttonDelete, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));
}

KCategoriesView::~KCategoriesView()
{
  writeConfig();
}

void KCategoriesView::refresh(void)
{
  categoryListView->clear();

  KCategoryListItem *saveptr=0;

  QListIterator<MyMoneyCategory> it = m_file->categoryIterator();
  for ( ; it.current(); ++it ) {
    MyMoneyCategory *data = it.current();
    // Construct a new list item using appropriate arguments.
    // See KCategoryListItem
    KCategoryListItem *item0 = new KCategoryListItem(categoryListView, data->name(), data->minorCategories(), data->isIncome(), true);
    if (data->name()==m_lastCat)
      saveptr = item0;
    for ( QStringList::Iterator it2 = data->minorCategories().begin(); it2 != data->minorCategories().end(); ++it2 ) {
      (void) new KCategoryListItem(item0, (*it2), data->isIncome(), false, item0->text(0));
    }
    categoryListView->setOpen(item0, true);
  }
  if (saveptr)
    categoryListView->setCurrentItem(saveptr);
}

void KCategoriesView::show()
{
  refresh();
  emit signalViewActivated();
  QWidget::show();
}

void KCategoriesView::resizeEvent(QResizeEvent* e)
{
  categoryListView->setColumnWidth(0, categoryListView->width()-105);
  categoryListView->setColumnWidth(1, 100);

  // call base class resizeEvent()
  kCategoriesViewDecl::resizeEvent(e);
}

void KCategoriesView::slotNewClicked()
{
  // Uses the class KNewCategoryDlg
  MyMoneyCategory category;
  KNewCategoryDlg dlg(&category, this);
  if (!dlg.exec())
    return;

  m_file->addCategory(category.isIncome(), category.name(), category.minorCategories());
  categoryListView->clear();
  refresh();
}

void KCategoriesView::slotDeleteClicked()
{
  KCategoryListItem *item = (KCategoryListItem *)categoryListView->selectedItem();
  if (!item)
    return;

  QString prompt;
  if (item->isMajor()) {
    prompt = i18n("By deleting a major category all minor(s) will be lost.\nAre you sure you want to delete: ");
    prompt += item->text(0);
  } else {
    prompt = i18n("Delete this minor category item: ");
    prompt += item->text(0);
    prompt += i18n(" in major category: ");
    prompt += item->majorName();
  }

  if ((KMessageBox::questionYesNo(this, prompt))==KMessageBox::Yes) {
    if (item->isMajor())
      m_file->removeMajorCategory(item->text(0));
    else
      m_file->removeMinorCategory(item->majorName(), item->text(0));
  }
  categoryListView->clear();
  refresh();
}

void KCategoriesView::slotSelectionChanged(QListViewItem* item)
{
  KCategoryListItem *kitem = (KCategoryListItem *)item;
  if (!kitem) {
    buttonEdit->setEnabled(false);
    buttonDelete->setEnabled(false);
  }
  else if(kitem->isMajor()) {
    buttonEdit->setEnabled(true);
    buttonDelete->setEnabled(true);
  } else {
    buttonEdit->setEnabled(false);
    buttonDelete->setEnabled(true);
  }
}

void KCategoriesView::slotEditClicked()
{
  KCategoryListItem *item = (KCategoryListItem *)categoryListView->selectedItem();
  if (!item)
    return;

  QString prompt;
  if(item->isMajor()) {
    MyMoneyCategory category(item->income(), item->text(0), item->minors());

    KNewCategoryDlg dlg(&category, this);
    if (!dlg.exec())
      return;

    m_file->removeMajorCategory(item->text(0));
    m_file->addCategory(category.isIncome(), category.name(), category.minorCategories());
    categoryListView->clear();
    refresh();
  }
}

void KCategoriesView::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_lastCat = config->readEntry("KCategoriesView_LastCategory");
}

void KCategoriesView::writeConfig(void)
{
  KCategoryListItem *item = (KCategoryListItem *)categoryListView->selectedItem();
  if (!item || !item->isMajor())
    return;

  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KCategoriesView_LastCategory", item->text(0));
  config->sync();
}
