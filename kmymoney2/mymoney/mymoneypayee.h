/***************************************************************************
                          mymoneypayee.h
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

#ifndef MYMONEYPAYEE_H
#define MYMONEYPAYEE_H

#include <qstring.h>
#include <qcstring.h>

/**
  * This class represents a payee or receiver within the MyMoney engine
  *
  * @author Thomas Baumgart
  */
class MyMoneyPayee {
private:
	// Simple fields
	QString m_name;
	QString m_address;
	QString m_postcode;
	QString m_telephone;
	QString m_email;

  /**
    * This member keeps the MyMoney id
    */
  QCString m_id;

  /**
    * This member keeps a reference to an external database
    * (e.g. kaddressbook). It is the responsability of the
    * application to format the reference string
    * (e.g. encoding the name of the external database into the
    * reference string).
    * If no external database is available it should be kept
    * emtpy by the application.
    */
	QString m_reference;

  friend QDataStream &operator<<(QDataStream &, const MyMoneyPayee &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyPayee &);

public:
	MyMoneyPayee();
  MyMoneyPayee(const QCString& id, const MyMoneyPayee& payee);
	MyMoneyPayee(const QString& name, const QString address=QString::null, const QString postcode=QString::null, const QString telephone=QString::null, const QString email=QString::null);
	~MyMoneyPayee();
	
	// Simple get operations
	QString name(void) const { return m_name; }
	QString address(void) const { return m_address; }
	QString postcode(void) const { return m_postcode; }
	QString telephone(void) const { return m_telephone; }
	QString email(void) const { return m_email; }
  const QCString id(void) const { return m_id; };
  const QString reference(void) const { return m_reference; };
  	
	// Simple set operations
	void setName(const QString& val) { m_name = val; }
	void setAddress(const QString& val) { m_address = val; }
	void setPostcode(const QString& val) { m_postcode = val; }
	void setTelephone(const QString& val) { m_telephone = val; }
	void setEmail(const QString& val) { m_email = val; }
  void setReference(const QString& ref) { m_reference = ref; }
	
  // Copy constructors
  MyMoneyPayee(const MyMoneyPayee&);
};

#endif
