/***************************************************************************
                          mymoneyonlinepriceupdate.h
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

#ifndef MYMONEYONLINEPRICEUPDATE_H
#define MYMONEYONLINEPRICEUPDATE_H

#include "mymoneymoney.h"
#include "mymoneycurrency.h"
#include "mymoneyequity.h"

#include <qdatetime.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qfileinfo.h>
#include <qobject.h>
#include <qapplication.h>
#include <qstring.h>
#include <qprocess.h>
#include <qdom.h>

typedef struct {
  QString symbolName;
  MyMoneyMoney value;
  QDate lastUpdated;
  int error;
} OnlineUpdateStruct;

#define SUCCESS                         0x0
#define SUCCESS_WITH_ERRORS             0x1
#define ERROR_SYMBOL_NAME_NOT_FOUND     0x100
#define ERROR_PERL_NOT_FOUND            0x101
#define ERROR_NO_CONNECTION             0x102

//******************************************************************************************
// This class runs the quoter process
class MMQProcess : public QProcess
{
  Q_OBJECT          // for signal/slots
public:
    MMQProcess ();        // constructor
    ~MMQProcess();        // destructor
    // execute the process
    void execute (void *, QString, QString, void (*)(void *));
    // get quoter's standard output/ error
    QString getStdout () { return QString(m_stdoutBuffer); };
    QString getStderr () { return QString(m_stderrBuffer); };
    // whether quoter process failed
    bool failed() { return (!m_endStatus); };
    // Script error codes
    typedef enum _scriptErrorsE {
        ProcessOK = 0,          // no problems
        MissingModules = -2,    // Finance::Quote interface, missing Perl modules
        PerlError = -1        // perl error, shouldn't happen
      } scriptErrorsE;
    
private:
    QString m_perlBinary;
    bool m_endStatus;
    
    void (* m_is) (void *);     // stdin callback function pointer
    void *m_obj;          // calling object ptr
    // buffers
    QString m_stdoutBuffer;
    QString m_stderrBuffer;
    
public slots:
    void readFromStdout() { m_stdoutBuffer += readStdout(); };
    void readFromStderr() { m_stderrBuffer += readStderr(); };
    void wroteStdin () { if (m_is) m_is(m_obj); };
        
};


//******************************************************************************************
// This is the class called by the user to retrieve quotes
class MyMoneyOnlinePriceUpdate
{
public:
    MyMoneyOnlinePriceUpdate();     // constructor
    ~MyMoneyOnlinePriceUpdate();      // destructor
    
    ///Retrieves quotes using
    int getQuotes(const QStringList& symbolNames);
    
    ///Tells this object to update price histories of MyMoneyEquity objects.
    int applyNewQuotes();
    
    ///Retrieves the date of the last update for this symbol name.
    int getLastUpdateDate(const QString& symbolName, QDate& date);
    
    ///Retrieves the price returned from the quote engine.  
    int getLastValue(const QString& symbolName, MyMoneyMoney& value);

private:    
    // get a single currency quote
    const MyMoneyCurrency getQuote(const MyMoneyCurrency& quoteItem); // throw MyMoneyException; 
    
    // get a list of quotes
    const QValueList<MyMoneyCurrency> getQuotes(const QValueList<MyMoneyCurrency>&); // throw MyMoneyException;
    
    // get a single stock quote
    const MyMoneyEquity getQuote(const MyMoneyEquity& quoteItem); // throw MyMoneyException;
    
    // get a list of quotes
    const QValueList<MyMoneyEquity> getQuotes(const QValueList<MyMoneyEquity>&); // throw MyMoneyException; */
    
    // get a list of F::Q price sources (key = name used by f::q, data = sensible name)
    const QMap<QString, QString> getSources();
    
    ///Sets the location of the perl binary for our use.
    void setPerlLocation(const QString& strLoc);

    void retrieve (); // throw MyMoneyException
    
    void parseOutput (const QString&);  // throw MyMoneyException
    
    void readInvestments (const QDomElement&); // throw MyMoneyException
    
    void readCurrencies (const QDomElement&); // throw MyMoneyException
    
    void checkScript(); // throw MyMoneyException // to check presence and executability of script
    
    // to provide the XML input to the quoter
    void inputSource(); 
    
    static void inputWrapper(void*);  // static wrapper for callback
    
    QValueList<MyMoneyEquity> *m_ql;      // list of items to quote for
    QValueList<MyMoneyEquity>::iterator m_qit;
    
    typedef enum _quoteTypeE {
        Equity, Currency
      } quoteTypeE;
    int m_quoteType;        //
    
    int m_inputState;       // to control XML input sequence
    
    MMQProcess m_p;     // quoter process 
    // a couple of static variables, so we do check_script once only
    bool m_scriptOK;   // checkScript() worked okay
    QString m_scriptPath;    // path to quoter script
    
    QString m_perlBinary;// ("/usr/bin/perl");
    QString m_perlScript;// ("/home/tonyb/devkmm/FQuote/bin/mymoneyquoter.pl");
    QString m_baseCurrency;// ("GBP");
};

/*class MyMoneyOnlinePriceUpdate{
public:
    MyMoneyOnlinePriceUpdate();
    ~MyMoneyOnlinePriceUpdate();

 
 
    
private:
    QString m_strPerlLocation;
    QMap<QString, OnlineUpdateStruct> m_data;
};*/

#endif
