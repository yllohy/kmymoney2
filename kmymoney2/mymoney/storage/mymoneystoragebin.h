/***************************************************************************
                          imymoneystoragebin.h  -  description
                             -------------------
    begin                : Sun May 5 2002
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

#ifndef MYMONEYSTORAGEBIN_H
#define MYMONEYSTORAGEBIN_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qdatastream.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "imymoneyserialize.h"
#include "imymoneystorageformat.h"

/**
  *@author Thomas Baumgart
  */

class IMyMoneyStorageFormat;

class MyMoneyStorageBin : public IMyMoneyStorageFormat {
public: 
	MyMoneyStorageBin();
	~MyMoneyStorageBin();

  //enum fileVersionDirectionType {
  //  Reading = 0,          /**< version of file to be read */
  //  Writing = 1,          /**< version to be used when writing a file */
  //};

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

  void readStream(QDataStream& s, IMyMoneySerialize* storage);

  void writeStream(QDataStream& s, IMyMoneySerialize* storage);

private:
  void readOldFormat(QDataStream& s, IMyMoneySerialize* storage);
  void readNewFormat(QDataStream& s, IMyMoneySerialize* storage);
  void addCategory(IMyMoneySerialize* storage,
                   QMap<QString, QCString>& categories,
                   const QString& majorName,
                   const QString& minorName,
                   const MyMoneyAccount::accountTypeE type);

  void writeInstitutions(QDataStream& s, IMyMoneySerialize* storage);
  void writeInstitution(QDataStream&s, const MyMoneyInstitution& i);
  void readInstitutions(QDataStream&s, IMyMoneySerialize *storage);
  const MyMoneyInstitution readInstitution(QDataStream& s);

  void writePayees(QDataStream& s, IMyMoneySerialize* storage);
  void writePayee(QDataStream& s, const MyMoneyPayee& p);
  void readPayees(QDataStream& s, IMyMoneySerialize* storage);
  const MyMoneyPayee readPayee(QDataStream& s);

  void writeAccounts(QDataStream& s, IMyMoneySerialize* storage);
  void writeAccount(QDataStream& s, const MyMoneyAccount& p);
  void readAccounts(QDataStream& s, IMyMoneySerialize* storage);
  const MyMoneyAccount readAccount(QDataStream& s);

  void writeTransactions(QDataStream& s, IMyMoneySerialize* storage);
  void writeTransaction(QDataStream& s, const MyMoneyTransaction& t);
  void readTransactions(QDataStream& s, IMyMoneySerialize* storage);
  const MyMoneyTransaction readTransaction(QDataStream& s);

  void writeSplits(QDataStream& s, const MyMoneyTransaction& t);
  void writeSplit(QDataStream& s, const MyMoneySplit& s);
  const QValueList<MyMoneySplit> readSplits(QDataStream& s);
  const MyMoneySplit readSplit(QDataStream& s);

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

  #define VERSION_0_3_3 0x00000006    // MAGIC1 for version 0.33 files

  #define VERSION_0_4_0 0x00000007    // MAGIC1 for version 0.4 files

  #define MAGIC_0_50  0x4B4D794D      // "KMyM" MAGIC1 for version 0.5 files
  #define MAGIC_0_51  0x6F6E6579      // "oney" second part of MAGIC

  #define VERSION_0_5_0 0x00000010    // Version 0.5 file version info

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
