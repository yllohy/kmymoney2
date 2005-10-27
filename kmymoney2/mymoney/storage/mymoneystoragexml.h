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
  *@author Kevin Tambascio (ktambascio@users.sourceforge.net)
  */

#define VERSION_0_60_XML  0x10000010    // Version 0.5 file version info
#define VERSION_0_61_XML  0x10000011    // use 8 bytes for MyMoneyMoney objects

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

  void readFileInformation(QDomElement fileInfo);
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

  virtual void readFile(QIODevice* s, IMyMoneySerialize* storage);
  virtual void writeFile(QIODevice* s, IMyMoneySerialize* storage);

  virtual QDomElement writeKeyValuePairs(const QMap<QCString, QString> pairs);
  virtual QMap<QCString, QString> readKeyValuePairs(QDomElement& element);

  void readUserInformation(QDomElement userElement);
  /** No descriptions */

  void readInstitutions(QDomElement& childElement);
  MyMoneyInstitution readInstitution(const QDomElement& institution);

  void readPayees(QDomElement& payees);
  MyMoneyPayee readPayee(const QDomElement& payee);

  void readAccounts(QDomElement& accounts);
  MyMoneyAccount readAccount(const QDomElement& account);

  MyMoneySplit readSplit(QDomElement& splitElement);
  void readSplits(MyMoneyTransaction& t, QDomElement& splits);

  void readTransactions(QDomElement& transactions);
  MyMoneyTransaction readTransaction(QDomElement& transaction, const bool withinSchedule = false);

  void readSchedules(QDomElement& schedules);
  MyMoneySchedule readSchedule(QDomElement& schedule);

  virtual void writeSecurities(QDomElement& securities);
  virtual void writeSecurity(QDomElement& securityElement, const MyMoneySecurity& security);

  virtual void writeCurrencies(QDomElement& currencies);
  virtual void writeCurrency(QDomElement& currencyElement, const MyMoneySecurity& currency);

  void readSecurities(QDomElement& securites);
  MyMoneySecurity readSecurity(QDomElement& securityElement);

  void readCurrencies(QDomElement& currencies);
  const MyMoneySecurity readCurrency(QDomElement& currencyElement);

  void readPrices(QDomElement& prices);
  void readPricePair(const QDomElement& pricePair);
  const MyMoneyPrice readPrice(const QCString& from, const QCString& to, const QDomElement& price);

  void readReports(QDomElement& e);
  virtual void writeReports(QDomElement& e);

  QDomElement findChildElement(const QString& name, const QDomElement& root);

  /**
    * This method converts a valid date into the ISO format. If
    * the date is invalid an empty string is returned.
    *
    * @param date reference to QDate object to be converted
    * @return QString containing converted date
    */
  QString getString(const QDate& date) const;

private:
  void (*m_progressCallback)(int, int, const QString&);
  const unsigned long extractId(const QCString& txt) const;
  QDate getDate(const QString& strText) const;
  const QCString QCStringEmpty(const QString& val) const;
  const QString QStringEmpty(const QString& val) const;
  const uint getChildCount(const QDomElement& element) const;

protected:
  IMyMoneySerialize *m_storage;
  QDomDocument *m_doc;

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

  /**
    * This member keeps the id of the base currency. We need this
    * temporarily to convert the price history from the old to the
    * new format. This should go at some time beyond 0.8 (ipwizard)
    */
  QCString m_baseCurrencyId;

};

#endif
