/***************************************************************************
                          kcategoriesdlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kmessagebox.h>
#include <qpushbutton.h>
#include <qheader.h>

#include "kcategoriesdlg.h"
#include "kcategorylistitem.h"
#include "knewcategorydlg.h"

KCategoriesDlg::KCategoriesDlg(MyMoneyFile *file, QWidget *parent, const char *name)
 : KCategoryDlgDecl(parent,name,true)
{
	m_file = file;
	
	categoryListView->setRootIsDecorated(true);
	categoryListView->addColumn(i18n("Category"));
	categoryListView->addColumn(i18n("Type"));
	categoryListView->setMultiSelection(false);
	categoryListView->setColumnWidthMode(0, QListView::Manual);
	categoryListView->header()->setResizeEnabled(false);
	
  refresh();

	connect(categoryListView, SIGNAL(selectionChanged(QListViewItem*)),
	  this, SLOT(slotSelectionChanged(QListViewItem*)));
	connect(buttonEdit, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
	connect(buttonNew, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
  connect(buttonDelete, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));
}

KCategoriesDlg::~KCategoriesDlg(){
}

void KCategoriesDlg::refresh(void)
{
  QListIterator<MyMoneyCategory> it = m_file->categoryIterator();
  for ( ; it.current(); ++it ) {
    MyMoneyCategory *data = it.current();
    // Construct a new list item using appropriate arguments.
    // See KCategoryListItem
    KCategoryListItem *item0 = new KCategoryListItem(categoryListView, data->name(), data->minorCategories(), data->isIncome(), true);
    for ( QStringList::Iterator it2 = data->minorCategories().begin(); it2 != data->minorCategories().end(); ++it2 ) {
      (void) new KCategoryListItem(item0, (*it2).latin1(), data->isIncome(), false, item0->text(0));
    }
  }
}

void KCategoriesDlg::slotNewClicked()
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

void KCategoriesDlg::slotDeleteClicked()
{
  KCategoryListItem *item = (KCategoryListItem *)categoryListView->selectedItem();
  if (!item)
    return;

  QString prompt;
  if (item->major()) {
    prompt = i18n("By deleting a major category all minor(s) will be lost.\nAre you sure you want to delete ");
    prompt += item->text(0);
  } else {
    prompt = i18n("Delete this minor category item: ");
    prompt += item->text(0);
  }

  if ((KMessageBox::questionYesNo(this, prompt))==KMessageBox::Yes) {
    if (item->major())
      m_file->removeMajorCategory(item->text(0));
    else
      m_file->removeMinorCategory(item->majorName(), item->text(0));
  }
  categoryListView->clear();
  refresh();
}

void KCategoriesDlg::resizeEvent(QResizeEvent*)
{
	categoryListView->setColumnWidth(0, categoryListView->width()-105);
  categoryListView->setColumnWidth(1, 100);
}

void KCategoriesDlg::slotSelectionChanged(QListViewItem* item)
{
  KCategoryListItem *kitem = (KCategoryListItem *)item;
  if (!kitem) {
    buttonEdit->setEnabled(false);
    buttonDelete->setEnabled(false);
  }
  else if (kitem->major()) {
    buttonEdit->setEnabled(true);
    buttonDelete->setEnabled(true);
  } else {
    buttonEdit->setEnabled(false);
    buttonDelete->setEnabled(true);
  }
}

void KCategoriesDlg::slotEditClicked()
{
  KCategoryListItem *item = (KCategoryListItem *)categoryListView->selectedItem();
  if (!item)
    return;

  QString prompt;
  if (item->major()) {
    MyMoneyCategory category(item->income(), item->text(0), item->minors());

    KNewCategoryDlg dlg(&category, this);
    if (!dlg.exec())
      return;

    m_file->addCategory(category.isIncome(), category.name(), category.minorCategories());
    categoryListView->clear();
    refresh();
  }
}
