/***************************************************************************
                          kscheduleview.h
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

#ifndef KSCHEDULEVIEW_H
#define KSCHEDULEVIEW_H

#include <klocale.h>

#include <qframe.h>
#include <qpushbutton.h>
//#include <qdbt/qdbttabular.h>
//#include <qdbt/qdbtsection.h>
//#include <qdbt/qdbttabcell.h>

#include "mymoney/mymoneyfile.h"
#include "kscheduleviewdecl.h"

class KScheduleView : public KScheduleViewDecl  {
   Q_OBJECT
public: 
	KScheduleView(QWidget *parent=0, const char *name=0);
	~KScheduleView();
  void refresh(MyMoneyFile *file);

protected slots:
  void newBtnClicked();

private:
  QColor m_defaultBGColor;
  QColor m_defaultListColor;
  unsigned int m_lastCount;
  unsigned int m_insertCount;
};

#endif
