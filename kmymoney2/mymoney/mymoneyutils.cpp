/***************************************************************************
                          mymoneyutils.cpp  -  description
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

///////////////////////////////////////////////////////////////////////////////////////////////
/**
*	Adds the file extension to the end of the file name.
*
*	@return		bool
*						- true if name was changed
*						- false if it wasn't.
*
*	@todo			This function should be moved to a separate file, or utility file somewhere
*						in the library files, because it appears in numerous places.
*/
///////////////////////////////////////////////////////////////////////////////////////////////
bool MyMoneyUtils::appendCorrectFileExt(QString& str, const QString strExtToUse)
{
	/*if(!str.isEmpty())
  {
		//find last . delminator
		int nLoc = str.findRev('.');
    if(nLoc != -1)
		{
			QString strExt, strTemp;
      strTemp = str.left(nLoc + 1);
			strExt = str.right(str.length() - (nLoc + 1));
			if(strExt.find(strExtToUse, 0, FALSE) == -1)
			{
				//append to make complete file name
				strTemp.append(strExtToUse);
				str = strTemp;
			}
			else
			{
				return false;
			}
		}
		else
		{
			str.append(".");
			str.append(strExtToUse);
		}
	}
	else
	{
		return false;
	}
  */
	return true;
}

/*QString MyMoneyUtils::StringFromQString(const QString& str)
{
	QString strRetval(str);
	strRetval[str.length()] = '\0';
	return strRetval;	
}

QString MyMoneyUtils::QStringFromString(const QString& str)
{
	QString strRetval(str.data());
	return strRetval;
}  */
