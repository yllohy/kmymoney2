/***************************************************************************
                          mymoneysubject.cpp  -  description
                             -------------------
    begin                : Sat May 18 2002
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

#include "mymoneysubject.h"
#include "mymoneyobserver.h"
#include <qptrvector.h>

MyMoneySubject::MyMoneySubject()
{
}

MyMoneySubject::~MyMoneySubject()
{
}

void MyMoneySubject::attach (MyMoneyObserver* o)
{
  _observers.append(o);
}

void MyMoneySubject::detach (MyMoneyObserver* o)
{
  _observers.remove(o);
}

void MyMoneySubject::notify(const QCString& id) const
{
  QPtrList<MyMoneyObserver> ptrList = _observers;
  MyMoneyObserver* i;

  for (i = ptrList.first(); i != NULL; i = ptrList.next()) {
    i->update(id);
  }
}

