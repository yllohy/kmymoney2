/***************************************************************************
                          mymoneystorageanon.cpp
                             -------------------
    begin                : Thu Oct 24 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <acejones@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qfile.h>
#include <qdom.h>
#include <qmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystorageanon.h"
#include "../../kmymoneyutils.h"
#include "../mymoneyreport.h"
#include "../mymoneyinstitution.h"

QStringList MyMoneyStorageANON::zKvpNoModify = QStringList::split(",","kmm-baseCurrency,PreferredAccount,Tax,fixed-interest,interest-calculation,payee,schedule,term");
QStringList MyMoneyStorageANON::zKvpXNumber = QStringList::split(",","final-payment,loan-amount,periodic-payment");


MyMoneyStorageANON::MyMoneyStorageANON() :
  MyMoneyStorageXML()
{
}

MyMoneyStorageANON::~MyMoneyStorageANON()
{
}

void MyMoneyStorageANON::readFile(QIODevice* , IMyMoneySerialize* )
{
  throw new MYMONEYEXCEPTION("Cannot read a file through MyMoneyStorageANON!!");
}

void MyMoneyStorageANON::writeUserInformation(QDomElement& userInfo)
{
  userInfo.setAttribute(QString("name"), hideString(m_storage->userName()));
  userInfo.setAttribute(QString("email"), hideString(m_storage->userEmail()));

  QDomElement address = m_doc->createElement("ADDRESS");
  address.setAttribute(QString("street"), hideString(m_storage->userStreet()));
  address.setAttribute(QString("city"), hideString(m_storage->userTown()));
  address.setAttribute(QString("county"), hideString(m_storage->userCounty()));
  address.setAttribute(QString("zipcode"), hideString(m_storage->userPostcode()));
  address.setAttribute(QString("telephone"), hideString(m_storage->userTelephone()));

  userInfo.appendChild(address);
}

void MyMoneyStorageANON::writeInstitution(QDomElement& institution, const MyMoneyInstitution& _i)
{
  MyMoneyInstitution i(_i);

  // mangle fields
  i.setName(i.id());
  i.setManager(hideString(i.manager()));
  i.setSortcode(hideString(i.sortcode()));

  i.setStreet(hideString(i.street()));
  i.setCity(hideString(i.city()));
  i.setPostcode(hideString(i.postcode()));
  i.setTelephone(hideString(i.telephone()));

  MyMoneyStorageXML::writeInstitution(institution, i);
}


void MyMoneyStorageANON::writePayee(QDomElement& payee, const MyMoneyPayee& _p)
{
  MyMoneyPayee p(_p);

  p.setName(p.id());
  p.setReference(hideString(p.reference()));

  p.setAddress(hideString(p.address()));
  p.setCity(hideString(p.city()));
  p.setPostcode(hideString(p.postcode()));
  p.setState(hideString(p.state()));
  p.setTelephone(hideString(p.telephone()));

  MyMoneyStorageXML::writePayee(payee, p);
}

void MyMoneyStorageANON::writeAccount(QDomElement& account, const MyMoneyAccount& _p)
{
  MyMoneyAccount p(_p);

  p.setNumber(hideString(p.number()));
  p.setOpeningBalance(hideNumber(p.openingBalance()));
  p.setName(p.id());
  p.setDescription(hideString(p.description()));

  MyMoneyStorageXML::writeAccount(account, p);
}

void MyMoneyStorageANON::writeTransaction(QDomElement& transaction, const MyMoneyTransaction& tx)
{
  transaction.setAttribute(QString("id"), tx.id());
  transaction.setAttribute(QString("postdate"), getString(tx.postDate()));
  transaction.setAttribute(QString("memo"), tx.id());
  transaction.setAttribute(QString("entrydate"), getString(tx.entryDate()));
  transaction.setAttribute(QString("commodity"), tx.commodity());
  transaction.setAttribute(QString("bankid"), hideString(tx.bankID()));

  QDomElement splits = m_doc->createElement("SPLITS");
  QValueList<MyMoneySplit> splitList = tx.splits();

  writeSplits(splits, splitList,tx.id());
  transaction.appendChild(splits);

  //Add in Key-Value Pairs for transactions.
  QDomElement keyValPairs = writeKeyValuePairs(tx.pairs());
  transaction.appendChild(keyValPairs);
}

void MyMoneyStorageANON::writeSchedule(QDomElement& scheduledTx, const MyMoneySchedule& tx)
{
  scheduledTx.setAttribute(QString("name"), tx.id());
  scheduledTx.setAttribute(QString("type"), tx.type());
  scheduledTx.setAttribute(QString("occurence"), tx.occurence());
  scheduledTx.setAttribute(QString("paymentType"), tx.paymentType());
  scheduledTx.setAttribute(QString("startDate"), getString(tx.startDate()));
  scheduledTx.setAttribute(QString("endDate"), getString(tx.endDate()));
  scheduledTx.setAttribute(QString("fixed"), tx.isFixed());
  scheduledTx.setAttribute(QString("autoEnter"), tx.autoEnter());
  scheduledTx.setAttribute(QString("id"), tx.id());
  scheduledTx.setAttribute(QString("lastPayment"), getString(tx.lastPayment()));
  scheduledTx.setAttribute(QString("weekendOption"), tx.weekendOption());

  //store the payment history for this scheduled task.
  QValueList<QDate> payments = tx.recordedPayments();
  QValueList<QDate>::Iterator it;
  QDomElement paymentsElement = m_doc->createElement("PAYMENTS");
  paymentsElement.setAttribute(QString("count"), payments.count());
  for (it=payments.begin(); it!=payments.end(); ++it)
  {
    QDomElement paymentEntry = m_doc->createElement("PAYMENT");
    paymentEntry.setAttribute(QString("date"), getString(*it));
    paymentsElement.appendChild(paymentEntry);
  }
  scheduledTx.appendChild(paymentsElement);

  //store the transaction data for this task.
  QDomElement transactionElement = m_doc->createElement("TRANSACTION");
  writeTransaction(transactionElement, tx.transaction());
  scheduledTx.appendChild(transactionElement);
}

void MyMoneyStorageANON::writeSplits(QDomElement& splits, const QValueList<MyMoneySplit> splitList, const QCString& transactionId)
{
  //FIXME: Get the splits to balance out

  QValueList<MyMoneySplit>::const_iterator it;
  for(it = splitList.begin(); it != splitList.end(); ++it)
  {
    QDomElement split = m_doc->createElement("SPLIT");
    writeSplit(split, (*it), transactionId);
    splits.appendChild(split);
  }
}

void MyMoneyStorageANON::writeSplit(QDomElement& splitElement, const MyMoneySplit& split,const QCString& transactionId)
{
  splitElement.setAttribute(QString("payee"), split.payeeId());
  splitElement.setAttribute(QString("reconciledate"), getString(split.reconcileDate()));
  splitElement.setAttribute(QString("action"), split.action());
  splitElement.setAttribute(QString("reconcileflag"), split.reconcileFlag());

  MyMoneyMoney price = split.shares() / split.value();
  MyMoneyMoney hidevalue = hideNumber(split.value());
  MyMoneyMoney hideshares;
  if(! split.value().isZero()){
    hideshares = (hidevalue * price).convert();
  }

  splitElement.setAttribute(QString("value"), hidevalue.toString());
  splitElement.setAttribute(QString("shares"), hideshares.toString());
  splitElement.setAttribute(QString("memo"), QString(transactionId) + "/" + QString(split.id()));
  splitElement.setAttribute(QString("id"), split.id());
  splitElement.setAttribute(QString("account"), split.accountId());
  splitElement.setAttribute(QString("number"), hideString(split.number()));
}

void MyMoneyStorageANON::writeSecurity(QDomElement& securityElement, const MyMoneySecurity& security)
{
  securityElement.setAttribute(QString("name"), security.id());
  securityElement.setAttribute(QString("symbol"), hideString(security.tradingSymbol()));
  securityElement.setAttribute(QString("type"), static_cast<int>(security.securityType()));
  securityElement.setAttribute(QString("id"), security.id());
  securityElement.setAttribute(QString("saf"), security.smallestAccountFraction());
}

QDomElement MyMoneyStorageANON::writeKeyValuePairs(const QMap<QCString, QString> pairs)
{
  if(m_doc)
  {
    QDomElement keyValPairs = m_doc->createElement("KEYVALUEPAIRS");

    QMap<QCString, QString>::const_iterator it;
    for(it = pairs.begin(); it != pairs.end(); ++it)
    {
      QDomElement pair = m_doc->createElement("PAIR");
      pair.setAttribute(QString("key"), it.key());

      if ( zKvpXNumber.contains( it.key() ) || it.key().left(3)=="ir-" )
        pair.setAttribute(QString("value"), hideNumber(MyMoneyMoney(it.data())).toString());
      else if ( zKvpNoModify.contains( it.key() ) )
        pair.setAttribute(QString("value"), it.data());
      else
        pair.setAttribute(QString("value"), hideString(it.data()));
      keyValPairs.appendChild(pair);
    }
    return keyValPairs;
  }
  return QDomElement();
}

const QString MyMoneyStorageANON::hideString(const QString& _in) const
{
  return QString(_in).fill('x');
}

const MyMoneyMoney MyMoneyStorageANON::hideNumber(const MyMoneyMoney& _in) const
{
  MyMoneyMoney result;
  static MyMoneyMoney counter = MyMoneyMoney(100,100);

  // preserve sign
  if ( _in.isNegative() )
    result = MyMoneyMoney(-1);
  else
    result = MyMoneyMoney(1);

  result = result * counter;
  counter += MyMoneyMoney("10/100");

  // preserve > 1000
  if ( _in >= MyMoneyMoney(1000) )
    result = result * MyMoneyMoney(1000);
  if ( _in <= MyMoneyMoney(-1000) )
    result = result * MyMoneyMoney(1000);

  return result.convert();
}

// vim:cin:si:ai:et:ts=2:sw=2:
