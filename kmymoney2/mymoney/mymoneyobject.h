/***************************************************************************
                          mymoneyobject.h
                             -------------------
    copyright            : (C) 2005 by Thomas Baumgart
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

#ifndef MYMONEYOBJECT_H
#define MYMONEYOBJECT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qcstring.h>
#include <qdom.h>
#include <qdatetime.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/export.h>

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents the base class of all MyMoney objects.
  */
class KMYMONEY_EXPORT MyMoneyObject
{
public:
  /**
    * This contructor assigns the id to the MyMoneyObject
    *
    * @param id ID of object
    */
  MyMoneyObject(const QCString& id);

  /**
    * This is the constructor for the MyMoneyObject object
    */
  MyMoneyObject();

  /**
    * This is the destructor for the MyMoneyObject object
    */
  virtual ~MyMoneyObject();

  /**
    * This method retrieves the id of the object
    *
    * @return ID of object
    */
  const QCString& id(void) const { return m_id; };

  void setId(const QCString& id) __attribute__ ((deprecated));

  /**
    * This method clears the id of the object
    */
  void clearId(void);

  /**
    * This method creates a QDomElement for the @p document
    * under the parent node @p parent.
    *
    * @param document reference to QDomDocument
    * @param parent reference to QDomElement parent node
    */
  virtual void writeXML(QDomDocument& document, QDomElement& parent) const = 0;

  /**
    * This method reads in data for the object from the node
    * The type will be checked and an exception thrown if
    * it does not match.
    *
    * @param node QDomElement containing the data
    */
  virtual void readXML(const QDomElement& node) = 0;

  const bool operator == (const MyMoneyObject& right) const;

  /**
    * This method returns a date in the form specified by Qt::ISODate.
    * If the @p date is invalid an empty string will be returned.
    *
    * @param date const reference to date to be converted
    * @return QString containing the converted date
    */
  QString dateToString(const QDate& date) const;

  /**
    * This method returns a date as QDate object as specified by
    * the parameter @p str. @p str must be in Qt::ISODate format.
    * If @p str is empty or contains an invalid date, QDate() is
    * returned.
    *
    * @param str date in Qt::ISODate format
    * @return QDate object
    */
  QDate stringToDate(const QString& str) const;

  QCString QCStringEmpty(const QString& val) const;
  QString QStringEmpty(const QString& val) const;

protected:
  QCString               m_id;
};
#endif
