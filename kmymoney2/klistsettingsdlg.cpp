/***************************************************************************
                          klistsettingsdlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kfontdialog.h>
#include <qcheckbox.h>
#include <kcolorbutton.h>

#include "klistsettingsdlg.h"
#include "kmymoneysettings.h"

KListSettingsDlg::KListSettingsDlg(QWidget *parent, const char *name)
 : KListSettingsDlgDecl(parent,name,true)
{
//	initDialog();
	
  KMyMoneySettings *p_settings = KMyMoneySettings::singleton();

  listColourBtn->setColor(p_settings->lists_color());
	listBGColourBtn->setColor(p_settings->lists_BGColor());
	
	headerFontBtn->setFont(p_settings->lists_headerFont());
	cellFontBtn->setFont(p_settings->lists_cellFont());
	
	connect(headerFontBtn, SIGNAL(clicked()), this, SLOT(headerBtnClicked()));
	connect(cellFontBtn, SIGNAL(clicked()), this, SLOT(cellBtnClicked()));
	
	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
	connect(okBtn, SIGNAL(clicked()), this, SLOT(okClicked()));
}

KListSettingsDlg::~KListSettingsDlg()
{
}

void KListSettingsDlg::okClicked()
{
	m_listBGColor = listBGColourBtn->color();
	m_listColor = listColourBtn->color();
	m_listHeaderFont = headerFontBtn->font();
	m_listCellFont = cellFontBtn->font();
	accept();
}

void KListSettingsDlg::headerBtnClicked()
{
  QFont myFont;
  if (KFontDialog::getFont( myFont )) {
    m_listHeaderFont = myFont;
    headerFontBtn->setFont(myFont);
  }
}

void KListSettingsDlg::cellBtnClicked()
{
  QFont myFont;
  if (KFontDialog::getFont( myFont )) {
    m_listCellFont = myFont;
    cellFontBtn->setFont(myFont);
  }
}
