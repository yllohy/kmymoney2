/***************************************************************************
                          mymoneybank.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYBANK_H
#define MYMONEYBANK_H

#include <qdatastream.h>
#include "mymoneyaccount.h"

// This class represents a Bank contained within a file
class MyMoneyBank {
private:
  // Bank 'fields'
  QString m_name;
  QString m_city;
  QString m_street;
  QString m_postcode;
  QString m_telephone;
  QString m_manager;
  QString m_sortCode;

  // The accounts this bank contains
  QList<MyMoneyAccount> m_accounts;

  friend QDataStream &operator<<(QDataStream &, const MyMoneyBank &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyBank &);

  bool findAccountPosition(const MyMoneyAccount& account, unsigned int&);

public:
	MyMoneyBank();
	MyMoneyBank(const QString& name, const QString& sortCode, const QString& city, const QString& street,
	  const QString& postcode, const QString& telephone, const QString& manager);
	~MyMoneyBank();

	// Simple get operations
	QString name(void) const { return m_name; }
  QString city(void) const { return m_city; }
  QString street(void) const { return m_street; }
  QString postcode(void) const { return m_postcode; }
  QString telephone(void) const { return m_telephone; }
  QString manager(void) const { return m_manager; }
  QString sortCode(void) const { return m_sortCode; }

	// Simple set operations
  void setName(const QString& name) { m_name = name; }
  void setCity(const QString& val) { m_city = val; }
  void setStreet(const QString& val) { m_street = val; }
  void setPostcode(const QString& val) { m_postcode = val; }
  void setTelephone(const QString& val) { m_telephone = val; }
  void setManager(const QString& val) { m_manager = val; }
  void setSortCode(const QString& val) { m_sortCode = val; }

	void clear(void);

	// Operations on the contained accounts
  bool newAccount(const QString& name, const QString& number, MyMoneyAccount::accountTypeE type,
    const QString& description, const QDate& lastReconcile);
  MyMoneyAccount* account(const MyMoneyAccount& account);
	MyMoneyAccount* accountFirst(void);
	MyMoneyAccount* accountNext(void);
	MyMoneyAccount* accountLast(void);
  unsigned int accountCount(void);
  bool removeAccount(const MyMoneyAccount& account);

  bool operator == (const MyMoneyBank&);

  // Copy constructors
  MyMoneyBank(const MyMoneyBank&);
  MyMoneyBank& operator = (const MyMoneyBank&);
};

#endif
