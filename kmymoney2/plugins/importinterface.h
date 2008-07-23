/***************************************************************************
                          importinterface.h
                             -------------------
    begin                : Mon Apr 14 2008
    copyright            : (C) 2008 Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IMPORTINTERFACE_H
#define IMPORTINTERFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qobject.h>
#include <qstring.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kfile.h>
#include <kurl.h>
class KPopupMenu;

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/export.h>

namespace KMyMoneyPlugin {

/**
  * This abstract class represents the ImportInterface to
  * add new importers to KMyMoney.
  */
class KMYMONEY_EXPORT ImportInterface : public QObject
{
  Q_OBJECT

public:
  ImportInterface(QObject* parent, const char* name = 0);
  ~ImportInterface() {}

  virtual KURL selectFile(const QString& title, const QString& path, const QString& mask, KFile::Mode mode) const = 0;

signals:
};

}; // namespace
#endif
