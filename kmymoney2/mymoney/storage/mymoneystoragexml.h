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

// ----------------------------------------------------------------------------
// QT Includes

#include <qdom.h>
#include <qdatastream.h>
class QIODevice;

// ----------------------------------------------------------------------------
// Project Includes

#include "imymoneyserialize.h"
#include "imymoneystorageformat.h"

/**
  *@author Kevin Tambascio (ktambascio@yahoo.com)
  */
class MyMoneyStorageXML : public IMyMoneyStorageFormat
{
public: 
	MyMoneyStorageXML();
	virtual ~MyMoneyStorageXML();

  enum fileVersionDirectionType {
    Reading = 0,          /**< version of file to be read */
    Writing = 1           /**< version to be used when writing a file */
  };

protected:
   void          setProgressCallback(void(*callback)(int, int, const QString&)) {};

private:
  IMyMoneySerialize*  m_pStorage;
  
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

  void writeUserInformation(QDomDocument *pDoc, QDomElement& userInfo, IMyMoneySerialize* storage);
  
  void writeInstitution(QDomDocument *pDoc, QDomElement& institutions, const MyMoneyInstitution& i);
  void writeInstitutions(QDomDocument *pDoc, QDomElement& institutions, IMyMoneySerialize* storage);

  void writePayees(QDomDocument *pDoc, QDomElement& payees, IMyMoneySerialize* storage);
  void writePayee(QDomDocument *pDoc, QDomElement& payees, const MyMoneyPayee& p);
  
  void writeAccounts(QDomDocument *pDoc, QDomElement& accounts, IMyMoneySerialize* storage);
  void writeAccount(QDomDocument *pDoc, QDomElement& accounts, const MyMoneyAccount& p);

  void writeTransactions(QDomDocument *pDoc, QDomElement& transactions, IMyMoneySerialize* storage);
  void writeTransaction(QDomDocument *pDoc, QDomElement& transactions, const MyMoneyTransaction& tx);

  void writeSplits(QDomDocument *pDoc, QDomElement& splits, IMyMoneySerialize* storage);
  void writeSplit(QDomDocument *pDoc, QDomElement& splitElement, const MyMoneySplit& split);
    
  void readFile(QIODevice* s, IMyMoneySerialize* storage);
  void writeFile(QIODevice* s, IMyMoneySerialize* storage);
  /** No descriptions */


private:
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
