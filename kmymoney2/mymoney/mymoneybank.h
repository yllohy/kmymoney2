/***************************************************************************
                          mymoneybank.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
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

class MyMoneyFile;

// This class represents a Bank contained within a file
class MyMoneyBank {
private:
	MyMoneyFile *m_parent;
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

	MyMoneyBank(MyMoneyFile* parent, const QString& name, const QString& sortCode, const QString& city, const QString& street,
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
	MyMoneyFile *file(void) { return m_parent; }

	// Simple set operations
  void setName(const QString& name);
  void setCity(const QString& val);
  void setStreet(const QString& val);
  void setPostcode(const QString& val);
  void setTelephone(const QString& val);
  void setManager(const QString& val);
  void setSortCode(const QString& val);

	void clear(void);

  /**
    * Set the parent's dirty flag, if a parent is available
    *
    * @param flag The value to which the dirty flag should be set
    *
    * @return none
  **/
  void setDirty(const bool flag);

	// Operations on the contained accounts
  bool newAccount(const QString& name, const QString& number, MyMoneyAccount::accountTypeE type,
    const QString& description, const QDate openingDate, const MyMoneyMoney openingBal, const QDate& lastReconcile);
  MyMoneyAccount* account(const MyMoneyAccount& account);
	MyMoneyAccount* accountFirst(void);
	MyMoneyAccount* accountNext(void);
	MyMoneyAccount* accountLast(void);
  unsigned int accountCount(void);
  bool removeAccount(const MyMoneyAccount& account);

  bool readAllData(int version, QDataStream& stream);

  bool operator == (const MyMoneyBank&);

  // Copy constructors
  MyMoneyBank(const MyMoneyBank&);
  MyMoneyBank& operator = (const MyMoneyBank&);
};

#endif
