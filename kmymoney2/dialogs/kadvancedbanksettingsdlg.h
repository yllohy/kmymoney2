/***************************************************************************
                          kadvancedbanksettingsdlg.h
                             -------------------
    copyright            : (C) 2004 by Ace Jones
    email                : acejones@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KADVANCEDBANKSETTINGSDLG_H
#define KADVANCEDBANKSETTINGSDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kadvancedbanksettingsdlgdecl.h"

class MyMoneyKeyValueContainer;

/**
  @author Ace Jones
*/
class KAdvancedBankSettingsDlg : public KAdvancedBankSettingsDlgDecl
{
Q_OBJECT
public:
  KAdvancedBankSettingsDlg(QWidget *parent = 0, const char *name = 0);
  ~KAdvancedBankSettingsDlg();
    
  void setValues(const MyMoneyKeyValueContainer& _values);
  MyMoneyKeyValueContainer values(void) const;
    
public slots:
  void slotToggleEnabled(bool);
  void slotHelp();
};

#endif
