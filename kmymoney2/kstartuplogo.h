/***************************************************************************
                          kstartuplogo.h
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

#ifndef KSTARTUPLOGO_H
#define KSTARTUPLOGO_H

#include <qwidget.h>
#include <qframe.h>
#include <kapplication.h>

// Simple class that just shows a picture
class KStartupLogo : public QFrame  {
   Q_OBJECT
public: 
	KStartupLogo(QWidget *parent=0, const char *name=0);
	~KStartupLogo();
private slots: // Private slots
  /** Time 0.5 second */
  void timerDone();
};

#endif
