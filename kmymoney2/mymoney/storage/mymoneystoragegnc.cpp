/***************************************************************************
                          mymoneystoragegnc  -  description
                             -------------------
    begin                : Wed Mar 3 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#include "config.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qfile.h>
#include <qdom.h>
#include <qmap.h>

// ----------------------------------------------------------------------------
// Third party Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragegnc.h"
#include "../../kmymoneyutils.h"
#include "imymoneystorage.h"

unsigned int MyMoneyStorageGNC::fileVersionRead = 0;
unsigned int MyMoneyStorageGNC::fileVersionWrite = 0;

MyMoneyStorageGNC::MyMoneyStorageGNC()
{
  m_storage = NULL;
  m_doc     = NULL;
}

MyMoneyStorageGNC::~MyMoneyStorageGNC()
{
  
}

//Function to read in the file, send to XML parser.
void MyMoneyStorageGNC::readFile(QIODevice* pDevice, IMyMoneySerialize* storage)
{
  Q_CHECK_PTR(storage);
  Q_CHECK_PTR(pDevice);
  if(!storage)
  {
    return;
  }
  m_storage = storage;
  
  m_doc = new QDomDocument;
  Q_CHECK_PTR(m_doc);
  if(m_doc->setContent(pDevice, FALSE))
  {
    QDomElement rootElement = m_doc->documentElement();
    if(!rootElement.isNull())
    {
      qDebug("GNCREADER: Root element of this file is %s\n", rootElement.tagName().data());

      if(QString("gnc-v2") == rootElement.tagName())
      {
        qDebug("GNCREADER: parsing gnucash v2 file\n");

        QDomNode child = rootElement.firstChild();
        while(!child.isNull() && child.isElement())
        {
          QDomElement childElement = child.toElement();
          qDebug("GNCREADER: Processing child node %s", childElement.tagName().data());
          
          QDomNodeList nodeList = childElement.childNodes();
          if(nodeList.count())
          {
            signalProgress(0, nodeList.count(), QObject::tr("Loading GNUCash File..."));
            
            for(int x = 0; x < nodeList.count(); x++)
            {
              QDomElement temp = nodeList.item(x).toElement();
              qDebug("GNCREADER: Dealing with %s\n", temp.tagName().data());
              if(QString("gnc:account") == temp.tagName())
              {
                /*MyMoneyAccount account = */readAccount(temp);

                //tell the storage objects we have a new institution.
                //m_storage->loadAccount(account);

                //unsigned long id = extractId(account.id().data());
                //if(id > m_storage->accountId())
                //{
                //  m_storage->loadAccountId(id);
                //}
              }
              if(QString("gnc:transaction") == temp.tagName())
              {
                readTransaction(temp);

                //tell the storage objects we have a new institution.
                //m_storage->loadTransaction(transaction);

                //id = extractId(transaction.id().data());
                ///if(id > m_storage->transactionId())
                //{
                //  m_storage->loadTransactionId(id);
                //}

                signalProgress(x, 0);
              }
            }
          }
          
          child = child.nextSibling();
        }
      }
    }   
    delete m_doc;
    m_doc = NULL;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // Next step is to walk the list and assign the parent/child relationship between the objects.


    //this code is just temporary to show us what is in the file.
    qDebug("%d accounts found in the GNU Cash file", m_mapIds.count());
    for(map_accountIds::Iterator it = m_mapIds.begin(); it != m_mapIds.end(); ++it)
    {
      qDebug("key = %s, value = %s", it.key().data(), it.data().data());
    }

    //we need a IMyMoneyStorage pointer, since m_storage is IMyMoneySerialize.
    IMyMoneyStorage* pStoragePtr = dynamic_cast<IMyMoneyStorage*>(m_storage);

    QValueList<MyMoneyAccount> list;
    QValueList<MyMoneyAccount>::Iterator theAccount;
    list = m_storage->accountList();
    for(theAccount = list.begin(); theAccount != list.end(); ++theAccount)
    {
      if((*theAccount).parentAccountId() == QCString(m_mainAssetId))
      {
        MyMoneyAccount assets = m_storage->asset();
        m_storage->addAccount(assets, (*theAccount));
        qDebug("The account id %s is a child of the main asset account", (*theAccount).id().data());
      }
      else if((*theAccount).parentAccountId() == QCString(m_mainLiabilityId))
      {
        MyMoneyAccount liabilities = m_storage->liability();
        m_storage->addAccount(liabilities, (*theAccount));
        qDebug("The account id %s is a child of the main liability account", (*theAccount).id().data());
      }
      else if((*theAccount).parentAccountId() == QCString(m_mainIncomeId))
      {
        MyMoneyAccount incomes = m_storage->income();
        m_storage->addAccount(incomes, (*theAccount));
        qDebug("The account id %s is a child of the main income account", (*theAccount).id().data());
      }
      else if((*theAccount).parentAccountId() == QCString(m_mainExpenseId))
      {
        MyMoneyAccount expenses = m_storage->expense();
        m_storage->addAccount(expenses, (*theAccount));
        qDebug("The account id %s is a child of the main expense account", (*theAccount).id().data());
      }
      else if((*theAccount).parentAccountId() == QCString(m_mainEquityId))
      {
        MyMoneyAccount assets = m_storage->asset();
        m_storage->addAccount(assets, (*theAccount));
        qDebug("The account id %s is a child of the main asset account", (*theAccount).id().data());
      }
      else
      {
        QCString parentKey = (*theAccount).parentAccountId();
        map_accountIds::Iterator id = m_mapIds.find(parentKey);
        if(id != m_mapIds.end())
        {
          qDebug("Setting account id %s's parent account id to %s", (*theAccount).id().data(), id.data().data());
          MyMoneyAccount parent = pStoragePtr->account(id.data());
          m_storage->addAccount(parent, (*theAccount));
        }
        else
        {
          Q_ASSERT(FALSE);
        }
      }    
    }

    //m_mapIds.clear();
    
    // this seems to be nonsense, but it clears the dirty flag
    // as a side-effect.
    m_storage->setLastModificationDate(m_storage->lastModificationDate());
    m_storage = NULL;

    //hides the progress bar.
    signalProgress(-1, -1);
  }
  else
  {
    throw new MYMONEYEXCEPTION("File was not parsable!");
  }
}

void MyMoneyStorageGNC::readAccount(const QDomElement& account)
{
  MyMoneyAccount acc;
  QCString id;
  QString tmp;
  QString gncName, gncAccountId, gncType, gncDescription, gncParent;
  bool bHasParent = false;
  
  QString gncVersion = account.attributes().namedItem(QString("version")).nodeValue();
  qDebug("Version of this account object is %s\n", gncVersion.data());

  if(QString("2.0.0") == gncVersion)
  {
    QDomNodeList nodeList = account.childNodes();
    qDebug("Account has %d children\n", nodeList.count());
    for(int x = 0; x < nodeList.count(); x++)
    {
      QDomElement temp = nodeList.item(x).toElement();

      if(getChildCount(temp))
      {
        QDomText text = temp.firstChild().toText();

        if(QString("act:id") == temp.tagName())
        {
          gncAccountId = QStringEmpty(text.nodeValue());
          qDebug("gnucash account id = %s\n", gncAccountId.data());
        }
        else if(QString("act:name") == temp.tagName())
        {
          gncName = QStringEmpty(text.nodeValue());
        }
        else if(QString("act:description") == temp.tagName())
        {
          gncDescription = QStringEmpty(text.nodeValue());
        }
        else if(QString("act:type") == temp.tagName())
        {
          gncType = QStringEmpty(text.nodeValue());
        }
        else if(QString("act:parent") == temp.tagName())
        {
          bHasParent = true;
          gncParent = QStringEmpty(text.nodeValue());
        }
      }

      //if(getChildCount(temp))
      //{
      //  qDebug("Account child node child count is %d\n", getChildCount(temp));
      //  qDebug("Child node has character data, %s\n", text.nodeValue().data());
      //}
      //qDebug("Account child name is %s\n", temp.tagName().data());
    }

    //this is the top-level asset account.  We don't have to create an account for this, because it
    //is already created by the engine at this point.
    if(QString("ASSET") == gncType && !bHasParent)
    {
      m_mainAssetId = gncAccountId;
    }
    else if(QString("LIABILITIES") == gncType && !bHasParent)
    {
      m_mainLiabilityId = gncAccountId;
    }
    else if(QString("INCOME") == gncType && !bHasParent)
    {
      m_mainIncomeId = gncAccountId;
    }
    else if(QString("EXPENSE") == gncType && !bHasParent)
    {
      m_mainExpenseId = gncAccountId;
    }
    else if(QString("EQUITY") == gncType && !bHasParent)
    {
      m_mainEquityId = gncAccountId;
    }
    else
    {
      acc.setName(gncName);
      acc.setDescription(gncDescription);

      QDate currentDate = QDate::currentDate();
      
      acc.setOpeningDate(currentDate);
      acc.setLastModified(currentDate);
      acc.setLastReconciliationDate(currentDate);
      //acc.setInstitutionId(QCStringEmpty(account.attribute(QString("institution"))));
      //acc.setOpeningBalance(MyMoneyMoney(account.attribute(QString("openingbalance"))));
      acc.setParentAccountId(QCString(gncParent));

      if(QString("BANK") == gncType)
      {
        acc.setAccountType(MyMoneyAccount::Checkings);
      }
      else if(QString("ASSET") == gncType)
      {
        acc.setAccountType(MyMoneyAccount::Asset);
      }
      else if(QString("CASH") == gncType)
      {
        acc.setAccountType(MyMoneyAccount::Cash);
      }
      else if(QString("STOCK") == gncType || QString("MUTUAL") == gncType)
      {
        acc.setAccountType(MyMoneyAccount::Investment);
      }
      else if(QString("LIABILITY") == gncType)
      {
        acc.setAccountType(MyMoneyAccount::Liability);
      }
      else if(QString("INCOME") == gncType)
      {
        acc.setAccountType(MyMoneyAccount::Income);
      }
      else if(QString("EXPENSE") == gncType)
      {
        acc.setAccountType(MyMoneyAccount::Expense);
      }
      else if(QString("EQUITY") == gncType)
      {
        acc.setAccountType(MyMoneyAccount::Asset);
      }

      //all the details from the file about the account should be known by now.
      //calling new account should automatically fill in the account ID.
      m_storage->newAccount(acc);
      id = acc.id();
    }   
  }

  //assign the gnucash id as the key into the map to find the
  m_mapIds[QCString(gncAccountId)] = QCString(id);
  
  qDebug("Account %s has id of %s, type of %d, parent is %s, this=0x%08X.", acc.name().data(), id.data(), acc.accountType(), acc.parentAccountId().data(),&acc);

  
  return acc;//MyMoneyAccount(id, acc);
}

/*void MyMoneyStorageGNC::readTransactions(QDomElement& transactions)
{
  unsigned long id = 0;
  QDomNode child = transactions.firstChild();
  int x = 0;
  signalProgress(0, getChildCount(transactions), QObject::tr("Loading transactions..."));

  while(!child.isNull() && child.isElement())
  {
    QDomElement childElement = child.toElement();
    if(QString("TRANSACTION") == childElement.tagName())
    {
      readTransaction(childElement);

      //tell the storage objects we have a new institution.
      //m_storage->loadTransaction(transaction);

      //id = extractId(transaction.id().data());
      //if(id > m_storage->transactionId())
      //{
      //  m_storage->loadTransactionId(id);
      //}
    }
    child = child.nextSibling();
    signalProgress(x++, 0);
  }
}  */


void MyMoneyStorageGNC::readTransaction(QDomElement& transaction, const bool withinSchedule)
{
  /*
  <gnc:transaction version="2.0.0">
  <trn:id type="guid">5d572b538b2fe48ccdfa082585c02b49</trn:id>
  <trn:currency>
    <cmdty:space>ISO4217</cmdty:space>
    <cmdty:id>USD</cmdty:id>
  </trn:currency>
  <trn:num>1</trn:num>
  <trn:date-posted>
    <ts:date>2004-03-06 00:00:00 -0500</ts:date>
  </trn:date-posted>
  <trn:date-entered>
    <ts:date>2004-03-06 09:01:56 -0500</ts:date>
  </trn:date-entered>
  <trn:description>gas</trn:description>
  <trn:splits>
    <trn:split>
      <split:id type="guid">9a200b66bcb2b11f9da9ea87544a8296</split:id>
      <split:reconciled-state>n</split:reconciled-state>
      <split:value>3200/100</split:value>
      <split:quantity>3200/100</split:quantity>
      <split:account type="guid">ce7bc9b6e58abf56c17be5fef32c6cd9</split:account>
    </trn:split>
    <trn:split>
      <split:id type="guid">b2d7bf93bdcf2f3165b25d622b7f0bce</split:id>
      <split:reconciled-state>n</split:reconciled-state>
      <split:value>-3200/100</split:value>
      <split:quantity>-3200/100</split:quantity>
      <split:account type="guid">708265b881486aa4d0ce285d97e4175f</split:account>
    </trn:split>
  </trn:splits>
</gnc:transaction>
*/
  MyMoneyTransaction tx;
  QCString id;
  QString tmp;
  QString gncTxId, gncCurrency, gncNum, gncDatePosted, gncDateEntered, gncDescription;
  bool bHasParent = false;

  QString gncVersion = transaction.attributes().namedItem(QString("version")).nodeValue();
  qDebug("Version of this transaction object is %s\n", gncVersion.data());

  if(QString("2.0.0") == gncVersion)
  {
    QDomNodeList nodeList = transaction.childNodes();
    qDebug("Transaction has %d children\n", nodeList.count());
    for(unsigned int x = 0; x < nodeList.count(); x++)
    {
      QDomElement temp = nodeList.item(x).toElement();

      if(getChildCount(temp))
      {
        QDomText text = temp.firstChild().toText();

        if(QString("trn:id") == temp.tagName())
        {
          gncTxId = QStringEmpty(text.nodeValue());
          qDebug("gnucash transaction id = %s\n", gncTxId.data());
        }
        else if(QString("trn:num") == temp.tagName())
        {
          gncNum = QStringEmpty(text.nodeValue());
        }
        else if(QString("trn:date-posted") == temp.tagName())
        {
          if(temp.hasChildNodes() && temp.firstChild().isElement())
          {
            QDomElement date = temp.firstChild().toElement();
            qDebug("element is %s", date.tagName().data());
            QDomText dateText = date.firstChild().toText();
            gncDatePosted = QStringEmpty(dateText.nodeValue());
          }
        }
        else if(QString("trn:date-entered") == temp.tagName())
        {
          if(temp.hasChildNodes() && temp.firstChild().isElement())
          {
            QDomElement date = temp.firstChild().toElement();
            qDebug("element is %s", date.tagName().data());
            QDomText dateText = date.firstChild().toText();
            gncDateEntered = QStringEmpty(dateText.nodeValue());
          }
        }
        else if(QString("trn:splits") == temp.tagName())
        {
          readSplits(tx, temp);  
        }
      }
    }
   }

   QStringList fields = QStringList::split(" ", gncDatePosted);
   if(fields.count())
   {
     QString firstField = fields.first();
     QDate postedDate = getDate(firstField);
     tx.setPostDate(postedDate);
     qDebug("Date is %s", postedDate.toString().data());
   }

   fields = QStringList::split(" ", gncDateEntered);
   if(fields.count())
   {
     QString firstField = fields.first();
     QDate enteredDate = getDate(firstField);
     tx.setEntryDate(enteredDate);
     qDebug("Date is %s", enteredDate.toString().data());
   }
   
//  qDebug("Account %s has id of %s, type of %d, parent is %s, this=0x%08X.", acc.name().data(), id.data(), acc.accountType(), acc.parentAccountId().data(),&acc);
  
  //tx.setPostDate(getDate(QStringEmpty(transaction.attribute(QString("postdate")))));
  tx.setMemo("");
  
  m_storage->addTransaction(tx, true);
  
}

void MyMoneyStorageGNC::readSchedules(QDomElement& schedules)
{
  unsigned long id = 0;
  QDomNode child = schedules.firstChild();
  int x = 0;
  signalProgress(0, getChildCount(schedules), QObject::tr("Loading schedules..."));
  
  while(!child.isNull() && child.isElement())
  {
    QDomElement childElement = child.toElement();
    if(QString("SCHEDULED_TX") == childElement.tagName())
    {
      MyMoneySchedule schedule = readSchedule(childElement);

      //tell the storage objects we have a new schedule.
      m_storage->loadSchedule(schedule);

      id = extractId(schedule.id().data());
      if(id > m_storage->scheduleId())
      {
        m_storage->loadScheduleId(id);
      }
    }
    child = child.nextSibling();
    signalProgress(x++, 0);
  }
}

 
MyMoneySchedule MyMoneyStorageGNC::readSchedule(QDomElement& schedule)
{
  MyMoneySchedule sc;
  sc.setStartDate(getDate(QStringEmpty(schedule.attribute(QString("startDate")))));
  sc.setAutoEnter(static_cast<bool>(schedule.attribute(QString("autoEnter")).toInt()));
  sc.setLastPayment(getDate(QStringEmpty(schedule.attribute(QString("lastPayment")))));
  sc.setPaymentType(static_cast<MyMoneySchedule::paymentTypeE>(schedule.attribute(QString("paymentType")).toInt()));
  sc.setEndDate(getDate(QStringEmpty(schedule.attribute(QString("endDate")))));
  sc.setType(static_cast<MyMoneySchedule::typeE>(schedule.attribute(QString("type")).toInt()));
  sc.setId(QCStringEmpty(schedule.attribute(QString("id"))));
  sc.setName(QStringEmpty(schedule.attribute(QString("name"))));
  sc.setFixed(static_cast<bool>(schedule.attribute(QString("fixed")).toInt()));
  sc.setOccurence(static_cast<MyMoneySchedule::occurenceE>(schedule.attribute(QString("occurence")).toInt()));
  sc.setWeekendOption(static_cast<MyMoneySchedule::weekendOptionE>(schedule.attribute(QString("weekendOption")).toInt()));

  QDomElement payments = findChildElement(QString("PAYMENTS"), schedule);
  if(!payments.isNull() && payments.isElement())
  {
    QDomNode child = payments.firstChild();
    while(!child.isNull())
    {
      if(child.isElement())
      {
        QDomElement childElement = child.toElement();
        if(QString("PAYMENT") == childElement.tagName())
        {
          QDate date = getDate(QStringEmpty(childElement.attribute(QString("date"))));
          Q_ASSERT(date.isValid());
          sc.recordPayment(date);
        }
      }
      child = child.nextSibling();
    }
  }

  QDomElement transaction = findChildElement(QString("TRANSACTION"), schedule);
  if(!transaction.isNull() && transaction.isElement())
  {
//    MyMoneyTransaction t = readTransaction(transaction, true);
//    sc.setTransaction(t);
  }

  return sc;  
}

void MyMoneyStorageGNC::readSplits(MyMoneyTransaction& t, QDomElement& splits)
{
  /*
  <trn:splits>
    <trn:split>
      <split:id type="guid">9a200b66bcb2b11f9da9ea87544a8296</split:id>
      <split:reconciled-state>n</split:reconciled-state>
      <split:value>3200/100</split:value>
      <split:quantity>3200/100</split:quantity>
      <split:account type="guid">ce7bc9b6e58abf56c17be5fef32c6cd9</split:account>
    </trn:split>
    <trn:split>
      <split:id type="guid">b2d7bf93bdcf2f3165b25d622b7f0bce</split:id>
      <split:reconciled-state>n</split:reconciled-state>
      <split:value>-3200/100</split:value>
      <split:quantity>-3200/100</split:quantity>
      <split:account type="guid">708265b881486aa4d0ce285d97e4175f</split:account>
    </trn:split>
  </trn:splits>
  */
  
  QDomNode child = splits.firstChild();
  while(!child.isNull() && child.isElement())
  {
    QDomElement childElement = child.toElement();
    if(QString("trn:split") == childElement.tagName())
    {
      MyMoneySplit split = readSplit(childElement);
      t.addSplit(split);
    }
    child = child.nextSibling();
  }
}

MyMoneySplit MyMoneyStorageGNC::readSplit(QDomElement& splitElement)
{
  /*
  <trn:splits>
    <trn:split>
      <split:id type="guid">9a200b66bcb2b11f9da9ea87544a8296</split:id>
      <split:reconciled-state>n</split:reconciled-state>
      <split:value>3200/100</split:value>
      <split:quantity>3200/100</split:quantity>
      <split:account type="guid">ce7bc9b6e58abf56c17be5fef32c6cd9</split:account>
    </trn:split>
    <trn:split>
      <split:id type="guid">b2d7bf93bdcf2f3165b25d622b7f0bce</split:id>
      <split:reconciled-state>n</split:reconciled-state>
      <split:value>-3200/100</split:value>
      <split:quantity>-3200/100</split:quantity>
      <split:account type="guid">708265b881486aa4d0ce285d97e4175f</split:account>
    </trn:split>
  </trn:splits>
  */
  
  MyMoneySplit split;
  QString strTmp;
  QString gncSplitId, gncSplitReconciledState, gncSplitValue, gncSplitQuantity, gncSplitAccount;

  if(splitElement.hasChildNodes())
  {
    QDomNodeList nodeList = splitElement.childNodes();
    qDebug("Split has %d children\n", nodeList.count());
    for(unsigned int x = 0; x < nodeList.count(); x++)
    {
      QDomElement temp = nodeList.item(x).toElement();

      if(temp.hasChildNodes())
      {
        QDomText text = temp.firstChild().toText();
        if(QString("split:id") == temp.tagName())
        {
          gncSplitId = QStringEmpty(text.nodeValue());
          qDebug("gnucash split id = %s\n", gncSplitId.data());
        }
        else if(QString("split:reconciled-state") == temp.tagName())
        {
          gncSplitReconciledState = QStringEmpty(text.nodeValue());
        }
        else if(QString("split:value") == temp.tagName())
        {
          gncSplitValue = QStringEmpty(text.nodeValue());
        }
        else if(QString("split:quantity") == temp.tagName())
        {
          gncSplitQuantity = QStringEmpty(text.nodeValue());
        }
        else if(QString("split:account") == temp.tagName())
        {
          gncSplitAccount = QStringEmpty(text.nodeValue());
           
          map_accountIds::Iterator id = m_mapIds.find(QCString(gncSplitAccount));
          if(id != m_mapIds.end())
          {
            qDebug("Split:  Swapping account id %s's with our account id %s", gncSplitAccount.data(), id.data().data());
            gncSplitAccount = id.data();
          }
        }
      }
    }
  }
  
  split.setPayeeId("");

  if(QString("n") == gncSplitReconciledState)
  {
    split.setReconcileFlag(MyMoneySplit::NotReconciled);
  }
  else if(QString("c") == gncSplitReconciledState)
  {
    split.setReconcileFlag(MyMoneySplit::Cleared);
  }

  //if the money amount is negative, mark it as a withdrawal, other wise it's a deposit.
  MyMoneyMoney splitValue(gncSplitValue);
  if(splitValue.isNegative())
  {
    split.setAction("Withdrawal");
  }
  else
  {
    split.setAction("Deposit");
  }

  split.setValue(splitValue);
  //split.setReconcileDate(getDate(QStringEmpty(splitElement.attribute(QString("reconciledate")))));
  split.setMemo("");//QStringEmpty(splitElement.attribute(QString("memo"))));
  split.setAccountId(QCString(gncSplitAccount));
 
  return split;
}

const unsigned long MyMoneyStorageGNC::extractId(const QCString& txt) const
{
  int pos;
  unsigned long rc = 0;

  pos = txt.find(QRegExp("\\d+"), 0);
  if(pos != -1) {
    rc = atol(txt.mid(pos));
  }
  return rc;
}

QDate MyMoneyStorageGNC::getDate(const QString& strText) const
{
  QDate date;
  if(strText.length())
  {
    QDate date = QDate::fromString(strText, Qt::ISODate);
    if(!date.isNull() && date.isValid())
    {
      return date;
    }
    else
    {
      return QDate();
    }
  }

  return date;
}

QString MyMoneyStorageGNC::getString(const QDate& date) const
{
  QString str("");
  if(!date.isNull() && date.isValid())
  {
    str = date.toString(Qt::ISODate);
  }

  return str;
}

QDomElement MyMoneyStorageGNC::findChildElement(const QString& name, const QDomElement& root)
{
  QDomNode child = root.firstChild();
  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(name == childElement.tagName())
      {
        return childElement;
      }
    }

    child = child.nextSibling();
  }
  return QDomElement();
}

QMap<QCString, QString> MyMoneyStorageGNC::readKeyValuePairs(QDomElement& element)
{
  QMap<QCString, QString> pairs;
  QDomNode child = element.firstChild();
  while(!child.isNull() && child.isElement())
  {
    QDomElement childElement = child.toElement();
    if(QString("PAIR") == childElement.tagName())
    {
      QCString key = QCString(childElement.attribute(QString("key")));
      QString value = childElement.attribute(QString("value"));

      pairs.insert(key, value);
    }

    child = child.nextSibling();
  }

  return pairs;
}

const QCString MyMoneyStorageGNC::QCStringEmpty(const QString& val) const
{
  QCString rc;

  if(!val.isEmpty())
    rc = QCString(val);

  return rc;
}

const QString MyMoneyStorageGNC::QStringEmpty(const QString& val) const
{
  QString rc;
  if(!val.isEmpty())
  {
    rc = QString(val);
  }
  return rc;
}

void MyMoneyStorageGNC::setProgressCallback(void(*callback)(int, int, const QString&))
{
  m_progressCallback = callback;
}

void MyMoneyStorageGNC::signalProgress(int current, int total, const QString& msg)
{
  if(m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}

const uint MyMoneyStorageGNC::getChildCount(const QDomElement& element) const
{
  QDomNodeList tempList = element.childNodes();
  return tempList.count();
}


