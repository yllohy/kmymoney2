/***************************************************************************
                          krecentfileitem.h  -  description
                             -------------------
    begin                : Wed Jul 30 2003
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
#ifndef KRECENTFILEITEM_H
#define KRECENTFILEITEM_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qwidget.h>
#include <qiconview.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kurl.h>

// ----------------------------------------------------------------------------
// Project Includes



/**
  *@author Michael Edwardes
  */

class KRecentFileItem : public QIconViewItem  {
public: 
  KRecentFileItem(const QString& url, QIconView* parent, const QString& text, const QPixmap& icon);
  ~KRecentFileItem();
  QString fileURL(void) const { return m_url; }

private:
  QString m_url;
};

#endif
