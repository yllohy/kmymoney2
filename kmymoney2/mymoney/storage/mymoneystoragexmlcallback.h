/***************************************************************************
                          mymoneystoragexmlcollection.h  -  description
                             -------------------
    begin                : Thu Oct 24 2002
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

#ifndef MYMONEYSTORAGEXMLCALLBACK_H
#define MYMONEYSTORAGEXMLCALLBACK_H

//include libxml++, using the SAX parser
#include <xml++.h>

/**
  *@author Kevin Tambascio (ktambascio@yahoo.com)
  */

class MyMoneyStorageXMLCallback : public XMLParserCallback
{
public: 
	MyMoneyStorageXMLCallback();
	virtual ~MyMoneyStorageXMLCallback();

  typedef enum {
    PARSE_CURRENCY_DATA,
    PARSE_ACCOUNT,
    PARSE_STATE_UNKNOWN
  }eParseState;

  void start_document(void);
  void end_document(void);
  void start_element(const string &n, const XMLPropertyHash &p);
  void end_element(const string &n);
  void characters(const string &s);
  void comment(const string &s);
  void warning(const string &s);
  void error(const string &s);
  void fatal_error(const string &s);

  void ChangeParseState(eParseState state);

private:
  eParseState m_parseState;
  eParseState m_previousParseState;

};
#endif
