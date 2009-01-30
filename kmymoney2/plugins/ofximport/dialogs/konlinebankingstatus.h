/***************************************************************************
                          konlinebankingstatus.h
                             -------------------
    begin                : Wed Apr 16 2008
    copyright            : (C) 2008 by Thomas Baumgart
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

#ifndef KONLINEBANKINGSTATUS_H
#define KONLINEBANKINGSTATUS_H

// ----------------------------------------------------------------------------
// Library Includes

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "konlinebankingstatusdecl.h"
class MyMoneyAccount;
class OfxAppVersion;
class OfxHeaderVersion;

/**
  * @author Thomas Baumgart
  */

class KOnlineBankingStatus : public KOnlineBankingStatusDecl
{
  Q_OBJECT
public:
  KOnlineBankingStatus(const MyMoneyAccount& acc, QWidget *parent=0, const char *name=0);
  ~KOnlineBankingStatus();
  const QString& appId(void) const;
  QString headerVersion(void) const;
private:
  OfxAppVersion*  m_appId;
  OfxHeaderVersion* m_headerVersion;
};

#endif
// vim:cin:si:ai:et:ts=2:sw=2:
