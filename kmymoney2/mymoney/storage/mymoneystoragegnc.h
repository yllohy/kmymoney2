/***************************************************************************
                          mymoneystoragegnc  -  description
                             -------------------
    begin                : Wed Mar 3 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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


#ifndef MYMONEYSTORAGEGNC_H
#define MYMONEYSTORAGEGNC_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qdom.h>
#include <qdatastream.h>
class QIODevice;

// ----------------------------------------------------------------------------
// Project Includes

#include "imymoneyserialize.h"
#include "imymoneystorageformat.h"

typedef QMap<QCString, QCString> map_accountIds;
typedef map_accountIds::iterator map_accountIds_iter;
typedef map_accountIds::const_iterator map_accountIds_citer;

/**
  *@author Kevin Tambascio (ktambascio@users.sourceforge.net)
  */

#define VERSION_0_60_XML  0x10000010    // Version 0.5 file version info
#define VERSION_0_61_XML  0x10000011    // use 8 bytes for MyMoneyMoney objects

#define GNUCASH_ID_KEY "GNUCASH_ID"
   
class MyMoneyStorageGNC : public IMyMoneyStorageFormat
{
public: 
	MyMoneyStorageGNC();
	virtual ~MyMoneyStorageGNC();

  enum fileVersionDirectionType {
    Reading = 0,          /**< version of file to be read */
    Writing = 1           /**< version to be used when writing a file */
  };

protected:
   void          setProgressCallback(void(*callback)(int, int, const QString&));
   void          signalProgress(int current, int total, const QString& = "");
private:
  IMyMoneySerialize *m_storage;
  QDomDocument *m_doc;
  
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

  /**
    * A parameter is used to determine the direction.
    *
    * @param pDoc: pointer to the entire DOM document
    * @param userInfo: DOM Element to write the user information to.  
    *
    * @return void
    *
    * @see
    */

  void readFile(QIODevice* s, IMyMoneySerialize* storage);
  void writeFile(QIODevice* s, IMyMoneySerialize* storage) {};

  QMap<QCString, QString> readKeyValuePairs(QDomElement& element);

  void readUserInformation(QDomElement userElement);
  /** No descriptions */

  void readAccount(const QDomElement& account);

  MyMoneySplit readSplit(QDomElement& splitElement);
  void readSplits(MyMoneyTransaction& t, QDomElement& splits);

  void readTransaction(QDomElement& transaction, const bool withinSchedule = false);

  void readSchedules(QDomElement& schedules);
  MyMoneySchedule readSchedule(QDomElement& schedule);
  
  QDomElement findChildElement(const QString& name, const QDomElement& root);
  
private:
  void (*m_progressCallback)(int, int, const QString&);
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

  const unsigned long extractId(const QCString& txt) const;
  QDate getDate(const QString& strText) const;
  QString getString(const QDate& date) const;
  const QCString QCStringEmpty(const QString& val) const;
  const QString QStringEmpty(const QString& val) const;
  const uint getChildCount(const QDomElement& element) const;
	QString m_mainAssetId;
  QString m_mainLiabilityId;
  QString m_mainIncomeId;
  QString m_mainExpenseId;
  QString m_mainEquityId;

  QMap<QCString, QCString> m_mapIds;
};

#endif
