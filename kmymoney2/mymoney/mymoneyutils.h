/***************************************************************************
                          mymoneyutils.h  -  description
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

#ifndef _MYMONEYUTILS_H_
#define _MYMONEYUTILS_H_

#include <kglobal.h>
#include <klocale.h>

//Includes for STL support below
#include <vector>
#include <map>
#include <list>
#include <string>
using namespace std;

#ifdef _UNICODE
typedef std::wstring String;
#else
typedef std::string String;
#endif

//typedef for data type to store currency with.
typedef long long DLONG;

//class that has utility functions to use throughout the application.
class MyMoneyUtils
{
public:
	MyMoneyUtils() {};
	~MyMoneyUtils() {};
	
	//static function to add the correct file extension at the end of the file name
	static bool appendCorrectFileExt(String& str, const String strExtToUse);
};


#endif
