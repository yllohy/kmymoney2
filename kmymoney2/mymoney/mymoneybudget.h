/***************************************************************************
                          mymoneybudget.h
                             -------------------
    begin                : Sun Jan 22 2006
    copyright            : (C) 2006 by Darren Gould
    email                : darren_gould@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYBUDGET_H
#define MYMONEYBUDGET_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes
#include <qmap.h>
#include <qvaluelist.h>
#include <qstring.h>
class QDomElement;
class QDomDocument;

// ----------------------------------------------------------------------------
// Project Includes
#include <kmymoney/mymoneyobject.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneytransactionfilter.h>
#include <kmymoney/export.h>

/**
  * This class defines a Budget within the MyMoneyEngine.  The Budget class
  * contains all the configuration parameters needed to run a Budget, plus
  * XML serialization.
  *
  * A Budget is a transactionfilter, so any Budget can specify which
  * transactions it's interested down to the most minute level of detail.
  * It extends the transactionfilter by providing identification (name,
  * comments, group type, etc) as well as layout information (what kind
  * of layout should be used, how the rows & columns should be presented,
  * currency converted, etc.)
  *
  * As noted above, this class only provides a Budget DEFINITION.  The
  * generation and presentation of the Budget itself are left to higher
  * level classes.
  *
  * @author Darren Gould <darren_gould@gmx.de>
  */

class KMYMONEY_EXPORT MyMoneyBudget: public MyMoneyObject, public MyMoneyTransactionFilter
{
public:
  MyMoneyBudget(void);
  ~MyMoneyBudget();
  MyMoneyBudget(const QString& _name);
  /**
    * This constructor creates an object based on the data found in the
    * QDomElement referenced by @p node. If problems arise, the @p id of
    * the object is cleared (see MyMoneyObject::clearId()).
    */
  MyMoneyBudget(const QDomElement& node);

  // Simple get operations
  const QString& name(void) const { return m_name; }
  QCString id(void) const { return m_id; }

  // Simple set operations
  void setName(const QString& _s) { m_name = _s; }


  /**
    * This method writes this Budget to the DOM element @p e,
    * within the DOM document @p doc.
    *
    * @param e The element which should be populated with info from this Budget
    * @param doc The document which we can use to create new sub-elements
    *              if needed
    * @param anonymous Whether the sensitive parts of the Budget should be
    *              masked
    */
  void write(QDomElement& e, QDomDocument *doc, bool anonymous=false) const;

  /**
    * This method reads a Budget from the DOM element @p e, and
    * populates this Budget with the results.
    *
    * @param e The element from which the Budget should be read
    *
    * @return bool True if a Budget was successfully loaded from the
    *    element @p e.  If false is returned, the contents of this Budget
    *    object are undefined.
    */
  bool read(const QDomElement& e);

  /**
    * This method creates a QDomElement for the @p document
    * under the parent node @p parent.  (This version overwrites the
    * MMObject base class.)
    *
    * @param document reference to QDomDocument
    * @param parent reference to QDomElement parent node
    */
  virtual void writeXML(QDomDocument& document, QDomElement& parent) const;

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  virtual bool hasReferenceTo(const QCString& id) const;

private:
  /**
    * The user-assigned name of the Budget
    */
  QString m_name;
};

#endif // MYMONEYBudget_H
