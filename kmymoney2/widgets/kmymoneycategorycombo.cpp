/***************************************************************************
                          kmymoneycategorycombo.cpp  -  description
                             -------------------
    begin                : Wed May 9 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneycategorycombo.h"

kMyMoneyCategoryCombo::kMyMoneyCategoryCombo():KComboBox(){
}
kMyMoneyCategoryCombo::~kMyMoneyCategoryCombo(){
}
/** No descriptions */
bool kMyMoneyCategoryCombo::eventFilter( QObject *o , QEvent *e ){

	if(e->type() == QEvent::KeyPress)
	{
		QKeyEvent *k = (QKeyEvent *) e;
    if((k->key() == Qt::Key_Return) ||
       (k->key() == Qt::Key_Enter))
    {
    	emit signalEnter();
		}
	}
	return KComboBox::eventFilter(o,e);

}
