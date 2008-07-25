/***************************************************************************
                         kofxdirectconnectdlg.cpp
                             -------------------
    begin                : Sat Nov 13 2004
    copyright            : (C) 2002 by Ace Jones
    email                : acejones@users.sourceforge.net
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
#include "config.h"
#endif
#ifdef USE_OFX_DIRECTCONNECT

// ----------------------------------------------------------------------------
// QT Includes

#include <qlabel.h>
#include <qfile.h>
#include <qtextstream.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kurl.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <kprogress.h>
#include <kmessagebox.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyinstitution.h>
#include <kmymoney/mymoneyfile.h>
#include "mymoneyofxconnector.h"
#include "kofxdirectconnectdlg.h"

KOfxDirectConnectDlg::KOfxDirectConnectDlg(const MyMoneyAccount& account, QWidget *parent, const char *name)
 : KOfxDirectConnectDlgDecl(parent, name),
   m_tmpfile(NULL),
   m_connector(account),
   m_job(NULL)
{
}

KOfxDirectConnectDlg::~KOfxDirectConnectDlg()
{
    delete m_tmpfile;
}

void KOfxDirectConnectDlg::init(void)
{
  show();

  QByteArray request = m_connector.statementRequest(QDate::currentDate().addMonths(-2));

  // For debugging, dump out the request
#if 0
  QFile g( "request.ofx" );
  g.open( IO_WriteOnly );
  QTextStream(&g) << m_connector.url() << "\n" << QString(request);
  g.close();
#endif

  m_job = KIO::http_post(
    m_connector.url(),
    request,
    true
  );
  m_job->addMetaData("content-type", "Content-type: application/x-ofx" );
  connect(m_job,SIGNAL(result(KIO::Job*)),this,SLOT(slotOfxFinished(KIO::Job*)));
  connect(m_job,SIGNAL(data(KIO::Job*, const QByteArray&)),this,SLOT(slotOfxData(KIO::Job*,const QByteArray&)));
  connect(m_job,SIGNAL(connected(KIO::Job*)),this,SLOT(slotOfxConnected(KIO::Job*)));

  setStatus(QString("Contacting %1...").arg(m_connector.url()));
  kProgress1->setTotalSteps(3);
  kProgress1->setProgress(1);
}

void KOfxDirectConnectDlg::setStatus(const QString& _status)
{
  textLabel1->setText(_status);
  kdDebug(2) << "STATUS: " << _status << endl;
}

void KOfxDirectConnectDlg::setDetails(const QString& _details)
{
  kdDebug(2) << "DETAILS: " << _details << endl;
}

void KOfxDirectConnectDlg::slotOfxConnected(KIO::Job*)
{
  if ( m_tmpfile )
  {
//     throw new MYMONEYEXCEPTION(QString("Already connected, using %1.").arg(m_tmpfile->name()));
    kdDebug(2) << "Already connected, using " << m_tmpfile->name() << endl;
    delete m_tmpfile; //delete otherwise we mem leak
  }
  m_tmpfile = new KTempFile();
  setStatus("Connection established, retrieving data...");
  setDetails(QString("Downloading data to %1...").arg(m_tmpfile->name()));
  kProgress1->advance(1);
}

void KOfxDirectConnectDlg::slotOfxData(KIO::Job*,const QByteArray& _ba)
{
  if ( !m_tmpfile )
//     throw new MYMONEYEXCEPTION("Not currently connected!!");
    kdDebug(2) << "void ofxdcon::slotOfxData():: Not currently connected!" << endl;
  *(m_tmpfile->textStream()) << QString(_ba);
  setDetails(QString("Got %1 bytes").arg(_ba.size()));
}

void KOfxDirectConnectDlg::slotOfxFinished(KIO::Job* /* e */)
{
  kProgress1->advance(1);
  setStatus("Completed.");

  int error = m_job->error();

  if ( m_tmpfile )
  {
    m_tmpfile->close();
  }

  if ( error )
  {
    m_job->showErrorDialog();
  }
  else if ( m_job->isErrorPage() )
  {
    QString details;
    QFile f( m_tmpfile->name() );
    if ( f.open( IO_ReadOnly ) )
    {
      QTextStream stream( &f );
      QString line;
      while ( !stream.atEnd() ) {
          details += stream.readLine(); // line of text excluding '\n'
      }
      f.close();

      kdDebug(2) << "The HTTP request failed: " << details << endl;
    }
    KMessageBox::detailedSorry( this, i18n("The HTTP request failed."), details, i18n("Failed") );
  }
  else if ( m_tmpfile )
  {

    emit statementReady(m_tmpfile->name());

// TODO (Ace) unlink this file, when I'm sure this is all really working.
// in the meantime, I'll leave the file around to assist people in debugging.
//     m_tmpfile->unlink();
  }
  delete m_tmpfile;
  m_tmpfile = 0;
  hide();
}

void KOfxDirectConnectDlg::reject(void)
{
  m_job->kill();
  if ( m_tmpfile )
  {
    m_tmpfile->close();
    delete m_tmpfile;
    m_tmpfile = NULL;
  }
  QDialog::reject();
}

#include "kofxdirectconnectdlg.moc"
#endif // USE_OFX_DIRECTCONNECT
