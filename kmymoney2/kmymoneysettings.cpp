/***************************************************************************
                          kmymoneysettings.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneysettings.h"

KMyMoneySettings *KMyMoneySettings::p_self = 0;

KMyMoneySettings *KMyMoneySettings::singleton()
{
  if (!p_self)
    p_self = new KMyMoneySettings;
  return p_self;
}

KMyMoneySettings::KMyMoneySettings()
{
	// Some defaults for the list views (same as for KMyMoney 0.2)
  m_listBGColor = QColor(255, 255, 255);
  m_listColor = QColor(168, 168, 168);
  m_listHeaderFont = QFont("helvetica",12,QFont::Bold);
  m_listCellFont = QFont("helvetica",10,QFont::Bold);
}

KMyMoneySettings::~KMyMoneySettings()
{
}

void KMyMoneySettings::setListSettings(
      	const QColor listColor,
      	const QColor listBGColor,
      	const QFont listHeaderFont,
      	const QFont listCellFont )
{
  m_listColor = listColor;
  m_listBGColor = listBGColor;
  m_listHeaderFont = listHeaderFont;
  m_listCellFont = listCellFont;
}
