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

  typedef enum {
    UnknownObject = 0,
    Account
  } MyMoneyObjectType;

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

  void setId(const QCString& id);

  /**
    * This method returns the runtime type of this object
    *
    * @sa setRtti()
    */
  const MyMoneyObjectType& rtti(void) const { return m_type; };

  /**
    * This method sets the runtime type of this object.
    *
    * @param type MyMoneyObjectType indication
    * @sa rtti
    */
  void setRtti(const MyMoneyObjectType& type);

  /**
    * This method clears the id of the object
    */
  void clearId(void);

  const bool operator == (const MyMoneyObject& right) const;

private:
  QCString               m_id;
  MyMoneyObjectType      m_type;
};
#endif

