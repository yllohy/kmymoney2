/***************************************************************************
                          kcategoriesview.h  -  description
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

#ifndef KCATEGORIESVIEW_H
#define KCATEGORIESVIEW_H

#include <qwidget.h>
#include "kcategoriesviewdecl.h"

#include "../mymoney/mymoneyfile.h"

/**
  *@author Michael Edwardes
  */

class KCategoriesView : public kCategoriesViewDecl  {
   Q_OBJECT
private:
  MyMoneyFile *m_file;
  void refresh(void);

protected:
  void resizeEvent(QResizeEvent*);

public:
	KCategoriesView(MyMoneyFile *file, QWidget *parent=0, const char *name=0);
	~KCategoriesView();
  void show();
};

#endif
