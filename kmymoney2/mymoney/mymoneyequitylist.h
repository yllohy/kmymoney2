/***************************************************************************
                          mymoneyequitylist.h  -  description
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

#ifndef MYMONEYEQUITYLIST_H
#define MYMONEYEQUITYLIST_H


/**
	* Class that holds the database of Equity that the user has entered, as well as
	* equities read from a persisted configuration file.
	*
  *@author Kevin Tambascio
  */

class MyMoneyEquity;

typedef vector<MyMoneyEquity*> EquityList;

class MyMoneyEquityList {
public: 
	MyMoneyEquityList();
	~MyMoneyEquityList();

  /** Removes an entry from the list. */
  bool removeEquity(const MyMoneyEquity* pEquity);

	/** Adds a new equity entry into the master list. */
  bool addEquity(MyMoneyEquity* pEquity);

  /** Returns an iterator if the item exist. */
  EquityList::iterator doesItemExist(const MyMoneyEquity* pEquity);

private:
	EquityList m_equityList;
};

#endif
