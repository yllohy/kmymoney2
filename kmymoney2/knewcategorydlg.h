/***************************************************************************
                          knewcategorydlg.h
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

#ifndef KNEWCATEGORYDLG_H
#define KNEWCATEGORYDLG_H
/*
//Generated area. DO NOT EDIT!!!(begin)
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
//Generated area. DO NOT EDIT!!!(end)
*/
#include <klocale.h>
#include <qdialog.h>

#include "mymoney/mymoneycategory.h"

#include "knewcategorydlgdecl.h"

// This dialog lets the user edit or create
// a category
class KNewCategoryDlg : public KNewCategoryDlgDecl  {
   Q_OBJECT
public: 
	KNewCategoryDlg(MyMoneyCategory* category, QWidget *parent=0, const char *name=0);
	~KNewCategoryDlg();

protected slots:
  void categoryNameChanged(const QString& text);
  void typeChanged(const QString& text);
  void minorEditChanged(const QString& text);
  void minorAddBtnClicked();

protected:
/*
	void initDialog();
	//Generated area. DO NOT EDIT!!!(begin)
	QLabel *titleLabel;
	QLabel *catNameLabel;
	QLineEdit *categoryNameEdit;
	QComboBox *typeCombo;
	QLabel *typeLabel;
	QGroupBox *minorGroupBox;
	QComboBox *minorCategoriesCombo;
	QLineEdit *minorCategoryEdit;
	QPushButton *minorAddBtn;
	QPushButton *okBtn;
	QPushButton *cancelBtn;
	//Generated area. DO NOT EDIT!!!(end)
*/
private:
  MyMoneyCategory* m_category; // Why does gcc-2.95.3 complain when I use a reference to a reference.
};

#endif
