/***************************************************************************
                          knewcategorydlg.h
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

#ifndef KNEWCATEGORYDLG_H
#define KNEWCATEGORYDLG_H

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
  void minorDeleteBtnClicked();
  void slotMinorActivated(const QString &text);

private:
  MyMoneyCategory* m_category;
};

#endif
