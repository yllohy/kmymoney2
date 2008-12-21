/***************************************************************************
                          mymoneystoragexml.h  -  description
                             -------------------
    begin                : Thu Oct 24 2002
    copyright            : (C) 2002 by Kevin Tambascio
                           (C) 2004 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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
class MyMoneyStorageXMLPrivate;
class MyMoneyXmlContentHandler;

/**
  *@author Kevin Tambascio (ktambascio@users.sourceforge.net)
  */

#define VERSION_0_60_XML  0x10000010    // Version 0.5 file version info
#define VERSION_0_61_XML  0x10000011    // use 8 bytes for MyMoneyMoney objects

class MyMoneyStorageXML : public IMyMoneyStorageFormat
{
  friend class MyMoneyXmlContentHandler;
public:
  MyMoneyStorageXML();
  virtual ~MyMoneyStorageXML();

  enum fileVersionDirectionType {
    Reading = 0,          /**< version of file to be read */
    Writing = 1           /**< version to be used when writing a file */
  };

protected:
   void          setProgressCallback(void(*callback)(int, int, const QString&));
   void          signalProgress(int current, int total, const QString& = "");

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

  QValueList<QDomElement> readElements(QString groupTag, QString itemTag = QString());

  bool readFileInformation(const QDomElement& fileInfo);
  virtual void writeFileInformation(QDomElement& fileInfo);

  virtual void writeUserInformation(QDomElement& userInfo);

  virtual void writeInstitution(QDomElement& institutions, const MyMoneyInstitution& i);
  virtual void writeInstitutions(QDomElement& institutions);

  virtual void writePrices(QDomElement& prices);
  virtual void writePricePair(QDomElement& price, const MyMoneyPriceEntries& p);
  virtual void writePrice(QDomElement& prices, const MyMoneyPrice& p);

  virtual void writePayees(QDomElement& payees);
  virtual void writePayee(QDomElement& payees, const MyMoneyPayee& p);

  virtual void writeAccounts(QDomElement& accounts);
  virtual void writeAccount(QDomElement& accounts, const MyMoneyAccount& p);

  virtual void writeTransactions(QDomElement& transactions);
  virtual void writeTransaction(QDomElement& transactions, const MyMoneyTransaction& tx);

  virtual void writeSchedules(QDomElement& scheduled);
  virtual void writeSchedule(QDomElement& scheduledTx, const MyMoneySchedule& tx);

  virtual void writeReports(QDomElement& e);
  virtual void writeBudgets(QDomElement& e);

  virtual void writeSecurities(QDomElement& securities);
  virtual void writeSecurity(QDomElement& securityElement, const MyMoneySecurity& security);

  virtual void writeCurrencies(QDomElement& currencies);

  virtual QDomElement writeKeyValuePairs(const QMap<QString, QString> pairs);

  virtual void readFile(QIODevice* s, IMyMoneySerialize* storage);
  virtual void writeFile(QIODevice* s, IMyMoneySerialize* storage);

  bool readUserInformation(const QDomElement& userElement);

  void readPricePair(const QDomElement& pricePair);
  const MyMoneyPrice readPrice(const QString& from, const QString& to, const QDomElement& price);

  QDomElement findChildElement(const QString& name, const QDomElement& root);

private:
  void (*m_progressCallback)(int, int, const QString&);

protected:
  IMyMoneySerialize *m_storage;
  QDomDocument *m_doc;

private:
  MyMoneyStorageXMLPrivate*  d;

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
    * This member keeps the id of the base currency. We need this
    * temporarily to convert the price history from the old to the
    * new format. This should go at some time beyond 0.8 (ipwizard)
    */
  QString m_baseCurrencyId;

};

#endif
