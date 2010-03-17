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
#include "../mymoneyreport.h"
#include "../mymoneyinstitution.h"

QStringList MyMoneyStorageANON::zKvpNoModify = QStringList::split(",","kmm-baseCurrency,PreferredAccount,Tax,fixed-interest,interest-calculation,payee,schedule,term,kmm-online-source,kmm-brokerage-account,lastStatementDate,kmm-sort-reconcile,kmm-sort-std,kmm-iconpos,mm-closed,payee,schedule,term,lastImportedTransactionDate,VatAccount,VatRate,kmm-matched-tx,Imported");
QStringList MyMoneyStorageANON::zKvpXNumber = QStringList::split(",","final-payment,loan-amount,periodic-payment,lastStatementBalance");


MyMoneyStorageANON::MyMoneyStorageANON() :
  MyMoneyStorageXML()
{
  // Choose a quasi-random 0.0-100.0 factor which will be applied to all splits this time
  // around.

  int msec;
  do {
    msec = QTime::currentTime().msec();
  } while(msec == 0);
  m_factor = MyMoneyMoney(msec, 10).reduce();
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
  MyMoneyPayee user = m_storage->user();

  userInfo.setAttribute(QString("name"), hideString(user.name()));
  userInfo.setAttribute(QString("email"), hideString(user.email()));

  QDomElement address = m_doc->createElement("ADDRESS");
  address.setAttribute(QString("street"), hideString(user.address()));
  address.setAttribute(QString("city"), hideString(user.city()));
  address.setAttribute(QString("county"), hideString(user.state()));
  address.setAttribute(QString("zipcode"), hideString(user.postcode()));
  address.setAttribute(QString("telephone"), hideString(user.telephone()));

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
  p.setNotes(hideString(p.notes()));
  bool ignoreCase;
  QStringList keys;
  MyMoneyPayee::payeeMatchType matchType = p.matchData(ignoreCase, keys);
  QRegExp exp("[A-Za-z]");
  p.setMatchData(matchType, ignoreCase, QStringList::split(";", keys.join(";").replace(exp, "x")));

  MyMoneyStorageXML::writePayee(payee, p);
}

void MyMoneyStorageANON::writeAccount(QDomElement& account, const MyMoneyAccount& _p)
{
  MyMoneyAccount p(_p);

  p.setNumber(hideString(p.number()));
  p.setName(p.id());
  p.setDescription(hideString(p.description()));
  fakeKeyValuePair(p);

  // Remove the online banking settings entirely.
  p.setOnlineBankingSettings(MyMoneyKeyValueContainer());

  MyMoneyStorageXML::writeAccount(account, p);
}

void MyMoneyStorageANON::fakeTransaction(MyMoneyTransaction& tx)
{
  MyMoneyTransaction tn = tx;

  // hide transaction data
  tn.setMemo(tx.id());
  tn.setBankID(hideString(tx.bankID()));

  // hide split data
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = tx.splits().begin(); it_s != tx.splits().end(); ++it_s) {
    MyMoneySplit s = (*it_s);
    s.setMemo(QString("%1/%2").arg(tn.id()).arg(s.id()));

    if(s.value() != MyMoneyMoney::autoCalc) {
      s.setValue((s.value() * m_factor));
      s.setShares((s.shares() * m_factor));
    }
    s.setNumber(hideString(s.number()));

    // obfuscate a possibly matched transaction as well
    if(s.isMatched()) {
      MyMoneyTransaction t = s.matchedTransaction();
      fakeTransaction(t);
      s.removeMatch();
      s.addMatch(t);
    }
    tn.modifySplit(s);
  }
  tx = tn;
  fakeKeyValuePair(tx);
}

void MyMoneyStorageANON::fakeKeyValuePair(MyMoneyKeyValueContainer& kvp)
{
  QMap<QString, QString> pairs;
  QMap<QString, QString>::const_iterator it;

  for(it = kvp.pairs().begin(); it != kvp.pairs().end(); ++it)
  {
    if ( zKvpXNumber.contains( it.key() ) || it.key().left(3)=="ir-" )
      pairs[it.key()] = hideNumber(MyMoneyMoney(it.data())).toString();
    else if ( zKvpNoModify.contains( it.key() ) )
      pairs[it.key()] = it.data();
    else
      pairs[it.key()] = hideString(it.data());
  }
  kvp.setPairs(pairs);
}

void MyMoneyStorageANON::writeTransaction(QDomElement& transactions, const MyMoneyTransaction& tx)
{
  MyMoneyTransaction tn = tx;

  fakeTransaction(tn);

  MyMoneyStorageXML::writeTransaction(transactions, tn);
}

void MyMoneyStorageANON::writeSchedule(QDomElement& scheduledTx, const MyMoneySchedule& sx)
{
  MyMoneySchedule sn = sx;
  MyMoneyTransaction tn = sn.transaction();

  fakeTransaction(tn);

  sn.setName(sx.id());
  sn.setTransaction(tn, true);

  MyMoneyStorageXML::writeSchedule(scheduledTx, sn);
}

void MyMoneyStorageANON::writeSecurity(QDomElement& securityElement, const MyMoneySecurity& security)
{
  MyMoneySecurity s = security;
  s.setName(security.id());
  fakeKeyValuePair(s);

  MyMoneyStorageXML::writeSecurity(securityElement, s);
}

QString MyMoneyStorageANON::hideString(const QString& _in) const
{
  return QString(_in).fill('x');
}

MyMoneyMoney MyMoneyStorageANON::hideNumber(const MyMoneyMoney& _in) const
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

void MyMoneyStorageANON::fakeBudget(MyMoneyBudget& bx)
{
  MyMoneyBudget bn;

  bn.setName(bx.name());
  bn.setBudgetStart(bx.budgetStart());
  bn = MyMoneyBudget(bx.id(), bn);

  QValueList<MyMoneyBudget::AccountGroup> list = bx.getaccounts();
  QValueList<MyMoneyBudget::AccountGroup>::iterator it;
  for(it = list.begin(); it != list.end(); ++it) {
    // only add the account if there is a budget entered
    if(!(*it).balance().isZero()) {
      MyMoneyBudget::AccountGroup account;
      account.setId((*it).id());
      account.setBudgetLevel((*it).budgetLevel());
      account.setBudgetSubaccounts((*it).budgetSubaccounts());
      QMap<QDate, MyMoneyBudget::PeriodGroup> plist = (*it).getPeriods();
      QMap<QDate, MyMoneyBudget::PeriodGroup>::const_iterator it_p;
      for(it_p = plist.begin(); it_p != plist.end(); ++it_p) {
        MyMoneyBudget::PeriodGroup pGroup;
        pGroup.setAmount((*it_p).amount() * m_factor );
        pGroup.setStartDate( (*it_p).startDate());
        account.addPeriod(pGroup.startDate(), pGroup);
      }
      bn.setAccount(account, account.id());
    }
  }

  bx = bn;
}

void MyMoneyStorageANON::writeBudget(QDomElement& budgets, const MyMoneyBudget& b)
{
  MyMoneyBudget bn = b;

  fakeBudget(bn);

  MyMoneyStorageXML::writeBudget(budgets, bn);
}


// vim:cin:si:ai:et:ts=2:sw=2:
