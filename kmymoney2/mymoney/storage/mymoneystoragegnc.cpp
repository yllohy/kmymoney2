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
                MyMoneyAccount account = readAccount(temp);

                //tell the storage objects we have a new institution.
                //m_storage->loadAccount(account);

                unsigned long id = extractId(account.id().data());
                if(id > m_storage->accountId())
                {
                  m_storage->loadAccountId(id);
                }
              }
              if(QString("gnc:transaction") == temp.tagName())
              {
                MyMoneyTransaction transaction = readTransaction(temp);

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

void MyMoneyStorageGNC::readAccounts(QDomElement& accounts)
{
  unsigned long id = 0;
  QDomNode child = accounts.firstChild();
  int x = 0;
  signalProgress(0, getChildCount(accounts), QObject::tr("Loading accounts..."));
  
  while(!child.isNull() && child.isElement())
  {
    QDomElement childElement = child.toElement();
    if(QString("ACCOUNT") == childElement.tagName())
    {
      MyMoneyAccount account = readAccount(childElement);

      //tell the storage objects we have a new account.
      m_storage->loadAccount(account);

      id = extractId(account.id().data());
      if(id > m_storage->accountId())
      {
        m_storage->loadAccountId(id);
      }
    }

    signalProgress(x++, 0);

    child = child.nextSibling();
  }
}

MyMoneyAccount MyMoneyStorageGNC::readAccount(const QDomElement& account)
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
    else
    {
    
      //all the details from the file about the account should be known by now.
      //calling new account should automatically fill in the account ID.
      m_storage->newAccount(acc);
      acc.setName(gncName);
      acc.setDescription(gncDescription);
      acc.setOpeningDate(getDate(QStringEmpty(account.attribute(QString("opened")))));
      acc.setLastModified(getDate(QStringEmpty(account.attribute(QString("lastmodified")))));
      acc.setLastReconciliationDate(getDate(QStringEmpty(account.attribute(QString("lastreconciled")))));
      //acc.setInstitutionId(QCStringEmpty(account.attribute(QString("institution"))));
      //acc.setOpeningBalance(MyMoneyMoney(account.attribute(QString("openingbalance"))));

      acc.setValue("GNUCASH_ID", gncAccountId);
      
      id = acc.id();
  
      if(QString("BANK") == gncType)
      {
        acc.setAccountType(MyMoneyAccount::Checkings);
        acc.setParentAccountId(findGNCParentAccount(QCString(gncParent)));
        //acc.setValue(QCString(GNUCASH_ID_KEY), gncAccountId);
      }
      else if(QString("ASSET") == gncType)
      {
        //if this is a second-level asset account, set the parent id to be the asset account's id.
        if(gncParent == m_mainAssetId)
    		{
  			  acc.setParentAccountId(m_storage->asset().id());
          //acc.setValue(QCString(GNUCASH_ID_KEY), gncAccountId);
  			}
        else
        {
          acc.setParentAccountId(findGNCParentAccount(QCString(gncParent)));
          //acc.setValue(QCString(GNUCASH_ID_KEY), gncAccountId);
        }
      }
    }   
  }
  
  qDebug("Account %s has id of %s, type of %d, parent is %s, this=0x%08X.", acc.name().data(), id.data(), acc.accountType(), acc.parentAccountId().data(),&acc);

  
  return acc;//MyMoneyAccount(id, acc);
}

QCString MyMoneyStorageGNC::findGNCParentAccount(QCString gnuCashParentAccountId)
{
  QValueList<MyMoneyAccount> list;
  QValueList<MyMoneyAccount>::ConstIterator it;

  list = m_storage->accountList();

  for(it = list.begin(); it != list.end(); ++it)
  {
    Q_ASSERT((*it).pairs().count());
    QString temp = (*it).value("GNUCASH_ID");
    Q_ASSERT(!temp.isEmpty());
    qDebug("GNUCASH id = %s, passed in id = %s, object address=0x%08X\n", temp.data(), gnuCashParentAccountId.data(), &(*it));
    if(QCString(temp) == gnuCashParentAccountId)
    {
      return (*it).id();
    }
  }

  return QCStringEmpty("");
}


void MyMoneyStorageGNC::readTransactions(QDomElement& transactions)
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
      MyMoneyTransaction transaction = readTransaction(childElement);

      //tell the storage objects we have a new institution.
      m_storage->loadTransaction(transaction);

      id = extractId(transaction.id().data());
      if(id > m_storage->transactionId())
      {
        m_storage->loadTransactionId(id);
      }
    }
    child = child.nextSibling();
    signalProgress(x++, 0);
  }
}


MyMoneyTransaction MyMoneyStorageGNC::readTransaction(QDomElement& transaction, const bool withinSchedule)
{
  QCString id;
  MyMoneyTransaction t;

  t.setEntryDate(getDate(QStringEmpty(transaction.attribute(QString("entrydate")))));
  t.setPostDate(getDate(QStringEmpty(transaction.attribute(QString("postdate")))));
  t.setMemo(QStringEmpty(transaction.attribute(QString("memo"))));

  id = QCStringEmpty(transaction.attribute(QString("id")));
  if(!withinSchedule)
    Q_ASSERT(id.size());
  else
    Q_ASSERT(id.size() == 0);
  // qDebug("Transaction has id of %s", id.data());


  QDomElement splits = findChildElement(QString("SPLITS"), transaction);
  if(!splits.isNull() && splits.isElement())
  {
    readSplits(t, splits);
  }

  //Process any KeyValue pairs information found inside the transaction entry.
  QDomElement keyValPairs = findChildElement(QString("KEYVALUEPAIRS"), transaction);
  if(!keyValPairs.isNull() && keyValPairs.isElement())
  {
    t.setPairs(readKeyValuePairs(keyValPairs));
  }

  return MyMoneyTransaction(id, t);
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
    MyMoneyTransaction t = readTransaction(transaction, true);
    sc.setTransaction(t);
  }

  return sc;  
}

void MyMoneyStorageGNC::readSplits(MyMoneyTransaction& t, QDomElement& splits)
{
  QDomNode child = splits.firstChild();
  while(!child.isNull() && child.isElement())
  {
    QDomElement childElement = child.toElement();
    if(QString("SPLIT") == childElement.tagName())
    {
      MyMoneySplit split = readSplit(childElement);
      t.addSplit(split);
    }
    child = child.nextSibling();
  }
}

MyMoneySplit MyMoneyStorageGNC::readSplit(QDomElement& splitElement)
{
  MyMoneySplit split;
  QString strTmp;
  
  split.setPayeeId(QCStringEmpty(splitElement.attribute(QString("payee"))));
  split.setReconcileDate(getDate(QStringEmpty(splitElement.attribute(QString("reconciledate")))));
  split.setAction(QCStringEmpty(splitElement.attribute(QString("action"))));
  split.setReconcileFlag(static_cast<MyMoneySplit::reconcileFlagE>(splitElement.attribute(QString("reconcileflag")).toInt()));
  split.setMemo(QStringEmpty(splitElement.attribute(QString("memo"))));
  split.setValue(MyMoneyMoney(QStringEmpty(splitElement.attribute(QString("value")))));
  split.setAccountId(QCStringEmpty(splitElement.attribute(QString("account"))));
 
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


