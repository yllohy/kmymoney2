/***************************************************************************
                          kcategoriesdlg.h
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

#ifndef KCATEGORIESDLG_H
#define KCATEGORIESDLG_H

#include <klocale.h>
#include <qdialog.h>
#include <klistview.h>

#include "../mymoney/mymoneyfile.h"
#include "kcategorydlgdecl.h"

// This class allows the user to view/edit the categories
// used by KMyMoney2.
// Uses KCategoryListItem to represent a list item.
class KCategoriesDlg : public KCategoryDlgDecl  {
   Q_OBJECT
public: 
	KCategoriesDlg(MyMoneyFile *file, QWidget *parent=0, const char *name=0);
	~KCategoriesDlg();
  void refresh(void);

protected:
  void resizeEvent(QResizeEvent*);

protected slots:
  void slotEditClicked();
  void slotNewClicked();
  void slotDeleteClicked();
  void slotSelectionChanged(QListViewItem*);

private:
	MyMoneyFile *m_file;
	QString m_lastCat;
	
  void readConfig(void);
  void writeConfig(void);
};

#endif
