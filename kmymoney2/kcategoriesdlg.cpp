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
	categoryListView->setColumnWidthMode(0, QListView::Maximum);
	
	// resize the columns in the list view to accomodate all space
	// there must be a better way of doing this !
	QFontMetrics met(categoryListView->fontMetrics());
	int w = (met.boundingRect("Expense").width());
	//categoryListView->setColumnWidth(1, w+5);
	//categoryListView->setColumnWidth(0, categoryListView->width()-5-(categoryListView->columnWidth(1)));

  refresh();

	connect(categoryListView, SIGNAL(executed(QListViewItem*)), this, SLOT(categoryExecuted(QListViewItem*)));
	connect(buttonNew, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
  connect(buttonDelete, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));
//  connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
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

void KCategoriesDlg::categoryExecuted(QListViewItem *item)
{
  // Might change this slot to be called only when an item selected
  KCategoryListItem *kitem = (KCategoryListItem *)item;
  if (!kitem)
    return;

  if (!kitem->major())
    return;

  MyMoneyCategory category(kitem->income(), kitem->text(0), kitem->minors());

  KNewCategoryDlg dlg(&category, this);
  if (!dlg.exec())
    return;

  m_file->addCategory(category.isIncome(), category.name(), category.minorCategories());
  categoryListView->clear();
  refresh();
}
