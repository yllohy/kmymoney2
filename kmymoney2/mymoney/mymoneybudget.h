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
#include <kmymoney/mymoneymoney.h>
#include <kmymoney/export.h>

/**
  * This class defines a Budget within the MyMoneyEngine.  The Budget class
  * contains all the configuration parameters needed to run a Budget, plus
  * XML serialization.
  *
  * As noted above, this class only provides a Budget DEFINITION.  The
  * generation and presentation of the Budget itself are left to higher
  * level classes.
  *
  * @author Darren Gould <darren_gould@gmx.de>
  */
class KMYMONEY_EXPORT MyMoneyBudget: public MyMoneyObject
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

  /**
    * This constructor creates an object based on the data found in the
    * MyMoneyBudget budget object.
    */
  MyMoneyBudget(const QCString& id, const MyMoneyBudget& budget);

  /**
    * Helper class for MyMoneyBudget
    *
    * This is an abstraction of the PERIOD stored in the BUDGET/ACCOUNT tag in XML
    *
    * @author Darren Gould
    */
  class PeriodGroup
  {
  public:
    // get functions
    const QDate&    startDate ( void ) const { return m_start; }
    const MyMoneyMoney& amount( void ) const { return m_amount; }

    // set functions
    void setStartDate  ( const QDate& _start )    { m_start  = _start; }
    void setAmount( const MyMoneyMoney& _amount ) { m_amount = _amount;}

    const bool operator == (const PeriodGroup &r) const { return (m_start == r.m_start && m_amount == r.m_amount); }

 private:
    QDate         m_start;
    MyMoneyMoney  m_amount;
  };

  /**
    * Helper class for MyMoneyBudget
    *
    * This is an abstraction of the Account Data stored in the BUDGET tag in XML
    *
    * @author Darren Gould
    */
  class AccountGroup
  {
  public:
    typedef enum
    {
      eNone = 0,
      eMonthly,
      eMonthByMonth,
      eYearly,
      eMax
    } eBudgetLevel;

    static const QStringList kBudgetLevelText;

  public:
    AccountGroup() : m_budgetlevel(eNone), m_budgetsubaccounts(false) {}

    // get functions
    const QCString& id( void ) const { return m_id; }
    const QCString& parentId( void ) const { return m_parentId; }
    bool budgetSubaccounts( void ) const { return m_budgetsubaccounts; }
    eBudgetLevel budgetLevel( void ) const { return m_budgetlevel; }
    const PeriodGroup& period( const QDate &_date ) const { return m_periods[_date]; }
    const QMap<QDate, PeriodGroup>& getPeriods( void ) const { return m_periods; }
    void clearPeriods(void) { m_periods.clear(); }
    const MyMoneyMoney balance( void ) const
    {
      MyMoneyMoney balance;

      QMap<QDate, PeriodGroup>::const_iterator it;
      for(it = m_periods.begin(); it != m_periods.end(); ++it)
      {
        balance += (*it).amount();
      }
      return balance;
    };

    const MyMoneyMoney totalBalance(void) const
    {
      MyMoneyMoney bal = balance();
      switch(m_budgetlevel) {
        default:
          break;
        case eMonthly:
          bal = bal * 12;
          break;
      }
      return bal;
    }

    // set functions
    void setId( QString _id ) { m_id = _id; }
    void setParentId( const QString& _id ) { m_parentId = _id; }
    void setBudgetLevel( eBudgetLevel _level ) { m_budgetlevel = _level; }
    void setBudgetSubaccounts( bool _b ) { m_budgetsubaccounts = _b; }
    void addPeriod( const QDate& _date, PeriodGroup &period ) { m_periods[_date] = period; }

    const bool operator == (const AccountGroup &r) const;

    bool isZero(void) const;

  private:
    QCString m_id;
    QCString m_parentId;

    eBudgetLevel             m_budgetlevel;
    bool                     m_budgetsubaccounts;
    QMap<QDate, PeriodGroup> m_periods;
  };

  /**
   * This operator tests for equality of two MyMoneyBudget objects
   */
  const bool operator == (const MyMoneyBudget &) const;


  // Simple get operations
  const QString& name(void) const { return m_name; }
  const QDate& budgetStart(void) const { return m_start; }
  QCString id(void) const { return m_id; }
  const AccountGroup & account(const QCString _id) const;
  bool contains(const QCString _id) const { return m_accounts.contains(_id); }
  QValueList<AccountGroup> getaccounts(void) const { return m_accounts.values(); }

  // Simple set operations
  void setName(const QString& _name) { m_name = _name; }
  void setBudgetStart(const QDate& _start);
  void setAccount(const AccountGroup &_account, const QCString _id);

  /**
    * This method writes this Budget to the DOM element @p e,
    * within the DOM document @p doc.
    *
    * @param e The element which should be populated with info from this Budget
    * @param doc The document which we can use to create new sub-elements
    *              if needed
    */
  void write(QDomElement& e, QDomDocument *doc) const;

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
    * parameter @p id and the balance() returned is zero.
    * If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  virtual bool hasReferenceTo(const QCString& id) const;

  /**
    * This member removes all references to object identified by @p id. Used
    * to remove objects which are about to be removed from the engine.
    */
  void removeReference(const QCString& id);

private:
  /**
    * The user-assigned name of the Budget
    */
  QString m_name;

  /**
    * The user-assigned year of the Budget
    */
  QDate m_start;

  /**
    * Map the budgeted accounts
    *
    * Each account Id is stored against the AccountGroup information
    */
  QMap<QCString, AccountGroup> m_accounts;
};

#endif // MYMONEYBudget_H
// vim:cin:si:ai:et:ts=2:sw=2:
