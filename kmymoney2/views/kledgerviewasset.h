/***************************************************************************
                          kledgerviewasset.h  -  description
                             -------------------
    begin                : Mon Dec 2 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

#ifndef KLEDGERVIEWASSET_H
#define KLEDGERVIEWASSET_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerviewcheckings.h"

/**
  * @author Thomas Baumgart
  */

class KLedgerViewAsset : public KLedgerViewCheckings
{
  Q_OBJECT

public: 
  KLedgerViewAsset(QWidget *parent=0, const char *name=0);
  ~KLedgerViewAsset();

protected slots:

  /* documented in base class */
  virtual void slotReconciliation(void);

};

#endif
