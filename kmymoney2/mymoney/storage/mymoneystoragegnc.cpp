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

// #define NOSUBFOLDERS 1 // test only

// to have a single investment account for all stocks, define its name here.
// if it exists within the gnucash file, it will be marked as an investment account,
// otherwise it will be created as such.
// the default behaviour is to create one investment account for each stock
//#define INVACCT "Investments"

bool gncdebug = false;

unsigned int MyMoneyStorageGNC::fileVersionRead = 0;
unsigned int MyMoneyStorageGNC::fileVersionWrite = 0;

//***************** Constructor ***********************

MyMoneyStorageGNC::MyMoneyStorageGNC() {
    m_storage = NULL;
    m_doc     = NULL;
    m_mainName[m_mainAssetId] = QString("AStd::Asset");
    m_mainName[m_mainLiabilityId] = QString("AStd::Liability");
    m_mainName[m_mainIncomeId] = QString("AStd::Income");
    m_mainName[m_mainExpenseId] = QString("AStd::Expense");
    //m_mainName[m_mainInvestmentId] = QObject::tr("Investments");
    m_inconsistentInput = 0;
    m_invAcctStored = false;
}

//***************** Destructor *************************
MyMoneyStorageGNC::~MyMoneyStorageGNC() {
}

//************************ readFile *****************************
//Function to read in the file, send to XML parser.
void MyMoneyStorageGNC::readFile(QIODevice* pDevice, IMyMoneySerialize* storage) {
    
    Q_CHECK_PTR(storage);
    Q_CHECK_PTR(pDevice);
    if(!storage) {
        return;
    }
    m_storage = storage;
    bool containsScheds = false;
    
    m_doc = new QDomDocument;
    Q_CHECK_PTR(m_doc);
    if(m_doc->setContent(pDevice, FALSE)) {
        QDomElement rootElement = m_doc->documentElement();
        if(!rootElement.isNull()) {
            if (gncdebug) qDebug("GNCREADER: Root element of this file is %s\n",rootElement.tagName().data());
            if(QString("gnc-v2") == rootElement.tagName())  {
                if (gncdebug) qDebug("GNCREADER: parsing gnucash v2 file\n");
                
                QDomNode child = rootElement.firstChild();
                while(!child.isNull() && child.isElement()) {
                    QDomElement childElement = child.toElement();
                    if (gncdebug) qDebug("GNCREADER: Processing child node %s", childElement.tagName().data());
                    
                    QDomNodeList nodeList = childElement.childNodes();
                    if(nodeList.count()) {
                        signalProgress(0, nodeList.count(), QObject::tr("Loading GNUCash File..."));
                        
                        for (unsigned int x = 0; x < nodeList.count(); x++) {
                            QDomElement temp = nodeList.item(x).toElement();
                            if (gncdebug) qDebug("GNCREADER: Dealing with %s\n", temp.tagName().data());
                            if(QString("gnc:account") == temp.tagName()) {
                                readAccount(temp);
                            }
                            if(QString("gnc:commodity") == temp.tagName()) {
                                readCommodity(temp);
                            }
                            if(QString("gnc:pricedb") == temp.tagName()) {
                                readPrices(temp);
                            }
                            if(QString("gnc:transaction") == temp.tagName())  {
                                readTransaction(temp);
                            }
                            if(QString("gnc:template-transactions") == temp.tagName())  {
                                readTemplates(temp);
                            }
                            if(QString("gnc:schedxaction") == temp.tagName())  {
                                readSchedule(temp);
                                containsScheds = true;
                            }
                            signalProgress(x, 0);
                        } // end for
                    } // end if
                    child = child.nextSibling();
                }  // end while child...
            } else {
                throw new MYMONEYEXCEPTION (QObject::tr("Can only handle gnc-v2 format files"));
            }  // end if gnc-v2
            
            delete m_doc;
            m_doc = NULL;
            
            //////////////////////////////////////////////////////////////////////////////////////////////////
            // Next step is to walk the list and assign the parent/child relationship between the objects.
            //this code is just temporary to show us what is in the file.
            
            if (gncdebug) qDebug("%d accounts found in the GNU Cash file", m_mapIds.count());
            for(map_accountIds::Iterator it = m_mapIds.begin(); it != m_mapIds.end(); ++it)       {
                if (gncdebug) qDebug("key = %s, value = %s", it.key().data(), it.data().data());
            }
            
            QValueList<MyMoneyAccount> list;
            QValueList<MyMoneyAccount>::Iterator theAccount;
            list = m_storage->accountList();
            
            for(theAccount = list.begin(); theAccount != list.end(); ++theAccount) {
                if((*theAccount).parentAccountId() == QCString(m_mainName[m_mainAssetId])) {
                    MyMoneyAccount assets = m_storage->asset();
                    m_storage->addAccount(assets, (*theAccount));
                    if (gncdebug) qDebug("Account id %s is a child of the main asset account", (*theAccount).id().data());
                } else if ((*theAccount).parentAccountId() == QCString(m_mainName[m_mainLiabilityId])) {
                    MyMoneyAccount liabilities = m_storage->liability();
                    m_storage->addAccount(liabilities, (*theAccount));
                    if (gncdebug) qDebug("Account id %s is a child of the main liability account", (*theAccount).id().data()); 
                } else if ((*theAccount).parentAccountId() == QCString(m_mainName[m_mainIncomeId])) {
                    MyMoneyAccount incomes = m_storage->income();
                    m_storage->addAccount(incomes, (*theAccount));
                    if (gncdebug) qDebug("Account id %s is a child of the main income account", (*theAccount).id().data());
                } else if ((*theAccount).parentAccountId() == QCString(m_mainName[m_mainExpenseId])) {
                    MyMoneyAccount expenses = m_storage->expense();
                    m_storage->addAccount(expenses, (*theAccount));
                    if (gncdebug) qDebug("Account id %s is a child of the main expense account", (*theAccount).id().data());
                } else {                
                    // it is not under one of the main accounts, so find gnucash parent
                    // we need a IMyMoneyStorage pointer, since m_storage is IMyMoneySerialize.
                    IMyMoneyStorage* pStoragePtr = dynamic_cast<IMyMoneyStorage*>(m_storage);
                    QCString parentKey = (*theAccount).parentAccountId();
                    if (gncdebug) qDebug ("acc %s, parent %s", (*theAccount).id().data(),
                                          (*theAccount).parentAccountId().data());
                    map_accountIds::Iterator id = m_mapIds.find(parentKey);
                    
                    if (id != m_mapIds.end()) {
                        if (gncdebug) qDebug("Setting account id %s's parent account id to %s",
                                             (*theAccount).id().data(),  id.data().data());
                        MyMoneyAccount parent = pStoragePtr->account(id.data());
                        parent = checkConsistency (parent, (*theAccount));
                        m_storage->addAccount(parent, (*theAccount));
                    } else {
                        Q_ASSERT(FALSE);
                    }
                }
            } // end for account
            
            // send a warning re scheduled txs
            if (containsScheds)
                QMessageBox::information (0, "KMyMoney2",
                                          QObject::tr("The file contains scheduled transactions.\n"
                                                      "If you have not run gnucash against it recently,\n"
                                                      "some of these may have been missed.\n"));
            
            // setup the base currency as the dominant currency of the gnc file
            unsigned long iMaxCount = 0;
            QString sDomCurrency = "";
            QMap<QString, unsigned long>::iterator cit;
            for (cit = m_currencyCounter.begin(); cit != m_currencyCounter.end(); cit++) {
                if (cit.data() > iMaxCount) {
                    iMaxCount =  cit.data();
                    sDomCurrency = cit.key();
                }
            }
            if (!sDomCurrency.isEmpty()) {
                QString sDomCurrencyName = "";
                QValueList<MyMoneyCurrency>::ConstIterator cit;
                for (cit = m_storage->currencyList().begin(); cit != m_storage->currencyList().end(); cit++) {
                    if (QString((*cit).id()) == sDomCurrency) {
                        if (gncdebug) qDebug ("curr %s = %s", (*cit).id().data(), (*cit).name().data());
                        sDomCurrencyName = QString((*cit).name());
                        break;
                    }
                }
                QString baseMess =
                        QObject::tr(QString().sprintf("Your main currency is %s (%s).\n"
                                                      "Do you want this to be set as your base currency?  ",
                                                      sDomCurrency.latin1(), sDomCurrencyName.latin1()));
                switch (QMessageBox::information
                        (0, "KMyMoney2", baseMess, QObject::tr("Yes"), QObject::tr("No"))) {
                case 0:
                    // get all key-value-pairs out of the engine
                    QMap<QCString, QString> pairs = m_storage->pairs();
                    pairs["kmm-baseCurrency"] = sDomCurrency;
                    // store the pairs back to the engine
                    m_storage->setPairs(pairs);
                }      // otherwise we don't need to do anything coz he will get asked for base curr later
            }
            // report any inconsistent input
            QString ii;
            if (m_inconsistentInput) {
                ii = QObject::tr(QString().sprintf("%ld inconsistencies were found. Run in 'konsole' for more details\n",
                                                   m_inconsistentInput));
            } else {
                ii = QObject::tr("No inconsistencies were found. However:\n");
            }
            ii = ii + ("It is advisable to run a consistency check (Tools menu) for a more thorough analysis");
            QMessageBox::information (0, "KMyMoney2", ii);
            
            // clear up the various maps and lists
            m_currencyCounter.clear();
            m_mapIds.clear();
            m_mapEquities.clear();
            m_mapSchedules.clear();
            m_splitList.clear();
            m_templateList.clear();
            // this seems to be nonsense, but it clears the dirty flag
            // as a side-effect.
            m_storage->setLastModificationDate(m_storage->lastModificationDate());
            m_storage = NULL;
            
            //hides the progress bar.
            signalProgress(-1, -1);
        } else {
            throw new MYMONEYEXCEPTION (QObject::tr("File is empty")); // root is null
        }  // end if root null
    }
    else {
        throw new MYMONEYEXCEPTION(QObject::tr("File was not parsable!"));
    }  // end set content
} // end read file

//****************************** readCommodity *******************************
void MyMoneyStorageGNC::readCommodity(const QDomElement& cmdty) {
    
    MyMoneyEquity equ;
    unsigned long eid;
    QString tmp;
    QString gncName, gncSpace, gncId, gncFraction;
    
    QString gncVersion = cmdty.attributes().namedItem(QString("version")).nodeValue();
    if (gncdebug) qDebug("Version of this cmdty object is %s\n", gncVersion.data());
    
    if(QString("2.0.0") == gncVersion)
    {
        QDomNodeList nodeList = cmdty.childNodes();
        if (gncdebug) qDebug("Cmdty has %d children\n", nodeList.count());
        for(unsigned int x = 0; x < nodeList.count(); x++) {
            QDomElement temp = nodeList.item(x).toElement();
            
            if(getChildCount(temp)) {
                QDomText text = temp.firstChild().toText();
                
                if(QString("cmdty:space") == temp.tagName()) {
                    gncSpace = QStringEmpty(text.nodeValue());
                } else if(QString("cmdty:id") == temp.tagName()) {
                    gncId = QStringEmpty(text.nodeValue());
                } else if(QString("cmdty:name") == temp.tagName()) {
                    gncName = QStringEmpty(text.nodeValue());
                } else if(QString("cmdty:fraction") == temp.tagName()) {
                    gncFraction = QStringEmpty(text.nodeValue());
                }
            }
        } // end for
        // we have all the gc data on this commodity
        // within this section of the gnucash file, we have only equities
        // not currencies (I hope!)
        equ.setName(gncName);
        equ.setTradingSymbol(gncId);
        equ.setTradingMarket(gncSpace);
        equ.setEquityType(MyMoneyEquity::ETYPE_NONE);
        //FIXME - replace this with the newEquity() call
        // assign the next available id - replace with newEquity()
#define EQUITY_ID_SIZE 6
        QCString id;
        id.setNum(m_storage->equityId() + 1);
        id = "E" + id.rightJustify(EQUITY_ID_SIZE, '0');
        //tell the storage objects we have a new equity object.
        MyMoneyEquity e = MyMoneyEquity(id, equ);
        m_storage->loadEquity(e);
        
        //assign the gnucash id as the key into the map to find our id
        if (gncdebug) qDebug ("mapping, key = %s, id = %s", gncId.latin1(), e.id().data());
        m_mapEquities[QCString(gncId)] = QCString(e.id());
        eid = extractId(e.id().data());
        if(eid > m_storage->equityId()) {
            m_storage->loadEquityId(eid);
        }
    } else {
        throw new MYMONEYEXCEPTION (QObject::tr("Can only handle commodity version 2.0.0"));
    }
}

/******************************* readPrices ******************************/

void MyMoneyStorageGNC::readPrices(const QDomElement& pricedb) {
    QDomNode child = pricedb.firstChild();
    while(!child.isNull() && pricedb.isElement()) {
        QDomElement childElement = child.toElement();
        if(QString("price") == childElement.tagName()) {
            readPrice(childElement);
        }
        child = child.nextSibling();
    }
}

/******************************* readPrice ******************************/

void MyMoneyStorageGNC::readPrice(const QDomElement& priceElement) {
    
    QString gncPriceCommodityId, gncPriceCommoditySpace, gncPriceCurrencyId, gncPriceDate, gncPriceValue;
    
    if(priceElement.hasChildNodes())
    {
        QDomNodeList nodeList = priceElement.childNodes();
        if (gncdebug) qDebug("Price has %d children\n", nodeList.count());
        for(unsigned int x = 0; x < nodeList.count(); x++) {
            QDomElement temp = nodeList.item(x).toElement();
            if(temp.hasChildNodes()) {
                QDomText text = temp.firstChild().toText();
                if(QString("price:commodity") == temp.tagName()) {
                    QDomNodeList cmdtyNodeList = temp.childNodes();
                    for(unsigned int cx = 0; cx < cmdtyNodeList.count(); cx++) {
                        QDomElement ctemp = cmdtyNodeList.item(cx).toElement();
                        if(getChildCount(ctemp))  {
                            QDomText ctext = ctemp.firstChild().toText();
                            if(QString("cmdty:id") == ctemp.tagName())  {
                                gncPriceCommodityId = QStringEmpty(ctext.nodeValue());
                            }
                        }
                    }
                } else if(QString("price:currency") == temp.tagName()) {
                    QDomNodeList currNodeList = temp.childNodes();
                    for(unsigned int ux = 0; ux < currNodeList.count(); ux++)  {
                        QDomElement utemp = currNodeList.item(ux).toElement();
                        if(getChildCount(utemp)) {
                            QDomText utext = utemp.firstChild().toText();
                            if(QString("cmdty:id") == utemp.tagName()) {
                                gncPriceCurrencyId = QStringEmpty(utext.nodeValue());
                            } else if (QString("cmdty:space") == utemp.tagName()) {
                                gncPriceCommoditySpace = QStringEmpty(utext.nodeValue());
                            }
                        }
                    }
                } else if(QString("price:time") == temp.tagName()) {
                    QDomNodeList dateNodeList = temp.childNodes();
                    for(unsigned int dx = 0; dx < dateNodeList.count(); dx++) {
                        QDomElement dtemp = dateNodeList.item(dx).toElement();
                        if(getChildCount(dtemp)) {
                            QDomText dtext = dtemp.firstChild().toText();
                            if(QString("ts:date") == dtemp.tagName())  {
                                gncPriceDate = QStringEmpty(dtext.nodeValue());
                            }
                        }
                    }
                } else if(QString("price:value") == temp.tagName()) {
                    gncPriceValue = QStringEmpty(text.nodeValue());
                }
            }
        }
    }
    // temporarily, ignore currency exchange rates
    if (gncPriceCommoditySpace == QString("ISO4217")) return;
    
    // now add this price to the price history
    //we need a IMyMoneyStorage pointer, since m_storage is IMyMoneySerialize.
    IMyMoneyStorage* pStoragePtr = dynamic_cast<IMyMoneyStorage*>(m_storage);
    MyMoneyEquity e = pStoragePtr->equity(m_mapEquities[QCString(gncPriceCommodityId)]);
    if (gncdebug) qDebug ("Searching map, key = %s, found id = %s",
                          gncPriceCommodityId.latin1(), e.id().data());
    
    QStringList fields = QStringList::split(" ", gncPriceDate);
    QString firstField = fields.first();
    QDate priceDate = getDate(firstField);
    
    MyMoneyMoney priceValue(gncPriceValue);
    e.addPriceHistory(priceDate, priceValue);
    
    pStoragePtr->modifyEquity(e);
    
}
//****************************** readAccount *******************************
void MyMoneyStorageGNC::readAccount(const QDomElement& account) { 
    
    MyMoneyAccount acc;
    QCString id;
    QString tmp;
    QString gncName, gncAccountId, gncType, gncDescription, gncParent;
    QString gncCurrencySpace, gncCurrencyId;
    bool bHasParent = false;
    
    QString gncVersion = account.attributes().namedItem(QString("version")).nodeValue();
    if (gncdebug) qDebug("Version of this account object is %s\n", gncVersion.data());
    
    if(QString("2.0.0") == gncVersion)
    {
        QDomNodeList nodeList = account.childNodes();
        if (gncdebug) qDebug("Account has %d children\n", nodeList.count());
        for(unsigned int x = 0; x < nodeList.count(); x++) {
            QDomElement temp = nodeList.item(x).toElement();
            
            if(getChildCount(temp)) {
                QDomText text = temp.firstChild().toText();
                
                if(QString("act:id") == temp.tagName()) {
                    gncAccountId = QStringEmpty(text.nodeValue());
                    if (gncdebug) qDebug("gnucash account id = %s\n", gncAccountId.data());
                } else if(QString("act:name") == temp.tagName()) {
                    gncName = QStringEmpty(text.nodeValue());
                } else if(QString("act:description") == temp.tagName()) {
                    gncDescription = QStringEmpty(text.nodeValue());
                } else if(QString("act:type") == temp.tagName()) {
                    gncType = QStringEmpty(text.nodeValue());
                } else if(QString("act:parent") == temp.tagName()) {
                    bHasParent = true;
                    gncParent = QStringEmpty(text.nodeValue());
                } else if (QString("act:commodity") == temp.tagName()) {
                    QDomNodeList cmdtyNodeList = temp.childNodes();
                    for(unsigned int cx = 0; cx < cmdtyNodeList.count(); cx++) {
                        QDomElement ctemp = cmdtyNodeList.item(cx).toElement();
                        if(getChildCount(ctemp)) {
                            QDomText ctext = ctemp.firstChild().toText();
                            if(QString("cmdty:space") == ctemp.tagName()) {
                                gncCurrencySpace = QStringEmpty(ctext.nodeValue());
                            } else if(QString("cmdty:id") == ctemp.tagName()) {
                                gncCurrencyId = QStringEmpty(ctext.nodeValue());
                            }
                        }
                    }
                }
            }
        } // end for
        // check we haven't got a name which clashes with one of our standard names
        for (unsigned int i = 0; i < m_mainIdCount; i++) {
            if (gncName == m_mainName[i]) gncName = "ex_gnucash_" + gncName;
        } 
        acc.setName(gncName);
        acc.setDescription(gncDescription);
        
        QDate currentDate = QDate::currentDate();
        
        acc.setOpeningDate(currentDate);
        acc.setLastModified(currentDate);
        acc.setLastReconciliationDate(currentDate);
        if (QString("ISO4217") == gncCurrencySpace) {
            acc.setCurrencyId (QCString(gncCurrencyId));
            m_currencyCounter[gncCurrencyId]++;
        }
#ifdef NOSUBFOLDERS
        bHasParent = false;
#endif
        acc.setParentAccountId(QCString(gncParent));
        // now determine the account type and its parent id
#ifdef INVACCT
        if (gncName == INVACCT) {
            acc.setAccountType(MyMoneyAccount::Investment);
            if (!bHasParent)
                acc.setParentAccountId(QCString(m_mainName[m_mainAssetId]));
            gncType = ""; // disable further processing
            m_invAcctStored = true;
        }
#endif
	
        if(QString("BANK") == gncType) {
            acc.setAccountType(MyMoneyAccount::Checkings);
            if (!bHasParent)
                acc.setParentAccountId(QCString(m_mainName[m_mainAssetId]));
        } else if(QString("ASSET") == gncType) {
            acc.setAccountType(MyMoneyAccount::Asset);
            if (!bHasParent)
                acc.setParentAccountId(QCString(m_mainName[m_mainAssetId]));
        } else if(QString("CASH") == gncType) {
            acc.setAccountType(MyMoneyAccount::Cash);
            if (!bHasParent)
                acc.setParentAccountId(QCString(m_mainName[m_mainAssetId]));
        } else if(QString("STOCK") == gncType || QString("MUTUAL") == gncType ) {
	    #ifdef INVACCT
	    if (!m_invAcctStored) {
	        MyMoneyAccount iacc = acc;
		iacc.setName (INVACCT);
		iacc.setAccountType(MyMoneyAccount::Investment);
		iacc.setParentAccountId(QCString(m_mainName[m_mainAssetId]));
		m_storage->addAccount (iacc);
		id = iacc.id();
		m_invAcctStored = true;
	    } else {
	        id = m_invAcctId;
	    }
	    #else
            // need both an investment and a stock account
            acc.setAccountType(MyMoneyAccount::Investment);
            if (!bHasParent)
                acc.setParentAccountId(QCString(m_mainName[m_mainAssetId]));
            m_storage->addAccount(acc);
            id = acc.id();
	    #endif
            m_mapIds[QCString(id)] = QCString(id); // map to ourself so that stock account can be linked as child later
            if (gncdebug) qDebug("Account %s has id of %s, type of %d, parent is %s",
                                 acc.name().data(), id.data(), acc.accountType(), acc.parentAccountId().data());
            //
            acc.setAccountType (MyMoneyAccount::Stock);
            acc.setParentAccountId (id);
            //we need a IMyMoneyStorage pointer, since m_storage is IMyMoneySerialize.
            IMyMoneyStorage* pStoragePtr = dynamic_cast<IMyMoneyStorage*>(m_storage);
            MyMoneyEquity e = pStoragePtr->equity(m_mapEquities[QCString(gncCurrencyId)]);
            if (gncdebug) qDebug ("Acct equity search, key = %s, found id = %s",
                                  gncCurrencyId.latin1(), e.id().data());
            acc.setCurrencyId (e.id());
        } else if(QString("LIABILITY") == gncType  || QString("EQUITY") == gncType) {
            acc.setAccountType(MyMoneyAccount::Liability);
            if (!bHasParent)
                acc.setParentAccountId(QCString(m_mainName[m_mainLiabilityId]));
        } else if(QString("CREDIT") == gncType) {
            acc.setAccountType(MyMoneyAccount::Liability);
            if (!bHasParent)
                acc.setParentAccountId(QCString(m_mainName[m_mainLiabilityId]));
        } else if(QString("INCOME") == gncType) {
            acc.setAccountType(MyMoneyAccount::Income);
            if (!bHasParent)
                acc.setParentAccountId(QCString(m_mainName[m_mainIncomeId]));
        } else if(QString("EXPENSE") == gncType) {
            acc.setAccountType(MyMoneyAccount::Expense);
            if (!bHasParent)
                acc.setParentAccountId(QCString(m_mainName[m_mainExpenseId]));
        }
        
        // all the details from the file about the account should be known by now.
        // calling new account should automatically fill in the account ID.
        m_storage->addAccount(acc);
        id = acc.id();
    } else {
        throw new MYMONEYEXCEPTION (QObject::tr("Can only handle account version 2.0.0"));
    }
    #ifdef INVACCT
    if (acc.accountType() == MyMoneyAccount::Investment) {
        m_invAcctId = id;
    }
    #endif
    // assign the gnucash id as the key into the map to find our id
    m_mapIds[QCString(gncAccountId)] = QCString(id);
    
    if (gncdebug) qDebug("Account %s has id of %s, type of %d, parent is %s",
                         acc.name().data(), id.data(), acc.accountType(), acc.parentAccountId().data());
    
}

//*************************** ReadTransaction ****************************

void MyMoneyStorageGNC::readTransaction(QDomElement& transaction, const bool withinSchedule) {
    
    MyMoneyTransaction tx;
    QCString id;
    QString tmp;
    QString gncTxId, gncDatePosted, gncDateEntered, gncDescription;
    QString gncTxCurrencySpace;
    QStringList fields;
    
    // initialize class variables related to transactions
    m_txChequeNumber = "";
    m_txCommodity = "";
    m_txPayeeId = "";
    m_txMemo = "";
    m_potentialTransfer = true;
    m_assetFound = false;
    Q_ASSERT(m_splitList.isEmpty());
    
    // read the gnucash data
    QString gncVersion = transaction.attributes().namedItem(QString("version")).nodeValue();
    if (gncdebug) qDebug("Version of this transaction object is %s\n", gncVersion.data());
    
    if(QString("2.0.0") == gncVersion)
    {
        QDomNodeList nodeList = transaction.childNodes();
        if (gncdebug) qDebug("Transaction has %d children\n", nodeList.count());
        for(unsigned int x = 0; x < nodeList.count(); x++) {
            QDomElement temp = nodeList.item(x).toElement();
            
            if(getChildCount(temp)) {
                QDomText text = temp.firstChild().toText();
                
                if(QString("trn:id") == temp.tagName()) {
                    gncTxId = QStringEmpty(text.nodeValue());
                    if (gncdebug) qDebug("gnucash transaction id = %s\n", gncTxId.data());
                } else if(QString("trn:num") == temp.tagName()) {
                    m_txChequeNumber = QStringEmpty(text.nodeValue());
                } else if(QString("trn:description") == temp.tagName()) {
                    gncDescription = QStringEmpty(text.nodeValue());
                    if (!gncDescription.isEmpty())
                        m_txPayeeId = createPayee (gncDescription);
                } else if(QString("trn:date-posted") == temp.tagName()) {
                    if(temp.hasChildNodes() && temp.firstChild().isElement()) {
                        QDomElement date = temp.firstChild().toElement();
                        if (gncdebug) qDebug("element is %s", date.tagName().data());
                        QDomText dateText = date.firstChild().toText();
                        gncDatePosted = QStringEmpty(dateText.nodeValue());
                         // save datePosted here coz splits need it
                       fields = QStringList::split(" ", gncDatePosted);
                       if(fields.count()) {
                        QString firstField = fields.first();
                        m_txDatePosted = getDate(firstField);
                        if (gncdebug) qDebug("Post date is %s", m_txDatePosted.toString().data());
                      }
                    }
                } else if(QString("trn:date-entered") == temp.tagName()) {
                    if(temp.hasChildNodes() && temp.firstChild().isElement())
                    {
                        QDomElement date = temp.firstChild().toElement();
                        if (gncdebug) qDebug("element is %s", date.tagName().data());
                        QDomText dateText = date.firstChild().toText();
                        gncDateEntered = QStringEmpty(dateText.nodeValue());
                    }
                } else if (QString("trn:currency") == temp.tagName()) {
                    QDomNodeList cmdtyNodeList = temp.childNodes();
                    for(unsigned int cx = 0; cx < cmdtyNodeList.count(); cx++) {
                        QDomElement ctemp = cmdtyNodeList.item(cx).toElement();
                        if(getChildCount(ctemp)) {
                            QDomText ctext = ctemp.firstChild().toText();
                            if(QString("cmdty:space") == ctemp.tagName()) {
                                gncTxCurrencySpace = QStringEmpty(ctext.nodeValue());
                            } else if(QString("cmdty:id") == ctemp.tagName())  {
                                m_txCommodity = QStringEmpty(ctext.nodeValue());
                            }
                        }
                    }
                } else if(QString("trn:splits") == temp.tagName()) {
                    readSplits(tx, temp);
                }
            }
        } // end for
    } else {
        throw new MYMONEYEXCEPTION (QObject::tr("Can only handle transaction version 2.0.0"));
    }
    
    // all gnucash data read, convert to native format
    // dateEntered
    fields = QStringList::split(" ", gncDateEntered);
    if(fields.count()) {
        QString firstField = fields.first();
        QDate enteredDate = getDate(firstField);
        tx.setEntryDate(enteredDate);
        if (gncdebug) qDebug("Enter date is %s", enteredDate.toString().data());
    }
    tx.setPostDate(m_txDatePosted);
    // memo - set by readSplit
    tx.setMemo(m_txMemo);
    // commodity, saved earlier
    tx.setCommodity(QCString(m_txCommodity));
    
    m_storage->addTransaction(tx, true);
}

//************************ createPayee ***************************

QString MyMoneyStorageGNC::createPayee (QString gncDescription) {
    // looks like we have to search for the payee
    QValueList<MyMoneyPayee> payeeList = m_storage->payeeList();
    QValueList<MyMoneyPayee>::iterator it;
    MyMoneyPayee payee;
    for ( it = payeeList.begin(); it != payeeList.end(); ++it ) {
        if ((*it).name() ==  gncDescription) {
            payee = *it;
            break;
        }
    }
    
    if (it == payeeList.end()) {
        // new payee, add to file
        // for now, we just add the payee to the pool. In the future,
        // we could open a dialog and ask for all the other attributes
        // of the payee.
        payee.setName(gncDescription);
        try {
            m_storage->addPayee(payee);
        } catch(MyMoneyException *e) {
            QMessageBox::critical(0, "KMyMoney2",
                                  QObject::tr("Unable to add payee/receiver\n" +
                                              e->what() + " " + QObject::tr("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
            qFatal("Can't add payee");
        }
    }
    return (payee.id());
}
//************************** readSplits ****************************************

void MyMoneyStorageGNC::readSplits(MyMoneyTransaction& tx, QDomElement& splits) {
    
    MyMoneySplit split;
    QDomNode child = splits.firstChild();
    while(!child.isNull() && child.isElement()) {
        QDomElement childElement = child.toElement();
        if(QString("trn:split") == childElement.tagName()) {
            split = readSplit(tx, childElement);
            saveSplits(tx, split);
        }
        child = child.nextSibling();
    }
    // the splits are in order in splitList. Transfer them to the tx
    // also, determine the action type
    // first off, is it a transfer (can only have 2 splits?)
    if (m_splitList.count() != 2) m_potentialTransfer = false;
    // at this point, if m_potentialTransfer is still true, it is actually one!
    QValueList<MyMoneySplit>::iterator it = m_splitList.begin();
    while (!m_splitList.isEmpty()) {
        split = *it;
        if (m_potentialTransfer) split.setAction(MyMoneySplit::ActionTransfer);
        tx.addSplit(split);
        it = m_splitList.remove(it);
    }
}

//******************************** readSplit ***************************
void MyMoneyStorageGNC::saveSplits (MyMoneyTransaction& tx, MyMoneySplit s) {
    // in kmm, the first split is important. in this routine we will
    // save the splits in our split list with the priority:
    // 1. assets
    // 2. liabilities
    // 3. others (categories)
    
    //we need a IMyMoneyStorage pointer, since m_storage is IMyMoneySerialize.
    IMyMoneyStorage* pStoragePtr = dynamic_cast<IMyMoneyStorage*>(m_storage);
    MyMoneyEquity e;
    MyMoneyMoney price;
    
    switch (m_splitAccount.accountType())   {
        // asset types
    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Asset:
        if (s.value().isNegative()) {
            s.setAction (MyMoneySplit::ActionDeposit);
        } else {
            bool isNumeric = false;
            if (!s.number().isEmpty()) {
                s.number().toLong(&isNumeric);    // No QString.isNumeric()??
            }
            if (isNumeric) {
                s.setAction (MyMoneySplit::ActionCheck);
            } else {
                s.setAction (MyMoneySplit::ActionWithdrawal);
            }
        }
        m_assetFound = true;
        m_splitList.prepend(s);
        break;
    case MyMoneyAccount::CreditCard:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::Liability:
        s.value().isNegative() ? 
                s.setAction (MyMoneySplit::ActionWithdrawal) :
                s.setAction (MyMoneySplit::ActionDeposit);
        m_assetFound ? m_splitList.append(s) : m_splitList.prepend(s);
        break;
    case MyMoneyAccount::Stock:
        s.value() == MyMoneyMoney(0) ?
                s.setAction (MyMoneySplit::ActionAddShares)  : // free shares?
                s.setAction (MyMoneySplit::ActionBuyShares);
        //qDebug ("Type %s, value %d, qty %d", s.action().data(), s.shares().toDouble(), s.value().toDouble());
        m_potentialTransfer = false;
        m_splitList.append (s);
        // add a price history entry
        //we need a IMyMoneyStorage pointer, since m_storage is IMyMoneySerialize.
        e = pStoragePtr->equity(m_splitAccount.currencyId());
        price = s.value() / s.shares();
        e.addPriceHistory(tx.postDate(), price);
        if (gncdebug) qDebug ("added price for %s, %s date %s",
                e.name().latin1(), price.toString().latin1(), tx.postDate().toString(Qt::ISODate).latin1());
        pStoragePtr->modifyEquity(e);
        break;
    default:
        m_potentialTransfer = false;
        m_splitList.append (s);
    }
    // backdate the account opening date if necessary
    if (m_txDatePosted < m_splitAccount.openingDate()) {
        if (gncdebug) qDebug ("changing opening date for %s from %s to %s",
          m_splitAccount.name().latin1(), m_splitAccount.openingDate().toString(Qt::ISODate).latin1(), m_txDatePosted.toString(Qt::ISODate).latin1());
        m_splitAccount.setOpeningDate(m_txDatePosted);
        // we need a IMyMoneyStorage pointer, since m_storage is IMyMoneySerialize.
        IMyMoneyStorage* pStoragePtr = dynamic_cast<IMyMoneyStorage*>(m_storage);
        pStoragePtr->modifyAccount(m_splitAccount);
    }
}

//******************************** readSplit ***************************

MyMoneySplit MyMoneyStorageGNC::readSplit(MyMoneyTransaction& tx, QDomElement& splitElement) {
    
    MyMoneySplit split;
    QString strTmp;
    QString gncSplitId, gncSplitMemo, gncSplitReconciledState, gncSplitValue,
    gncSplitQuantity, gncSplitAccount, gncDateReconciled;
    
    // load the gnucash data
    if(splitElement.hasChildNodes()) {
        QDomNodeList nodeList = splitElement.childNodes();
        if (gncdebug) qDebug("Split has %d children\n", nodeList.count());
        for(unsigned int x = 0; x < nodeList.count(); x++) {
            QDomElement temp = nodeList.item(x).toElement();
            
            if(temp.hasChildNodes()) {
                QDomText text = temp.firstChild().toText();
                if(QString("split:id") == temp.tagName()) {
                    gncSplitId = QStringEmpty(text.nodeValue());
                    if (gncdebug) qDebug("gnucash split id = %s\n", gncSplitId.data());
                } else if(QString("split:memo") == temp.tagName()) {
                    gncSplitMemo = QStringEmpty(text.nodeValue());
                } else if(QString("split:reconciled-state") == temp.tagName()) {
                    gncSplitReconciledState = QStringEmpty(text.nodeValue());
                } else if(QString("split:reconcile-date") == temp.tagName()) {
                    if(temp.hasChildNodes() && temp.firstChild().isElement()) {
                        QDomElement date = temp.firstChild().toElement();
                        if (gncdebug) qDebug("element is %s", date.tagName().data());
                        QDomText dateText = date.firstChild().toText();
                        gncDateReconciled = QStringEmpty(dateText.nodeValue());
                    }
                } else if(QString("split:value") == temp.tagName()) {
                    gncSplitValue = QStringEmpty(text.nodeValue());
                } else if(QString("split:quantity") == temp.tagName()) {
                    gncSplitQuantity = QStringEmpty(text.nodeValue());
                } else if(QString("split:account") == temp.tagName()) {
                    gncSplitAccount = QStringEmpty(text.nodeValue());
                    
                    map_accountIds::Iterator id = m_mapIds.find(QCString(gncSplitAccount));
                    if(id != m_mapIds.end()) {
                        if (gncdebug) qDebug("Split:  Swapping account id %s's with our account id %s", gncSplitAccount.data(), id.data().data());
                        gncSplitAccount = id.data();
                    }
                }
            }
        }
    }
    
    // convert gnucash data to native format
    // payee id
    split.setPayeeId(QCString(m_txPayeeId));
    // reconcile state and date
    if(QString("n") == gncSplitReconciledState) {
        split.setReconcileFlag(MyMoneySplit::NotReconciled);
    } else if(QString("c") == gncSplitReconciledState) {
        split.setReconcileFlag(MyMoneySplit::Cleared);
    } else if(QString("y") == gncSplitReconciledState) {
        split.setReconcileFlag(MyMoneySplit::Reconciled);
    }
    
    QStringList fields = QStringList::split(" ", gncDateReconciled);
    if(fields.count()) {
        QString firstField = fields.first();
        QDate reconciledDate = getDate(firstField);
        split.setReconcileDate(reconciledDate);
        if (gncdebug) qDebug("Recon Date is %s", reconciledDate.toString().data());
    }
    // memo
    // Arbitrarily, set the tx memo to the first non-null split memo
    // I think this is necessary because gnc txs with just 2 splits (the majority)
    // are not viewable as split transactions in kmm so the split memo is not seen
    if ((m_txMemo.isEmpty()) && (!gncSplitMemo.isEmpty()))
        m_txMemo = gncSplitMemo;
    split.setMemo(gncSplitMemo);
    
    // value and quantity
    MyMoneyMoney splitValue(gncSplitValue);
    MyMoneyMoney splitQuantity(gncSplitQuantity);
    /*  if(splitValue.isNegative())
  {
    split.setAction(MyMoneySplit::ActionWithdrawal);
  }
  else
  {
    split.setAction(MyMoneySplit::ActionDeposit);
  } */
    // number
    if (!m_txChequeNumber.isEmpty()) {
        split.setNumber(m_txChequeNumber); /*
    bool isNumeric;
    m_txChequeNumber.toLong(&isNumeric);    // No QString.isNumeric()??
    if ((split.action() == MyMoneySplit::ActionWithdrawal) &&
        (isNumeric))
      split.setAction (MyMoneySplit::ActionCheck); */
    }
    // accountId
    split.setAccountId (QCString(gncSplitAccount));
    
    // now we need to check if there is some form of currency conversion between the split
    // account and the tx
    // first, find the account type
    // doc says following returns a qmap, but it appears not. pity.
    QValueList<MyMoneyAccount> accList = m_storage->accountList();
    QValueList<MyMoneyAccount>::iterator it;
    for ( it = accList.begin(); it != accList.end(); ++it ) {
        if ((*it).id() ==  QCString(gncSplitAccount)) {
            break;
        }
    }
    if (it == accList.end()) //
        throw new MYMONEYEXCEPTION (QObject::tr("Can't find split account in readSplit"));
    m_splitAccount = *it;  // save for later
    
    /*//FIXME - handling of investments still to be determined
  if ((*it).currencyId() != QCString(m_txCommodity))
  {
    split.setValue(splitQuantity, QCString(m_txCommodity), (*it).currencyId()); // actually setShares?
    split.setValue(splitValue);
  } else {
    split.setValue(splitValue);
  } */
    split.setValue (splitValue);
    split.setShares (splitQuantity);
    
    return split;
}

//*****************************readTemplates **********************************

void MyMoneyStorageGNC::readTemplates(QDomElement& templates) {
    QDomNode child = templates.firstChild();
    while(!child.isNull() && templates.isElement())
    {
        QDomElement childElement = child.toElement();
        if(QString("gnc:transaction") == childElement.tagName()) {
            m_templateList.append(readTemplate(childElement));
        }
        child = child.nextSibling();
    }
}

//***************************** readTemplate **********************************

gncTemplateTx MyMoneyStorageGNC::readTemplate(QDomElement& templatetx) {
    
    gncTemplateTx tx = {false}; // set not suspect
    QString gncTxId, gncDatePosted, gncDateEntered, gncDescription;
    QString gncTxCurrencySpace;
    
    // initialize class variables related to transactions
    m_txChequeNumber = "";
    m_txCommodity = "";
    m_txPayeeId = "";
    m_txMemo = "";
    m_templateId = "";
    m_potentialTransfer = true;
    m_assetFound = false;
    Q_ASSERT(m_splitList.isEmpty());
    
    QString gncVersion = templatetx.attributes().namedItem(QString("version")).nodeValue();
    if (gncdebug) qDebug("Version of this template object is %s\n", gncVersion.data());
    
    if(QString("2.0.0") == gncVersion) {
        QDomNodeList nodeList = templatetx.childNodes();
        if (gncdebug) qDebug("template has %d children\n", nodeList.count());
        for(unsigned int x = 0; x < nodeList.count(); x++) {
            QDomElement temp = nodeList.item(x).toElement();
            
            if(getChildCount(temp)) {
                QDomText text = temp.firstChild().toText();
                
                if(QString("trn:num") == temp.tagName()) {
                    m_txChequeNumber = QStringEmpty(text.nodeValue());
                } else if(QString("trn:description") == temp.tagName()) {
                    gncDescription = QStringEmpty(text.nodeValue());
                    if (!gncDescription.isEmpty())
                        m_txPayeeId = createPayee (gncDescription);
                } else if(QString("trn:date-posted") == temp.tagName()) {
                    if(temp.hasChildNodes() && temp.firstChild().isElement()) {
                        QDomElement date = temp.firstChild().toElement();
                        if (gncdebug) qDebug("element is %s", date.tagName().data());
                        QDomText dateText = date.firstChild().toText();
                        gncDatePosted = QStringEmpty(dateText.nodeValue());
                    }
                } else if(QString("trn:date-entered") == temp.tagName()) {
                    if(temp.hasChildNodes() && temp.firstChild().isElement()) {
                        QDomElement date = temp.firstChild().toElement();
                        if (gncdebug) qDebug("element is %s", date.tagName().data());
                        QDomText dateText = date.firstChild().toText();
                        gncDateEntered = QStringEmpty(dateText.nodeValue());
                    }
                } else if (QString("trn:currency") == temp.tagName()) {
                    QDomNodeList cmdtyNodeList = temp.childNodes();
                    for(unsigned int cx = 0; cx < cmdtyNodeList.count(); cx++) {
                        QDomElement ctemp = cmdtyNodeList.item(cx).toElement();
                        if(getChildCount(ctemp))  {
                            QDomText ctext = ctemp.firstChild().toText();
                            if(QString("cmdty:space") == ctemp.tagName())  {
                                gncTxCurrencySpace = QStringEmpty(ctext.nodeValue());
                            } else if(QString("cmdty:id") == ctemp.tagName()) {
                                m_txCommodity = QStringEmpty(ctext.nodeValue());
                            }
                        }
                    }
                } else if(QString("trn:splits") == temp.tagName()) {
                    readTemplateSplits(tx, temp);
                }
            }
        } // end for
    } else {
        throw new MYMONEYEXCEPTION (QObject::tr("Can only handle template version 2.0.0"));
    }
    // we have all data for this template
    // since this is just a template tx, we can use the gnucash id
    // this will be referenced by the schedxaction and not used after
    tx.t.setId(QCString(m_templateId));
    // datePosted
    QStringList fields = QStringList::split(" ", gncDatePosted);
    if(fields.count()) {
        QString firstField = fields.first();
        QDate postedDate = getDate(firstField);
        tx.t.setPostDate(postedDate);
        if (gncdebug) qDebug("Date is %s", postedDate.toString().data());
    }
    // dateEntered
    fields = QStringList::split(" ", gncDateEntered);
    if(fields.count()) {
        QString firstField = fields.first();
        QDate enteredDate = getDate(firstField);
        tx.t.setEntryDate(enteredDate);
        if (gncdebug) qDebug("Date is %s", enteredDate.toString().data());
    }
    // memo - set by readSplit
    tx.t.setMemo(m_txMemo);
    // commodity, saved earlier
    tx.t.setCommodity(QCString(m_txCommodity));
    return (tx);
    
}
//************************** readTemplateSplits ****************************************

void MyMoneyStorageGNC::readTemplateSplits(gncTemplateTx& tx, QDomElement& splits) {
    MyMoneySplit split;
    
    QDomNode child = splits.firstChild();
    while(!child.isNull() && child.isElement()) {
        QDomElement childElement = child.toElement();
        if(QString("trn:split") == childElement.tagName()){
            split = readTemplateSplit(tx, childElement);
            saveSplits(tx.t, split);
        }
        child = child.nextSibling();
    }
    // the splits are in order in splitList. Transfer them to the tx
    // also, determine the action type
    // first off, is it a transfer (can only have 2 splits?)
    if (m_splitList.count() != 2) m_potentialTransfer = false;
    // at this point, if m_potentialTransfer is still true, it is actually one!
    // but work out the other type in case...
    if (m_splitList.first().value().isNegative()) {
        m_splitActionType = MyMoneySplit::ActionWithdrawal;
    } else {
        m_splitActionType = MyMoneySplit::ActionDeposit;
    }
    
    QValueList<MyMoneySplit>::iterator it = m_splitList.begin();
    while (!m_splitList.isEmpty()) {
        split = *it;
        if (m_potentialTransfer) {
            split.setAction(MyMoneySplit::ActionTransfer);
        } else {
            split.setAction (QCString(m_splitActionType));
        }
        tx.t.addSplit(split);
        it = m_splitList.remove(it);
    }
}

//******************************** readTemplateSplit ***************************

MyMoneySplit MyMoneyStorageGNC::readTemplateSplit(gncTemplateTx& tx, QDomElement& splitElement) {
    
    MyMoneySplit split;
    QString strTmp;
    QString gncSplitId, gncSplitMemo, gncSplitValue, gncSplitQuantity, gncSplitAccount;
    
    
    // load the gnucash data
    if(splitElement.hasChildNodes()) {
        QDomNodeList nodeList = splitElement.childNodes();
        if (gncdebug) qDebug("Template Split has %d children\n", nodeList.count());
        for(unsigned int x = 0; x < nodeList.count(); x++) {
            QDomElement temp = nodeList.item(x).toElement();
            
            if(temp.hasChildNodes()) {
                QDomText text = temp.firstChild().toText();
                if(QString("split:account") == temp.tagName())  {
                    m_templateId = QStringEmpty(text.nodeValue());
                }  else if(QString("split:slots") == temp.tagName())  {
                    if (!readSplitSlots(split, temp)) {
                        tx.suspectFlag = true;
                    }
                }
            }
        } //end for
    }
    
    // convert gnucash data to native format
    // action, value and account will have been set from slots
    // reconcile state, always not
    split.setReconcileFlag(MyMoneySplit::NotReconciled);
    // memo
    // Arbitrarily, set the tx memo to the first non-null split memo
    // I think this is necessary because gnx txs with just 2 splits (the majority)
    // are not viewable as split transactions in kmm so the split memo is not seen
    if ((m_txMemo.isEmpty()) && (!gncSplitMemo.isEmpty()))
        m_txMemo = gncSplitMemo;
    split.setMemo(gncSplitMemo);
    // number
    if (!m_txChequeNumber.isEmpty()) {
        split.setNumber(m_txChequeNumber);
    }
    
    return split;
}

//******************************** readSplitSlots ***************************

bool MyMoneyStorageGNC::readSplitSlots(MyMoneySplit& split, QDomElement& splitSlots) {
    bool bRc = true;  // default return value
    unsigned int iValidScheds = 0;
    
    // load the gnucash data
    if(splitSlots.hasChildNodes()){
        QDomNodeList nodeList = splitSlots.childNodes();
        if (gncdebug) qDebug("Split slot has %d children\n", nodeList.count());
        for(unsigned int x = 0; x < nodeList.count(); x++) {
            QDomElement temp = nodeList.item(x).toElement();
            
            if(temp.hasChildNodes()) {
                if(QString("slot") == temp.tagName()) {
                    QDomNodeList tmp2 = temp.childNodes();
                    QString tag1 = tmp2.item(0).toElement().tagName();
                    QString value1 = tmp2.item(0).toElement().firstChild().toText().nodeValue();
                    QString tag2 = tmp2.item(1).toElement().tagName();
                    QString attr2 =
                            tmp2.item(1).toElement().attributes().namedItem(QString("type")).nodeValue();
                    //if (gncdebug) qDebug ("tag1 = %s", tag1.latin1());
                    //if (gncdebug) qDebug ("value1 = %s", value1.latin1());
                    //if (gncdebug) qDebug ("tag2 = %s", tag2.latin1());
                    //if (gncdebug) qDebug ("attr2 = %s",attr2.latin1());
                    
                    // all the following looks ridiculous but the only example I have in front of me
                    // has all these values. Until we have a full definition of the options available,
                    // this is the only type I know how to convert
                    if ((QString("slot:key") == tag1)  &&
                        (QString("sched-xaction") == value1) &&
                        (QString("slot:value") == tag2) &&
                        (QString("frame") == attr2)) {
                        QDomElement slot = tmp2.item(1).toElement();
                        if (convertSplitSlot (split, slot))
                            iValidScheds++;
                    }
                }
            }
        } // end for
    }
    
    // we should find one and only one valid schedule slot, else we have a
    // potential error that the user must resolve
    if (iValidScheds != 1) bRc = false;
    
    return bRc;
}

//******************************** convertSplitSlot ***************************

bool MyMoneyStorageGNC::convertSplitSlot(MyMoneySplit& split, QDomElement& slot) {
    bool bRc = false;
    bool bFoundStringCreditFormula = false;
    bool bFoundStringDebitFormula = false;
    bool bFoundGuidAccountId = false;
    QString gncCreditFormula, gncDebitFormula, gncAccountId;
    
    QDomNodeList sslots = slot.childNodes();
    if (gncdebug) qDebug ("%s has %d children", slot.tagName().latin1(), getChildCount(slot));
    
    for (unsigned int x = 0; x < getChildCount(slot); x++) {
        QDomElement slot = sslots.item(x).toElement();
        
        QDomNodeList temp = slot.childNodes();
        QString tag1 = temp.item(0).toElement().tagName();
        QString value1 = temp.item(0).toElement().firstChild().toText().nodeValue();
        QString tag2 = temp.item(1).toElement().tagName();
        QString value2 = temp.item(1).toElement().firstChild().toText().nodeValue();
        QString attr2 =
                temp.item(1).toElement().attributes().namedItem(QString("type")).nodeValue();
        //if (gncdebug) qDebug ("tag1 = %s", tag1.latin1());
        //if (gncdebug) qDebug ("value1 = %s", value1.latin1());
        //if (gncdebug) qDebug ("tag2 = %s", tag2.latin1());
        //if (gncdebug) qDebug ("value2 = %s",value2.latin1());
        //if (gncdebug) qDebug ("attr2 = %s",attr2.latin1());
        // again, see comments above. when we have a full specification
        // of all the options available to us, we can no doubt improve on this
        if ((QString("slot:key") == tag1) && (QString("slot:value") == tag2)) {
            if ((QString("credit-formula") == value1) && (QString("string") == attr2)) {
                gncCreditFormula = value2;
                bFoundStringCreditFormula = true;
            }
            if ((QString("debit-formula") == value1) && (QString("string") == attr2)) {
                gncDebitFormula = value2;
                bFoundStringDebitFormula = true;
            }
            if ((QString("account") == value1) && (QString("guid") == attr2)) {
                gncAccountId = value2;
                bFoundGuidAccountId = true;
            }
        }
    }
    
    // all data read, now check we have everything
    if ((bFoundStringCreditFormula) && (bFoundStringDebitFormula) && (bFoundGuidAccountId))
        bRc = true;
    
    // now convert data to native format
    map_accountIds::Iterator id = m_mapIds.find(QCString(gncAccountId));
    if(id != m_mapIds.end()) {
        if (gncdebug) qDebug("Split slot:  Swapping account id %s's with our account id %s", gncAccountId.data(), id.data().data());
        gncAccountId = id.data();
    }
    split.setAccountId(QCString(gncAccountId));
    // payee id
    split.setPayeeId(QCString(m_txPayeeId));
    
    // now we need to check if there is some form of currency conversion between the split
    // account and the tx
    // doc says following returns a qmap, but it appears not. pity.
    QValueList<MyMoneyAccount> accList = m_storage->accountList();
    QValueList<MyMoneyAccount>::iterator it;
    for ( it = accList.begin(); it != accList.end(); ++it ) {
        if ((*it).id() ==  QCString(split.accountId())) {
            break;
        }
    }
    if (it == accList.end())
        throw new MYMONEYEXCEPTION (QObject::tr("Can't find split account in readSplit"));
    m_splitAccount = *it;
    if (!gncCreditFormula.isEmpty()) {
        split.setValue(MyMoneyMoney("-" + gncCreditFormula));
    } else if (!gncDebitFormula.isEmpty()) {
        split.setValue(MyMoneyMoney(gncDebitFormula));
    }
    return bRc;
}

// ****************************** readSchedule *******************************

void MyMoneyStorageGNC::readSchedule(QDomElement& schedule) {
    
    QDomNode child = schedule.firstChild();
    
    signalProgress(0, getChildCount(schedule), QObject::tr("Loading schedules..."));
    // variables to hold gnucash string data
    QString gncName, gncAutoCreate, gncAutoCreateNotify, gncAutoCreateDays,
    gncAdvanceCreateDays, gncAdvanceRemindDays, gncInstanceCount,
    gncStartDate, gncLastDate, gncNumOccur, gncRemOccur, gncTemplateId;
    gncFreqSpec fs = {false}; // suspectflag = false to start
    // to hold our data
    MyMoneySchedule sc;
    bool suspectFlag = false;
    // for date calculations
    unsigned char interval;  // day, week, month, year
    unsigned int intervalCount;
    QDate today = QDate::currentDate();
    QDate startDate, lastDate, nextDate;
    int numOccurs;
    
    //fs.suspectFlag = false;  // assume we're good to start with
    
    QString gncVersion = schedule.attributes().namedItem(QString("version")).nodeValue();
    if (gncdebug) qDebug("Version of this schedule object is %s\n", gncVersion.data());
    if(QString("1.0.0") == gncVersion) {
        QDomNodeList nodeList = schedule.childNodes();
        if (gncdebug) qDebug("schedule has %d children\n", nodeList.count());
        for(unsigned int x = 0; x < nodeList.count(); x++) {
            QDomElement temp = nodeList.item(x).toElement();
            
            if(getChildCount(temp)) {
                QDomText text = temp.firstChild().toText();
                
                if(QString("sx:id") == temp.tagName()) {
                    // nowt to do here
                } else if(QString("sx:name") == temp.tagName()) {
                    gncName = QStringEmpty(text.nodeValue());
                } else if(QString("sx:autoCreate") == temp.tagName()) {
                    gncAutoCreate = QStringEmpty(text.nodeValue());
                } else if(QString("sx:autoCreateNotify") == temp.tagName()) {
                    gncAutoCreateNotify = QStringEmpty(text.nodeValue());
                } else if(QString("sx:autoCreateDays") == temp.tagName()) {
                    gncAutoCreateDays = QStringEmpty(text.nodeValue());
                } else if(QString("sx:advanceCreateDays") == temp.tagName()) {
                    gncAdvanceCreateDays = QStringEmpty(text.nodeValue());
                } else if(QString("sx:advanceRemindDays") == temp.tagName()) {
                    gncAdvanceRemindDays = QStringEmpty(text.nodeValue());
                } else if(QString("sx:instanceCount") == temp.tagName()) {
                    gncInstanceCount = QStringEmpty(text.nodeValue());
                } else if(QString("sx:start") == temp.tagName()) {
                    if (temp.hasChildNodes()) {
                        gncStartDate =
                                QStringEmpty(temp.firstChild().toElement().firstChild().toText().nodeValue());
                        if (gncdebug) qDebug ("Sched start %s", gncStartDate.latin1());
                    }
                } else if(QString("sx:last") == temp.tagName()) {
                    if (temp.hasChildNodes()) {
                        gncLastDate =
                                QStringEmpty(temp.firstChild().toElement().firstChild().toText().nodeValue());
                    }
                } else if(QString("sx:num-occur") == temp.tagName()) {
                    gncNumOccur = QStringEmpty(text.nodeValue());
                } else if(QString("sx:rem-occur") == temp.tagName()) {
                    gncRemOccur = QStringEmpty(text.nodeValue());
                } else if(QString("sx:templ-acct") == temp.tagName()) {
                    gncTemplateId = QStringEmpty(text.nodeValue());
                    if (gncdebug) qDebug ("TemplateId = %s", gncTemplateId.latin1());
                } else if (QString("sx:freqspec") == temp.tagName()) {
                    QDomElement freqspec = temp.firstChild().toElement();
                    if (QString("gnc:freqspec") ==  freqspec.tagName()) {
                        fs.name = gncName;
                        readFreqSpec (fs, freqspec, false);
                        if (fs.suspectFlag)
                            suspectFlag = true;
                    }
                } else {
                    if (gncdebug) qDebug ("Don't know how to handle %s in sched-xaction for %s",
                                          temp.tagName().latin1(), gncName.latin1());
                    suspectFlag = true;
                }
            }
        }
    } else {
        throw new MYMONEYEXCEPTION (QObject::tr("Can only handle schedule version 1.0.0"));
    }
    
    // we have all the data, now create the schedule (well, try anyway)
    // find the transaction template
    gncTemplateTx ttx;
    QValueList<gncTemplateTx>::iterator itt;
    for (itt = m_templateList.begin(); itt != m_templateList.end(); ++itt) {
        if ((*itt).t.id() == (QCString(gncTemplateId))) break;
    }
    if (itt == m_templateList.end()) {
        suspectFlag = true;
    } else {
        ttx = *itt;
        ttx.t.setId("");
        sc.setTransaction(ttx.t);
        if (ttx.suspectFlag)
            suspectFlag = true;
    }
    
    // startDate
    QStringList fields = QStringList::split(" ", gncStartDate);
    if(fields.count()) {
        QString firstField = fields.first();
        startDate = getDate(firstField);
        sc.setStartDate(startDate);
    }
    // auto enter
    if (QString("y") == gncAutoCreate) {
        sc.setAutoEnter(true);
    } else {
        sc.setAutoEnter(false);
    }
    // set the occurrence interval
    if (QString("daily") == fs.intvlType) {
        sc.setOccurence((MyMoneySchedule::occurenceE)MyMoneySchedule::OCCUR_DAILY);
        interval = 'd';
        intervalCount = 1;
    } else if (QString("weekly") == fs.intvlType) {
        sc.setOccurence((MyMoneySchedule::occurenceE)MyMoneySchedule::OCCUR_WEEKLY);
        interval = 'w';
        intervalCount = 1;
    } else if (QString("bi-weekly") == fs.intvlType) {
        sc.setOccurence((MyMoneySchedule::occurenceE)MyMoneySchedule::OCCUR_FORTNIGHTLY);
        interval = 'w';
        intervalCount = 2;
    } else if (QString("monthly") == fs.intvlType) {
        sc.setOccurence((MyMoneySchedule::occurenceE)MyMoneySchedule::OCCUR_MONTHLY);
        interval = 'm';
        intervalCount = 1;
    }   else if (QString("quarterly") == fs.intvlType) {
        sc.setOccurence((MyMoneySchedule::occurenceE)MyMoneySchedule::OCCUR_QUARTERLY);
        interval = 'm';
        intervalCount = 3;
    } else if (QString("yearly") == fs.intvlType) {
        sc.setOccurence((MyMoneySchedule::occurenceE)MyMoneySchedule::OCCUR_YEARLY);
        interval = 'y';
        intervalCount = 1;
    }
    // work out the last payment date, set up to work out end date if any
    numOccurs = gncNumOccur.toUInt();
    nextDate = startDate;
    while (nextDate < today) {
        lastDate = nextDate;
        nextDate = incrDate (lastDate, interval, intervalCount);
        numOccurs--;
    }
    sc.setLastPayment(lastDate);
    if (nextDate == today) {
        QString dupMess =
                (QObject::tr(QString().sprintf("Scheduled transaction %s due today.\n"
                                               "Please check for duplicate post.", gncName.latin1())));
        QMessageBox::information (0, "KMyMoney2", dupMess);
    }
    
    // payment type
    sc.setPaymentType((MyMoneySchedule::paymentTypeE)MyMoneySchedule::STYPE_OTHER);
    // if the input file had a number of occurs and still some left
    // work out the final date
    if (numOccurs > 0) {
        while (numOccurs-- > 0) {
            lastDate = nextDate;
            nextDate = incrDate (lastDate, interval, intervalCount);
        }
        sc.setEndDate(lastDate);
    }
    // type
    QCString actionType = ttx.t.splits().first().action();
    if (actionType == MyMoneySplit::ActionDeposit) {
        sc.setType((MyMoneySchedule::typeE)MyMoneySchedule::TYPE_DEPOSIT);
    } else if (actionType == MyMoneySplit::ActionTransfer) {
        sc.setType((MyMoneySchedule::typeE)MyMoneySchedule::TYPE_TRANSFER);
    } else {
        sc.setType((MyMoneySchedule::typeE)MyMoneySchedule::TYPE_BILL);
    }
    // schedule name
    sc.setName(gncName);
    // if the tx is suspect set Fixed to false
    // (gnucash doesn't have variable txs)
    if (ttx.suspectFlag) {
        sc.setFixed(false);
    } else {
        sc.setFixed(true);
    }
    sc.setWeekendOption((MyMoneySchedule::weekendOptionE)MyMoneySchedule::MoveNothing);
    //FIXME - replace this with a proper id assigment function (where?)
    // assign the next available id - replace with newSchedule?()
#define SCHEDULE_ID_SIZE 6
    QCString id;
    id.setNum(m_storage->scheduleId()+1);
    id = "SCH" + id.rightJustify(SCHEDULE_ID_SIZE, '0');
    sc.setId(QCString(id));
    //tell the storage objects we have a new schedule object.
    m_storage->loadSchedule(sc);
    
    unsigned int scid = extractId(sc.id().data());
    if(scid > m_storage->scheduleId()) {
        m_storage->loadScheduleId(scid);
    }
    
    if (suspectFlag) {
        QString susMess =
                (QObject::tr(QString().sprintf
                             ("Scheduled transaction %s contains data which may not have been handled correctly\n"
                              "Please check for correct operation.", gncName.latin1())));
        QMessageBox::information (0, "KMyMoney2", susMess);
    }
    return;
}

// *************************** readFreqSpec *********************
void MyMoneyStorageGNC::readFreqSpec (gncFreqSpec& fs, QDomElement& fspec, bool isComposite)
{
    
    QString gncVersion = fspec.attributes().namedItem(QString("version")).nodeValue();
    if (gncdebug) qDebug("Version of this fspec object is %s\n", gncVersion.data());
    if(QString("1.0.0") == gncVersion)
    {
        QDomNodeList nodeList = fspec.childNodes();
        if (gncdebug) qDebug("fspec has %d children\n", nodeList.count());
        for(unsigned int x = 0; x < nodeList.count(); x++) {
            QDomElement temp = nodeList.item(x).toElement();
            if (getChildCount (temp)) {
                if (QString("fs:id") == temp.tagName()) {
                    // nothing to do
                }  else if (QString("fs:ui_type") == temp.tagName())  {
                    fs.intvlType = QStringEmpty(temp.firstChild().toText().nodeValue());
                }  else if ((QString("fs:monthly") == temp.tagName()) ||
                            (QString("fs:daily") == temp.tagName()) ||
                            (QString("fs:weekly") == temp.tagName()))  {
                    QDomNodeList ms = temp.childNodes();
                    for (unsigned int mx = 0; mx < ms.count(); mx++)  {
                        QDomElement me = ms.item(mx).toElement();
                        QDomText text = me.firstChild().toText();
                        if (QString("fs:id") == me.tagName())  {
                            // do nothing with this
                        } else if (QString("fs:interval") == me.tagName()) {
                            fs.intvlInterval = QStringEmpty(text.nodeValue());
                        } else  if (QString("fs:offset") == me.tagName()) {
                            fs.intvlOffset = QStringEmpty(text.nodeValue());
                        } else  if (QString("fs:day") == me.tagName()) {
                            fs.intvlDay = QStringEmpty(text.nodeValue());
                        }
                    }
                } else if (QString("fs:composite") == temp.tagName()) {
                    // not quite sure what composite represents, looks like this!!
                    gncFreqSpec fsc;
                    QDomElement freqspec = temp.firstChild().toElement();
                    if (QString("gnc:freqspec") ==  freqspec.tagName())  {
                        fsc.name = fs.name;
                        fsc.suspectFlag = false;
                        readFreqSpec (fsc, freqspec, true);
                        if (fsc.suspectFlag)
                            fs.suspectFlag = true;
                        fs.intvlInterval = fsc.intvlInterval;
                        fs.intvlOffset = fsc.intvlOffset;
                    }
                }  else { // not fs:monthly, daily, weekly
                    if (gncdebug) qDebug ("Don't know how to handle %s in freqspec for %s",
                                          temp.tagName().latin1(), fs.name.latin1());
                    fs.suspectFlag = true;
                }
            }
        }
    }
    if (gncdebug) qDebug ("%s %s %s %s", fs.intvlType.latin1(), fs.intvlInterval.latin1(),
                          fs.intvlOffset.latin1(), fs.intvlDay.latin1());
}

//****************************** incrDate ****************************
QDate MyMoneyStorageGNC::incrDate
        (QDate lastDate, unsigned char interval, unsigned int intervalCount) {
    switch (interval) {
    case 'd':
        return (lastDate.addDays(intervalCount));
    case 'w':
        return (lastDate.addDays(intervalCount * 7));
    case 'm':
        return (lastDate.addMonths(intervalCount));
    case 'y':
        return (lastDate.addYears(intervalCount));
    }
    throw new MYMONEYEXCEPTION(QObject::tr("invalid interval char in incrDate"));
    return (QDate(0, 0, 0)); // to keep compiler happy
}

//********************** extractId ***********************************

const unsigned long MyMoneyStorageGNC::extractId(const QCString& txt) const {
    int pos;
    unsigned long rc = 0;
    
    pos = txt.find(QRegExp("\\d+"), 0);
    if(pos != -1) {
        rc = atol(txt.mid(pos));
    }
    return rc;
}

//*************************** getDate *********************************

QDate MyMoneyStorageGNC::getDate(const QString& strText) const {
    QDate date;
    if(strText.length())
    {
        QDate date = QDate::fromString(strText, Qt::ISODate);
        if(!date.isNull() && date.isValid()) {
            return date;
        } else {
            return QDate();
        }
    }
    
    return date;
}

//************************** getString **************************

QString MyMoneyStorageGNC::getString(const QDate& date) const {
    QString str("");
    if(!date.isNull() && date.isValid()) {
        str = date.toString(Qt::ISODate);
    }
    
    return str;
}

//********************************* checkConsistency **********************************
MyMoneyAccount MyMoneyStorageGNC::checkConsistency (MyMoneyAccount& parent, MyMoneyAccount& child) {
    // gnucash is flexible/weird enough to allow various inconsistencies
    // these are a couple I found in my file, no doubt more will be discovered
    if ((child.accountType() == MyMoneyAccount::Investment) &&
        (parent.accountType() != MyMoneyAccount::Asset)) {
        qWarning (QObject::tr("KMyMoney requires an Investment account be a child of an Asset account"));
        qWarning (QObject::tr(QString().sprintf("Account %s will be stored under the main Asset account",
                                                child.name().latin1())));
        m_inconsistentInput++;
        return m_storage->asset();
    }
    if ((child.accountType() == MyMoneyAccount::Income) &&
        (parent.accountType() != MyMoneyAccount::Income)) {
        qWarning (QObject::tr("KMyMoney requires an Income account be a child of an Income account"));
        qWarning (QObject::tr(QString().sprintf("Account %s will be stored under the main Income account",
                                                child.name().latin1())));
        m_inconsistentInput++;
        return m_storage->income();
    }
    if ((child.accountType() == MyMoneyAccount::Expense) &&
        (parent.accountType() != MyMoneyAccount::Expense)) {
        qWarning (QObject::tr("KMyMoney requires an Expense account be a child of an Expense account"));
        qWarning (QObject::tr(QString().sprintf("Account %s will be stored under the main Expense account",
                                                child.name().latin1())));
        m_inconsistentInput++;
        return m_storage->expense();
    }
    return (parent);
}

//********************** findChildElement ***************************

QDomElement MyMoneyStorageGNC::findChildElement(const QString& name, const QDomElement& root) {
    QDomNode child = root.firstChild();
    while(!child.isNull())   {
        if(child.isElement()) {
            QDomElement childElement = child.toElement();
            if(name == childElement.tagName())  {
                return childElement;
            }
        }
        
        child = child.nextSibling();
    }
    return QDomElement();
}

//******************************* readKeyValuePairs ****************************

QMap<QCString, QString> MyMoneyStorageGNC::readKeyValuePairs(QDomElement& element)
{
    QMap<QCString, QString> pairs;
    QDomNode child = element.firstChild();
    while(!child.isNull() && child.isElement()) {
        QDomElement childElement = child.toElement();
        if(QString("PAIR") == childElement.tagName()) {
            QCString key = QCString(childElement.attribute(QString("key")));
            QString value = childElement.attribute(QString("value"));
            
            pairs.insert(key, value);
        }
        
        child = child.nextSibling();
    }
    
    return pairs;
}

//********************** QCStringEmpty *********************************

const QCString MyMoneyStorageGNC::QCStringEmpty(const QString& val) const {
    QCString rc;
    
    if(!val.isEmpty())
        rc = QCString(val);
    
    return rc;
}

//********************** QStringEmpty **************************************

const QString MyMoneyStorageGNC::QStringEmpty(const QString& val) const {
    QString rc;
    if(!val.isEmpty())  {
        rc = QString(val);
    }
    return rc;
}

//*********************** setProgressCallback *****************************

void MyMoneyStorageGNC::setProgressCallback(void(*callback)(int, int, const QString&)) {
    m_progressCallback = callback;
}

//************************** signalProgress *******************************

void MyMoneyStorageGNC::signalProgress(int current, int total, const QString& msg) {
    if(m_progressCallback != 0)
        (*m_progressCallback)(current, total, msg);
}

//*********************** getChildCount ***************************************

const unsigned int MyMoneyStorageGNC::getChildCount(const QDomElement& element) const {
    QDomNodeList tempList = element.childNodes();
    return tempList.count();
}
