/***************************************************************************
                          imymoneystorageformat.h  -  description
                             -------------------
    begin                : Sun Oct 27 2002
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

#ifndef IMYMONEYSTORAGEFORMAT_H
#define IMYMONEYSTORAGEFORMAT_H


/**
  *@author Kevin Tambascio (ktambascio@yahoo.com)
  */

// ----------------------------------------------------------------------------
// QT Includes

class QIODevice;
class QProgressDialog;

// ----------------------------------------------------------------------------
// Project Includes

class IMyMoneySerialize;
  

class IMyMoneyStorageFormat
{
public: 
	IMyMoneyStorageFormat();
	virtual ~IMyMoneyStorageFormat();

    enum fileVersionDirectionType {
    Reading = 0,          /**< version of file to be read */
    Writing = 1           /**< version to be used when writing a file */
  };

  virtual void readFile(QIODevice* qf, IMyMoneySerialize* storage) = 0;
  // virtual void readStream(QDataStream& s, IMyMoneySerialize* storage) = 0;

  virtual void writeFile(QIODevice* qf, IMyMoneySerialize* storage) = 0;
  //virtual void writeStream(QDataStream& s, IMyMoneySerialize* storage) = 0;

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
};

#endif
