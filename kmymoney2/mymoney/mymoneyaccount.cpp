/***************************************************************************
                          mymoneyaccount.cpp
                          -------------------
    copyright            : (C) 2000 by Michael Edwardes
                           (C) 2002 by Thomas Baumagrt
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
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

#include <qregexp.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneyaccount.h"

// uncomment the next line for debug purposes only
// #include <iostream>

MyMoneyAccount::MyMoneyAccount()
{
  m_file = 0;
  m_openingBalance = 0;
  m_accountType = UnknownAccountType;
}

MyMoneyAccount::~MyMoneyAccount()
{
}

MyMoneyAccount::MyMoneyAccount(const QCString& id, const MyMoneyAccount& right)
{
  *this = right;
  m_id = id;
}

void MyMoneyAccount::setName(const QString& name)
{
  m_name = name;
}

void MyMoneyAccount::setNumber(const QString& number)
{
  m_number = number;
}

void MyMoneyAccount::setDescription(const QString& desc)
{
  m_description = desc;
}

void MyMoneyAccount::setInstitutionId(const QCString& id)
{
  m_institution = id;
}

void MyMoneyAccount::setLastModified(const QDate& date)
{
  m_lastModified = date;
}

void MyMoneyAccount::setOpeningDate(const QDate& date)
{
  m_openingDate = date;
}

void MyMoneyAccount::setOpeningBalance(const MyMoneyMoney& balance)
{
/* removed with MyMoneyAccount::Transaction
  MyMoneyMoney diff;

  diff = balance - m_openingBalance;

  if(diff != 0) {
    QValueList<MyMoneyAccount::Transaction>::Iterator it_t;
    for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
      MyMoneyAccount::Transaction t((*it_t).transactionID(), (*it_t).balance()+diff);
      *it_t = t;
    }
  }
*/
  m_openingBalance = balance;
}

void MyMoneyAccount::setLastReconciliationDate(const QDate& date)
{
  m_lastReconciliationDate = date;
}

void MyMoneyAccount::setParentAccountId(const QCString& parent)
{
  m_parentAccount = parent;
}

void MyMoneyAccount::setAccountType(const accountTypeE type)
{
  m_accountType = type;
}

/*
void MyMoneyAccount::setAccountTypeByName(const QCString& strType)
{
  if(strType == "CHECKING")
  {
    m_accountType = MyMoneyAccount::Checkings;
  }
  else if(strType == "SAVINGS")
  {
    m_accountType = MyMoneyAccount::Savings;
  }
  else
  {
    m_accountType = MyMoneyAccount::UnknownAccountType;
  }

  //return m_accountType;
}
*/

/* removed with MyMoneyAccount::Transaction
const QValueList<MyMoneyAccount::Transaction>& MyMoneyAccount::transactionList(void) const
{
  return m_transactionList;
}

const MyMoneyAccount::Transaction& MyMoneyAccount::transaction(const QString id) const
{
  QValueList<MyMoneyAccount::Transaction>::ConstIterator it;

  for(it = m_transactionList.begin(); it != m_transactionList.end(); ++it) {
    if((*it).transactionID() == id)
      return *it;
  }
  throw new MYMONEYEXCEPTION("Invalid transaction id");
}
*/

/*
const MyMoneyAccount::Transaction& MyMoneyAccount::transaction(const int idx) const
{
  if(idx >= 0 && idx < static_cast<int> (m_transactionList.count()))
    return m_transactionList[idx];
  throw new MYMONEYEXCEPTION("Invalid transaction index");
}

void MyMoneyAccount::addTransaction(const MyMoneyAccount::Transaction& val)
{
  MyMoneyAccount::Transaction v(val.transactionID(), balance() + val.balance());
  m_transactionList += v;
}

void MyMoneyAccount::clearTransactions(void)
{
  m_transactionList.clear();
}

const MyMoneyMoney MyMoneyAccount::balance(void) const
{
  MyMoneyMoney result(0);

  if(m_transactionList.count() > 0)
    result = m_transactionList.back().balance();
  return result;
}
*/

void MyMoneyAccount::setAccountId(const QCString& id)
{
  m_id = id;
}

void MyMoneyAccount::addAccountId(const QCString& account)
{
  if(!m_accountList.contains(account))
    m_accountList += account;
}

void MyMoneyAccount::removeAccountId(const QCString& account)
{
  QCStringList::Iterator it;

  it = m_accountList.find(account);
  if(it != m_accountList.end())
    m_accountList.remove(it);
}

const MyMoneyAccount::accountTypeE MyMoneyAccount::accountGroup(MyMoneyAccount::accountTypeE type)
{
  switch(type) {
    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Currency:
    case MyMoneyAccount::Investment:
    case MyMoneyAccount::MoneyMarket:
    case MyMoneyAccount::CertificateDep:
    case MyMoneyAccount::AssetLoan:
      return MyMoneyAccount::Asset;

    case MyMoneyAccount::CreditCard:
    case MyMoneyAccount::Loan:
      return MyMoneyAccount::Liability;

    default:
      return type;
  }
}

const bool MyMoneyAccount::operator == (const MyMoneyAccount& right) const
{
  return ((m_id == right.m_id) &&
      (m_accountList == right.m_accountList) &&
      (m_accountType == right.m_accountType) &&
      (m_description == right.m_description) &&
      (m_lastModified == right.m_lastModified) &&
      (m_lastReconciliationDate == right.m_lastReconciliationDate) &&
      (m_name == right.m_name) &&
      (m_number == right.m_number) &&
      (m_openingBalance == right.m_openingBalance) &&
      (m_openingDate == right.m_openingDate) &&
      (m_parentAccount == right.m_parentAccount) &&
      (m_institution == right.m_institution) );
}

const MyMoneyAccount::accountTypeE MyMoneyAccount::accountGroup(void) const
{
  return accountGroup(m_accountType);  
}


MyMoneyAccountLoan::MyMoneyAccountLoan(const MyMoneyAccount& acc)
 : MyMoneyAccount(acc)
{
}

const MyMoneyMoney MyMoneyAccountLoan::loanAmount(void) const
{
  return MyMoneyMoney(value("loan-amount"));
}

void MyMoneyAccountLoan::setLoanAmount(const MyMoneyMoney& amount)
{
  setValue("loan-amount", amount.toString());
}

const MyMoneyMoney MyMoneyAccountLoan::interestRate(const QDate& date) const
{
  MyMoneyMoney rate;
  QCString key;
  QString val;

  if(!date.isValid())
    return rate;
      
  key.sprintf("ir-%04d-%02d-%02d", date.year(), date.month(), date.day());
  
  QRegExp regExp("ir-(\\d{4})-(\\d{2})-(\\d{2})");
  
  QMap<QCString, QString>::ConstIterator it;

  for(it = pairs().begin(); it != pairs().end(); ++it) {
    if(regExp.search(it.key()) > -1) {
      if(qstrcmp(it.key(),key) <= 0)
        val = *it;
      else
        break;
        
    } else if(!val.isEmpty())
      break;
  }

  if(!val.isEmpty()) {
    rate = MyMoneyMoney(val);
  }
    
  return rate;
}

void MyMoneyAccountLoan::setInterestRate(const QDate& date, const MyMoneyMoney& value)
{
  if(!date.isValid())
    return;
    
  QCString key;
  key.sprintf("ir-%04d-%02d-%02d", date.year(), date.month(), date.day());
  setValue(key, value.toString());
}

const MyMoneyAccountLoan::interestDueE MyMoneyAccountLoan::interestCalculation(void) const
{
  QString payTime(value("interest-calculation"));
  if(payTime == QString("paymentDue"))
    return paymentDue;
  return paymentReceived;
}

void MyMoneyAccountLoan::setInterestCalculation(const MyMoneyAccountLoan::interestDueE onReception)
{
  if(onReception == paymentDue)
    setValue("interest-calculation", "paymentDue");
  else
    setValue("interest-calculation", "paymentReceived");
}

const QDate MyMoneyAccountLoan::nextInterestChange(void) const
{
  QDate rc;
  
  QRegExp regExp("(\\d{4})-(\\d{2})-(\\d{2})");
  if(regExp.search(value("interest-nextchange")) != -1) {
    rc.setYMD(regExp.cap(1).toInt(), regExp.cap(2).toInt(), regExp.cap(3).toInt());
  }
  return rc;
}

void MyMoneyAccountLoan::setNextInterestChange(const QDate& date)
{
  setValue("interest-nextchange", date.toString(Qt::ISODate));
}

const int MyMoneyAccountLoan::interestChangeFrequency(int* unit) const
{
  int rc = -1;
  
  if(unit)
    *unit = 1;
  
  QRegExp regExp("(\\d+)/(\\d{1})");
  if(regExp.search(value("interest-changefrequency")) != -1) {
    rc = regExp.cap(1).toInt();
    if(unit != 0) {
      *unit = regExp.cap(2).toInt();
    }
  }
  return rc;
}

void MyMoneyAccountLoan::setInterestChangeFrequency(const int amount, const int unit)
{
  QString val;
  val.sprintf("%d/%d", amount, unit);
  setValue("interest-changeFrequency", val);
}

const QCString MyMoneyAccountLoan::schedule(void) const
{
  return QCString(value("schedule").latin1());
}

void MyMoneyAccountLoan::setSchedule(const QCString& sched)
{
  setValue("schedule", sched);
}

const bool MyMoneyAccountLoan::fixedInterestRate(void) const
{
  // make sure, that an empty kvp element returns true
  return !(value("fixed-interest") == "no");
}

void MyMoneyAccountLoan::setFixedInterestRate(const bool fixed)
{
  setValue("fixed-interest", fixed ? "yes" : "no");
  if(fixed) {
    deletePair("interest-nextchange");
    deletePair("interest-changeFrequency");
  }
}

const MyMoneyMoney MyMoneyAccountLoan::finalPayment(void) const
{
  return MyMoneyMoney(value("final-payment"));
}

void MyMoneyAccountLoan::setFinalPayment(const MyMoneyMoney& finalPayment)
{
  setValue("final-payment", finalPayment.toString());
}

const unsigned int MyMoneyAccountLoan::term(void) const
{
  return value("term").toUInt();  
}

void MyMoneyAccountLoan::setTerm(const unsigned int payments)
{
  setValue("term", QString::number(payments));
}

const MyMoneyMoney MyMoneyAccountLoan::periodicPayment(void) const
{
  return MyMoneyMoney(value("periodic-payment"));
}

void MyMoneyAccountLoan::setPeriodicPayment(const MyMoneyMoney& payment)
{
  setValue("periodic-payment", payment.toString());
}

const QCString MyMoneyAccountLoan::payee(void) const
{
  return QCString(value("payee").latin1());
}

void MyMoneyAccountLoan::setPayee(const QCString& payee)
{
  setValue("payee", payee);
}

