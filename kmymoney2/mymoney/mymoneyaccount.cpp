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
// KDE Includes

//#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyexception.h>
#include <kmymoney/mymoneyaccount.h>

MyMoneyAccount::MyMoneyAccount()
{
  m_openingBalance = MyMoneyMoney(0);
  m_accountType = UnknownAccountType;
}

MyMoneyAccount::~MyMoneyAccount()
{
}

MyMoneyAccount::MyMoneyAccount(const QCString& id, const MyMoneyAccount& right) :
  MyMoneyObject(id)
{
  *this = right;
  setId(id);
}

MyMoneyAccount::MyMoneyAccount(const QDomElement& node) :
  MyMoneyObject(node),
  MyMoneyKeyValueContainer(node.elementsByTagName("KEYVALUEPAIRS").item(0).toElement())
{
  if("ACCOUNT" != node.tagName())
    throw new MYMONEYEXCEPTION("Node was not ACCOUNT");

  setName(node.attribute("name"));

  // qDebug("Reading information for account %s", acc.name().data());

  setParentAccountId(QCStringEmpty(node.attribute("parentaccount")));
  setLastModified(stringToDate(QStringEmpty(node.attribute("lastmodified"))));
  setLastReconciliationDate(stringToDate(QStringEmpty(node.attribute("lastreconciled"))));
  setInstitutionId(QCStringEmpty(node.attribute("institution")));
  setNumber(QStringEmpty(node.attribute("number")));
  setOpeningDate(stringToDate(QStringEmpty(node.attribute("opened"))));
  setCurrencyId(QCStringEmpty(node.attribute("currency")));

  QString tmp = QStringEmpty(node.attribute("type"));
  bool bOK = false;
  int type = tmp.toInt(&bOK);
  if(bOK) {
    setAccountType(static_cast<MyMoneyAccount::accountTypeE>(type));
  } else {
    qWarning("XMLREADER: Account %s had invalid or no account type information.", name().data());
  }

  // setOpeningBalance(MyMoneyMoney(node.attribute("openingbalance")));
  setDescription(node.attribute("description"));

  m_id = QCStringEmpty(node.attribute("id"));
  // qDebug("Account %s has id of %s, type of %d, parent is %s.", acc.name().data(), id.data(), type, acc.parentAccountId().data());

  //  Process any Sub-Account information found inside the account entry.
  m_accountList.clear();
  QDomNodeList nodeList = node.elementsByTagName("SUBACCOUNTS");
  if(nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName("SUBACCOUNT");
    for(unsigned int i = 0; i < nodeList.count(); ++i) {
      addAccountId(QCString(nodeList.item(i).toElement().attribute("id")));
    }
  }

  nodeList = node.elementsByTagName("ONLINEBANKING");
  if(nodeList.count() > 0) {
    QDomNamedNodeMap attributes = nodeList.item(0).toElement().attributes();
    for(unsigned int i = 0; i < attributes.count(); ++i) {
      const QDomAttr& it_attr = attributes.item(i).toAttr();
      m_onlineBankingSettings.setValue(it_attr.name().utf8(), it_attr.value());
    }
  }

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

void MyMoneyAccount::addAccountId(const QCString& account)
{
  if(!m_accountList.contains(account))
    m_accountList += account;
}

void MyMoneyAccount::removeAccountIds(void)
{
  m_accountList.clear();
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
    case MyMoneyAccount::Stock:
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
  return (MyMoneyKeyValueContainer::operator==(right) &&
      MyMoneyObject::operator==(right) &&
      (m_accountList == right.m_accountList) &&
      (m_accountType == right.m_accountType) &&
      (m_lastModified == right.m_lastModified) &&
      (m_lastReconciliationDate == right.m_lastReconciliationDate) &&
      ((m_name.length() == 0 && right.m_name.length() == 0) || (m_name == right.m_name)) &&
      ((m_number.length() == 0 && right.m_number.length() == 0) || (m_number == right.m_number)) &&
      ((m_description.length() == 0 && right.m_description.length() == 0) || (m_description == right.m_description)) &&
      (m_openingBalance == right.m_openingBalance) &&
      (m_openingDate == right.m_openingDate) &&
      (m_parentAccount == right.m_parentAccount) &&
      (m_currencyId == right.m_currencyId) &&
      (m_institution == right.m_institution) );
}

const MyMoneyAccount::accountTypeE MyMoneyAccount::accountGroup(void) const
{
  return accountGroup(m_accountType);
}

void MyMoneyAccount::setCurrencyId(const QCString& id)
{
  m_currencyId = id;
}

bool MyMoneyAccount::isAssetLiability(void) const
{
  return accountGroup() == Asset || accountGroup() == Liability;
}

bool MyMoneyAccount::isIncomeExpense(void) const
{
  return accountGroup() == Income || accountGroup() == Expense;
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
  if(payTime == "paymentDue")
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

const QCString MyMoneyAccountLoan::interestAccountId(void) const
{
  return QCString();
}

void MyMoneyAccountLoan::setInterestAccountId(const QCString& /* id */)
{

}

bool MyMoneyAccountLoan::hasReferenceTo(const QCString& id) const
{
  return MyMoneyAccount::hasReferenceTo(id)
         || (id == payee())
         || (id == schedule());
}

void MyMoneyAccount::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("ACCOUNT");

  el.setAttribute("parentaccount", parentAccountId());
  el.setAttribute("lastreconciled", dateToString(lastReconciliationDate()));
  el.setAttribute("lastmodified", dateToString(lastModified()));
  el.setAttribute("institution", institutionId());
  el.setAttribute("opened", dateToString(openingDate()));
  el.setAttribute("number", number());
  // el.setAttribute("openingbalance", openingBalance().toString());
  el.setAttribute("type", accountType());
  el.setAttribute("id", id());
  el.setAttribute("name", name());
  el.setAttribute("description", description());
  if(!currencyId().isEmpty())
    el.setAttribute("currency", currencyId());

  //Add in subaccount information, if this account has subaccounts.
  if(accountCount())
  {
    QDomElement subAccounts = document.createElement("SUBACCOUNTS");
    QCStringList::ConstIterator it;
    for(it = accountList().begin(); it != accountList().end(); ++it)
    {
      QDomElement temp = document.createElement("SUBACCOUNT");
      temp.setAttribute("id", (*it));
      subAccounts.appendChild(temp);
    }

    el.appendChild(subAccounts);
  }

  // Write online banking settings
  if(m_onlineBankingSettings.pairs().count()) {
    QDomElement onlinesettings = document.createElement("ONLINEBANKING");
    QMap<QCString,QString>::const_iterator it_key = m_onlineBankingSettings.pairs().begin();
    while ( it_key != m_onlineBankingSettings.pairs().end() ) {
      onlinesettings.setAttribute(it_key.key(), it_key.data());
      ++it_key;
    }
    el.appendChild(onlinesettings);
  }


  //Add in Key-Value Pairs for accounts.
  MyMoneyKeyValueContainer::writeXML(document, el);

  parent.appendChild(el);
}

bool MyMoneyAccount::hasReferenceTo(const QCString& id) const
{
  return (id == m_institution) || (id == m_parentAccount) || (id == m_currencyId);
}

void MyMoneyAccount::setOnlineBankingSettings(const MyMoneyKeyValueContainer& values)
{
  m_onlineBankingSettings = values;
}

const MyMoneyKeyValueContainer& MyMoneyAccount::onlineBankingSettings(void) const
{
  return m_onlineBankingSettings;
}

void MyMoneyAccount::setClosed(bool closed)
{
  if(closed)
    setValue("mm-closed", "yes");
  else
    deletePair("mm-closed");
}

bool MyMoneyAccount::isClosed(void) const
{
  return !(value("mm-closed").isEmpty());
}

int MyMoneyAccount::fraction(const MyMoneySecurity& sec) const
{
  if(m_accountType == Cash)
    return sec.smallestCashFraction();
  return sec.smallestAccountFraction();
}

bool MyMoneyAccount::isCategory(void) const
{
  return m_accountType == Income || m_accountType == Expense;
}

