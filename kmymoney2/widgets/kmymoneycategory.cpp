/***************************************************************************
                          kmymoneycategory.cpp  -  description
                             -------------------
    begin                : Sun Aug 11 2002
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

#include "kmymoneycategory.h"

kMyMoneyCategory::kMyMoneyCategory(QWidget *parent, const char *name )
  : KLineEdit(parent,name)
{
  // make sure, the completion object exists
  if(compObj() == 0)
    completionObject();

  compObj()->setOrder(KCompletion::Sorted);
}

kMyMoneyCategory::~kMyMoneyCategory()
{
}
