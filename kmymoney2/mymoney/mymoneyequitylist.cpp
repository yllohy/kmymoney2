/***************************************************************************
                          mymoneyequitylist.cpp  -  description
                             -------------------
    begin                : Tue Jan 29 2002
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

#include "mymoneyutils.h"
#include "mymoneyequity.h"
#include "mymoneyequitylist.h"

MyMoneyEquityList::MyMoneyEquityList()
{
}

MyMoneyEquityList::~MyMoneyEquityList()
{
	while(m_equityList.begin() != m_equityList.end())
	{
		EquityList::iterator i = m_equityList.begin();
		if(*i)
		{
			delete *i;
		}
		m_equityList.erase(i);
	}
}

/** Adds a new equity entry into the master list. */
bool MyMoneyEquityList::addEquity(MyMoneyEquity* pEquity)
{
	if(!pEquity)
	{
		return false;
	}

	m_equityList.push_back(pEquity);
	return true;
}

/** Removes an entry from the list. */
bool MyMoneyEquityList::removeEquity(const MyMoneyEquity* pEquity)
{
	if(!pEquity)
	{
		return false;
	}
	
	EquityList::iterator i = doesItemExist(pEquity);//m_equityList.find(pEquity);
	if(i != m_equityList.end())
	{
		delete (*i);
		m_equityList.erase(i);
	}
	else
	{
		return false;
	}
	
	return true;
}

/** Returns an iterator if the item exist. */
EquityList::iterator MyMoneyEquityList::doesItemExist(const MyMoneyEquity* pEquity)
{
	for(vector<MyMoneyEquity*>::iterator i = m_equityList.begin(); i != m_equityList.end(); ++i)
	{
		if(pEquity == *i)
		{
			return i;
		}
	}
	
	return m_equityList.end();
}
