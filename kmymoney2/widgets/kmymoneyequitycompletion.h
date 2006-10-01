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

#ifndef KMYMONEYEQUITYCOMPLETION_H
#define KMYMONEYEQUITYCOMPLETION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyequityselector.h"
#include "../widgets/kmymoneycompletion.h"

/**
  * @author Thomas Baumgart
  */

class kMyMoneyEquityCompletion : public kMyMoneyCompletion
{
  Q_OBJECT
public:

  kMyMoneyEquityCompletion(QWidget *parent=0, const char *name=0);
  virtual ~kMyMoneyEquityCompletion();

  /**
    * Re-implemented for internal reasons.  API is unaffected.
    */
  virtual void show();

  const int loadList(void) { return m_equitySelector->loadList(); }

  /**
    * This method sets the current account with id @p id as
    * the current selection.
    *
    * @param id id of account to be selected
    */
  void setSelected(const QCString& id) { m_id = id; }

  void equityList(QCStringList& list) const { return m_equitySelector->itemList(list); }

  /**
    * This method returns the list of selected equity id's. If
    * no equity is selected, the list is empty.
    *
    * @return list of selected equity ids
    */
  void selectedEquities(QCStringList& list) const { return m_equitySelector->selectedItems(list); }

public slots:
  void slotMakeCompletion(const QString& txt);

private:
  kMyMoneyEquitySelector*  m_equitySelector;
};

#endif
