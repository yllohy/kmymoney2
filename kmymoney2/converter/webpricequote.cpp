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
  // from the config gile
  if ( kapp )
  {
    if ( _sourcename.isEmpty() )
      m_source = WebPriceQuoteSource("Yahoo");
    else
      m_source = WebPriceQuoteSource(_sourcename);
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
    
    // FIXME: Ideally, we want to support 'anything else' as a separator,
    // but for now, only " > " is working.
    QRegExp splitrx("([0-9A-Za-z\\.]+) > ([0-9A-Za-z\\.]+)");
    
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

#include <qfile.h>
#include <qtextstream.h>

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
      m_price = MyMoneyMoney(priceRegExp.cap(1).toDouble()).toString();
      emit status(i18n("Price found: %1").arg(m_price.toString()));
    }

    if(dateRegExp.search(quotedata) > -1)
    {
      QString datestr = dateRegExp.cap(1);
      // TODO: Fix this temporary hack.  We know yahoo returns mm/dd/yyyy
      QRegExp dateparse("([0-9]+)/([0-9]+)/([0-9]+)");
      if ( dateparse.search( datestr ) > -1 )
      {
        gotdate = true;
        m_date = QDate( dateparse.cap(3).toInt(), dateparse.cap(1).toInt(), dateparse.cap(2).toInt() );
        emit status(i18n("Date found: %1").arg(m_date.toString()));;
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
    "[^,]*,[^,]*,\"([^\"]*)\"" // dateregexp
  );

  result["Yahoo Currency"] = WebPriceQuoteSource("Yahoo Currency", 
    "http://finance.yahoo.com/d/quotes.csv?s=%1%2=X&f=sl1d1",
    "\"([^,\"]*)\",.*",  // symbolregexp
    "[^,]*,([^,]*),.*", // priceregexp
    "[^,]*,[^,]*,\"([^\"]*)\"" // dateregexp
  );

  result["Yahoo UK"] = WebPriceQuoteSource("Yahoo UK", 
    "http://uk.finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1",
    "\"([^,\"]*)\",.*",  // symbolregexp
    "[^,]*,([^,]*),.*", // priceregexp
    "[^,]*,[^,]*,\"([^\"]*)\"" // dateregexp
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
  if ( ! groups.count() )
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

WebPriceQuoteSource::WebPriceQuoteSource(const QString& name, const QString& url, const QString& sym, const QString& price, const QString& date):
  m_name(name),
  m_url(url),
  m_sym(sym),
  m_price(price),
  m_date(date)
{
}

WebPriceQuoteSource::WebPriceQuoteSource(const QString& name)
{
  m_name = name;
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup(QString("Online-Quote-Source-%1").arg(m_name));
  m_sym = kconfig->readEntry("SymbolRegex");
  m_date = kconfig->readEntry("DateRegex");
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
