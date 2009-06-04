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

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qguardedptr.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <ksplashscreen.h>

// ----------------------------------------------------------------------------
// Project Includes

class KStartupSplash : public KSplashScreen
{
  Q_OBJECT
  public:
    KStartupSplash(const QPixmap &pixmap, WFlags f = 0);
    ~KStartupSplash();
    void message( const QString &message, int alignment = AlignLeft, const QColor &color = black);

  protected:
    void drawContents(QPainter *p);

  private:
    class Private;
    Private *d;
};

// Simple class that just shows a picture
class KStartupLogo : public QObject
{
  Q_OBJECT
public:
  KStartupLogo();
  ~KStartupLogo();

private:
  QGuardedPtr<QWidget>  m_splash;
};

#endif
