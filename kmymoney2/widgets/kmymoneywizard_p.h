/***************************************************************************
                             kmymoneywizard_p.h
                             -------------------
    copyright            : (C) 2006 by Thomas Baumagrt
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

#ifndef KMYMONEYWIZARD_P_H
#define KMYMONEYWIZARD_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qobject.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * @author Thomas Baumgart (C) 2006
  *
  * This class represents a helper object required
  * to be able to use Qt's signal/slot mechanism within
  * the KMyMoneyWizardPage object which cannot be
  * derived from QObject directly.
  */
class KMyMoneyWizardPagePrivate : public QObject
{
  Q_OBJECT
public:
  /**
    * Constructor
    */
  KMyMoneyWizardPagePrivate(QObject* parent, const char* name = 0);

  void emitCompleteStateChanged(void);

signals:
  void completeStateChanged(void);
};

#endif
