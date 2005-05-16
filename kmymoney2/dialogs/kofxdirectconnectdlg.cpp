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

// ----------------------------------------------------------------------------
// QT Includes

#include <qlabel.h>

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

#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneyfile.h"
#include "../converter/mymoneyofxstatement.h"
#include "mymoneyofxconnector.h"
#include "kofxdirectconnectdlg.h"

KOfxDirectConnectDlg::KOfxDirectConnectDlg(const MyMoneyAccount& account, QWidget *parent, const char *name)
 : KOfxDirectConnectDlgDecl(parent, name),
   m_tmpfile(NULL),
   m_statement(NULL),
   m_connector(account),
   m_job(NULL)
{
}

KOfxDirectConnectDlg::~KOfxDirectConnectDlg()
{
  if ( m_statement )
    delete m_statement;
  if ( m_tmpfile )
    delete m_tmpfile;
}

void KOfxDirectConnectDlg::init(void)
{
  show();

  m_job = KIO::http_post(
    m_connector.url(), 
    m_connector.statementRequest(QDate::currentDate().addMonths(-2)), 
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
//     throw new MYMONEYEXCEPTION(QString("Already connected, using %1.").arg(m_tmpfile->name()));
    kdDebug(2) << "Already connected, using " << m_tmpfile->name() << endl;
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

void KOfxDirectConnectDlg::slotOfxFinished(KIO::Job*)
{
  kProgress1->advance(1);
  setStatus("Completed.");
  
  int error = m_job->error();
  
  if ( error )
  {
    m_job->showErrorDialog();

    if ( m_statement )
      delete m_statement;
    m_statement = 0;
    if ( m_tmpfile )
    {
      m_tmpfile->close();
      delete m_tmpfile;
    }
    m_tmpfile = 0;
  } 
  else if ( m_tmpfile )
  {
    m_tmpfile->close();  
    
    if ( m_statement )
      delete m_statement;
    
    m_statement = new MyMoneyOfxStatement(m_tmpfile->name());
    
// TODO: unlink this file, when I'm sure this is all really working.
// in the meantime, I'll leave the file around to assist people in debugging.
//     m_tmpfile->unlink();
    delete m_tmpfile;
    m_tmpfile = 0;

    emit statementReady(*m_statement);    
  }
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
  if ( m_statement )
  {
    delete m_statement;
    m_statement = 0;
  }
  QDialog::reject();
}

#include "kofxdirectconnectdlg.moc"
