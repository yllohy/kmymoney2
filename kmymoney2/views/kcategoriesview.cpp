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
#include <qheader.h>
#include "kcategoriesview.h"
#include "../dialogs/kcategorylistitem.h"

KCategoriesView::KCategoriesView(MyMoneyFile *file, QWidget *parent, const char *name )
  : kCategoriesViewDecl(parent,name)
{
  m_file = file;
	categoryListView->setRootIsDecorated(true);
	categoryListView->addColumn(i18n("Category"));
	categoryListView->addColumn(i18n("Type"));
	categoryListView->setMultiSelection(false);
	categoryListView->setColumnWidthMode(0, QListView::Manual);
	categoryListView->header()->setResizeEnabled(false);

  // never show a horizontal scroll bar
  categoryListView->setHScrollBarMode(QScrollView::AlwaysOff);

  categoryListView->setSorting(-1);
}

KCategoriesView::~KCategoriesView()
{
}

void KCategoriesView::refresh(void)
{
  categoryListView->clear();

//  KCategoryListItem *saveptr=0;

  QListIterator<MyMoneyCategory> it = m_file->categoryIterator();
  for ( ; it.current(); ++it ) {
    MyMoneyCategory *data = it.current();
    // Construct a new list item using appropriate arguments.
    // See KCategoryListItem
    KCategoryListItem *item0 = new KCategoryListItem(categoryListView, data->name(), data->minorCategories(), data->isIncome(), true);
//    if (data->name()==m_lastCat)
//      saveptr = item0;
    for ( QStringList::Iterator it2 = data->minorCategories().begin(); it2 != data->minorCategories().end(); ++it2 ) {
      (void) new KCategoryListItem(item0, (*it2), data->isIncome(), false, item0->text(0));
    }
    categoryListView->setOpen(item0, true);
  }
//  if (saveptr)
//    categoryListView->setCurrentItem(saveptr);
}

void KCategoriesView::show()
{
  refresh();
  QWidget::show();
}

void KCategoriesView::resizeEvent(QResizeEvent* e)
{
  categoryListView->setColumnWidth(0, categoryListView->width()-105);
  categoryListView->setColumnWidth(1, 100);

  // call base class resizeEvent()
  kCategoriesViewDecl::resizeEvent(e);
}
