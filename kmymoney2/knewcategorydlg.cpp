/***************************************************************************
                          knewcategorydlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qlineedit.h>
#include <kcombobox.h>
#include <qpushbutton.h>

#include "knewcategorydlg.h"

KNewCategoryDlg::KNewCategoryDlg(MyMoneyCategory* category, QWidget *parent, const char *name)
 : KNewCategoryDlgDecl(parent,name,true)
{
//	initDialog();
	
	m_category = category;
//  titleLabel->setFont(QFont("Helvetica", 18, QFont::Bold));
	
	categoryNameEdit->setText(m_category->name());
	(m_category->isIncome()) ? typeCombo->setCurrentItem(0) : typeCombo->setCurrentItem(1);
	minorCategoriesCombo->insertStringList(m_category->minorCategories());
	
	minorAddBtn->setEnabled(false);
	
	connect(categoryNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(categoryNameChanged(const QString&)));
	connect(typeCombo, SIGNAL(highlighted(const QString&)), this, SLOT(typeChanged(const QString&)));
	connect(minorCategoryEdit, SIGNAL(textChanged(const QString&)), this, SLOT(minorEditChanged(const QString&)));
	connect(minorAddBtn, SIGNAL(clicked()), this, SLOT(minorAddBtnClicked()));
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
  (text=="Income") ? m_category->setIncome(true) : m_category->setIncome(false);
}

void KNewCategoryDlg::minorEditChanged(const QString& text)
{
  if (!text.isEmpty())
    minorAddBtn->setEnabled(true);
  else
    minorAddBtn->setEnabled(false);
}

void KNewCategoryDlg::minorAddBtnClicked()
{
  m_category->addMinorCategory(minorCategoryEdit->text());
	minorCategoriesCombo->clear();
	minorCategoriesCombo->insertStringList(m_category->minorCategories());
	minorCategoryEdit->setText("");
}

//#include "knewcategorydlg.moc"
