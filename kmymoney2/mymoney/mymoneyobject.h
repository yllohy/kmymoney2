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

#include <qstring.h>
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
  const QString& id(void) const { return m_id; };

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
  virtual bool hasReferenceTo(const QString& id) const = 0;

  /**
    * This method creates a QDomElement for the @p document
    * under the parent node @p parent.
    *
    * @param document reference to QDomDocument
    * @param parent reference to QDomElement parent node
    */
  virtual void writeXML(QDomDocument& document, QDomElement& parent) const = 0;

  bool operator == (const MyMoneyObject& right) const;

  static const QString& emptyId(void);

protected:
  /**
    * This contructor assigns the id to the MyMoneyObject
    *
    * @param id ID of object
    */
  MyMoneyObject(const QString& id);

  /**
   * This contructor reads the id from the @p id attribute of the
   * QDomElement.
   *
   * @param node const reference to the QDomElement from which to
   *           obtain the id of the object
   * @param forceId flag to be able to suppress enforcement of an id
   *           defaults to true which requires the node to have an
   *           attribute with name @p id. If it does not contain such
   *           an attribute, an exception will be thrown. If @p forceId
   *           is false, no check for an id is performed. This will be
   *           used by objects, which are stored w/o id (eg. splits,
   *           transactions within schedules)
   */
  MyMoneyObject(const QDomElement& node, const bool forceId = true);

  void setId(const QString& id) /* __attribute__ ((deprecated)) */;

  /**
   * This method writes out the members contained in this object.
   */
  void writeBaseXML(QDomDocument& document, QDomElement& el) const;

protected:
  QString               m_id;
  static const QString  m_emptyId;
};

#endif

