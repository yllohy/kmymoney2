/***************************************************************************
                          kmymoneysettings.h
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

#ifndef KMYMONEYSETTINGS_H
#define KMYMONEYSETTINGS_H

#include <qcolor.h>
#include <qfont.h>

/**
  *@author Michael Edwardes
  */

class KMyMoneySettings {
private:
  static KMyMoneySettings *p_self;

  // List view settings (all three lists)
	QColor m_listColor;
	QColor m_listBGColor;
	QFont m_listHeaderFont;
	QFont m_listCellFont;
	
	KMyMoneySettings(); // Private

public:
  static KMyMoneySettings *singleton(); // Use this to create a KMyMoneySettings
	~KMyMoneySettings();
  // List stuff
  void setListSettings(
      	const QColor listColor,
      	const QColor listBGColor,
      	const QFont listHeaderFont,
      	const QFont listCellFont );

	QColor lists_color(void) { return m_listColor; }
	QColor lists_BGColor(void) { return m_listBGColor; }
	QFont lists_headerFont(void) { return m_listHeaderFont; }
	QFont lists_cellFont(void) { return m_listCellFont; }
};

#endif
