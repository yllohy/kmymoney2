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
#include <qpainter.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kstartuplogo.h"
#include "kmymoneyglobalsettings.h"

class KStartupSplash::Private
{
  public:
    QString message;
    QColor color;
    int align;
};

KStartupSplash::KStartupSplash(const QPixmap &pixmap, WFlags f) :
  KSplashScreen(pixmap, f),
  d(new Private)
{
}

KStartupSplash::~KStartupSplash()
{
  delete d;
}

void KStartupSplash::message( const QString &message, int alignment, const QColor &color)
{
  d->message = message;
  d->align = alignment;
  d->color = color;
  // the next line causes the base class signal management to happen
  // and also forces a repaint
  KSplashScreen::clear();
}

void KStartupSplash::drawContents( QPainter *painter )
{
  painter->setPen( d->color );
  QRect r = rect();
  r.setRect( r.x() + 15, r.y() + r.height() - 28, r.width() - 20, 20 );
  painter->drawText( r, d->align, d->message);
}

KStartupLogo::KStartupLogo() :
  QObject(0, 0),
  m_splash(0)
{
  // splash screen setting
  if(!KMyMoneyGlobalSettings::showSplash())
    return;

  QString filename = KGlobal::dirs()->findResource("appdata", "pics/startlogo.png");
  QPixmap splashPixmap(filename);

  if(!splashPixmap.isNull()) {
    QPixmap backGround(splashPixmap);
    backGround.fill(KGlobalSettings::highlightColor());
    bitBlt ( &backGround, 0, 0, &splashPixmap, 0, 0, splashPixmap.width(), splashPixmap.height(), Qt::CopyROP );

    KStartupSplash* splash = new KStartupSplash(backGround);
    splash->setFixedSize(backGround.size());

    // FIXME: I added the 'Loading file...' message here, because this was the only
    // existing string we have and I did not want to change the strings. We should
    // change that in the future.
    splash->message(i18n("Loading..."), AlignLeft, white);

    splash->show();
    splash->repaint();
    m_splash = splash;
  }
}

KStartupLogo::~KStartupLogo()
{
    delete m_splash;
}

#include "kstartuplogo.moc"
