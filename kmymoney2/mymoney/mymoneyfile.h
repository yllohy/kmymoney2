/***************************************************************************
                          mymoneyfile.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYFILE_H
#define MYMONEYFILE_H

#include <qstring.h>
#include <qlist.h>
#include <qdatetime.h>
#include <stdio.h>
#include <qstringlist.h>
#include "mymoneybank.h"
#include "mymoneypayee.h"
#include "mymoneycategory.h"

/**
  * This class is used by the GUI to interrogate the transaction engine.  It is not
  * library independant at the moment and still relies upon QT for many of it's
  * attributes and method calls.  Eventually this code will be separated from QT
  * and could be usable from other GUI toolkits such as gtk as used by the GNOME
  * desktop environment.
  *
  * TODO: Remove all QT dependance e.g QString, QList etc.
  *
  * @see MyMoneyBank
  *
  * @author Michael Edwardes 2000-2001
  * $Id: mymoneyfile.h,v 1.4 2001/06/16 21:12:46 mte Exp $
  *
  * @short A representation of the file format used by KMyMoney2.
**/
class MyMoneyFile {
private:
  // File 'fields'
  QString m_userName;
  QString m_userStreet;
  QString m_userTown;
  QString m_userCounty;
  QString m_userPostcode;
  QString m_userTelephone;
  QString m_userEmail;
  QDate m_createdDate;
  QDate m_lastAccess;
  QDate m_lastModify;
  bool m_dirty;
  bool m_passwordProtected;  // Not used yet
  bool m_encrypted;  // Not used yet
  QString m_password;  // Not used yet

  // 'Helper' variables and methods
  bool m_initialised;
  bool m_containsBanks, m_containsAccounts, m_containsTransactions;

public:
  void init(void);
  bool isInitialised(void);
  bool containsBanks(void);
  bool containsAccounts(void);
  bool containsTransactions(void);

private:
  // A list containing the banks
  QList<MyMoneyBank> m_banks;

  // A list containing the list of banks that ATM knows about
  // Not used yet.
  QStringList m_bankNames;

  // A list containing all the payees that have been used
  QList<MyMoneyPayee> m_payeeList;

  // A list containing all the categories that have been used.
  // When a new file is created the list is read in from
  // <kde2dir>/share/apps/kmymoney2/default_categories.dat
  QList<MyMoneyCategory> m_categoryList;
  bool findBankPosition(const MyMoneyBank& bank, unsigned int&);

public:
  /**
    * Standard constructor.  Just sets up some default values.
  **/
  MyMoneyFile();

  /**
    * This is the constructor usually used.  The passed parameters are used to
    * initialise the attributes of this class.
    *
    * @param username The users name.
    * @param userStreet Part of the users address.
    * @param userTown Part of the users address.
    * @param userCounty Part of the users address.
    * @param userPostcode Part of the users address.
    * @param userTelephone The users telephone number.
    * @param userEmail The users email address.
    * @param createDate TODO: remove this and use QDate::currentDate() in the constructor.
  **/
  MyMoneyFile(const QString& username, const QString& userStreet,
    const QString& userTown, const QString& userCounty, const QString& userPostcode, const QString& userTelephone,
    const QString& userEmail, const QDate& createDate);

  /**
    * Standard destructor.
  **/
  ~MyMoneyFile();

  /**
    * Adds a bank to the underlying data structures.  The parameters passed are used
    * to create the bank which is then added.
    *
    * @param name The banks name.
    * @param sortCode The banks sort code.
    * @param city Part of the banks address.
    * @param street Part of the banks address.
    * @param postcode Part of the banks address.
    * @param telephone The banks telephone number.
    * @param manager The bank managers name.
    *
    * @see MyMoneyBank
  **/
  bool addBank(const QString& name, const QString& sortCode, const QString& city, const QString& street,
    const QString& postcode, const QString& telephone, const QString& manager);

  /**
    * Finds the bank in the list that matches the supplied argument.
    *
    * @param bank The bank to look for.
    *
    * @return A pointer to the bank if found, O otherwise.
    *
    * @see MyMoneyBank
  **/
  MyMoneyBank* bank(const MyMoneyBank& bank);

  /**
    * Returns the first bank stored.  Typically used in for
    * statements as follows:
    * <pre> MyMoneyBank *bank;
    * for (bank=bankFirst(); bank; bank=bankNext()) {
    *   // Do something with the bank
    * }</pre>
    *
    * @return The first bank in the list or 0 if no banks are present.
    *
    * @see MyMoneyBank
  **/
  MyMoneyBank* bankFirst(void);

  /**
    * Returns the 'next' bank stored.  Typically used in for
    * statements as follows:
    * <pre> MyMoneyBank *bank;
    * for (bank=bankFirst(); bank; bank=bankNext()) {
    *   // Do something with the bank
    * }</pre>
    *
    * @return The next bank in the list or 0 if no banks are present or there is no next.
    *
    * @see MyMoneyBank
  **/
  MyMoneyBank* bankNext(void);

  /**
    * Returns the first bank stored.  Typically used in while
    * statements as follows:
    * <pre> MyMoneyBank *bank = bankFirst();
    * while (bank != bankLast()) {
    *   // Do something with the bank
    * }</pre>
    *
    * @return The first bank in the list or 0 if no banks are present.
    *
    * @see MyMoneyBank
  **/
  MyMoneyBank* bankLast(void);

  /**
    * Returns the number of banks held in the file.
    *
    * @return The bank count.
    *
    * @see MyMoneyBank
  **/
  unsigned int bankCount(void);

  /**
    * Removes a bank from the list that matches the parameter.
    *
    * @param bank A copy of the bank to remove.
    *
    * @return Whether the remove was succesfull.
    *
    * @see MyMoneyBank
  **/
  bool removeBank(const MyMoneyBank& bank);


  QString userAddress(void);
  QString userName(void) { return m_userName; }
  QString userStreet(void) { return m_userStreet; }
  QString userTown(void) { return m_userTown; }
  QString userCounty(void) { return m_userCounty; }
  QString userPostcode(void) { return m_userPostcode; }
  QString userTelephone(void) { return m_userTelephone; }
  QString userEmail(void) { return m_userEmail; }
  QDate createdDate(void) { return m_createdDate; }
  QDate lastAccessDate(void) { return m_lastAccess; }
  QDate lastModifyDate(void) { return m_lastModify; }

  // Simple set operations
  void set_userName(const QString& val) { m_userName = val; }
  void set_userStreet(const QString& val) { m_userStreet = val; }
  void set_userTown(const QString& val) { m_userTown = val; }
  void set_userCounty(const QString& val) { m_userCounty = val; }
  void set_userPostcode(const QString& val) { m_userPostcode = val; }
  void set_userTelephone(const QString& val) { m_userTelephone = val; }
  void set_userEmail(const QString& val) { m_userEmail = val; }
  void setCreateDate(const QDate& val) { m_createdDate = val; }

  // File reading/saving code
  int saveAllData(const QString& fileName);
  int readAllData(const QString& fileName);
  void resetAllData(void);

  void addBankName(const QString& val);

  // Category operations
  void addMajorCategory(const bool income, const QString& val);
  void addMinorCategory(const bool income, const QString& major, const QString& minor);
  void addCategory(const bool income, const QString& major, QStringList& minors);
  void addCategory(const bool income, const QString& major, const QString& minor);
  QList<MyMoneyCategory> categoryList(void) { return m_categoryList; }
  QListIterator<MyMoneyCategory> categoryIterator(void);
  void removeMajorCategory(const QString& major);
  void removeMinorCategory(const QString& major, const QString& minor);
  void renameMajor(const QString& oldName, const QString& newName);
  void renameMinor(const QString& major, const QString& oldName, const QString& newName);

  // Internal modified flag
  bool dirty(void) { return m_dirty; }
  void setDirty(bool dirty) { m_dirty = dirty; }

  // Payee operations
  void addPayee(const QString& newPayee, const QString address=QString::null, const QString postcode=QString::null, const QString telephone=QString::null, const QString email=QString::null);
  QListIterator<MyMoneyPayee> payeeIterator(void);

  // Copy constructors
  MyMoneyFile(const MyMoneyFile&);
  MyMoneyFile& operator = (const MyMoneyFile&);
};

#endif
