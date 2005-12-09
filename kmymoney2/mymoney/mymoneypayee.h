/***************************************************************************
                          mymoneypayee.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
                               2005 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qcstring.h>
#include <kmymoney/export.h>
#include <kmymoney/mymoneyobject.h>

/**
  * This class represents a payee or receiver within the MyMoney engine.
  * Since it is not payee-specific, it is also used as a generic address
  * book entry.
  *
  * @author Thomas Baumgart
  */
class KMYMONEY_EXPORT MyMoneyPayee : public MyMoneyObject
{
private:
  // Simple fields
  QString m_name;
  QString m_address;
  QString m_city;
  QString m_state;
  QString m_postcode;
  QString m_telephone;
  QString m_email;

  /**
    * This member keeps a reference to an external database
    * (e.g. kaddressbook). It is the responsability of the
    * application to format the reference string
    * (e.g. encoding the name of the external database into the
    * reference string).
    * If no external database is available it should be kept
    * empty by the application.
    */
  QString m_reference;

  // friend QDataStream &operator<<(QDataStream &, const MyMoneyPayee &);
  // friend QDataStream &operator>>(QDataStream &, MyMoneyPayee &);

public:
  MyMoneyPayee();
  MyMoneyPayee(const QCString& id, const MyMoneyPayee& payee);
  MyMoneyPayee(const QString& name,
	  const QString& address=QString::null,
	  const QString& city=QString::null,
	  const QString& state=QString::null,
	  const QString& postcode=QString::null,
	  const QString& telephone=QString::null,
	  const QString& email=QString::null);
  ~MyMoneyPayee();

  // Simple get operations
  QString name(void) const            { return m_name; }
  QString address(void) const         { return m_address; }
  QString city(void) const            { return m_city; }
  QString state(void) const           { return m_state; }
  QString postcode(void) const        { return m_postcode; }
  QString telephone(void) const       { return m_telephone; }
  QString email(void) const           { return m_email; }
  const QCString id(void) const       { return m_id; };
  const QString reference(void) const { return m_reference; };

  // Simple set operations
  void setName(const QString& val)      { m_name = val; }
  void setAddress(const QString& val)   { m_address = val; }
  void setCity(const QString& val)      { m_city = val; }
  void setState(const QString& val)     { m_state = val; }
  void setPostcode(const QString& val)  { m_postcode = val; }
  void setTelephone(const QString& val) { m_telephone = val; }
  void setEmail(const QString& val)     { m_email = val; }
  void setReference(const QString& ref) { m_reference = ref; }
  void setId(const QCString& val)       { m_id = val; }

  // Copy constructors
  MyMoneyPayee(const MyMoneyPayee&);

  // Equality operator
  const bool operator == (const MyMoneyPayee &) const;

  void writeXML(QDomDocument& document, QDomElement& parent) const;

  void readXML(const QDomElement& node);

  static MyMoneyPayee null;
};

inline bool operator==(const MyMoneyPayee& lhs, const QCString& rhs) { return lhs.id() == rhs; }

#endif
