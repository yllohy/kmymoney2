/***************************************************************************
                          webpricequote.cpp
                             -------------------
    begin                : Thu Dec 30 2004
    copyright            : (C) 2004 by Ace Jones
    email                : Ace Jones <acejones@users.sourceforge.net>
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
// QT Headers

#include <qfile.h>
#include <qregexp.h>
#include <qtextstream.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kio/netaccess.h>
#include <kurl.h>
#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "../mymoney/mymoneyexception.h"
#include "mymoneyqifprofile.h"
#include "webpricequote.h"

WebPriceQuote::WebPriceQuote( QObject* _parent, const char* _name ):
  QObject( _parent, _name )
{
  connect(&m_filter,SIGNAL(processExited(const QString&)),this,SLOT(slotParseQuote(const QString&)));
}

WebPriceQuote::~WebPriceQuote()
{
}
  
bool WebPriceQuote::launch( const QString& _symbol, const QString& _sourcename )
{
  bool result = true;
  m_symbol = _symbol;

  // if we're running normally, with a UI, we can just get these the normal way,
  // from the config file
  if ( kapp )
  {
    QString sourcename = _sourcename;
    if ( sourcename.isEmpty() )
      sourcename = "Yahoo";
      
    if ( quoteSources().contains(sourcename) )
      m_source = WebPriceQuoteSource(sourcename);
    else
      emit error(QString("Source <%1> does not exist.").arg(sourcename));
  }
  // otherwise, if we have no kapp, we have no config.  so we just get them from
  // the defaults
  else
  {
    if ( _sourcename.isEmpty() )
      m_source = defaultQuoteSources()["Yahoo"];
    else
      m_source = defaultQuoteSources()[_sourcename];
  }
  
  KURL url;
  
  // if the source has room for TWO symbols..
  if ( m_source.m_url.contains("%2") )
  {
    // this is a two-symbol quote.  split the symbol into two.  valid symbol
    // characters are: 0-9, A-Z and the dot.  anything else is a separator
    QRegExp splitrx("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)",false /*case sensitive*/);    
    
    // if we've truly found 2 symbols delimited this way...
    if ( splitrx.search(m_symbol) != -1 )
      url = KURL::fromPathOrURL(m_source.m_url.arg(splitrx.cap(1),splitrx.cap(2)));
    else
      kdDebug(2) << "WebPriceQuote::launch() did not find 2 symbols" << endl;
  }
  else
    // a regular one-symbol quote
    url = KURL::fromPathOrURL(m_source.m_url.arg(m_symbol));

  // If we're running a non-interactive session (with no UI), we can't
  // use KIO::NetAccess, so we have to get our web data the old-fashioned
  // way... with 'wget'.
  //
  // Note that a 'non-interactive' session right now means only the test
  // cases.  Although in the future if KMM gains a non-UI mode, this would
  // still be useful
  if ( ! kapp && ! url.isLocalFile() )
    url = KURL::fromPathOrURL("/usr/bin/wget -O - " + url.prettyURL());
    
  if ( url.isLocalFile() )
  {
    emit status(QString("Executing %1...").arg(url.path()));

    m_filter.clearArguments();
    m_filter << QStringList::split(" ",url.path());
    m_filter.setSymbol(m_symbol);

    // if we're running non-interactive, we'll need to block.
    // otherwise, just let us know when it's done.
    KProcess::RunMode mode = KProcess::NotifyOnExit;
    if ( ! kapp )
      mode = KProcess::Block;
    
    if(m_filter.start(mode, KProcess::All))
    {
      result = true;
      m_filter.resume();
    }
    else
    {
      emit error(QString("Unable to launch: %1").arg(url.path()));
      slotParseQuote(QString());
    } 
  }
  else
  {
    emit status(QString("Fetching URL %1...").arg(url.prettyURL()));

    QString tmpFile;
    if( KIO::NetAccess::download( url, tmpFile, NULL ) )
    {
      kdDebug(2) << "Downloaded " << tmpFile << endl;
      QFile f(tmpFile);
      if ( f.open( IO_ReadOnly ) )
      {
        result = true;
        QString quote = QTextStream(&f).read();
        f.close();
        slotParseQuote(quote);
      }
      else
      {
        slotParseQuote(QString());
      }
      KIO::NetAccess::removeTempFile( tmpFile );
    }
    else
    {
      emit error(KIO::NetAccess::lastErrorString());
      slotParseQuote(QString());
    }
  }
  return result;
}

void WebPriceQuote::slotParseQuote(const QString& _quotedata)
{
  QString quotedata = _quotedata;
  bool gotprice = false;
  bool gotdate = false;

//    kdDebug(2) << "WebPriceQuote::slotParseQuote( " << _quotedata << " ) " << endl;
  
  if ( ! quotedata.isEmpty() )
  {
    //
    // First, remove extranous non-data elements
    //

    // HTML tags
    quotedata.remove(QRegExp("<[^>]*>"));

    // &...;'s
    quotedata.replace(QRegExp("&[^;]+;")," ");
    
    // Extra white space
    quotedata = quotedata.simplifyWhiteSpace();      
        
    QFile file("stripped.txt");
    if ( file.open( IO_WriteOnly ) )
    {
      QTextStream( &file ) << quotedata;
      file.close();
    }
      
    QRegExp symbolRegExp(m_source.m_sym);
    QRegExp dateRegExp(m_source.m_date);
    QRegExp priceRegExp(m_source.m_price);

    if( symbolRegExp.search(quotedata) > -1)
      emit status(i18n("Symbol found: %1").arg(symbolRegExp.cap(1)));

    if(priceRegExp.search(quotedata)> -1)
    {
      gotprice = true;
      m_price = priceRegExp.cap(1).toDouble();
      emit status(i18n("Price found: %1").arg(m_price));
    }

    if(dateRegExp.search(quotedata) > -1)
    {
      QString datestr = dateRegExp.cap(1);
      
      MyMoneyDateFormat dateparse(m_source.m_dateformat);
      try
      {
        m_date = dateparse.convertString( datestr,false /*strict*/ );
        gotdate = true;
        emit status(i18n("Date found: %1").arg(m_date.toString()));;
      }
      catch (MyMoneyException* e)
      {
        emit error(i18n("Unable to parse date %1 using format %2: %3").arg(datestr,dateparse.format(),e->what()));
        delete e;
      }
    }

    if ( gotprice && gotdate )
    {
      emit quote( m_symbol, m_date, m_price );
    }
    else
    {
      emit error(i18n("Unable to update price for %1").arg(m_symbol));
    }
  }
}

QMap<QString,WebPriceQuoteSource> WebPriceQuote::defaultQuoteSources(void)
{
  QMap<QString,WebPriceQuoteSource> result;

  result["Yahoo"] = WebPriceQuoteSource("Yahoo", 
    "http://finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1",
    "\"([^,\"]*)\",.*",  // symbolregexp
    "[^,]*,([^,]*),.*", // priceregexp
    "[^,]*,[^,]*,\"([^\"]*)\"", // dateregexp
    "%m %d %y" // dateformat
  );

  result["Yahoo Currency"] = WebPriceQuoteSource("Yahoo Currency", 
    "http://finance.yahoo.com/d/quotes.csv?s=%1%2=X&f=sl1d1",
    "\"([^,\"]*)\",.*",  // symbolregexp
    "[^,]*,([^,]*),.*", // priceregexp
    "[^,]*,[^,]*,\"([^\"]*)\"", // dateregexp
    "%m %d %y" // dateformat
  );

  result["Yahoo UK"] = WebPriceQuoteSource("Yahoo UK", 
    "http://uk.finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1",
    "\"([^,\"]*)\",.*",  // symbolregexp
    "[^,]*,([^,]*),.*", // priceregexp
    "[^,]*,[^,]*,\"([^\"]*)\"", // dateregexp
    "%m %d %y" // dateformat
  );

  result["Globe & Mail"] = WebPriceQuoteSource("Globe & Mail", 
    "http://globefunddb.theglobeandmail.com/gishome/plsql/gis.price_history?pi_fund_id=%1",
    QString(),  // symbolregexp
    "Reinvestment Price \\w+ \\d+, \\d+ (\\d+\\.\\d+)", // priceregexp
    "Reinvestment Price (\\w+ \\d+, \\d+)", // dateregexp
    "%m %d %y" // dateformat
  );

  result["MSN.CA"] = WebPriceQuoteSource("MSN.CA", 
    "http://ca.moneycentral.msn.com/investor/quotes/quotes.asp?symbol=%1",
    QString(),  // symbolregexp
    "Net Asset Value (\\d+\\.\\d+)", // priceregexp
    "NAV update (\\d+\\D+\\d+\\D+\\d+)", // dateregexp
    "%d %m %y" // dateformat
  );
    
  return result;
}

QStringList WebPriceQuote::quoteSources(void)
{
  KConfig *kconfig = KGlobal::config();
  QStringList groups = kconfig->groupList();

  QStringList::Iterator it;
  QRegExp onlineQuoteSource(QString("^Online-Quote-Source-(.*)$"));

  // get rid of all 'non online quote source' entries
  for(it = groups.begin(); it != groups.end(); it = groups.remove(it)) {
    if(onlineQuoteSource.search(*it) >= 0) {
      // Insert the name part
      groups.insert(it, onlineQuoteSource.cap(1));
    }
  }

  // if the user has the OLD quote source defined, now is the
  // time to remove that entry and convert it to the new system.
  if ( ! groups.count() && kconfig->hasGroup("Online Quotes Options") )
  {
    kconfig->setGroup("Online Quotes Options");
    QString url(kconfig->readEntry("URL","http://finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1"));
    QString symbolRegExp(kconfig->readEntry("SymbolRegex","\"([^,\"]*)\",.*"));
    QString priceRegExp(kconfig->readEntry("PriceRegex","[^,]*,([^,]*),.*"));
    QString dateRegExp(kconfig->readEntry("DateRegex","[^,]*,[^,]*,\"([^\"]*)\""));
    kconfig->deleteGroup("Online Quotes Options");
    
    groups += "Old Source";
    kconfig->setGroup(QString("Online-Quote-Source-%1").arg("Old Source"));
    kconfig->writeEntry("URL", url);
    kconfig->writeEntry("SymbolRegex", symbolRegExp);
    kconfig->writeEntry("PriceRegex",priceRegExp);
    kconfig->writeEntry("DateRegex", dateRegExp);
    kconfig->writeEntry("DateFormatRegex", "%m %d %y");
    kconfig->sync();
  }

  // Set up each of the default sources.  These are done piecemeal so that
  // when we add a new source, it's automatically picked up.
  QMap<QString,WebPriceQuoteSource> defaults = defaultQuoteSources();
  QMap<QString,WebPriceQuoteSource>::const_iterator it_source = defaults.begin();
  while ( it_source != defaults.end() )
  {
    if ( ! groups.contains( (*it_source).m_name ) )
    {
      groups += (*it_source).m_name;
      (*it_source).write();
      kconfig->sync();
    }
    ++it_source;
  }
    
  return groups;
}

//
// Helper class to load/save an individual source
//

WebPriceQuoteSource::WebPriceQuoteSource(const QString& name, const QString& url, const QString& sym, const QString& price, const QString& date, const QString& dateformat):
  m_name(name),
  m_url(url),
  m_sym(sym),
  m_price(price),
  m_date(date),
  m_dateformat(dateformat)
{
}

WebPriceQuoteSource::WebPriceQuoteSource(const QString& name)
{
  m_name = name;
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup(QString("Online-Quote-Source-%1").arg(m_name));
  m_sym = kconfig->readEntry("SymbolRegex");
  m_date = kconfig->readEntry("DateRegex");
  m_dateformat = kconfig->readEntry("DateFormatRegex","%m %d %y");
  m_price = kconfig->readEntry("PriceRegex");
  m_url = kconfig->readEntry("URL");
}

void WebPriceQuoteSource::write(void) const
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup(QString("Online-Quote-Source-%1").arg(m_name));
  kconfig->writeEntry("URL", m_url);
  kconfig->writeEntry("PriceRegex", m_price);
  kconfig->writeEntry("DateRegex", m_date);
  kconfig->writeEntry("DateFormatRegex", m_dateformat);
  kconfig->writeEntry("SymbolRegex", m_sym);
}

void WebPriceQuoteSource::rename(const QString& name)
{
  remove();
  m_name = name;
  write();
}

void WebPriceQuoteSource::remove(void) const
{
  KConfig *kconfig = KGlobal::config();
  kconfig->deleteGroup(QString("Online-Quote-Source-%1").arg(m_name));
}

//
// Helper class to babysit the KProcess used for running the local script in that case
//

WebPriceQuoteProcess::WebPriceQuoteProcess(void)
{
  connect(this, SIGNAL(receivedStdout(KProcess*, char*, int)), this, SLOT(slotReceivedDataFromFilter(KProcess*, char*, int)));
  connect(this, SIGNAL(processExited(KProcess*)), this, SLOT(slotProcessExited(KProcess*)));
}

void WebPriceQuoteProcess::slotReceivedDataFromFilter(KProcess* /*_process*/, char* _pcbuffer, int _nbufferlen)
{
  QByteArray data;
  data.duplicate(_pcbuffer, _nbufferlen);
  
//   kdDebug(2) << "WebPriceQuoteProcess::slotReceivedDataFromFilter(): " << QString(data) << endl;

  m_string += QString(data);
}

void WebPriceQuoteProcess::slotProcessExited(KProcess*)
{
//   kdDebug(2) << "WebPriceQuoteProcess::slotProcessExited()" << endl;
  
  emit processExited(m_string);
  m_string.truncate(0);
}

//
// Universal date converter
//

// In 'strict' mode, this is designed to be compatable with the QIF profile date
// converter.  However, that converter deals with the concept of an apostrophe
// format in a way I don't understand.  So for the moment, they are 99% 
// compatable, waiting on that issue. (acejones)

QDate MyMoneyDateFormat::convertString(const QString& _in, bool _strict, unsigned _centurymidpoint) const
{
  //
  // Break date format string into component parts
  //
  
  QRegExp formatrex("%([mdy]+)(\\W+)%([mdy]+)(\\W+)%([mdy]+)",false /* case sensitive */);
  if ( formatrex.search(m_format) == -1 )
  {
    throw new MYMONEYEXCEPTION("Invalid format string");
  }
  
  QStringList formatParts;
  formatParts += formatrex.cap(1);
  formatParts += formatrex.cap(3);
  formatParts += formatrex.cap(5);
  
  QStringList formatDelimiters;
  formatDelimiters += formatrex.cap(2);
  formatDelimiters += formatrex.cap(4);
  
  //
  // Break input string up into component parts,
  // using the delimiters found in the format string
  //
  
  QRegExp inputrex;
  inputrex.setCaseSensitive(false);
  
  // strict mode means we must enforce the delimiters as specified in the 
  // format.  non-strict allows any delimiters
  if ( _strict )
    inputrex.setPattern(QString("(\\w+)%1(\\w+)%2(\\w+)").arg(formatDelimiters[0],formatDelimiters[1]));
  else
    inputrex.setPattern("(\\w+)\\W+(\\w+)\\W+(\\w+)");

  if ( inputrex.search(_in) == -1 )
  {
    throw new MYMONEYEXCEPTION("Invalid input string");
  }

  QStringList scannedParts;
  scannedParts += inputrex.cap(1).lower();
  scannedParts += inputrex.cap(2).lower();
  scannedParts += inputrex.cap(3).lower();

  //
  // Convert the scanned parts into actual date components
  //
  
  unsigned day = 0, month = 0, year = 0;
  bool ok;
  QRegExp digitrex("(\\d+)");
  QStringList::const_iterator it_scanned = scannedParts.begin();
  QStringList::const_iterator it_format = formatParts.begin();
  while ( it_scanned != scannedParts.end() )
  {
    switch ( (*it_format)[0] )
    {
    case 'd':
      // remove any extraneous non-digits (e.g. read "3rd" as 3)
      ok = false;
      if ( digitrex.search(*it_scanned) != -1 )
        day = digitrex.cap(1).toUInt(&ok);
      if ( !ok || day > 31 )
        throw new MYMONEYEXCEPTION(QString("Invalid day entry: %1").arg(*it_scanned));
      break;
    case 'm':
      month = (*it_scanned).toUInt(&ok);
      if ( !ok )
      {
        // maybe it's a textual date
        unsigned i = 1;
        while ( i <= 12 )
        {
          if (  QDate::shortMonthName(i).lower() == *it_scanned || QDate::longMonthName(i).lower() == *it_scanned )
            month = i;
          ++i;
        }
      }
      
      if ( month < 1 || month > 12 )
        throw new MYMONEYEXCEPTION(QString("Invalid month entry: %1").arg(*it_scanned));
      
      break;
    case 'y':
      if ( _strict && (*it_scanned).length() != (*it_format).length())
        throw new MYMONEYEXCEPTION(QString("Length of year (%1) does not match expected length (%2).")
                .arg(*it_scanned,*it_format));
      
      year = (*it_scanned).toUInt(&ok);
      
      if (!ok)
        throw new MYMONEYEXCEPTION(QString("Invalid year entry: %1").arg(*it_scanned));
        
      //
      // 2-digit year case
      //
      // this algorithm will pick a year within +/- 50 years of the 
      // centurymidpoint parameter.  i.e. if the midpoint is 2000,
      // then 0-49 will become 2000-2049, and 50-99 will become 1950-1999
      if ( year < 100 )
      {
        unsigned centuryend = _centurymidpoint + 50;
        unsigned centurybegin = _centurymidpoint - 50;
        
        if ( year < centuryend % 100 )
          year += 100;
        year += centurybegin - centurybegin % 100;
      }
      
      if ( year < 1900 )
        throw new MYMONEYEXCEPTION(QString("Invalid year (%1)").arg(year));
      
      break;
    default:
      throw new MYMONEYEXCEPTION("Invalid format character");
    }
  
    ++it_scanned;
    ++it_format; 
  }
  
  QDate result(year,month,day);
  if ( ! result.isValid() )
    throw new MYMONEYEXCEPTION(QString("Invalid date (yr%1 mo%2 dy%3)").arg(year).arg(month).arg(day));

  return result;
}

//
// Unit test helpers
//

test::QuoteReceiver::QuoteReceiver(WebPriceQuote* q, QObject* parent, const char *name) :
  QObject(parent,name)
{
  connect(q,SIGNAL(quote(const QString&,const QDate&, const MyMoneyMoney&)),
    this,SLOT(slotGetQuote(const QString&,const QDate&, const MyMoneyMoney&)));
  connect(q,SIGNAL(status(const QString&)),
    this,SLOT(slotStatus(const QString&)));
  connect(q,SIGNAL(error(const QString&)),
    this,SLOT(slotError(const QString&)));
}

test::QuoteReceiver::~QuoteReceiver()
{
}

void test::QuoteReceiver::slotGetQuote(const QString&,const QDate& d, const MyMoneyMoney& m)
{
//   kdDebug(2) << "test::QuoteReceiver::slotGetQuote( , " << d << " , " << m.toString() << " )" << endl;
 
  m_price = m;
  m_date = d;
}
void test::QuoteReceiver::slotStatus(const QString& msg)
{
//   kdDebug(2) << "test::QuoteReceiver::slotStatus( " << msg << " )" << endl;
  
  m_statuses += msg;
}
void test::QuoteReceiver::slotError(const QString& msg)
{
//   kdDebug(2) << "test::QuoteReceiver::slotError( " << msg << " )" << endl;
  
  m_errors += msg;
}

// vim:cin:si:ai:et:ts=2:sw=2:
