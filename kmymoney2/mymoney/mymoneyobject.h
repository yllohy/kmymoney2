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
    * This method must be provided by all derived objects. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  virtual bool hasReferenceTo(const QCString& id) const = 0;

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

protected:
  QCString               m_id;
};
#endif

