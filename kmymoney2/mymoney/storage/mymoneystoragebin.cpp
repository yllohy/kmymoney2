/***************************************************************************
                          imymoneystoragebin.cpp  -  description
                             -------------------
    begin                : Sun May 5 2002
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <qstringlist.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragebin.h"
#include "../mymoneyaccount.h"

#include <iostream>

unsigned int MyMoneyStorageBin::fileVersionRead;
unsigned int MyMoneyStorageBin::fileVersionWrite;

MyMoneyStorageBin::MyMoneyStorageBin()
{
  fileVersionWrite = VERSION_0_5_0;
}

MyMoneyStorageBin::~MyMoneyStorageBin()
{
}

unsigned int MyMoneyStorageBin::fileVersion(fileVersionDirectionType dir)
{
  switch(dir) {
    case Reading:
      return MyMoneyStorageBin::fileVersionRead;
    case Writing:
      return MyMoneyStorageBin::fileVersionWrite;
  }
  return 0;
}

void MyMoneyStorageBin::readStream(QDataStream& s, IMyMoneySerialize* storage)
{
  // process version and magic number to get the version information
  // files written by previous versions had the following layout:
  //
  //   QString with program version info
  //   Q_INT32 with magic code
  //
  // Newer files will use a sligthly different layout as follows:
  //
  //   QByteArray with the magic eight byte contents 'KMyMoney'
  //   Q_INT32 with file version information
  //

  // first read a four byte Q_INT32
  Q_INT32 len;
  QString prog_version("");

  s >> len;
  if(len < 30) {            // this seems to be a valid maximum length
    Q_UINT8 c, r;           // for a program version
    while(len) {
      s >> r;
      s >> c;
      prog_version += QChar(c, r);
      len -= 2;
    }
  } else {                  // check if it's the magic sequence
    // we've already read half of the magic code in, so we just
    // read the second half as well and see if it matches the new
    // magic code 'KMyMoney'
    Q_INT32 len1;
    s >> len1;
    if(len != MAGIC_0_50
    || len1 != MAGIC_0_51)
      throw new MYMONEYEXCEPTION("Unknown file type");
  }

  // next we read a Q_INT32 file version code
  fileVersionRead = 0;
  s >> fileVersionRead;
  if(fileVersionRead != VERSION_0_3_3
  && fileVersionRead != VERSION_0_4_0
  && fileVersionRead != VERSION_0_5_0)
    throw new MYMONEYEXCEPTION("Unknown file format");

  Q_INT32 tmp;
  s >> tmp;
  if (tmp==1)
    m_passwordProtected = true;
  else
    m_passwordProtected = false;
  s >> tmp;
  if (tmp==1)
    m_encrypted = true;
  else
    m_encrypted = false;

  // if (m_encrypted ==true) {
  //   All data that follows needs to be decrypted
  // }
  s >> m_password;

  // Simple Data
  if(fileVersionRead == VERSION_0_3_3) {
    qDebug("\nConverting from old 0.3.3 release\n\tRemoving old file::name field");
    QString temp_delete;
    s >> temp_delete;
  }

  switch(fileVersionRead) {
    case VERSION_0_3_3:
    case VERSION_0_4_0:
      readOldFormat(s, storage);
      break;
    case VERSION_0_5_0:
      // readNewFormat(s, storage);
      break;
  }
}

void MyMoneyStorageBin::addCategory(IMyMoneySerialize* storage,
                                    QMap<QString, QString>& categories,
                                    const QString& majorName,
                                    const QString& minorName,
                                    const MyMoneyAccount::accountTypeE type)
{
  MyMoneyAccount account;

  MyMoneyAccount group, majorCat;
  QString name;

  if (type==MyMoneyAccount::Expense) {
    group = storage->expense();
    majorCat.setAccountType(MyMoneyAccount::Expense);
  } else {
    group = storage->income();
    majorCat.setAccountType(MyMoneyAccount::Income);
  }

  name = majorName;
  if(!categories.contains(name)) {
    majorCat.setName(name);
    storage->newAccount(majorCat);
    storage->addAccount(group, majorCat);
    categories[name] = majorCat.id();
  }

  if(minorName != "") {
    name += ":" + minorName;
    if(!categories.contains(name)) {
      MyMoneyAccount minorCat;
      QString id = categories[majorName];
      QValueList<MyMoneyAccount> list = storage->accountList();
      QValueList<MyMoneyAccount>::Iterator it;
      for(it = list.begin(); it != list.end(); ++it) {
        if((*it).id() == id)
          break;
      }
      minorCat.setName(minorName);
      minorCat.setAccountType(majorCat.accountType());
      storage->newAccount(minorCat);
      storage->addAccount(*it, minorCat);
      categories[name] = minorCat.id();
    }
  }
}

void MyMoneyStorageBin::readOldFormat(QDataStream& s, IMyMoneySerialize* storage)
{
  QMap<QString, QString> categoryConversion;
  QMap<QString, QString> accountConversion;
  QValueList<MyMoneyTransaction> transactionList;

  QDate lastModificationDate;
  Q_INT32 cnt, inst_cnt, acc_cnt, tr_cnt;
  Q_INT32 tmp_int32;
  QString tmp;
  QDate date;

  s >> tmp; storage->setUserName(tmp);
  s >> tmp; storage->setUserStreet(tmp);
  s >> tmp; storage->setUserTown(tmp);
  s >> tmp; storage->setUserCounty(tmp);
  s >> tmp; storage->setUserPostcode(tmp);
  s >> tmp; storage->setUserTelephone(tmp);
  s >> tmp; storage->setUserEmail(tmp);
  s >> date; storage->setCreationDate(date);
  s >> date; // lastAccessDate not used anymore, so we skip it here
  s >> lastModificationDate;

  // process name list of institutions, the real list comes later ;-)
  Q_INT32 bankCount;
  s >> bankCount;
  while(bankCount--)
    s >> tmp;

  // read list of categories and convert it to accounts
  // in the expense and income group
  s >> cnt;
  for (int i=0; i < cnt; i++) {
    Q_INT32 inc;
    QString name;

    s >> inc;
    s >> name;

    addCategory(storage, categoryConversion, name, "",
           inc ? MyMoneyAccount::Income : MyMoneyAccount::Expense);

    Q_UINT32 minorCount;
    s >> minorCount;
    for (unsigned int j=0; j<minorCount; j++) {
      s >> tmp;
      addCategory(storage, categoryConversion, name, tmp,
           inc ? MyMoneyAccount::Income : MyMoneyAccount::Expense);
    }
  }

  // read list of payees
  MyMoneyPayee payee;
  s >> cnt;
  for (int i=0; i < cnt; i++) {
    s >> payee;
    // FIXME: Payee's are yet unknown !!!!! HELP HELP HELP
    // addPayee(payee.name(), payee.address(), payee.postcode(), payee.telephone(), payee.email());
  }

  // read list of institutions and create new objects for them
  // in the storage area
  s >> inst_cnt;
  for (int i=0; i < inst_cnt; i++) {
    MyMoneyInstitution institution;
    s >> tmp; institution.setName(tmp);
    s >> tmp; institution.setCity(tmp);
    s >> tmp; institution.setStreet(tmp);
    s >> tmp; institution.setPostcode(tmp);
    s >> tmp; institution.setTelephone(tmp);
    s >> tmp; institution.setManager(tmp);
    s >> tmp; institution.setSortcode(tmp);

    storage->addInstitution(institution);

    // now comes the hard part: the accounts and their transactions.
    // the old format stores the relevant transactions with the account
    // they belong to. Splits do not yet exist. Transfers are a little
    // tricky, as their counter-account might not yet exist and adding
    // such a transaction will cause an exception to be thrown when
    // added to the storage area. Therefore, we keep a list of all such
    // transactions locally and process it when all accounts are known.
    s >> acc_cnt;
    for(int j=0; j < acc_cnt; j++) {
      double tmp_d;
      MyMoneyAccount parent = storage->asset();
      MyMoneyAccount acc;
      acc.setAccountType(MyMoneyAccount::Checkings);
      s >> tmp; acc.setName(tmp);
      s >> tmp; acc.setDescription(tmp);
      s >> tmp; acc.setNumber(tmp);
      s >> tmp_int32; // type
      if(tmp_int32 != 0)
        qDebug("I thought, the old format only knew about checking accounts");

      if(fileVersionRead == VERSION_0_4_0) {
        s >> date; acc.setOpeningDate(date);
        s >> tmp_d; // opening balance
        acc.setOpeningBalance(MyMoneyMoney(static_cast<signed64>(tmp_d*100)));
      } else
        acc.setOpeningDate(QDate(1970,1,1));

      s >> date; acc.setLastReconciliationDate(date);

      storage->newAccount(acc);
      storage->addAccount(institution, acc);
      storage->addAccount(parent, acc);

      // keep the assignment info for later conversion of splits
      accountConversion[acc.name()] = acc.id();

      // now we come to read the transactions. as mentioned above, we
      // need to store them away until we have all the account information
      // For now, we store the names of the accounts
      s >> tr_cnt;
      for(int k=0; k < tr_cnt; ++k) {
        MyMoneyTransaction tr;
        MyMoneySplit sp1, sp2;
        MyMoneyMoney amount;
        QString category;
        Q_INT32 method;

        sp1.setAccountId(acc.id());
        s >> tmp_int32;   // id
        s >> tmp; // FIXME: sp1.setNumber(tmp);
        s >> tmp; // FIXME: payee must be stored

        // converting from a double to a MyMoneyMoney object is not that
        // trivial. I tried 1.23 to convert and ended up with 122.
        // I added a conversion to it. Does it need to be negative for
        // negative values? I am not sure.
        s >> tmp_d; amount = MyMoneyMoney(static_cast<signed64>(tmp_d*100+0.5));
        s >> date; tr.setPostDate(date);
        s >> method;      // method
        s >> category;    // major category
        s >> tmp;         // minor category
        if(tmp != "")
          category += ":" + tmp;
        // for now, we keep it as it is and convert sp2.account later on
        sp2.setAccountId(category);
        s >> tmp; // ATM bank name
        s >> tmp; // account from
        s >> tmp; // account to
        s >> tmp; sp1.setMemo(tmp); sp2.setMemo(tmp);
        s >> tmp_int32;  // state
        switch(tmp_int32) {
          case 0:
            sp1.setReconcileFlag(MyMoneySplit::Cleared);
            sp2.setReconcileFlag(MyMoneySplit::Cleared);
            break;
          case 1:
            sp1.setReconcileFlag(MyMoneySplit::Reconciled);
            sp2.setReconcileFlag(MyMoneySplit::Reconciled);
            sp1.setReconcileDate(tr.postDate());
            sp2.setReconcileDate(tr.postDate());
            break;
          default:
            sp1.setReconcileFlag(MyMoneySplit::NotReconciled);
            sp2.setReconcileFlag(MyMoneySplit::NotReconciled);
            break;
        }
        // depending on the method, the amount must be stored
        // as negative number for all transactions other than
        // 'deposits' (1). We correct that here and setup the
        // amount in the splits.
        if(method != 1) {
          amount = amount * (-1);
        }
        sp1.setShares(amount);
        sp1.setValue(amount);
        sp2.setShares(amount * (-1));
        sp2.setValue(amount * (-1));

        tr.addSplit(sp1);
        tr.addSplit(sp2);

        // we do not keep the transaction that is marked as
        // 'transfer' (2). The old format had two entries and we use
        // the one that is marked 'deposit' (1)
        if(method != 2)
          transactionList.append(tr);
      }
    }
  }

  // we have all institution, account and payee information in
  // the storage object in the new format. Now we need to convert
  // the categories to accounts for all the transactions we have
  // read and store them in the storage object.
  QValueList<MyMoneyTransaction>::Iterator it;
  for(it = transactionList.begin(); it != transactionList.end(); ++it) {
    MyMoneySplit sp = (*it).splits()[1];
    QString accname = sp.accountId();
    if(accname.left(1) == "<" && accname.right(1) == ">") {
      accname = accname.mid(1, accname.length()-2);
      sp.setAccountId(accountConversion[accname]);
    } else {
     	int colonindex = accname.find(":");
     	QString catmajor;
    	QString catminor;
      if(colonindex == -1) {
       	catmajor = accname;
    		catminor = "";
      }	else {
    		int len = accname.length();
    		len--;
       	catmajor = accname.left(colonindex);
    		catminor = accname.right(len - colonindex);
    	}
      addCategory(storage, categoryConversion, catmajor, catminor,
           sp.value() >= 0 ? MyMoneyAccount::Expense : MyMoneyAccount::Income);
      sp.setAccountId(categoryConversion[accname]);
    }
    (*it).modifySplit(sp);
    storage->addTransaction(*it, true);
  }

  storage->refreshAllAccountTransactionLists();
  storage->setLastModificationDate(lastModificationDate);

}
