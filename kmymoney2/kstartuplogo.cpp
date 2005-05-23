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

#include <kdecompat.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qapplication.h>
#include <qpixmap.h>
#include <qframe.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kconfig.h>

#if KDE_IS_VERSION(3,2,0)
#include <ksplashscreen.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kstartuplogo.h"

KStartupLogo::KStartupLogo() :
  QObject(0, 0),
  m_splash(0)
{
  KConfig* config = kapp->config();
  config->setGroup("General Options");

  // splash screen setting
  if(config->readBoolEntry("Show Splash", true) == false)
    return;

  QString filename = KGlobal::dirs()->findResource("appdata", "pics/startlogo.png");
  QPixmap pm(filename);

#if KDE_IS_VERSION(3,2,0)
  KSplashScreen* splash = new KSplashScreen(pm);
  splash->setFixedSize(pm.size());

#else
  QFrame* splash = new QFrame(0, 0, QFrame::WStyle_NoBorder | QFrame::WStyle_StaysOnTop | QFrame::WStyle_Tool | QFrame::WWinOwnDC | QFrame::WStyle_Customize);
  splash->setBackgroundPixmap(pm);
  splash->setFrameShape( QFrame::StyledPanel );
  splash->setFrameShadow( QFrame::Raised );
  splash->setLineWidth( 2 );
  splash->setGeometry( QRect( (QApplication::desktop()->width()/2)-(pm.width()/2), (QApplication::desktop()->height()/2)-(pm.height()/2), pm.width(), pm.height() ) );

#endif

  splash->show();
  m_splash = splash;
}

KStartupLogo::~KStartupLogo()
{
    delete m_splash;
}

#include "kstartuplogo.moc"
