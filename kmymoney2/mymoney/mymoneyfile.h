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

// This class is the interface to the GUI and contains code to retrieve
// it's contents.
class MyMoneyFile {
private:
  // File 'fields'
  QString m_moneyName;
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
  bool m_passwordProtected;
  bool m_encrypted;
  QString m_password;

  // 'Helper' variables and methods
  bool m_initialised;
  bool m_containsBanks, m_containsAccounts, m_containsTransactions;

  // Iterator helper

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
  MyMoneyFile();
  MyMoneyFile(const QString& szname, const QString& usern, const QString& userStreet,
    const QString& userTown, const QString& userCounty, const QString& userPostcode, const QString& userTelephone,
    const QString& userEmail, const QDate& createDate);
    ~MyMoneyFile();

	// Bank operations
  bool addBank(const QString& name, const QString& sortCode, const QString& city, const QString& street,
    const QString& postcode, const QString& telephone, const QString& manager);
  MyMoneyBank* bank(const MyMoneyBank& bank);
  MyMoneyBank* bankFirst(void);
  MyMoneyBank* bankNext(void);
  MyMoneyBank* bankLast(void);
  unsigned int bankCount(void);
  bool removeBank(const MyMoneyBank& bank);

private:
//  bool addAccount(const QString& name, const QString& number, MyMoneyAccount::accountTypeE type,
//    const QString& description, const QDate& lastReconcile, const MyMoneyBank& bank);
//  bool addTransaction(const long id, MyMoneyTransaction::transactionMethod methodType, const QString& number, const QString& memo,
//                     const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
//                     const QString& fromTo, const QString& bankFrom, const QString& bankTo, MyMoneyTransaction::stateE state,
//                     const MyMoneyBank& bank, const MyMoneyAccount& account);

public:
  // Simple get operations
  QString userAddress(void);
  QString name(void) const { return m_moneyName; }
  QString moneyName(void) { return m_moneyName; }
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
  void set_moneyName(const QString& val) { m_moneyName = val; }
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
