/***************************************************************************
                          kpayeesview.h  -  description
                             -------------------
    begin                : Thu Jan 24 2002
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

#ifndef KPAYEESVIEW_H
#define KPAYEESVIEW_H

#include <qwidget.h>
#include "kpayeesviewdecl.h"

#include "../mymoney/mymoneyfile.h"

/**
  *@author Michael Edwardes
  */

class KPayeesView : public kPayeesViewDecl  {
   Q_OBJECT
public: 
	KPayeesView(MyMoneyFile *file, QWidget *parent=0, const char *name=0);
	~KPayeesView();
  void show();

protected slots:
  void payeeHighlighted(const QString&);
  void slotAddClicked();
  void slotPayeeTextChanged(const QString& text);
  void slotUpdateClicked();
  void slotDeleteClicked();

private:
  MyMoneyFile *m_file;
  QString m_lastPayee;

  void readConfig(void);
  void writeConfig(void);

  void refresh(void);

signals:
  void signalViewActivated();

};

#endif
