/***************************************************************************
                          mymoneyonlinepriceupdate.cpp
                          -------------------
    copyright            : (C) 2004 by Kevin Tambascio, 2004 by Thomas Baumgart, 2004 by Tony Bloomfield
    email                : ktambascio@users.sourceforge.net, 
                           ipwizard@users.sourceforge.net,
                           tonyb.lx@btinternet.com
    
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
#include <qdatetime.h>
#include <qmap.h>
#include <qstringlist.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneyonlinepriceupdate.h"
#include "mymoneyexception.h"

// to control sequence of input to quoter
enum inputStateE {XMLHDR, DOCTYPE, FILESTART, BASECURR, BEGINV, INVEST, ENDINV,
                   BEGCURR, CURRCY, ENDCURR, ENDFILE, ENDINPUT };

                   
//*********************** MMQProcess ********************************************

//constructor
MMQProcess::MMQProcess() {
}

//destructor
MMQProcess::~MMQProcess() {
}

// kick off the process

void MMQProcess::execute (void *obj, QString path, QString scriptArg, void (*is)(void* obj) ) {
    
    //m_qp = q; // should have function pointer i/o object ptr
    m_is = is;
    m_obj = obj;
    addArgument (m_perlBinary);
    addArgument (path);
    if (scriptArg) addArgument (scriptArg);
    // set up input/output signals
    connect(this, SIGNAL(readyReadStdout()),
              SLOT(readFromStdout()) );
    connect( this, SIGNAL(readyReadStderr()),
                SLOT(readFromStderr()) );
    m_stdoutBuffer = "";
    m_stderrBuffer = "";
    
    if (m_is) {
        connect (this, SIGNAL(wroteToStdin()), SLOT (wroteStdin()));
    }
   // start the process
    if (!start()) {
        QString e = QString (QObject::tr("Unable to start process for script file %1")).arg(path);
        throw new MYMONEYEXCEPTION(e);
    }
    // send initial input if any
   if (m_is) m_is(m_obj);
     // wait for it to finish
    while (isRunning()) {
        qApp->processEvents();
    }
    if ((normalExit()) && (exitStatus() == 0)) {
        m_endStatus = TRUE;
    } else {
        m_endStatus = FALSE;
    }
}


MyMoneyOnlinePriceUpdate::MyMoneyOnlinePriceUpdate()
  : m_perlBinary("/usr/bin/perl"),
    m_perlScript("/home/tonyb/devkmm/FQuote/bin/mymoneyquoter.pl"),
    m_baseCurrency("GBP")
{
}


MyMoneyOnlinePriceUpdate::~MyMoneyOnlinePriceUpdate()
{
}

// get a single currency quote
const MyMoneyCurrency MyMoneyOnlinePriceUpdate::getQuote(const MyMoneyCurrency& quoteItem)
{
    QValueList<MyMoneyCurrency> cl;
    cl.append (quoteItem);
    cl = getQuotes (cl);
    return (cl.first());
}
    
// get multiple currency quotes

const QValueList<MyMoneyCurrency> MyMoneyOnlinePriceUpdate::getQuotes(const QValueList<MyMoneyCurrency>& quoteList)
{
    m_ql = (QValueList<MyMoneyEquity>* ) (&quoteList);
    m_quoteType = MyMoneyOnlinePriceUpdate::Currency;
    retrieve();
    return (*(reinterpret_cast <QValueList<MyMoneyCurrency>*> (m_ql)));
}

// get a single equity quote
const MyMoneyEquity MyMoneyOnlinePriceUpdate::getQuote(const MyMoneyEquity& quoteItem) {
    
    QValueList<MyMoneyEquity> cl;
    cl.append (quoteItem);
    cl = getQuotes (cl);
    return (cl.first());
}
    
// get  multiple equity quotes
const QValueList<MyMoneyEquity> MyMoneyOnlinePriceUpdate::getQuotes(const QValueList<MyMoneyEquity>& quoteList)
{
    m_ql = (QValueList<MyMoneyEquity>* ) (&quoteList);
    m_quoteType = MyMoneyOnlinePriceUpdate::Equity;
    retrieve();
    return (*(reinterpret_cast <QValueList<MyMoneyEquity>*> (m_ql)));
}

// get a list of F::Q price sources
const QMap<QString, QString> MyMoneyOnlinePriceUpdate::getSources()
{
    QMap<QString, QString> sList;
    QString line;
    int i = 0;
    
    checkScript();
    m_p.execute (this, m_scriptPath, "-l", NULL);
    if (m_p.failed()) {
        QString e = QString (QObject::tr("Script %1 - list failed\n%2"))
                    .arg(m_scriptPath).arg(m_p.getStderr().latin1());
        throw new MYMONEYEXCEPTION(e);
    }
    do {
        line = m_p.getStdout().section ((char)0x0a, i, i++, QString::SectionSkipEmpty);
        // qDebug (line);
        // FIXME - get (manually) a list of sensible exchange names to pair with the obscure FQ ones
        if (line != "") sList.insert (line, line.upper());
    } while (line != "");
    return (sList);
    
}

// ******************************************************************************************
// static wrapper function to be able to callback the member function inputSource() 
void MyMoneyOnlinePriceUpdate::inputWrapper(void* obj) {
    
     // explicitly cast to a pointer to MyMoneyQuoter 
     MyMoneyOnlinePriceUpdate* me = (MyMoneyOnlinePriceUpdate*) obj; 
     // call member 
     me->inputSource(); 
 } 

void MyMoneyOnlinePriceUpdate::retrieve () {
    
    checkScript(); // throws exception on failure
    
    m_qit = m_ql->begin();
    m_inputState = XMLHDR;
    m_p.execute ((void *)this, m_scriptPath, NULL, MyMoneyOnlinePriceUpdate::inputWrapper);
    // qDebug ("out = %s", m_p.getStdout().latin1());
    if (m_p.failed()) {
        QString e = QString (QObject::tr("Script %1 failed\n%2"))
                    .arg(m_scriptPath).arg(m_p.getStderr().latin1());
        throw new MYMONEYEXCEPTION(e);
    }
    m_qit = m_ql->begin();
    parseOutput (m_p.getStdout());
    return;
}
//*********************************************************************************
// this routine will write the XML to the script
void MyMoneyOnlinePriceUpdate::inputSource () {
    
    //qDebug ("providing input at state %d", inputState);
    
  /*  //FIXME - it is probably neither efficient nor required to write separate lines for each statement
    // however, it may make life easier if we convert this code to use qdom or something
    switch (m_inputState) {
    case XMLHDR:
        m_p.writeToStdin ("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
        m_inputState = DOCTYPE;
        break;
    case DOCTYPE:
        m_p.writeToStdin ("<!DOCTYPE KMYMONEYQUOTER-FILE>\n");
        m_inputState = FILESTART;
        break;
    case FILESTART:
        m_p.writeToStdin ("<KMYMONEYQUOTER-FILE>\n");
        m_inputState = BASECURR;
        break;
    case BASECURR:
        // FIXME - get users base currency
        m_p.writeToStdin ("<BASECURRENCY id=\"GBP\"/>\n");
        // work out next state
        if (m_quoteType == MyMoneyOnlinePriceUpdate::Equity) {
             m_inputState = BEGINV;
        } else {
            m_inputState = BEGCURR;
        }
        break;
    case BEGINV:
        m_p.writeToStdin ("<INVESTMENTS>\n");
        m_inputState =INVEST;
        break;
    case INVEST:
        // write equity details
        m_p.writeToStdin (QString("<EQUITY source = \"%1\" id = \"%2\"/>")
                        .arg((*m_qit).exchange()).arg((*m_qit).symbol()));
        if (++m_qit == m_ql->end()) m_inputState = ENDINV;
        break;
    case ENDINV:
        m_p.writeToStdin ("</INVESTMENTS>\n");
        m_inputState = ENDFILE;
        break;
    case BEGCURR:
        m_p.writeToStdin ("<CURRENCIES>\n");
        m_inputState = CURRCY;
        break;
    case CURRCY:
        // write currency details
        m_p.writeToStdin (QString("<CURRENCY id = \"%1\"/>")
                        .arg((*m_qit).symbol())); 
        if (++m_qit == m_ql->end()) m_inputState = ENDCURR;
        break;
    case ENDCURR:
        m_p.writeToStdin ("</CURRENCIES>\n");
        m_inputState = ENDFILE;
        break;
    case ENDFILE:
        m_p.writeToStdin ("</KMYMONEYQUOTER-FILE>\n");
        m_inputState = ENDINPUT;
        break;
    case ENDINPUT:
        m_p.closeStdin();
    }
    
    qApp->processEvents();*/
}

//*********************************************************************************
// this routine will parse the returned XML and update the input items

void MyMoneyOnlinePriceUpdate::parseOutput (const QString& xmlBuffer) {
    
   /* QDomDocument doc;
    // do some sanity checks
    if (!doc.setContent (xmlBuffer, FALSE)) {
        QString e = QString (QObject::tr("Could not parse XML output from script %1")).arg(m_scriptPath);
        throw new MYMONEYEXCEPTION(e);
    }
    QDomElement rootElement = doc.documentElement();
    if (rootElement.isNull()) {
        QString e = QString (QObject::tr("Script %1 returned empty file")).arg(m_scriptPath);
        throw new MYMONEYEXCEPTION(e);
    }
    if (rootElement.tagName() != QString("KMYMONEYQUOTER-FILE")) {
        QString e = QString (QObject::tr("Script %1 returned invalid file header - %2"))
                    .arg(m_scriptPath).arg(rootElement.tagName());
        throw new MYMONEYEXCEPTION(e);
    }
 
    QDomNode child = rootElement.firstChild();
    while(!child.isNull() && child.isElement())
    {
        QDomElement childElement = child.toElement();
        //qDebug("Processing child node %s", childElement.tagName().data());
        if (childElement.tagName() == "BASECURRENCY") {
            // nowt to do?
        }
        if (childElement.tagName() == QString("INVESTMENTS")) {
            readInvestments (childElement);
        }
        if (childElement.tagName() == "CURRENCIES") {
            readCurrencies (childElement);
        }
    
        child = child.nextSibling();
    }
    return;*/
}

//
void MyMoneyOnlinePriceUpdate::readInvestments (const QDomElement& elem) {

  /*  QDomNodeList nodeList = elem.childNodes();
    for (unsigned int x = 0; x < nodeList.count(); x++) {
        
        QString source, id, errcode, pdate, eqcurr, eqrate, price;
        
        QDomElement temp = nodeList.item(x).toElement();
        source = temp.attributes().namedItem(QString("source")).nodeValue();
        id = temp.attributes().namedItem(QString("id")).nodeValue();
        
        QDomElement priceData = temp.firstChild().toElement();
        errcode = priceData.attributes().namedItem(QString("error")).nodeValue();
        if (errcode == "0" ) {
            pdate = priceData.attributes().namedItem(QString("date")).nodeValue();
            eqcurr = priceData.attributes().namedItem(QString("currency")).nodeValue();
            eqrate = priceData.attributes().namedItem(QString("rate")).nodeValue();
            price = priceData.attributes().namedItem(QString("price")).nodeValue();
        }
        // match against next in list, update data
        // items should be returned in same order as requested
        if (((*m_qit).exchange() != source) || ((*m_qit).symbol() != id))
            qFatal ("%s != %s or %s != %s",
                    (*m_qit).exchange().latin1(), source.latin1(), (*m_qit).symbol().latin1(), id.latin1());
        (*m_qit).m_errorCode = (MMQItem::quoterErrorsE)errcode.toInt();
        if (errcode == "0") {
            (*m_qit).m_quoteDate = QDate::fromString(pdate, Qt::ISODate);
            (*m_qit).m_quoteCurrency = eqcurr;
            (*m_qit).m_quoteRate = eqrate;
            (*m_qit).m_value= price;
        }
        // FIXME - convert curr/rate to MyMoneyMoney, MyMopneyCurrency or whatever
        m_qit++;
    }    
    return;  */  
}

//
void MyMoneyOnlinePriceUpdate::readCurrencies (const QDomElement& elem) {
    
 /*   QDomNodeList nodeList = elem.childNodes();
    for (unsigned int x = 0; x < nodeList.count(); x++) {
        
        QString source, id, errcode, pdate, rate;
        
        QDomElement temp = nodeList.item(x).toElement();
        source = temp.attributes().namedItem(QString("source")).nodeValue();
        id = temp.attributes().namedItem(QString("id")).nodeValue();
        
        QDomElement priceData = temp.firstChild().toElement();
        errcode = priceData.attributes().namedItem(QString("error")).nodeValue();
        if (errcode == "0" ) {
            pdate = priceData.attributes().namedItem(QString("date")).nodeValue();
            rate = priceData.attributes().namedItem(QString("rate")).nodeValue();
        }
        
        // match against next in list, update data
        // items should be returned in same order as requested
        if ((*m_qit).symbol() != id)
            qFatal ("%s != %s", (*m_qit).symbol().latin1(), id.latin1());
        (*m_qit).m_errorCode = (MyMoneyEquity::quoterErrorsE)errcode.toInt();
        if (errcode == "0") {
            (*m_qit).m_quoteDate = QDate::fromString(pdate, Qt::ISODate);
            // FIXME - convert rate to MyMoneyMoney
            (*m_qit).m_value= rate;
        }
        m_qit++;
    }
    return;  */  
}

//**********************************************************************************
// this routine checks that the quoter script file is executable, then calls it with a 'test' flag
//  the script should return with a status of 0 if all is ok
// otherwise, a non-zero status with error details on stderr

void MyMoneyOnlinePriceUpdate::checkScript() {
    
    if (m_scriptOK) return;
    // FIXME - initialize script path to user script, or kmymoney path location
    m_scriptPath = m_perlScript;
    QFileInfo f(m_scriptPath);
    if (!f.isExecutable()) {
        QString e = QString (QObject::tr("Script file %1 is not executable")).arg(m_scriptPath);
        throw new MYMONEYEXCEPTION(e);
     }
     //
    m_p.execute (this, m_scriptPath, "-t", NULL);
    if (m_p.failed()) {
        QString e;
        if (m_p.exitStatus() == MMQProcess::MissingModules) {
            // FIXME - document in README (or somewhere)
            e = QString (QObject::tr("You need to install the following perl modules - see README\n%1"))
                .arg(m_p.getStderr().latin1());
        } else {
            e = QString (QObject::tr("Script %1 failed\n%2"))
                .arg(m_scriptPath).arg(m_p.getStderr().latin1());
        }
        throw new MYMONEYEXCEPTION(e);
    } else {
        m_scriptOK = TRUE;
    } 
}

void MyMoneyOnlinePriceUpdate::setPerlLocation(const QString& strLoc)
{
  m_perlBinary = strLoc;
}

int MyMoneyOnlinePriceUpdate::getQuotes(const QStringList& symbolNames)
{
  return 0;
}

int MyMoneyOnlinePriceUpdate::applyNewQuotes()
{
  return 0;
}

int MyMoneyOnlinePriceUpdate::getLastUpdateDate(const QString& symbolName, QDate& date)
{
  return 0;
}

int MyMoneyOnlinePriceUpdate::getLastValue(const QString& symbolName, MyMoneyMoney& value)
{
  return 0;
}
