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
#include <qiodevice.h>
#include <qregexp.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragebin.h"
#include "../mymoneyaccount.h"

unsigned int MyMoneyStorageBin::fileVersionRead;
unsigned int MyMoneyStorageBin::fileVersionWrite;

MyMoneyStorageBin::MyMoneyStorageBin()
{
  fileVersionWrite = MAX_FILE_VERSION;
  m_progressCallback = 0;
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

void MyMoneyStorageBin::readFile(QIODevice* qfile, IMyMoneySerialize* storage)
{
  QDataStream s(qfile);
  readStream(s, storage);
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

  // see, if we know how to handle this stuff
  if(fileVersionRead < MIN_FILE_VERSION || fileVersionRead > MAX_FILE_VERSION) {
    // make sure it's not one of the old 0.4 formats
    if(fileVersionRead != VERSION_0_3_3
    && fileVersionRead != VERSION_0_4_0) {
      throw new MYMONEYEXCEPTION("Unknown file format");
    }
  }

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

    default:
      readNewFormat(s, storage);
      break;
  }

  // make sure the progress bar is not shown any longer
  signalProgress(-1, -1);
}

void MyMoneyStorageBin::addCategory(IMyMoneySerialize* storage,
                                    QMap<QString, QCString>& categories,
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
      QCString id = categories[majorName];
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
  QMap<QString, QCString> categoryConversion;
  QMap<QString, QCString> accountConversion;
  QMap<QString, QCString> payeeConversion;
  QValueList<MyMoneyTransaction> transactionList;

  QDate lastModificationDate;
  Q_INT32 cnt, inst_cnt, acc_cnt, tr_cnt;
  Q_INT32 tmp_int32;
  QString tmp;
  QDate date;

  // the old format stored MyMoneyMoney objects in 4 bytes
  MyMoneyMoney::setFileVersion(MyMoneyMoney::FILE_4_BYTE_VALUE);

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
  s >> cnt;
  for (int i=0; i < cnt; i++) {
    MyMoneyPayee payee;
    s >> tmp; payee.setName(tmp);
    s >> tmp; // address
    s >> tmp; // postcode
    s >> tmp; // telephone
    s >> tmp; // email

    storage->addPayee(payee);
    payeeConversion[payee.name()] = payee.id();
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
      switch(tmp_int32) {
        case 1:       // Savings
          acc.setAccountType(MyMoneyAccount::Savings);
          break;

        case 2:       // Current/Checkings
          // is already set as default
          break;

        default:
          qDebug("I thought, the old format only knew about savings and checking accounts");
          break;
      }

      if(fileVersionRead == VERSION_0_4_0) {
        s >> date; acc.setOpeningDate(date);
        s >> tmp_d; // opening balance
        acc.setOpeningBalance(tmp_d);
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
      signalProgress(0, tr_cnt);
      for(int k=0; k < tr_cnt; ++k) {
        MyMoneyTransaction tr;
        MyMoneySplit sp1, sp2;
        MyMoneyMoney amount;
        QString category;
        QString payee;
        QCString action;
        Q_INT32 method;

        sp1.setAccountId(acc.id());
        s >> tmp_int32;   // id
        s >> tmp; sp1.setNumber(tmp);
        s >> payee;

        // converting from a double to a MyMoneyMoney object is not that
        // trivial. I tried 1.23 to convert and ended up with 122.
        // I added a conversion to it. Does it need to be negative for
        // negative values? I am not sure.
        s >> tmp_d; amount = MyMoneyMoney(tmp_d);
        s >> date; tr.setPostDate(date);
        s >> method;
        switch(method) {
          default:
            qDebug("Unknown transaction method %d", method);
            // tricky fall through here, so that the default
            // is cheque
          case 0:         // cheque
            action = MyMoneySplit::ActionCheck;
            break;
          case 1:
            action = MyMoneySplit::ActionDeposit;
            break;
          case 2:
            action = MyMoneySplit::ActionTransfer;
            break;
          case 3:
            action = MyMoneySplit::ActionWithdrawal;
            break;
          case 4:
            action = MyMoneySplit::ActionATM;
            break;
        }
        sp1.setAction(action);

        s >> category;    // major category
        s >> tmp;         // minor category
        if(tmp != "")
          category += ":" + tmp;
        // for now, we keep it as it is and convert sp2.account later on
        sp2.setAccountId(static_cast<QCString> (category));
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
          amount = -amount;
        }
        sp1.setShares(amount);
        sp1.setValue(amount);
        sp2.setShares(-amount);
        sp2.setValue(-amount);

        QMap<QString, QCString>::ConstIterator it_p;
        it_p = payeeConversion.find(payee);
        if(it_p != payeeConversion.end()) {
          sp1.setPayeeId(*it_p);
          sp2.setPayeeId(*it_p);
        }

        tr.addSplit(sp1);
        tr.addSplit(sp2);

        // we do not keep the transaction that is marked as
        // 'transfer' (2). The old format had two entries and we use
        // the one that is marked 'deposit' (1)
        if(method != 2)
          transactionList.append(tr);

        signalProgress(k, 0);
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
      } else {
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

  storage->setLastModificationDate(lastModificationDate);

}

void MyMoneyStorageBin::readNewFormat(QDataStream&s, IMyMoneySerialize* storage)
{
  QString tmp_s;
  QDate date;
  Q_INT32 tmp;

  // setup MyMoneyMoney objects to read the correct amount of data bytes
  if(fileVersionRead >= VERSION_0_51) {
    MyMoneyMoney::setFileVersion(MyMoneyMoney::FILE_8_BYTE_VALUE);
  } else {
    qDebug("Use 4 byte MyMoneyMoney reader");
    MyMoneyMoney::setFileVersion(MyMoneyMoney::FILE_4_BYTE_VALUE);
  }

  s >> tmp_s; storage->setUserName(tmp_s);
  s >> tmp_s; storage->setUserStreet(tmp_s);
  s >> tmp_s; storage->setUserTown(tmp_s);
  s >> tmp_s; storage->setUserCounty(tmp_s);
  s >> tmp_s; storage->setUserPostcode(tmp_s);
  s >> tmp_s; storage->setUserTelephone(tmp_s);
  s >> tmp_s; storage->setUserEmail(tmp_s);
  s >> date; storage->setCreationDate(date);
  s >> date; storage->setLastModificationDate(date);    // this resets the dirty flag

  s >> tmp;     // estimated number of items that follow

  readInstitutions(s, storage);
  readPayees(s, storage);
  readAccounts(s, storage);

  QValueList<MyMoneyAccount> list = storage->accountList();
  QValueList<MyMoneyAccount>::ConstIterator it;
  m_accountList.clear();
  for(it = list.begin(); it != list.end(); ++it) {
    m_accountList[(*it).id()] = *it;
  }
  list.clear();

  readTransactions(s, storage);

  if(s.atEnd())
    return;

  // for now, we don't read the scheduled transactions back, as there
  // might be files out there that don't have it.
  readScheduledTransactions(s, storage);
  if(s.atEnd())
    return;

  storage->setPairs(readKeyValueContainer(s));
}

void MyMoneyStorageBin::writeFile(QIODevice* qfile, IMyMoneySerialize* storage)
{
  QDataStream s(qfile);
  writeStream(s, storage);
}

void MyMoneyStorageBin::writeStream(QDataStream& s, IMyMoneySerialize* storage)
{
  Q_INT32 magic;
  QString pwd;

  // write header
  magic = MAGIC_0_50;
  s << magic;
  magic = MAGIC_0_51;
  s << magic;
  magic = fileVersionWrite;
  s << magic;

  if(fileVersionWrite >= VERSION_0_51) {
    // make sure that from now on we write MyMoneyMoney objects with 64 bytes
    // we signal that to the reader by using VERSION_0_51 instead of VERSION_0_50
    MyMoneyMoney::setFileVersion(MyMoneyMoney::FILE_8_BYTE_VALUE);
  } else {
    qDebug("Using old 4 byte writer for MyMoneyMoney objects");
    MyMoneyMoney::setFileVersion(MyMoneyMoney::FILE_4_BYTE_VALUE);
  }

  Q_INT32 tmp;
  tmp = 0;
  s << tmp;       // no password
  s << tmp;       // not encrypted

  s << pwd;

  // so much for the common header, now get the specific 0.5 stuff

  s << storage->userName();
  s << storage->userStreet();
  s << storage->userTown();
  s << storage->userCounty();
  s << storage->userPostcode();
  s << storage->userTelephone();
  s << storage->userEmail();
  s << storage->creationDate();
  s << storage->lastModificationDate();

  // leave an estimate on the number of items that follow
  s << storage->institutionList().count() +
       storage->payeeList().count() +
       storage->accountList().count() +
       storage->transactionList().count() +
       storage->scheduleList().count();

  writeInstitutions(s, storage);
  writePayees(s, storage);
  writeAccounts(s, storage);
  writeTransactions(s, storage);
  writeKeyValueContainer(s, storage->pairs());
  writeScheduledTransactions(s, storage);

  // this seems to be nonsense, but it clears the dirty flag
  // as a side-effect.
  storage->setLastModificationDate(storage->lastModificationDate());

  // make sure the progress bar is not shown any longer
  signalProgress(-1, -1);
}

void MyMoneyStorageBin::writeInstitutions(QDataStream& s, IMyMoneySerialize* storage)
{
  Q_INT32 tmp;
  QValueList<MyMoneyInstitution> list;
  QValueList<MyMoneyInstitution>::ConstIterator it;

  tmp = 1;      // version
  s << tmp;

  list = storage->institutionList();
  s << list.count();
  for(it = list.begin(); it != list.end(); ++it) {
    writeInstitution(s, *it);
  }
}

void MyMoneyStorageBin::readInstitutions(QDataStream&s, IMyMoneySerialize *storage)
{
  Q_INT32 version;
  Q_INT32 cnt;
  unsigned long id;

  s >> version;

  s >> cnt;
  signalProgress(0, cnt, QObject::tr("Loading institutions..."));
  for(int i = 0; i < cnt; ++i) {
    MyMoneyInstitution inst = readInstitution(s);
    storage->loadInstitution(inst);

    id = extractId(inst.id().data());
    if(id > storage->institutionId())
      storage->loadInstitutionId(id);
    signalProgress(i, 0);
  }
}

void MyMoneyStorageBin::writeInstitution(QDataStream&s, const MyMoneyInstitution& i)
{
  Q_INT32 tmp;
  tmp = 1;    // version
  s << tmp;

  s << i.id();
  s << i.name();
  s << i.city();
  s << i.street();
  s << i.postcode();
  s << i.telephone();
  s << i.manager();
  s << i.sortcode();
  s << i.accountList();
}

const MyMoneyInstitution MyMoneyStorageBin::readInstitution(QDataStream& s)
{
  QCString id;
  MyMoneyInstitution  i;
  Q_INT32 version;
  QString tmp_s;

  s >> version;
  s >> id;
  s >> tmp_s; i.setName(tmp_s);
  s >> tmp_s; i.setCity(tmp_s);
  s >> tmp_s; i.setStreet(tmp_s);
  s >> tmp_s; i.setPostcode(tmp_s);
  s >> tmp_s; i.setTelephone(tmp_s);
  s >> tmp_s; i.setManager(tmp_s);
  s >> tmp_s; i.setSortcode(tmp_s);

  QCStringList list;
  s >> list;
  QCStringList::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it)
    i.addAccountId(*it);

  return MyMoneyInstitution(id, i);
}

void MyMoneyStorageBin::writePayees(QDataStream& s, IMyMoneySerialize *storage)
{
  Q_INT32 tmp;
  QValueList<MyMoneyPayee> list;
  QValueList<MyMoneyPayee>::ConstIterator it;

  tmp = 1;      // version
  s << tmp;

  list = storage->payeeList();
  s << list.count();
  for(it = list.begin(); it != list.end(); ++it) {
    writePayee(s, *it);
  }
}

void MyMoneyStorageBin::readPayees(QDataStream& s, IMyMoneySerialize *storage)
{
  Q_INT32 version;
  Q_INT32 cnt;
  unsigned long id;

  s >> version;

  s >> cnt;
  signalProgress(0, cnt, QObject::tr("Loading payees..."));
  for(int i = 0; i < cnt; ++i) {
    MyMoneyPayee p = readPayee(s);
    storage->loadPayee(p);

    id = extractId(p.id().data());
    if(id > storage->payeeId())
      storage->loadPayeeId(id);
    signalProgress(i, 0);
  }
}

void MyMoneyStorageBin::writePayee(QDataStream& s, const MyMoneyPayee& p)
{
  Q_INT32 tmp;
  tmp = 1;    // version
  s << tmp;

  s << p.id();
  s << p.name();
  s << p.address();
  s << p.email();
  s << p.postcode();
  s << p.telephone();
  s << p.reference();
}

const MyMoneyPayee MyMoneyStorageBin::readPayee(QDataStream& s)
{
  Q_INT32 tmp;
  QString tmp_s;
  QCString id;

  MyMoneyPayee p;

  s >> tmp;
  s >> id;
  s >> tmp_s; p.setName(tmp_s);
  s >> tmp_s; p.setAddress(tmp_s);
  s >> tmp_s; p.setEmail(tmp_s);
  s >> tmp_s; p.setPostcode(tmp_s);
  s >> tmp_s; p.setTelephone(tmp_s);
  s >> tmp_s; p.setReference(tmp_s);

  return MyMoneyPayee(id, p);
}

void MyMoneyStorageBin::writeAccounts(QDataStream& s, IMyMoneySerialize *storage)
{
  Q_INT32 tmp;
  QValueList<MyMoneyAccount> list;
  QValueList<MyMoneyAccount>::ConstIterator it;

  tmp = 1;      // version
  s << tmp;

  list = storage->accountList();

  s << list.count()+4;
  writeAccount(s, storage->asset());
  writeAccount(s, storage->liability());
  writeAccount(s, storage->expense());
  writeAccount(s, storage->income());

  signalProgress(0, list.count(), QObject::tr("Saving accounts..."));
  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i) {
    writeAccount(s, *it);

    signalProgress(i, 0);
  }
}

void MyMoneyStorageBin::readAccounts(QDataStream& s, IMyMoneySerialize *storage)
{
  Q_INT32 version;
  Q_INT32 cnt;
  unsigned long id;

  s >> version;

  s >> cnt;
  signalProgress(0, cnt, QObject::tr("Loading accounts..."));
  for(int i = 0; i < cnt; ++i) {
    MyMoneyAccount acc = readAccount(s);
    storage->loadAccount(acc);

    id = extractId(acc.id().data());
    if(id > storage->accountId())
      storage->loadAccountId(id);

    signalProgress(i, 0);
  }
}

void MyMoneyStorageBin::writeAccount(QDataStream& s, const MyMoneyAccount& acc)
{
  Q_INT32 tmp;
  tmp = 2;    // version
  s << tmp;

  s << acc.id();
  s << acc.name();
  s << acc.description();
  s << acc.accountType();
  s << acc.institutionId();
  s << acc.lastModified();
  s << acc.lastReconciliationDate();
  s << acc.number();
  s << acc.openingBalance();
  s << acc.openingDate();
  s << acc.parentAccountId();
  s << acc.accountList();
  writeKeyValueContainer(s, acc.pairs());
}

const MyMoneyAccount MyMoneyStorageBin::readAccount(QDataStream& s)
{
  Q_INT32 version;
  Q_INT32 tmp;
  QString tmp_s;
  QDate tmp_d;
  QCString tmp_c;
  MyMoneyMoney tmp_m;
  QCString id;

  MyMoneyAccount acc;

  s >> version;
  s >> id;
  s >> tmp_s; acc.setName(tmp_s);
  s >> tmp_s; acc.setDescription(tmp_s);
  s >> tmp;   acc.setAccountType(static_cast<MyMoneyAccount::accountTypeE> (tmp));
  s >> tmp_c; acc.setInstitutionId(tmp_c);
  s >> tmp_d; acc.setLastModified(tmp_d);
  s >> tmp_d; acc.setLastReconciliationDate(tmp_d);
  s >> tmp_s; acc.setNumber(tmp_s);
  s >> tmp_m; acc.setOpeningBalance(tmp_m);
  s >> tmp_d; acc.setOpeningDate(tmp_d);
  s >> tmp_c; acc.setParentAccountId(tmp_c);

  QCStringList list;
  s >> list;
  QCStringList::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it)
    acc.addAccountId(*it);

  if(version > 1) {
    acc.setPairs(readKeyValueContainer(s));
  }

  return MyMoneyAccount(id, acc);
}

void MyMoneyStorageBin::writeTransactions(QDataStream& s, IMyMoneySerialize* storage)
{
  Q_INT32 tmp;
  QValueList<MyMoneyTransaction> list;
  QValueList<MyMoneyTransaction>::ConstIterator it;

  tmp = 1;      // version
  s << tmp;

  list = storage->transactionList();

  s << list.count();

  signalProgress(0, list.count(), QObject::tr("Saving transactions..."));

  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i) {
    writeTransaction(s, *it);

    signalProgress(i, 0);
  }
}

void MyMoneyStorageBin::readTransactions(QDataStream& s, IMyMoneySerialize* storage)
{
  Q_INT32 version;
  Q_INT32 cnt;
  unsigned long id;

  s >> version;

  s >> cnt;
  signalProgress(0, cnt, QObject::tr("Loading transactions..."));

  for(int i = 0; i < cnt; ++i) {
    MyMoneyTransaction t = readTransaction(s);
    storage->loadTransaction(t);

    id = extractId(t.id().data());
    if(id > storage->transactionId())
      storage->loadTransactionId(id);
    signalProgress(i, 0);
  }
}

void MyMoneyStorageBin::writeTransaction(QDataStream& s, const MyMoneyTransaction& t)
{
  Q_INT32 tmp;

  tmp = 1;      // version
  s << tmp;

  s << t.id();
  s << t.entryDate();
  s << t.postDate();
  s << t.memo();
  tmp = t.splits().count();
  s << tmp;
  for(int i = 0; i < tmp; ++i) {
    writeSplit(s, t.splits()[i]);
  }
}

const MyMoneyTransaction MyMoneyStorageBin::readTransaction(QDataStream& s)
{
  Q_INT32 tmp;
  QString tmp_s;
  QDate tmp_d;
  QCString tmp_c;
  QCString id;

  MyMoneyTransaction t;

  s >> tmp;
  s >> id;
  s >> tmp_d; t.setEntryDate(tmp_d);
  s >> tmp_d; t.setPostDate(tmp_d);
  s >> tmp_s; t.setMemo(tmp_s);
  s >> tmp;
  for(int i = 0; i < tmp; ++i) {
    MyMoneySplit sp = readSplit(s);
    t.addSplit(sp);
  }

  return MyMoneyTransaction(id, t);
}

void MyMoneyStorageBin::writeSplit(QDataStream& s, const MyMoneySplit& sp)
{
  Q_INT32 tmp;

  tmp = 1;      // version
  s << tmp;

  s << sp.accountId();
  s << sp.action();
  s << sp.memo();
  s << sp.number();
  s << sp.payeeId();
  s << sp.reconcileDate();
  s << sp.reconcileFlag();
  s << sp.shares();
  s << sp.value();
}

const MyMoneySplit MyMoneyStorageBin::readSplit(QDataStream& s)
{
  Q_INT32 tmp;
  QString tmp_s;
  QDate tmp_d;
  QCString tmp_c;
  MyMoneyMoney tmp_m;

  MyMoneySplit sp;

  s >> tmp;
  s >> tmp_c; sp.setAccountId(tmp_c);
  s >> tmp_c; sp.setAction(tmp_c);
  s >> tmp_s; sp.setMemo(tmp_s);
  s >> tmp_s; sp.setNumber(tmp_s);
  s >> tmp_c; sp.setPayeeId(tmp_c);
  s >> tmp_d; sp.setReconcileDate(tmp_d);
  s >> tmp;   sp.setReconcileFlag(static_cast<MyMoneySplit::reconcileFlagE> (tmp));
  s >> tmp_m; sp.setShares(tmp_m);
  s >> tmp_m; sp.setValue(tmp_m);

  // make sure, that the payeeId for non-asset and non-liability
  // account references is empty
  MyMoneyAccount acc = m_accountList[sp.accountId()];
  switch(MyMoneyAccount::accountGroup(acc.accountType())) {
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
      break;
    default:
      sp.setPayeeId("");
      break;
  }
  return sp;
}

const unsigned long MyMoneyStorageBin::extractId(const QCString& txt) const
{
  int pos;
  unsigned long rc = 0;

  pos = txt.find(QRegExp("\\d+"), 0);
  if(pos != -1) {
    rc = atol(txt.mid(pos));
  }
  return rc;
}

void MyMoneyStorageBin::writeScheduledTransactions(QDataStream& s, IMyMoneySerialize* storage)
{
  s << (Q_INT32) 1;   // version

  s << (Q_INT32) 0;   // for now there's nothing
}

void MyMoneyStorageBin::readScheduledTransactions(QDataStream& s, IMyMoneySerialize* storage)
{
  Q_INT32 version;
  Q_INT32 cnt;

  s >> version;

  s >> cnt;

  for(int i = 0; i < cnt; ++i) {
  }
}

void MyMoneyStorageBin::writeKeyValueContainer(QDataStream& s, QMap<QCString, QString> list) const
{
  s << list;
}

const QMap<QCString, QString> MyMoneyStorageBin::readKeyValueContainer(QDataStream& s) const
{
  QMap<QCString, QString> keyValuePairs;
  s >> keyValuePairs;

  return keyValuePairs;
}

void MyMoneyStorageBin::setProgressCallback(void(*callback)(int, int, const QString&))
{
  m_progressCallback = callback;
}

void MyMoneyStorageBin::signalProgress(int current, int total, const QString& msg)
{
  if(m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}
