/***************************************************************************
                          mymoneystoragexml.h  -  description
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

#ifndef MYMONEYSTORAGEXML_H
#define MYMONEYSTORAGEXML_H

#ifdef _COMPILE_XML

#include <xml++.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qdatastream.h>
class QIODevice;

// ----------------------------------------------------------------------------
// Project Includes

#include "imymoneyserialize.h"
#include "imymoneystorageformat.h"
#include "mymoneyxmlparser.h"
//#include "mymoneystoragexmlcallback.h"

typedef enum {
    PARSE_NEXTIDS,
    PARSE_USERINFO,
    PARSE_USERINFO_ADDRESS,
    PARSE_USERINFO_ADDRESS_STREET,
    PARSE_USERINFO_ADDRESS_CITY,
    PARSE_USERINFO_ADDRESS_STATE,
    PARSE_USERINFO_ADDRESS_ZIPCODE,
    PARSE_USERINFO_ADDRESS_COUNTY,
    PARSE_USERINFO_ADDRESS_COUNTRY,
    PARSE_USERINFO_ADDRESS_TELEPHONE,
    PARSE_ACCOUNTS,
    PARSE_INSTITUTIONS,
    PARSE_PAYEES,
    PARSE_STATE_UNKNOWN
  }eParseState;

using namespace xmlpp;

/**
  *@author Kevin Tambascio (ktambascio@yahoo.com)
  */

                            
class MyMoneyStorageXML : public IMyMoneyStorageFormat, public xmlpp::XMLParserCallback
{
public: 
	MyMoneyStorageXML();
	virtual ~MyMoneyStorageXML();

  enum fileVersionDirectionType {
    Reading = 0,          /**< version of file to be read */
    Writing = 1           /**< version to be used when writing a file */
  };

  void start_document(void);
  void end_document(void);
  void start_element(const std::string &n, const xmlpp::XMLPropertyMap &p);
  void end_element(const std::string &n);
  void characters(const std::string &s);
  void comment(const std::string &s);
  void warning(const std::string &s);
  void error(const std::string &s);
  void fatal_error(const std::string &s);

  void ChangeParseState(eParseState state);

  void setProgressCallback(void(*callback)(int, int, const QString&)) {};

private:
  IMyMoneySerialize* m_pStorage;
  std::string getPropertyValue(std::string str, XMLPropertyMap p);
  eParseState m_parseState;
  eParseState m_previousParseState;


  /**
    * This method returns the version of the underlying file. It
    * is used by the MyMoney objects contained in a MyMoneyStorageBin object (e.g.
    * MyMoneyAccount, MyMoneyInstitution, MyMoneyTransaction, etc.) to
    * determine the layout used when reading/writing a persistant file.
    * A parameter is used to determine the direction.
    *
    * @param dir information about the direction (reading/writing). The
    *            default is reading.
    *
    * @return version QString of file's version
    *
    * @see m_fileVersionRead, m_fileVersionWrite
    */
  static unsigned int fileVersion(fileVersionDirectionType dir = Reading);

  void readFile(QIODevice* s, IMyMoneySerialize* storage);
  void writeFile(QIODevice* s, IMyMoneySerialize* storage);
  /** No descriptions */


private:
  MyMoneyXMLParser *m_parser;
  //MyMoneyStorageXMLCallback* m_callback;
  /**
    * Instantiates the XML parser if it hasn't been created already.
    */
  bool CreateXMLParser();

  /**
    * This member is used to store the file version information
    * obtained while reading a file.
    */
  static unsigned int fileVersionRead;

  /**
    * This member is used to store the file version information
    * to be used when writing a file.
    */
  static unsigned int fileVersionWrite;

  /**
    * This member variable signals, if the MyMoneyFile object is
    * protected by a password
    */
  bool m_passwordProtected;

  /**
    * This member keeps a copy of the password required to access
    * the MyMoneyFile object.
    */
  QString m_password;

  /**
    * This member variable signals, if the MyMoneyFile object is
    * encrypted on the permanent storage device
    */
  bool m_encrypted;

  
};

#endif
#endif
