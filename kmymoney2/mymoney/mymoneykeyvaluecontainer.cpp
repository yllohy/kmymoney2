/***************************************************************************
                          mymoneykeyvaluecontainer.cpp
                             -------------------
    begin                : Sun Nov 10 2002
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

#include "mymoneykeyvaluecontainer.h"

MyMoneyKeyValueContainer::MyMoneyKeyValueContainer()
{
}

MyMoneyKeyValueContainer::~MyMoneyKeyValueContainer()
{
}

const QString MyMoneyKeyValueContainer::value(const QCString& key) const
{
  QMap<QCString, QString>::ConstIterator it;

  it = m_kvp.find(key);
  if(it != m_kvp.end())
    return (*it);
  return QString();
}

void MyMoneyKeyValueContainer::setValue(const QCString& key, const QString& value)
{
  m_kvp[key] = value;
}


void MyMoneyKeyValueContainer::setPairs(const QMap<QCString, QString>& list)
{
  m_kvp = list;
}

void MyMoneyKeyValueContainer::deletePair(const QCString& key)
{
  QMap<QCString, QString>::Iterator it;

  it = m_kvp.find(key);
  if(it != m_kvp.end())
    m_kvp.remove(it);
}

