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

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
class QStringList;

// ----------------------------------------------------------------------------
// Project Includes

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
  QString m_notes;

  // Transaction matching fields
  bool m_matchingEnabled;      //< Whether this payee should be matched at all
  bool m_usingMatchKey;        //< If so, whether a m_matchKey list is used (true), or just m_name is used (false)
  bool m_matchKeyIgnoreCase;   //< Whether to ignore the case of the match key or name

  /**
   * Semicolon separated list of matching keys used when trying to find a suitable
   * payee for imported transactions.
   */
  QString m_matchKey;

  // Category (account) matching fields
  QString m_defaultAccountId;

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

public:
  typedef enum {
    matchDisabled = 0,
    matchName,
    matchKey
  } payeeMatchType;

  MyMoneyPayee();
  MyMoneyPayee(const QString& id, const MyMoneyPayee& payee);
  MyMoneyPayee(const QString& name,
          const QString& address=QString::null,
          const QString& city=QString::null,
          const QString& state=QString::null,
          const QString& postcode=QString::null,
          const QString& telephone=QString::null,
          const QString& email=QString::null);
  /**
    * This is the constructor for a payee that is described by a
    * QDomElement (e.g. from a file).
    *
    * @param el const reference to the QDomElement from which to
    *           create the object
    */
  MyMoneyPayee(const QDomElement& el);

  ~MyMoneyPayee();

  // Simple get operations
  QString name(void) const            { return m_name; }
  QString address(void) const         { return m_address; }
  QString city(void) const            { return m_city; }
  QString state(void) const           { return m_state; }
  QString postcode(void) const        { return m_postcode; }
  QString telephone(void) const       { return m_telephone; }
  QString email(void) const           { return m_email; }
  QString notes(void) const           { return m_notes; }

  const QString id(void) const        { return m_id; };
  const QString reference(void) const { return m_reference; };

  // Simple set operations
  void setName(const QString& val)      { m_name = val; }
  void setAddress(const QString& val)   { m_address = val; }
  void setCity(const QString& val)      { m_city = val; }
  void setState(const QString& val)     { m_state = val; }
  void setPostcode(const QString& val)  { m_postcode = val; }
  void setTelephone(const QString& val) { m_telephone = val; }
  void setEmail(const QString& val)     { m_email = val; }
  void setNotes(const QString& val)     { m_notes = val; }
  void setReference(const QString& ref) { m_reference = ref; }

  /**
   * Get all match data in one call
   *
   * @param ignorecase Bool which will be replaced to indicate whether the match is
   * case-sensitive (false) or case-insensitive (true)
   * @param keys List of strings which will be replaced by the match key to use for this payee
   *
   * @return the matching type (see payeeMatchType for details)
   */
  payeeMatchType matchData(bool& ignorecase, QStringList& keys) const;

  /**
   * Set all match data in one call
   *
   * @param type matching type (see payeeMatchType for details)
   * @param ignorecase Whether case should be ignored for the key/name match
   * @param keys A list of keys themselves, if applicable
   */
  void setMatchData(payeeMatchType type, bool ignorecase, const QStringList& keys);

  /**
   * Get all match data in one call (overloaded version for database module)
   *
   * @param ignorecase Bool which will be replaced to indicate whether the match is
   * case-sensitive (false) or case-insensitive (true)
   * @param keyString A list of keys in single-string format, if applicable
   *
   * @return the matching type (see payeeMatchType for details)
   */
  payeeMatchType matchData(bool& ignorecase, QString& keyString) const;

  /**
   * Set all match data in one call (overloaded version for database module)
   *
   * @param type matching type (see payeeMatchType for details)
   * @param ignorecase Whether case should be ignored for the key/name match
   * @param keys A list of keys in single-string format, if applicable
   */
  void setMatchData(payeeMatchType type, bool ignorecase, const QString& keys);


  bool defaultAccountEnabled() const { return !m_defaultAccountId.isEmpty(); }
  const QString& defaultAccountId() const { return m_defaultAccountId; }
  void setDefaultAccountId(const QString& id = QString()) {
    m_defaultAccountId = id;
  }

  // Copy constructors
  MyMoneyPayee(const MyMoneyPayee&);

  // Equality operator
  bool operator == (const MyMoneyPayee &) const;

  void writeXML(QDomDocument& document, QDomElement& parent) const;

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  virtual bool hasReferenceTo(const QString& id) const;

  static MyMoneyPayee null;
};

inline bool operator==(const MyMoneyPayee& lhs, const QString& rhs) { return lhs.id() == rhs; }

#endif
// vim:cin:si:ai:et:ts=2:sw=2:
