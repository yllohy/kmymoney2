/***************************************************************************
                          knewcategorydlg.cpp
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
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <qpixmap.h>

#include <qlineedit.h>
#include <kcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "knewcategorydlg.h"

KNewCategoryDlg::KNewCategoryDlg(MyMoneyCategory* category, QWidget *parent, const char *name)
 : KNewCategoryDlgDecl(parent,name,true)
{
  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_edit_category.png");
  QPixmap *pm = new QPixmap(filename);
  m_qpixmaplabel->setPixmap(*pm);

	m_category = category;

	categoryNameEdit->setText(m_category->name());
	(m_category->isIncome()) ? typeCombo->setCurrentItem(0) : typeCombo->setCurrentItem(1);
	minorCategoriesCombo->insertStringList(m_category->minorCategories());
	
	connect(categoryNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(categoryNameChanged(const QString&)));
	connect(typeCombo, SIGNAL(highlighted(const QString&)), this, SLOT(typeChanged(const QString&)));
	connect(minorCategoryEdit, SIGNAL(textChanged(const QString&)), this, SLOT(minorEditChanged(const QString&)));
	connect(minorAddBtn, SIGNAL(clicked()), this, SLOT(minorAddBtnClicked()));
	connect(minorDeleteBtn, SIGNAL(clicked()), this, SLOT(minorDeleteBtnClicked()));
	connect(minorCategoriesCombo, SIGNAL(activated(const QString&)), this, SLOT(slotMinorActivated(const QString&)));
	
	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
	connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
}

KNewCategoryDlg::~KNewCategoryDlg()
{
}

void KNewCategoryDlg::categoryNameChanged(const QString& text)
{
  m_category->setName(text);
}

void KNewCategoryDlg::typeChanged(const QString& text)
{
  (text==i18n("Income")) ? m_category->setIncome(true) : m_category->setIncome(false);
}

void KNewCategoryDlg::minorEditChanged(const QString& text)
{
  if (!text.isEmpty()) {
    minorAddBtn->setEnabled(true);
    minorDeleteBtn->setEnabled(true);
  }
  else {
    minorAddBtn->setEnabled(false);
    minorDeleteBtn->setEnabled(false);
  }
}

void KNewCategoryDlg::minorAddBtnClicked()
{
  m_category->addMinorCategory(minorCategoryEdit->text());
	minorCategoriesCombo->clear();
	minorCategoriesCombo->insertStringList(m_category->minorCategories());
	minorCategoryEdit->setText("");
}

void KNewCategoryDlg::minorDeleteBtnClicked()
{
  m_category->removeMinorCategory(minorCategoryEdit->text());
	minorCategoriesCombo->clear();
	minorCategoriesCombo->insertStringList(m_category->minorCategories());
	minorCategoryEdit->setText("");
}

void KNewCategoryDlg::slotMinorActivated(const QString &text)
{
  minorCategoryEdit->setText(text);
}
