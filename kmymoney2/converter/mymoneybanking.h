/***************************************************************************
                          mymoneybanking.h
                             -------------------
    begin                : Thu Aug 26 2004
    copyright            : (C) 2004 Martin Preuss
    email                : aquamaniac@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KMYMONEYBANKING_H
#define KMYMONEYBANKING_H

#include <kbanking/kbanking.h>

class KMyMoneyBanking: public KBanking {
public:
  KMyMoneyBanking(const char *appname,
		  const char *fname=0);
  virtual ~KMyMoneyBanking();

  virtual bool importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai);

private:
  const AB_ACCOUNT_STATUS *_getAccountStatus(AB_IMEXPORTER_ACCOUNTINFO *ai);

};



#endif
