/***************************************************************************
                          kmymoneyutils.h  -  description
                             -------------------
    begin                : Wed Feb 5 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYUTILS_H
#define KMYMONEYUTILS_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Headers
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/mymoneyaccount.h"
#include "mymoney/mymoneyscheduled.h"
#include "widgets/kmymoneypayee.h"

/**
  * @author Thomas Baumgart
  */

class KMyMoneyUtils
{
public:
  enum categoryTypeE {
    none =       0x00,
    liability =  0x01,
    asset =      0x02,
    expense =    0x04,
    income =     0x08,
    last =       0x10
  };

  static const int maxHomePageItems = 3;
  
  KMyMoneyUtils();
  ~KMyMoneyUtils();

  /**
    * This method is used to convert the internal representation of
    * an account type into a human readable format
    *
    * @param accountType numerical representation of the account type.
                         For possible values, see MyMoneyAccount::accountTypeE
    * @return QString representing the human readable form
    */
  static const QString accountTypeToString(const MyMoneyAccount::accountTypeE accountType);

  /**
    * This method is used to convert an account type from it's
    * string form to the internal used numeric value.
    *
    * @param accountType reference to a QString containing the string to convert
    * @return accountTypeE containing the internal used numeric value. For possible
    *         values see MyMoneyAccount::accountTypeE
    */
  static const MyMoneyAccount::accountTypeE stringToAccountType(const QString& type);

  /**
    * This method is used to convert the occurence type from it's
    * internal representation into a human readable format.
    *
    * @param occurence numerical representation of the MyMoneySchedule
    *                  occurence type
    *
    * @return QString representing the human readable format
    */
  static const QString occurenceToString(const MyMoneySchedule::occurenceE occurence);

  /**
    * This method is used to convert the occurence type from the
    * human readable form into it's internal representation.
    *
    * @param text reference to QString representing the human readable format
    * @return numerical representation of the occurence
    */
  static const MyMoneySchedule::occurenceE stringToOccurence(const QString& text);
  
  /**
    * This method is used to convert the payment type from it's
    * internal representation into a human readable format.
    *
    * @param occurence numerical representation of the MyMoneySchedule
    *                  payment type
    *
    * @return QString representing the human readable format
    */
  static const QString paymentMethodToString(MyMoneySchedule::paymentTypeE paymentType);

  /**
    * This method is used to convert the schedule type from it's
    * internal representation into a human readable format.
    *
    * @param occurence numerical representation of the MyMoneySchedule
    *                  schedule type
    *
    * @return QString representing the human readable format
    */
  static const QString scheduleTypeToString(MyMoneySchedule::typeE type);

  /**
    * This method is used to convert a numeric index of an item
    * represented on the home page into it's string form.
    *
    * @param idx numeric index of item
    *
    * @return QString with text of this item
    */
  static const QString homePageItemToString(const int i);

  /**
    * This method is used to convert the name of a home page item
    * to it's internal numerical representation
    *
    * @param txt QString reference of the items name
    *
    * @retval 0 @p txt is unknown
    * @retval >0 numeric value for @p txt
    */
  static const int stringToHomePageItem(const QString& txt);

  /**
    * This method is used to add home page items to a list that
    * does not contain all of them. Missing items will be added
    * at the end as positive values turning the options on.
    *
    * @param list reference to QStringList containing the option list
    */
  static void addDefaultHomePageItems(QStringList& list);

  /**
    * This methods ask the user if a new payee should be added to the
    * storage object. The widget is updated.
    *
    * @param parent pointer to the widget which should be the parent widget
    *               for message boxes etc.
    * @param payeeEdit pointer to the payee edit widget
    * @param payeeName new name of the payee.
    *
    * Usually, one would connect the kMyMoneyPayee::newPayee() signal to a slot
    * in a view or dialog which call this helper method. Here's an example:
    *
    * @code
    *   :
    *   // this could be somewhere in the constructor
    *   connect(m_payeeEdit, SIGNAL(newPayee(const QString&)), this, SLOT(slotNewPayee(const QString&)));
    *   :
    *   :
    * xxx::slotNewPayee(const QString& payeeName)
    * {
    *   KMyMoneyUtils::newPayee(this, m_payeeEdit, payeeName); 
    * }
    *
    * @endcode
    */
  static void newPayee(QWidget* parent, kMyMoneyPayee* payeeEdit, const QString& payeeName);

  /**
    * Retrieve a KDE KGuiItem for the split button
    *
    * @param none
    * @return The KGuiItem that can be used to display the icon and text for a split button.
  **/
  static KGuiItem splitGuiItem(void);

  /**
    * Retrieve a KDE KGuiItem for the new schedule button.
    *
    * @param none
    * @return The KGuiItem that can be used to display the icon and text
  **/
  static KGuiItem scheduleNewGuiItem(void);

  /**
    * Retrieve a KDE KGuiItem for the account filter button
    *
    * @param none
    * @return The KGuiItem that can be used to display the icon and text
  **/
  static KGuiItem accountsFilterGuiItem(void);

  /**
    * This method adds the file extension passed as argument @p extension
    * to the end of the file name passed as argument @name if it is not present.
    * If @p name contains an extension it will be removed.
    *
    * @param str filename to be checked
    * @param extension extension to be added (w/o the dot)
    *
    * @retval true if @p name was changed
    * @retval false if @p name remained unchanged
    */
  static bool appendCorrectFileExt(QString& name, const QString& extension);

  static QPixmap billScheduleIcon(int size);
  static QPixmap depositScheduleIcon(int size);
  static QPixmap transferScheduleIcon(int size);
  static QPixmap scheduleIcon(int size);

  /**
    * This method is used to convert a MyMoneySchedule occurence period into
    * the frequency used in the MyMoneyFinancialCalculator.
    *
    * @param occurence MyMoneySchedule::occurenceE type occurence of a payment
    * @return int reflecting the payment frequency in days
    */
  static int occurenceToFrequency(const MyMoneySchedule::occurenceE occurence);
  
};

#endif
