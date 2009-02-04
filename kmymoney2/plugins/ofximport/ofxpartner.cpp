/***************************************************************************
                             ofxpartner.cpp
                             ----------
    begin                : Fri Jan 23 2009
    copyright            : (C) 2009 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qdatetime.h>
#include <qeventloop.h>
#include <qfileinfo.h>
#include <qvaluelist.h>
#include <qapplication.h>
#include <qdom.h>
#include <qregexp.h>
#include <qdir.h>
#include <qtextstream.h>

// ----------------------------------------------------------------------------
// KDE Includes


#include <kio/job.h>
#include <klocale.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ofxpartner.h"

namespace OfxPartner
{
bool post(const QString& request, const QMap<QString, QString>& attr, const KURL& url, const KURL& filename);
bool get(const QString& request, const QMap<QString, QString>& attr, const KURL& url, const KURL& filename);

const QString kBankFilename = "ofx-bank-index.xml";
const QString kCcFilename = "ofx-cc-index.xml";
const QString kInvFilename = "ofx-inv-index.xml";

#define VER "9"

static QString directory;

void setDirectory(const QString& dir)
{
  directory = dir;
}

void ValidateIndexCache(void)
{
  // TODO (Ace) Check whether these files exist and are recent enough before getting them again

  struct stat filestats;
  KURL fname;

  QMap<QString, QString> attr;
  attr["content-type"] = "application/x-www-form-urlencoded";
  attr["accept"] = "*/*";

  fname = directory + kBankFilename;
  QFileInfo i(fname.path());
  if(!i.isReadable() || i.lastModified().addDays(7) < QDateTime::currentDateTime())
    post("T=1&S=*&R=1&O=0&TEST=0", attr, KURL("http://moneycentral.msn.com/money/2005/mnynet/service/ols/filist.aspx?SKU=3&VER=" VER), fname);

  fname = directory + kCcFilename;
  i = QFileInfo(fname.path());
  if(!i.isReadable() || i.lastModified().addDays(7) < QDateTime::currentDateTime())
    post("T=2&S=*&R=1&O=0&TEST=0", attr, KURL("http://moneycentral.msn.com/money/2005/mnynet/service/ols/filist.aspx?SKU=3&VER=" VER) ,fname);

  fname = directory + kInvFilename;
  i = QFileInfo(fname.path());
  if(!i.isReadable() || i.lastModified().addDays(7) < QDateTime::currentDateTime())
    post("T=3&S=*&R=1&O=0&TEST=0", attr, KURL("http://moneycentral.msn.com/money/2005/mnynet/service/ols/filist.aspx?SKU=3&VER=" VER), fname);
}

static void ParseFile(QMap<QString, QString>& result, const QString& fileName, const QString& bankName)
{
  QFile f(fileName);
  if(f.open(IO_ReadOnly)) {
    QTextStream stream(&f);
    stream.setEncoding(QTextStream::Unicode);
    QString msg;
    int errl, errc;
    QDomDocument doc;
    if(doc.setContent(stream.read(), &msg, &errl, &errc)) {
      QDomNodeList olist = doc.elementsByTagName("prov");
      for(int i = 0; i < olist.count(); ++i) {
        QDomNode onode = olist.item(i);
        if(onode.isElement()) {
          bool collectGuid = false;
          QDomElement elo = onode.toElement();
          QDomNodeList ilist = onode.childNodes();
          for(int j = 0; j < ilist.count(); ++j) {
            QDomNode inode = ilist.item(j);
            QDomElement el = inode.toElement();
            if(el.tagName() == "name") {
              if(bankName.isEmpty())
                result[el.text()] = QString();
              else if(el.text() == bankName) {
                collectGuid = true;
              }
            }
            if(el.tagName() == "guid" && collectGuid) {
              result[el.text()] = QString();
            }
          }
        }
      }
    }
    f.close();
  }
}

QValueList<QString> BankNames(void)
{
  QMap<QString, QString> result;

  // Make sure the index files are up to date
  ValidateIndexCache();

  ParseFile(result, directory + kBankFilename, QString());
  ParseFile(result, directory + kCcFilename, QString());
  ParseFile(result, directory + kInvFilename, QString());

  // Add Innovision
  result["Innovision"] = QString();

  return result.keys();
}

QValueList<QString> FipidForBank(const QString& bank)
{
  QMap<QString, QString> result;

  ParseFile(result, directory + kBankFilename, bank);
  ParseFile(result, directory + kCcFilename, bank);
  ParseFile(result, directory + kInvFilename, bank);

  // the fipid for Innovision is 1.
  if ( bank == "Innovision" )
    result["1"] = QString();

  return result.keys();
}

QString extractNodeText(QDomElement& node, const QString& name)
{
  QString res;
  QRegExp exp("([^/]+)/?([^/].*)?");
  if(exp.search(name) != -1) {
    QDomNodeList olist = node.elementsByTagName(exp.cap(1));
    if(olist.count()) {
      QDomNode onode = olist.item(0);
      if(onode.isElement()) {
        QDomElement elo = onode.toElement();
        if(exp.cap(2).isEmpty()) {
          res = elo.text();
        } else {
          res = extractNodeText(elo, exp.cap(2));
        }
      }
    }
  }
  return res;
}

QString extractNodeText(QDomDocument& doc, const QString& name)
{
  QString res;
  QRegExp exp("([^/]+)/?([^/].*)?");
  if(exp.search(name) != -1) {
    QDomNodeList olist = doc.elementsByTagName(exp.cap(1));
    if(olist.count()) {
      QDomNode onode = olist.item(0);
      if(onode.isElement()) {
        QDomElement elo = onode.toElement();
        if(exp.cap(2).isEmpty()) {
          res = elo.text();
        } else {
          res = extractNodeText(elo, exp.cap(2));
        }
      }
    }
  }
  return res;
}

OfxFiServiceInfo ServiceInfo(const QString& fipid)
{
  OfxFiServiceInfo result;
  memset(&result, 0, sizeof(OfxFiServiceInfo));

  // Hard-coded values for Innovision test server
  if ( fipid == "1" )
  {
    strncpy(result.fid,"00000",OFX_FID_LENGTH-1);
    strncpy(result.org,"ReferenceFI",OFX_ORG_LENGTH-1);
    strncpy(result.url,"http://ofx.innovision.com",OFX_URL_LENGTH-1);
    result.accountlist = 1;
    result.statements = 1;
    result.billpay = 1;
    result.investments = 1;

    return result;
  }

  QMap<QString, QString> attr;
  attr["content-type"] = "application/x-www-form-urlencoded";
  attr["accept"] = "*/*";

  KURL guidFile(QString("%1fipid-%2.xml").arg(directory).arg(fipid));

  // Apparently at some point in time, for VER=6 msn returned an online URL
  // to a static error page (http://moneycentral.msn.com/cust404.htm).
  // Increasing to VER=9 solved the problem. This may happen again in the
  // future.
  QFileInfo i(guidFile.path());
  if(!i.isReadable() || i.lastModified().addDays(7) < QDateTime::currentDateTime())
    get("", attr, KURL(QString("http://moneycentral.msn.com/money/2005/mnynet/service/olsvcupd/OnlSvcBrandInfo.aspx?MSNGUID=&GUID=%1&SKU=3&VER=" VER).arg(fipid)), guidFile);

  QFile f(guidFile.path());
  if(f.open(IO_ReadOnly)) {
    QTextStream stream(&f);
    stream.setEncoding(QTextStream::Unicode);
    QString msg;
    int errl, errc;
    QDomDocument doc;
    if(doc.setContent(stream.read(), &msg, &errl, &errc)) {
      QString fid = extractNodeText(doc, "ProviderSettings/FID");
      QString org = extractNodeText(doc, "ProviderSettings/Org");
      QString url = extractNodeText(doc, "ProviderSettings/ProviderURL");
      strncpy(result.fid, fid.latin1(), OFX_FID_LENGTH-1);
      strncpy(result.org, org.latin1(), OFX_ORG_LENGTH-1);
      strncpy(result.url, url.latin1(), OFX_URL_LENGTH-1);
      result.accountlist = (extractNodeText(doc, "ProviderSettings/AcctListAvail") == "1");
      result.statements = (extractNodeText(doc, "BankingCapabilities/Bank") == "1");
      result.billpay= (extractNodeText(doc, "BillPayCapabilities/Pay") == "1");
      result.investments= (extractNodeText(doc, "InvestmentCapabilities/BrkStmt") == "1");
    }
  }

  return result;
}

bool get(const QString& request, const QMap<QString, QString>& attr, const KURL& url, const KURL& filename)
{
  QByteArray req(0);
  OfxHttpRequest job("GET", url, req, attr, filename, true);

  return job.error() == QHttp::NoError;
}

bool post(const QString& request, const QMap<QString, QString>& attr, const KURL& url, const KURL& filename)
{
  QByteArray req;
  req.fill(0, request.length()+1);
  req.duplicate(request.ascii(), request.length());

  OfxHttpRequest job("POST", url, req, attr, filename, true);
  return job.error() == QHttp::NoError;
}

} // namespace OfxPartner

class OfxHttpsRequest::Private
{
public:
  QFile		m_fpTrace;
};

OfxHttpsRequest::OfxHttpsRequest(const QString& type, const KURL &url, const QByteArray &postData, const QMap<QString, QString>& metaData, const KURL& dst, bool showProgressInfo) :
  d(new Private),
  m_dst(dst)
{
  QDir homeDir(QDir::home());
  if(homeDir.exists("ofxlog.txt")) {
    d->m_fpTrace.setName(QString("%1/ofxlog.txt").arg(QDir::homeDirPath()));
    d->m_fpTrace.open(IO_WriteOnly | IO_Append);
  }

  m_job = KIO::http_post(url, postData, showProgressInfo);
  m_job->addMetaData("content-type", "Content-type: application/x-ofx" );

  if(d->m_fpTrace.isOpen()) {
    QTextStream ts(&d->m_fpTrace);
    ts << "url: " << url.prettyURL() << "\n";
    ts << "request:\n" << QString(postData) << "\n" << "response:\n";
  }

  connect(m_job,SIGNAL(result(KIO::Job*)),this,SLOT(slotOfxFinished(KIO::Job*)));
  connect(m_job,SIGNAL(data(KIO::Job*, const QByteArray&)),this,SLOT(slotOfxData(KIO::Job*,const QByteArray&)));
  connect(m_job,SIGNAL(connected(KIO::Job*)),this,SLOT(slotOfxConnected(KIO::Job*)));

  qApp->enter_loop();
}

OfxHttpsRequest::~OfxHttpsRequest()
{
  if(d->m_fpTrace.isOpen()) {
    d->m_fpTrace.close();
  }
}

void OfxHttpsRequest::slotOfxConnected(KIO::Job*)
{
  m_file.setName(m_dst.path());
  m_file.open(IO_WriteOnly);
}

void OfxHttpsRequest::slotOfxData(KIO::Job*,const QByteArray& _ba)
{
  if(m_file.isOpen()) {
    QTextStream ts(&m_file);
    ts << QString(_ba);

    if(d->m_fpTrace.isOpen()) {
      d->m_fpTrace.writeBlock(_ba, _ba.size());
    }


  }
}

void OfxHttpsRequest::slotOfxFinished(KIO::Job* /* e */)
{
  if(m_file.isOpen()) {
    m_file.close();
    if(d->m_fpTrace.isOpen()) {
      d->m_fpTrace.writeBlock("\nCompleted\n\n\n\n", 14);
    }
  }

  int error = m_job->error();
  if ( error ) {
    m_job->showErrorDialog();
    unlink(m_dst.path());

  } else if ( m_job->isErrorPage() ) {
    QString details;
    QFile f( m_dst.path() );
    if ( f.open( IO_ReadOnly ) ) {
      QTextStream stream( &f );
      QString line;
      while ( !stream.atEnd() ) {
        details += stream.readLine(); // line of text excluding '\n'
      }
      f.close();
    }
    KMessageBox::detailedSorry( 0, i18n("The HTTP request failed."), details, i18n("Failed") );
    unlink(m_dst.path());
  }

  qApp->exit_loop();
}



OfxHttpRequest::OfxHttpRequest(const QString& type, const KURL &url, const QByteArray &postData, const QMap<QString, QString>& metaData, const KURL& dst, bool showProgressInfo)
{
  QFile f(dst.path());
  m_error = QHttp::NoError;
  QString errorMsg;
  if(f.open(IO_WriteOnly)) {
    m_job = new QHttp(url.host());
    QHttpRequestHeader header(type, url.encodedPathAndQuery());
    header.setValue("Host", url.host());
    QMap<QString, QString>::const_iterator it;
    for(it = metaData.begin(); it != metaData.end(); ++it) {
      header.setValue(it.key(), *it);
    }

    m_job->request(header, postData, &f);

    connect(m_job, SIGNAL(requestFinished(int, bool)),
            this, SLOT(slotOfxFinished(int, bool)));

    qApp->enter_loop();

    if(m_error != QHttp::NoError)
      errorMsg = m_job->errorString();

    delete m_job;
  } else {
    m_error = QHttp::Aborted;
    errorMsg = i18n("Cannot open file %1 for writing").arg(dst.path());
  }

  if(m_error != QHttp::NoError) {
    KMessageBox::error(0, errorMsg, i18n("OFX setup error"));
    unlink(dst.path());
  }
}

void OfxHttpRequest::slotOfxFinished(int, bool rc)
{
  if(rc) {
    m_error = m_job->error();
  }
  qApp->exit_loop();
}

#include "ofxpartner.moc"

// vim:cin:si:ai:et:ts=2:sw=2:
