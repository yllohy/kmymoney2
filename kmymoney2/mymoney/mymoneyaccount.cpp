/***************************************************************************
                          mymoneyaccount.cpp
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
// ----------------------------------------------------------------------------
// QT Includes
#include <qfile.h>
#include <qtextstream.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kglobal.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "mymoneyaccount.h"
#include "mymoney_config.h"
#include "mymoneyfile.h"
#include "mymoneybank.h"

MyMoneyAccount::MyMoneyAccount()
{
  m_qdateOpening = QDate::currentDate();
  m_ulLastId = 0L;
  m_accountType = MyMoneyAccount::Current;
  m_qdateLastReconcile = QDate::currentDate();
  m_parent=0;
}

MyMoneyAccount::MyMoneyAccount(MyMoneyBank *parent, const QString& name, const QString& number, accountTypeE type,
    const QString& description, const QDate openingDate, const MyMoneyMoney openingBal,
    const QDate& lastReconcile)
{
  m_parent = parent;
  m_qstringName = name;
  m_qstringNumber = number;
  m_ulLastId=0L;
  m_accountType = type;
  m_qstringDescription = description;
  m_qdateOpening = openingDate;
  m_mymoneymoneyOpeningBalance = openingBal;
  m_qdateLastReconcile = lastReconcile;
}

MyMoneyAccount::~MyMoneyAccount()
{
}

MyMoneyMoney MyMoneyAccount::balance(void) const
{
  // Recalculate the balance each time it is requested
  MyMoneyMoney balance;

  QListIterator<MyMoneyTransaction> it(m_qlistTransactions);
  for ( ; it.current(); ++it )
 {
    MyMoneyTransaction *trans = it.current();
    if (trans->type()==MyMoneyTransaction::Credit)
      balance += trans->amount();
    else
      balance -= trans->amount();
  }

  return balance;
}

MyMoneyTransaction* MyMoneyAccount::transaction(const MyMoneyTransaction& transaction)
{
  unsigned int pos;
  if (findTransactionPosition(transaction, pos)) {
    return m_qlistTransactions.at(pos);
  }
  return 0;
}

void MyMoneyAccount::clear(void)
{
  m_qlistTransactions.clear();
}

MyMoneyTransaction* MyMoneyAccount::transactionFirst(void)
{
  return m_qlistTransactions.first();
}

MyMoneyTransaction* MyMoneyAccount::transactionNext(void)
{
  return m_qlistTransactions.next();
}

MyMoneyTransaction* MyMoneyAccount::transactionLast(void)
{
  return m_qlistTransactions.last();
}
/*
MyMoneyTransaction* MyMoneyAccount::transactionAt(int index)
{
  return m_qlistTransactions.at(index);
}
*/
unsigned int MyMoneyAccount::transactionCount(void) const
{
  return m_qlistTransactions.count();
}

unsigned int MyMoneyAccount::transactionCount(const QDate start, const QDate end)
{
  unsigned int nCount = 0;
  MyMoneyTransaction *mymoneytransaction = 0;

  for (mymoneytransaction = m_qlistTransactions.first(); mymoneytransaction;
        mymoneytransaction = m_qlistTransactions.next()) {
    if (mymoneytransaction->date()>=start && mymoneytransaction->date()<=end)
      nCount++;
  }

  return nCount;
}

/*
bool MyMoneyAccount::removeCurrentTransaction(unsigned int pos)
{
  return m_qlistTransactions.remove(pos);
}
*/
bool MyMoneyAccount::removeTransaction(const MyMoneyTransaction& transaction)
{
  unsigned int pos;
  if (findTransactionPosition(transaction, pos)) {
    if (m_parent)
      m_parent->file()->setDirty(true);
    return m_qlistTransactions.remove(pos);
  }
  return false;
}

bool MyMoneyAccount::addTransaction(MyMoneyTransaction::transactionMethod methodType, const QString& number, const QString& memo,
  const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
  const QString& fromTo, const QString& bankFrom, const QString& bankTo, MyMoneyTransaction::stateE state)
{
  MyMoneyTransaction *transaction = new MyMoneyTransaction(this, m_ulLastId++, methodType, number,
    memo, amount, date, categoryMajor, categoryMinor, atmName,
    fromTo, bankFrom, bankTo, state);


  if (m_qlistTransactions.isEmpty()) {
    m_qlistTransactions.append(transaction);
    if (m_parent)
      m_parent->file()->setDirty(true);
    return true;
  }
  int idx=0;

  // Sort on date
  QListIterator<MyMoneyTransaction> it(m_qlistTransactions);
  for ( ; it.current(); ++it, idx++ ) {
    MyMoneyTransaction *trans = it.current();
    if (trans->date() < date)
      continue;
    else if (trans->date()==date)
      continue;
    else
      break;
  }

  m_qlistTransactions.insert(idx,transaction);
  if (m_parent)
    m_parent->file()->setDirty(true);

  return true;
}

bool MyMoneyAccount::findTransactionPosition(const MyMoneyTransaction& transaction, unsigned int& pos)
{
  int k=0;

  QListIterator<MyMoneyTransaction> it(m_qlistTransactions);
  for (k=0; it.current(); ++it, k++) {
    if (*it.current() == transaction) {
      pos=k;
      return true;
    }
  }
  pos=k;
  return false;
}

bool MyMoneyAccount::operator == (const MyMoneyAccount& right)
{
  if ( (m_qstringName == right.m_qstringName) ) {
    if (m_qstringNumber == right.m_qstringNumber) {
      if (m_accountType == right.m_accountType) {
        if (m_qstringDescription == right.m_qstringDescription) {
          if (m_qdateLastReconcile == right.m_qdateLastReconcile) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

MyMoneyAccount::MyMoneyAccount(const MyMoneyAccount& right)
{
  m_qstringName = right.m_qstringName;
  m_qstringNumber = right.m_qstringNumber;
  m_accountType = right.m_accountType;
  m_ulLastId = right.m_ulLastId;
  m_qstringDescription = right.m_qstringDescription;
  m_qdateLastReconcile = right.m_qdateLastReconcile;
  m_mymoneymoneyBalance = right.m_mymoneymoneyBalance;
  m_qlistTransactions.clear();
  m_qlistTransactions = right.m_qlistTransactions;
  m_parent = right.m_parent;
}

MyMoneyAccount& MyMoneyAccount::operator = (const MyMoneyAccount& right)
{
  m_qstringName = right.m_qstringName;
  m_qstringNumber = right.m_qstringNumber;
  m_accountType = right.m_accountType;
  m_ulLastId = right.m_ulLastId;
  m_qstringDescription = right.m_qstringDescription;
  m_qdateOpening = right.m_qdateOpening;
  m_mymoneymoneyOpeningBalance = right.m_mymoneymoneyOpeningBalance;
  m_qdateLastReconcile = right.m_qdateLastReconcile;
  m_mymoneymoneyBalance = right.m_mymoneymoneyBalance;
  m_qlistTransactions.clear();
  m_qlistTransactions = right.m_qlistTransactions;
  m_parent = right.m_parent;
  return *this;
}

QDataStream &operator<<(QDataStream &s, const MyMoneyAccount &account)
{
  return s << account.m_qstringName
    << account.m_qstringDescription
    << account.m_qstringNumber
    << (Q_INT32)account.m_accountType
    << account.m_qdateOpening
    << account.m_mymoneymoneyOpeningBalance
    << account.m_qdateLastReconcile;
}

QDataStream &operator>>(QDataStream &s, MyMoneyAccount &account)
{
  return s >> account.m_qstringName
    >> account.m_qstringDescription
    >> account.m_qstringNumber
    >> (Q_INT32 &)account.m_accountType
    >> account.m_qdateLastReconcile;
}

bool MyMoneyAccount::readAllData(int version, QDataStream& stream)
{
  stream >> m_qstringName
    >> m_qstringDescription
    >> m_qstringNumber
    >> (Q_INT32 &)m_accountType;
  if (version==VERSION_0_3_3) {
    qDebug("\tIn MyMoneyAccount::readAllData:\n\t\tFound version 0.3.3 skipping opening account fields");
    stream >> m_qdateLastReconcile;
  } else {
    stream >> m_qdateOpening
      >> m_mymoneymoneyOpeningBalance
      >> m_qdateLastReconcile;
  }

  return true;
}
/** No descriptions */
QList<MyMoneyTransaction> * MyMoneyAccount::getTransactionList(){

  return &m_qlistTransactions;

}

void MyMoneyAccount::setOpeningDate(QDate date) { m_qdateOpening = date; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setOpeningBalance(MyMoneyMoney money) { m_mymoneymoneyOpeningBalance = money; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setName(const QString& name) { m_qstringName = name; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setAccountNumber(const QString& number) { m_qstringNumber = number; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setLastId(const long id) { m_ulLastId = id; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setAccountType(MyMoneyAccount::accountTypeE type) { m_accountType = type; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setDescription(const QString& description) { m_qstringDescription = description; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setLastReconcile(const QDate& date) { m_qdateLastReconcile = date; if (m_parent) m_parent->file()->setDirty(true); }

/** No descriptions */
bool MyMoneyAccount::readQIFFile(const QString& name, const QString& dateFormat, const int apostrophe, int& transCount, int& catCount)
{
  bool catmode = false;
  bool transmode = false;
  bool writecat = false;
  bool writetrans = false;
  bool cleared = false;
  int numcat = 0;
  int numtrans = 0;
  QString catname = "";
  QString expense = "";
  QString date = "";
  QString amount = "";
  QString type = "";
  QString payee = "";
  QString category = "";
  MyMoneyCategory *oldcategory = 0;

  // Find out how many transactions are in the file first
  int nTransactionGuess = 0;
  QFile qfile(name);
  if (qfile.open(IO_ReadOnly)) {
    QString qstring;
    QTextStream qtextstream(&qfile);
    while (!qtextstream.eof()) {
      qstring = qtextstream.readLine();
      if (qstring.left(1) == "D") // Assume it's a transaction
        nTransactionGuess++;
    }
    qfile.close();
  }

  emit signalProgressCount(nTransactionGuess);

    QFile f(name);
    if ( f.open(IO_ReadOnly) ) {    // file opened successfully
        QTextStream t( &f );        // use a text stream
        QString s;
        //int n = 1;
        while ( !t.eof() ) {        // until end of file...
            s = t.readLine();       // line of text excluding '\n'
            if(s.left(9) == "!Type:Cat")
            {
              catmode = true;
              transmode = false;
            }
            else if(s.left(10) == "!Type:Bank")
            {
                    transmode = true;
                catmode = false;
            }
            else if(s.left(5) == "!Type")
            {
              catmode = false;
              transmode = false;
            }
            else if(catmode)
            {
              if(s.left(1) == "N")
              {
                catname = s.mid(1);
              }
              else if(s.left(1) == "E")
              {
                expense = "E";
              }
              else if(s.left(1) == "I")
              {
                expense = "I";
              }
              else if(s.left(1) == "^")
              {
                writecat = true;
              }
            }
            else if(transmode)
            {
                    if(s.left(1) == "^")
              {
                      writetrans = true;
              }
              if(s.left(1) == "D")
              {
                    date = s.mid(1);
              }
              if(s.left(1) == "T")
              {
                      amount = s.mid(1);
              }
              if(s.left(1) == "N")
              {
                type = s.mid(1);
              }
              if(s.left(1) == "P")
              {
                    payee = s.mid(1);
              }
              if(s.left(1) == "L")
              {
                      category = s.mid(1);
              }
              if(s.left(1) == "C")
              {
                      cleared = true;
              }
            }
            

            if(transmode && writetrans)
            {
//              int slash = -1;
//              int apost = -1;
              int checknum = 0;
              bool isnumber = false;
//              int intyear = 0;
//              int intmonth = 0;
//              int intday = 0;
              QString checknumber = "";
              qDebug("Date: %s, dateFormat: %s", date.latin1(), dateFormat.latin1());


              MyMoneyTransaction::transactionMethod transmethod;

              int day=0, month=0, year=0;
              char *buffer = (char*)date.latin1();
              char *format = (char*)dateFormat.latin1();
              int res = convertQIFDate(buffer, format, apostrophe, &day, &month, &year);
              qDebug("day: %d, month: %d, year %d", day, month, year);
              qDebug("res = %s", getQIFDateFormatErrorString(res));
              QDate transdate(year, month, day);

/*
              if(dateFormat == "MM/DD'YY")
              {
                slash = date.find("/");             
                apost = date.find("'");                      use_current=1;

                QString month = date.left(slash);
                QString day = date.mid(slash + 1,2);
                day = day.stripWhiteSpace();
                QString year = date.mid(apost + 1,2);
                year = year.stripWhiteSpace();
                intyear = year.toInt();
                if(intyear > 80)
                  intyear = 1900 + year.toInt();
                else
                  intyear = 2000 + year.toInt();
                intmonth = month.toInt();
                intday = day.toInt();
              }
              else if(dateFormat == "MM/DD/YYYY")
              {
                slash = date.find("/");             
                apost = date.findRev("/");
                QString month = date.left(slash);
                QString day = date.mid(slash + 1,2);
                day = day.stripWhiteSpace();
                QString year = date.mid(apost + 1,4);
                year = year.stripWhiteSpace();
                intyear = year.toInt();
                intmonth = month.toInt();
                intday = day.toInt();
              }     
*/
              checknum = type.toInt(&isnumber);
              if(isnumber == false)
              {
                      if(type == "ATM")
                {
                  if(amount.find("-") > -1)
                          transmethod = MyMoneyTransaction::ATM;
                  else
                    transmethod = MyMoneyTransaction::Deposit;
                }
                else if(type == "DEP")
                {
                  if(amount.find("-") == -1)
                            transmethod = MyMoneyTransaction::Deposit;
                  else
                    transmethod = MyMoneyTransaction::Withdrawal;
                }
                else if(type == "TXFR")
                {
                  if(amount.find("-") > -1)
                            transmethod = MyMoneyTransaction::Transfer;
                  else
                    transmethod = MyMoneyTransaction::Deposit;
                }
                else if(type == "WTHD")
                {
                  if(amount.find("-") > -1)
                            transmethod = MyMoneyTransaction::Withdrawal;
                  else
                    transmethod = MyMoneyTransaction::Deposit;
                }
                else
                {
                  if(amount.find("-") > -1)
                            transmethod = MyMoneyTransaction::Withdrawal;
                  else
                    transmethod = MyMoneyTransaction::Deposit;
                }
                
              }
              else
              {
                  if(amount.find("-") > -1)
                          transmethod = MyMoneyTransaction::Cheque;
                  else
                    transmethod = MyMoneyTransaction::Deposit;
                  checknumber=type;
              }
//              QDate transdate(intyear,intmonth,intday);
                     int commaindex = amount.find(",");
              double dblamount = 0;
              if(commaindex != -1)
                dblamount = amount.remove(commaindex,1).toDouble();
              else
                dblamount = amount.toDouble();
              if(dblamount < 0)
                dblamount = dblamount * -1;
              MyMoneyMoney moneyamount(dblamount);
              QString majorcat = "";
              QString minorcat = "";
              int catindex = category.find(":");
              if(catindex == -1)        
                majorcat = category;
              else
              {
                majorcat = category.left(catindex);
                minorcat = category.mid(catindex + 1);
              }
              
              MyMoneyTransaction::stateE transcleared;

              if(cleared == true)
                transcleared = MyMoneyTransaction::Reconciled;
              else
                transcleared = MyMoneyTransaction::Unreconciled;

              if (!payee.isEmpty())
                bank()->file()->addPayee(payee);
              addTransaction(transmethod, checknumber, payee,
                                      moneyamount, transdate, majorcat, minorcat, "",payee,
                                      "", "", transcleared);

              date = "";
              amount = "";
              type = "";
              payee = "";
              category = "";
              cleared = false;
              writetrans = false;
              numtrans += 1;

              emit signalProgress(numtrans);
            }
            if(catmode && writecat)
            {
              QString majorcat = "";
              QString minorcat = "";
              bool minorcatexists = false;
              bool majorcatexists = false;
              int catindex = catname.find(":");
              if(catindex == -1)
                majorcat = catname;
              else
              {
                      majorcat = catname.left(catindex);
                minorcat = catname.mid(catindex + 1);
              }
                QListIterator<MyMoneyCategory> it = bank()->file()->categoryIterator();
                for ( ; it.current(); ++it ) {
                  MyMoneyCategory *data = it.current();
                if((majorcat == data->name()) && (minorcat == ""))
                {
                  majorcatexists = true;
                            minorcatexists = true;
                }
                else if(majorcat == data->name())
                {
                  oldcategory = it.current();
                  majorcatexists = true;
                    for ( QStringList::Iterator it2 = data->minorCategories().begin(); it2 != data->minorCategories().end(); ++it2 ) {
                               if((*it2) == minorcat)
                    {
                       minorcatexists = true;
                    }
                  }

                  }
              }

              MyMoneyCategory category;
              category.setName(majorcat);
              if(expense == "E")
                category.setIncome(false);
              if(expense == "I")
                category.setIncome(true);
              if(majorcatexists == true)
              {
                if((minorcatexists == false) && (minorcat != ""))
                {
                  category.addMinorCategory(minorcat);
                  oldcategory->addMinorCategory(minorcat);
                  numcat += 1;
                }
              }
              else
              {
                if(minorcat != "")
                  category.addMinorCategory(minorcat);
                bank()->file()->addCategory(category.isIncome(), category.name(), category.minorCategories());
                           numcat += 1;
              }
              catname = "";
              expense = "";
              writecat = false;
            } 
            //qDebug("%s",s.latin1());
        }
        f.close();
        transCount = numtrans;
        catCount = numcat;
    }
    return true;
}

bool MyMoneyAccount::writeQIFFile(const QString& name, const QString& dateFormat, bool expCat,
    bool expAcct, QDate startDate, QDate endDate, int& transCount, int& catCount)
{
  int numcat = 0;
  int numtrans = 0;
  emit signalProgressCount(transactionCount(startDate, endDate));

    QFile f(name);
    if ( f.open(IO_WriteOnly) ) {    // file opened successfully
      QTextStream t( &f );        // use a text stream

      if(expCat)
      {
        t << "!Type:Cat" << endl;
        QListIterator<MyMoneyCategory> it = bank()->file()->categoryIterator();
        for ( ; it.current(); ++it ) {
          MyMoneyCategory *data = it.current();
            t << "N" + data->name() << endl;
            if(data->isIncome())
              t << "I" << endl;
            else
              t << "E" << endl;
            t << "^" << endl;
            numcat += 1;
            for ( QStringList::Iterator it2 = data->minorCategories().begin(); it2 != data->minorCategories().end(); ++it2 ) {
                t << "N" << data->name() << ":" << *it2 << endl;
                if(data->isIncome())
                  t << "I" << endl;
                else
                  t << "E" << endl;
                t << "^" << endl;
                numcat += 1;
            }
        }           
      }
      if(expAcct)
      {
        t << "!Type:Bank" << endl;
        MyMoneyTransaction *transaction;
        for ( transaction = transactionFirst(); transaction; transaction=transactionNext())
        {
          if((transaction->date() >= startDate) && (transaction->date() <= endDate))
          {
            QString qstringBuffer;
            QDateToQIFDate(transaction->date(), qstringBuffer, dateFormat);
            qDebug("QDate: %s, QIF Date: %s", transaction->date().toString().latin1(), qstringBuffer.latin1());
            t << "D" << qstringBuffer << endl;

            /*
            int year = transaction->date().year();
            if(dateFormat == "MM/DD'YY")
            {
              if(year >=2000)
                      year -= 2000;
              else
                year -= 1900;
            }
            int month = transaction->date().month();
            int day = transaction->date().day();
            */

            double amount = transaction->amount().amount();
            if(transaction->type() == MyMoneyTransaction::Debit)
              amount = amount * -1;
            QString transmethod;
            if(transaction->method() == MyMoneyTransaction::ATM)
              transmethod = "ATM";
            if(transaction->method() == MyMoneyTransaction::Deposit)
              transmethod = "DEP";
            if(transaction->method() == MyMoneyTransaction::Transfer)
              transmethod = "TXFR";
            if(transaction->method() == MyMoneyTransaction::Withdrawal)
              transmethod = "WTHD";
            if(transaction->method() == MyMoneyTransaction::Cheque)
              transmethod = transaction->number();
            QString Payee = transaction->payee();
            QString Category;
            if(transaction->categoryMinor() == "")
              Category = transaction->categoryMajor();
            else
              Category = transaction->categoryMajor() + ":" + transaction->categoryMinor();
            /*
            if(dateFormat == "MM/DD'YY")
            {
              t << "D" << month << "/" << day << "'" << year << endl;
            }
            if(dateFormat == "MM/DD/YYYY")
            {
              t << "D" << month << "/" << day << "/" << year << endl;
            }
            */
            t << "U" << amount << endl;
            t << "T" << amount << endl;
            if(transaction->state() == MyMoneyTransaction::Reconciled)
              t << "CX" << endl;
            t << "N" << transmethod << endl;
            t << "P" << Payee << endl;
            t << "L" << Category << endl;
            t << "^" << endl;
            numtrans += 1;
            emit signalProgress(numtrans);
          }
        }
      }
      f.close();
      transCount = numtrans;
      catCount = numcat;
    }
  return true;
}

// Doesn't do any sanity checks on the days, months or years e.g
// days could be 7 or 78.  Months could be 3 or 83.  Both can't be
// > 99 e.g only 2 digits.
int MyMoneyAccount::convertQIFDate(const QString buffer, const QString format, const int apostrophe, int *da, int *mo, int *ye)
{
  int result=0;
  char delimiter = 0;

  *da = *mo = *ye = 0;
  
  // result gets sets to the error in validate
  if (validateQIFDateFormat(buffer, format, result, true)) {
    int d_count=0, m_count=0, y_count=0;
    int buf_count=0;
    unsigned int nFormatCount=0;
  
    while ((nFormatCount!=format.length()) && (result==0)) {
      switch (format[nFormatCount]) {
        case '%':
          nFormatCount++;
          switch (format[nFormatCount]) {
            case 'd':
              while (format[nFormatCount]=='d') { nFormatCount++; d_count++; }
              // See if the next char is a digit so we can use %d
              // and still pick up 10 etc after 9
              if (d_count==1 && isdigit(buffer[buf_count+d_count])) {
                d_count++;
              }
                
              *da = to_days(buffer.mid(buf_count, d_count), d_count);
              if (*da>0) {
                buf_count += d_count;
              } else
                result = 1;
              break;
            case 'm':
              while (format[nFormatCount]=='m') { nFormatCount++; m_count++; }
              // See if the next char is a digit so we can use %m
              // and still pick up 10 etc after 9.  WILL BREAK WHEN
              // USING %d%m%y for instance.  Could use a flag to indicate
              // if any separators are present and if they are check to
              // make sure the next digit isn't a separator.
              if (m_count==1 && isdigit(buffer[buf_count+m_count])) {
                m_count++;
              }
              *mo = to_months(buffer.mid(buf_count, m_count), m_count);
              if (*mo>0) {
                buf_count += m_count;
              } else
                result = 2;
              break;
            case 'y':
              while (format[nFormatCount]=='y') { nFormatCount++; y_count++; }
              *ye = to_year(buffer.mid(buf_count, y_count), y_count, delimiter, apostrophe);
              if (*ye>0)
                buf_count += y_count;
              else
                result = 3;
              break;
          }
          break;
        default:
          // must match exactly or / or . is required and ' is found
          if (format[nFormatCount]==buffer[buf_count]
          || ((format[nFormatCount] == '/' || format[nFormatCount] == '.')
              && buffer[buf_count] == '\'')) {
            delimiter = buffer[buf_count];
            buf_count++;
            nFormatCount++;
          } else {
            result = 4;
          }
          break;
      }
    }
/*
    if ((*format == NULL) && (result!=0)) { // end of format.  check for buffer underun
      if (buffer[buf_count]!=NULL)
        result = 14;
    }
  */
  }
    
  return result;
}

bool MyMoneyAccount::validateQIFDateFormat(const char *buffer, const char *format, int& result, bool checkBuffer)
{
  int special_count=0, d_count=0, m_count=0, y_count=0, normal_count=0;
  int found_current=0;
  
  if (!format)
    result =  5;

  if (checkBuffer && !buffer)
    result =  5;

  if ((strlen(format)<3))
    result =  6;

  if (checkBuffer && strlen(buffer)<3)
    result =  6;

  if (checkBuffer) {
    char *p_buffer = (char*)buffer;
    while (*p_buffer) {
      if (*p_buffer=='\'')
        found_current=1;
      p_buffer++;
    }
  }
  
  char *p_format = (char*)format;
  while (*p_format) {
    switch (*p_format) {
      case '%':
        special_count++;
        break;
      case 'd':
        d_count++;
        break;
      case 'm':
        m_count++;
        break;
      case 'y':
        y_count++;
        break;
      default:
        normal_count++;
        break;
    }
    p_format++;
  }

/*
  if (found_current) {
    if ((strlen(buffer)-1) != (strlen(format)-special_count))
      result =  7;
  } else {
    if ((strlen(buffer)) != (strlen(format)-special_count))
      result =  8;
  }
*/  
  if (special_count != 3)
    result =  9;
    
  if (d_count < 1 || d_count >= 3)
    result =  10;
    
  if (m_count < 1 || m_count >= 4)
    result =  11;
    
  if ((y_count < 2 || y_count >= 5) || y_count == 3)
    result =  12;
    
  if (normal_count < 1 || normal_count > 2)
    result =  13;
    
  if (result!=0)
    return false;
  return true;
}

int MyMoneyAccount::to_days(const QString buffer, int dcount)
{
  bool ok = false;
  int nResult=-1;

  switch (dcount) {
    case 1:
      nResult = buffer.toInt(&ok);
      if (ok)
        return nResult;
      break;
    case 2:
      nResult = buffer.toInt(&ok);
      if (ok)
        return nResult;
/*
      if (s_number[0]=='0') {
        return atoi(s_number+1);
      }
      else
        return atoi(s_number);
*/
      break;
  }
  
  return -1;
}

int MyMoneyAccount::to_months(const QString buffer, int mcount)
{
  bool ok = false;
  int nResult = -1;

  switch (mcount) {
    case 1:
      nResult = buffer.toInt(&ok);
      if (ok)
        return nResult;
//      return atoi(s_number);
    case 2:
      nResult = buffer.toInt(&ok);
      if (ok)
        return nResult;
/*
      if (s_number[0]=='0') {
        return atoi(s_number+1);
      }
      else
        return atoi(s_number);
*/
      break;
    case 3:
      return month_to_no(buffer);
  }

  return -1;
}

int MyMoneyAccount::month_to_no(const QString s_number)
{
  QString qstringBuffer = s_number.upper();

  if (i18n("JAN") == qstringBuffer)
    return 1;
  else if (i18n("FEB") == qstringBuffer)
    return 2;
  else if (i18n("MAR") == qstringBuffer)
    return 3;
  else if (i18n("APR") == qstringBuffer)
    return 4;
  else if (i18n("MAY") == qstringBuffer)
    return 5;
  else if (i18n("JUN") == qstringBuffer)
    return 6;
  else if (i18n("JUL") == qstringBuffer)
    return 7;
  else if (i18n("AUG") == qstringBuffer)
    return 8;
  else if (i18n("SEP") == qstringBuffer)
    return 9;
  else if (i18n("OCT") == qstringBuffer)
    return 10;
  else if (i18n("NOV") == qstringBuffer)
    return 11;
  else if (i18n("DEC") == qstringBuffer)
    return 12;

  return -1;
}
/*
void MyMoneyAccount::strupper(char *buffer)
{
  while (*buffer) {
    *buffer = toupper(*buffer);
    buffer++;
  }
}
*/
int MyMoneyAccount::to_year(const QString buffer, int ycount, char delimiter, int apostrophe)
{
  bool ok=false;
  int result=-1;
  QString qstringConv;

  result = buffer.toInt(&ok);
  if(ok) {
    switch (ycount) {
      case 2:
        if(ok) {
          switch(apostrophe) {
            case 0:   // ' is 1901-1949, / is 1950-2026
              if((result < 50 && delimiter == '\'')
              || ((delimiter == '/' || delimiter == '.') && result > 49))
                qstringConv = "19";
              else
                qstringConv = "20";
              break;

            case 1:   // ' is 1900-1999, / is 2000-2099
              if(delimiter == '\'')
                qstringConv = "19";
              else
                qstringConv = "20";
              break;

            case 2:   // ' is 2000-2099, / is 1900-1999
              if(delimiter == '\'')
                qstringConv = "20";
              else
                qstringConv = "19";
              break;
          }
        }

        qstringConv += buffer;
        result = qstringConv.toInt(&ok);
        if (!ok)
          result = -1;
        break;

      case 4:     // everything's already done
        break;

      default:    // and every other length is invalid
        result = -1;
        break;
    }
  } else
    result = -1;
  return result;
}

const char *MyMoneyAccount::getQIFDateFormatErrorString(int res)
{
  switch (res) {
    case 0:
      return i18n("No error");
    case 1:
      return i18n("Cannot convert number to Days");
    case 2:
      return i18n("Cannot convert number to Months");
    case 3:
      return i18n("Cannot convert number to Years");
    case 4:
      return i18n("Character literal in format does not match in buffer");
    case 5:
      return i18n("Arguments have not been allocated memory");
    case 6:
      return i18n("Arguments aren't long enough");
    case 7:
    case 8:
      return i18n("Format and Buffer types do not match");
    case 9:
      return i18n("Number of format specifiers invalid (%)");
    case 10:
      return i18n("Number of day format options invalid (d)");
    case 11:
      return i18n("Number of month format options invalid (m)");
    case 12:
      return i18n("Number of year format options invalid (y)");
    case 13:
      return i18n("Too many literal characters. (Just use them as separators)");
    case 14:
      return i18n("Too many characters for a format specifier found in buffer");
    default:
      return i18n("Unknown error.  Please mail mte@users.sourceforge.net with error number. Sorry.");
  }
}

int MyMoneyAccount::QDateToQIFDate(const QDate date, QString& buffer, const char* format)
{
  int result=0;
  char cLastLetter = 'x';

  // result gets sets to the error in validate
  buffer = "";

  if (validateQIFDateFormat("", format, result, false)) {
    int d_count=0, m_count=0, y_count=0;

    while (*format && result==0) {
      switch (*format) {
        case '%':
          ++format;
          switch (*format) {
            case 'd':
              while (*format=='d') { format++; d_count++; }
              buffer += QString::number(date.day());
              cLastLetter = 'd';
              break;
            case 'm':
              while (*format=='m') { format++; m_count++; }
              if (m_count==3)
                buffer += date.monthName(date.month());
              else
                buffer += QString::number(date.month());
              cLastLetter = 'm';
              break;
            case 'y':
              while (*format=='y') { format++; y_count++; }
              if (y_count==2) {
                int nYear = 0;
                if(date.year() >=2000)
                  nYear = date.year() - 2000;
                else
                  nYear = date.year() - 1900;

                if (cLastLetter == 'm' || cLastLetter == 'd') // Insert a '
                  buffer += "\'";

                if (nYear<10) // Prefix with a zero first
                  buffer += "0";
                buffer += QString::number(nYear);
              } else
                buffer += QString::number(date.year());
              cLastLetter = 'y';
              break;
          }
          break;
        default:
          buffer += *format; // Hopefully just a separator or the ' char
          cLastLetter = *format;
          format++;
          break;
      }
    }
  }

  buffer += "\0";
  return result;
}

bool MyMoneyAccount::writeCSVFile(const char *filename, QDate startDate, QDate endDate, int& transCount)
{
  QString qstringBuffer, qstringTmpBuf1, qstringTmpBuf2;
  int numtrans = 0;
  emit signalProgressCount(transactionCount(startDate, endDate));

  QFile f(filename);
  if ( f.open(IO_WriteOnly) ) {    // file opened successfully
    QTextStream qtextstream( &f );        // use a text stream

    MyMoneyTransaction *mymoneytransaction;
    for ( mymoneytransaction = transactionFirst(); mymoneytransaction; mymoneytransaction=transactionNext())
    {
      if((mymoneytransaction->date() >= startDate) && (mymoneytransaction->date() <= endDate))
      {
        switch (mymoneytransaction->type()) {
          case MyMoneyTransaction::Cheque:
            qstringTmpBuf1 = i18n("Cheque");
            break;
          case MyMoneyTransaction::Deposit:
            qstringTmpBuf1 = i18n("Deposit");
            break;
          case MyMoneyTransaction::ATM:
            qstringTmpBuf1 = i18n("ATM");
            break;
          case MyMoneyTransaction::Withdrawal:
            qstringTmpBuf1 = i18n("Withdrawal");
            break;
          case MyMoneyTransaction::Transfer:
            qstringTmpBuf1 = i18n("Transfer");
            break;
          default:
            qstringTmpBuf1 = i18n("Unknown");
            break;
        }

        switch (mymoneytransaction->state()) {
          case MyMoneyTransaction::Reconciled:
            qstringTmpBuf2 = i18n("Reconciled");
            break;
          case MyMoneyTransaction::Cleared:
            qstringTmpBuf2 = i18n("Cleared");
            break;
          case MyMoneyTransaction::Unreconciled:
            qstringTmpBuf2 = i18n("Unreconciled");
            break;
          default:
            qstringTmpBuf2 = i18n("Unknown");
            break;
        }

        qstringBuffer =
            KGlobal::locale()->formatDate(mymoneytransaction->date(), true)
            + ","
            + qstringTmpBuf1
            + ","
            + mymoneytransaction->payee()
            + ","
            + qstringTmpBuf2
            + ","
            + ((mymoneytransaction->type()==MyMoneyTransaction::Credit) ?
              QString::number(mymoneytransaction->amount().amount()) :
              QString(""))
            + ","
            + ((mymoneytransaction->type()==MyMoneyTransaction::Credit) ?
              QString("") :
              QString::number(mymoneytransaction->amount().amount()))
            + ","
//            + "\n"
//            + ",,"
            + mymoneytransaction->number()
            + ","
            + (mymoneytransaction->categoryMajor() + ":" + mymoneytransaction->categoryMinor())
            + ","
            + mymoneytransaction->memo()
            + "\n";
        qtextstream << qstringBuffer;
        numtrans++;
        emit signalProgress(numtrans);
      }
    } // End for loop
    f.close();
    transCount = numtrans;
  }
  return true;
}

bool MyMoneyAccount::readCSVFile(const char *filename, int& transCount)
{
  QString qstringBuffer, qstringTmpBuf1, qstringTmpBuf2;
  int numtrans = 0;

  // Find out how many transactions are in the file first
  int nTransactionGuess = 0;
  QFile qfileGuess(filename);
  if (qfileGuess.open(IO_ReadOnly)) {
    QTextStream qtextstreamGuess(&qfileGuess);
    while (!qtextstreamGuess.eof()) {
      qtextstreamGuess.readLine();
      nTransactionGuess++;
    }
    qfileGuess.close();
  }

  emit signalProgressCount(nTransactionGuess);

  QFile qfile(filename);
  if ( qfile.open(IO_ReadOnly) ) {    // file opened successfully
    QTextStream qtextstream( &qfile );        // use a text stream

    for (int i=0; i<nTransactionGuess; i++)
    {
      MyMoneyTransaction *mymoneytransaction = new MyMoneyTransaction;
      QString qstringLine, qstringField;
      qstringLine = qtextstream.readLine();
      mymoneytransaction->setDate( stringToDate(getField(1, qstringLine)));

      mymoneytransaction->setMethod(MyMoneyTransaction::stringToMethod(getField(2, qstringLine)));

      mymoneytransaction->setPayee(getField(3, qstringLine));

      QString qstringStatus = getField(4, qstringLine);
      if (qstringStatus == i18n("Reconciled"))
        mymoneytransaction->setState(MyMoneyTransaction::Reconciled);
      else if (qstringStatus == i18n("Cleared"))
        mymoneytransaction->setState(MyMoneyTransaction::Cleared);
      else
        mymoneytransaction->setState(MyMoneyTransaction::Unreconciled);

      QString qstringAmount = getField(5, qstringLine);
      if (qstringAmount!="")
      {
        mymoneytransaction->setAmount(qstringAmount.toLong());
      }
      else
      {
        mymoneytransaction->setAmount(getField(6, qstringLine).toLong());
      }

      mymoneytransaction->setNumber(getField(7, qstringLine));

      QString qstringCategory = getField(8, qstringLine);
      QString qstringMajor, qstringMinor;
      if (qstringCategory.contains(':'))
      {
        qstringMajor = qstringCategory.left(qstringCategory.find(':'));
        qstringMinor = qstringCategory.right(qstringCategory.length()-qstringCategory.find(':'));
      }
      else
        qstringMajor = qstringCategory;
      mymoneytransaction->setCategoryMajor(qstringMajor);
      mymoneytransaction->setCategoryMinor(qstringMinor);

      mymoneytransaction->setMemo(getField(9, qstringLine));

      bank()->file()->addPayee(mymoneytransaction->payee());
      bank()->file()->addMinorCategory(
        ((mymoneytransaction->type()==MyMoneyTransaction::Credit)?true:false), qstringMajor, qstringMinor);

      addTransaction(mymoneytransaction->method(), mymoneytransaction->number(), mymoneytransaction->memo(),
        mymoneytransaction->amount(), mymoneytransaction->date(), mymoneytransaction->categoryMajor(),
        mymoneytransaction->categoryMinor(), mymoneytransaction->atmBankName(), mymoneytransaction->payee(),
        mymoneytransaction->accountFrom(), mymoneytransaction->accountTo(), mymoneytransaction->state());

      delete mymoneytransaction;
      numtrans++;
      emit signalProgress(numtrans);
    }

    qfile.close();
    transCount = numtrans;
  }

  return true;
}

QString MyMoneyAccount::getField(const int fieldnum, const char* buffer)
{
  QString qstringReturn="";
  int n=0, nFieldCount=0;
  bool bContinue=true;

  while (buffer[n] && bContinue)
  {
    switch (buffer[n])
    {
      case '\n':
        bContinue=false;
        break;
      case ',':
        if (nFieldCount==fieldnum)
          bContinue=false;
        nFieldCount++;
        break;
      default:
        if (nFieldCount == (fieldnum-1))
          qstringReturn += buffer[n];
    }
    n++;
  }

  return qstringReturn;
}

QDate MyMoneyAccount::stringToDate(const char *string)
{
  return KGlobal::locale()->readDate(string);
}
