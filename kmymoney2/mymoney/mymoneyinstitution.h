/***************************************************************************
                          mymoneyinstitution.h
                          -------------------
    copyright            : (C) 2002-2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYINSTITUTION_H
#define MYMONEYINSTITUTION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include <kmymoney/mymoneyobject.h>
#include <kmymoney/mymoneykeyvaluecontainer.h>
#include <kmymoney/export.h>

class MyMoneyFile;
class MyMoneyMoney;

/**
  * This class represents a Bank contained within a MyMoneyFile object
  *
  * @author Thomas Baumgart
  */
class KMYMONEY_EXPORT MyMoneyInstitution : public MyMoneyObject, public MyMoneyKeyValueContainer
{
public:
  /**
    * This is the constructor for a new empty institution description
    */
  MyMoneyInstitution();

  /**
    * This is the constructor used by an application to fill the
    * values required for a new institution. This object should then be
    * passed to @see MyMoneyFile::addInstitution
    */
  MyMoneyInstitution(const QString& name,
              const QString& city,
              const QString& street,
              const QString& postcode,
              const QString& telephone,
              const QString& manager,
              const QString& sortCode);

  /**
    * This is the destructor for any MyMoneyInstitution object
    */
  ~MyMoneyInstitution();

  /**
    * This is the constructor for a new institution known to the current file
    *
    * @param id id assigned to the new institution object
    * @param right institution definition
    */
  MyMoneyInstitution(const QString& id, const MyMoneyInstitution& right);

  /**
    * This is the constructor for an institution that is described by a
    * QDomElement (e.g. from a file).
    *
    * @param el const reference to the QDomElement from which to
    *           create the object
    */
  MyMoneyInstitution(const QDomElement& el);

  const QString& manager(void) const { return m_manager; }
  const QString& name(void) const { return m_name; }
  const QString& postcode(void) const { return m_postcode; }
  const QString& street(void) const { return m_street; }
  const QString& telephone(void) const { return m_telephone; }
  const QString& town(void) const { return m_town; }
  const QString& city(void) const { return town(); }
  const QString& sortcode(void) const { return m_sortcode; }

  void setManager(QString manager) { m_manager = manager; }
  void setName(QString name) { m_name = name; }
  void setPostcode(QString code) { m_postcode = code; }
  void setStreet(QString street) { m_street = street; }
  void setTelephone(QString tel) { m_telephone = tel; }
  void setTown(QString town) { m_town = town; }
  void setCity(QString town) { setTown(town); }
  void setSortcode(QString code) { m_sortcode = code; }

  /**
    * This method adds the id of an account to the account list of
    * this institution It is verified, that the account is only
    * mentioned once.
    *
    * @param account id of the account to be added
    */
  void addAccountId(const QString& account);

  /**
    * This method deletes the id of an account from the account list
    * of this institution
    *
    * @param account id of the account to be deleted
    * @return id of account deleted, otherwise empty string
    */
  QString removeAccountId(const QString& account);

  /**
    * This method is used to return the set of accounts known to
    * this institution
    * return QStringList of account ids
    */
  const QStringList& accountList(void) const { return m_accountList; }

  /**
    * This method returns the number of accounts known to
    * this institution
    * @return number of accounts
    */
  unsigned int accountCount(void) const { return m_accountList.count(); }

  bool operator == (const MyMoneyInstitution&) const;
  bool operator < (const MyMoneyInstitution& right) const;

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

  QPixmap pixmap() const;

private:
  // Bank 'fields'
  /**
    * This member variable keeps the name of the institution
    */
  QString m_name;

  /**
    * This member variable keeps the city of the institution
    */
  QString m_town;

  /**
    * This member variable keeps the street of the institution
    */
  QString m_street;

  /**
    * This member variable keeps the zip-code of the institution
    */
  QString m_postcode;

  /**
    * This member variable keeps the telephone number of the institution
    */
  QString m_telephone;

  /**
    * This member variable keeps the name of the representative of
    * the institution
    */
  QString m_manager;

  /**
    * This member variable keeps the sort code of the institution.
    * FIXME: I have no idea
    * what it is good for. I keep it because it was in the old engine.
    */
  QString m_sortcode;

  /**
    * This member variable keeps the sorted list of the account ids
    * available at this institution
    */
  QStringList m_accountList;
};

#endif
