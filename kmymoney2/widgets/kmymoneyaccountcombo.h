/***************************************************************************
                         kmymoneyaccountbutton  -  description
                            -------------------
   begin                : Mon May 31 2004
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

#ifndef KMYMONEYACCOUNTBUTTON_H
#define KMYMONEYACCOUNTBUTTON_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyutils.h"
#include "kmymoneyaccountcompletion.h"

/**
  * @author Thomas Baumgart
  */
class kMyMoneyAccountCombo : public KPushButton
{
  Q_OBJECT
public:
  kMyMoneyAccountCombo( QWidget* parent = 0, const char* name = 0 );
  ~kMyMoneyAccountCombo();

  /**
    * Method returns how many items are in the account selector list.
    */
  int count(void) const { return m_selector->accountList().count(); };

  const QCStringList accountList(void) const { return m_selector->accountList(); };

  int loadList(KMyMoneyUtils::categoryTypeE typeMask) { return m_selector->loadList(typeMask); };

  void setSelected(const MyMoneyAccount& acc);

public slots:
  void slotButtonPressed(void);

signals:
  void accountSelected(const QCString&);

protected:
  void paintEvent( QPaintEvent * );

private:
  kMyMoneyAccountCompletion*    m_selector;
};

#endif
