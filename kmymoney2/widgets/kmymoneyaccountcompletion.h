/***************************************************************************
                          kmymoneyaccountcompletion.h  -  description
                             -------------------
    begin                : Mon Apr 26 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#ifndef KMYMONEYACCOUNTCOMPLETION_H
#define KMYMONEYACCOUNTCOMPLETION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
class QListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneyaccountselector.h>
#include "kmymoneycompletion.h"

/**
  * @author Thomas Baumgart
  */
class kMyMoneyAccountCompletion : public kMyMoneyCompletion
{
  Q_OBJECT
public:

  kMyMoneyAccountCompletion(QWidget *parent=0, const char *name=0);
  virtual ~kMyMoneyAccountCompletion();

  QStringList accountList(const QValueList<MyMoneyAccount::accountTypeE>& list = QValueList<MyMoneyAccount::accountTypeE>()) const { return selector()->accountList(list); }

  /**
    * reimplemented from kMyMoneyCompletion
    */
  kMyMoneyAccountSelector* selector(void) const { return dynamic_cast<kMyMoneyAccountSelector*>(m_selector); }

public slots:
  void slotMakeCompletion(const QString& txt);
};

#endif
