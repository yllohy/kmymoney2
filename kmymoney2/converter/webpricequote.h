/***************************************************************************
                          webpricequote.h
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

#ifndef WEBPRICEQUOTE_H 
#define WEBPRICEQUOTE_H 

// ----------------------------------------------------------------------------
// QT Headers

#include <qobject.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <qmap.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kprocess.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "../mymoney/mymoneymoney.h"

/**
Helper class to attend the process which is running the script, in the case
of a local script being used to fetch the quote.

@author Thomas Baumgart <thb@net-bembel.de> & Ace Jones <acejones@users.sourceforge.net>
*/
class WebPriceQuoteProcess: public KProcess
{
  Q_OBJECT
public:
  WebPriceQuoteProcess(void);
  void setSymbol(const QString& _symbol) { m_symbol = _symbol; m_string.truncate(0); }

public slots:
  void slotReceivedDataFromFilter(KProcess*, char*, int);
  void slotProcessExited(KProcess*);

signals:
  void processExited(const QString&);

private:
  QString m_symbol;
  QString m_string;
};

/**
  * @author Thomas Baumgart & Ace Jones
  *
  * This is a helper class to store information about an online source
  * for stock prices or currency exchange rates.
  */
struct WebPriceQuoteSource
{
  WebPriceQuoteSource() {};
  WebPriceQuoteSource(const QString& name);
  WebPriceQuoteSource(const QString& name, const QString& url, const QString& sym, const QString& price, const QString& date, const QString& dateformat);
  ~WebPriceQuoteSource() {};

  void write(void) const;
  void rename(const QString& name);
  void remove(void) const;

  QString    m_name;
  QString    m_url;
  QString    m_sym;
  QString    m_price;
  QString    m_date;
  QString    m_dateformat;
};

/**
Retrieves a price quote from a web-based quote source

@author Ace Jones <acejones@users.sourceforge.net>
*/
class WebPriceQuote: public QObject
{
  Q_OBJECT
public:
  WebPriceQuote( QObject* = 0, const char* = 0 );
  ~WebPriceQuote();
  
  /**
    * This launches a web-based quote update for the given @p _symbol.
    * When the quote is received back from the web source, it will be
    * emitted on the 'quote' signal.
    *
    * @param _symbol the trading symbol of the stock to fetch a price for
    * @param _source the source of the quote (must be a valid value returned
    *                by quoteSources().  Send QString() to use the default
    *                source.
    * @return bool Whether the quote fetch process was launched successfully
    */
  bool launch(const QString& _symbol, const QString& _source=QString());

  /**
    * This returns a list of the names of the quote sources
    * currently defined.
    *
    * @return QStringList of quote source names
    */
  static QStringList quoteSources(void);
    
signals:
  void quote(const QString&, const QDate&, const double&);
  void status(const QString&);
  void error(const QString&);
  
protected slots:
  void slotParseQuote(const QString&);

protected:
  static QMap<QString,WebPriceQuoteSource> defaultQuoteSources(void);
  
private:
  WebPriceQuoteProcess m_filter;
  QString m_symbol;
  QDate m_date;
  double m_price;
  WebPriceQuoteSource m_source;  
};

class MyMoneyDateFormat
{
public:
  MyMoneyDateFormat(const QString& _format): m_format(_format) {}
  QString convertDate(const QDate& _in) const;
  QDate convertString(const QString& _in, bool _strict=true, unsigned _centurymidpoint = QDate::currentDate().year() ) const;
  const QString& format(void) const { return m_format; }
private:
  QString m_format;
};

namespace test {

/**
Simple class to handle signals/slots for unit tests 

@author Ace Jones <acejones@users.sourceforge.net>
*/
class QuoteReceiver : public QObject
{
Q_OBJECT
public:
    QuoteReceiver(WebPriceQuote* q, QObject *parent = 0, const char *name = 0);
    ~QuoteReceiver();
public slots:
  void slotGetQuote(const QString&,const QDate&, const MyMoneyMoney&);
  void slotStatus(const QString&);
  void slotError(const QString&);
public:
  QStringList m_statuses;
  QStringList m_errors;
  MyMoneyMoney m_price;
  QDate m_date;
};

} // end namespace test


#endif // WEBPRICEQUOTE_H

// vim:cin:si:ai:et:ts=2:sw=2:
