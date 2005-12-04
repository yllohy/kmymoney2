/***************************************************************************
                          knewaccountwizard.h  -  description
                             -------------------
    begin                : Thu Jul 4 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

#ifndef KNEWACCOUNTWIZARD_H
#define KNEWACCOUNTWIZARD_H


// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../dialogs/knewaccountwizarddecl.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneyscheduled.h"
#include "../mymoney/mymoneyinstitution.h"
#include "../views/kbanklistitem.h"

/**
  *@author Thomas Baumgart
  */

class KNewAccountWizard : public KNewAccountWizardDecl  {
   Q_OBJECT
public:
  KNewAccountWizard(QWidget *parent=0, const char *name=0);
  ~KNewAccountWizard();

  /**
    * This method returns the account information entered by the user
    *
    * @return MyMoneyAccount filled with information by the user
    */
  const MyMoneyAccount& account(void) const { return m_account; };

  /**
    * This method returns the account information about a brokerage
    * account entered by the user. The value returned only makes sense,
    * if account() returned an account of type investment. If the returned
    * value does not contain a name then the user selected not to create
    * a corresponding brokerage account to the investment account.
    *
    * @return MyMoneyAccount filled with information by the user
    */
  const MyMoneyAccount& brokerageAccount(void) const { return m_brokerage; };

  /**
    * This method returns the parent account selected that is appropriate
    * to the account type selected by the user
    *
    * @return MyMoneyAccount with information about standard account
    */
  const MyMoneyAccount& parentAccount(void) const { return m_parent; };

  /**
    * This method is used to preset the name of the account in the wizard
    *
    * @param accountName name of the account
    */
  void setAccountName(const QString& accountName);

  /**
    * This method is used to preset the opening balance of the account
    *
    * @param balance opening balance for the account
    */
  void setOpeningBalance(const MyMoneyMoney& balance);

  /**
    * This method is used to retrieve the opening balance entered
    * by the user during data collection for the new acocunt.
    *
    * @return MyMoneyMoney object containing the opening balance.
    *
    * @note The date for the opening balance transaction can be retrieved
    *       from the account data returned by account().
    */
  MyMoneyMoney openingBalance(void) const;

  /**
    * This method is used to preset the opening date of the account
    *
    * @param date opening date for the account
    */
  void setOpeningDate(const QDate& date);

  /**
    * This method is used to preset the account type of the account
    *
    * @param type account type as specified in MyMoneyAccount
    */
  void setAccountType(const MyMoneyAccount::accountTypeE type);

  /**
    * Get the schedule.
    *
    * If the schedule has not been created name() should be empty.
    */
  MyMoneySchedule schedule(void) const { return m_schedule; }

  /**
    * This method is used to preset the institution.
    *
    * @param institution The institution
    */
  void setInstitution(const MyMoneyInstitution& institution) { m_institution = institution; }

protected:
  /**
    * This method is used to reload the institution combo box
    * with all institution names found in the engine
    */
  void loadInstitutionList(void);

  /**
    * This method is used to load the account type combo box
    * with all available account types
    */
  void loadAccountTypes(void);

  /**
    * Pre-filter showPage so we can select the account type
    */
  void showPage(QWidget* page);

public slots:
  int exec();
  void next();
  void accept();

protected slots:
  void slotNewInstitution(void);
  void slotAccountType(const QString& sel);
  void slotCheckPageFinished(void);
  void slotNewPayee(const QString&);
  void slotCurrencyChanged(int);
  void slotPriceUpdate(void);

  void help(void);

signals:
  void newInstitutionClicked();
  /**
    * This signal is emitted, when a new category name has been
    * entered by the user and this name is not known as account
    * by the MyMoneyFile object.
    * Before the signal is emitted, a MyMoneyAccount is constructed
    * by this object and filled with the desired name. All other members
    * of MyMoneyAccount will remain in their default state. Upon return,
    * the connected slot should have created the object in the MyMoneyFile
    * engine and filled the member @p id.
    *
    * @param acc reference to MyMoneyAccount object that caries the name
    *            and will return information about the created category.
    */
  void newCategory(MyMoneyAccount& acc);

private:
  void loadSubAccountList(KAccountListItem* parent, const QCString& accountId);
  void loadSubAccountList(KListView* parent, const QCString& accountId);
  QValueList<MyMoneyAccount>::ConstIterator findAccount(const QCString& accountId) const;
  void loadAccountList(void);
  void loadPaymentMethods();

private:
  MyMoneyAccount::accountTypeE m_accountType;
  QValueList<MyMoneyAccount> m_accountList;
  QString m_accountPaymentPageTitle;
  MyMoneyAccount m_account;
  MyMoneyAccount m_parent;
  MyMoneyAccount m_brokerage;
  MyMoneySchedule m_schedule;
  MyMoneyInstitution m_institution;
  QMap<QWidget*, QString>         m_helpAnchor;
};

#endif
