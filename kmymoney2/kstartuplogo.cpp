/***************************************************************************
                          kstartuplogo.cpp
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

#include "kstartuplogo.h"

#include <kglobal.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <qpixmap.h>
#include <qtimer.h>

KStartupLogo::KStartupLogo(QWidget *parent, const char *name )
  : QFrame(parent, name, WStyle_NoBorder | WStyle_Customize)
{ 	
  QString filename = KGlobal::dirs()->findResource("appdata", "pics/startlogo.png");
  QPixmap *pm = new QPixmap(filename);
  setBackgroundPixmap(*pm);
  setFrameShape( QFrame::StyledPanel );
  setFrameShadow( QFrame::Raised );
	setLineWidth( 2 );
  setGeometry( QRect( (QApplication::desktop()->width()/2)-(pm->width()/2), (QApplication::desktop()->height()/2)-(pm->height()/2), pm->width(), pm->height() ) );
	QTimer *timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), this, SLOT(timerDone()) );
	timer->start( 500, TRUE );
}

KStartupLogo::~KStartupLogo()
{
}

/** Timeout 0.5 second */
void KStartupLogo::timerDone()
{
	this->close();
}
