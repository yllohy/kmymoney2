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
  m_openingDate = QDate::currentDate();
  m_lastId = 0L;
  m_accountType = MyMoneyAccount::Current;
  m_lastReconcile = QDate::currentDate();
  m_parent=0;
}

MyMoneyAccount::MyMoneyAccount(MyMoneyBank *parent, const QString& name, const QString& number, accountTypeE type,
    const QString& description, const QDate openingDate, const MyMoneyMoney openingBal,
    const QDate& lastReconcile)
{
  m_parent = parent;
  m_accountName = name;
  m_accountNumber = number;
  m_lastId=0L;
  m_accountType = type;
  m_description = description;
  m_openingDate = openingDate;
  m_openingBalance = openingBal;
  m_lastReconcile = lastReconcile;
}

MyMoneyAccount::~MyMoneyAccount()
{
}

MyMoneyMoney MyMoneyAccount::balance(void) const
{
  // Recalculate the balance each time it is requested
  MyMoneyMoney balance;

  QListIterator<MyMoneyTransaction> it(m_transactions);
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
    return m_transactions.at(pos);
  }
  return 0;
}

void MyMoneyAccount::clear(void)
{
  m_transactions.clear();
}

MyMoneyTransaction* MyMoneyAccount::transactionFirst(void)
{
  return m_transactions.first();
}

MyMoneyTransaction* MyMoneyAccount::transactionNext(void)
{
  return m_transactions.next();
}

MyMoneyTransaction* MyMoneyAccount::transactionLast(void)
{
  return m_transactions.last();
}
/*
MyMoneyTransaction* MyMoneyAccount::transactionAt(int index)
{
  return m_transactions.at(index);
}
*/
unsigned int MyMoneyAccount::transactionCount(void) const
{
  return m_transactions.count();
}

unsigned int MyMoneyAccount::transactionCount(const QDate start, const QDate end)
{
  unsigned int nCount = 0;
  MyMoneyTransaction *mymoneytransaction = 0;

  for (mymoneytransaction = m_transactions.first(); mymoneytransaction;
        mymoneytransaction = m_transactions.next()) {
    if (mymoneytransaction->date()>=start && mymoneytransaction->date()<=end)
      nCount++;
  }

  return nCount;
}

/*
bool MyMoneyAccount::removeCurrentTransaction(unsigned int pos)
{
  return m_transactions.remove(pos);
}
*/
bool MyMoneyAccount::removeTransaction(const MyMoneyTransaction& transaction)
{
  unsigned int pos;
  if (findTransactionPosition(transaction, pos)) {
    if (m_parent)
      m_parent->file()->setDirty(true);
    return m_transactions.remove(pos);
  }
  return false;
}

bool MyMoneyAccount::addTransaction(MyMoneyTransaction::transactionMethod methodType, const QString& number, const QString& memo,
  const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
  const QString& fromTo, const QString& bankFrom, const QString& bankTo, MyMoneyTransaction::stateE state)
{
  MyMoneyTransaction *transaction = new MyMoneyTransaction(this, m_lastId++, methodType, number,
    memo, amount, date, categoryMajor, categoryMinor, atmName,
    fromTo, bankFrom, bankTo, state);


  if (m_transactions.isEmpty()) {
    m_transactions.append(transaction);
    if (m_parent)
      m_parent->file()->setDirty(true);
    return true;
  }
  int idx=0;

  // Sort on date
  QListIterator<MyMoneyTransaction> it(m_transactions);
  for ( ; it.current(); ++it, idx++ ) {
    MyMoneyTransaction *trans = it.current();
    if (trans->date() < date)
      continue;
    else if (trans->date()==date)
      continue;
    else
      break;
  }

  m_transactions.insert(idx,transaction);
  if (m_parent) 
    m_parent->file()->setDirty(true);

  return true;
}

bool MyMoneyAccount::findTransactionPosition(const MyMoneyTransaction& transaction, unsigned int& pos)
{
  int k=0;

  QListIterator<MyMoneyTransaction> it(m_transactions);
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
  if ( (m_accountName == right.m_accountName) ) {
    if (m_accountNumber == right.m_accountNumber) {
      if (m_accountType == right.m_accountType) {
        if (m_description == right.m_description) {
          if (m_lastReconcile == right.m_lastReconcile) {
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
  m_accountName = right.m_accountName;
  m_accountNumber = right.m_accountNumber;
  m_accountType = right.m_accountType;
  m_lastId = right.m_lastId;
  m_description = right.m_description;
  m_lastReconcile = right.m_lastReconcile;
  m_balance = right.m_balance;
  m_transactions.clear();
  m_transactions = right.m_transactions;
  m_parent = right.m_parent;
}

MyMoneyAccount& MyMoneyAccount::operator = (const MyMoneyAccount& right)
{
  m_accountName = right.m_accountName;
  m_accountNumber = right.m_accountNumber;
  m_accountType = right.m_accountType;
  m_lastId = right.m_lastId;
  m_description = right.m_description;
  m_openingDate = right.m_openingDate;
  m_openingBalance = right.m_openingBalance;
  m_lastReconcile = right.m_lastReconcile;
  m_balance = right.m_balance;
  m_transactions.clear();
  m_transactions = right.m_transactions;
  m_parent = right.m_parent;
  return *this;
}

QDataStream &operator<<(QDataStream &s, const MyMoneyAccount &account)
{
  return s << account.m_accountName
    << account.m_description
    << account.m_accountNumber
    << (Q_INT32)account.m_accountType
    << account.m_openingDate
    << account.m_openingBalance
    << account.m_lastReconcile;
}

QDataStream &operator>>(QDataStream &s, MyMoneyAccount &account)
{
  return s >> account.m_accountName
    >> account.m_description
    >> account.m_accountNumber
    >> (Q_INT32 &)account.m_accountType
    >> account.m_lastReconcile;
}

bool MyMoneyAccount::readAllData(int version, QDataStream& stream)
{
  stream >> m_accountName
    >> m_description
    >> m_accountNumber
    >> (Q_INT32 &)m_accountType;
  if (version==VERSION_0_3_3) {
    qDebug("\tIn MyMoneyAccount::readAllData:\n\t\tFound version 0.3.3 skipping opening account fields");
    stream >> m_lastReconcile;
  } else {
    stream >> m_openingDate
      >> m_openingBalance
      >> m_lastReconcile;
  }

  return true;
}
/** No descriptions */
QList<MyMoneyTransaction> * MyMoneyAccount::getTransactionList(){

  return &m_transactions;

}

void MyMoneyAccount::setOpeningDate(QDate date) { m_openingDate = date; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setOpeningBalance(MyMoneyMoney money) { m_openingBalance = money; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setName(const QString& name) { m_accountName = name; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setAccountNumber(const QString& number) { m_accountNumber = number; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setLastId(const long id) { m_lastId = id; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setAccountType(MyMoneyAccount::accountTypeE type) { m_accountType = type; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setDescription(const QString& description) { m_description = description; if (m_parent) m_parent->file()->setDirty(true); }
void MyMoneyAccount::setLastReconcile(const QDate& date) { m_lastReconcile = date; if (m_parent) m_parent->file()->setDirty(true); }

/** No descriptions */
bool MyMoneyAccount::readQIFFile(const QString& name, const QString& dateFormat, int& transCount, int& catCount)
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
              int slash = -1;
              int apost = -1;
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
              int res = convertQIFDate(buffer, format, &day, &month, &year);
              qDebug("day: %d, month: %d, year %d", day, month, year);
              qDebug("res = %s", getQIFDateFormatErrorString(res));
              QDate transdate(year, month, day);

/*
              if(dateFormat == "MM/DD'YY")
              {
                slash = date.find("/");             
                apost = date.find("'");
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
int MyMoneyAccount::convertQIFDate(char* buffer, char* format, int *da, int *mo, int *ye)
{
  int result=0;
  
  *da = *mo = *ye = 0;
  
  // result gets sets to the error in validate
  if (validateQIFDateFormat(buffer, format, result, true)) {
    int d_count=0, m_count=0, y_count=0;
    int buf_count=0;
  
    while (*format && result==0) {
      switch (*format) {
        case '%':
          ++format;
          switch (*format) {
            case 'd':
              while (*format=='d') { format++; d_count++; }
              // See if the next char is a digit so we can use %d
              // and still pick up 10 etc after 9
              if (d_count==1 && isdigit(buffer[buf_count+d_count])) {
                d_count++;
              }
                
              *da = to_days(buffer+buf_count, d_count);
              if (*da>0) {
                buf_count += d_count;
              } else
                result = 1;
              break;
            case 'm':
              while (*format=='m') { format++; m_count++; }
              // See if the next char is a digit so we can use %m
              // and still pick up 10 etc after 9.  WILL BREAK WHEN
              // USING %d%m%y for instance.  Could use a flag to indicate
              // if any separators are present and if they are check to
              // make sure the next digit isn't a separator.
              if (m_count==1 && isdigit(buffer[buf_count+m_count])) {
                m_count++;
              }
              *mo = to_months(buffer+buf_count, m_count);
              if (*mo>0) {
                buf_count += m_count;
              } else
                result = 2;
              break;
            case 'y':
              while (*format=='y') { format++; y_count++; }
              if (isdigit(buffer[buf_count+y_count]) &&
                !buffer_contains(buffer, '\''))
                result = 14;
              else {
                *ye = to_year(buffer+buf_count, y_count);
                if (*ye>0)
                  buf_count += y_count;
                else
                  result = 3;
              }
              break;
          }
          break;
        default:
          if (*format==buffer[buf_count]) {
            buf_count++;
            format++;
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

int MyMoneyAccount::to_days(char *buffer, int dcount)
{
  char s_number[10];

  if (str_has_alpha(buffer, dcount))
    return -1;
    
  for (int i=0; i<dcount; i++)
    s_number[i]=buffer[i];
  s_number[dcount]=NULL;
  
  if (strlen(s_number)>=1) {
    switch (dcount) {
      case 1:
        return atoi(s_number);
      case 2:
        if (s_number[0]=='0') {
          return atoi(s_number+1);
        }
        else
          return atoi(s_number);
        break;
    }
  }
  
  return -1;
}

int MyMoneyAccount::to_months(char *buffer, int mcount)
{
  char s_number[10];

  if (mcount!=3) {
    if (str_has_alpha(buffer, mcount))
      return -1;
  }
    
  for (int i=0; i<mcount; i++)
    s_number[i]=buffer[i];
  s_number[mcount]=NULL;
  
  if (strlen(s_number)>=1) {
    switch (mcount) {
      case 1:
        return atoi(s_number);
      case 2:
        if (s_number[0]=='0') {
          return atoi(s_number+1);
        }
        else
          return atoi(s_number);
        break;
      case 3:
        return month_to_no(s_number);
    }
  }
  
  return -1;
}

int MyMoneyAccount::month_to_no(char *s_number)
{
  strupper(s_number);

  if ((strcmp("JAN", s_number))==0)
    return 1;
  else if ((strcmp("FEB", s_number))==0)
    return 2;
  else if ((strcmp("MAR", s_number))==0)
    return 3;
  else if ((strcmp("APR", s_number))==0)
    return 4;
  else if ((strcmp("MAY", s_number))==0)
    return 5;
  else if ((strcmp("JUN", s_number))==0)
    return 6;
  else if ((strcmp("JUL", s_number))==0)
    return 7;
  else if ((strcmp("AUG", s_number))==0)
    return 8;
  else if ((strcmp("SEP", s_number))==0)
    return 9;
  else if ((strcmp("OCT", s_number))==0)
    return 10;
  else if ((strcmp("NOV", s_number))==0)
    return 11;
  else if ((strcmp("DEC", s_number))==0)
    return 12;

  return -1;
}

void MyMoneyAccount::strupper(char *buffer)
{
  while (*buffer) {
    *buffer = toupper(*buffer);
    buffer++;
  }
}

int MyMoneyAccount::to_year(char *buffer, int ycount)
{
  char s_number[10];
  int i=0;
  int k=0;
  int use_current=0;
  int l_count=0;

  if (str_has_alpha(buffer, ycount))
    return -1;
    
  if (buffer[0]=='\'') {
    use_current=1;
    i=1;
    l_count = ycount+1;
  } else {
    i=0;
    l_count = ycount;
  }
    
  int current = 20; // CHANGE CHANGE CHANGE !
  for (k=0; i<l_count; i++, k++)
    s_number[k]=buffer[i];
  s_number[ycount]=NULL;
  
  if (strlen(s_number)>=1) {
    switch (ycount) {
      case 1:
        return -1;
      case 2:
        char conv[5];
        if (use_current)
          strcpy(conv, itoa(current, buffer));
        else
          strcpy(conv, itoa(current-1, buffer));
        strcat(conv, s_number);
        return atoi(conv);
      case 3:
        return -1;
      case 4:
        return atoi(s_number);
    }
  }
  
  return -1;
}

char *MyMoneyAccount::itoa(int num, char *buffer)
{
  snprintf(buffer, strlen(buffer), "%d", num);
  return buffer;
}

const char *MyMoneyAccount::getQIFDateFormatErrorString(int res)
{
  switch (res) {
    case 0:
      return "No error";
    case 1:
      return "Cannot convert number to Days";
    case 2:
      return "Cannot convert number to Months";
    case 3:
      return "Cannot convert number to Years";
    case 4:
      return "Character literal in format does not match in buffer";
    case 5:
      return "Arguments have not been allocated memory";
    case 6:
      return "Arguments aren't long enough";
    case 7:
    case 8:
      return "Format and Buffer types do not match";
    case 9:
      return "Number of format specifiers invalid (%)";
    case 10:
      return "Number of day format options invalid (d)";
    case 11:
      return "Number of month format options invalid (m)";
    case 12:
      return "Number of year format options invalid (y)";
    case 13:
      return "Too many literal characters. (Just use them as separators)";
    case 14:
      return "Too many characters for a format specifier found in buffer";
    default:
      return "Unknown error.  Please mail mte@users.sourceforge.net with error number. Sorry.";
  }
}

int MyMoneyAccount::str_has_alpha(const char *buffer, int len)
{
  int count=0;
  while (*buffer && (count<len)) {
    if (isalpha(*buffer))
      return 1;
    buffer++;
    count ++;
  }
  
  return 0;
}

int MyMoneyAccount::buffer_contains(const char *buffer, char let)
{
  char *pbuf = (char*)buffer;
  while (*pbuf) {
    if (*pbuf == let)
      return 1;
    pbuf++;
  }
  
  return 0;
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
