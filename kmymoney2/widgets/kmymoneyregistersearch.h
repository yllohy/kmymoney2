/***************************************************************************
                          kmymoneyregistersearch.h  -  description
                             -------------------
    begin                : Sun Aug 10 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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

#ifndef KMYMONEYREGISTERSEARCH_H
#define KMYMONEYREGISTERSEARCH_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <../widgets/kmymoneyregister.h>

/**
  * @author Thomas Baumgart
  */

class kMyMoneyRegisterSearch : public kMyMoneyRegister  {
   Q_OBJECT
public: 
  kMyMoneyRegisterSearch(QWidget *parent=0, const char *name=0);
  ~kMyMoneyRegisterSearch();

  bool eventFilter(QObject* o, QEvent* e);

  virtual const int maxRpt(void) const { return 3; };

public slots:
  void adjustColumn(int col);

protected:
  void paintCell(QPainter *p, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);
};

#endif
