/***************************************************************************
                          kmymoneyequityselector.h
                             -------------------
    begin                : Wed 02 June 2004
    copyright            : (C) 2004 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYEQUITYSELECTOR_H
#define KMYMONEYEQUITYSELECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneyselector.h>

/**
  * @author Thomas Baumgart
  */

class kMyMoneyEquitySelector : public KMyMoneySelector
{
  Q_OBJECT

public:
  kMyMoneyEquitySelector(QWidget *parent=0, const char *name=0, QWidget::WFlags flags = 0);
  virtual ~kMyMoneyEquitySelector();

  /**
    * This method loads the set of equities known to the engine into the widget.
    *
    * @return This method returns the number of equities loaded into the list
    */
  const int loadList(void);

};
#endif
