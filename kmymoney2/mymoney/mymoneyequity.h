/***************************************************************************
                          mymoneyequity.h  -  description
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

#ifndef MYMONEYEQUITY_H
#define MYMONEYEQUITY_H


/**
	* Class that holds all the required information about an equity that the user
	* has entered information about.
	*
  *@author Kevin Tambascio
  */

#include "mymoneymoney.h"
#include "mymoneyutils.h"

class MyMoneyEquity
{
public: 
	MyMoneyEquity();
	~MyMoneyEquity();

public:
	typedef enum {
		ETYPE_NONE,
		ETYPE_STOCK,
		ETYPE_MUTUALFUND,
		ETYPE_BOND
	} eEQUITYTYPE;
	
	String 	getEquityName() 												{ return m_strEquityName; }
	void			setEquityName(const String& str)			{ m_strEquityName = str; }

	String 	getEquitySymbol() 											{ return m_strEquitySymbol; }
	void			setEquitySymbol(const String& str)		{ m_strEquitySymbol = str; }

	eEQUITYTYPE 	getEquityType() 													{ return m_equityType; }
	void					setEquityType(const eEQUITYTYPE& e)			{ m_equityType = e; }

	String 	getEquityMarket() 											{ return m_strEquityMarket; }
	void			setEquityMarket(const String& str)		{ m_strEquityMarket = str; }

	MyMoneyMoney		getCurrentPrice()								{ return m_CurrentPrice; }
	void						setCurrentPrice(const MyMoneyMoney *m)	{ m_CurrentPrice = *m; }
  /** No descriptions */
  void setEquityType(const String& str);
	
private:
	String m_strEquityName;
	String m_strEquitySymbol;
	String m_strEquityMarket;
	eEQUITYTYPE m_equityType;
	MyMoneyMoney m_CurrentPrice;
};

#endif
