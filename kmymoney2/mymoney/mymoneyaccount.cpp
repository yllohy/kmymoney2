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
bool MyMoneyAccount::readQIFFile(const QString& name, const QString& dateFormat)
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
							qDebug("Found Bank Type");
             				transmode = true;
						  	catmode = false;
						}
						else if(s.left(5) == "!Type")
						{
							qDebug("Found Just Type");
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
							int intyear = 0;
							int intmonth = 0;
							int intday = 0;
							QString checknumber = "";
							MyMoneyTransaction::transactionMethod transmethod;
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
							QDate transdate(intyear,intmonth,intday);
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
							qDebug("Date:%s",date.latin1());
							qDebug("Amount:%s",amount.latin1());
							qDebug("Type:%s",type.latin1());
							qDebug("Payee:%s",payee.latin1());
							qDebug("Category:%s",category.latin1());

			        date = "";
              amount = "";
							type = "";
		          payee = "";
              category = "";
              cleared = false;
							writetrans = false;
							numtrans += 1;
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
    }
    return true;
}

bool MyMoneyAccount::writeQIFFile(const QString& name, const QString& dateFormat, bool expCat,bool expAcct,
																QDate startDate, QDate endDate)
{
	int numcat = 0;
	int numtrans = 0;

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
							
						if(dateFormat == "MM/DD'YY")
						{
							t << "D" << month << "/" << day << "'" << year << endl;
						}
						if(dateFormat == "MM/DD/YYYY")
						{
							t << "D" << month << "/" << day << "/" << year << endl;
						}
						t << "U" << amount << endl;
						t << "T" << amount << endl;
						if(transaction->state() == MyMoneyTransaction::Reconciled)
							t << "CX" << endl;
						t << "N" << transmethod << endl;
						t << "P" << Payee << endl;
						t << "L" << Category << endl;
						t << "^" << endl;
						numtrans += 1;

													
					}
				}
			}
      f.close();
		}
	return true;
}
